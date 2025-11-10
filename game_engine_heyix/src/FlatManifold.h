#pragma once
#include "FlatFixture.h"
#include "Vector2.h"
#include <vector>
#include "Collision.h"
namespace FlatPhysics {
	class FlatManifold {
	public:
		FlatManifold(FlatFixture* fixtureA_, FlatFixture* fixtureB_, const Vector2& normal_, float depth_, const ContactPointsOld& contact_points_)
			:fixtureA(fixtureA_), fixtureB(fixtureB_),normal(normal_),depth(depth_),contact_points(contact_points_)
		{
		}
	public:
		FlatFixture* fixtureA;
		FlatFixture* fixtureB;
		Vector2 normal;
		float depth;
		ContactPointsOld contact_points;
	};
}
