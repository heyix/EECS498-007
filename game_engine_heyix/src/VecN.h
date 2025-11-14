#pragma once

namespace FlatPhysics {

    template<int N_>
    class VecN {
    public:
        int N = N_;

        VecN() {
            Zero();
        }

        VecN(const VecN& v) {
            for (int i = 0; i < N_; ++i)
                data[i] = v.data[i];
        }

        explicit VecN(float value) {
            for (int i = 0; i < N_; ++i)
                data[i] = value;
        }

        ~VecN() = default;

    public:
        VecN operator+(const VecN& v) const {
            VecN out;
            for (int i = 0; i < N_; ++i) {
                out.data[i] = data[i] + v.data[i];
            }
            return out;
        }

        VecN operator-(const VecN& v) const {
            VecN out;
            for (int i = 0; i < N_; ++i) {
                out.data[i] = data[i] - v.data[i];
            }
            return out;
        }

        VecN operator*(const float n) const {
            VecN out;
            for (int i = 0; i < N_; ++i) {
                out.data[i] = data[i] * n;
            }
            return out;
        }

        const VecN& operator+=(const VecN& v) {
            for (int i = 0; i < N_; ++i) {
                data[i] += v.data[i];
            }
            return *this;
        }

        const VecN& operator-=(const VecN& v) {
            for (int i = 0; i < N_; ++i) {
                data[i] -= v.data[i];
            }
            return *this;
        }

        const VecN& operator*=(const float v) {
            for (int i = 0; i < N_; ++i) {
                data[i] *= v;
            }
            return *this;
        }

        float operator()(const int index) const {
            return data[index];
        }

        float& operator()(const int index) {
            return data[index];
        }

        float Dot(const VecN& v) const {
            float sum = 0.0f;
            for (int i = 0; i < N_; ++i) {
                sum += data[i] * v.data[i];
            }
            return sum;
        }

    public:
        void Zero() {
            for (int i = 0; i < N_; ++i) {
                data[i] = 0.0f;
            }
        }

    private:
        float data[N_]{};
    };

} 
