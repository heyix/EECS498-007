#pragma once
#include "FlatFixture.h"
#include "Vector2.h"
#include <vector>
#include "FlatContact.h"
#include "Collision.h"
namespace FlatPhysics {
	class FlatManifold {
	public:
		FlatManifold(FlatFixture* fixtureA_, FlatFixture* fixtureB_, const FixedSizeContainer<ContactPoint, 2>& contact_point)
			:fixtureA(fixtureA_), fixtureB(fixtureB_), contact_points(contact_point),touched_this_step(true)
		{
		}
		FlatManifold(FlatFixture* a, FlatFixture* b)
			: fixtureA(a), fixtureB(b), touched_this_step(false)
		{}
	public:
		FlatFixture* fixtureA;
		FlatFixture* fixtureB;
		FixedSizeContainer<ContactPoint, 2> contact_points;
		bool touched_this_step = false;
		bool island_flag = false;
		FlatContactEdge* edgeA = nullptr;
		FlatContactEdge* edgeB = nullptr;
	};
}
