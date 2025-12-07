// TestSceneMPI.cpp
#include "TestSceneMPI.h"
#include <random>
#include <cmath>
#include <algorithm>

namespace FlatPhysics {
    static const float skew_power = 1.2f;
    // ----- basic single-body helpers (geometry identical to your original) -----

    FlatBody* CreateStaticBoxBodyMPI(
        FlatWorld& world,
        const Vector2& center,
        float half_w,
        float half_h,
        float friction,
        float restitution)
    {
        BodyDef bdef;
        bdef.position = center;
        bdef.angle_rad = 0.0f;
        bdef.is_static = true;
        bdef.linear_damping = 0.0f;
        bdef.angular_damping = 0.0f;
        bdef.gravity_scale = 0.0f;
        bdef.allow_sleep = true;
        bdef.awake = false;

        FlatBody* body = world.CreateBody(bdef);

        PolygonShape box;
        // IMPORTANT: exactly like your original: SetAsBox takes full width/height.
        box.SetAsBox(half_w * 2, half_h * 2);

        FixtureDef fdef;
        fdef.shape = &box;
        fdef.density = 1.0f;
        fdef.friction = friction;
        fdef.restitution = restitution;
        fdef.is_trigger = false;

        body->CreateFixture(fdef);
        return body;
    }

    FlatBody* CreateDynamicBodyMPI(
        FlatWorld& world,
        const Vector2& pos,
        bool use_box,
        float half_w,
        float half_h,
        float radius,
        float density,
        float friction,
        float restitution)
    {
        BodyDef bdef;
        bdef.position = pos;
        bdef.angle_rad = 0.0f;
        bdef.is_static = false;
        bdef.linear_damping = 0.0f;
        bdef.angular_damping = 0.0f;
        bdef.gravity_scale = 1.0f;
        bdef.allow_sleep = true;
        bdef.awake = true;

        FlatBody* body = world.CreateBody(bdef);

        FixtureDef fdef;
        fdef.density = density;
        fdef.friction = friction;
        fdef.restitution = restitution;
        fdef.is_trigger = false;

        PolygonShape poly;
        CircleShape  circle;

        if (use_box) {
            // IMPORTANT: same trick as you used.
            poly.SetAsBox(half_w * 2, half_h * 2);
            fdef.shape = &poly;
        }
        else {
            circle.center = Vector2::Zero();
            circle.radius = radius;
            fdef.shape = &circle;
        }

        body->CreateFixture(fdef);
        return body;
    }

    // ----- helpers to map a point to a process-grid rank -----

    static inline int RankFromPosition(
        const Vector2& p,
        const FlatAABB& bounds,
        const MpiSimConfig& cfg)
    {
        float rx = (p.x() - bounds.min.x()) /
            (bounds.max.x() - bounds.min.x());
        float ry = (p.y() - bounds.min.y()) /
            (bounds.max.y() - bounds.min.y());

        int ix = static_cast<int>(rx * cfg.nx);
        int iy = static_cast<int>(ry * cfg.ny);

        if (ix < 0)         ix = 0;
        if (ix >= cfg.nx)   ix = cfg.nx - 1;
        if (iy < 0)         iy = 0;
        if (iy >= cfg.ny)   iy = cfg.ny - 1;

        return iy * cfg.nx + ix;
    }

    // ----- counting pass -----

