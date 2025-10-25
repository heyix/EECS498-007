#pragma once
#include "Vector2.h"
#include <string>
#include <memory>
namespace FlatPhysics {
	enum class ShapeType {
		Circle, Box
	};


	class FlatBody {
	private:
		FlatBody(
			const Vector2& position,
			float density,
			float mass,
			float restitution,
			float area,
			bool is_static,
			float radius,
			float width,
			float height,
			ShapeType shape_type
		)
			: position(position),
			linear_velocity(Vector2::Zero()),
			rotation(0.0f),
			rotation_velocity(0.0f),
			density(density),
			mass(mass),
			restitution(restitution),
			area(area),
			is_static(is_static),
			radius(radius),
			width(width),
			height(height),
			shape_type(shape_type)
		{
		}
	private:
		Vector2 position;
		Vector2 linear_velocity;
		float rotation;
		float rotation_velocity;



	public:
		const float density;
		const float mass;
		const float restitution;
		const float area;

		const bool is_static;

		const float radius;
		const float width;
		const float height;

		const ShapeType shape_type;
	public:
		const Vector2& GetPosition()const { return position; }
	public:
		void Move(Vector2 amount);
		void MoveTo(Vector2 position);
	public:
		static bool CreateCircleBody(float radius, Vector2 position, float density, bool is_static, float restitution, std::unique_ptr<FlatBody>& out_body, std::string* error_message = nullptr);
		static bool CreateBoxBody(float width, float height, Vector2 position, float density, bool is_static, float restitution, std::unique_ptr<FlatBody>& out_body, std::string* error_message = nullptr);
	};
}
