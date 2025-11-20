#include "DrawBodyComponent.h"
#include  "Vector2.h"
#include "GameObject.h"
#include "Engine.h"
#include "Game.h"
#include <algorithm>
#include "BoxCollider.h"
#include "CircleCollider.h"
#include "Input.h"
#include "Collision.h"
#include "FlatShape.h"
#include "FlatMath.h" 
#include "PhysicsDB.h"
#include "FlatFixture.h"
#include "FlatWorld.h"
#include "MatMN.h"
#include "JointConstraint.h"
#include "FlatBody.h"
#include "FlatShape.h"
#include "TextDB.h"
#include <string>
#include "Game.h"
namespace {
    static inline uint32_t Hash32(uint32_t x) {
        x ^= x >> 16; x *= 0x7feb352d; x ^= x >> 15; x *= 0x846ca68b; x ^= x >> 16;
        return x;
    }

    static void HSVtoRGB(float h, float s, float v, int& r, int& g, int& b) {
        float c = v * s;
        float hp = h * 6.0f;
        float x = c * (1.0f - std::fabsf(fmodf(hp, 2.0f) - 1.0f));
        float m = v - c;
        float rf = 0, gf = 0, bf = 0;
        if (hp < 1) { rf = c; gf = x; bf = 0; }
        else if (hp < 2) { rf = x; gf = c; bf = 0; }
        else if (hp < 3) { rf = 0; gf = c; bf = x; }
        else if (hp < 4) { rf = 0; gf = x; bf = c; }
        else if (hp < 5) { rf = x; gf = 0; bf = c; }
        else { rf = c; gf = 0; bf = x; }
        r = (int)std::round((rf + m) * 255.0f);
        g = (int)std::round((gf + m) * 255.0f);
        b = (int)std::round((bf + m) * 255.0f);
    }

    // Pastel, well-spaced per-ID color
    static void PastelColorFromID(int id, int& r, int& g, int& b) {
        // 1) spread hues evenly with golden ratio; quantize to bins for separation
        constexpr float PHI = 0.61803398875f;      // golden-ratio conjugate
        const int bins = 32;                       // increase for more unique hues
        float baseH = std::fmodf(id * PHI, 1.0f);  // [0,1)
        float h = (std::floor(baseH * bins) + 0.5f) / bins; // center of bin

        // 2) small deterministic jitter for S to avoid “samey” look
        uint32_t seed = Hash32((uint32_t)id);
        float j = ((seed & 0xFFFF) / 65535.0f);    // [0,1)

        // Pastel: medium S, high V
        float s = 0.50f + 0.20f * j;               // ~[0.50, 0.70]
        float v = 0.90f + 0.07f * ((seed >> 16) & 0xFF) / 255.0f; // ~[0.90, 0.97]

        HSVtoRGB(h, s, v, r, g, b);
    }
    static bool IsAABBVisible(const FlatPhysics::FlatAABB& aabb)
    {
        const float ppm = 100.0f;

        std::unique_ptr<Game>& game = Engine::instance->running_game;
        const glm::vec2  cam_pos = game->Get_Camera_Position(); 
        const glm::ivec2 cam_dim = game->Get_Camera_Dimension(); 
        const float      zoom = game->Get_Zoom_Factor();

        const float half_w = (cam_dim.x * 0.5f) / (ppm * zoom);
        const float half_h = (cam_dim.y * 0.5f) / (ppm * zoom);

        const float cam_min_x = cam_pos.x - half_w;
        const float cam_max_x = cam_pos.x + half_w;
        const float cam_min_y = cam_pos.y - half_h;
        const float cam_max_y = cam_pos.y + half_h;

        const float pad = 0.2f; 
        const float min_x = aabb.min.x() - pad;
        const float max_x = aabb.max.x() + pad;
        const float min_y = aabb.min.y() - pad;
        const float max_y = aabb.max.y() + pad;

        if (max_x < cam_min_x) return false;
        if (min_x > cam_max_x) return false;
        if (max_y < cam_min_y) return false;
        if (min_y > cam_max_y) return false;

        return true;
    }
}

