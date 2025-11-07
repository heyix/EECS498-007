#include "FlatFixture.h"
#include "FlatTransform.h"
#include "FlatBody.h"
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

}
