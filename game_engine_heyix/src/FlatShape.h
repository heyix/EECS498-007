#pragma once
#include "Vector2.h"
#include <memory>

namespace FlatPhysics {
	enum class ShapeType {
		Circle,Polygon
	};

	class CircleShape;
	class PolygonShape;
	class Shape {
	public:
		virtual ~Shape() = default;
		virtual ShapeType GetType() const = 0;
		virtual std::unique_ptr<Shape> Clone() const = 0;
		CircleShape* AsCircle(); 

		const CircleShape* AsCircle() const;

		PolygonShape* AsPolygon();

		const PolygonShape* AsPolygon() const;
	};





	class CircleShape :public Shape {
	public:
		CircleShape() {}
		CircleShape(float radius):radius(radius) {}
		CircleShape(const Vector2& center, float radius) :center(center), radius(radius) {}
	public:
		float radius = 1.0f;
		Vector2 center{};
		ShapeType GetType()const override { return ShapeType::Circle; }
		std::unique_ptr<Shape> Clone() const override { return std::make_unique<CircleShape>(*this); }
	};





	class PolygonShape : public Shape {
	public:
		PolygonShape() {}
		PolygonShape(const std::vector<Vector2>& verts) { SetVertices(verts); }
	public:
		std::vector<Vector2> vertices;

		ShapeType GetType() const override { return ShapeType::Polygon; }
		std::unique_ptr<Shape> Clone() const override {
			return std::make_unique<PolygonShape>(*this);
		}

		void SetVertices(const std::vector<Vector2>& verts) {
			vertices = verts;
		}
		void SetAsBox(float width, float height, Vector2 center = Vector2::Zero(), float angle = 0.0f);

	};
}