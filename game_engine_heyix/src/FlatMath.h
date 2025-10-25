#pragma once
namespace FlatPhysics {
	class FlatMath {
	public:
		template<typename T>
		constexpr const static T& Clamp(const T& value, const T& min, const T& max)
		{
			return (value < min) ? min : (value > max ? max : value);
		}
	};
}