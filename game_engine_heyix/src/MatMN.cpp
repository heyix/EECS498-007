#include "MatMN.h"
#include <cmath>
#include <algorithm>

namespace FlatPhysics {

	MatMN::MatMN()
		: M(0), N(0)
	{
	}

	MatMN::MatMN(int m, int n)
		: M(m), N(n)
	{
		data.resize(M * N, 0.0f);
	}

	MatMN::MatMN(const MatMN& m)
		: M(m.M), N(m.N), data(m.data)
	{
	}

	MatMN& MatMN::operator=(const MatMN& m)
	{
		if (this == &m) return *this;
		M = m.M;
		N = m.N;
		data = m.data;
		return *this;
	}

	VecN MatMN::operator*(const VecN& v) const
	{
		VecN out(M);
		for (int i = 0; i < M; i++) {
			float sum = 0.0f;
			for (int j = 0; j < N; j++) {
				sum += (*this)[i][j] * v[j];
			}
			out[i] = sum;
		}
		return out;
	}

	MatMN MatMN::operator*(const MatMN& m) const
	{
		MatMN out(M, m.N);
		for (int i = 0; i < M; i++) {
			for (int j = 0; j < m.N; j++) {
				float s = 0.0f;
				for (int k = 0; k < N; k++) {
					s += (*this)[i][k] * m[k][j];
				}
				out[i][j] = s;
			}
		}
		return out;
	}

	void MatMN::Zero()
	{
		for (int i = 0; i < M * N; i++) {
			data[i] = 0.0f;
		}
	}

	MatMN MatMN::Transpose() const
	{
		MatMN t(N, M);
		for (int i = 0; i < M; i++) {
			for (int j = 0; j < N; j++) {
				t[j][i] = (*this)[i][j];
			}
		}
		return t;
	}

} 
