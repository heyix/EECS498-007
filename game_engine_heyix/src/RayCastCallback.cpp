#include "RayCastCallback.h"
#include <iostream>
#include "RigidBody.h"
float SingleHitRayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
{
	RigidBody* rigid = reinterpret_cast<RigidBody*>(fixture->GetUserData().pointer);
	if (!rigid) {
		return -1.0f;
	}
	if (fraction == 0.0f) {
		return -1.0f;
	}
	if (fraction < min_fraction) {
		min_fraction = fraction;
		fixture_found = fixture;
		point_found = point;
		normal_found = normal;
	}
	return fraction;
}

float AllHitRayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
{
	RigidBody* rigid = reinterpret_cast<RigidBody*>(fixture->GetUserData().pointer);
	if (!rigid) {
		return -1.0f;
	}
	if (fraction == 0.0f) {
		return -1.0f;
	}
	hits.push_back({fixture,point,normal,fraction});
	return 1.0f;
}
