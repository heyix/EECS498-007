#include "FlatWorld.h"
#include "FlatFixture.h"
#include "Collision.h"
#include <algorithm>
#include "Renderer.h"
#include "Engine.h"
#include "FlatMath.h"
#include "BroadPhaseNaive.h"
#include "FlatSolverNaive.h"
#include "FlatSolverPGS.h"
#include "BroadPhaseQuadTree.h"
#include "FlatContact.h"
#include "FlatHelper.h"
#include <omp.h>
namespace FlatPhysics {
	namespace {
		constexpr float kLinearSleepTolerance = 0.1f;             
		constexpr float kLinearSleepToleranceSq = kLinearSleepTolerance * kLinearSleepTolerance;
		constexpr float kAngularSleepTolerance = FlatMath::DegToRad(4.0f);
		constexpr float kTimeToSleep = 0.5f;
	}
	class FlatWorld::BroadPhasePairCollector : public FlatPhysics::IPairCallback {
	public:
		BroadPhasePairCollector(int max_threads) :per_thread_pairs_(max_threads) {}
		void AddPair(void* user_data_a, void* user_data_b) override {
			int tid = 0;
#ifdef _OPENMP
			if (omp_in_parallel()) {
				tid = omp_get_thread_num();
			}
#endif
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
			std::vector<ContactPair>& vec = per_thread_pairs_[tid];
			vec.push_back({ fixtureA, fixtureB });
		}

		void FlattenInfo(std::vector<ContactPair>& out) {
			const int n = static_cast<int>(per_thread_pairs_.size());
			std::size_t total = 0;
			if (n == 0) {
				out.clear();
				total = 0;
				return;
			}

			std::vector<std::size_t> sizes(n);
			for (int i = 0; i < n; ++i) {
				sizes[i] = per_thread_pairs_[i].size();
			}

			std::vector<std::size_t> offsets(n + 1);
			offsets[0] = 0;
			for (int i = 0; i < n; ++i) {
				offsets[i + 1] = offsets[i] + sizes[i];
			}

			total = offsets[n];

			out.clear();
			out.resize(total);

#pragma omp parallel for schedule(static)
			for (int i = 0; i < n; ++i) {
				const auto& src = per_thread_pairs_[i];
				if (src.empty()) continue;

				const std::size_t offset = offsets[i];
				std::copy(src.begin(), src.end(), out.begin() + offset);
			}
		}
		void Clear() {
			for (auto& v : per_thread_pairs_) {
				v.clear();
			}
		}
	private:
		std::vector<std::vector<ContactPair>> per_thread_pairs_;
	};


	FlatWorld::FlatWorld()
		: gravity({ 0.0f, 9.81f })
	{
		SetBroadPhase(std::make_unique<BroadPhaseQuadTree>());
		SetSolver(std::make_unique<FlatSolverPGS>());
		int max_threads = 1;
#ifdef _OPENMP
		max_threads = omp_get_max_threads();
#endif
		collector_ = std::make_unique<BroadPhasePairCollector>(max_threads);
	}
	FlatWorld::~FlatWorld()
	{
	}
	void FlatWorld::AddBody(FlatBody* body)
	{
		if (!body || index_map.count(body))return;
		const int index = static_cast<int>(bodies.size());
		index_map[body] = index;

		body->SetWorld(this);

		if (broadphase_) {
			for (const auto& fixture_uptr : body->GetFixtures()) {
				RegisterFixture(fixture_uptr.get());
			}
		}
	}

