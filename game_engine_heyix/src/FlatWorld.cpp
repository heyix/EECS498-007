#include "FlatWorld.h"
#include "FlatFixture.h"
#include "Collision.h"
#include <algorithm>
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
			bodies[i]->Step(time);
		}
		CollisionDetectionStep();

	}
	void FlatWorld::CollisionDetectionStep()
	{
		for (int i = 0; i < (int)bodies.size()-1; i++) {
			FlatBody* bodyA = bodies[i];
			const std::vector<std::unique_ptr<FlatFixture>>& fixturesA = bodyA->GetFixtures();
			for (int j = i + 1; j < bodies.size(); j++) {
				FlatBody* bodyB = bodies[j];
				const std::vector<std::unique_ptr<FlatFixture>>& fixturesB = bodyB->GetFixtures();

				for (const std::unique_ptr<FlatFixture>& pa : fixturesA) {
					for (const std::unique_ptr<FlatFixture>& pb : fixturesB) {
						FlatFixture* fa = pa.get();
						FlatFixture* fb = pb.get();
						if (!ShouldCollide(bodyA, bodyB, fa, fb)) {
							continue;
						}
						Vector2 normal;
						float depth;
						if (DetectCollision(fa, fb, &normal, &depth)) {
							bodyA->Move(-normal * (depth / 2.0f));
							bodyB->Move(normal * (depth / 2.0f));
							ResolveCollision(bodyA, bodyB, normal, depth);
							contacts.push_back({ fa,fb,normal,depth });
						}
					}
				}
			}
		}
	}
	bool FlatWorld::DetectCollision(FlatFixture* fa, FlatFixture* fb, Vector2* normal, float* depth)
	{
        auto bodyA = fa->GetBody();
        auto bodyB = fb->GetBody();

        const auto& transformA = bodyA->GetTransform();
        const auto& transformB = bodyB->GetTransform();

        switch (fa->GetShapeType()) {
        case ShapeType::Circle: {
            auto* ca = fa->GetShape().AsCircle();
            Vector2 cA = FlatTransform::TransformVector(ca->center, transformA);
            float rA = ca->radius;

            switch (fb->GetShapeType()) {
            case ShapeType::Circle: {
                auto* cb = fb->GetShape().AsCircle();
                Vector2 cB = FlatTransform::TransformVector(cb->center, transformB);
                float rB = cb->radius;
                return Collision::IntersectCircles(cA, rA, cB, rB, normal, depth);
            }
            case ShapeType::Polygon: {
                const auto& vertsB_local = fb->GetShape().AsPolygon()->vertices;
                auto vertsB = FlatTransform::TransformVectors(vertsB_local, transformB);
                return Collision::IntersectCirclePolygon(cA, rA, vertsB, normal, depth);
            }
            default: break;
            }
            break;
        }

        case ShapeType::Polygon: {
            const auto& vertsA_local = fa->GetShape().AsPolygon()->vertices;
            auto vertsA = FlatTransform::TransformVectors(vertsA_local, transformA);

            switch (fb->GetShapeType()) {
            case ShapeType::Polygon: {
                const auto& vertsB_local = fb->GetShape().AsPolygon()->vertices;
                auto vertsB = FlatTransform::TransformVectors(vertsB_local, transformB);
                return Collision::IntersectPolygons(vertsA, vertsB, normal, depth);
            }
            case ShapeType::Circle: {
                auto* cb = fb->GetShape().AsCircle();
                Vector2 cB = FlatTransform::TransformVector(cb->center, transformB);
                float rB = cb->radius;
                bool hit = Collision::IntersectCirclePolygon(cB, rB, vertsA, normal, depth);
                if (hit && normal) *normal = -*normal;
                return hit;
            }
            default: break;
            }
            break;
        }
        default: break;
        }

        return false;

	}
	bool FlatWorld::ShouldCollide(FlatBody* a, FlatBody* b, FlatFixture* fixture_a, FlatFixture* fixture_b)
	{
		if (a == b) {
			return false;
		}
		return true;
	}
	void FlatWorld::ResolveCollision(FlatBody* bodyA, FlatBody* bodyB, Vector2 normal, float depth)
	{
		float e = std::min(bodyA->restitution, bodyB->restitution);
		Vector2 relative_velocity = bodyB->GetLinearVelocity() - bodyA->GetLinearVelocity();
		float j = -(1 + e) * Vector2::Dot(relative_velocity, normal);
		j /= (1 / bodyA->mass + 1 / bodyB->mass);
		Vector2 velocity_a = bodyA->GetLinearVelocity();
		velocity_a -= j / bodyA->mass * normal;
		bodyA->SetLinearVelocity(velocity_a);
		Vector2 velocity_b = bodyB->GetLinearVelocity();
		velocity_b += j / bodyB->mass * normal;
		bodyB->SetLinearVelocity(velocity_b);
	}
}
