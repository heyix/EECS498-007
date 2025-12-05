#include "TestScene.h"
#include "Game.h"
#include "DrawBodyComponent.h"
static std::string empty_name = "Empty";


FlatPhysics::FlatBody* FlatPhysics::CreateStaticBoxBody(FlatPhysics::DistributedDomain& domain, const Vector2& center, float half_w, float half_h, float friction, float restitution)
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

    FlatBody* body = domain.CreateBody(bdef);

    PolygonShape box;
    box.SetAsBox(half_w * 2, half_h * 2);

    FixtureDef fdef;
    fdef.shape = &box; 
    fdef.density = 1.0f; 
    fdef.friction = friction;
    fdef.restitution = restitution;
    fdef.is_trigger = false;

    body->CreateFixture(fdef);

    std::shared_ptr<GameObject> object = Engine::instance->running_game->Instantiate_GameObject_From_Template(empty_name).lock();
    std::shared_ptr<Component> comp = object->Get_Component("DrawBodyComponent").lock();
    std::shared_ptr<DrawBodyComponent> draw = std::dynamic_pointer_cast<DrawBodyComponent>(comp);
    draw->Set_Global_ID(body->GetGlobalID());


    return body;
}

void FlatPhysics::BuildGridWalls(FlatPhysics::DistributedDomain& domain, const FlatPhysics::FlatAABB& world_bounds, int grid_x, int grid_y, float wall_thickness)
{
    if (grid_x <= 0 || grid_y <= 0 || wall_thickness <= 0.0f) return;

    const float world_w = world_bounds.max.x() - world_bounds.min.x();
    const float world_h = world_bounds.max.y() - world_bounds.min.y();

    const float cell_w = world_w / static_cast<float>(grid_x);
    const float cell_h = world_h / static_cast<float>(grid_y);

    const float half_t = 0.5f * wall_thickness;

    for (int row = 0; row < grid_y; ++row)
    {
        const float cell_min_y = world_bounds.min.y() + row * cell_h;
        const float cell_max_y = cell_min_y + cell_h;
        const float cell_center_y = 0.5f * (cell_min_y + cell_max_y);

        for (int col = 0; col < grid_x; ++col)
        {
            const float cell_min_x = world_bounds.min.x() + col * cell_w;
            const float cell_max_x = cell_min_x + cell_w;
            const float cell_center_x = 0.5f * (cell_min_x + cell_max_x);


            {
                const Vector2 center(cell_center_x, cell_min_y + half_t);
                const float half_w = 0.5f * cell_w;
                const float half_h = half_t;
                CreateStaticBoxBody(domain, center, half_w, half_h);
            }


            {
                const Vector2 center(cell_center_x, cell_max_y - half_t);
                const float half_w = 0.5f * cell_w;
                const float half_h = half_t;
                CreateStaticBoxBody(domain, center, half_w, half_h);
            }


            {
                const Vector2 center(cell_min_x + half_t, cell_center_y);
                const float half_w = half_t;
                const float half_h = 0.5f * cell_h;
                CreateStaticBoxBody(domain, center, half_w, half_h);
            }


            {
                const Vector2 center(cell_max_x - half_t, cell_center_y);
                const float half_w = half_t;
                const float half_h = 0.5f * cell_h;
                CreateStaticBoxBody(domain, center, half_w, half_h);
            }
        }
    }
}

FlatPhysics::FlatBody* FlatPhysics::CreateDynamicBody(FlatPhysics::DistributedDomain& domain, const Vector2& pos, bool use_box, float half_w, float half_h, float radius, float density, float friction, float restitution)
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

    FlatBody* body = domain.CreateBody(bdef);

    FixtureDef fdef;
    fdef.density = density;
    fdef.friction = friction;
    fdef.restitution = restitution;
    fdef.is_trigger = false;

    PolygonShape poly;
    CircleShape  circle;

    if (use_box) {
        poly.SetAsBox(half_w * 2, half_h * 2);
        fdef.shape = &poly;
    }
    else {
        circle.center = Vector2::Zero();
        circle.radius = radius;
        fdef.shape = &circle;
    }

    body->CreateFixture(fdef);


    std::shared_ptr<GameObject> object = Engine::instance->running_game->Instantiate_GameObject_From_Template(empty_name).lock();
    std::shared_ptr<Component> comp = object->Get_Component("DrawBodyComponent").lock();
    std::shared_ptr<DrawBodyComponent> draw = std::dynamic_pointer_cast<DrawBodyComponent>(comp);
    draw->Set_Global_ID(body->GetGlobalID());



    return body;
}

void FlatPhysics::GenerateHugeTestCase(FlatPhysics::DistributedDomain& domain, const FlatPhysics::FlatAABB& world_bounds, int grid_x, int grid_y, int num_bodies, float p_polygon)
{
    float box_half_w = 0.2f;
    float box_half_h = 0.2f;
    float circle_radius = 0.2f;

    float max_linear_speed = 20.0f;
    float max_angular_speed = 5.0f;


    const float wall_thickness = 1.0f;
    BuildGridWalls(domain, world_bounds, grid_x, grid_y, wall_thickness);

    const float margin = wall_thickness * 2.0f;
    const float inner_min_x = world_bounds.min.x() + margin;
    const float inner_max_x = world_bounds.max.x() - margin;
    const float inner_min_y = world_bounds.min.y() + margin;
    const float inner_max_y = world_bounds.max.y() - margin;

    const float inner_w = inner_max_x - inner_min_x;
    const float inner_h = inner_max_y - inner_min_y;

    if (inner_w <= 0.0f || inner_h <= 0.0f || num_bodies <= 0) {
        return;
    }

    const int bodies_cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(num_bodies))));
    const int bodies_rows = static_cast<int>(std::ceil(num_bodies / static_cast<float>(bodies_cols)));

    const float step_x = inner_w / static_cast<float>(bodies_cols);
    const float step_y = inner_h / static_cast<float>(bodies_rows);

    const float world_w = world_bounds.max.x() - world_bounds.min.x();
    const float world_h = world_bounds.max.y() - world_bounds.min.y();
    const float cell_w = world_w / static_cast<float>(grid_x);
    const float cell_h = world_h / static_cast<float>(grid_y);

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

        if (cell_col < 0)         cell_col = 0;
        if (cell_col >= grid_x)   cell_col = grid_x - 1;
        if (cell_row < 0)         cell_row = 0;
        if (cell_row >= grid_y)   cell_row = grid_y - 1;

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

        const Vector2 pos(x, y);

        FlatBody* body = CreateDynamicBody(
            domain,
            pos,
            use_box,
            box_half_w,
            box_half_h,
            circle_radius);

        if (!body) continue;

        const float speed = speed_dist(rng);
        const float dir = angle_dist(rng);
        const Vector2 v(speed * std::cos(dir), speed * std::sin(dir));
        body->SetLinearVelocity(v);

        const float w = ang_speed_dist(rng);
        body->SetAngularVelocity(w);
        body->SetAwake(true);
    }
}
