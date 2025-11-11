#include "VecN.h"

namespace FlatPhysics {

	VecN::VecN()
		: N(0)
	{
	}

	VecN::VecN(int n)
		: N(n)
	{
		data.resize(N, 0.0f);
	}

	VecN::VecN(const VecN& v)
		: N(v.N), data(v.data)
	{
	}

	VecN::VecN(int n, float value) 
		: N(n), data(n, value)
	{}

	VecN::~VecN() = default;

	VecN VecN::operator+(const VecN& v)
	{
		VecN out(N);
		for (int i = 0; i < N; i++) {
			out.data[i] = data[i] + v.data[i];
		}
		return out;
	}

	VecN VecN::operator-(const VecN& v)
	{
		VecN out(N);
		for (int i = 0; i < N; i++) {
			out.data[i] = data[i] - v.data[i];
		}
		return out;
	}

	VecN VecN::operator*(float n)
	{
		VecN out(N);
		for (int i = 0; i < N; i++) {
			out.data[i] = data[i] * n;
		}
		return out;
	}

	const VecN& VecN::operator+=(const VecN& v)
	{
		for (int i = 0; i < N; i++) {
			data[i] += v.data[i];
		}
		return *this;
	}

	const VecN& VecN::operator-=(const VecN& v)
	{
		for (int i = 0; i < N; i++) {
			data[i] -= v.data[i];
		}
		return *this;
	}

	const VecN& VecN::operator*=(float v)
	{
		for (int i = 0; i < N; i++) {
			data[i] *= v;
		}
		return *this;
	}

	float VecN::operator()(int index) const
	{
		return data[index];
	}

	float& VecN::operator()(int index)
	{
		return data[index];
	}

	float VecN::Dot(const VecN& v) const
	{
		float sum = 0.0f;
		for (int i = 0; i < N; i++) {
			sum += data[i] * v.data[i];
		}
		return sum;
	}

	void VecN::Zero()
	{
		for (int i = 0; i < N; i++) {
			data[i] = 0.0f;
		}
	}

}