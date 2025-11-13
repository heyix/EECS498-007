#pragma once
#include "FlatFixture.h"
#include "Vector2.h"
#include <vector>
#include "FlatContact.h"
#include "Collision.h"
namespace FlatPhysics {
	class FlatManifold {
	public:
		FlatManifold(FlatFixture* fixtureA_, FlatFixture* fixtureB_, std::vector<ContactPoint>&& contact_point)
			:fixtureA(fixtureA_), fixtureB(fixtureB_), contact_points(std::move(contact_point))
		{
		}
	public:
		FlatFixture* fixtureA;
		FlatFixture* fixtureB;
		std::vector<ContactPoint> contact_points;
	};
}
