#pragma once
#include "box2d/box2d.h"
#include <cmath>
#include <functional>

class Vector2 {
private:
	b2Vec2 vector2;
public:
	Vector2() {
	}
	Vector2(float x, float y) {
		vector2 = { x,y };
	}
	Vector2(const b2Vec2& vec) {
		vector2 = vec;
	}
	float& x() { return vector2.x; }
	float& y() { return vector2.y; }
	const float& x() const { return vector2.x; }
	const float& y() const { return vector2.y; }
	Vector2 operator+(const Vector2& other) const {
		return Vector2(vector2.x + other.vector2.x, vector2.y + other.vector2.y);
	}

	Vector2 operator-(const Vector2& other) const {
		return Vector2(vector2.x - other.vector2.x, vector2.y - other.vector2.y);
	}

	Vector2 operator*(float scalar) const {
		return Vector2(vector2.x * scalar, vector2.y * scalar);
	}
	bool operator==(const Vector2& other) const {
		return vector2 == other.vector2;
	}
	float GetX() const { return vector2.x; }
	void SetX(float x) { vector2.x = x;}

	float GetY() const { return vector2.y; }
	void SetY(float y) { vector2.y = y; }
	float Normalize();
	float Length();
	static float Distance(const Vector2& a, const Vector2& b);
	static float Dot(const Vector2& a, const Vector2& b);
	static float Cross(const Vector2& a, const Vector2& b);
public:
	float Lua_Normalize() { return Normalize(); }
	float Lua_Length() { return Length(); }
	Vector2 Lua_Operator_Add(const Vector2& other);
	Vector2 Lua_Operator_Sub(const Vector2& other);
	Vector2 Lua_Operator_Mul(const float multiplier);
	static float Lua_Distance(const Vector2& a, const Vector2& b) { return Distance(a, b); }
	static float Lua_Dot(const Vector2& a, const Vector2& b) { return Dot(a, b); }

};

namespace std {
	template<>
	struct hash<Vector2> {
		std::size_t operator()(const Vector2& v) const noexcept {
			std::size_t hx = std::hash<float>{}(v.x());
			std::size_t hy = std::hash<float>{}(v.y());
			return hx ^ (hy + 0x9e3779b9 + (hx << 6) + (hx >> 2));
		}
	};
}