	bool FlatWorld::DestroyBody(FlatBody* body)
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
			index_map[bodies[index].get()] = index;
		}
		bodies.pop_back();
		index_map.erase(it);

		body->SetWorld(nullptr);
		return true;
	}

	FlatBody* FlatWorld::CreateBody(const BodyDef& def)
	{
		std::unique_ptr<FlatBody> body(
			new FlatBody(
				def.position,
				def.angle_rad,
				def.linear_damping,
				def.angular_damping,
				def.is_static
			)
		);

		FlatBody* body_ptr = body.get();
		body_ptr->SetGravityScale(def.gravity_scale);
		body_ptr->SetCanSleep(def.allow_sleep);
		body_ptr->SetAwake(def.awake);
		bodies.push_back(std::move(body));
		AddBody(body_ptr);
		return body_ptr;
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
		int islandCount = 0;
		for (std::unique_ptr<FlatBody>& body : bodies)
		{
			if (body->IsStatic())
				continue;

			const int idx = body->GetIslandIndex();
			if (idx >= 0)
			{
				islandCount = std::max(islandCount, idx + 1);
			}
		}

		if (islandCount == 0)
		{
			return;
		}

		std::vector<float> minSleepTime(islandCount, std::numeric_limits<float>::max());
		std::vector<bool>  islandCanSleep(islandCount, true);

		for (std::unique_ptr<FlatBody>& body : bodies)
		{
			if (body->IsStatic())
				continue;

			const int idx = body->GetIslandIndex();
			if (idx < 0 || idx >= islandCount)
			{
				continue;
			}

			const bool   canSleep = body->GetCanSleep();
			const Vector2 v = body->GetLinearVelocity();
			const float   w = body->GetAngularVelocity();
			const float   lin2 = v.LengthSquared();
			const float   ang = std::fabs(w);

			float sleep_time = body->GetSleepTime();

			bool eligible =
				canSleep &&
				lin2 <= kLinearSleepToleranceSq &&
				ang <= kAngularSleepTolerance;

			if (!eligible)
			{
				sleep_time = 0.0f;
				body->SetSleepTime(0.0f);

				islandCanSleep[idx] = false;
				minSleepTime[idx] = 0.0f;
			}
			else
			{
				sleep_time += dt;
				body->SetSleepTime(sleep_time);

				if (sleep_time < minSleepTime[idx])
					minSleepTime[idx] = sleep_time;
			}
		}

		for (std::unique_ptr<FlatBody>& body : bodies)
		{
			if (body->IsStatic())
				continue;

			const int idx = body->GetIslandIndex();
			if (idx < 0 || idx >= islandCount)
			{
				continue;
			}

			if (islandCanSleep[idx] && minSleepTime[idx] >= kTimeToSleep)
			{
				body->SetAwake(false);
			}
			else
			{
				body->SetAwake(true);
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
	//idx = -1 and flag = false only if static or sleep island
	//has index and flag only if directly or indirectly connected to a awaken body
	void FlatWorld::BuildIslands()
	{
		for (std::unique_ptr<FlatBody>& body : bodies)
		{
			body->SetIslandFlag(false);
			body->SetIslandIndex(-1);

			if (body->IsStatic())
			{
				body->SetAwake(false);
				body->SetSleepTime(0.0f);
			}
		}

		for (FlatManifold& m : contacts)
		{
			m.island_flag = false;
		}
		std::vector<FlatBody*> stack;
		stack.reserve(bodies.size());

		int islandIndex = 0;

		for (std::unique_ptr<FlatBody>& seed : bodies)
		{
			if (seed->IsStatic())
				continue;
			if (!seed->IsAwake())
				continue;
			if (seed->GetIslandFlag())
				continue;

			stack.clear();
			stack.push_back(seed.get());
			seed->SetIslandFlag(true);

			while (!stack.empty())
			{
				FlatBody* body = stack.back();
				stack.pop_back();

				body->SetIslandIndex(islandIndex);

				for (FlatContactEdge* edge = body->GetContactList(); edge; edge = edge->next)
				{
					const int contact_index = edge->contact_index;
					FlatManifold& manifold = contacts[contact_index];

					if (!manifold.touched_this_step)
						continue;

					if (!manifold.island_flag)
					{
						manifold.island_flag = true;
					}

					FlatBody* other = edge->other;
					if (other->IsStatic())
						continue;
					if (other->GetIslandFlag())
						continue;

					other->SetIslandFlag(true);
					stack.push_back(other);
				}
			}

			++islandIndex;
		}
	}
	void FlatWorld::SynchronizeFixtures()
	{
		if (!broadphase_) {
			return;
		}

		for (std::unique_ptr<FlatBody>& body : bodies) {
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
		//for (std::unique_ptr<FlatBody>& body : bodies) {
		//	if (!body->IsStatic()) {
		//		if (!body->IsAwake()) {
		//			count++;
		//		}
		//	}
		//}
		//std::cout << count<<std::endl;
		for (std::unique_ptr<FlatBody>& body : bodies) {
			body->IntegrateForces(time,gravity);
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
		BuildIslands();
		//MeasureTime("solver initialize", [this]() {
		 	solver_->Initialize(contacts, constraints);
		//});
		MeasureTime("island", [this,time]() {
			solver_->PreSolve(time);
			solver_->Solve(time, 15);
		});
		for (std::unique_ptr<FlatBody>& body : bodies) {
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

	void FlatWorld::BroadPhase()
	{
		if (!broadphase_) {
			return;
		}
		contact_pairs.clear();
		SynchronizeFixtures();

		//MeasureTime("Broadphase", [this]() {
			this->broadphase_->UpdatePairs(this->collector_.get());		
			this->collector_->FlattenInfo(contact_pairs);
		//});
		this->collector_->Clear();
	}



	void FlatWorld::NarrowPhase()
	{
		for (FlatManifold& m : contacts) {
			m.touched_this_step = false;
		}

		const int pairCount = static_cast<int>(contact_pairs.size());

		if (pairCount == 0) {
			for (int i = static_cast<int>(contacts.size()) - 1; i >= 0; --i) {
				DestroyContactManifold(i);
			}
			return;
		}

		np_results.resize(pairCount);
		//MeasureTime("NarrowPhase", [this,pairCount]() {
#pragma omp parallel for schedule(dynamic,64)
			for (int i = 0; i < pairCount; ++i) {
				const ContactPair& pair = contact_pairs[i];
				FlatFixture* fa = pair.fixture_a;
				FlatFixture* fb = pair.fixture_b;

				NarrowPhaseResult& r = np_results[i];
				r.fa = fa;
				r.fb = fb;
				r.touching = false;

				if (!fa || !fb) {
					continue;
				}

				FlatBody* bodyA = fa->GetBody();
				FlatBody* bodyB = fb->GetBody();

				if (!bodyA->IsAwake() && !bodyB->IsAwake()) {
					continue;
				}

				FixedSizeContainer<ContactPoint, 2> local_points;
				const bool touching = Collision::DetectCollision(fa, fb, local_points);
				r.touching = touching;

				if (touching) {
					r.contact_points = local_points;
				}
			}
		//});


		for (int i = 0; i < pairCount; ++i) {
			NarrowPhaseResult& r = np_results[i];
			FlatFixture* fa = r.fa;
			FlatFixture* fb = r.fb;
			if (!fa || !fb) {
				continue;
			}

			FlatBody* bodyA = fa->GetBody();
			FlatBody* bodyB = fb->GetBody();

			std::uint64_t key = MakeContactKey(fa, fb);
			auto it = contact_map_.find(key);
			const bool existed = (it != contact_map_.end());

			const bool bothSleeping = !bodyA->IsAwake() && !bodyB->IsAwake();

			// --- SPECIAL CASE: both sleeping ---
			// we skipped collision, so r.touching is false,
			// but if there was an old contact we want to keep it.
			if (bothSleeping) {
				if (existed) {
					FlatManifold& manifold = contacts[it->second];
					manifold.touched_this_step = true;
					manifold.is_new_contact = false;
				}
				// if not existed, we just don't create one while they sleep
				continue;
			}
			if (!r.touching) {
				if (existed) {
					const int idx = it->second;
					DestroyContactManifold(idx);
				}
				continue;
			}

			if (!existed) {
				const int index = static_cast<int>(contacts.size());
				contacts.emplace_back(fa, fb);
				FlatManifold& manifold = contacts.back();

				manifold.touched_this_step = true;
				manifold.is_new_contact = true;
				manifold.contact_points = r.contact_points;

				contact_map_[key] = index;
				AttachContactToBodies(index, manifold);

				if (!bodyA->IsStatic()) bodyA->SetAwake(true);
				if (!bodyB->IsStatic()) bodyB->SetAwake(true);
			}
			else {
				FlatManifold& manifold = contacts[it->second];
				manifold.touched_this_step = true;
				manifold.is_new_contact = false;

				FixedSizeContainer<ContactPoint, 2> merged;
				for (ContactPoint& new_point : r.contact_points) {
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
					constexpr float kMarkerHalfSize = 0.03f;
					static const std::vector<Vector2> markerVerts = {
						{ -kMarkerHalfSize, -kMarkerHalfSize },
						{  kMarkerHalfSize, -kMarkerHalfSize },
						{  kMarkerHalfSize,  kMarkerHalfSize },
						{ -kMarkerHalfSize,  kMarkerHalfSize }
					};

					auto queueMarker = [&](const Vector2& p) {
						Engine::instance->renderer->draw_polygon(markerVerts, p, 0.0f, 255, 0, 0, 255, false,1);
						};
					queueMarker(contact_point);
				}
			}
		}
		
	}

	void FlatWorld::SetBroadPhase(std::unique_ptr<IBroadPhase> bp)
	{
		if (broadphase_) {
			for (std::unique_ptr<FlatBody>& body : bodies) {
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