void DrawBodyComponent::On_Update()
{
    //Rotate();
    Vector2 dir(0.0f, 0.0f);
    if (Input::GetKey(SDL_SCANCODE_W)) dir += Vector2(0.0f, -1.0f);
    if (Input::GetKey(SDL_SCANCODE_S)) dir += Vector2(0.0f, 1.0f);
    if (Input::GetKey(SDL_SCANCODE_A)) dir += Vector2(-1.0f, 0.0f);
    if (Input::GetKey(SDL_SCANCODE_D)) dir += Vector2(1.0f, 0.0f);
    move_dir = dir;

    //auto transform = this->holder_object->Get_Transform().lock();
    //if (holder_object->ID == 44) {
    //    auto bodies = Engine::instance->running_game->Find_All_GameObjects_By_Name("Body");
    //    for (std::weak_ptr<GameObject> p : bodies) {
    //        std::shared_ptr<GameObject> object = p.lock();
    //        //std::cout << object->ID << std::endl;
    //        if (object->ID == holder_object->ID)continue;
    //        auto comp = std::dynamic_pointer_cast<DrawBodyComponent>(object->Get_Component("DrawBodyComponent").lock());
    //        if (comp->body) {
    //            Vector2 normal;
    //            float depth;
    //            auto& my_fixture = body->GetFixtures().front();
    //            auto& other_fixture = comp->body->GetFixtures().front();
    //            if (body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Circle && comp->body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Circle) {
    //                if (FlatPhysics::Collision::IntersectCircles(body->GetPosition(), my_fixture->GetShape().AsCircle()->radius, comp->body->GetPosition(), other_fixture->GetShape().AsCircle()->radius, &normal, &depth)) {
    //                    std::cout << "Collision between Body " << holder_object->ID << " and " << comp->holder_object->ID << std::endl;
    //                    comp->body->Move(normal * depth);
    //                }
    //            }
    //            else if (body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Polygon && comp->body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Polygon) {
    //                Vector2 normal;
    //                float depth;
    //                auto my_vertices = my_fixture->GetShape().AsPolygon()->vertices;
    //                auto other_vertices = other_fixture->GetShape().AsPolygon()->vertices;
    //                auto my_transform = body->GetTransform();
    //                auto other_transform = comp->body->GetTransform();
    //                if (FlatPhysics::Collision::IntersectPolygons(FlatPhysics::FlatTransform::TransformVectors(my_vertices,my_transform), FlatPhysics::FlatTransform::TransformVectors(other_vertices, other_transform),&normal,&depth)) {
    //                    std::cout << "Collision between Body " << holder_object->ID << " and " << comp->holder_object->ID <<" Polygon" <<std::endl;
    //                    comp->body->Move(normal * depth);
    //                }
    //            }
    //            else if (body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Polygon && comp->body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Circle) {
    //                Vector2 normal;
    //                float depth;
    //                auto my_vertices = my_fixture->GetShape().AsPolygon()->vertices;
    //                auto circle = comp->body->GetFixtures().front()->GetShape().AsCircle();
    //                auto my_transform = body->GetTransform();
    //                auto other_transform = comp->body->GetTransform();
    //                if (FlatPhysics::Collision::IntersectCirclePolygon(FlatPhysics::FlatTransform::TransformVector(circle->center,other_transform),circle->radius, FlatPhysics::FlatTransform::TransformVectors(my_vertices, my_transform), &normal, &depth)) {
    //                    std::cout << "Collision between Body " << holder_object->ID << " and " << comp->holder_object->ID << " Circle" << std::endl;
    //                    comp->body->Move(-normal * depth);
    //                }
    //            }
    //            else if (body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Circle && comp->body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Polygon) {
    //                Vector2 normal;
    //                float depth;
    //                auto other_vertices = other_fixture->GetShape().AsPolygon()->vertices;
    //                auto circle = my_fixture->GetShape().AsCircle();
    //                auto my_transfrom = body->GetTransform();
    //                auto other_transform = comp->body->GetTransform();
    //                if (FlatPhysics::Collision::IntersectCirclePolygon(FlatPhysics::FlatTransform::TransformVector(circle->center,my_transfrom), circle->radius, FlatPhysics::FlatTransform::TransformVectors(other_vertices, other_transform), &normal, &depth)) {
    //                    std::cout << "Collision between Body " << holder_object->ID << " and " << comp->holder_object->ID << " Circle" << std::endl;
    //                    comp->body->Move(normal * depth);
    //                }
    //            }
    //        }
    //    }
    //}
    if(this->holder_object->ID == 5)DrawTime();
    DrawBody();
    DrawAABB();
    if(holder_object->ID == 5)PhysicsDB::flat_world->DrawContactPoints();
    //std::cout << transform->Get_World_Position().x() << " " << transform->Get_World_Position().y() << std::endl;
}

