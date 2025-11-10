#define _USE_MATH_DEFINES
#include <cmath>
#include "Collision.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include "FlatTransform.h"
#include "FlatMath.h"
#include "FlatBody.h"
namespace FlatPhysics {
	bool FlatPhysics::Collision::IntersectCirclesOld(const Vector2& centerA, float radiusA, const Vector2& centerB, float radiusB, Vector2* normal, float* depth)
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

	bool Collision::IntersectPolygonsOld(const std::vector<Vector2>& verticesA, const std::vector<Vector2>& verticesB, Vector2* normal, float* depth)
	{
		Vector2 centerA = FlatMath::FindPolygonCentroid(verticesA);
		Vector2 centerB = FlatMath::FindPolygonCentroid(verticesB);
		return IntersectPolygonsOld(verticesA, centerA, verticesB, centerB, normal, depth);
	}

	bool Collision::IntersectPolygonsOld(const std::vector<Vector2>& verticesA, const Vector2& centerA, const std::vector<Vector2>& verticesB, const Vector2& centerB, Vector2* normal, float* depth)
	{
		float min_depth = std::numeric_limits<float>::max();
		Vector2 result_normal = Vector2::Zero();
		for (int i = 0; i < verticesA.size(); i++) {
			const Vector2& va = verticesA[i];
			const Vector2& vb = verticesA[(i + 1) % verticesA.size()];

			Vector2 edge = vb - va;
			Vector2 axis = edge.NormalDirection();

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
			Vector2 axis = edge.NormalDirection();

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

	bool Collision::IntersectCirclePolygonOld(const Vector2& center, float radius, const std::vector<Vector2>& vertices, Vector2* normal, float* depth)
	{
		Vector2 polygon_center = FlatMath::FindPolygonCentroid(vertices);
		return IntersectCirclePolygonOld(center, radius, vertices, polygon_center, normal, depth);
	}

	bool Collision::IntersectCirclePolygonOld(const Vector2& center, float radius, const std::vector<Vector2>& vertices, const Vector2& polygon_center, Vector2* normal, float* depth)
	{
		float min_depth = std::numeric_limits<float>::max();
		Vector2 result_normal = Vector2::Zero();
		for (int i = 0; i < vertices.size(); i++) {
			const Vector2& va = vertices[i];
			const Vector2& vb = vertices[(i + 1) % vertices.size()];

			Vector2 edge = vb - va;
			Vector2 axis = edge.NormalDirection();

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
		int closest_index = FindClosestVertexBetweenCircleAndPolygon(center, vertices);
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

	bool Collision::DetectCollisionOld(const FlatFixture* fa, const FlatFixture* fb, Vector2* normal, float* depth)
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
				return Collision::IntersectCirclesOld(cA, rA, cB, rB, normal, depth);
			}
			case ShapeType::Polygon: {
				const auto& vertsB_local = fb->GetShape().AsPolygon()->vertices;
				auto vertsB = FlatTransform::TransformVectors(vertsB_local, transformB);
				return Collision::IntersectCirclePolygonOld(cA, rA, vertsB, normal, depth);
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
				return Collision::IntersectPolygonsOld(vertsA, vertsB, normal, depth);
			}
			case ShapeType::Circle: {
				auto* cb = fb->GetShape().AsCircle();
				Vector2 cB = FlatTransform::TransformVector(cb->center, transformB);
				float rB = cb->radius;
				bool hit = Collision::IntersectCirclePolygonOld(cB, rB, vertsA, normal, depth);
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
	ContactPointsOld Collision::FindCircleCircleContactPointOld(const Vector2& centerA, float radiusA, const Vector2& centerB)
	{
		Vector2 direction = centerB - centerA;
		direction.Normalize();
		Vector2 result = centerA + direction * radiusA;
		return { result };
	}
	//circle-polygon
	ContactPointsOld Collision::FindCirclePolygonContactPointOld(const Vector2& circle_center, float circle_radius, const std::vector<Vector2>& vertices)
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
	ContactPointsOld Collision::FindPolygonPolygonContactPointOld(const std::vector<Vector2>& vertices_a, const std::vector<Vector2>& vertices_b)
	{
		float min_distance_squared = std::numeric_limits<float>::max();
		ContactPointsOld result;
		for (int i = 0; i < vertices_a.size(); i++) {
			const Vector2& p = vertices_a[i];
			for (int j = 0; j < vertices_b.size(); j++) {
				Vector2 va = vertices_b[j];
				Vector2 vb = vertices_b[(j + 1) % vertices_b.size()];
				Vector2 contact_point;
				float distance_squared = PointSegmentDistanceSquared(p, va, vb, &contact_point);
				if (FlatMath::NearlyEqual(distance_squared,min_distance_squared)) {
					if (!FlatMath::NearlyEqual(contact_point,result.point1)) {
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
				if (FlatMath::NearlyEqual(distance_squared, min_distance_squared)) {
					if (!FlatMath::NearlyEqual(contact_point, result.point1)) {
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

	ContactPointsOld Collision::FindContactPointsOld(const FlatFixture* fa, const FlatFixture* fb)
	{
		switch (fa->GetShapeType()) {
		case ShapeType::Circle: {
			const CircleShape* circleA = fa->GetShape().AsCircle();
			Vector2 centerA = FlatTransform::TransformVector(circleA->center, fa->GetBody()->GetTransform());
			switch (fb->GetShapeType()) {
			case ShapeType::Circle: {
				const CircleShape* circleB = fb->GetShape().AsCircle();
				Vector2 centerB = FlatTransform::TransformVector(circleB->center, fb->GetBody()->GetTransform());
				return FindCircleCircleContactPointOld(centerA, circleA->radius, centerB);
			}
			case ShapeType::Polygon: {
				const PolygonShape* polygonB = fb->GetShape().AsPolygon();
				return FindCirclePolygonContactPointOld(centerA, circleA->radius, FlatTransform::TransformVectors(polygonB->vertices, fb->GetBody()->GetTransform()));
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
				return FindPolygonPolygonContactPointOld(transformed_a, FlatTransform::TransformVectors(polygonB->vertices, fb->GetBody()->GetTransform()));
			}
			case ShapeType::Circle: {
				const CircleShape* circleB = fb->GetShape().AsCircle();
				Vector2 centerB = FlatTransform::TransformVector(circleB->center, fb->GetBody()->GetTransform());
				return FindCirclePolygonContactPointOld(centerB, circleB->radius, transformed_a);
			}
			default: break;
			}
			break;
		}
		default: break;
		}
		return {};
	}

	bool Collision::IsCollidingCircleCirle(const Vector2& centerA, float radiusA, const Vector2& centerB, float radiusB, ContactPoint& contact)
	{
		Vector2 d = centerB - centerA;
		float rSum = radiusA + radiusB;
		float dist2 = d.LengthSquared();

		if (dist2 > rSum * rSum) return false;

		float dist = 0.0f;
		Vector2 n;
		if (dist2 > 1e-12f) {
			dist = std::sqrt(dist2);
			n = d * (1.0f / dist);
		}
		else {
			n = Vector2(1, 0);     
			dist = 0.0f;
		}

		contact.normal = n;
		contact.contact_point = centerA + n * radiusA;      
		contact.depth = std::max(0.0f, rSum - dist);  
		return true;
	}

	bool Collision::IsCollidingCirclePolygon(const Vector2& center, float radius, const std::vector<Vector2>& vertices, ContactPoint& contact)
	{
		Vector2 min_cur_vertex;
		Vector2 min_next_vertex;

		bool is_outside = false;
		float distance_circle_edge = std::numeric_limits<float>::lowest();

		for (int i = 0; i < vertices.size(); i++) {
			Vector2 va = vertices[i];
			Vector2 vb = vertices[(i + 1) % vertices.size()];
			Vector2 edge = vb - va;
			Vector2 normal = edge.NormalDirection();
			normal.Normalize();
			Vector2 vertex_to_circle_center = center - va;
			float projection = Vector2::Dot(center - va, normal);
			if (projection > 0) {
				if (projection > distance_circle_edge) {
					min_cur_vertex = va;
					min_next_vertex = vb;
					distance_circle_edge = projection;
				}
				is_outside = true;
			}
			else {
				if (projection > distance_circle_edge) {
					distance_circle_edge = projection;
					min_cur_vertex = va;
					min_next_vertex = vb;
				}
			}
		}

		if (is_outside) {
			Vector2 closest_point;
			float center_edge_dist_squared = PointSegmentDistanceSquared(center, min_cur_vertex, min_next_vertex, &closest_point);
			if (center_edge_dist_squared > radius * radius) {
				return false;
			}
			contact.depth = radius - std::sqrt(center_edge_dist_squared);
			contact.normal = closest_point - center;
			contact.normal.Normalize();
			contact.contact_point = center + contact.normal * radius;
		}
		else {
			contact.depth = radius - distance_circle_edge;
			contact.normal = (min_next_vertex - min_cur_vertex).NormalDirection();
			contact.normal.Normalize();
			contact.contact_point = center + contact.normal * radius;
		}

		return true;
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
	int Collision::FindClosestVertexBetweenCircleAndPolygon(const Vector2& center, const std::vector<Vector2>& vertices)
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


}
