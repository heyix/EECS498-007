#pragma once
#include "Vector2.h"
#include "FlatBody.h"
#include <vector>
#include <unordered_map>
#include "FlatManifold.h"
#include "FlatContact.h"
#include "IBroadPhase.h"
#include "FlatSolver.h"
namespace FlatPhysics {
	class FlatWorld {
	public:
		FlatWorld();
	public:
		int GetBodyCount() { return bodies.size(); }
	public:
		void AddBody(FlatBody* body);
		bool RemoveBody(FlatBody* body);
		FlatBody* GetBody(int index);
		void Step(float time);
	public:
		void DrawContactPoints();
		void SetBroadPhase(std::unique_ptr<IBroadPhase> bp);
		void SetSolver(std::unique_ptr<IContactSolver> solver);
	private:
		void CollisionDetectionStep(float dt);
		void SeperateBodies(FlatBody* bodyA, FlatBody* bodyB, const Vector2& mtv);
		void SynchronizeFixtures();
		void BroadPhase();
		void NarrowPhase();
	private:
		Vector2 gravity;
		std::vector<FlatBody*> bodies;
		std::unordered_map<FlatBody*, int> index_map;
		std::vector<FlatManifold> contacts;
		std::vector<ContactPair> contact_pairs;
		std::unique_ptr<IBroadPhase>    broadphase_;
		std::unique_ptr<IContactSolver> solver_;
		int velocity_iterations_{ 1 };
		int position_iterations_{ 1 };
	};
}