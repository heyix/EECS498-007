#pragma once

#include "IBroadPhase.h"
#include <vector>
namespace FlatPhysics {
    class BroadphaseNaive final : public IBroadPhase {
    public:
        BroadphaseNaive() = default;
        ProxyID CreateProxy(const FlatAABB& aabb, void* user_data) override;
        void DestroyProxy(ProxyID id) override;
        void MoveProxy(ProxyID id, const FlatAABB& aabb, const Vector2& displacement) override;
        void TouchProxy(ProxyID id) override;
        void UpdatePairs(IPairCallback* callback) override;
        void Query(const FlatAABB& aabb, IQueryCallback& callback) override;

    private:
        struct Proxy {
            FlatAABB aabb{ 0.0f, 0.0f, 0.0f, 0.0f };
            void* user_data{ nullptr };
            bool dirty{ false };
            bool active{ false };
        };

        bool IsValid(ProxyID id) const;
        bool IsActive(ProxyID id) const;

        std::vector<Proxy> proxies_;
        std::vector<ProxyID> free_list_;
    };

}