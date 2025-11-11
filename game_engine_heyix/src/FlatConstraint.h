#pragma once
#include "MatMN.h"
namespace FlatPhysics {
	class FlatBody;
	class FlatConstraint {
	public:
		virtual ~FlatConstraint() = default;
	public:
		MatMN GetInverseM()const;
		VecN GetVelocities()const;
	public:
		virtual void Solve()const;
	public:
		FlatBody* a;
		FlatBody* b;
	};
}