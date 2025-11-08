#include "Collision.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include "FlatTransform.h"
#include "FlatBody.h"
namespace FlatPhysics {
	bool FlatPhysics::Collision::IntersectCircles(const Vector2& centerA, float radiusA, const Vector2& centerB, float radiusB, Vector2* normal, float* depth)
	{
		float distance = Vector2::Distance(centerA, centerB);
		float radii = radiusA + radiusB;
		if (distance >= radii) {
			return false;
		}
		if (normal) {
			*normal = centerB - centerA;
			normal->Normalize();
		}
		if (depth) {
			*depth = radii - distance;
		}
		return true;
	}

	bool Collision::IntersectPolygons(const std::vector<Vector2>& verticesA, const std::vector<Vector2>& verticesB, Vector2* normal, float* depth)
	{
		Vector2 centerA = FindPolygonCentroid(verticesA);
		Vector2 centerB = FindPolygonCentroid(verticesB);
		return IntersectPolygons(verticesA, centerA, verticesB, centerB, normal, depth);
	}

	bool Collision::IntersectPolygons(const std::vector<Vector2>& verticesA, const Vector2& centerA, const std::vector<Vector2>& verticesB, const Vector2& centerB, Vector2* normal, float* depth)
	{
		float min_depth = std::numeric_limits<float>::max();
		Vector2 result_normal = Vector2::Zero();
		for (int i = 0; i < verticesA.size(); i++) {
			const Vector2& va = verticesA[i];
			const Vector2& vb = verticesA[(i + 1) % verticesA.size()];

			Vector2 edge = vb - va;
			Vector2 axis = Vector2(-edge.y(), edge.x());

			float axis_len = axis.Normalize();
			if (axis_len <= 1e-8f) continue;

			std::pair<float, float> projA = ProjectVertices(verticesA, axis);
			std::pair<float, float> projB = ProjectVertices(verticesB, axis);

			if (projA.first >= projB.second || projB.first >= projA.second) {
				return false;
			}
			float axis_depth = std::min(projB.second - projA.first, projA.second - projB.first);
			if (axis_depth < min_depth) {
				min_depth = axis_depth;
				result_normal = axis;
			}
		}

		for (int i = 0; i < verticesB.size(); i++) {
			const Vector2& va = verticesB[i];
			const Vector2& vb = verticesB[(i + 1) % verticesB.size()];

			Vector2 edge = vb - va;
			Vector2 axis = Vector2(-edge.y(), edge.x());

			float axis_len = axis.Normalize();
			if (axis_len <= 1e-8f) continue;

			std::pair<float, float> projA = ProjectVertices(verticesA, axis);
			std::pair<float, float> projB = ProjectVertices(verticesB, axis);

			if (projA.first >= projB.second || projB.first >= projA.second) {
				return false;
			}
			float axis_depth = std::min(projB.second - projA.first, projA.second - projB.first);
			if (axis_depth < min_depth) {
				min_depth = axis_depth;
				result_normal = axis;
			}
		}
		Vector2 direction = centerB - centerA;
		if (Vector2::Dot(direction, result_normal) < 0) {
			result_normal = -result_normal;
		}
		if (normal) {
			*normal = result_normal;
		}
		if (depth) {
			*depth = min_depth;
		}
		return true;
	}

	bool Collision::IntersectCirclePolygon(const Vector2& center, float radius, const std::vector<Vector2>& vertices, Vector2* normal, float* depth)
	{
		Vector2 polygon_center = FindPolygonCentroid(vertices);
		return IntersectCirclePolygon(center, radius, vertices, polygon_center, normal, depth);
	}

