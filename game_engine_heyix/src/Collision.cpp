#include "Collision.h"
#include <iostream>
#include <limits>
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

	bool Collision::IntersectPolygons(const std::vector<Vector2>& verticesA, const std::vector<Vector2>& verticesB)
	{
		for (int i = 0; i < verticesA.size(); i++) {
			const Vector2& va = verticesA[i];
			const Vector2& vb = verticesA[(i + 1)%verticesA.size()];

			Vector2 edge = vb - va;
			Vector2 axis = Vector2(-edge.y(), edge.x());

			std::pair<float, float> projA = ProjectVertices(verticesA, axis);
			std::pair<float, float> projB = ProjectVertices(verticesB, axis);
			if (projA.first >= projB.second || projB.first >= projA.second) {
				return false;
			}
		}
		for (int i = 0; i < verticesB.size(); i++) {
			const Vector2& va = verticesB[i];
			const Vector2& vb = verticesB[(i + 1) % verticesB.size()];

			Vector2 edge = vb - va;
			Vector2 axis = Vector2(-edge.y(), edge.x());

			std::pair<float, float> projA = ProjectVertices(verticesA, axis);
			std::pair<float, float> projB = ProjectVertices(verticesB, axis);
			if (projA.first >= projB.second || projB.first >= projA.second) {
				return false;
			}
		}
		return true;
	}

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

}
