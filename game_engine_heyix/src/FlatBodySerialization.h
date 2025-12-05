#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include "FlatBody.h"
#include "FlatFixture.h"
#include "FlatShape.h"
#include "FlatDefs.h"
#include "FlatWorld.h"

namespace FlatPhysics
{
    template <typename T>
    inline void WritePod(std::vector<std::uint8_t>& buf, const T& v)
    {
        static_assert(std::is_trivially_copyable<T>::value,
            "WritePod only supports trivially copyable types");
        const std::size_t old_size = buf.size();
        buf.resize(old_size + sizeof(T));
        std::memcpy(buf.data() + old_size, &v, sizeof(T));
    }

    template <typename T>
    inline bool ReadPod(const std::uint8_t* data,
        std::size_t size,
        std::size_t& offset,
        T& out)
    {
        static_assert(std::is_trivially_copyable<T>::value,
            "ReadPod only supports trivially copyable types");
        if (offset + sizeof(T) > size) return false;
        std::memcpy(&out, data + offset, sizeof(T));
        offset += sizeof(T);
        return true;
    }

    inline void WriteVector2(std::vector<std::uint8_t>& buf, const Vector2& v)
    {
        float x = v.x();
        float y = v.y();
        WritePod(buf, x);
        WritePod(buf, y);
    }

    inline bool ReadVector2(const std::uint8_t* data,
        std::size_t size,
        std::size_t& offset,
        Vector2& out)
    {
        float x, y;
        if (!ReadPod(data, size, offset, x)) return false;
        if (!ReadPod(data, size, offset, y)) return false;
        out = Vector2(x, y);
        return true;
    }

    void SerializeFlatBody(const FlatBody& body, std::vector<std::uint8_t>& buf);

    FlatBody* DeserializeFlatBody(FlatWorld& world,
        const std::uint8_t* data,
        std::size_t size,
        std::size_t& offset,
        FlatBody* existing_body /* = nullptr */);
}