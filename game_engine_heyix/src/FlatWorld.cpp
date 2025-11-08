#include "FlatWorld.h"
#include "FlatFixture.h"
#include "Collision.h"
#include <algorithm>
#include "Renderer.h"
#include "Engine.h"
#include "FlatMath.h"
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
	void FlatWorld::ResolveCollisionBasic(const FlatManifold& manifold)
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
		bodyA->AddLinearVelocity(-impulse * bodyA->GetInverseMass());
		bodyB->AddLinearVelocity(impulse * bodyB->GetInverseMass());
	}
	void FlatWorld::ResolveCollisionWithRotation(const FlatManifold& manifold)
	{
		FlatFixture* fixture_a = manifold.fixtureA;
		FlatFixture* fixture_b = manifold.fixtureB;
		float depth = manifold.depth;
		const Vector2& normal = manifold.normal;
		FlatBody* bodyA = fixture_a->GetBody();
		FlatBody* bodyB = fixture_b->GetBody();
		Vector2 world_mass_center_a = FlatTransform::TransformVector(bodyA->GetMassCenter(), bodyA->GetTransform());
		Vector2 world_mass_center_b = FlatTransform::TransformVector(bodyB->GetMassCenter(), bodyB->GetTransform());
		const ContactPoints& contact_points = manifold.contact_points;
		float e = std::min(bodyA->restitution, bodyB->restitution);
		std::vector<Vector2> contact_list{ contact_points.point1,contact_points.point2 };
		std::vector<Vector2> impulse_list;
		std::vector<Vector2> ra_list;
		std::vector<Vector2> rb_list;
		for (int i = 0; i < contact_points.points_num; i++) {
			Vector2 ra = contact_list[i] - world_mass_center_a;
			Vector2 rb = contact_list[i] - world_mass_center_b;
			Vector2 ra_perp = { -ra.y(),ra.x() };
			Vector2 rb_perp = { -rb.y(),rb.x() };
			Vector2 angular_linear_velocity_a = ra_perp * bodyA->GetAngularVelocity();
			Vector2 angular_linear_velocity_b = rb_perp * bodyB->GetAngularVelocity();
			Vector2 relative_velocity = (bodyB->GetLinearVelocity() + angular_linear_velocity_b) - (bodyA->GetLinearVelocity() + angular_linear_velocity_a);
			
			float contact_velocity_mag = Vector2::Dot(relative_velocity, normal);
			if (contact_velocity_mag > 0) {
				continue;
			}
			float ra_perp_dot_n = Vector2::Dot(ra_perp, normal);
			float rb_perp_dot_n = Vector2::Dot(rb_perp, normal);
			float denom = bodyA->GetInverseMass() + bodyB->GetInverseMass() + (ra_perp_dot_n * ra_perp_dot_n) * bodyA->GetInverseInertia() + (rb_perp_dot_n * rb_perp_dot_n) * bodyB->GetInverseInertia();
			float j = -(1 + e) * contact_velocity_mag;
			j /= denom;
			j /= (float)contact_points.points_num;
			Vector2 impulse = j * normal;
			impulse_list.push_back(impulse);
			ra_list.push_back(ra);
			rb_list.push_back(rb);
		}
		for (int i = 0; i < impulse_list.size(); i++) {
			Vector2& impulse = impulse_list[i];
			Vector2& ra = ra_list[i];
			Vector2& rb = rb_list[i];
			bodyA->AddLinearVelocity(-impulse * bodyA->GetInverseMass());
			bodyB->AddLinearVelocity(impulse * bodyB->GetInverseMass());
			bodyA->AddAngularVelocity(-Vector2::Cross(ra, impulse) * bodyA->GetInverseInertia());
			bodyB->AddAngularVelocity(Vector2::Cross(rb, impulse) * bodyB->GetInverseInertia());
		}
	}
	void FlatWorld::ResolveCollisionWithRotationAndFriction(const FlatManifold& manifold)
	{
		FlatFixture* fixture_a = manifold.fixtureA;
		FlatFixture* fixture_b = manifold.fixtureB;
		const Vector2& normal = manifold.normal;
		FlatBody* bodyA = fixture_a->GetBody();
		FlatBody* bodyB = fixture_b->GetBody();
		Vector2 world_mass_center_a = FlatTransform::TransformVector(bodyA->GetMassCenter(), bodyA->GetTransform());
		Vector2 world_mass_center_b = FlatTransform::TransformVector(bodyB->GetMassCenter(), bodyB->GetTransform());
		const ContactPoints& contact_points = manifold.contact_points;
		float e = std::min(bodyA->restitution, bodyB->restitution);
		std::vector<Vector2> contact_list{ contact_points.point1, contact_points.point2 };

		std::vector<Vector2> impulse_list;
		std::vector<float>   j_list;
		std::vector<int>     active_ids;

		for (int i = 0; i < contact_points.points_num; i++) {
			Vector2 ra = contact_list[i] - world_mass_center_a;
			Vector2 rb = contact_list[i] - world_mass_center_b;
			Vector2 ra_perp = { -ra.y(), ra.x() };
			Vector2 rb_perp = { -rb.y(), rb.x() };
			Vector2 angular_linear_velocity_a = ra_perp * bodyA->GetAngularVelocity();
			Vector2 angular_linear_velocity_b = rb_perp * bodyB->GetAngularVelocity();
			Vector2 relative_velocity = (bodyB->GetLinearVelocity() + angular_linear_velocity_b) - (bodyA->GetLinearVelocity() + angular_linear_velocity_a);

			float contact_velocity_mag = Vector2::Dot(relative_velocity, normal);
			if (contact_velocity_mag > 0) {
				continue;
			}
			float ra_perp_dot_n = Vector2::Dot(ra_perp, normal);
			float rb_perp_dot_n = Vector2::Dot(rb_perp, normal);
			float denom = bodyA->GetInverseMass() + bodyB->GetInverseMass()
				+ (ra_perp_dot_n * ra_perp_dot_n) * bodyA->GetInverseInertia()
				+ (rb_perp_dot_n * rb_perp_dot_n) * bodyB->GetInverseInertia();
			if (denom <= 0.0f) continue;

			float j = -(1 + e) * contact_velocity_mag;
			j /= denom;
			j /= (float)contact_points.points_num;
			j_list.push_back(j);

			impulse_list.push_back(j * normal);
			active_ids.push_back(i);
		}
		for (int k = 0; k < (int)impulse_list.size(); k++) {
			int i = active_ids[k];                
			Vector2& impulse = impulse_list[k];
			Vector2 ra = contact_list[i] - world_mass_center_a;
			Vector2 rb = contact_list[i] - world_mass_center_b;
			bodyA->AddLinearVelocity(-impulse * bodyA->GetInverseMass());
			bodyB->AddLinearVelocity(impulse * bodyB->GetInverseMass());
			bodyA->AddAngularVelocity(-Vector2::Cross(ra, impulse) * bodyA->GetInverseInertia());
			bodyB->AddAngularVelocity(Vector2::Cross(rb, impulse) * bodyB->GetInverseInertia());
		}
		std::vector<Vector2> friction_impulse_list;
		std::vector<int>     friction_ids;
		float friction_coef = std::sqrt(fixture_a->GetFriction() * fixture_b->GetFriction());

		for (int k = 0; k < (int)active_ids.size(); k++) {
			int i = active_ids[k];
			Vector2 ra = contact_list[i] - world_mass_center_a;
			Vector2 rb = contact_list[i] - world_mass_center_b;
			Vector2 ra_perp = { -ra.y(), ra.x() };
			Vector2 rb_perp = { -rb.y(), rb.x() };
			Vector2 angular_linear_velocity_a = ra_perp * bodyA->GetAngularVelocity();
			Vector2 angular_linear_velocity_b = rb_perp * bodyB->GetAngularVelocity();
			Vector2 relative_velocity = (bodyB->GetLinearVelocity() + angular_linear_velocity_b) - (bodyA->GetLinearVelocity() + angular_linear_velocity_a);

			Vector2 tangent = relative_velocity - Vector2::Dot(relative_velocity, normal) * normal;
			if (FlatMath::NearlyEqual(tangent, Vector2::Zero())) continue;
			tangent.Normalize();

			float ra_perp_dot_t = Vector2::Dot(ra_perp, tangent);
			float rb_perp_dot_t = Vector2::Dot(rb_perp, tangent);
			float denom = bodyA->GetInverseMass() + bodyB->GetInverseMass()
				+ (ra_perp_dot_t * ra_perp_dot_t) * bodyA->GetInverseInertia()
				+ (rb_perp_dot_t * rb_perp_dot_t) * bodyB->GetInverseInertia();
			if (denom <= 0.0f) continue;

			float jt = -Vector2::Dot(relative_velocity, tangent);
			std::cout << jt << std::endl;
			if (std::abs(jt) < 1e-6f) continue;
			jt /= denom;
			jt /= (float)contact_points.points_num;

			float j = j_list[k];
			Vector2 friction_impulse;
			if (std::abs(jt) <= std::abs(j) * friction_coef) {
				friction_impulse = jt * tangent;
			}
			else {
				friction_impulse = -j * tangent * friction_coef;
			}
			friction_impulse_list.push_back(friction_impulse);
			friction_ids.push_back(i);
		}

		for (int k = 0; k < (int)friction_impulse_list.size(); k++) {
			int i = friction_ids[k];
			Vector2& friction_impulse = friction_impulse_list[k];
			Vector2 ra = contact_list[i] - world_mass_center_a;
			Vector2 rb = contact_list[i] - world_mass_center_b;
			bodyA->AddLinearVelocity(-friction_impulse * bodyA->GetInverseMass());
			bodyB->AddLinearVelocity(friction_impulse * bodyB->GetInverseMass());
			bodyA->AddAngularVelocity(-Vector2::Cross(ra, friction_impulse) * bodyA->GetInverseInertia());
			bodyB->AddAngularVelocity(Vector2::Cross(rb, friction_impulse) * bodyB->GetInverseInertia());
		}
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
				ResolveCollisionWithRotationAndFriction(contact);
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
