#pragma once
#include <vector>
namespace FlatPhysics {
	class VecN {
	public:
		VecN();
		VecN(int N);
		VecN(const VecN& v);
		VecN(int N, float value);
		~VecN();
	public:
		VecN operator+(const VecN& v);
		VecN operator-(const VecN& v);
		VecN operator*(const float n);
		const VecN& operator+= (const VecN& v);
		const VecN& operator-= (const VecN& v);
		const VecN& operator*= (const float v);
		float operator()(const int index)const;
		float& operator()(const int index);
		float Dot(const VecN& v)const;
	public:
		void Zero();
	public:
		int N;
		std::vector<float> data;
	};
}