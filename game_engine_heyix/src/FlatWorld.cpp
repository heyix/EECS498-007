#include "FlatWorld.h"
#include "FlatFixture.h"
#include "Collision.h"
#include <algorithm>
#include "Renderer.h"
#include "Engine.h"
#include "FlatMath.h"
#include "BroadPhaseNaive.h"
#include "FlatSolverNaive.h"

namespace FlatPhysics {
	namespace {
		class BroadphasePairCollector : public FlatPhysics::IPairCallback {
		public:
			explicit BroadphasePairCollector(std::vector<ContactPair>& out) : pairs_(out) {}
			void AddPair(void* user_data_a, void* user_data_b) override {
				auto* fa = static_cast<FlatFixture*>(user_data_a);
				auto* fb = static_cast<FlatFixture*>(user_data_b);
				if (fa && fb) {
					pairs_.push_back({ fa, fb });
				}
			}
		private:
			std::vector<ContactPair>& pairs_;
		};
	}

	FlatWorld::FlatWorld()
		: gravity({ 0.0f, 9.81f })
	{
		SetBroadPhase(std::make_unique<BroadphaseNaive>());
		SetSolver(std::make_unique<FlatSolverNaive>());
	}
	void FlatWorld::AddBody(FlatBody* body)
	{
		if (!body || index_map.count(body))return;
		int index = bodies.size();
		bodies.push_back(body);
		index_map[body] = index;
		if (broadphase_) {
			for (const auto& fixture_uptr : body->GetFixtures()) {
				FlatFixture* fixture = fixture_uptr.get();
				FlatAABB aabb = fixture->GetAABB();
				ProxyID id = broadphase_->CreateProxy(aabb, fixture);
				fixture->SetProxyID(id);
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
				FlatFixture* fixture = fixture_uptr.get();
				ProxyID id = fixture->GetProxyID();
				if (id != kNullProxy) {
					broadphase_->DestroyProxy(id);
					fixture->SetProxyID(kNullProxy);
				}
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
		return true;
	}

	FlatBody* FlatWorld::GetBody(int index)
	{
		if (index < 0 || index >= bodies.size()) {
			return nullptr;
		}
		return bodies[index];
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
				FlatAABB aabb = fixture->GetAABB();
				ProxyID id = fixture->GetProxyID();

				if (id == kNullProxy) {
					ProxyID new_id = broadphase_->CreateProxy(aabb, fixture);
					fixture->SetProxyID(new_id);
				}
				else {
					broadphase_->MoveProxy(id, aabb, Vector2::Zero());
				}
			}
		}
	}
	void FlatWorld::Step(float time)
	{
		for (FlatBody* body : bodies) {
			body->Step(time, gravity);
		}

		SynchronizeFixtures();
		CollisionDetectionStep(time);

	}
	void FlatWorld::CollisionDetectionStep(float dt)
	{
		contacts.clear();
		contact_pairs.clear();

		BroadPhase();
		NarrowPhase();

		if (!contacts.empty() && solver_) {
			solver_->Initialize(contacts);
			solver_->WarmStart();
			solver_->SolveVelocity(dt, velocity_iterations_);
			solver_->SolvePosition(dt, position_iterations_);
			solver_->StoreImpulses();
		}
	}


	void FlatWorld::SeperateBodies(FlatBody* bodyA, FlatBody* bodyB, const Vector2& mtv)
	{
		if (!bodyA->is_static && !bodyB->is_static) {
			bodyA->Move(-mtv * 0.5f);
			bodyB->Move(mtv * 0.5f);
		}
		else if (!bodyA->is_static && bodyB->is_static) {
			bodyA->Move(-mtv);
		}
		else if (bodyA->is_static && !bodyB->is_static) {
			bodyB->Move(mtv);
		}
	}
	void FlatWorld::BroadPhase()
	{
		if (!broadphase_) {
			return;
		}
		BroadphasePairCollector collector(contact_pairs);
		broadphase_->UpdatePairs(&collector);
	}
	void FlatWorld::NarrowPhase()
	{
		for (const ContactPair& pair : contact_pairs) {
			FlatFixture* fa = pair.fixture_a;
			FlatFixture* fb = pair.fixture_b;
			if (!fa || !fb || fa->GetBody() == fb->GetBody()) {
				continue;
			}

			Vector2 normal;
			float depth;
			if (Collision::DetectCollision(fa, fb, &normal, &depth)) {
				ContactPoints contact_points = Collision::FindContactPoints(fa, fb);
				contacts.emplace_back(fa, fb, normal, depth, contact_points);
			}
		}
	}
	void FlatWorld::DrawContactPoints()
	{
		for (FlatManifold& manifold : contacts) {
			ContactPoints& contact_points = manifold.contact_points;
			if (contact_points.points_num > 0) {
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

				queueMarker(contact_points.point1);
				if (contact_points.points_num > 1) {
					queueMarker(contact_points.point2);
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

	void FlatWorld::SetSolver(std::unique_ptr<IContactSolver> solver)
	{
		solver_ = std::move(solver);
	}

}
