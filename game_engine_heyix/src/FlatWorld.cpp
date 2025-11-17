#include "FlatWorld.h"
#include "FlatFixture.h"
#include "Collision.h"
#include <algorithm>
#include "Renderer.h"
#include "Engine.h"
#include "FlatMath.h"
#include "BroadPhaseNaive.h"
#include "FlatSolverNaive.h"
#include "PenetrationConstraint.h"
#include "FlatSolverPGS.h"
#include "BroadPhaseQuadTree.h"
#include "FlatContact.h"
namespace FlatPhysics {
	namespace {
		constexpr float kLinearSleepTolerance = 0.1f;             
		constexpr float kLinearSleepToleranceSq = kLinearSleepTolerance * kLinearSleepTolerance;
		constexpr float kAngularSleepTolerance = FlatMath::DegToRad(4.0f);
		constexpr float kTimeToSleep = 0.5f;
		class BroadphasePairCollector : public FlatPhysics::IPairCallback {
		public:
			explicit BroadphasePairCollector(std::vector<ContactPair>& out) : pairs_(out) {}
			void AddPair(void* user_data_a, void* user_data_b) override {
				auto* fixtureA = static_cast<FlatFixture*>(user_data_a);
				auto* fixtureB = static_cast<FlatFixture*>(user_data_b);
				if (!fixtureA || !fixtureB) {
					return;
				}

				FlatBody* bodyA = fixtureA->GetBody();
				FlatBody* bodyB = fixtureB->GetBody();
				if (bodyA == bodyB) {
					return;
				}
				if (bodyA && bodyB && bodyA->IsStatic() && bodyB->IsStatic()) {
					return;
				}
				if (fixtureA && fixtureB) {
					pairs_.push_back({ fixtureA, fixtureB });
				}
			}
		private:
			std::vector<ContactPair>& pairs_;
		};
	}

	FlatWorld::FlatWorld()
		: gravity({ 0.0f, 9.81f })
	{
		SetBroadPhase(std::make_unique<BroadPhaseQuadTree>());
		SetSolver(std::make_unique<FlatSolverPGS>());
	}
	void FlatWorld::AddBody(FlatBody* body)
	{
		if (!body || index_map.count(body))return;
		const int index = static_cast<int>(bodies.size());
		bodies.push_back(body);
		index_map[body] = index;

		body->SetWorld(this);

		if (broadphase_) {
			for (const auto& fixture_uptr : body->GetFixtures()) {
				RegisterFixture(fixture_uptr.get());
			}
		}
	}

	bool FlatWorld::RemoveBody(FlatBody* body)
	{
		auto it = index_map.find(body);
		if (it == index_map.end()) {
			return false;
		}
		UnregisterAllFixtureContactEdges(body);
		if (broadphase_) {
			for (const auto& fixture_uptr : body->GetFixtures()) {
				UnregisterFixtureForBroadphase(fixture_uptr.get());
			}
		}

		const int index = it->second;
		const int last = static_cast<int>(bodies.size()) - 1;
		if (index != last) {
			std::swap(bodies[index], bodies[last]);
			index_map[bodies[index]] = index;
		}
		bodies.pop_back();
		index_map.erase(it);

		body->SetWorld(nullptr);
		return true;
	}

	FlatBody* FlatWorld::GetBody(int index)
	{
		if (index < 0 || index >= bodies.size()) {
			return nullptr;
		}
		return bodies[index];
	}
	void FlatWorld::RegisterFixture(FlatFixture* fixture)
	{
		if (!fixture || !broadphase_) {
			return;
		}

		FlatAABB aabb = fixture->GetAABB();
		ProxyID id = broadphase_->CreateProxy(aabb, fixture->GetBroadPhaseUserData());
		fixture->SetProxyID(id);
		fixture->SetLastAABB(aabb);
		fixture->ClearProxyDirty();
	}

