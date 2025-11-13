#include "FlatSolverNaive.h"

#include "FlatManifold.h"
#include "FlatFixture.h"
#include "FlatBody.h"
#include "FlatTransform.h"
#include "FlatMath.h"

#include <algorithm>
#include <cmath>

namespace FlatPhysics {

    namespace {
        constexpr float kPositionSlop = 0.005f;
        constexpr float kPositionPercent = 0.8f;
    } // namespace

    void FlatSolverNaive::Initialize(const std::vector<FlatManifold>& manifolds) {
        manifolds_ = &manifolds;
    }

    void FlatSolverNaive::PreSolve() {}

    void FlatSolverNaive::Solve(float /*dt*/, int iterations) {
        if (!manifolds_) {
            return;
        }

        const int iters = std::max(iterations, 1);
        for (int i = 0; i < iters; ++i) {
            for (const FlatManifold& manifold : *manifolds_) {
                SolveVelocityForManifold(manifold);
            }
        }
    }

    void FlatSolverNaive::SolveVelocityForManifold(const FlatManifold& manifold) const {
		FlatFixture* fixture_a = manifold.fixtureA;
		FlatFixture* fixture_b = manifold.fixtureB;
		const Vector2& normal = manifold.contact_points[0].normal;
		FlatBody* bodyA = fixture_a->GetBody();
		FlatBody* bodyB = fixture_b->GetBody();
		Vector2 world_mass_center_a = bodyA->GetMassCenterWorld();
		Vector2 world_mass_center_b = bodyB->GetMassCenterWorld();
		ContactPointsOld contact_points;
		if (manifold.contact_points.size() == 1) {
			contact_points.SetPoint(manifold.contact_points[0].end);
		}
		else {
			contact_points.SetPoints(manifold.contact_points[0].end, manifold.contact_points[1].end);
		}
		float e = std::min(fixture_a->GetRestitution(), fixture_b->GetRestitution());
		std::vector<Vector2> contact_list{ contact_points.point1, contact_points.point2 };

		std::vector<Vector2> impulse_list;
		std::vector<float>   j_list;
		std::vector<int>     active_ids;

		//normal impulse
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



		//friction impulse
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

    void FlatSolverNaive::PostSolve(float /*dt*/, int iterations) {
        if (!manifolds_) {
            return;
        }

        const int iters = std::max(iterations, 1);
        const float step_percent = kPositionPercent / static_cast<float>(iters);

        for (int i = 0; i < iters; ++i) {
            for (const FlatManifold& manifold : *manifolds_) {
                CorrectPositions(manifold, step_percent);
            }
        }
    }

    void FlatSolverNaive::CorrectPositions(const FlatManifold& manifold, float percent) const {
		auto* fixtureA = manifold.fixtureA;
		auto* fixtureB = manifold.fixtureB;
		if (!fixtureA || !fixtureB) {
			return;
		}

		FlatBody* bodyA = fixtureA->GetBody();
		FlatBody* bodyB = fixtureB->GetBody();
		if (!bodyA || !bodyB) {
			return;
		}

		const float invMassA = bodyA->is_static ? 0.0f : bodyA->GetInverseMass();
		const float invMassB = bodyB->is_static ? 0.0f : bodyB->GetInverseMass();
		const float invMassSum = invMassA + invMassB;
		if (invMassSum <= 0.0f) {
			return;
		}

		const int pointCount = static_cast<int>(manifold.contact_points.size());
		if (pointCount == 0) {
			return;
		}

		for (const ContactPoint& point : manifold.contact_points) {
			float bias = point.depth * percent / pointCount;
			if (bias <= 0.0f) continue;

			Vector2 correction = point.normal * (bias / invMassSum);
			if (invMassA > 0.0f) bodyA->Move(-correction * invMassA);
			if (invMassB > 0.0f) bodyB->Move(correction * invMassB);
		}

    }

    void FlatSolverNaive::StoreImpulses() {}

}


//void FlatWorld::ResolveCollisionBasic(const FlatManifold& manifold)
//{
//	FlatBody* bodyA = manifold.fixtureA->GetBody();
//	FlatBody* bodyB = manifold.fixtureB->GetBody();
//	float depth = manifold.depth;
//	const Vector2& normal = manifold.normal;
//
//	Vector2 relative_velocity = bodyB->GetLinearVelocity() - bodyA->GetLinearVelocity();
//	if (Vector2::Dot(relative_velocity, normal) > 0) {
//		return;
//	}
//	float e = std::min(bodyA->restitution, bodyB->restitution);
//	float j = -(1 + e) * Vector2::Dot(relative_velocity, normal);
//	j /= (bodyA->GetInverseMass() + bodyB->GetInverseMass());
//
//	Vector2 impulse = j * normal;
//	bodyA->AddLinearVelocity(-impulse * bodyA->GetInverseMass());
//	bodyB->AddLinearVelocity(impulse * bodyB->GetInverseMass());
//}
//void FlatWorld::ResolveCollisionWithRotation(const FlatManifold& manifold)
//{
//	FlatFixture* fixture_a = manifold.fixtureA;
//	FlatFixture* fixture_b = manifold.fixtureB;
//	float depth = manifold.depth;
//	const Vector2& normal = manifold.normal;
//	FlatBody* bodyA = fixture_a->GetBody();
//	FlatBody* bodyB = fixture_b->GetBody();
//	Vector2 world_mass_center_a = FlatTransform::TransformVector(bodyA->GetMassCenter(), bodyA->GetTransform());
//	Vector2 world_mass_center_b = FlatTransform::TransformVector(bodyB->GetMassCenter(), bodyB->GetTransform());
//	const ContactPoints& contact_points = manifold.contact_points;
//	float e = std::min(bodyA->restitution, bodyB->restitution);
//	std::vector<Vector2> contact_list{ contact_points.point1,contact_points.point2 };
//	std::vector<Vector2> impulse_list;
//	std::vector<Vector2> ra_list;
//	std::vector<Vector2> rb_list;
//	for (int i = 0; i < contact_points.points_num; i++) {
//		Vector2 ra = contact_list[i] - world_mass_center_a;
//		Vector2 rb = contact_list[i] - world_mass_center_b;
//		Vector2 ra_perp = { -ra.y(),ra.x() };
//		Vector2 rb_perp = { -rb.y(),rb.x() };
//		Vector2 angular_linear_velocity_a = ra_perp * bodyA->GetAngularVelocity();
//		Vector2 angular_linear_velocity_b = rb_perp * bodyB->GetAngularVelocity();
//		Vector2 relative_velocity = (bodyB->GetLinearVelocity() + angular_linear_velocity_b) - (bodyA->GetLinearVelocity() + angular_linear_velocity_a);
//
//		float contact_velocity_mag = Vector2::Dot(relative_velocity, normal);
//		if (contact_velocity_mag > 0) {
//			continue;
//		}
//		float ra_perp_dot_n = Vector2::Dot(ra_perp, normal);
//		float rb_perp_dot_n = Vector2::Dot(rb_perp, normal);
//		float denom = bodyA->GetInverseMass() + bodyB->GetInverseMass() + (ra_perp_dot_n * ra_perp_dot_n) * bodyA->GetInverseInertia() + (rb_perp_dot_n * rb_perp_dot_n) * bodyB->GetInverseInertia();
//		float j = -(1 + e) * contact_velocity_mag;
//		j /= denom;
//		j /= (float)contact_points.points_num;
//		Vector2 impulse = j * normal;
//		impulse_list.push_back(impulse);
//		ra_list.push_back(ra);
//		rb_list.push_back(rb);
//	}
//	for (int i = 0; i < impulse_list.size(); i++) {
//		Vector2& impulse = impulse_list[i];
//		Vector2& ra = ra_list[i];
//		Vector2& rb = rb_list[i];
//		bodyA->AddLinearVelocity(-impulse * bodyA->GetInverseMass());
//		bodyB->AddLinearVelocity(impulse * bodyB->GetInverseMass());
//		bodyA->AddAngularVelocity(-Vector2::Cross(ra, impulse) * bodyA->GetInverseInertia());
//		bodyB->AddAngularVelocity(Vector2::Cross(rb, impulse) * bodyB->GetInverseInertia());
//	}
//}
//void FlatWorld::ResolveCollisionWithRotationAndFriction(const FlatManifold& manifold)
//{
//	FlatFixture* fixture_a = manifold.fixtureA;
//	FlatFixture* fixture_b = manifold.fixtureB;
//	const Vector2& normal = manifold.normal;
//	FlatBody* bodyA = fixture_a->GetBody();
//	FlatBody* bodyB = fixture_b->GetBody();
//	Vector2 world_mass_center_a = FlatTransform::TransformVector(bodyA->GetMassCenter(), bodyA->GetTransform());
//	Vector2 world_mass_center_b = FlatTransform::TransformVector(bodyB->GetMassCenter(), bodyB->GetTransform());
//	const ContactPoints& contact_points = manifold.contact_points;
//	float e = std::min(bodyA->restitution, bodyB->restitution);
//	std::vector<Vector2> contact_list{ contact_points.point1, contact_points.point2 };
//
//	std::vector<Vector2> impulse_list;
//	std::vector<float>   j_list;
//	std::vector<int>     active_ids;
//
//	//normal impulse
//	for (int i = 0; i < contact_points.points_num; i++) {
//		Vector2 ra = contact_list[i] - world_mass_center_a;
//		Vector2 rb = contact_list[i] - world_mass_center_b;
//		Vector2 ra_perp = { -ra.y(), ra.x() };
//		Vector2 rb_perp = { -rb.y(), rb.x() };
//		Vector2 angular_linear_velocity_a = ra_perp * bodyA->GetAngularVelocity();
//		Vector2 angular_linear_velocity_b = rb_perp * bodyB->GetAngularVelocity();
//		Vector2 relative_velocity = (bodyB->GetLinearVelocity() + angular_linear_velocity_b) - (bodyA->GetLinearVelocity() + angular_linear_velocity_a);
//
//		float contact_velocity_mag = Vector2::Dot(relative_velocity, normal);
//		if (contact_velocity_mag > 0) {
//			continue;
//		}
//		float ra_perp_dot_n = Vector2::Dot(ra_perp, normal);
//		float rb_perp_dot_n = Vector2::Dot(rb_perp, normal);
//		float denom = bodyA->GetInverseMass() + bodyB->GetInverseMass()
//			+ (ra_perp_dot_n * ra_perp_dot_n) * bodyA->GetInverseInertia()
//			+ (rb_perp_dot_n * rb_perp_dot_n) * bodyB->GetInverseInertia();
//		if (denom <= 0.0f) continue;
//
//		float j = -(1 + e) * contact_velocity_mag;
//		j /= denom;
//		j /= (float)contact_points.points_num;
//		j_list.push_back(j);
//
//		impulse_list.push_back(j * normal);
//		active_ids.push_back(i);
//	}
//	for (int k = 0; k < (int)impulse_list.size(); k++) {
//		int i = active_ids[k];
//		Vector2& impulse = impulse_list[k];
//		Vector2 ra = contact_list[i] - world_mass_center_a;
//		Vector2 rb = contact_list[i] - world_mass_center_b;
//		bodyA->AddLinearVelocity(-impulse * bodyA->GetInverseMass());
//		bodyB->AddLinearVelocity(impulse * bodyB->GetInverseMass());
//		bodyA->AddAngularVelocity(-Vector2::Cross(ra, impulse) * bodyA->GetInverseInertia());
//		bodyB->AddAngularVelocity(Vector2::Cross(rb, impulse) * bodyB->GetInverseInertia());
//	}
//	std::vector<Vector2> friction_impulse_list;
//	std::vector<int>     friction_ids;
//	float friction_coef = std::sqrt(fixture_a->GetFriction() * fixture_b->GetFriction());
//
//
//
//	//friction impulse
//	for (int k = 0; k < (int)active_ids.size(); k++) {
//		int i = active_ids[k];
//		Vector2 ra = contact_list[i] - world_mass_center_a;
//		Vector2 rb = contact_list[i] - world_mass_center_b;
//		Vector2 ra_perp = { -ra.y(), ra.x() };
//		Vector2 rb_perp = { -rb.y(), rb.x() };
//		Vector2 angular_linear_velocity_a = ra_perp * bodyA->GetAngularVelocity();
//		Vector2 angular_linear_velocity_b = rb_perp * bodyB->GetAngularVelocity();
//		Vector2 relative_velocity = (bodyB->GetLinearVelocity() + angular_linear_velocity_b) - (bodyA->GetLinearVelocity() + angular_linear_velocity_a);
//
//		Vector2 tangent = relative_velocity - Vector2::Dot(relative_velocity, normal) * normal;
//		if (FlatMath::NearlyEqual(tangent, Vector2::Zero())) continue;
//		tangent.Normalize();
//
//		float ra_perp_dot_t = Vector2::Dot(ra_perp, tangent);
//		float rb_perp_dot_t = Vector2::Dot(rb_perp, tangent);
//		float denom = bodyA->GetInverseMass() + bodyB->GetInverseMass()
//			+ (ra_perp_dot_t * ra_perp_dot_t) * bodyA->GetInverseInertia()
//			+ (rb_perp_dot_t * rb_perp_dot_t) * bodyB->GetInverseInertia();
//		if (denom <= 0.0f) continue;
//
//		float jt = -Vector2::Dot(relative_velocity, tangent);
//		if (std::abs(jt) < 1e-6f) continue;
//		jt /= denom;
//		jt /= (float)contact_points.points_num;
//
//		float j = j_list[k];
//		Vector2 friction_impulse;
//		if (std::abs(jt) <= std::abs(j) * friction_coef) {
//			friction_impulse = jt * tangent;
//		}
//		else {
//			friction_impulse = -j * tangent * friction_coef;
//		}
//		friction_impulse_list.push_back(friction_impulse);
//		friction_ids.push_back(i);
//	}
//
//	for (int k = 0; k < (int)friction_impulse_list.size(); k++) {
//		int i = friction_ids[k];
//		Vector2& friction_impulse = friction_impulse_list[k];
//		Vector2 ra = contact_list[i] - world_mass_center_a;
//		Vector2 rb = contact_list[i] - world_mass_center_b;
//		bodyA->AddLinearVelocity(-friction_impulse * bodyA->GetInverseMass());
//		bodyB->AddLinearVelocity(friction_impulse * bodyB->GetInverseMass());
//		bodyA->AddAngularVelocity(-Vector2::Cross(ra, friction_impulse) * bodyA->GetInverseInertia());
//		bodyB->AddAngularVelocity(Vector2::Cross(rb, friction_impulse) * bodyB->GetInverseInertia());
//	}
//}