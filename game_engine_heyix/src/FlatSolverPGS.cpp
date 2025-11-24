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

        int island_count = 0;

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
                island_count = std::max(island_count, idx + 1);
            }
        }

        for (const std::unique_ptr<FlatConstraint>& uptr : constraints)
        {
            FlatConstraint* c = uptr.get();
            if (!c || !c->a || !c->b) continue;

            FlatBody* bodyA = c->a->GetBody();
            FlatBody* bodyB = c->b->GetBody();

            int idx = GetIslandIndex(bodyA, bodyB);
            if (idx >= 0) {
                island_count = std::max(island_count, idx + 1);
            }
        }

        if (island_count == 0) {
            return;
        }

        if (island_count > static_cast<int>(islands_.size())) {
            islands_.resize((island_count * 3) / 2);
        }
        active_island_count_ = island_count;

#ifdef _OPENMP
#pragma omp parallel for
#endif
        for (int i = 0; i < active_island_count_; ++i) {
            islands_[i].Clear();
        }

        if (manifolds.size() > one_point_constraints_.capacity()) {
            one_point_constraints_.reserve((manifolds.size() * 3) / 2);
        }
        if (manifolds.size() > two_point_constraints_.capacity()) {
            two_point_constraints_.reserve((manifolds.size() * 3) / 2);
        }

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
    }



    void FlatSolverPGS::PreSolve(float dt)
    {
        if (active_island_count_ == 0) return;
        HandleGraphColoring();
        if (!enable_intra_island_parallel_)
        {
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic,4)
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
            return;
        }

        const int maxColor = global_max_color_;
        for (int color = 0; color < maxColor; ++color)
        {
            auto& work = per_color_work_[color];
            const int workCount = static_cast<int>(work.size());
            if (workCount == 0) continue;

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
            for (int wi = 0; wi < workCount; ++wi)
            {
                ConstraintWorkItem& item = work[wi];
                IslandConstraints* island = item.island;
                IslandConstraints::ConstraintRef& cr =
                    island->all_constraints[item.constraint_index];

                if (cr.type == IslandConstraints::ConstraintRef::Type::Other)
                {
                    FlatConstraint* c = cr.other;
                    if (c && CanFixtureCollide(c->a, c->b)) {
                        c->PreSolve(dt);
                    }
                }
                else
                {
                    PenetrationConstraintBase* pc = cr.penetration;
                    if (pc && CanFixtureCollide(pc->a, pc->b)) {
                        pc->PreSolve(dt);
                    }
                }
            }
        }
    }

    void FlatSolverPGS::Solve(float dt, int iterations)
    {
        if (active_island_count_ == 0) return;

        if (!enable_intra_island_parallel_)
        {
            for (int iter = 0; iter < iterations; ++iter)
            {
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic,4)
#endif
                for (int i = 0; i < active_island_count_; ++i)
                {
                    IslandConstraints& island = islands_[i];

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
            return;
        }

        const int maxColor = global_max_color_;

        for (int iter = 0; iter < iterations; ++iter)
        {
            for (int color = 0; color < maxColor; ++color)
            {
                auto& work = per_color_work_[color];
                const int workCount = static_cast<int>(work.size());
                if (workCount == 0) continue;

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
                for (int wi = 0; wi < workCount; ++wi)
                {
                    ConstraintWorkItem& item = work[wi];
                    IslandConstraints* island = item.island;
                    IslandConstraints::ConstraintRef& cr =
                        island->all_constraints[item.constraint_index];

                    if (cr.type == IslandConstraints::ConstraintRef::Type::Other)
                    {
                        FlatConstraint* c = cr.other;
                        if (c && CanFixtureCollide(c->a, c->b)) {
                            c->Solve();
                        }
                    }
                    else
                    {
                        PenetrationConstraintBase* pc = cr.penetration;
                        if (pc && CanFixtureCollide(pc->a, pc->b)) {
                            pc->Solve();
                        }
                    }
                }
            }
        }
    }
//    void FlatSolverPGS::Solve(float dt, int iterations)
//    {
//        if (active_island_count_ == 0) return;
//
//        // Fallback: original inter-island parallelization
//        if (!enable_intra_island_parallel_)
//        {
//            for (int iter = 0; iter < iterations; ++iter)
//            {
//#ifdef _OPENMP
//#pragma omp parallel for schedule(dynamic,8)
//#endif
//                for (int i = 0; i < active_island_count_; ++i)
//                {
//                    IslandConstraints& island = islands_[i];
//
//                    for (FlatConstraint* c : island.constraints)
//                    {
//                        if (CanFixtureCollide(c->a, c->b)) {
//                            c->Solve();
//                        }
//                    }
//
//                    for (PenetrationConstraintBase* pc : island.penetration_constraints)
//                    {
//                        if (CanFixtureCollide(pc->a, pc->b)) {
//                            pc->Solve();
//                        }
//                    }
//                }
//            }
//            return;
//        }
//
//        // --- New path: ignore GS color sweep order ---
//        const int workCount = static_cast<int>(color_work_items_.size());
//        if (workCount == 0) return;
//
//        for (int iter = 0; iter < iterations; ++iter)
//        {
//#ifdef _OPENMP
//#pragma omp parallel for schedule(dynamic,8)
//#endif
//            for (int wi = 0; wi < workCount; ++wi)
//            {
//                const ColorWorkItem& item = color_work_items_[wi];
//                SolveColorGroup(item);
//            }
//        }
//    }
    void FlatSolverPGS::PostSolve(float dt, int iterations)
    {
        if (active_island_count_ == 0) return;

        if (!enable_intra_island_parallel_)
        {
            for (int iter = 0; iter < iterations; ++iter)
            {
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic,4)
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
            return;
        }

        const int maxColor = global_max_color_;

        for (int iter = 0; iter < iterations; ++iter)
        {
            for (int color = 0; color < maxColor; ++color)
            {
                auto& work = per_color_work_[color];
                const int workCount = static_cast<int>(work.size());
                if (workCount == 0) continue;

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
                for (int wi = 0; wi < workCount; ++wi)
                {
                    ConstraintWorkItem& item = work[wi];
                    IslandConstraints* island = item.island;
                    IslandConstraints::ConstraintRef& cr =
                        island->all_constraints[item.constraint_index];

                    if (cr.type == IslandConstraints::ConstraintRef::Type::Other)
                    {
                        FlatConstraint* c = cr.other;
                        if (c && CanFixtureCollide(c->a, c->b)) {
                            c->PostSolve();
                        }
                    }
                    else
                    {
                        PenetrationConstraintBase* pc = cr.penetration;
                        if (pc && CanFixtureCollide(pc->a, pc->b)) {
                            pc->PostSolve();
                        }
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
            return idxA;
        }
        if (idxA >= 0) return idxA;
        if (idxB >= 0) return idxB;
        return -1;
    }

    void FlatSolverPGS::BuildColoringForIslands(IslandConstraints& island)
    {
        std::vector<IslandConstraints::ConstraintRef>& all_constraints = island.all_constraints;
        const int constraint_num = static_cast<int>(all_constraints.size());
        if (constraint_num == 0) return;

        std::vector<FlatBody*>& bodies = island.bodies;
        bodies.clear();

        for (int i = 0; i < constraint_num; ++i)
        {
            const auto& cr = all_constraints[i];
            FlatFixture* fa = cr.GetFixtureA();
            FlatFixture* fb = cr.GetFixtureB();

            if (fa) {
                FlatBody* b = fa->GetBody();
                if (b && !b->IsStatic() && b->GetSolverTempIndex() < 0) {
                    b->SetSolverTempIndex(static_cast<int>(bodies.size()));
                    bodies.push_back(b);
                }
            }

            if (fb) {
                FlatBody* b = fb->GetBody();
                if (b && !b->IsStatic() && b->GetSolverTempIndex() < 0) {
                    b->SetSolverTempIndex(static_cast<int>(bodies.size()));
                    bodies.push_back(b);
                }
            }
        }

        const int bodyCount = static_cast<int>(bodies.size());

        std::vector<std::vector<int>>& body_constraints = island.body_constraints;
        if ((int)body_constraints.capacity() < bodyCount) {
            body_constraints.reserve(bodyCount * 2);
        }
        body_constraints.resize(bodyCount);
        for (int i = 0; i < bodyCount; ++i) {
            body_constraints[i].clear();
        }

        for (int i = 0; i < constraint_num; ++i)
        {
            const IslandConstraints::ConstraintRef& cr = all_constraints[i];
            FlatFixture* fa = cr.GetFixtureA();
            FlatFixture* fb = cr.GetFixtureB();

            if (fa) {
                FlatBody* bodyA = fa->GetBody();
                if (bodyA && !bodyA->IsStatic()) {
                    int idxA = bodyA->GetSolverTempIndex();
                    if (idxA >= 0 && idxA < bodyCount) {
                        body_constraints[idxA].push_back(i);
                    }
                }
            }

            if (fb) {
                FlatBody* bodyB = fb->GetBody();
                if (bodyB && !bodyB->IsStatic()) {
                    int idxB = bodyB->GetSolverTempIndex();
                    if (idxB >= 0 && idxB < bodyCount) {
                        if (!fa || bodyB != fa->GetBody()) {
                            body_constraints[idxB].push_back(i);
                        }
                    }
                }
            }
        }

        std::vector<int>& constraint_to_color = island.constraint_to_color;
        if (static_cast<int>(constraint_to_color.capacity()) < constraint_num) {
            constraint_to_color.reserve(constraint_num * 2);
        }
        constraint_to_color.assign(constraint_num, -1);

        auto& color_used_mark = island.color_used_mark;
        int used_needed = constraint_num + 1;
        if (static_cast<int>(color_used_mark.capacity()) < used_needed) {
            color_used_mark.reserve(used_needed * 2);
        }
        color_used_mark.assign(used_needed, 0);

        int max_color = -1;
        int current_mark = 1;
        for (int ci = 0; ci < constraint_num; ++ci)
        {
            current_mark++;
            FlatFixture* fa = all_constraints[ci].GetFixtureA();
            FlatFixture* fb = all_constraints[ci].GetFixtureB();

            auto mark = [&](FlatFixture* f) {
                if (!f) return;
                FlatBody* b = f->GetBody();
                if (!b || b->IsStatic()) return;
                int bi = b->GetSolverTempIndex();
                if (bi < 0 || bi >= bodyCount) return;

                const auto& constraint_indexes = body_constraints[bi];
                for (int constraint_index : constraint_indexes) {
                    int color = constraint_to_color[constraint_index];
                    if (color >= 0) color_used_mark[color] = current_mark;
                }
                };

            mark(fa);
            mark(fb);

            int color = 0;
            while (color <= max_color && color_used_mark[color] == current_mark) {
                ++color;
            }

            constraint_to_color[ci] = color;
            if (color > max_color) max_color = color;
        }

        auto& color_to_constraints = island.color_to_constraints;
        int   color_count = max_color + 1;
        if (static_cast<int>(color_to_constraints.capacity()) < color_count) {
            color_to_constraints.reserve(color_count * 2);
        }
        color_to_constraints.resize(color_count);
        for (int c = 0; c < color_count; ++c) {
            color_to_constraints[c].clear();
        }

        for (int i = 0; i < constraint_num; ++i) {
            int color = constraint_to_color[i];
            color_to_constraints[color].push_back(i);
        }

        for (FlatBody* b : bodies) {
            b->SetSolverTempIndex(-1);
        }
        island.max_color_plus_one = color_count;
    }

    void FlatSolverPGS::HandleGraphColoring()
    {
        if (!enable_intra_island_parallel_)
        {
            global_max_color_ = 0;
            return;
        }

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 4)
#endif
        for (int i = 0; i < active_island_count_; ++i) {
            BuildColoringForIslands(islands_[i]);
        }

        global_max_color_ = 0;
        int local_max_color = 0;
#ifdef _OPENMP
#pragma omp parallel
        {
            int thread_max = 0;

#pragma omp for nowait
            for (int i = 0; i < active_island_count_; ++i)
            {
                int mc = islands_[i].max_color_plus_one;
                if (mc > thread_max)
                    thread_max = mc;
            }

#pragma omp critical
            {
                if (thread_max > local_max_color)
                    local_max_color = thread_max;
            }
        }
#else
        for (int i = 0; i < active_island_count_; ++i)
        {
            int mc = islands_[i].max_color_plus_one;
            if (mc > local_max_color)
                local_max_color = mc;
        }
#endif

        global_max_color_ = local_max_color;

        if (global_max_color_ == 0) {
            return;
        }

        const int needed = global_max_color_;

        int current_capacity = static_cast<int>(per_color_work_.capacity());
        if (current_capacity < needed)
        {
            per_color_work_.reserve(needed * 2);
        }
        per_color_work_.resize(needed);

        for (int c = 0; c < global_max_color_; ++c) {
            per_color_work_[c].clear();
        }

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
        for (int color = 0; color < global_max_color_; ++color)
        {
            auto& color_work = per_color_work_[color];
            color_work.clear();

            for (int islandIndex = 0; islandIndex < active_island_count_; ++islandIndex)
            {
                IslandConstraints& island = islands_[islandIndex];
                if (color >= island.max_color_plus_one) continue;

                const auto& color_to_constraints = island.color_to_constraints[color];
                if (color_to_constraints.empty()) continue;

                std::size_t needed_inner = color_work.size() + color_to_constraints.size();
                if (color_work.capacity() < needed_inner)
                {
                    color_work.reserve(needed_inner * 2);
                }

                for (int idx : color_to_constraints)
                {
                    color_work.push_back(ConstraintWorkItem{ &island, idx });
                }
            }
        }
    }

}
