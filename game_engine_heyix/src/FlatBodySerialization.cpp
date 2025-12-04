#include "FlatBodySerialization.h"

namespace FlatPhysics {
	namespace {
		enum class SerializedShapeType : std::int32_t
		{
			Circle = 0,
			Polygon = 1
		};
	}

    void SerializeFlatBody(const FlatBody& body, std::vector<std::uint8_t>& buf)
	{
        buf.clear();
        buf.reserve(256);

        {
            std::int32_t global_id = body.GetGlobalID();
            std::int32_t owner_cell = body.GetOwnerCell();
            std::uint8_t is_static = body.IsStatic() ? 1 : 0;
            std::uint8_t is_ghost = body.IsGhost() ? 1 : 0;

            WritePod(buf, global_id);
            WritePod(buf, owner_cell);
            WritePod(buf, is_static);
            WritePod(buf, is_ghost);

            WriteVector2(buf, body.GetPosition());
            float angle = body.GetAngle();
            WritePod(buf, angle);

            WriteVector2(buf, body.GetLinearVelocity());
            float angVel = body.GetAngularVelocity();
            WritePod(buf, angVel);

            float gravity_scale = body.GetGravityScale();
            WritePod(buf, gravity_scale);

            std::uint8_t has_custom_gravity =
                body.HasCustomGravity() ? 1 : 0;
            WritePod(buf, has_custom_gravity);

            if (has_custom_gravity)
            {
                Vector2 custom_g = body.GetCustomGravity();
                WriteVector2(buf, custom_g);
            }

            float sleep_time = body.GetSleepTime();
            std::uint8_t awake = body.IsAwake() ? 1 : 0;
            std::uint8_t can_slp = body.GetCanSleep() ? 1 : 0;

            WritePod(buf, sleep_time);
            WritePod(buf, awake);
            WritePod(buf, can_slp);

            float lin_damp = body.GetLinearDamping();
            float ang_damp = body.GetAngularDamping();
            WritePod(buf, lin_damp);
            WritePod(buf, ang_damp);
        }

        const auto& fixtures = body.GetFixtures();
        std::int32_t fixture_count =
            static_cast<std::int32_t>(fixtures.size());
        WritePod(buf, fixture_count);

        for (const auto& fUPtr : fixtures)
        {
            const FlatFixture* f = fUPtr.get();
            if (!f)
            {
                std::int32_t shape_type = -1;
                WritePod(buf, shape_type);
                continue;
            }

            const Shape& shape = f->GetShape();
            ShapeType st = shape.GetType();

            SerializedShapeType sst =
                (st == ShapeType::Circle)
                ? SerializedShapeType::Circle
                : SerializedShapeType::Polygon;

            std::int32_t shape_type = static_cast<std::int32_t>(sst);
            WritePod(buf, shape_type);

            float density = f->GetDensity();
            float friction = f->GetFriction();
            float restitution = f->GetRestitution();
            std::uint8_t is_trigger = f->GetIsTrigger() ? 1 : 0;
            Filter filter = f->GetFilter();

            WritePod(buf, density);
            WritePod(buf, friction);
            WritePod(buf, restitution);
            WritePod(buf, is_trigger);
            WritePod(buf, filter);

            if (sst == SerializedShapeType::Circle)
            {
                const CircleShape* c = shape.AsCircle();
                Vector2 center = c ? c->center : Vector2::Zero();
                float radius = c ? c->radius : 0.0f;
                WriteVector2(buf, center);
                WritePod(buf, radius);
            }
            else
            {
                const PolygonShape* poly = shape.AsPolygon();
                const auto& verts = poly->GetVertices();

                std::int32_t vert_count =
                    static_cast<std::int32_t>(verts.size());
                WritePod(buf, vert_count);
                for (const Vector2& v : verts)
                {
                    WriteVector2(buf, v);
                }
            }
        }
	}

