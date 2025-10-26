#pragma once
#include "FlatShape.h"
#include <memory>
namespace FlatPhysics {
	struct Filter {
		uint16_t category_bits = 0x0001; 
		uint16_t mask_bits = 0xFFFF;
		int16_t  group_index = 0;
	};
	struct FixtureDef {
		const Shape* shape = nullptr;
		float density = 1.0f;
		float friction = 0.3f;
		float restitution = 0.0f;
		bool is_trigger = false;
		Filter filter{};
        void* user_data = nullptr;
	};
	class FlatBody;
    class FlatFixture {
    public:
        FlatFixture(FlatBody* body, const FixtureDef& def)
            : body_(body),
            shape_(def.shape ? def.shape->Clone() : nullptr),
            density_(def.density),
            friction_(def.friction),
            restitution_(def.restitution),
            is_trigger_(def.is_trigger),
            filter_(def.filter),
            user_data_(def.user_data) {}

        FlatBody* GetBody()        const { return body_; }
        const Shape& GetShape()       const { return *shape_; }
        const Filter& GetFilter()      const { return filter_; }
        bool            GetIsTrigger()       const { return is_trigger_; }
        float           GetDensity()     const { return density_; }
        float           GetFriction()    const { return friction_; }
        float           GetRestitution() const { return restitution_; }
        void* GetUserData() const { return user_data_; }

    private:
        FlatBody* body_;
        std::unique_ptr<Shape> shape_;
        float density_{ 1.0f };
        float friction_{ 0.3f };
        float restitution_{ 0.0f };
        bool  is_trigger_{ false };
        Filter filter_{};
        void* user_data_ = nullptr;
    };
}