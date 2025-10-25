#define _USE_MATH_DEFINES
#include "FlatBody.h"
#include <cmath>
#include "FlatWorld.h"
#include "iostream"
#include "FlatMath.h"
void FlatPhysics::FlatBody::Move(Vector2 amount)
{
	this->position += amount;
}
void FlatPhysics::FlatBody::MoveTo(Vector2 position)
{
}
bool FlatPhysics::FlatBody::CreateCircleBody(float radius, Vector2 position, float density, bool is_static, float restitution, std::unique_ptr<FlatBody>& out_body, std::string* error_message)
{
	float area = radius * radius * M_PI;
	out_body = nullptr;
	if (error_message)*error_message = "";
	if (area < FlatWorld::MinBodySize) {
		if (error_message)*error_message = "Body circle is too small, where requested area is " + std::to_string(area) + " and minimum allowed area is " + std::to_string(FlatWorld::MinBodySize);
		return false;
	}
	if (area > FlatWorld::MaxBodySize) {
		if (error_message)*error_message = "Body circle is too big, where requested area is " +std::to_string(area) + " and maximum allowed area is " + std::to_string(FlatWorld::MaxBodySize);
		return false;
	}
	if (density < FlatWorld::MinDensity) {
		if (error_message)*error_message = "Body density is too small, where requested density is " + std::to_string(density) + " and minimum allowed density is " + std::to_string(FlatWorld::MinDensity);
		return false;
	}
	if (density > FlatWorld::MaxDensity) {
		if (error_message)*error_message = "Body density is too big, where requested density is " + std::to_string(density) + " and maximum allowed density is " + std::to_string(FlatWorld::MaxDensity);
		return false;
	}
	restitution = FlatMath::Clamp(restitution, 0.0f, 1.0f);
	float mass = area * 1.0f * density;
	out_body = std::unique_ptr<FlatBody>(new FlatBody(position, density, mass, restitution, area, is_static, radius, 0.0f, 0.0f, ShapeType::Circle));
	return true;
}

bool FlatPhysics::FlatBody::CreateBoxBody(float width, float height, Vector2 position, float density, bool is_static, float restitution, std::unique_ptr<FlatBody>& out_body, std::string* error_message)
{
	float area = width * height;
	out_body = nullptr;
	if (error_message)*error_message = "";
	if (area < FlatWorld::MinBodySize) {
		if (error_message)*error_message = "Body box is too small, where requested area is " + std::to_string(area) + " and minimum allowed area is " + std::to_string(FlatWorld::MinBodySize);
		return false;
	}
	if (area > FlatWorld::MaxBodySize) {
		if (error_message)*error_message = "Body box is too big, where requested area is " + std::to_string(area) + " and maximum allowed area is " + std::to_string(FlatWorld::MaxBodySize);
		return false;
	}
	if (density < FlatWorld::MinDensity) {
		if (error_message)*error_message = "Body density is too small, where requested density is " + std::to_string(density) + " and minimum allowed density is " + std::to_string(FlatWorld::MinDensity);
		return false;
	}
	if (density > FlatWorld::MaxDensity) {
		if (error_message)*error_message = "Body density is too big, where requested density is " + std::to_string(density) + " and maximum allowed density is " + std::to_string(FlatWorld::MaxDensity);
		return false;
	}
	restitution = FlatMath::Clamp(restitution, 0.0f, 1.0f);
	float mass = area * 1.0f * density;
	out_body = std::unique_ptr<FlatBody>(new FlatBody(position, density, mass, restitution, area, is_static, 0.0f, width, height, ShapeType::Box));
	return true;
}
