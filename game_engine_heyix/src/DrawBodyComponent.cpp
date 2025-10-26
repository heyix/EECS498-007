#include "DrawBodyComponent.h"
#include  "Vector2.h"
#include "GameObject.h"
#include "Engine.h"
#include "Game.h"
#include <algorithm>
#include "BoxCollider.h"
#include "CircleCollider.h"
#include "Input.h"
void DrawBodyComponent::On_Update()
{
    DrawBody();
    MoveFirstBody();
    auto transform = this->holder_object->Get_Transform().lock();
    auto bodies = Engine::instance->running_game->Find_All_GameObjects_By_Name("body");
    int i = 0;
    //for (auto p : bodies) {
    //    std::cout << p.lock()->name << i << std::endl;
    //    i++;
    //}
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
	else {
		std::unique_ptr<FlatPhysics::FlatBody> body;
		FlatPhysics::FlatBody::CreateCircleBody(0.5f, transform->Get_World_Position(), 2.0f, false, 0.5f, this->body);
	}
}


void DrawBodyComponent::DrawBody()
{
    const float x = body->GetPosition().GetX();
    const float y = body->GetPosition().GetY();
    const float rot = 0;

    // Color: gray for static, cyan for dynamic
    const float r = 255;
    const float g = 255;
    const float bl = 255;
    const float a = 255;

    switch (body->shape_type)
    {
    case FlatPhysics::ShapeType::Circle:
        // scale_x/scale_y are final on-screen size; use diameter
        Engine::instance->renderer->draw_ex("circle", x, y, rot,
            body->radius * 2.0f, body->radius * 2.0f,   // scale to diameter
            0.5f, 0.5f,                         // pivot at center
            r, g, bl, a, 0);
        break;
    case FlatPhysics::ShapeType::Box:
        Engine::instance->renderer->draw_ex("box" + std::to_string(holder_object->ID % 3 + 1), x, y, rot,
            body->width, body->height,                   // scale to size
            0.5f, 0.5f,
            r, g, bl, a, 0);
        break;
    }
}

void DrawBodyComponent::MoveFirstBody()
{
    if (holder_object->ID != 154)return;
    Vector2 dir(0.0f, 0.0f);
    if (Input::GetKey(SDL_SCANCODE_W)) dir += Vector2(0.0f, -1.0f);
    if (Input::GetKey(SDL_SCANCODE_S)) dir += Vector2(0.0f, 1.0f);
    if (Input::GetKey(SDL_SCANCODE_A)) dir += Vector2(-1.0f, 0.0f);
    if (Input::GetKey(SDL_SCANCODE_D)) dir += Vector2(1.0f, 0.0f);
    float len = dir.Length();
    if (len > 0.0f) {
        dir = dir * (1.0f / len);               
        const Vector2 delta = dir * 0.05;
        body->Move(delta);                        
    }
}
