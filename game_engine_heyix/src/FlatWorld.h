#pragma once
#include "Vector2.h"
#include "FlatBody.h"
#include <vector>
#include <unordered_map>
#include "FlatManifold.h"
namespace FlatPhysics {
	class FlatWorld {
	private:
		struct ContactPair {
			FlatFixture* fixture_a;
			FlatFixture* fixture_b;
		};
	public:
		FlatWorld() 
			:gravity({0,9.81})
		{}
	public:
		int GetBodyCount() { return bodies.size(); }
	public:
		void AddBody(FlatBody* body);
		bool RemoveBody(FlatBody* body);
		FlatBody* GetBody(int index);
		void Step(float time);
	public:
		void DrawContactPoints();

	private:
		void CollisionDetectionStep();
		bool ShouldCollide(FlatFixture* fixture_a, FlatFixture* fixture_b);
		void ResolveCollisionBasic(const FlatManifold& manifold);
		void ResolveCollisionWithRotation(const FlatManifold& manifold);
		void ResolveCollisionWithRotationAndFriction(const FlatManifold& manifold);
		void SeperateBodies(FlatBody* bodyA, FlatBody* bodyB, const Vector2& mtv);
		void BroadPhase();
		void NarrowPhase();
	private:
		Vector2 gravity;
		std::vector<FlatBody*> bodies;
		std::unordered_map<FlatBody*, int> index_map;
		std::vector<FlatManifold> contacts;
		std::vector<ContactPair> contact_pairs;
	};
}