	void FlatWorld::UnregisterFixtureForBroadphase(FlatFixture* fixture)
	{
		if (!fixture || !broadphase_) {
			return;
		}

		ProxyID id = fixture->GetProxyID();
		if (id != kNullProxy) {
			broadphase_->DestroyProxy(id);
			fixture->SetProxyID(kNullProxy);
		}
		fixture->ClearLastAABB();
		fixture->MarkProxyDirty();
	}
	void FlatWorld::UnregisterFixtureContactEdge(FlatFixture* fixture)
	{
		for (int i = static_cast<int>(contacts.size()) - 1; i >= 0; --i) {
			FlatManifold& m = contacts[i];
			if (m.fixtureA == fixture || m.fixtureB == fixture) {
				DestroyContactManifold(i);
			}
		}
	}
	void FlatWorld::UnregisterAllFixtureContactEdges(FlatBody* body)
	{
		while (FlatContactEdge* edge = body->contact_list_) {
			int index = edge->contact_index;
			DestroyContactManifold(index);
		}
	}
	void FlatWorld::UpdateSleeping(float dt)
	{
		for (FlatBody* body : bodies) {
			body->SetIslandFlag(false);
			body->SetIslandIndex(-1);
			if (body->IsStatic()) {
				body->SetAwake(false);
				body->SetSleepTime(0.0f);
			}
		}
		for (FlatManifold& m : contacts) {
			m.island_flag = false;
		}
		std::vector<FlatBody*> stack;
		stack.reserve(bodies.size());
		std::vector<FlatBody*> island_bodies;
		island_bodies.reserve(bodies.size());
		std::vector<FlatManifold*> island_contacts;
		island_contacts.reserve(contacts.size());
		int islandIndex = 0;
		for (FlatBody* seed : bodies) {
			if (seed->IsStatic()) {
				continue;
			}
			if (!seed->IsAwake()) {
				continue;
			}
			if (seed->GetIslandFlag()) {
				continue;
			}
			stack.clear();
			island_bodies.clear();
			island_contacts.clear();

			stack.push_back(seed);
			seed->SetIslandFlag(true);

			float min_sleep_time = std::numeric_limits<float>::max();
			bool island_can_sleep = true;

			while (!stack.empty()) {
				FlatBody* body = stack.back();
				stack.pop_back();
				island_bodies.push_back(body);
				body->SetIslandIndex(islandIndex);

				const bool can_body_sleep = body->GetCanSleep();
				const Vector2 v = body->GetLinearVelocity();
				const float w = body->GetAngularVelocity();
				const float lin2 = v.LengthSquared();
				const float ang = std::fabs(w);

				float sleep_time = body->GetSleepTime();

				if (!can_body_sleep || lin2 > kLinearSleepToleranceSq || ang > kAngularSleepTolerance) {
					sleep_time = 0.0f;
					body->SetSleepTime(0.0f);
					island_can_sleep = false;
					min_sleep_time = 0.0f;
				}
				else {
					sleep_time += dt;
					body->SetSleepTime(sleep_time);
					min_sleep_time = std::min(min_sleep_time, sleep_time);
				}
				for (FlatContactEdge* edge = body->GetContactList(); edge; edge = edge->next) {
					const int contact_index = edge->contact_index;
					FlatManifold& manifold = contacts[contact_index];
					if (manifold.island_flag) {
						continue;
					}

					manifold.island_flag = true;
					island_contacts.push_back(&manifold);
					FlatBody* other = edge->other;
					if (other->GetIslandFlag()) {
						continue;
					}
					if (other->IsStatic()) {
						continue;
					}
					other->SetIslandFlag(true);
					stack.push_back(other);
				}
			}
			if (island_can_sleep && min_sleep_time >= kTimeToSleep) {
				for (FlatBody* body : island_bodies) {
					body->SetAwake(false);
				}
			}
			else {
				for (FlatBody* body : island_bodies) {
					body->SetAwake(true);
				}
			}
			islandIndex++;
		}
	}
	std::uint64_t FlatWorld::MakeContactKey(const FlatFixture* a, const FlatFixture* b)
	{
		ProxyID idA = a->GetProxyID();
		ProxyID idB = b->GetProxyID();

		if (idA > idB) std::swap(idA, idB);

		std::uint32_t lo = static_cast<std::uint32_t>(idA);
		std::uint32_t hi = static_cast<std::uint32_t>(idB);

		return (static_cast<std::uint64_t>(hi) << 32) |
			static_cast<std::uint64_t>(lo);
	}
	void FlatWorld::AttachContactToBodies(int contactIndex, FlatManifold& manifold)
	{
		FlatBody* bodyA = manifold.fixtureA->GetBody();
		FlatBody* bodyB = manifold.fixtureB->GetBody();
		FlatContactEdge* edgeA = edge_pool_.Allocate();
		edgeA->other = bodyB;
		edgeA->contact_index = contactIndex;
		edgeA->prev = nullptr;
		edgeA->next = bodyA->contact_list_;
		if (bodyA->contact_list_) {
			bodyA->contact_list_->prev = edgeA;
		}
		bodyA->contact_list_ = edgeA;

		FlatContactEdge* edgeB = edge_pool_.Allocate();
		edgeB->other = bodyA;
		edgeB->contact_index = contactIndex;
		edgeB->prev = nullptr;
		edgeB->next = bodyB->contact_list_;
		if (bodyB->contact_list_) {
			bodyB->contact_list_->prev = edgeB;
		}
		bodyB->contact_list_ = edgeB;

		manifold.edgeA = edgeA;
		manifold.edgeB = edgeB;
	}
	void FlatWorld::DestroyContactManifold(int contact_index)
	{
		FlatManifold& manifold = contacts[contact_index];
		FlatBody* bodyA = manifold.fixtureA->GetBody();
		FlatBody* bodyB = manifold.fixtureB->GetBody();

		auto unlinkEdge = [](FlatBody* body, FlatContactEdge* edge) {
			if (!edge)return;
			if (edge->prev) {
				edge->prev->next = edge->next;
			}
			if (edge->next) {
				edge->next->prev = edge->prev;
			}
			if (body->GetContactList() == edge) {
				body->SetContactList(edge->next);
			}
		};
		unlinkEdge(bodyA, manifold.edgeA);
		unlinkEdge(bodyB, manifold.edgeB);
		edge_pool_.Free(manifold.edgeA);
		edge_pool_.Free(manifold.edgeB);
		manifold.edgeA = nullptr;
		manifold.edgeB = nullptr;

		std::uint64_t key = MakeContactKey(manifold.fixtureA, manifold.fixtureB);
		auto it = contact_map_.find(key);
		if (it != contact_map_.end()) {
			contact_map_.erase(it);
		}
		int last = static_cast<int>(contacts.size()) - 1;
		if (contact_index != last) {
			contacts[contact_index] = contacts[last];
			FlatFixture* movedA = contacts[contact_index].fixtureA;
			FlatFixture* movedB = contacts[contact_index].fixtureB;
			std::uint64_t movedKey = MakeContactKey(movedA, movedB);
			contact_map_[movedKey] = contact_index;
			if (contacts[contact_index].edgeA)contacts[contact_index].edgeA->contact_index = contact_index;
			if (contacts[contact_index].edgeB)contacts[contact_index].edgeB->contact_index = contact_index;
		}
		contacts.pop_back();
	}
	void FlatWorld::SynchronizeFixtures()
	{
		if (!broadphase_) {
			return;
		}

		for (FlatBody* body : bodies) {
			const auto& fixtures = body->GetFixtures();
			for (const auto& fixture_uptr : fixtures) {
				FlatFixture* fixture = fixture_uptr.get();
				ProxyID id = fixture->GetProxyID();

				if (id == kNullProxy) {
					RegisterFixture(fixture);
					continue;
				}

				if (!fixture->IsProxyDirty()) {
					continue;
				}

				FlatAABB new_aabb = fixture->GetAABB();
				Vector2 displacement = Vector2::Zero();

				if (fixture->HasLastAABB()) {
					FlatAABB old_aabb = fixture->GetLastAABB();
					Vector2 old_center{
						(old_aabb.min.x() + old_aabb.max.x()) * 0.5f,
						(old_aabb.min.y() + old_aabb.max.y()) * 0.5f
					};
					Vector2 new_center{
						(new_aabb.min.x() + new_aabb.max.x()) * 0.5f,
						(new_aabb.min.y() + new_aabb.max.y()) * 0.5f
					};
					displacement = new_center - old_center;
				}

				broadphase_->MoveProxy(id, new_aabb, displacement);
				fixture->SetLastAABB(new_aabb);
				fixture->ClearProxyDirty();
			}
		}
	}
	void FlatWorld::Step(float time)
	{
		/*for (FlatBody* body : bodies) {
			body->Step(time, gravity);
		}*/
		//int count = 0;
		//for (FlatBody* body : bodies) {
		//	if (!body->IsStatic()) {
		//		if (!body->IsAwake()) {
		//			count++;
		//		}
		//	}
		//}
		//std::cout << count<<std::endl;
		for (FlatBody* body : bodies) {
			body->IntegrateForces(time,gravity);
		}
		for (FlatBody* body : bodies) {
			body->ApplyDampling(time);
		}
		//std::chrono::steady_clock::time_point step_start;
		//step_start = std::chrono::steady_clock::now();
		BroadPhase();
		//if (bodies.size() % 100 == 0) {
		//	const double physics_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - step_start).count();
		//	std::cout << physics_ms << " ms" << std::endl;
		//}
		NarrowPhase();
		solver_->Initialize(contacts,constraints);
		solver_->PreSolve(time);
		solver_->Solve(time, 15);
		for (FlatBody* body : bodies) {
			body->IntegrateVelocities(time);
		}
		solver_->PostSolve(time, 2);
		UpdateSleeping(time);
		//CollisionDetectionStep(time);
	}
	void FlatWorld::AddConstraint(std::unique_ptr<FlatConstraint> constraint)
	{
		constraints.push_back(std::move(constraint));
	}
	std::vector<std::unique_ptr<FlatConstraint>>& FlatWorld::GetConstraints()
	{
		return constraints;
	}
	//void FlatWorld::CollisionDetectionStep(float dt)
	//{
	//	contacts.clear();
	//	contact_pairs.clear();

