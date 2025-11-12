#pragma once
#include "Vector2.h"
#include "FlatShape.h"
namespace FlatPhysics {
	enum class BroadPhaseType {Naive, QuadTree};
	enum class SolverType { Naive, PGS, Jacobi };
	using ProxyID = int32_t;
	constexpr ProxyID kNullProxy = -1;
	struct StepConfig {
		float dt{ 1 / 50 };
		int velocity_iteration = 8;
		int position_iteration = 3;
	};

	struct WorldConfig {
		Vector2 gravity{ 0,9.81f };
		BroadPhaseType broad_phase_type{ BroadPhaseType::Naive };
		SolverType solver_type{ SolverType::Naive };
		int max_thread = 1;
		bool allow_sleep = true;
	};
	struct Filter {
		uint16_t category_bits = 0x0001;
		uint16_t mask_bits = 0xFFFF;
		int16_t  group_index = 0;
	};
	struct FixtureDef {
		const Shape* shape = nullptr;
		float density = 1.0f;
		float friction = 0.2f;
		float restitution = 0.3f;
		bool is_trigger = false;
		Filter filter{};
		void* user_data = nullptr;
	};
	class FlatBody;

}