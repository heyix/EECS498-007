#pragma once
namespace FlatPhysics {
	class FlatMath {
	public:
		template<typename T>
		constexpr const static T& Clamp(const T& value, const T& min, const T& max)
		{
			return (value < min) ? min : (value > max ? max : value);
		}
		template<typename T>
		constexpr static T RadToDeg(T radians)
		{
			return radians * static_cast<T>(180.0 / M_PI);
		}

		// Degrees → Radians
		template<typename T>
		constexpr static T DegToRad(T degrees)
		{
			return degrees * static_cast<T>(M_PI / 180.0);
		}
	};
}