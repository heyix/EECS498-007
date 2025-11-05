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
void DrawBodyComponent::On_Update()
{
    Rotate();
    MoveFirstBody();



    auto transform = this->holder_object->Get_Transform().lock();
    if (holder_object->ID == 44) {
        auto bodies = Engine::instance->running_game->Find_All_GameObjects_By_Name("Body");
        for (std::weak_ptr<GameObject> p : bodies) {
            std::shared_ptr<GameObject> object = p.lock();
            //std::cout << object->ID << std::endl;
            if (object->ID == holder_object->ID)continue;
            auto comp = std::dynamic_pointer_cast<DrawBodyComponent>(object->Get_Component("DrawBodyComponent").lock());
            if (comp->body) {
                Vector2 normal;
                float depth;
                auto& my_fixture = body->GetFixtures().front();
                auto& other_fixture = comp->body->GetFixtures().front();
                if (body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Circle && comp->body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Circle) {
                    if (FlatPhysics::Collision::IntersectCircles(body->GetPosition(), my_fixture->GetShape().AsCircle()->radius, comp->body->GetPosition(), other_fixture->GetShape().AsCircle()->radius, &normal, &depth)) {
                        std::cout << "Collision between Body " << holder_object->ID << " and " << comp->holder_object->ID << std::endl;
                        comp->body->Move(normal * depth);
                    }
                }
                else if (body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Polygon && comp->body->GetFixtures().front()->GetShapeType() == FlatPhysics::ShapeType::Polygon) {
                    Vector2 normal;
                    float depth;
                    auto my_vertices = my_fixture->GetShape().AsPolygon()->vertices;
                    auto other_vertices = other_fixture->GetShape().AsPolygon()->vertices;
                    auto my_transform = body->GetTransform();
                    auto other_transform = comp->body->GetTransform();
                    if (FlatPhysics::Collision::IntersectPolygons(FlatPhysics::FlatTransform::TransformVectors(my_vertices,my_transform), FlatPhysics::FlatTransform::TransformVectors(other_vertices, other_transform),&normal,&depth)) {
                        std::cout << "Collision between Body " << holder_object->ID << " and " << comp->holder_object->ID <<" Polygon" <<std::endl;
                        comp->body->Move(normal * depth);
                    }
                }
            }
        }
    }


     

    DrawBody();

    //std::cout << transform->Get_World_Position().x() << " " << transform->Get_World_Position().y() << std::endl;
}

void DrawBodyComponent::On_Start()
{
	auto transform = this->holder_object->Get_Transform().lock();
	if (shape == FlatPhysics::ShapeType::Box) {
		std::unique_ptr<FlatPhysics::FlatBody> body;
		std::string error_message;
		FlatPhysics::FlatBody::CreateBoxBody(1, 1, transform->Get_World_Position(), 2.0f, false, 0.5f, this->body, &error_message);
	}
    else if (shape == FlatPhysics::ShapeType::Polygon) {
        std::unique_ptr<FlatPhysics::FlatBody> body;
        const float s = 0.4f;
        std::vector<Vector2> poly;
        poly.emplace_back(-s, -s+0.2f);       // bottom-left
        poly.emplace_back(+s, -s);       // bottom-right
        poly.emplace_back(+s, +s);       // top-right
        //poly.emplace_back(0.0f, +s * 0.3f); // inner dent (makes it concave)
        poly.emplace_back(-s, +s);       // top-left
        FlatPhysics::FlatBody::CreatePolygonBody(poly, transform->Get_World_Position(), 2.0f, false, 0.5f, this->body);
    }
	else {
		std::unique_ptr<FlatPhysics::FlatBody> body;
		FlatPhysics::FlatBody::CreateCircleBody(0.5f, transform->Get_World_Position(), 2.0f, false, 0.5f, this->body);
	}
}


void DrawBodyComponent::DrawBody()
{
    const float x = body->GetPosition().GetX();
    const float y = body->GetPosition().GetY();
    const float rot = body->GetRotation();

    const float r = 255;
    const float g = 255;
    const float bl = 255;
    const float a = 255;

    switch (body->shape_type)
    {
        case FlatPhysics::ShapeType::Circle: {
            auto& my_fixture = body->GetFixtures().front();
            float radius = my_fixture->GetShape().AsCircle()->radius;
            Engine::instance->renderer->draw_ex("circle", x, y, rot,
                radius * 2.0f, radius * 2.0f,  
                0.5f, 0.5f,                   
                r, g, bl, a, 0);
            break;
        }

        case FlatPhysics::ShapeType::Box: {
            auto& my_fixture = body->GetFixtures().front();
            float width = my_fixture->GetShape().AsBox()->width;
            float height = my_fixture->GetShape().AsBox()->height;
            Engine::instance->renderer->draw_ex("box" + std::to_string(holder_object->ID % 3 + 1), x, y, rot,
                width, height,
                0.5f, 0.5f,
                r, g, bl, a, 0);
            break;
        }
        case FlatPhysics::ShapeType::Polygon: {
            auto& my_fixture = body->GetFixtures().front();
            auto& vertices = my_fixture->GetShape().AsPolygon()->vertices;
            Engine::instance->renderer->draw_polygon_world(FlatPhysics::FlatTransform::TransformVectors(vertices,body->GetTransform()),  0, 255, 0, 255);
            break;
        }
    }

}

void DrawBodyComponent::MoveFirstBody()
{
    if (holder_object->ID != 44)return;
    Vector2 dir(0.0f, 0.0f);
    if (Input::GetKey(SDL_SCANCODE_W)) dir += Vector2(0.0f, -1.0f);
    if (Input::GetKey(SDL_SCANCODE_S)) dir += Vector2(0.0f, 1.0f);
    if (Input::GetKey(SDL_SCANCODE_A)) dir += Vector2(-1.0f, 0.0f);
    if (Input::GetKey(SDL_SCANCODE_D)) dir += Vector2(1.0f, 0.0f);
    float len = dir.Length();
    if (len > 0.0f) {
        dir = dir * (1.0f / len);               
        const Vector2 delta = dir * 0.05;
        body->Move(delta * Engine::instance->running_game->Delta_Time() *50);                        
    }
}

void DrawBodyComponent::Rotate()
{
    if (!body) return;
    if (body->shape_type == FlatPhysics::ShapeType::Circle) {
        float rotation_speed = 1.0f * Engine::instance->running_game->Delta_Time();
        body->Rotate(rotation_speed);
    }
    else if (body->shape_type == FlatPhysics::ShapeType::Polygon) {
        float rotation_speed = 1.0f * Engine::instance->running_game->Delta_Time();
        body->Rotate(rotation_speed);
    }
}