void DrawBodyComponent::On_Start()
{
    active_body++;
    bool is_static = false;
    if (shape == "Box") {
        is_static = true;
    }
	auto transform = this->holder_object->Get_Transform().lock();
    FlatPhysics::BodyDef bodyDef;
    bodyDef.position = transform->Get_World_Position();
    bodyDef.linear_damping = 0.2f;
    bodyDef.angular_damping = 0.5f;
    bodyDef.is_static = is_static;
    bodyDef.allow_sleep = true;
    this->body = PhysicsDB::flat_world->CreateBody(bodyDef);
    if (shape == "Box") {
        std::unique_ptr<FlatPhysics::PolygonShape> polygon_shape = std::make_unique<FlatPhysics::PolygonShape>();
        polygon_shape->SetAsBox(width, height);

        FlatPhysics::FixtureDef fixtureDef;
        fixtureDef.shape = polygon_shape.get();
        fixtureDef.density = 2.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.friction = 1.0f;

        body->CreateFixture(fixtureDef);

    }
	else if (shape == "Polygon") {
        const float s = 0.2f;
        std::vector<Vector2> poly;
        poly.emplace_back(-s, -s);
        poly.emplace_back(+s, -s);
        poly.emplace_back(+s, +s);
        poly.emplace_back(-s, +s);

        std::unique_ptr<FlatPhysics::PolygonShape> polygon_shape = std::make_unique<FlatPhysics::PolygonShape>(poly);

        FlatPhysics::FixtureDef fixtureDef;
        fixtureDef.shape = polygon_shape.get();
        fixtureDef.density = 5.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.friction = 1.0f;

        body->CreateFixture(fixtureDef);
    }
	else {
        std::unique_ptr<FlatPhysics::CircleShape> circle_shape = std::make_unique<FlatPhysics::CircleShape>(Vector2::Zero(), 0.2f);

        FlatPhysics::FixtureDef fixtureDef;
        fixtureDef.shape = circle_shape.get();
        fixtureDef.density = 2.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.friction = 1.0f;

        body->CreateFixture(fixtureDef);
	}

    //if (holder_object->ID >= 8) {
    //    auto sb = Engine::instance->running_game->Find_All_GameObjects_By_Name("Body");
    //    auto sb2 = Engine::instance->running_game->Find_All_GameObjects_By_Name("Body2");
    //    std::vector<std::weak_ptr<GameObject>> merged;
    //    merged.insert(merged.end(), sb.begin(), sb.end());
    //    merged.insert(merged.end(), sb2.begin(), sb2.end());
    //    std::shared_ptr<Component> nt;
    //    for (auto i : merged) {
    //        auto ptr = i.lock();
    //        if (ptr->ID == holder_object->ID -1)
    //            nt = ptr->Get_Component("DrawBodyComponent").lock();
    //    }
    //    std::shared_ptr<DrawBodyComponent> body2 = std::dynamic_pointer_cast<DrawBodyComponent>(nt);
    //    std::unique_ptr<FlatPhysics::JointConstraint> constraint = std::make_unique<FlatPhysics::JointConstraint>(body->GetFixtures()[0].get(), body2->body->GetFixtures()[0].get(), body2->body->GetMassCenterWorld());
    //    PhysicsDB::flat_world->AddConstraint(std::move(constraint));
    //}
}

void DrawBodyComponent::On_Fixed_Update()
{
    if (this->holder_object->ID == 5)GetTime();
    MoveFirstBody();
}

void DrawBodyComponent::Add_Int_Property(const std::string& key, int new_property)
{
    if (key == "width") {
        width = new_property;
    }
    else if (key == "height") {
        height = new_property;
    }
    else if (key == "radius") {
        radius = new_property;
    }
    else {
        std::cout << "undefined property: " << key << std::endl;
    }
}

void DrawBodyComponent::Add_Float_Property(const std::string& key, float new_property)
{
    if (key == "width") {
        width = new_property;
    }
    else if (key == "height") {
        height = new_property;
    }
    else if (key == "radius") {
        radius = new_property;
    }
    else {
        std::cout << "undefined property: " << key << std::endl;
    }
}

void DrawBodyComponent::Add_String_Property(const std::string& key, const std::string& new_property)
{
    if (key == "shape") {
        if (new_property == "circle") {
            shape = "Circle";
        }
        else if (new_property == "polygon") {
            shape = "Polygon";
        }
        else if (new_property == "box") {
            shape = "Box";
        }
        else {
            std::cout << "unknown shape: " << new_property << std::endl;
        }
    }
    else {
        std::cout << "undefined property: " << key << std::endl;
    }
}