	//	BroadPhase();
	//	NarrowPhase();

	//	if (!contacts.empty() && solver_) {
	//		solver_->Initialize(contacts);
	//		solver_->PreSolve();
	//		solver_->Solve(dt, velocity_iterations_);
	//		solver_->PostSolve(dt, position_iterations_);
	//		solver_->StoreImpulses();
	//	}
	//}

	void FlatWorld::BroadPhase()
	{
		if (!broadphase_) {
			return;
		}
		contact_pairs.clear();
		SynchronizeFixtures();
		BroadphasePairCollector collector(contact_pairs);
		broadphase_->UpdatePairs(&collector);
	}
	void FlatWorld::NarrowPhase()
	{
		for (FlatManifold& m : contacts) {
			m.touched_this_step = false;
		}
		for (const ContactPair& pair : contact_pairs) {
			FlatFixture* fa = pair.fixture_a;
			FlatFixture* fb = pair.fixture_b;
			if (!fa || !fb) {
				continue;
			}
			FlatBody* bodyA = fa->GetBody();
			FlatBody* bodyB = fb->GetBody();
			FixedSizeContainer<ContactPoint, 2> contact_points;
			const bool touching = Collision::DetectCollision(fa, fb, contact_points);

			std::uint64_t key = MakeContactKey(fa, fb);
			auto it = contact_map_.find(key);
			const bool existed = (it != contact_map_.end());

			if (!touching) {
				if (existed) {
					int idx = it->second;
					DestroyContactManifold(idx);
				}
				continue;
			}

			if (!existed) {
				// New contact
				int index = static_cast<int>(contacts.size());
				contacts.emplace_back(fa, fb);
				FlatManifold& manifold = contacts.back();
				manifold.touched_this_step = true;
				manifold.contact_points = contact_points;
				manifold.is_new_contact = true;
				contact_map_[key] = index;
				AttachContactToBodies(index, manifold);
	
				if (!bodyA->IsStatic()) bodyA->SetAwake(true);
				if (!bodyB->IsStatic()) bodyB->SetAwake(true);
			}
			else {
				// Existing contact: update + warm start impulses, but don't wake
				FlatManifold& manifold = contacts[it->second];
				manifold.touched_this_step = true;
				manifold.is_new_contact = false;
				FixedSizeContainer<ContactPoint, 2> merged;
				for (ContactPoint& new_point : contact_points) {
					ContactPoint merged_point = new_point;
					merged_point.normal_impulse = 0.0f;
					merged_point.tangent_impulse = 0.0f;

					for (ContactPoint& old_point : manifold.contact_points) {
						if (old_point.id.key == new_point.id.key) {
							merged_point.normal_impulse = old_point.normal_impulse;
							merged_point.tangent_impulse = old_point.tangent_impulse;
							break;
						}
					}
					merged.Push_Back(merged_point);
				}
				manifold.contact_points = merged;
			}
		}

		for (int i = static_cast<int>(contacts.size()) - 1; i >= 0; --i) {
			if (!contacts[i].touched_this_step) {
				DestroyContactManifold(i);
			}
		}
	}
	void FlatWorld::DrawContactPoints()
	{
		for (FlatManifold& manifold : contacts) {
			for (ContactPoint& point : manifold.contact_points) {
				Vector2 contact_point = point.end; {
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
					queueMarker(contact_point);
				}
			}
		}
		
	}

	void FlatWorld::SetBroadPhase(std::unique_ptr<IBroadPhase> bp)
	{
		if (broadphase_) {
			for (FlatBody* body : bodies) {
				for (const auto& fixture_uptr : body->GetFixtures()) {
					FlatFixture* fixture = fixture_uptr.get();
					UnregisterFixtureForBroadphase(fixture);
				}
			}
		}

		broadphase_ = std::move(bp);
		SynchronizeFixtures();
	}

	void FlatWorld::SetSolver(std::unique_ptr<IFlatSolver> solver)
	{
		solver_ = std::move(solver);
	}

}
