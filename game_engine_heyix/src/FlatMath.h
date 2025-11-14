#pragma once
constexpr float kPi = 3.14159265358979323846f;

#include "Vector2.h"
namespace FlatPhysics {
	class FlatMath {
	public:
		template<typename T>
		constexpr const static T& Clamp(const T& value, const T& min, const T& max)
		{
			return (value < min) ? min : (value > max ? max : value);
		}
		constexpr static float RadToDeg(float radians)
		{
			return radians * (180.0 / kPi);
		}

		constexpr static float DegToRad(float degrees)
		{
			return degrees * (kPi / 180.0);
		}
		static Vector2 FindPolygonCentroid(const std::vector<Vector2>& vertices);
		static bool NearlyEqual(float a, float b);
		static bool NearlyEqual(const Vector2& a, const Vector2& b);
	};
}