    int CountLocalObjectsForRankMPI(
        const FlatAABB& world_bounds,
        int   wall_grid_x,
        int   wall_grid_y,
        int   num_bodies,
        float p_polygon,
        const MpiSimConfig& cfg,
        int   my_rank)
    {
        int local_count = 0;

        // ----- Walls: identical layout to your BuildGridWalls -----
        const float world_w = world_bounds.max.x() - world_bounds.min.x();
        const float world_h = world_bounds.max.y() - world_bounds.min.y();

        const float cell_w = world_w / static_cast<float>(wall_grid_x);
        const float cell_h = world_h / static_cast<float>(wall_grid_y);
        const float half_t = 0.5f * 1.0f; // wall_thickness = 1.0f in your generator

        for (int row = 0; row < wall_grid_y; ++row) {
            const float cell_min_y = world_bounds.min.y() + row * cell_h;
            const float cell_max_y = cell_min_y + cell_h;
            const float cell_center_y = 0.5f * (cell_min_y + cell_max_y);

            for (int col = 0; col < wall_grid_x; ++col) {
                const float cell_min_x = world_bounds.min.x() + col * cell_w;
                const float cell_max_x = cell_min_x + cell_w;
                const float cell_center_x = 0.5f * (cell_min_x + cell_max_x);

                // bottom
                {
                    Vector2 center(cell_center_x, cell_min_y + half_t);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                        ++local_count;
                }

                // top
                {
                    Vector2 center(cell_center_x, cell_max_y - half_t);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                        ++local_count;
                }

                // left
                {
                    Vector2 center(cell_min_x + half_t, cell_center_y);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                        ++local_count;
                }

                // right
                {
                    Vector2 center(cell_max_x - half_t, cell_center_y);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                        ++local_count;
                }
            }
        }

        // ----- Dynamic bodies: same layout & constraints as GenerateHugeTestCase -----
        if (num_bodies <= 0) {
            return local_count;
        }

        const float wall_thickness = 1.0f;
        const float margin = wall_thickness * 2.0f;

        const float inner_min_x = world_bounds.min.x() + margin;
        const float inner_max_x = world_bounds.max.x() - margin;
        const float inner_min_y = world_bounds.min.y() + margin;
        const float inner_max_y = world_bounds.max.y() - margin;

        const float inner_w = inner_max_x - inner_min_x;
        const float inner_h = inner_max_y - inner_min_y;

        if (inner_w <= 0.0f || inner_h <= 0.0f) {
            return local_count;
        }

        const int bodies_cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(num_bodies))));
        const int bodies_rows = static_cast<int>(std::ceil(num_bodies / static_cast<float>(bodies_cols)));

        const float step_x = inner_w / static_cast<float>(bodies_cols);
        const float step_y = inner_h / static_cast<float>(bodies_rows);

        const float box_half_w = 0.2f;
        const float box_half_h = 0.2f;
        const float circle_radius = 0.2f;
        const float world_w2 = world_w;
        const float world_h2 = world_h;
        const float cell_w2 = world_w2 / static_cast<float>(wall_grid_x);
        const float cell_h2 = world_h2 / static_cast<float>(wall_grid_y);

        std::mt19937 rng(123456);
        std::uniform_real_distribution<float> uni01(0.0f, 1.0f);

        for (int i = 0; i < num_bodies; ++i) {
            const int row = i / bodies_cols;
            const int col = i % bodies_cols;
            if (row >= bodies_rows) break;

            float x = inner_min_x + (col + 0.5f) * step_x;
            float y = inner_min_y + (row + 0.5f) * step_y;

            x = std::min(std::max(x, world_bounds.min.x()), world_bounds.max.x());
            y = std::min(std::max(y, world_bounds.min.y()), world_bounds.max.y());

            const bool use_box = (uni01(rng) < p_polygon);

            const float body_half_x = use_box ? box_half_w : circle_radius;
            const float body_half_y = use_box ? box_half_h : circle_radius;

            int cell_col = static_cast<int>((x - world_bounds.min.x()) / cell_w2);
            int cell_row = static_cast<int>((y - world_bounds.min.y()) / cell_h2);

            if (cell_col < 0)            cell_col = 0;
            if (cell_col >= wall_grid_x) cell_col = wall_grid_x - 1;
            if (cell_row < 0)            cell_row = 0;
            if (cell_row >= wall_grid_y) cell_row = wall_grid_y - 1;

            const float cell_min_x = world_bounds.min.x() + cell_col * cell_w2;
            const float cell_max_x = cell_min_x + cell_w2;
            const float cell_min_y = world_bounds.min.y() + cell_row * cell_h2;
            const float cell_max_y = cell_min_y + cell_h2;

            float interior_min_x = cell_min_x + wall_thickness + body_half_x;
            float interior_max_x = cell_max_x - wall_thickness - body_half_x;
            float interior_min_y = cell_min_y + wall_thickness + body_half_y;
            float interior_max_y = cell_max_y - wall_thickness - body_half_y;

            if (interior_min_x > interior_max_x) {
                interior_min_x = interior_max_x = 0.5f * (cell_min_x + cell_max_x);
            }
            if (interior_min_y > interior_max_y) {
                interior_min_y = interior_max_y = 0.5f * (cell_min_y + cell_max_y);
            }

            x = std::min(std::max(x, interior_min_x), interior_max_x);
            y = std::min(std::max(y, interior_min_y), interior_max_y);

            Vector2 pos(x, y);

            // Decide if this body belongs to this rank (process grid).
            if (RankFromPosition(pos, world_bounds, cfg) == my_rank) {
                ++local_count;
            }
        }

        return local_count;
    }

    // ----- creation pass -----

    void CreateLocalObjectsForRankMPI(
        FlatWorld& world,
        const FlatAABB& world_bounds,
        int   wall_grid_x,
        int   wall_grid_y,
        int   num_bodies,
        float p_polygon,
        const MpiSimConfig& cfg,
        int   my_rank,
        int   start_gid,
        std::unordered_map<int, FlatBody*>& primary_by_id)
    {
        int next_gid = start_gid;

        // for random bodies
        float box_half_w = 0.2f;
        float box_half_h = 0.2f;
        float circle_radius = 0.2f;
        float max_linear_speed = 20.0f;
        float max_angular_speed = 5.0f;

        const float world_w = world_bounds.max.x() - world_bounds.min.x();
        const float world_h = world_bounds.max.y() - world_bounds.min.y();

        const float wall_thickness = 1.0f;
        const float cell_w_wall = world_w / static_cast<float>(wall_grid_x);
        const float cell_h_wall = world_h / static_cast<float>(wall_grid_y);
        const float half_t = 0.5f * wall_thickness;

        // ----- Walls -----
        for (int row = 0; row < wall_grid_y; ++row) {
            const float cell_min_y = world_bounds.min.y() + row * cell_h_wall;
            const float cell_max_y = cell_min_y + cell_h_wall;
            const float cell_center_y = 0.5f * (cell_min_y + cell_max_y);

            for (int col = 0; col < wall_grid_x; ++col) {
                const float cell_min_x = world_bounds.min.x() + col * cell_w_wall;
                const float cell_max_x = cell_min_x + cell_w_wall;
                const float cell_center_x = 0.5f * (cell_min_x + cell_max_x);

                // bottom
                {
                    Vector2 center(cell_center_x, cell_min_y + half_t);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank) {
                        FlatBody* b = CreateStaticBoxBodyMPI(world, center,
                            0.5f * cell_w_wall,
                            half_t);
                        b->SetGlobalID(next_gid);
                        b->SetOwnerCell(my_rank);
                        primary_by_id[next_gid] = b;
                        ++next_gid;
                    }
                }

                // top
                {
                    Vector2 center(cell_center_x, cell_max_y - half_t);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank) {
                        FlatBody* b = CreateStaticBoxBodyMPI(world, center,
                            0.5f * cell_w_wall,
                            half_t);
                        b->SetGlobalID(next_gid);
                        b->SetOwnerCell(my_rank);
                        primary_by_id[next_gid] = b;
                        ++next_gid;
                    }
                }

                // left
                {
                    Vector2 center(cell_min_x + half_t, cell_center_y);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank) {
                        FlatBody* b = CreateStaticBoxBodyMPI(world, center,
                            half_t,
                            0.5f * cell_h_wall);
                        b->SetGlobalID(next_gid);
                        b->SetOwnerCell(my_rank);
                        primary_by_id[next_gid] = b;
                        ++next_gid;
                    }
                }

                // right
                {
                    Vector2 center(cell_max_x - half_t, cell_center_y);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank) {
                        FlatBody* b = CreateStaticBoxBodyMPI(world, center,
                            half_t,
                            0.5f * cell_h_wall);
                        b->SetGlobalID(next_gid);
                        b->SetOwnerCell(my_rank);
                        primary_by_id[next_gid] = b;
                        ++next_gid;
                    }
                }
            }
        }

        // ----- Dynamic bodies -----
        if (num_bodies <= 0) return;

        const float margin = wall_thickness * 2.0f;
        const float inner_min_x = world_bounds.min.x() + margin;
        const float inner_max_x = world_bounds.max.x() - margin;
        const float inner_min_y = world_bounds.min.y() + margin;
        const float inner_max_y = world_bounds.max.y() - margin;

        const float inner_w = inner_max_x - inner_min_x;
        const float inner_h = inner_max_y - inner_min_y;

        if (inner_w <= 0.0f || inner_h <= 0.0f) {
            return;
        }

        const int bodies_cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(num_bodies))));
        const int bodies_rows = static_cast<int>(std::ceil(num_bodies / static_cast<float>(bodies_cols)));

        const float step_x = inner_w / static_cast<float>(bodies_cols);
        const float step_y = inner_h / static_cast<float>(bodies_rows);

        const float cell_w = world_w / static_cast<float>(wall_grid_x);
        const float cell_h = world_h / static_cast<float>(wall_grid_y);

        std::mt19937 rng(123456);
        std::uniform_real_distribution<float> uni01(0.0f, 1.0f);
        std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * 3.1415926535f);
        std::uniform_real_distribution<float> speed_dist(0.0f, max_linear_speed);
        std::uniform_real_distribution<float> ang_speed_dist(-max_angular_speed, max_angular_speed);

        for (int i = 0; i < num_bodies; ++i) {
            const int row = i / bodies_cols;
            const int col = i % bodies_cols;
            if (row >= bodies_rows) break;

            float x = inner_min_x + (col + 0.5f) * step_x;
            float y = inner_min_y + (row + 0.5f) * step_y;

            x = std::min(std::max(x, world_bounds.min.x()), world_bounds.max.x());
            y = std::min(std::max(y, world_bounds.min.y()), world_bounds.max.y());

            const bool use_box = (uni01(rng) < p_polygon);

            const float body_half_x = use_box ? box_half_w : circle_radius;
            const float body_half_y = use_box ? box_half_h : circle_radius;

            int cell_col = static_cast<int>((x - world_bounds.min.x()) / cell_w);
            int cell_row = static_cast<int>((y - world_bounds.min.y()) / cell_h);

            if (cell_col < 0)            cell_col = 0;
            if (cell_col >= wall_grid_x) cell_col = wall_grid_x - 1;
            if (cell_row < 0)            cell_row = 0;
            if (cell_row >= wall_grid_y) cell_row = wall_grid_y - 1;

            const float cell_min_x = world_bounds.min.x() + cell_col * cell_w;
            const float cell_max_x = cell_min_x + cell_w;
            const float cell_min_y = world_bounds.min.y() + cell_row * cell_h;
            const float cell_max_y = cell_min_y + cell_h;

            float interior_min_x = cell_min_x + wall_thickness + body_half_x;
            float interior_max_x = cell_max_x - wall_thickness - body_half_x;
            float interior_min_y = cell_min_y + wall_thickness + body_half_y;
            float interior_max_y = cell_max_y - wall_thickness - body_half_y;

            if (interior_min_x > interior_max_x) {
                interior_min_x = interior_max_x = 0.5f * (cell_min_x + cell_max_x);
            }
            if (interior_min_y > interior_max_y) {
                interior_min_y = interior_max_y = 0.5f * (cell_min_y + cell_max_y);
            }

            x = std::min(std::max(x, interior_min_x), interior_max_x);
            y = std::min(std::max(y, interior_min_y), interior_max_y);

            Vector2 pos(x, y);

            if (RankFromPosition(pos, world_bounds, cfg) != my_rank) {
                continue;
            }

            FlatBody* body = CreateDynamicBodyMPI(
                world,
                pos,
                use_box,
                box_half_w,
                box_half_h,
                circle_radius);

            if (!body) continue;

            body->SetGlobalID(next_gid);
            body->SetOwnerCell(my_rank);
            primary_by_id[next_gid] = body;
            ++next_gid;

            const float speed = speed_dist(rng);
            const float dir = angle_dist(rng);
            const Vector2 v(speed * std::cos(dir), speed * std::sin(dir));
            body->SetLinearVelocity(v);

            const float w = ang_speed_dist(rng);
            body->SetAngularVelocity(w);
            body->SetAwake(true);
        }
    }
    int CountLocalObjectsForRankMPI_SkewedCorner(
        const FlatAABB& world_bounds,
        int   wall_grid_x,
        int   wall_grid_y,
        int   num_bodies,
        float p_polygon,
        const MpiSimConfig& cfg,
        int   my_rank)
    {
        int local_count = 0;

        // ---------- Walls (unchanged) ----------

        const float world_w = world_bounds.max.x() - world_bounds.min.x();
        const float world_h = world_bounds.max.y() - world_bounds.min.y();

        const float cell_w = world_w / static_cast<float>(wall_grid_x);
        const float cell_h = world_h / static_cast<float>(wall_grid_y);
        const float half_t = 0.5f * 1.0f; // wall_thickness

        for (int row = 0; row < wall_grid_y; ++row)
        {
            const float cell_min_y = world_bounds.min.y() + row * cell_h;
            const float cell_max_y = cell_min_y + cell_h;
            const float cell_center_y = 0.5f * (cell_min_y + cell_max_y);

            for (int col = 0; col < wall_grid_x; ++col)
            {
                const float cell_min_x = world_bounds.min.x() + col * cell_w;
                const float cell_max_x = cell_min_x + cell_w;
                const float cell_center_x = 0.5f * (cell_min_x + cell_max_x);

                // bottom
                {
                    Vector2 center(cell_center_x, cell_min_y + half_t);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                        ++local_count;
                }

                // top
                {
                    Vector2 center(cell_center_x, cell_max_y - half_t);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                        ++local_count;
                }

                // left
                {
                    Vector2 center(cell_min_x + half_t, cell_center_y);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                        ++local_count;
                }

                // right
                {
                    Vector2 center(cell_max_x - half_t, cell_center_y);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                        ++local_count;
                }
            }
        }

        // ---------- Dynamic bodies (skewed) ----------

        if (num_bodies <= 0)
            return local_count;

        const float wall_thickness = 1.0f;
        const float margin = wall_thickness * 2.0f;

        const float inner_min_x = world_bounds.min.x() + margin;
        const float inner_max_x = world_bounds.max.x() - margin;
        const float inner_min_y = world_bounds.min.y() + margin;
        const float inner_max_y = world_bounds.max.y() - margin;

        const float inner_w = inner_max_x - inner_min_x;
        const float inner_h = inner_max_y - inner_min_y;

        if (inner_w <= 0.0f || inner_h <= 0.0f)
            return local_count;

        const int bodies_cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(num_bodies))));
        const int bodies_rows = static_cast<int>(std::ceil(num_bodies / static_cast<float>(bodies_cols)));

        const float cell_w2 = world_w / static_cast<float>(wall_grid_x);
        const float cell_h2 = world_h / static_cast<float>(wall_grid_y);

        // RNG for use_box decision
        std::mt19937 rng(123456);
        std::uniform_real_distribution<float> uni01(0.0f, 1.0f);


        for (int i = 0; i < num_bodies; ++i)
        {
            const int row = i / bodies_cols;
            const int col = i % bodies_cols;
            if (row >= bodies_rows) break;

            // normalized grid coords in [0,1]
            float ux = (static_cast<float>(col) + 0.5f) / static_cast<float>(bodies_cols);
            float uy = (static_cast<float>(row) + 0.5f) / static_cast<float>(bodies_rows);

            // *** skew toward bottom-left ***
            ux = std::pow(ux, skew_power);
            uy = std::pow(uy, skew_power);

            // map to world
            float x = inner_min_x + ux * inner_w;
            float y = inner_min_y + uy * inner_h;

            x = std::min(std::max(x, world_bounds.min.x()), world_bounds.max.x());
            y = std::min(std::max(y, world_bounds.min.y()), world_bounds.max.y());

            const bool use_box = (uni01(rng) < p_polygon);

            const float body_half_x = use_box ? 0.2f : 0.2f;
            const float body_half_y = use_box ? 0.2f : 0.2f;

            int cell_col = static_cast<int>((x - world_bounds.min.x()) / cell_w2);
            int cell_row = static_cast<int>((y - world_bounds.min.y()) / cell_h2);

            if (cell_col < 0) cell_col = 0;
            if (cell_col >= wall_grid_x) cell_col = wall_grid_x - 1;
            if (cell_row < 0) cell_row = 0;
            if (cell_row >= wall_grid_y) cell_row = wall_grid_y - 1;

            const float cell_min_x = world_bounds.min.x() + cell_col * cell_w2;
            const float cell_max_x = cell_min_x + cell_w2;
            const float cell_min_y = world_bounds.min.y() + cell_row * cell_h2;
            const float cell_max_y = cell_min_y + cell_h2;

            // shrink by wall thickness and radius (same logic)
            float interior_min_x = cell_min_x + wall_thickness + body_half_x;
            float interior_max_x = cell_max_x - wall_thickness - body_half_x;
            float interior_min_y = cell_min_y + wall_thickness + body_half_y;
            float interior_max_y = cell_max_y - wall_thickness - body_half_y;

            if (interior_min_x > interior_max_x)
                interior_min_x = interior_max_x = 0.5f * (cell_min_x + cell_max_x);

            if (interior_min_y > interior_max_y)
                interior_min_y = interior_max_y = 0.5f * (cell_min_y + cell_max_y);

            x = std::min(std::max(x, interior_min_x), interior_max_x);
            y = std::min(std::max(y, interior_min_y), interior_max_y);

            Vector2 pos(x, y);

            if (RankFromPosition(pos, world_bounds, cfg) == my_rank)
                ++local_count;
        }

        return local_count;
    }
    void CreateLocalObjectsForRankMPI_SkewedCorner(
        FlatWorld& world,
        const FlatAABB& world_bounds,
        int   wall_grid_x,
        int   wall_grid_y,
        int   num_bodies,
        float p_polygon,
        const MpiSimConfig& cfg,
        int   my_rank,
        int   start_gid,
        std::unordered_map<int, FlatBody*>& primary_by_id)
    {
        int next_gid = start_gid;

        // --- same body params as your original generator ---
        float box_half_w = 0.2f;
        float box_half_h = 0.2f;
        float circle_radius = 0.2f;
        float max_linear_speed = 20.0f;
        float max_angular_speed = 5.0f;

        const float world_w = world_bounds.max.x() - world_bounds.min.x();
        const float world_h = world_bounds.max.y() - world_bounds.min.y();

        const float wall_thickness = 1.0f;
        const float cell_w_wall = world_w / static_cast<float>(wall_grid_x);
        const float cell_h_wall = world_h / static_cast<float>(wall_grid_y);
        const float half_t = 0.5f * wall_thickness;

        // ==================================================================
        // 1) WALLS – layout identical to BuildGridWalls, but only created
        //    on ranks whose process-tile contains the wall center.
        // ==================================================================
        for (int row = 0; row < wall_grid_y; ++row)
        {
            const float cell_min_y = world_bounds.min.y() + row * cell_h_wall;
            const float cell_max_y = cell_min_y + cell_h_wall;
            const float cell_center_y = 0.5f * (cell_min_y + cell_max_y);

            for (int col = 0; col < wall_grid_x; ++col)
            {
                const float cell_min_x = world_bounds.min.x() + col * cell_w_wall;
                const float cell_max_x = cell_min_x + cell_w_wall;
                const float cell_center_x = 0.5f * (cell_min_x + cell_max_x);

                // bottom
                {
                    Vector2 center(cell_center_x, cell_min_y + half_t);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                    {
                        FlatBody* b = CreateStaticBoxBodyMPI(
                            world,
                            center,
                            0.5f * cell_w_wall,
                            half_t);
                        b->SetGlobalID(next_gid);
                        b->SetOwnerCell(my_rank);
                        primary_by_id[next_gid] = b;
                        ++next_gid;
                    }
                }

                // top
                {
                    Vector2 center(cell_center_x, cell_max_y - half_t);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                    {
                        FlatBody* b = CreateStaticBoxBodyMPI(
                            world,
                            center,
                            0.5f * cell_w_wall,
                            half_t);
                        b->SetGlobalID(next_gid);
                        b->SetOwnerCell(my_rank);
                        primary_by_id[next_gid] = b;
                        ++next_gid;
                    }
                }

                // left
                {
                    Vector2 center(cell_min_x + half_t, cell_center_y);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                    {
                        FlatBody* b = CreateStaticBoxBodyMPI(
                            world,
                            center,
                            half_t,
                            0.5f * cell_h_wall);
                        b->SetGlobalID(next_gid);
                        b->SetOwnerCell(my_rank);
                        primary_by_id[next_gid] = b;
                        ++next_gid;
                    }
                }

                // right
                {
                    Vector2 center(cell_max_x - half_t, cell_center_y);
                    if (RankFromPosition(center, world_bounds, cfg) == my_rank)
                    {
                        FlatBody* b = CreateStaticBoxBodyMPI(
                            world,
                            center,
                            half_t,
                            0.5f * cell_h_wall);
                        b->SetGlobalID(next_gid);
                        b->SetOwnerCell(my_rank);
                        primary_by_id[next_gid] = b;
                        ++next_gid;
                    }
                }
            }
        }

        // ==================================================================
        // 2) DYNAMIC BODIES – same constraints as original generator, but
        //    positions are skewed toward bottom-left via pow().
        // ==================================================================
        if (num_bodies <= 0)
            return;

        const float margin = wall_thickness * 2.0f;
        const float inner_min_x = world_bounds.min.x() + margin;
        const float inner_max_x = world_bounds.max.x() - margin;
        const float inner_min_y = world_bounds.min.y() + margin;
        const float inner_max_y = world_bounds.max.y() - margin;

        const float inner_w = inner_max_x - inner_min_x;
        const float inner_h = inner_max_y - inner_min_y;

        if (inner_w <= 0.0f || inner_h <= 0.0f)
            return;

        const int bodies_cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(num_bodies))));
        const int bodies_rows = static_cast<int>(std::ceil(num_bodies / static_cast<float>(bodies_cols)));

        const float cell_w = world_w / static_cast<float>(wall_grid_x);
        const float cell_h = world_h / static_cast<float>(wall_grid_y);

        std::mt19937 rng(123456);
        std::uniform_real_distribution<float> uni01(0.0f, 1.0f);
        std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * 3.1415926535f);
        std::uniform_real_distribution<float> speed_dist(0.0f, max_linear_speed);
        std::uniform_real_distribution<float> ang_speed_dist(-max_angular_speed, max_angular_speed);


        for (int i = 0; i < num_bodies; ++i)
        {
            const int row = i / bodies_cols;
            const int col = i % bodies_cols;
            if (row >= bodies_rows) break;

            // Normalized grid coords in [0,1]
            float ux = (static_cast<float>(col) + 0.5f) / static_cast<float>(bodies_cols);
            float uy = (static_cast<float>(row) + 0.5f) / static_cast<float>(bodies_rows);

            // Skew toward bottom-left corner
            ux = std::pow(ux, skew_power);
            uy = std::pow(uy, skew_power);

            // Map back into inner region
            float x = inner_min_x + ux * inner_w;
            float y = inner_min_y + uy * inner_h;

            // Clamp to world bounds (same as original)
            x = std::min(std::max(x, world_bounds.min.x()), world_bounds.max.x());
            y = std::min(std::max(y, world_bounds.min.y()), world_bounds.max.y());

            const bool use_box = (uni01(rng) < p_polygon);

            const float body_half_x = use_box ? box_half_w : circle_radius;
            const float body_half_y = use_box ? box_half_h : circle_radius;

            int cell_col = static_cast<int>((x - world_bounds.min.x()) / cell_w);
            int cell_row = static_cast<int>((y - world_bounds.min.y()) / cell_h);

            if (cell_col < 0)            cell_col = 0;
            if (cell_col >= wall_grid_x) cell_col = wall_grid_x - 1;
            if (cell_row < 0)            cell_row = 0;
            if (cell_row >= wall_grid_y) cell_row = wall_grid_y - 1;

            const float cell_min_x = world_bounds.min.x() + cell_col * cell_w;
            const float cell_max_x = cell_min_x + cell_w;
            const float cell_min_y = world_bounds.min.y() + cell_row * cell_h;
            const float cell_max_y = cell_min_y + cell_h;

            float interior_min_x = cell_min_x + wall_thickness + body_half_x;
            float interior_max_x = cell_max_x - wall_thickness - body_half_x;
            float interior_min_y = cell_min_y + wall_thickness + body_half_y;
            float interior_max_y = cell_max_y - wall_thickness - body_half_y;

            if (interior_min_x > interior_max_x) {
                interior_min_x = interior_max_x = 0.5f * (cell_min_x + cell_max_x);
            }
            if (interior_min_y > interior_max_y) {
                interior_min_y = interior_max_y = 0.5f * (cell_min_y + cell_max_y);
            }

            x = std::min(std::max(x, interior_min_x), interior_max_x);
            y = std::min(std::max(y, interior_min_y), interior_max_y);

            Vector2 pos(x, y);

            // Only create body on owning rank
            if (RankFromPosition(pos, world_bounds, cfg) != my_rank) {
                continue;
            }

            FlatBody* body = CreateDynamicBodyMPI(
                world,
                pos,
                use_box,
                box_half_w,
                box_half_h,
                circle_radius);

            if (!body) continue;

            body->SetGlobalID(next_gid);
            body->SetOwnerCell(my_rank);
            primary_by_id[next_gid] = body;
            ++next_gid;

            const float speed = speed_dist(rng);
            const float dir = angle_dist(rng);
            const Vector2 v(speed * std::cos(dir), speed * std::sin(dir));
            body->SetLinearVelocity(v);

            const float w = ang_speed_dist(rng);
            body->SetAngularVelocity(w);
            body->SetAwake(true);
        }
    }
} // namespace FlatPhysics
