#include "Collision.h"
#include <iostream>
#include <limits>
#include <algorithm>
namespace FlatPhysics {
	bool FlatPhysics::Collision::IntersectCircles(Vector2 centerA, float radiusA, Vector2 centerB, float radiusB, Vector2* normal, float* depth)
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
		Vector2 centerA = FindPolygonArithmeticMean(verticesA);
		Vector2 centerB = FindPolygonArithmeticMean(verticesB);
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

	//return {min,max}
	std::pair<float, float> Collision::ProjectVertices(const std::vector<Vector2>& vertices, Vector2 axis)
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

	Vector2 Collision::FindPolygonArithmeticMean(const std::vector<Vector2>& vertices)
	{
		float sumX = 0;
		float sumY = 0;
		for (int i = 0; i < vertices.size(); i++) {
			sumX += vertices[i].x();
			sumY += vertices[i].y();
		}
		return { sumX / vertices.size(),sumY / vertices.size() };
	}

}
