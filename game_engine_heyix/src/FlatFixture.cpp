#define _USE_MATH_DEFINES
#include <cmath>
#include "FlatFixture.h"
#include "FlatTransform.h"
#include "FlatBody.h"
#include <iostream>
#include "Collision.h"
namespace {
    inline float PolygonArea(const std::vector<Vector2>& verts) {
        const size_t n = verts.size();
        if (n < 3) return 0.0f;
        double sum = 0.0;
        for (size_t i = 0; i < n; ++i) {
            const Vector2& a = verts[i];
            const Vector2& b = verts[(i + 1) % n];
            sum += static_cast<double>(a.x()) * static_cast<double>(b.y())
                - static_cast<double>(a.y()) * static_cast<double>(b.x());
        }
        return static_cast<float>(0.5 * sum);
    }
}
namespace FlatPhysics {
    FlatAABB FlatPhysics::FlatFixture::GetAABB()
    {
        if (!body_ || !shape_) {
            Vector2 p = body_ ? body_->GetPosition() : Vector2::Zero();
            return FlatAABB(p, p);
        }

        const auto transform = body_->GetTransform();

        float minX = std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float maxY = std::numeric_limits<float>::lowest();

        switch (shape_->GetType()) {
        case ShapeType::Circle: {
            const auto* circle = shape_->AsCircle();
            Vector2 center = FlatTransform::TransformVector(circle->center, transform);
            float radius = circle->radius;

            minX = center.x() - radius;
            minY = center.y() - radius;
            maxX = center.x() + radius;
            maxY = center.y() + radius;
            break;
        }
        case ShapeType::Polygon: {
            const auto* polygon = shape_->AsPolygon();
            for (const auto& local : polygon->vertices) {
                Vector2 world = FlatTransform::TransformVector(local, transform);
                minX = std::min(minX, world.x());
                minY = std::min(minY, world.y());
                maxX = std::max(maxX, world.x());
                maxY = std::max(maxY, world.y());
            }
            break;
        }
        default:
            break;
        }

        if (minX > maxX || minY > maxY) {
            Vector2 p = body_->GetPosition();
            return FlatAABB(p, p);
        }

        return FlatAABB(minX, minY, maxX, maxY);
    }

    float FlatFixture::ComputeArea() const
    {
        if (!shape_) return 0.0f;

        switch (shape_->GetType()) {
        case ShapeType::Circle: {
            const CircleShape* circle = shape_->AsCircle();
            if (!circle) return 0.0f;
            const float r = circle->radius;
            return static_cast<float>(M_PI) * r * r;
        }
        case ShapeType::Polygon: {
            const PolygonShape* poly = shape_->AsPolygon();
            if (!poly) return 0.0f;
            return std::abs(PolygonArea(poly->vertices));
        }
        default:
            return 0.0f;
        }
    }

    float FlatFixture::ComputeMass() const
    {
        return density_ * ComputeArea();
    }

    float FlatFixture::ComputeLocalInertia() const
    {
        if (!shape_) return 0.0f;

        const float mass = ComputeMass();
        if (mass <= 0.0f) return 0.0f;

        switch (shape_->GetType()) {
        case ShapeType::Circle: {
            const CircleShape* circle = shape_->AsCircle();
            if (!circle) return 0.0f;
            const float r = circle->radius;
            const Vector2& c = circle->center;
            const float i_center = 0.5f * mass * r * r;
            return i_center + mass * Vector2::Dot(c, c);
        }
        case ShapeType::Polygon: {
            const PolygonShape* poly = shape_->AsPolygon();
            if (!poly || poly->vertices.size() < 3) return 0.0f;

            const auto& verts = poly->vertices;
            double area_sum = 0.0;
            double inertia_sum = 0.0;

            for (size_t i = 0, j = verts.size() - 1; i < verts.size(); j = i++) {
                const Vector2& a = verts[j];
                const Vector2& b = verts[i];
                const double cross = static_cast<double>(a.x()) * b.y() - static_cast<double>(a.y()) * b.x();
                const double term = static_cast<double>(a.x()) * a.x() + static_cast<double>(a.x()) * b.x()
                    + static_cast<double>(b.x()) * b.x()
                    + static_cast<double>(a.y()) * a.y() + static_cast<double>(a.y()) * b.y()
                    + static_cast<double>(b.y()) * b.y();
                area_sum += cross;
                inertia_sum += cross * term;
            }

            if (std::fabs(area_sum) <= 1e-8) return 0.0f;

            const double inertia = (mass / (6.0 * area_sum)) * inertia_sum;
            return static_cast<float>(std::fabs(inertia));
        }

        default:
            return 0.0f;
        }
    }

    Vector2 FlatFixture::ComputeLocalCenter() const
    {
        if (!shape_) return Vector2::Zero();

        switch (shape_->GetType()) {
        case ShapeType::Circle: {
            const CircleShape* circle = shape_->AsCircle();
            return circle ? circle->center : Vector2::Zero();
        }

        case ShapeType::Polygon: {
            const PolygonShape* poly = shape_->AsPolygon();
            if (!poly || poly->vertices.size() < 3) {
                return Vector2::Zero();
            }

            const auto& verts = poly->vertices;
            double area2 = 0.0;
            Vector2 centroid = Vector2::Zero();

            for (size_t i = 0, j = verts.size() - 1; i < verts.size(); j = i++) {
                const Vector2& a = verts[j];
                const Vector2& b = verts[i];
                const double cross = static_cast<double>(a.x()) * b.y()
                    - static_cast<double>(a.y()) * b.x();
                area2 += cross;
                centroid += (a + b) * static_cast<float>(cross);
            }

            if (std::fabs(area2) <= 1e-8) {
                Vector2 mean = Vector2::Zero();
                for (const auto& v : verts) {
                    mean += v;
                }
                return mean / static_cast<float>(verts.size());
            }

            return centroid / static_cast<float>(3.0 * area2);
        }

        default:
            return Vector2::Zero();
        }
    }

}