void DrawBodyComponent::Add_Bool_Property(const std::string& key, bool new_property)
{
    if (key == "is_static") {
        is_static = new_property;
    }
    else {
        std::cout << "undefined property: " << key << std::endl;
    }
}


void DrawBodyComponent::DrawBody()
{
    for (const std::unique_ptr<FlatPhysics::FlatFixture>& fixture : body->GetFixtures()) {
        FlatPhysics::FlatAABB aabb = fixture->GetAABB();
        if (!IsAABBVisible(aabb)) {
            continue;
        }

        const float x = body->GetPosition().GetX();
        const float y = body->GetPosition().GetY();
        const float rot = body->GetAngle();

        const float r = 255;
        const float g = 255;
        const float bl = 255;
        const float a = 255;

        switch (fixture->GetShapeType())
        {
        case FlatPhysics::ShapeType::Circle:
        {
            float radius = fixture->GetShape().AsCircle()->radius;
            Engine::instance->renderer->draw_ex(
                "circle",
                x, y,
                FlatPhysics::FlatMath::RadToDeg(rot),
                radius * 2.0f, radius * 2.0f,
                0.5f, 0.5f,
                r, g, bl, a, 0
            );
            break;
        }
        case FlatPhysics::ShapeType::Polygon:
        {
            const FlatPhysics::PolygonShape* polygon = fixture->GetShape().AsPolygon();
            const auto& vertices = polygon->GetVertices();

            int pr, pg, pb;
            PastelColorFromID(holder_object->ID, pr, pg, pb);

            Engine::instance->renderer->draw_polygon(vertices, body->GetPosition(), body->GetAngle(), pr, pg, pb, 255, true);
            break;
        }
        }
    }
}

void DrawBodyComponent::MoveFirstBody()
{
    if (holder_object->ID != 7)return;
    float len = move_dir.Length();
    if (len > 0.0f) {
        move_dir = move_dir * (1.0f / len);               
        const Vector2 delta = move_dir * 2.5;
        //body->Move(delta * Engine::instance->running_game->Delta_Time()); 
        body->AddForce(delta);
    }
}

void DrawBodyComponent::Rotate()
{
    if (!body) return;
    float rotation_speed = 360 * Engine::instance->running_game->Delta_Time();
    body->Rotate(FlatPhysics::FlatMath::DegToRad(rotation_speed));
}

void DrawBodyComponent::DrawAABB()
{
    Vector2 bodyPos = body->GetPosition();

    for (const auto& fixture : body->GetFixtures()) {
        FlatPhysics::FlatAABB aabb = fixture->GetAABB();
        AABB[0] = { aabb.min.x(), aabb.min.y() };
        AABB[1] = { aabb.max.x(), aabb.min.y() };
        AABB[2] = { aabb.max.x(), aabb.max.y() };
        AABB[3] = { aabb.min.x(), aabb.max.y() };
        Engine::instance->renderer->draw_polygon_world(AABB, 0, 128, 0, 128, false);
    }
}

void DrawBodyComponent::DrawTime()
{
    auto dimension = Engine::instance->running_game->Get_Camera_Dimension();
    Engine::instance->renderer->draw_text("NotoSans-Regular", "FPS: " + std::to_string(fps), 24, {0,0,0},dimension.x / 200,dimension.y/20 );
    Engine::instance->renderer->draw_text("NotoSans-Regular", "Physics Step Time: " + std::to_string(physics_step_time) + " ms", 24, { 0,0,0 }, dimension.x / 200, dimension.y / 12);
    Engine::instance->renderer->draw_text("NotoSans-Regular", "Physics FPS: " + std::to_string(physics_fps), 24, { 0,0,0 }, dimension.x / 200, dimension.y / 8.5);
    Engine::instance->renderer->draw_text("NotoSans-Regular", "Current Bodies: " + std::to_string(active_body), 24, { 0,0,0 }, dimension.x / 200, dimension.y / 6.7);
}

void DrawBodyComponent::GetTime()
{
    fps = Engine::instance->running_game->GetFPS();;
    physics_step_time = Engine::instance->running_game->GetPhysicsStepTime();
    physics_fps = Engine::instance->running_game->GetPhysicsFPS();
}
