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
     

    DrawBody();
    DrawAABB();
    if(holder_object->ID == 4)PhysicsDB::flat_world->DrawContactPoints();
    //std::cout << transform->Get_World_Position().x() << " " << transform->Get_World_Position().y() << std::endl;
}

void DrawBodyComponent::On_Start()
{
	auto transform = this->holder_object->Get_Transform().lock();
    if (holder_object->ID == 4 || holder_object->ID == 5 || holder_object->ID == 6) {
        std::unique_ptr<FlatPhysics::PolygonShape> shape = std::make_unique<FlatPhysics::PolygonShape>();
        shape->SetAsBox(width, height);
        FlatPhysics::FlatBody::CreatePolygonBody(shape->vertices, transform->Get_World_Position(), 2.0f, true, 0.5f, this->body);
    }
	else if (shape == FlatPhysics::ShapeType::Polygon) {
        const float s = 0.2f;
        std::vector<Vector2> poly;
        poly.emplace_back(-s, -s);       // bottom-left
        poly.emplace_back(+s, -s);       // bottom-right
        poly.emplace_back(+s, +s);       // top-right
        //poly.emplace_back(0.0f, +s * 0.3f); // inner dent (makes it concave)
        poly.emplace_back(-s, +s);       // top-left
        FlatPhysics::FlatBody::CreatePolygonBody(poly, transform->Get_World_Position(), 2.0f, false, 0.5f, this->body);
    }
	else {
		FlatPhysics::FlatBody::CreateCircleBody(0.2f, transform->Get_World_Position(), 2.0f, false, 0.5f, this->body);
	}
    PhysicsDB::flat_world->AddBody(body.get());
}

void DrawBodyComponent::On_Fixed_Update()
{
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
            shape = FlatPhysics::ShapeType::Circle;
        }
        else if (new_property == "polygon") {
            shape = FlatPhysics::ShapeType::Polygon;
        }
        else {
            std::cout << "unknown shape: " << new_property << std::endl;
        }
    }
    else {
        std::cout << "undefined property: " << key << std::endl;
    }
}


void DrawBodyComponent::DrawBody()
{
    for (const std::unique_ptr<FlatPhysics::FlatFixture>& fixture : body->GetFixtures()) {
        const float x = body->GetPosition().GetX();
        const float y = body->GetPosition().GetY();
        const float rot = body->GetRotation();

        const float r = 255;
        const float g = 255;
        const float bl = 255;
        const float a = 255;

        switch (fixture->GetShapeType())
        {
        case FlatPhysics::ShapeType::Circle: {
            float radius = fixture->GetShape().AsCircle()->radius;
            Engine::instance->renderer->draw_ex("circle", x, y, FlatPhysics::FlatMath::RadToDeg(rot),
                radius * 2.0f, radius * 2.0f,
                0.5f, 0.5f,
                r, g, bl, a, 0);
            break;
        }
        case FlatPhysics::ShapeType::Polygon: {
            auto& vertices = fixture->GetShape().AsPolygon()->vertices;
            int r, g, b;
            PastelColorFromID(holder_object->ID, r, g, b);

            Engine::instance->renderer->draw_polygon_world(FlatPhysics::FlatTransform::TransformVectors(vertices, body->GetTransform()), r, g, b, 255, true);
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
    for (const auto& fixture : body->GetFixtures()) {
        FlatPhysics::FlatAABB aabb = fixture->GetAABB();
        std::vector<Vector2> corners{
            {aabb.min.x(), aabb.min.y()},
            {aabb.max.x(), aabb.min.y()},
            {aabb.max.x(), aabb.max.y()},
            {aabb.min.x(), aabb.max.y()}
        };
        Engine::instance->renderer->draw_polygon_world(corners, 0, 128, 0, 128, false);
    }
}