	bool Collision::IntersectCirclePolygon(const Vector2& center, float radius, const std::vector<Vector2>& vertices, const Vector2& polygon_center, Vector2* normal, float* depth)
	{
		float min_depth = std::numeric_limits<float>::max();
		Vector2 result_normal = Vector2::Zero();
		for (int i = 0; i < vertices.size(); i++) {
			const Vector2& va = vertices[i];
			const Vector2& vb = vertices[(i + 1) % vertices.size()];

			Vector2 edge = vb - va;
			Vector2 axis = Vector2(-edge.y(), edge.x());

			float axis_len = axis.Normalize();
			if (axis_len <= 1e-8f) continue;

			std::pair<float, float> projA = ProjectVertices(vertices, axis);
			std::pair<float, float> projB = ProjectCircle(center, radius, axis);

			if (projA.first >= projB.second || projB.first >= projA.second) {
				return false;
			}
			float axis_depth = std::min(projB.second - projA.first, projA.second - projB.first);
			if (axis_depth < min_depth) {
				min_depth = axis_depth;
				result_normal = axis;
			}
		}
		int closest_index = FindClosestPointFromCircleToPolygon(center, vertices);
		Vector2 closest = vertices[closest_index];
		Vector2 axis = closest - center;
		float axis_len = axis.Normalize();
		if (axis_len > 1e-8f) {
			auto projA = ProjectVertices(vertices, axis);
			auto projB = ProjectCircle(center, radius, axis);
			if (projA.first >= projB.second || projB.first >= projA.second) return false;

			float axis_depth = std::min(projB.second - projA.first, projA.second - projB.first);
			if (axis_depth < min_depth) {
				min_depth = axis_depth;
				result_normal = axis;
			}
		}

		if (min_depth == std::numeric_limits<float>::max()) return false;

		Vector2 direction = polygon_center - center;
		if (Vector2::Dot(direction, result_normal) < 0) result_normal = -result_normal;

		if (normal) *normal = result_normal;
		if (depth)  *depth = min_depth;
		return true;
	}