    FlatBody* DeserializeFlatBody(FlatWorld& world,
        const std::uint8_t* data,
        std::size_t size,
        std::size_t& offset,
        FlatBody* existing_body /* = nullptr */)
    {
        std::int32_t global_id = 0;
        std::int32_t owner_cell = -1;
        std::uint8_t is_static_u8 = 0;
        std::uint8_t is_ghost_u8 = 0;

        if (!ReadPod(data, size, offset, global_id))    return nullptr;
        if (!ReadPod(data, size, offset, owner_cell))   return nullptr;
        if (!ReadPod(data, size, offset, is_static_u8)) return nullptr;
        if (!ReadPod(data, size, offset, is_ghost_u8))  return nullptr;

        Vector2 position;
        if (!ReadVector2(data, size, offset, position)) return nullptr;

        float angle = 0.0f;
        if (!ReadPod(data, size, offset, angle)) return nullptr;

        Vector2 linVel;
        if (!ReadVector2(data, size, offset, linVel)) return nullptr;

        float angVel = 0.0f;
        if (!ReadPod(data, size, offset, angVel)) return nullptr;

        float gravity_scale = 1.0f;
        if (!ReadPod(data, size, offset, gravity_scale)) return nullptr;

        std::uint8_t has_custom_gravity_u8 = 0;
        if (!ReadPod(data, size, offset, has_custom_gravity_u8)) return nullptr;

        Vector2 custom_g = Vector2::Zero();
        if (has_custom_gravity_u8)
        {
            if (!ReadVector2(data, size, offset, custom_g)) return nullptr;
        }

        float sleep_time = 0.0f;
        std::uint8_t awake_u8 = 1;
        std::uint8_t can_slp_u8 = 1;

        if (!ReadPod(data, size, offset, sleep_time))   return nullptr;
        if (!ReadPod(data, size, offset, awake_u8))     return nullptr;
        if (!ReadPod(data, size, offset, can_slp_u8))   return nullptr;

        float lin_damp = 0.0f;
        float ang_damp = 0.0f;
        if (!ReadPod(data, size, offset, lin_damp)) return nullptr;
        if (!ReadPod(data, size, offset, ang_damp)) return nullptr;

        bool is_static = (is_static_u8 != 0);
        bool is_ghost = (is_ghost_u8 != 0);
        bool awake = (awake_u8 != 0);
        bool can_sleep = (can_slp_u8 != 0);
        bool has_custom_gravity = (has_custom_gravity_u8 != 0);

        std::int32_t fixture_count = 0;
        if (!ReadPod(data, size, offset, fixture_count)) return nullptr;
        if (fixture_count < 0) return nullptr;

        FlatBody* body = existing_body;

        if (!body)
        {
            BodyDef def;
            def.position = position;
            def.angle_rad = angle;
            def.is_static = is_static;
            def.linear_damping = lin_damp;
            def.angular_damping = ang_damp;
            def.gravity_scale = gravity_scale;
            def.allow_sleep = can_sleep;
            def.awake = awake;

            body = world.CreateBody(def);
            if (!body) return nullptr;

            body->ReserveFixtures(static_cast<std::size_t>(fixture_count));
            body->SetGhost(is_ghost);
            body->SetOwnerCell(owner_cell);
            body->SetGlobalID(global_id);

            if (has_custom_gravity)
                body->SetCustomGravity(custom_g);
            else
                body->ClearCustomGravity();

            body->SetSleepTime(sleep_time);
            body->SetAwake(awake);
            body->SetLinearVelocity(linVel, false);
            body->SetAngularVelocity(angVel, false);

            for (int fi = 0; fi < fixture_count; ++fi)
            {
                std::int32_t shape_type_i = -1;
                if (!ReadPod(data, size, offset, shape_type_i)) return nullptr;
                if (shape_type_i < 0)
                {
                    continue;
                }

                auto sst = static_cast<SerializedShapeType>(shape_type_i);

                float density = 1.0f;
                float friction = 0.3f;
                float restitution = 0.0f;
                std::uint8_t is_trigger_u8 = 0;
                Filter filter{};

                if (!ReadPod(data, size, offset, density))       return nullptr;
                if (!ReadPod(data, size, offset, friction))      return nullptr;
                if (!ReadPod(data, size, offset, restitution))   return nullptr;
                if (!ReadPod(data, size, offset, is_trigger_u8)) return nullptr;
                if (!ReadPod(data, size, offset, filter))        return nullptr;

                bool is_trigger = (is_trigger_u8 != 0);

                FixtureDef fdef;
                fdef.density = density;
                fdef.friction = friction;
                fdef.restitution = restitution;
                fdef.is_trigger = is_trigger;
                fdef.filter = filter;

                if (sst == SerializedShapeType::Circle)
                {
                    Vector2 center;
                    float radius = 0.0f;
                    if (!ReadVector2(data, size, offset, center)) return nullptr;
                    if (!ReadPod(data, size, offset, radius))     return nullptr;

                    CircleShape tmp;
                    tmp.center = center;
                    tmp.radius = radius;
                    fdef.shape = &tmp;
                    body->CreateFixture(fdef);
                }
                else
                {
                    std::int32_t vert_count = 0;
                    if (!ReadPod(data, size, offset, vert_count)) return nullptr;
                    if (vert_count < 0) return nullptr;

                    std::vector<Vector2> verts;
                    verts.reserve(static_cast<std::size_t>(vert_count));
                    for (int vi = 0; vi < vert_count; ++vi)
                    {
                        Vector2 v;
                        if (!ReadVector2(data, size, offset, v)) return nullptr;
                        verts.push_back(v);
                    }

                    auto shape = std::make_unique<PolygonShape>(std::move(verts));
                    body->CreateFixtureWithShape(fdef, std::move(shape));
                }
            }
        }
        else
        {
            body->SetGhost(is_ghost);
            body->SetOwnerCell(owner_cell);
            body->SetGlobalID(global_id);

            body->SetGravityScale(gravity_scale);
            if (has_custom_gravity)
                body->SetCustomGravity(custom_g);
            else
                body->ClearCustomGravity();

            body->SetCanSleep(can_sleep);
            body->SetSleepTime(sleep_time);
            body->SetAwake(awake);

            body->MoveTo(position, false);
            float angle_delta = angle - body->GetAngle();
            if (angle_delta != 0.0f)
            {
                body->Rotate(angle_delta, false);
            }

            body->SetLinearVelocity(linVel, false);
            body->SetAngularVelocity(angVel, false);
            body->SetLinearDamping(lin_damp);
            body->SetAngularDamping(ang_damp);

            for (int fi = 0; fi < fixture_count; ++fi)
            {
                std::int32_t shape_type_i = -1;
                if (!ReadPod(data, size, offset, shape_type_i)) return nullptr;
                if (shape_type_i < 0)
                {
                    continue;
                }

                auto sst = static_cast<SerializedShapeType>(shape_type_i);

                float density = 1.0f;
                float friction = 0.3f;
                float restitution = 0.0f;
                std::uint8_t is_trigger_u8 = 0;
                Filter filter{};

                if (!ReadPod(data, size, offset, density))       return nullptr;
                if (!ReadPod(data, size, offset, friction))      return nullptr;
                if (!ReadPod(data, size, offset, restitution))   return nullptr;
                if (!ReadPod(data, size, offset, is_trigger_u8)) return nullptr;
                if (!ReadPod(data, size, offset, filter))        return nullptr;

                if (sst == SerializedShapeType::Circle)
                {
                    Vector2 center;
                    float radius = 0.0f;
                    if (!ReadVector2(data, size, offset, center)) return nullptr;
                    if (!ReadPod(data, size, offset, radius))     return nullptr;
                }
                else
                {
                    std::int32_t vert_count = 0;
                    if (!ReadPod(data, size, offset, vert_count)) return nullptr;
                    if (vert_count < 0) return nullptr;

                    for (int vi = 0; vi < vert_count; ++vi)
                    {
                        Vector2 v;
                        if (!ReadVector2(data, size, offset, v)) return nullptr;
                    }
                }
            }
        }

        return body;
    }

}
