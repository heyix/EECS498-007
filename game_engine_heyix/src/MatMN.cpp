#include "MatMN.h"
#include <algorithm>

namespace FlatPhysics {
    MatMN::MatMN()
        :M(0),N(0)
    {
    }
    MatMN::MatMN(int m, int n) : M(m), N(n) {
        data.resize((size_t)M * N, 0.0f);
    }

    MatMN::MatMN(const MatMN& m) : M(m.M), N(m.N), data(m.data) {}

    MatMN::MatMN(int m, int n, float init_value)
        : M(m), N(n)
    {
        data.assign((size_t)M * (size_t)N, init_value);
    }

    MatMN::MatMN(MatMN&& other) noexcept
        : M(other.M), N(other.N), data(std::move(other.data))
    {
        other.M = 0;
        other.N = 0;
    }

    MatMN& MatMN::operator=(const MatMN& m) {
        if (this == &m) return *this;
        M = m.M;
        N = m.N;
        data = m.data;
        return *this;
    }

    VecN MatMN::operator*(const VecN& v) const {
        VecN out(M);
        for (int i = 0; i < M; ++i) {
            float sum = 0.0f;
            for (int j = 0; j < N; ++j) {
                sum += (*this)(i, j) * v(j);
            }
            out(i) = sum;
        }
        return out;
    }

    MatMN MatMN::operator*(const MatMN& m) const {
        MatMN out(M, m.N);
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < m.N; ++j) {
                float s = 0.0f;
                for (int k = 0; k < N; ++k) {
                    s += (*this)(i, k) * m(k, j);
                }
                out(i, j) = s;
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
                t(j, i) = (*this)(i, j);
            }
        }
        return t;
    }

    VecN MatMN::SolveGS(const MatMN& A, const VecN& b)
    {
        int N = b.N;
        VecN X(N,0);
        for (int iterations = 0; iterations < N; iterations++) {
            for (int i = 0; i < N; i++) {
                if (A(i, i) != 0) {
                    X(i) += (b(i) / A(i, i)) - (A[i].Dot(X) / A(i, i));
                }
            }
        }
        return X;
    }

    float MatMN::RowView::Dot(const RowView& other) const noexcept
    {
        float sum = 0.0f;
        for (int i = 0; i < n; ++i)
            sum += p[i] * other.p[i];
        return sum;
    }

    float MatMN::RowView::Dot(const VecN& v) const noexcept
    {
        float sum = 0.0f;
        for (int i = 0; i < n; ++i)
            sum += p[i] * v(i);
        return sum;
    }

    float MatMN::RowViewConst::Dot(const RowViewConst& other) const noexcept
    {
        float sum = 0.0f;
        for (int i = 0; i < n; ++i)
            sum += p[i] * other.p[i];
        return sum;
    }

    float MatMN::RowViewConst::Dot(const VecN& v) const noexcept
    {
        float sum = 0.0f;
        for (int i = 0; i < n; ++i)
            sum += p[i] * v(i);
        return sum;
    }

    float MatMN::ColumnView::Dot(const ColumnView& other) const noexcept
    {
        float sum = 0.0f;
        for (int i = 0; i < m; ++i)
            sum += base[i * stride] * other.base[i * other.stride];
        return sum;
    }

    float MatMN::ColumnView::Dot(const VecN& v) const noexcept
    {
        float sum = 0.0f;
        for (int i = 0; i < m; ++i)
            sum += base[i * stride] * v(i);
        return sum;
    }

    float MatMN::ColumnViewConst::Dot(const ColumnViewConst& other) const noexcept
    {
        float sum = 0.0f;
        for (int i = 0; i < m; ++i)
            sum += base[i * stride] * other.base[i * other.stride];
        return sum;
    }

    float MatMN::ColumnViewConst::Dot(const VecN& v) const noexcept
    {
        float sum = 0.0f;
        for (int i = 0; i < m; ++i)
            sum += base[i * stride] * v(i);
        return sum;
    }

}