	bool Collision::DetectCollision(const FlatFixture* fa, const FlatFixture* fb, Vector2* normal, float* depth)
	{
		auto bodyA = fa->GetBody();
		auto bodyB = fb->GetBody();

		const auto& transformA = bodyA->GetTransform();
		const auto& transformB = bodyB->GetTransform();

		switch (fa->GetShapeType()) {
		case ShapeType::Circle: {
			auto* ca = fa->GetShape().AsCircle();
			Vector2 cA = FlatTransform::TransformVector(ca->center, transformA);
			float rA = ca->radius;

			switch (fb->GetShapeType()) {
			case ShapeType::Circle: {
				auto* cb = fb->GetShape().AsCircle();
				Vector2 cB = FlatTransform::TransformVector(cb->center, transformB);
				float rB = cb->radius;
				return Collision::IntersectCircles(cA, rA, cB, rB, normal, depth);
			}
			case ShapeType::Polygon: {
				const auto& vertsB_local = fb->GetShape().AsPolygon()->vertices;
				auto vertsB = FlatTransform::TransformVectors(vertsB_local, transformB);
				return Collision::IntersectCirclePolygon(cA, rA, vertsB, normal, depth);
			}
			default: break;
			}
			break;
		}

		case ShapeType::Polygon: {
			const auto& vertsA_local = fa->GetShape().AsPolygon()->vertices;
			auto vertsA = FlatTransform::TransformVectors(vertsA_local, transformA);

			switch (fb->GetShapeType()) {
			case ShapeType::Polygon: {
				const auto& vertsB_local = fb->GetShape().AsPolygon()->vertices;
				auto vertsB = FlatTransform::TransformVectors(vertsB_local, transformB);
				return Collision::IntersectPolygons(vertsA, vertsB, normal, depth);
			}
			case ShapeType::Circle: {
				auto* cb = fb->GetShape().AsCircle();
				Vector2 cB = FlatTransform::TransformVector(cb->center, transformB);
				float rB = cb->radius;
				bool hit = Collision::IntersectCirclePolygon(cB, rB, vertsA, normal, depth);
				if (hit && normal) *normal = -*normal;
				return hit;
			}
			default: break;
			}
			break;
		}
		default: break;
		}

		return false;
	}
	//circle-circle
	ContactPoints Collision::FindContactPoint(const Vector2& centerA, float radiusA, const Vector2& centerB)
	{
		Vector2 direction = centerB - centerA;
		direction.Normalize();
		Vector2 result = centerA + direction * radiusA;
		return { result };
	}
	//circle-polygon
	ContactPoints Collision::FindContactPoint(const Vector2& circle_center, float circle_radius, const std::vector<Vector2>& vertices)
	{
		float min_distance = std::numeric_limits<float>::max();
		Vector2 result;
		for (int i = 0; i < vertices.size(); i++) {
			const Vector2& va = vertices[i];
			const Vector2& vb = vertices[(i + 1) % vertices.size()];
			Vector2 contact_point;
			float distance_squared = PointSegmentDistanceSquared(circle_center, va, vb, &contact_point);
			if (distance_squared < min_distance) {
				min_distance = distance_squared;
				result = contact_point;
			}
		}
		return { result };
	}
	//polygon-polygon
	ContactPoints Collision::FindContactPoint(const std::vector<Vector2>& vertices_a, const std::vector<Vector2>& vertices_b)
	{
		float min_distance_squared = std::numeric_limits<float>::max();
		ContactPoints result;
		for (int i = 0; i < vertices_a.size(); i++) {
			const Vector2& p = vertices_a[i];
			for (int j = 0; j < vertices_b.size(); j++) {
				Vector2 va = vertices_b[j];
				Vector2 vb = vertices_b[(j + 1) % vertices_b.size()];
				Vector2 contact_point;
				float distance_squared = PointSegmentDistanceSquared(p, va, vb, &contact_point);
				if (distance_squared == min_distance_squared) {
					if (contact_point != result.point1) {
						result.point2 = contact_point;
						result.points_num = 2;
					}
				}
				else if (distance_squared < min_distance_squared) {
					min_distance_squared = distance_squared;
					result.point1 = contact_point;
					result.points_num = 1;
				}
			}
		}
		for (int i = 0; i < vertices_b.size(); i++) {
			const Vector2& p = vertices_b[i];
			for (int j = 0; j < vertices_a.size(); j++) {
				Vector2 va = vertices_a[j];
				Vector2 vb = vertices_a[(j + 1) % vertices_a.size()];
				Vector2 contact_point;
				float distance_squared = PointSegmentDistanceSquared(p, va, vb, &contact_point);
				if (distance_squared == min_distance_squared) {
					if (contact_point != result.point1) {
						result.point2 = contact_point;
						result.points_num = 2;
					}
				}
				else if (distance_squared < min_distance_squared) {
					min_distance_squared = distance_squared;
					result.point1 = contact_point;
					result.points_num = 1;
				}
			}
		}
		return result;
	}

	ContactPoints Collision::FindContactPoints(const FlatFixture* fa, const FlatFixture* fb)
	{
		switch (fa->GetShapeType()) {
		case ShapeType::Circle: {
			const CircleShape* circleA = fa->GetShape().AsCircle();
			Vector2 centerA = FlatTransform::TransformVector(circleA->center, fa->GetBody()->GetTransform());
			switch (fb->GetShapeType()) {
			case ShapeType::Circle: {
				const CircleShape* circleB = fb->GetShape().AsCircle();
				Vector2 centerB = FlatTransform::TransformVector(circleB->center, fb->GetBody()->GetTransform());
				return FindContactPoint(centerA, circleA->radius, centerB);
			}
			case ShapeType::Polygon: {
				const PolygonShape* polygonB = fb->GetShape().AsPolygon();
				return FindContactPoint(centerA, circleA->radius, FlatTransform::TransformVectors(polygonB->vertices, fb->GetBody()->GetTransform()));
			}
			default: break;
			}
			break;
		}

		case ShapeType::Polygon: {
			const PolygonShape* polygonA = fa->GetShape().AsPolygon();
			std::vector<Vector2> transformed_a = FlatTransform::TransformVectors(polygonA->vertices, fa->GetBody()->GetTransform());
			switch (fb->GetShapeType()) {
			case ShapeType::Polygon: {
				const PolygonShape* polygonB = fb->GetShape().AsPolygon();
				return FindContactPoint(transformed_a, FlatTransform::TransformVectors(polygonB->vertices, fb->GetBody()->GetTransform()));
			}
			case ShapeType::Circle: {
				const CircleShape* circleB = fb->GetShape().AsCircle();
				Vector2 centerB = FlatTransform::TransformVector(circleB->center, fb->GetBody()->GetTransform());
				return FindContactPoint(centerB, circleB->radius, transformed_a);
			}
			default: break;
			}
			break;
		}
		default: break;
		}
		return {};
	}

