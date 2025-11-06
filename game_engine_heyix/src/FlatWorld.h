#pragma once
#include "Vector2.h"
#include "FlatBody.h"
#include <vector>
#include <unordered_map>
namespace FlatPhysics {
	class ContactHit {
	public:
		FlatFixture* fixtureA = nullptr;
		FlatFixture* fixtureB = nullptr;
		Vector2 normal = Vector2::Zero();
		float depth = 0.0f;
	};
	class FlatWorld {
	public:
		FlatWorld() 
			:gravity({0,9.81})
		{}
	public:
		const static inline float MinBodySize = 0.01f * 0.01f;
		const static inline float MaxBodySize = 64.0f * 64.0f;
		
		const static inline float MinDensity = 0.5f;
		const static inline float MaxDensity = 21.4f;
	public:
		int GetBodyCount() { return bodies.size(); }
	public:
		void AddBody(FlatBody* body);
		bool RemoveBody(FlatBody* body);
		FlatBody* GetBody(int index);
		void Step(float time);
	private:
		void CollisionDetectionStep();
		bool DetectCollision(FlatFixture* fa, FlatFixture* fb, Vector2* normal = nullptr, float* depth = nullptr);
		bool ShouldCollide(FlatBody* a, FlatBody* b, FlatFixture* fixture_a, FlatFixture* fixture_b);
	private:
		Vector2 gravity;
		std::vector<FlatBody*> bodies;
		std::unordered_map<FlatBody*, int> index_map;
		std::vector<ContactHit> contacts;
	};
}