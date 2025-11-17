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
				const PolygonShape* polygon_shapeB = fb->GetShape().AsPolygon();
				const std::vector<Vector2>& vertsB_local = polygon_shapeB->GetVertices();
				std::vector<Vector2>& vertsB = polygon_shapeB->GetVerticesSizedBuffer();
				 FlatTransform::TransformVectors(vertsB_local, vertsB, transformB);
				return Collision::IntersectCirclePolygonOld(cA, rA, vertsB, normal, depth);
			}
			default: break;
			}
			break;
		}

		case ShapeType::Polygon: {
			const PolygonShape* polygon_shapeA = fa->GetShape().AsPolygon();
			const std::vector<Vector2>& vertsA_local = polygon_shapeA->GetVertices();
			std::vector<Vector2>& vertsA = polygon_shapeA->GetVerticesSizedBuffer();
			FlatTransform::TransformVectors(vertsA_local, vertsA, transformA);

			switch (fb->GetShapeType()) {
			case ShapeType::Polygon: {
				const PolygonShape* polygon_shapeB = fb->GetShape().AsPolygon();
				const std::vector<Vector2>& vertsB_local = polygon_shapeB->GetVertices();
				std::vector<Vector2>& vertsB = polygon_shapeB->GetVerticesSizedBuffer();
				FlatTransform::TransformVectors(vertsB_local, vertsB, transformB);
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
		Vector2 direction = (centerB - centerA).Normalized();
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
						result.SetPoints(result.point1, contact_point);
					}
				}
				else if (distance_squared < min_distance_squared) {
					min_distance_squared = distance_squared;
					result.SetPoint(contact_point);
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
						result.SetPoints(result.point1, contact_point);
					}
				}
				else if (distance_squared < min_distance_squared) {
					min_distance_squared = distance_squared;
					result.SetPoint(contact_point);
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
				std::vector<Vector2>& vertsB = polygonB->GetVerticesSizedBuffer();
				fb->GetBody()->LocalToWorld(polygonB->GetVertices(), vertsB);
				return FindCirclePolygonContactPointOld(centerA, circleA->radius, vertsB);
			}
			default: break;
			}
			break;
		}

		case ShapeType::Polygon: {
			const PolygonShape* polygonA = fa->GetShape().AsPolygon();
			std::vector<Vector2>& transformed_a = polygonA->GetVerticesSizedBuffer();
			fa->GetBody()->LocalToWorld(polygonA->GetVertices(), transformed_a);
			switch (fb->GetShapeType()) {
			case ShapeType::Polygon: {
				const PolygonShape* polygonB = fb->GetShape().AsPolygon();
				std::vector<Vector2>& vertsB = polygonB->GetVerticesSizedBuffer();
				fb->GetBody()->LocalToWorld(polygonB->GetVertices(), vertsB);
				return FindPolygonPolygonContactPointOld(transformed_a, vertsB);
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

	bool Collision::IsCollidingCircleCirle(const Vector2& centerA, float radiusA, const Vector2& centerB, float radiusB, FixedSizeContainer<ContactPoint, 2>& contact)
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
		ContactPoint contact_point;
		contact_point.normal = n;
		contact_point.start = centerB - n * radiusB;
		contact_point.end = centerA + n * radiusA;
		contact_point.depth = (contact_point.end - contact_point.start).Length();

		contact_point.id.contact_feature.indexA = 0;
		contact_point.id.contact_feature.indexB = 0;
		contact_point.id.contact_feature.typeA = ContactFeatureType::Feature_Vertex;
		contact_point.id.contact_feature.typeB = ContactFeatureType::Feature_Vertex;
		contact_point.normal_impulse = 0.0f;
		contact_point.tangent_impulse = 0.0f;


		contact.Push_Back(contact_point);
		return true;
	}

	bool Collision::IsCollidingPolygonCircle(const std::vector<Vector2>& vertices, const Vector2& center, float radius, FixedSizeContainer<ContactPoint, 2>& contact)
	{
		Vector2 min_cur_vertex;
		Vector2 min_next_vertex;
		int result_edge_index = -1;

		bool is_outside = false;
		float distance_circle_edge = std::numeric_limits<float>::lowest();

		for (int i = 0; i < vertices.size(); i++) {
			Vector2 va = vertices[i];
			Vector2 vb = vertices[(i + 1) % vertices.size()];
			Vector2 edge = vb - va;
			Vector2 normal = edge.NormalDirection().Normalized();
			float projection = Vector2::Dot(center - va, normal);
			if (projection > 0) {
				if (projection > distance_circle_edge) {
					min_cur_vertex = va;
					min_next_vertex = vb;
					distance_circle_edge = projection;
					result_edge_index = i;
				}
				is_outside = true;
			}
			else {
				if (projection > distance_circle_edge) {
					distance_circle_edge = projection;
					min_cur_vertex = va;
					min_next_vertex = vb;
					result_edge_index = i;
				}
			}
		}
		ContactPoint contact_point;
		if (is_outside) {
			Vector2 closest_point;
			float center_edge_dist_squared = PointSegmentDistanceSquared(center, min_cur_vertex, min_next_vertex, &closest_point);
			if (center_edge_dist_squared > radius * radius) {
				return false;
			}
			contact_point.depth = radius - std::sqrt(center_edge_dist_squared);
			contact_point.normal = (center - closest_point).Normalized();
			contact_point.start = center - contact_point.normal * radius;
			contact_point.end = contact_point.start + (contact_point.normal * contact_point.depth);
		}
		else {
			contact_point.depth = radius - distance_circle_edge;
			contact_point.normal = (min_next_vertex - min_cur_vertex).NormalDirection().Normalized();
			contact_point.start = center - contact_point.normal * radius;
			contact_point.end = contact_point.start + (contact_point.normal * contact_point.depth);
		}


		ContactFeature& feature = contact_point.id.contact_feature;
		feature.typeA = ContactFeatureType::Feature_Face;
		feature.typeB = ContactFeatureType::Feature_Vertex;
		feature.indexA = static_cast<std::uint8_t>(result_edge_index);
		feature.indexB = 0;
		contact_point.normal_impulse = 0.0f;
		contact_point.tangent_impulse = 0.0f;

		contact.Push_Back(contact_point);
		return true;
	}

	bool Collision::IsCollidingPolygonPolygon(const std::vector<Vector2>& verticesA, const std::vector<Vector2>& verticesB, FixedSizeContainer<ContactPoint, 2>& contact)
	{
		int a_reference_edge_index, b_reference_edge_index;
		Vector2 a_support_point, b_support_point;
		float ab_separation = FindMinSeperation(verticesA, verticesB, a_reference_edge_index, a_support_point);
		if (ab_separation >= 0) {
			return false;
		}
		float ba_separation = FindMinSeperation(verticesB, verticesA, b_reference_edge_index, b_support_point);
		if (ba_separation >= 0) {
			return false;
		}
		int reference_edge_index;
		const std::vector<Vector2>* reference_vertices;
		const std::vector<Vector2>* incident_vertices;

		bool reference_is_A = false;
		if (ab_separation > ba_separation) {
			reference_edge_index = a_reference_edge_index;
			reference_vertices = &verticesA;
			incident_vertices = &verticesB;
			reference_is_A = true;
		}
		else {
			reference_edge_index = b_reference_edge_index;
			reference_vertices = &verticesB;
			incident_vertices = &verticesA;
			reference_is_A = false;
		}
		ContactPoint contact_point;
		Vector2 reference_edge = EdgeAt(*reference_vertices, reference_edge_index);

		Vector2 reference_edge_normal = reference_edge.NormalDirection().Normalized();
		//clip
		int incident_edge_index = FindIncidentEdgeIndex(*incident_vertices, reference_edge_normal);
		int incident_next_index = (incident_edge_index + 1) % incident_vertices->size();

		Vector2 v0 = (*incident_vertices)[incident_edge_index];
		Vector2 v1 = (*incident_vertices)[incident_next_index];

		FixedSizeContainer<Vector2,2> contact_points = { v0,v1 };
		FixedSizeContainer<Vector2, 2> clipped_points = contact_points;
		for (int i = 0; i < reference_vertices->size(); i++) {
			if (i == reference_edge_index) {
				continue;
			}
			Vector2 c0 = (*reference_vertices)[i];
			Vector2 c1 = (*reference_vertices)[(i + 1) % reference_vertices->size()];
			int num_clipped = ClipSegmentToLine(*reference_vertices,contact_points, clipped_points, c0, c1);
			if (num_clipped < 2) {
				break;
			}
			contact_points = clipped_points;
		}

		int point_slot = 0;
		Vector2 reference_vertex = (*reference_vertices)[reference_edge_index];
		for (Vector2& v_clip : clipped_points) {
			float separation = Vector2::Dot((v_clip - reference_vertex), reference_edge_normal);
			if (separation <= 0) {
				ContactPoint contact_point;
				contact_point.normal = reference_edge_normal;
				contact_point.start = v_clip;
				contact_point.end = v_clip + contact_point.normal * -separation;
				if (ba_separation >= ab_separation) {
					contact_point.normal *= -1;
					std::swap(contact_point.start, contact_point.end);
				}
				contact_point.depth = -separation;

				ContactFeature& feature = contact_point.id.contact_feature;
				std::uint8_t ref_edge = static_cast<std::uint8_t>(reference_edge_index);
				std::uint8_t inc_edge = static_cast<std::uint8_t>(incident_edge_index);
				if (reference_is_A) {
					feature.typeA = ContactFeatureType::Feature_Face;
					feature.typeB = ContactFeatureType::Feature_Vertex;
					feature.indexA = ref_edge;
					feature.indexB = static_cast<std::uint8_t>(inc_edge * 2 + point_slot);

				}
				else {
					feature.typeA = ContactFeatureType::Feature_Vertex;
					feature.typeB = ContactFeatureType::Feature_Face;
					feature.indexA = static_cast<std::uint8_t>(inc_edge * 2 + point_slot);
					feature.indexB = ref_edge;
				}
				contact_point.normal_impulse = 0.0f;
				contact_point.tangent_impulse = 0.0f;

				contact.Push_Back(contact_point);
				++point_slot;
			}
		}
		return true;
	}
	//a->b collision, start is inside a, end is at the surface of a
	bool Collision::DetectCollision(const FlatFixture* fa, const FlatFixture* fb, FixedSizeContainer<ContactPoint, 2>& contact)
	{
		auto bodyA = fa->GetBody();
		auto bodyB = fb->GetBody();


		switch (fa->GetShapeType()) {
		case ShapeType::Circle: {
			auto* ca = fa->GetShape().AsCircle();
			Vector2 cA = bodyA->LocalToWorld(ca->center);
			float rA = ca->radius;

			switch (fb->GetShapeType()) {
			case ShapeType::Circle: {
				auto* cb = fb->GetShape().AsCircle();
				Vector2 cB = bodyB->LocalToWorld(cb->center);
				float rB = cb->radius;
				return IsCollidingCircleCirle(cA, rA, cB, rB, contact);
			}
			case ShapeType::Polygon: {
				const PolygonShape* polygonB = fb->GetShape().AsPolygon();
				const std::vector<Vector2>& vertsB_local = polygonB->GetVertices();
				std::vector<Vector2>& vertsB = polygonB->GetVerticesSizedBuffer();
				bodyB->LocalToWorld(vertsB_local, vertsB);
				bool hit = IsCollidingPolygonCircle(vertsB, cA, rA, contact);;
				if (hit)contact[0].normal *= -1;
				return hit;
			}
			default: break;
			}
			break;
		}

		case ShapeType::Polygon: {
			const PolygonShape* polygonA = fa->GetShape().AsPolygon();
			const std::vector<Vector2>& vertsA_local = polygonA->GetVertices();
			std::vector<Vector2>& vertsA = polygonA->GetVerticesSizedBuffer();
			bodyA->LocalToWorld(vertsA_local,vertsA);

			switch (fb->GetShapeType()) {
			case ShapeType::Polygon: {
				const PolygonShape* polygonB = fb->GetShape().AsPolygon();
				const std::vector<Vector2>& vertsB_local = polygonB->GetVertices();
				std::vector<Vector2>& vertsB = polygonB->GetVerticesSizedBuffer();
				bodyB->LocalToWorld(vertsB_local, vertsB);
				bool hit = IsCollidingPolygonPolygon(vertsA, vertsB, contact);
				return hit;
			}
			case ShapeType::Circle: {
				auto* cb = fb->GetShape().AsCircle();
				Vector2 cB = bodyB->LocalToWorld(cb->center);
				float rB = cb->radius;
				return IsCollidingPolygonCircle(vertsA, cB, rB, contact);
			}
			default: break;
			}
			break;
		}
		default: break;
		}

		return false;
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

	float Collision::FindMinSeperation(const std::vector<Vector2>& verticesA, const std::vector<Vector2>& verticesB,int& reference_edge_index, Vector2& support_point)
	{
		float seperation = std::numeric_limits<float>::lowest();
		for (int i = 0; i < verticesA.size(); i++) {
			Vector2 edge = EdgeAt(verticesA, i);
			Vector2 normal = edge.NormalDirection().Normalized();
			float min_sep = std::numeric_limits<float>::max();
			Vector2 min_vertex;
			for (const Vector2& vb : verticesB) {
				float proj = Vector2::Dot(vb - verticesA[i], normal);
				if (proj < min_sep) {
					min_sep = proj;
					min_vertex = vb;
				}
			}
			if (min_sep > seperation) {
				seperation = min_sep;
				reference_edge_index = i;
				support_point = min_vertex;
			}
		}
		return seperation;
	}

	int Collision::FindIncidentEdgeIndex(const std::vector<Vector2>& vertices, const Vector2& normal)
	{
		int result;
		float min_proj = std::numeric_limits<float>::max();
		for (int i = 0; i < vertices.size(); i++) {
			auto edge_normal = EdgeAt(vertices, i).NormalDirection().Normalized();
			auto proj = Vector2::Dot(edge_normal, normal);
			if (proj < min_proj) {
				min_proj = proj;
				result = i;
			}
		}
		return result;
	}

	Vector2 Collision::EdgeAt(const std::vector<Vector2>& vertices, int index)
	{
		return vertices[(index + 1) % vertices.size()] - vertices[index];
	}

	int Collision::ClipSegmentToLine(const std::vector<Vector2>& vertices, FixedSizeContainer<Vector2, 2>& contacts_in, FixedSizeContainer<Vector2, 2>& contacts_out, const Vector2& c0, const Vector2& c1)
	{
		int num_out = 0;
		Vector2 edge = (c1 - c0).Normalized();
		float dist0 = Vector2::Cross((contacts_in[0] - c0), edge);
		float dist1 = Vector2::Cross((contacts_in[1] - c0), edge);

		if (dist0 <= 0) {
			contacts_out[num_out++] = contacts_in[0];
		}
		if (dist1 <= 0) {
			contacts_out[num_out++] = contacts_in[1];
		}

		if (dist0 * dist1 < 0.0f) {
			float total_dist = dist0 - dist1;
			float t = dist0 / total_dist;
			Vector2 contact = contacts_in[0] + (contacts_in[1] - contacts_in[0]) * t;
			contacts_out[num_out] = contact;
			num_out++;
		}
		return num_out;
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
