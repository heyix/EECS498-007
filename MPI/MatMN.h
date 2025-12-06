#pragma once
#include <vector>
#include "VecN.h"

namespace FlatPhysics {

    template<int M_, int N_>
    class MatMN {
    public:

        class RowView {
        public:
            RowView(float* p, int n) noexcept : p(p), n(n) {}

            float& operator()(int j) { return p[j]; }
            const float& operator()(int j) const { return p[j]; }
            int size() const noexcept { return n; }

            float Dot(const RowView& other) const noexcept {
                float sum = 0.0f;
                for (int i = 0; i < n; ++i)
                    sum += p[i] * other.p[i];
                return sum;
            }

            float Dot(const VecN<N_>& v) const noexcept {
                float sum = 0.0f;
                for (int i = 0; i < n; ++i)
                    sum += p[i] * v(i);
                return sum;
            }

        private:
            float* p = nullptr;
            int    n = 0;
        };

        class RowViewConst {
        public:
            RowViewConst(const float* p, int n) noexcept : p(p), n(n) {}

            const float& operator()(int j) const { return p[j]; }
            int size() const noexcept { return n; }

            float Dot(const RowViewConst& other) const noexcept {
                float sum = 0.0f;
                for (int i = 0; i < n; ++i)
                    sum += p[i] * other.p[i];
                return sum;
            }

            float Dot(const VecN<N_>& v) const noexcept {
                float sum = 0.0f;
                for (int i = 0; i < n; ++i)
                    sum += p[i] * v(i);
                return sum;
            }

        private:
            const float* p = nullptr;
            int          n = 0;
        };

        class ColumnView {
        public:
            ColumnView(float* base, int m, int stride) noexcept
                : base(base), m(m), stride(stride) {}

            float& operator()(int i) { return base[i * stride]; }
            const float& operator()(int i) const { return base[i * stride]; }
            int size() const noexcept { return m; }

            float Dot(const ColumnView& other) const noexcept {
                float sum = 0.0f;
                for (int i = 0; i < m; ++i)
                    sum += base[i * stride] * other.base[i * other.stride];
                return sum;
            }

            float Dot(const VecN<M_>& v) const noexcept {
                float sum = 0.0f;
                for (int i = 0; i < m; ++i)
                    sum += base[i * stride] * v(i);
                return sum;
            }

        private:
            float* base = nullptr;
            int    m = 0;
            int    stride = 0;
        };

        class ColumnViewConst {
        public:
            ColumnViewConst(const float* base, int m, int stride) noexcept
                : base(base), m(m), stride(stride) {}

            const float& operator()(int i) const { return base[i * stride]; }
            int size() const noexcept { return m; }

            float Dot(const ColumnViewConst& other) const noexcept {
                float sum = 0.0f;
                for (int i = 0; i < m; ++i)
                    sum += base[i * stride] * other.base[i * other.stride];
                return sum;
            }

            float Dot(const VecN<M_>& v) const noexcept {
                float sum = 0.0f;
                for (int i = 0; i < m; ++i)
                    sum += base[i * stride] * v(i);
                return sum;
            }

        private:
            const float* base = nullptr;
            int          m = 0;
            int          stride = 0;
        };

    public:
        MatMN() {
            Zero();
        }

        explicit MatMN(float init_value) {
            for (int i = 0; i < M_ * N_; ++i)
                data[i] = init_value;
        }

        MatMN(const MatMN& m) {
            for (int i = 0; i < M_ * N_; ++i)
                data[i] = m.data[i];
        }

        MatMN& operator=(const MatMN& m) {
            if (this == &m) return *this;
            for (int i = 0; i < M_ * N_; ++i)
                data[i] = m.data[i];
            return *this;
        }

        MatMN(MatMN&& other) noexcept {
            for (int i = 0; i < M_ * N_; ++i)
                data[i] = other.data[i];
        }

        ~MatMN() = default;

        RowView operator[](int i) {
            return RowView(data + i * N_, N_);
        }

        RowViewConst operator[](int i) const {
            return RowViewConst(data + i * N_, N_);
        }

        inline float& operator()(int i, int j)       noexcept { return data[i * N_ + j]; }
        inline const float& operator()(int i, int j) const noexcept { return data[i * N_ + j]; }

        VecN<M_> operator*(const VecN<N_>& v) const {
            VecN<M_> out;
            for (int i = 0; i < M_; ++i) {
                float sum = 0.0f;
                for (int j = 0; j < N_; ++j) {
                    sum += (*this)(i, j) * v(j);
                }
                out(i) = sum;
            }
            return out;
        }

        template<int P_>
        MatMN<M_, P_> operator*(const MatMN<N_, P_>& rhs) const {
            MatMN<M_, P_> out;
            out.Zero();
            for (int i = 0; i < M_; ++i) {
                for (int j = 0; j < P_; ++j) {
                    float sum = 0.0f;
                    for (int k = 0; k < N_; ++k) {
                        sum += (*this)(i, k) * rhs(k, j);
                    }
                    out(i, j) = sum;
                }
            }
            return out;
        }

        void Zero() {
            for (int i = 0; i < M_ * N_; ++i)
                data[i] = 0.0f;
        }

        MatMN<N_, M_> Transpose() const {
            MatMN<N_, M_> t;
            for (int i = 0; i < M_; ++i) {
                for (int j = 0; j < N_; ++j) {
                    t(j, i) = (*this)(i, j);
                }
            }
            return t;
        }

        ColumnView GetColumn(int j) {
            return ColumnView(data + j, M_, N_);
        }

        ColumnViewConst GetColumn(int j) const {
            return ColumnViewConst(data + j, M_, N_);
        }

        static VecN<N_> SolveGS(const MatMN<M_, N_>& A, const VecN<N_>& b) {
            VecN<N_> X(0.0f); 

            for (int iter = 0; iter < N_; ++iter) {
                for (int i = 0; i < N_; ++i) {
                    float diag = A(i, i);
                    if (diag != 0.0f) {
                        float rowDotX = 0.0f;
                        for (int j = 0; j < N_; ++j) {
                            rowDotX += A(i, j) * X(j);
                        }
                        X(i) += (b(i) - rowDotX) / diag;
                    }
                }
            }
            return X;
        }

    private:
        float data[M_ * N_]{}; 
    };

}