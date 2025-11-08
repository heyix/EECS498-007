#pragma once
#include "Vector2.h"
#include "FlatBody.h"
#include <vector>
#include <unordered_map>
#include "FlatManifold.h"
namespace FlatPhysics {
	class FlatWorld {
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
		void ResolveCollision(const FlatManifold& manifold);
	private:
		Vector2 gravity;
		std::vector<FlatBody*> bodies;
		std::unordered_map<FlatBody*, int> index_map;
		std::vector<FlatManifold> contacts;
	};
}