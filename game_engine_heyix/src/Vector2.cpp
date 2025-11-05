#include "Vector2.h"

float Vector2::Normalize()
{
	float result = vector2.Normalize();
	return result;
}

Vector2 Vector2::NormalizedVector()
{
	b2Vec2 v = vector2;
	float len = v.Normalize();
	if (len == 0.0f) {
		return Vector2::Zero();
	}
	return Vector2(v);
}

float Vector2::Length()
{
	return vector2.Length();
}

float Vector2::Distance(const Vector2& a, const Vector2& b)
{
	return b2Distance(a.vector2, b.vector2);
}

float Vector2::Dot(const Vector2& a, const Vector2& b)
{
	return b2Dot(a.vector2, b.vector2);
}

float Vector2::Cross(const Vector2& a, const Vector2& b)
{
	return b2Cross(a.vector2, b.vector2);
}



Vector2 Vector2::Lua_Operator_Add(const Vector2& other)
{
	return Vector2(vector2 + other.vector2);
}

Vector2 Vector2::Lua_Operator_Sub(const Vector2& other)
{
	return Vector2(vector2 - other.vector2);
}

Vector2 Vector2::Lua_Operator_Mul(const float multiplier)
{
	return Vector2(multiplier * vector2);
}

