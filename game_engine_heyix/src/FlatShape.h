#pragma once
#include "Vector2.h"
#include <memory>

namespace FlatPhysics {
	enum class ShapeType {
		Circle,Box
	};

	class Shape {
	public:
		virtual ~Shape() = default;
		virtual ShapeType GetType() const = 0;
		virtual std::unique_ptr<Shape> Clone() const = 0;
	};
	class CircleShape :public Shape {
	public:
		float radius = 1.0f;
		ShapeType GetType()const override { return ShapeType::Circle; }
		std::unique_ptr<Shape> Clone() const override { return std::make_unique<CircleShape>(*this); }
	};

	class BoxShape :public Shape {
	public:
		float width = 1.0f;
		float height = 1.0f;
		ShapeType GetType()const override { return ShapeType::Box; }
		std::unique_ptr<Shape> Clone() const override { return std::make_unique<BoxShape>(*this); }
	};
}