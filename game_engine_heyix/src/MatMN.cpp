#include "MatMN.h"

namespace FlatPhysics {

	MatMN::MatMN()
		: M(0), N(0)
	{
	}

	MatMN::MatMN(int m, int n)
		: M(m), N(n)
	{
		rows.reserve(M);
		for (int i = 0; i < M; i++) {
			rows.push_back(VecN(N));
		}
	}

	MatMN::MatMN(const MatMN& m)
		: M(m.M), N(m.N), rows(m.rows)
	{
	}

	const MatMN& MatMN::operator=(const MatMN& m)
	{
		if (this == &m) return *this;
		M = m.M;
		N = m.N;
		rows = m.rows;
		return *this;
	}

	VecN MatMN::operator*(const VecN& v) const
	{
		VecN out(M);
		for (int i = 0; i < M; i++) {
			out[i] = rows[i].Dot(v);
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
					s += rows[i][k] * m.rows[k][j];
				}
				out.rows[i][j] = s;
			}
		}
		return out;
	}

	void MatMN::Zero()
	{
		for (int i = 0; i < M; i++) {
			rows[i].Zero();
		}
	}

	MatMN MatMN::Transpose()
	{
		MatMN t(N, M);
		for (int i = 0; i < M; i++) {
			for (int j = 0; j < N; j++) {
				t.rows[j][i] = rows[i][j];
			}
		}
		return t;
	}

} 