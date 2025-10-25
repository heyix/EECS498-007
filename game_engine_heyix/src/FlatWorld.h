#pragma once
namespace FlatPhysics {
	class FlatWorld {
	public:
		const static inline float MinBodySize = 0.01f * 0.01f;
		const static inline float MaxBodySize = 64.0f * 64.0f;
		
		const static inline float MinDensity = 0.5f;
		const static inline float MaxDensity = 21.4f;
	};
}