#pragma once
#include "VecN.h"
namespace FlatPhysics{
	class MatMN {
	public:
		MatMN();
		MatMN(int M, int N);
		MatMN(const MatMN& m);
		const MatMN& operator=(const MatMN& m);
		VecN operator*(const VecN& v)const;
		MatMN operator*(const MatMN& m)const;
	public:
		void Zero();
		MatMN Transpose();

	public:
		int M;
		int N;
		std::vector<VecN> rows;

	};
}
