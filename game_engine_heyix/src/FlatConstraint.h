#pragma once
#include "MatMN.h"
namespace FlatPhysics {
	class FlatBody;
	class FlatConstraint {
	public:
		void Solve()const;
		MatMN GetInverseM()const;
	public:
		FlatBody* a;
		FlatBody* b;
	};
}