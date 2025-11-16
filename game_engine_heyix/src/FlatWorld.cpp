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
namespace FlatPhysics {
	namespace {
		constexpr float kLinearSleepTolerance = 0.05f;             
		constexpr float kLinearSleepToleranceSq = kLinearSleepTolerance * kLinearSleepTolerance;
		constexpr float kAngularSleepTolerance = FlatMath::DegToRad(2.0f);
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

		if (broadphase_) {
			for (const auto& fixture_uptr : body->GetFixtures()) {
				UnregisterFixture(fixture_uptr.get());
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

	void FlatWorld::UnregisterFixture(FlatFixture* fixture)
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
	void FlatWorld::UpdateSleeping(float dt)
	{
		//need to change this to sleep island
		for (FlatBody* body : bodies) {
			if (body->IsStatic()) {
				continue;
			}

			if (!body->GetCanSleep()) {
				body->SetSleepTime(0.0f);
				body->SetAwake(true);
				continue;
			}

			if (!body->IsAwake()) {
				body->SetSleepTime(0.0f);
				continue;
			}

			const Vector2 v = body->GetLinearVelocity();
			const float w = body->GetAngularVelocity();

			const float lin2 = v.LengthSquared();
			const float ang = std::fabs(w);

			if (lin2 > kLinearSleepToleranceSq || ang > kAngularSleepTolerance) {
				body->SetSleepTime(0.0f);
				continue;
			}

			float t = body->GetSleepTime() + dt;
			body->SetSleepTime(t);

			if (t >= kTimeToSleep) {
				body->SetAwake(false);
			}
		}
		//brute force for now, need to build island
		for (FlatManifold& manifold:contacts) {
			FlatBody* bodyA = manifold.fixtureA->GetBody();
			FlatBody* bodyB = manifold.fixtureB->GetBody();
			if (!bodyA->IsStatic() && !bodyA->IsAwake()) {
				bodyB->SetAwake(false);
			}
			if (!bodyB->IsStatic() && !bodyB->IsAwake()) {
				bodyA->SetAwake(false);
			}
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
		for (FlatBody* body : bodies) {
			body->IntegrateForces(time,gravity);
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
		solver_->Solve(time, 20);
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
		for (FlatManifold& m : contacts) {
			m.touched_this_step = false;
		}
		contact_pairs.clear();
		SynchronizeFixtures();
		BroadphasePairCollector collector(contact_pairs);
		broadphase_->UpdatePairs(&collector);
	}
	void FlatWorld::NarrowPhase()
	{
		//for (const ContactPair& pair : contact_pairs) {
		//	FlatFixture* fa = pair.fixture_a;
		//	FlatFixture* fb = pair.fixture_b;
		//	if (!fa || !fb || fa->GetBody() == fb->GetBody()) {
		//		continue;
		//	}
		//	std::vector<ContactPoint> contact_points;
		//	if (Collision::DetectCollision(fa, fb, contact_points)) {
		//		contacts.push_back({ fa, fb, contact_points });

		//		//ContactPointsOld contact_points = Collision::FindContactPointsOld(fa, fb);
		//	}
		//}
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
					contact_map_.erase(it);
					int last = static_cast<int>(contacts.size()) - 1;
					if (idx != last) {
						contacts[idx] = contacts[last];
						FlatFixture* movedA = contacts[idx].fixtureA;
						FlatFixture* movedB = contacts[idx].fixtureB;
						std::uint64_t moved_key = MakeContactKey(movedA, movedB);
						contact_map_[moved_key] = idx;
					}
					contacts.pop_back();
				}
				continue;
			}
			FlatManifold* manifold = nullptr;
			if (!existed) {
				int index = contacts.size();
				contacts.emplace_back(fa, fb);//touched_this_step default false
				contact_map_[key] = index;
				manifold = &contacts.back();
				if (!bodyA->IsStatic()) bodyA->SetAwake(true);
				if (!bodyB->IsStatic()) bodyB->SetAwake(true);
			}
			else {
				manifold = &contacts[it->second];
			}
			manifold->touched_this_step = true;
			manifold->contact_points = contact_points;
		}
		for (int i = static_cast<int>(contacts.size()) - 1; i >= 0; --i) {
			if (!contacts[i].touched_this_step) {
				FlatFixture* fa = contacts[i].fixtureA;
				FlatFixture* fb = contacts[i].fixtureB;
				std::uint64_t key = MakeContactKey(fa, fb);

				contact_map_.erase(key);

				int last = static_cast<int>(contacts.size()) - 1;
				if (i != last) {
					contacts[i] = contacts[last];

					FlatFixture* movedA = contacts[i].fixtureA;
					FlatFixture* movedB = contacts[i].fixtureB;
					std::uint64_t movedKey = MakeContactKey(movedA, movedB);
					contact_map_[movedKey] = i;
				}
				contacts.pop_back();
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
					ProxyID id = fixture->GetProxyID();
					if (id != kNullProxy) {
						broadphase_->DestroyProxy(id);
						fixture->SetProxyID(kNullProxy);
					}
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
