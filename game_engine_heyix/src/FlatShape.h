#pragma once
#include "Vector2.h"
#include <memory>

namespace FlatPhysics {
	enum class ShapeType {
		Circle,Box,Polygon
	};

	class CircleShape;
	class BoxShape;
	class PolygonShape;
	class Shape {
	public:
		virtual ~Shape() = default;
		virtual ShapeType GetType() const = 0;
		virtual std::unique_ptr<Shape> Clone() const = 0;
		CircleShape* AsCircle(); 

		const CircleShape* AsCircle() const;

		BoxShape* AsBox();

		const BoxShape* AsBox() const; 

		PolygonShape* AsPolygon();

		const PolygonShape* AsPolygon() const;
	};





	class CircleShape :public Shape {
	public:
		CircleShape() {}
		CircleShape(float radius):radius(radius) {}
		CircleShape(Vector2 center, float radius) :center(center), radius(radius) {}
	public:
		float radius = 1.0f;
		Vector2 center{};
		ShapeType GetType()const override { return ShapeType::Circle; }
		std::unique_ptr<Shape> Clone() const override { return std::make_unique<CircleShape>(*this); }
	};





	class BoxShape :public Shape {
	public:
		BoxShape() { UpdateVertices(); }
		BoxShape(float w, float h) : width(w), height(h){
			UpdateVertices();
		}
	public:
		ShapeType GetType()const override { return ShapeType::Box; }
		std::unique_ptr<Shape> Clone() const override { return std::make_unique<BoxShape>(*this); }

		void SetSize(float w, float h) {
			width = w;
			height = h;
			UpdateVertices();
		}
	private:
		void UpdateVertices() {
			float hx = width * 0.5f;
			float hy = height * 0.5f;
			vertices = {
				{-hx, -hy},
				{+hx, -hy},
				{+hx, +hy},
				{-hx, +hy}
			};
		}
	public:
		float width = 1.0f;
		float height = 1.0f;
		std::vector<Vector2> vertices;
	};





	class PolygonShape : public Shape {
	public:
		PolygonShape() {}
		PolygonShape(const std::vector<Vector2>& verts) :vertices(verts){}
	public:
		std::vector<Vector2> vertices;

		ShapeType GetType() const override { return ShapeType::Polygon; }
		std::unique_ptr<Shape> Clone() const override {
			return std::make_unique<PolygonShape>(*this);
		}

		void SetVertices(const std::vector<Vector2>& verts) {
			vertices = verts;
		}
	};
}