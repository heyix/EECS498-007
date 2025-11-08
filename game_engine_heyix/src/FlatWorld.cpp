#include "FlatWorld.h"
#include "FlatFixture.h"
#include "Collision.h"
#include <algorithm>
#include "Renderer.h"
#include "Engine.h"
namespace FlatPhysics {
	void FlatWorld::AddBody(FlatBody* body)
	{
		if (!body || index_map.count(body))return;
		int index = bodies.size();
		bodies.push_back(body);
		index_map[body] = index;
	}

	bool FlatWorld::RemoveBody(FlatBody* body)
	{
		auto it = index_map.find(body);
		if (it == index_map.end())return false;
		int index = it->second;
		int last = bodies.size() - 1;
		if (index != last) {
			std::swap(bodies[index], bodies[last]);
			index_map[bodies[index]] = index;
		}
		bodies.pop_back();
		index_map.erase(it);
		return true;
	}

	FlatBody* FlatWorld::GetBody(int index)
	{
		if (index < 0 || index >= bodies.size()) {
			return nullptr;
		}
		return bodies[index];
	}
	void FlatWorld::Step(float time)
	{
		for (int i = 0; i < bodies.size(); i++) {
			bodies[i]->Step(time, gravity);
		}
		CollisionDetectionStep();

	}
	void FlatWorld::CollisionDetectionStep()
	{
		contacts.clear();
		BroadPhase();
		NarrowPhase();

	}
	bool FlatWorld::ShouldCollide(FlatFixture* fixture_a, FlatFixture* fixture_b)
	{
		FlatBody* a = fixture_a->GetBody();
		FlatBody* b = fixture_b->GetBody();
		if (a == b) {
			return false;
		}
		if (!FlatAABB::IntersectAABB(fixture_a->GetAABB(), fixture_b->GetAABB())) {
			return false;
		}
		return true;
	}
	void FlatWorld::ResolveCollision(const FlatManifold& manifold)
	{
		FlatBody* bodyA = manifold.fixtureA->GetBody();
		FlatBody* bodyB = manifold.fixtureB->GetBody();
		float depth = manifold.depth;
		const Vector2& normal = manifold.normal;

		Vector2 relative_velocity = bodyB->GetLinearVelocity() - bodyA->GetLinearVelocity();
		if (Vector2::Dot(relative_velocity, normal) > 0) {
			return;
		}
		float e = std::min(bodyA->restitution, bodyB->restitution);
		float j = -(1 + e) * Vector2::Dot(relative_velocity, normal);
		j /= (bodyA->GetInverseMass() + bodyB->GetInverseMass());

		Vector2 impulse = j * normal;
		Vector2 velocity_a = bodyA->GetLinearVelocity();
		velocity_a -= impulse * bodyA->GetInverseMass();
		bodyA->SetLinearVelocity(velocity_a);
		Vector2 velocity_b = bodyB->GetLinearVelocity();
		velocity_b += impulse * bodyB->GetInverseMass();
		bodyB->SetLinearVelocity(velocity_b);
	}
	void FlatWorld::SeperateBodies(FlatBody* bodyA, FlatBody* bodyB, const Vector2& mtv)
	{
		if (!bodyA->is_static && !bodyB->is_static) {
			bodyA->Move(-mtv * 0.5f);
			bodyB->Move(mtv * 0.5f);
		}
		else if (!bodyA->is_static && bodyB->is_static) {
			bodyA->Move(-mtv);
		}
		else if (bodyA->is_static && !bodyB->is_static) {
			bodyB->Move(mtv);
		}
	}
	void FlatWorld::BroadPhase()
	{
		contact_pairs.clear();
		for (int i = 0; i < (int)bodies.size() - 1; i++) {
			FlatBody* bodyA = bodies[i];
			const std::vector<std::unique_ptr<FlatFixture>>& fixturesA = bodyA->GetFixtures();
			for (int j = i + 1; j < bodies.size(); j++) {
				FlatBody* bodyB = bodies[j];
				if (bodyA->is_static && bodyB->is_static) {
					continue;
				}
				const std::vector<std::unique_ptr<FlatFixture>>& fixturesB = bodyB->GetFixtures();
				for (const std::unique_ptr<FlatFixture>& pa : fixturesA) {
					for (const std::unique_ptr<FlatFixture>& pb : fixturesB) {
						FlatFixture* fa = pa.get();
						FlatFixture* fb = pb.get();
						if (!ShouldCollide(fa, fb)) {
							continue;
						}
						contact_pairs.push_back({ fa,fb });
					}
				}
			}
		}
	}
	void FlatWorld::NarrowPhase()
	{
		for (ContactPair& pair : contact_pairs) {
			FlatFixture* fa = pair.fixture_a;
			FlatFixture* fb = pair.fixture_b;
			FlatBody* bodyA = fa->GetBody();
			FlatBody* bodyB = fb->GetBody();
			Vector2 normal;
			float depth;
			if (Collision::DetectCollision(fa, fb, &normal, &depth)) {
				SeperateBodies(bodyA, bodyB, normal * depth);
				ContactPoints contact_points = Collision::FindContactPoints(fa, fb);
				FlatManifold contact{ fa,fb,normal,depth,contact_points };
				contacts.push_back(contact);
				ResolveCollision(contact);

			}
		}
	}
	void FlatWorld::DrawContactPoints()
	{
		for (FlatManifold& manifold : contacts) {
			ContactPoints& contact_points = manifold.contact_points;
			if (contact_points.points_num > 0) {
				constexpr float kMarkerHalfSize = 0.05f;
				static const std::vector<Vector2> markerVerts = {
					{ -kMarkerHalfSize, -kMarkerHalfSize },
					{  kMarkerHalfSize, -kMarkerHalfSize },
					{  kMarkerHalfSize,  kMarkerHalfSize },
					{ -kMarkerHalfSize,  kMarkerHalfSize }
				};

				auto queueMarker = [&](const Vector2& p) {
					Engine::instance->renderer->draw_polygon(markerVerts, p, 255, 0, 0, 255, false);
					};

				queueMarker(contact_points.point1);
				if (contact_points.points_num > 1) {
					queueMarker(contact_points.point2);
				}
			}
		}
		
	}
}
