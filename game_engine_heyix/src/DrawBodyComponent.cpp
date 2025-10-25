#include "DrawBodyComponent.h"
#include  "Vector2.h"
#include "GameObject.h"
#include "Engine.h"
#include "Game.h"
#include <algorithm>
#include "ColliderBase.h"
void DrawBodyComponent::On_Update()
{
    DrawBody();
    auto transform = this->holder_object->Get_Transform().lock();
    //std::cout << transform->Get_World_Position().x() << " " << transform->Get_World_Position().y() << std::endl;
}

void DrawBodyComponent::On_Start()
{
	auto transform = this->holder_object->Get_Transform().lock();
	if (shape == FlatPhysics::ShapeType::Box) {
		std::unique_ptr<FlatPhysics::FlatBody> body;
		std::string error_message;
        auto collider = std::dynamic_pointer_cast<ColliderBase>(this->holder_object->Get_Component("BoxCollider").lock());
		FlatPhysics::FlatBody::CreateBoxBody(collider->width, collider->height, transform->Get_World_Position(), 2.0f, false, 0.5f, this->body, &error_message);
	}
	else {
		std::unique_ptr<FlatPhysics::FlatBody> body;
        auto collider = std::dynamic_pointer_cast<ColliderBase>(this->holder_object->Get_Component("CircleCollider").lock());
		FlatPhysics::FlatBody::CreateCircleBody(collider->radius, transform->Get_World_Position(), 2.0f, false, 0.5f, this->body);
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
        Engine::instance->renderer->draw_ex("box1", x, y, rot,
            body->width, body->height,                   // scale to size
            0.5f, 0.5f,
            r, g, bl, a, 0);
        break;
    }
}
