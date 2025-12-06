#include "FlatMath.h"
#include <algorithm>
static float VerySmallAmount = 0.00005f;
Vector2 FlatPhysics::FlatMath::FindPolygonCentroid(const std::vector<Vector2>& vertices)
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

bool FlatPhysics::FlatMath::NearlyEqual(float a, float b)
{
	return std::abs(a - b) < VerySmallAmount;
}

bool FlatPhysics::FlatMath::NearlyEqual(const Vector2& a, const Vector2& b)
{
	return Vector2::DistanceSquared(a, b) < VerySmallAmount * VerySmallAmount;
}