	std::pair<float, float> Collision::ProjectCircle(const Vector2& center, float radius, const Vector2& axis)
	{
		//Vector2& direction = axis;
		//Vector2 direction_and_radius = direction * radius;

		//Vector2 p1 = center + direction_and_radius;
		//Vector2 p2 = center - direction_and_radius;

		//float min = Vector2::Dot(p1, axis);
		//float max = Vector2::Dot(p2, axis);
		//if (min > max) {
		//	std::swap(min, max);
		//}
		//return { min,max };
		Vector2 normalized_axis = axis.NormalizedVector();
		float c = Vector2::Dot(center, normalized_axis);
		return { c - radius, c + radius };
	}
	int Collision::FindClosestPointFromCircleToPolygon(const Vector2& center, const std::vector<Vector2>& vertices)
	{
		int result = -1;
		float min_distance = std::numeric_limits<float>::max();
		for (int i = 0; i < vertices.size(); i++) {
			float distance = Vector2::Distance(vertices[i], center);
			if (distance < min_distance) {
				min_distance = distance;
				result = i;
			}
		}
		return result;
	}

	float Collision::PointSegmentDistanceSquared(const Vector2& point, const Vector2& line_a, const Vector2& line_b, Vector2* contact)
	{
		Vector2 ab = line_b - line_a;
		Vector2 ap = point - line_a;

		float proj = Vector2::Dot(ap, ab);
		float ab_length_square = ab.LengthSquared();
		if (ab_length_square <= std::numeric_limits<float>::epsilon()) {
			*contact = line_a;
			return Vector2::DistanceSquared(point, line_a);
		}
		float d = proj / ab_length_square;

		if (d < 0) {
			*contact = line_a;
		}
		else if (d > 1) {
			*contact = line_b;
		}
		else {
			*contact = line_a + ab * d;
		}
		return Vector2::DistanceSquared(point, *contact);
	}

	//return {min,max}
	std::pair<float, float> Collision::ProjectVertices(const std::vector<Vector2>& vertices, const Vector2& axis)
	{
#pragma warning(push)
#pragma warning(disable : 26498)  // disable constexpr recommendation
		float min = std::numeric_limits<float>::max();
		float max = std::numeric_limits<float>::lowest();
		for (int i = 0; i < vertices.size(); i++) {
			const Vector2& v = vertices[i];
			float proj = Vector2::Dot(v, axis);
			if (proj < min) {
				min = proj;
			}
			if (proj > max) {
				max = proj;
			}
		}
		return { min,max };

#pragma warning(pop)
	}



	Vector2 Collision::FindPolygonCentroid(const std::vector<Vector2>& vertices)
	{
		const size_t n = vertices.size();
		if (n < 3) {
			return n > 0 ? vertices[0] : Vector2::Zero();
		}

		float area2 = 0.0f;         
		Vector2 centroid = Vector2::Zero();

		for (size_t i = 0, j = n - 1; i < n; j = i++) {
			float cross = vertices[j].x() * vertices[i].y() - vertices[i].x() * vertices[j].y();
			area2 += cross;
			centroid += (vertices[j] + vertices[i]) * cross;
		}

		return centroid / (3.0f * area2);
	}


}
