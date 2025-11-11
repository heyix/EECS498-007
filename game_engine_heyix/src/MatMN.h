#pragma once
#include <vector>
#include "VecN.h"

namespace FlatPhysics {

    class MatMN {
    public:
        class RowView {
        public:
            RowView(float* p, int n) noexcept : p(p), n(n) {}
            float& operator()(int j) {  return p[j]; }
            const float& operator()(int j) const {  return p[j]; }
            int size() const noexcept { return n; }
            float Dot(const RowView& other) const noexcept;
            float Dot(const VecN& v) const noexcept;
        private:
            float* p = nullptr;
            int    n = 0;
        };
        class RowViewConst {
        public:
            RowViewConst(const float* p, int n) noexcept : p(p), n(n) {}
            const float& operator()(int j) const {  return p[j]; }
            int size() const noexcept { return n; }
            float Dot(const RowViewConst& other) const noexcept;
            float Dot(const VecN& v) const noexcept;
        private:
            const float* p = nullptr;
            int          n = 0;
        };

        class ColumnView {
        public:
            ColumnView(float* base, int m, int stride) noexcept : base(base), m(m), stride(stride) {}
            float& operator()(int i) {  return base[i * stride]; }
            const float& operator()(int i) const {  return base[i * stride]; }
            int size() const noexcept { return m; }
            float Dot(const ColumnView& other) const noexcept;
            float Dot(const VecN& v) const noexcept;
        private:
            float* base = nullptr;
            int    m = 0;
            int    stride = 0;
        };
        class ColumnViewConst {
        public:
            ColumnViewConst(const float* base, int m, int stride) noexcept : base(base), m(m), stride(stride) {}
            const float& operator()(int i) const { return base[i * stride]; }
            int size() const noexcept { return m; }
            float Dot(const ColumnViewConst& other) const noexcept;
            float Dot(const VecN& v) const noexcept;
        private:
            const float* base = nullptr;
            int          m = 0;
            int          stride = 0;
        };

    public:
        MatMN();
        MatMN(int M, int N);
        MatMN(int M, int N, float init_value);
        MatMN(const MatMN& m);
        MatMN& operator=(const MatMN& m);
        MatMN(MatMN&& other) noexcept;

        VecN operator*(const VecN& v) const;

        MatMN operator*(const MatMN& m) const;
        RowView operator[](int i) {
            return RowView(data.data() + i * N, N);
        }
        RowViewConst operator[](int i) const {
            return RowViewConst(data.data() + i * N, N);
        }
        inline float& operator()(int i, int j)       noexcept { return data[i * N + j]; }
        inline const float& operator()(int i, int j) const noexcept { return data[i * N + j]; }
    public:


        void Zero();

        MatMN Transpose() const;




        ColumnView GetColumn(int j) {
            return ColumnView(data.data() + j, M, N);
        }
        ColumnViewConst GetColumn(int j) const {
            return ColumnViewConst(data.data() + j, M, N);
        }
    public:
        static VecN SolveGS(const MatMN& A, const VecN& b);
    public:
        int M = 0;
        int N = 0;

    private:
        std::vector<float> data;
    };

}
