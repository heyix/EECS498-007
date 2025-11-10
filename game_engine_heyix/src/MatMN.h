#pragma once
#include <vector>
#include <cassert>
#include "VecN.h"

namespace FlatPhysics {

    class MatMN {
    public:
        class RowView {
        public:
            RowView(float* p, int n) noexcept : p_mut(p), p_const(p), n(n), is_const(false) {}
            RowView(const float* p, int n) noexcept : p_mut(nullptr), p_const(p), n(n), is_const(true) {}

            float& operator[](int j) {
                return p_mut[j];
            }
            const float& operator[](int j) const {
                return p_const[j];
            }

            int size() const noexcept { return n; }

        private:
            float* p_mut = nullptr;
            const float* p_const = nullptr;
            int          n = 0;
            bool         is_const = false;
        };

    public:
        MatMN();
        MatMN(int M, int N);
        MatMN(const MatMN& m);
        MatMN& operator=(const MatMN& m); 

        VecN  operator*(const VecN& v) const;
        MatMN operator*(const MatMN& m) const;

        void  Zero();
        MatMN Transpose() const;

        RowView operator[](int i) {
            assert(i >= 0 && i < M);
            return RowView(&data[i * N], N); 
        }
        RowView operator[](int i) const { 
            assert(i >= 0 && i < M);
            return RowView(&data[i * N], N);
        }

    public:
        int M = 0;
        int N = 0;
    private:
        std::vector<float> data;          
    };

}
