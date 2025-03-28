#pragma once
#include "box2d/box2d.h"
#include  "Vector2.h"
#include <vector>
class SingleHitRayCastCallback:public b2RayCastCallback {
public:
	b2Fixture* fixture_found = nullptr;
	b2Vec2 point_found;
	b2Vec2 normal_found;
	float min_fraction = 2.0f;
public:
	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)override;
};

class AllHitRayCastCallback :public b2RayCastCallback {
public:
	class RayCastHit {
	public:
		b2Fixture* fixture = nullptr;
		b2Vec2 point_found;
		b2Vec2 normal_found;
		float fraction;
	};
	class RayCastHitComparator {
	public:
		bool operator()(const RayCastHit& a, const RayCastHit& b) {
			return a.fraction < b.fraction;
		}
	};
	std::vector<RayCastHit> hits;
public:
	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)override;
};