#pragma once
#include "Vector2.h"
#include "FlatBody.h"
#include <vector>
#include <unordered_map>
#include "FlatManifold.h"
#include "FlatContact.h"
#include "IBroadPhase.h"
#include "FlatSolver.h"
#include "FlatConstraint.h"
namespace FlatPhysics {
	class FlatWorld {
	public:
		FlatWorld();
		friend class FlatBody;
	public:
		int GetBodyCount() { return bodies.size(); }
	public:
		void AddBody(FlatBody* body);
		bool RemoveBody(FlatBody* body);
		FlatBody* GetBody(int index);
		void Step(float time);

		void AddConstraint(std::unique_ptr<FlatConstraint> constraint);
		std::vector<std::unique_ptr<FlatConstraint>>& GetConstraints();
	public:
		void DrawContactPoints();
		void SetBroadPhase(std::unique_ptr<IBroadPhase> bp);
		void SetSolver(std::unique_ptr<IFlatSolver> solver);


	private:
		void CollisionDetectionStep(float dt);
		void SynchronizeFixtures();
		void BroadPhase();
		void NarrowPhase();

		void RegisterFixture(FlatFixture* fixture);
		void UnregisterFixture(FlatFixture* fixture);
	private:
		Vector2 gravity;
		std::vector<FlatBody*> bodies;
		std::unordered_map<FlatBody*, int> index_map;
		std::vector<FlatManifold> contacts;
		std::vector<ContactPair> contact_pairs;
		std::unique_ptr<IBroadPhase>    broadphase_;
		std::unique_ptr<IFlatSolver> solver_;
		std::vector<std::unique_ptr<FlatConstraint>> constraints;
		int velocity_iterations_{ 1 };
		int position_iterations_{ 1 };
	};
}