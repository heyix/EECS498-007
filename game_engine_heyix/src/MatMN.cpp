#include "MatMN.h"
#include <algorithm>

namespace FlatPhysics {

    MatMN::MatMN(int m, int n) : M(m), N(n) {
        data.resize((size_t)M * N, 0.0f);
    }

    MatMN::MatMN(const MatMN& m) : M(m.M), N(m.N), data(m.data) {}
    MatMN::MatMN(int m, int n, float init_value)
        : M(m), N(n)
    {
        data.assign(static_cast<size_t>(M) * static_cast<size_t>(N), init_value);
    }
    MatMN& MatMN::operator=(const MatMN& m) {
        if (this == &m) return *this;
        M = m.M; N = m.N; data = m.data;
        return *this;
    }

    VecN MatMN::operator*(const VecN& v) const {
        VecN out(M);
        for (int i = 0; i < M; ++i) {
            float sum = 0.0f;
            for (int j = 0; j < N; ++j) {
                sum += At(i, j) * v[j];
            }
            out[i] = sum;
        }
        return out;
    }

    MatMN MatMN::operator*(const MatMN& m) const {
        MatMN out(M, m.N);
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < m.N; ++j) {
                float s = 0.0f;
                for (int k = 0; k < N; ++k) {
                    s += At(i, k) * m.At(k, j);
                }
                out.At(i, j) = s;
            }
        }
        return out;
    }

    void MatMN::Zero() {
        std::fill(data.begin(), data.end(), 0.0f);
    }

    MatMN MatMN::Transpose() const {
        MatMN t(N, M);
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                t.At(j, i) = At(i, j);
            }
        }
        return t;
    }

}
