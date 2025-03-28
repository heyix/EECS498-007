#pragma once
#include "box2d/box2d.h"
class Vector2 {
public:
	float x = 0;
	float y = 0;
private:
	b2Vec2 vector2;
	void update_x_y() {
		x = vector2.x;
		y = vector2.y;
	}
public:
	Vector2() {
	}
	Vector2(float x, float y) {
		vector2 = { x,y };
		update_x_y();
	}
	Vector2(const b2Vec2& vec) {
		vector2 = vec;
		update_x_y();
	}
	Vector2 operator+(const Vector2& other) const {
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 operator-(const Vector2& other) const {
		return Vector2(x - other.x, y - other.y);
	}

	Vector2 operator*(float scalar) const {
		return Vector2(x * scalar, y * scalar);
	}
	float GetX() const { return vector2.x; }
	void SetX(float x) { vector2.x = x; update_x_y();}

	float GetY() const { return vector2.y; }
	void SetY(float y) { vector2.y = y; update_x_y();}
	float Normalize();
	float Length();
	static float Distance(const Vector2& a, const Vector2& b);
	static float Dot(const Vector2& a, const Vector2& b);
public:
	float Lua_Normalize() { return Normalize(); }
	float Lua_Length() { return Length(); }
	Vector2 Lua_Operator_Add(const Vector2& other);
	Vector2 Lua_Operator_Sub(const Vector2& other);
	Vector2 Lua_Operator_Mul(const float multiplier);
	static float Lua_Distance(const Vector2& a, const Vector2& b) { return Distance(a, b); }
	static float Lua_Dot(const Vector2& a, const Vector2& b) { return Dot(a, b); }

};