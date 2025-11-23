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
#include <memory>
namespace FlatPhysics {
	class FlatWorld {
	private:
		struct NarrowPhaseResult {
			FlatFixture* fa = nullptr;
			FlatFixture* fb = nullptr;
			bool touching = false;
			FixedSizeContainer<ContactPoint, 2> contact_points;
		};
	public:
		FlatWorld();
		~FlatWorld();
		friend class FlatBody;
	public:
		int GetBodyCount() { return bodies.size(); }
	public:
		bool DestroyBody(FlatBody* body);
		FlatBody* CreateBody(const BodyDef& def);
		void Step(float time);

		void AddConstraint(std::unique_ptr<FlatConstraint> constraint);
		std::vector<std::unique_ptr<FlatConstraint>>& GetConstraints();
	public:
		void SetBroadPhase(std::unique_ptr<IBroadPhase> bp);
		void SetSolver(std::unique_ptr<IFlatSolver> solver);
		const std::vector<FlatManifold>& GetContactPoints() const { return contacts; }
	private:
		void AddBody(FlatBody* body);
		void SynchronizeFixtures();
		void BroadPhase();
		void NarrowPhase();

		void RegisterFixture(FlatFixture* fixture);
		void UnregisterFixtureForBroadphase(FlatFixture* fixture);
		void UnregisterFixtureContactEdge(FlatFixture* fixture);
		void UnregisterAllFixtureContactEdges(FlatBody* body);
		void UpdateSleeping(float dt);
		std::uint64_t MakeContactKey(const FlatFixture* a, const FlatFixture* b);

		void AttachContactToBodies(int contactIndex, FlatManifold& manifold);
		void DestroyContactManifold(int contact_index);

		void BuildIslands();
	private:
		Vector2 gravity;
		std::vector<std::unique_ptr<FlatBody>> bodies;
		std::unordered_map<FlatBody*, int> index_map;
		std::vector<FlatManifold> contacts;
		std::vector<ContactPair> contact_pairs;
		std::unique_ptr<IBroadPhase>    broadphase_;
		std::unique_ptr<IFlatSolver> solver_;
		std::vector<std::unique_ptr<FlatConstraint>> constraints;
		std::unordered_map<std::uint64_t, int> contact_map_;
		int velocity_iterations_{ 1 };
		int position_iterations_{ 1 };
		ContactEdgePool edge_pool_;

		class BroadPhasePairCollector;
		std::unique_ptr<BroadPhasePairCollector> collector_;
		std::vector<NarrowPhaseResult> np_results;
	};
}