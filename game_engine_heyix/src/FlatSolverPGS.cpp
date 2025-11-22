#include "FlatSolverPGS.h"
#include "FlatContact.h"
#include "FlatManifold.h"
#include "FlatFixture.h"
#include "FlatBody.h"

#include <algorithm>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace FlatPhysics {

    void FlatSolverPGS::Initialize(std::vector<FlatManifold>& manifolds,
        const std::vector<std::unique_ptr<FlatConstraint>>& constraints)
    {
        one_point_constraints_.clear();
        two_point_constraints_.clear();
        active_island_count_ = 0;

        int islandCount = 0;

        // --- find max island index from contact manifolds ---
        for (FlatManifold& m : manifolds)
        {
            if (!m.island_flag)
                continue;

            FlatFixture* fa = m.fixtureA;
            FlatFixture* fb = m.fixtureB;
            if (!fa || !fb) continue;

            FlatBody* bodyA = fa->GetBody();
            FlatBody* bodyB = fb->GetBody();
            if (!bodyA && !bodyB) continue;

            int idx = GetIslandIndex(bodyA, bodyB);
            if (idx >= 0) {
                islandCount = std::max(islandCount, idx + 1);
            }
        }

        // --- also include joint/other constraints ---
        for (const std::unique_ptr<FlatConstraint>& uptr : constraints)
        {
            FlatConstraint* c = uptr.get();
            if (!c || !c->a || !c->b) continue;

            FlatBody* bodyA = c->a->GetBody();
            FlatBody* bodyB = c->b->GetBody();

            int idx = GetIslandIndex(bodyA, bodyB);
            if (idx >= 0) {
                islandCount = std::max(islandCount, idx + 1);
            }
        }

        if (islandCount == 0) {
            return;
        }

        // grow island storage if needed (don’t shrink)
        if (islandCount > static_cast<int>(islands_.size())) {
            islands_.resize((islandCount * 3) / 2);
        }
        active_island_count_ = islandCount;

        for (int i = 0; i < active_island_count_; ++i) {
            islands_[i].Clear();
        }

        // reserve contact constraint arrays
        if (manifolds.size() > one_point_constraints_.capacity()) {
            one_point_constraints_.reserve((manifolds.size() * 3) / 2);
        }
        if (manifolds.size() > two_point_constraints_.capacity()) {
            two_point_constraints_.reserve((manifolds.size() * 3) / 2);
        }

        // --- build penetration constraints per island ---
        for (FlatManifold& manifold : manifolds)
        {
            if (!manifold.island_flag)
                continue;

            FlatFixture* fa = manifold.fixtureA;
            FlatFixture* fb = manifold.fixtureB;
            if (!fa || !fb) continue;

            FlatBody* bodyA = fa->GetBody();
            FlatBody* bodyB = fb->GetBody();
            if (!bodyA && !bodyB) continue;

            int islandIndex = GetIslandIndex(bodyA, bodyB);
            if (islandIndex < 0 || islandIndex >= active_island_count_)
                continue;

            const int count = manifold.contact_points.Size();
            if (count == 0)
                continue;

            const bool is_new = manifold.is_new_contact;

            IslandConstraints& island = islands_[islandIndex];

            if (count == 1)
            {
                ContactPoint& cp = manifold.contact_points[0];

                Vector2 start = cp.start;
                Vector2 end = cp.end;

                if (fa->GetShapeType() == ShapeType::Circle &&
                    fb->GetShapeType() == ShapeType::Polygon)
                {
                    std::swap(start, end);
                }

                one_point_constraints_.emplace_back(
                    fa, fb,
                    start, end,
                    cp.normal,
                    &cp.normal_impulse,
                    &cp.tangent_impulse,
                    is_new
                );

                PenetrationConstraintBase* base = &one_point_constraints_.back();
                island.penetration_constraints.push_back(base);

                IslandConstraints::ConstraintRef ref;
                ref.type = IslandConstraints::ConstraintRef::Type::Penetration;
                ref.penetration = base;
                ref.other = nullptr;
                island.all_constraints.push_back(ref);
            }
            else
            {
                ContactPoint& cp0 = manifold.contact_points[0];
                ContactPoint& cp1 = manifold.contact_points[1];

                Vector2 p0a = cp0.start;
                Vector2 p0b = cp0.end;
                Vector2 p1a = cp1.start;
                Vector2 p1b = cp1.end;

                if (fa->GetShapeType() == ShapeType::Circle &&
                    fb->GetShapeType() == ShapeType::Polygon)
                {
                    std::swap(p0a, p0b);
                    std::swap(p1a, p1b);
                }

                two_point_constraints_.emplace_back(
                    fa, fb,
                    p0a, p0b,
                    p1a, p1b,
                    cp0.normal,
                    &cp0.normal_impulse,
                    &cp0.tangent_impulse,
                    &cp1.normal_impulse,
                    &cp1.tangent_impulse,
                    is_new
                );

                PenetrationConstraintBase* base = &two_point_constraints_.back();
                island.penetration_constraints.push_back(base);

                IslandConstraints::ConstraintRef ref;
                ref.type = IslandConstraints::ConstraintRef::Type::Penetration;
                ref.penetration = base;
                ref.other = nullptr;
                island.all_constraints.push_back(ref);
            }
        }

        // --- add joint/other constraints per island ---
        for (const std::unique_ptr<FlatConstraint>& uptr : constraints)
        {
            FlatConstraint* c = uptr.get();
            if (!c || !c->a || !c->b) continue;

            FlatBody* bodyA = c->a->GetBody();
            FlatBody* bodyB = c->b->GetBody();

            int islandIndex = GetIslandIndex(bodyA, bodyB);
            if (islandIndex >= 0 && islandIndex < active_island_count_)
            {
                IslandConstraints& island = islands_[islandIndex];
                island.constraints.push_back(c);

                IslandConstraints::ConstraintRef ref;
                ref.type = IslandConstraints::ConstraintRef::Type::Other;
                ref.penetration = nullptr;
                ref.other = c;
                island.all_constraints.push_back(ref);
            }
        }

        // --- build graph coloring for each island ---
        for (int i = 0; i < active_island_count_; ++i) {
            BuildColoringForIslands(islands_[i]);
        }
    }

    void FlatSolverPGS::PreSolve(float dt)
    {
        if (active_island_count_ == 0) return;

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
        for (int i = 0; i < active_island_count_; ++i)
        {
            IslandConstraints& island = islands_[i];

            for (FlatConstraint* c : island.constraints)
            {
                if (CanFixtureCollide(c->a, c->b)) {
                    c->PreSolve(dt);
                }
            }

            for (PenetrationConstraintBase* pc : island.penetration_constraints)
            {
                if (CanFixtureCollide(pc->a, pc->b)) {
                    pc->PreSolve(dt);
                }
            }
        }
    }

    void FlatSolverPGS::Solve(float dt, int iterations)
    {
        if (active_island_count_ == 0) return;

        for (int iter = 0; iter < iterations; ++iter)
        {
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < active_island_count_; ++i)
            {
                IslandConstraints& island = islands_[i];

                // TODO: later you can use island.color_groups here
                // for intra-island parallelization.

                for (FlatConstraint* c : island.constraints)
                {
                    if (CanFixtureCollide(c->a, c->b)) {
                        c->Solve();
                    }
                }

                for (PenetrationConstraintBase* pc : island.penetration_constraints)
                {
                    if (CanFixtureCollide(pc->a, pc->b)) {
                        pc->Solve();
                    }
                }
            }
        }
    }

    void FlatSolverPGS::PostSolve(float dt, int iterations)
    {
        if (active_island_count_ == 0) return;

        for (int iter = 0; iter < iterations; ++iter)
        {
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int i = 0; i < active_island_count_; ++i)
            {
                IslandConstraints& island = islands_[i];

                for (FlatConstraint* c : island.constraints)
                {
                    if (CanFixtureCollide(c->a, c->b)) {
                        c->PostSolve();
                    }
                }

                for (PenetrationConstraintBase* pc : island.penetration_constraints)
                {
                    if (CanFixtureCollide(pc->a, pc->b)) {
                        pc->PostSolve();
                    }
                }
            }
        }
    }

    bool FlatSolverPGS::CanFixtureCollide(FlatFixture* fixtureA, FlatFixture* fixtureB)
    {
        FlatBody* bodyA = fixtureA->GetBody();
        FlatBody* bodyB = fixtureB->GetBody();

        if (bodyA->IsStatic() && bodyB->IsStatic()) {
            return false;
        }
        if (!bodyA->IsAwake() && !bodyB->IsAwake()) {
            return false;
        }
        return true;
    }

    int FlatSolverPGS::GetIslandIndex(FlatBody* bodyA, FlatBody* bodyB) const
    {
        int idxA = bodyA ? bodyA->GetIslandIndex() : -1;
        int idxB = bodyB ? bodyB->GetIslandIndex() : -1;

        if (idxA >= 0 && idxB >= 0) {
            return idxA;             // both dynamic & awake, same island
        }
        if (idxA >= 0) return idxA;
        if (idxB >= 0) return idxB;
        return -1;
    }

    void FlatSolverPGS::BuildColoringForIslands(IslandConstraints& island)
    {
        auto& all = island.all_constraints;
        const int n = static_cast<int>(all.size());
        if (n == 0) return;

        // ---- collect unique bodies & assign solver_temp_index ----
        auto& bodies = island.bodies;
        bodies.clear();

        for (int i = 0; i < n; ++i)
        {
            const auto& cr = all[i];
            FlatFixture* fa = cr.GetFixtureA();
            FlatFixture* fb = cr.GetFixtureB();

            if (fa) {
                FlatBody* b = fa->GetBody();
                if (b->GetSolverTempIndex() < 0) {
                    b->SetSolverTempIndex(static_cast<int>(bodies.size()));
                    bodies.push_back(b);
                }
            }

            if (fb) {
                FlatBody* b = fb->GetBody();
                if (b->GetSolverTempIndex() < 0) {
                    b->SetSolverTempIndex(static_cast<int>(bodies.size()));
                    bodies.push_back(b);
                }
            }
        }

        const int bodyCount = static_cast<int>(bodies.size());

        // ---- build per-body incident constraint lists ----
        auto& body_constraints = island.body_constraints;
        if (static_cast<int>(body_constraints.size()) < bodyCount) {
            body_constraints.resize(bodyCount); // grow outer vector if needed
        }
        for (int i = 0; i < bodyCount; ++i) {
            body_constraints[i].clear();       // keep capacity per inner vector
        }

        for (int i = 0; i < n; ++i)
        {
            const auto& cr = all[i];
            FlatFixture* fa = cr.GetFixtureA();
            FlatFixture* fb = cr.GetFixtureB();

            if (fa) {
                FlatBody* bodyA = fa->GetBody();
                int idxA = bodyA->GetSolverTempIndex();
                if (idxA >= 0) {
                    body_constraints[idxA].push_back(i);
                }
            }

            if (fb) {
                FlatBody* bodyB = fb->GetBody();
                int idxB = bodyB->GetSolverTempIndex();
                if (idxB >= 0) {
                    if (!fa || bodyB != fa->GetBody()) {
                        body_constraints[idxB].push_back(i);
                    }
                }
            }
        }

        // ---- prepare colors & used arrays ----
        auto& colors = island.colors;
        if (static_cast<int>(colors.capacity()) < n) {
            int newCap = colors.capacity() == 0 ? n : static_cast<int>(colors.capacity());
            while (newCap < n) newCap *= 2;
            colors.reserve(newCap);
        }
        colors.assign(n, -1);

        auto& used = island.used;
        int usedNeeded = n + 1;
        if (static_cast<int>(used.capacity()) < usedNeeded) {
            int newCap = used.capacity() == 0 ? usedNeeded : static_cast<int>(used.capacity());
            while (newCap < usedNeeded) newCap *= 2;
            used.reserve(newCap);
        }
        used.assign(usedNeeded, false);

        int maxColor = -1;

        // ---- greedy coloring ----
        for (int ci = 0; ci < n; ++ci)
        {
            if (maxColor >= 0) {
                std::fill(used.begin(), used.begin() + maxColor + 1, false);
            }

            FlatFixture* fa = all[ci].GetFixtureA();
            FlatFixture* fb = all[ci].GetFixtureB();

            auto mark = [&](FlatFixture* f) {
                if (!f) return;
                FlatBody* b = f->GetBody();
                int bi = b->GetSolverTempIndex();
                if (bi < 0) return;
                const auto& inc = body_constraints[bi];
                for (int idx : inc) {
                    int c = colors[idx];
                    if (c >= 0) used[c] = true;
                }
                };

            mark(fa);
            mark(fb);

            int c = 0;
            while (c <= maxColor && used[c]) ++c;

            colors[ci] = c;
            if (c > maxColor) maxColor = c;
        }

        // ---- build color groups ----
        auto& groups = island.color_groups;
        int groupCount = maxColor + 1;
        if (static_cast<int>(groups.capacity()) < groupCount) {
            int newCap = groups.capacity() == 0 ? groupCount : static_cast<int>(groups.capacity());
            while (newCap < groupCount) newCap *= 2;
            groups.reserve(newCap);
        }
        if (static_cast<int>(groups.size()) < groupCount) {
            groups.resize(groupCount);
        }
        for (int c = 0; c < groupCount; ++c) {
            groups[c].clear();
        }

        for (int i = 0; i < n; ++i) {
            int c = colors[i];
            groups[c].push_back(i);
        }

        // ---- reset solver_temp_index for bodies ----
        for (FlatBody* b : bodies) {
            b->SetSolverTempIndex(-1);
        }
    }

} // namespace FlatPhysics
