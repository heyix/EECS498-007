#include "BroadPhaseNaive.h"
#include <algorithm>

namespace FlatPhysics {

    ProxyID BroadphaseNaive::CreateProxy(const FlatAABB& aabb, void* user_data) {
        Proxy proxy;
        proxy.aabb = aabb;
        proxy.user_data = user_data;
        proxy.dirty = true;
        proxy.active = true;

        if (!free_list_.empty()) {
            ProxyID id = free_list_.back();
            free_list_.pop_back();
            proxies_[id] = proxy;
            return id;
        }

        proxies_.push_back(proxy);
        return static_cast<ProxyID>(proxies_.size() - 1);
    }

    void BroadphaseNaive::DestroyProxy(ProxyID id) {
        if (!IsValid(id)) {
            return;
        }

        Proxy& proxy = proxies_[id];
        proxy = Proxy{};
        free_list_.push_back(id);
    }

    void BroadphaseNaive::MoveProxy(ProxyID id, const FlatAABB& aabb, const Vector2& /*displacement*/) {
        if (!IsActive(id)) {
            return;
        }

        Proxy& proxy = proxies_[id];
        proxy.aabb = aabb;
        proxy.dirty = true;
    }

    void BroadphaseNaive::TouchProxy(ProxyID id) {
        if (!IsActive(id)) {
            return;
        }

        proxies_[id].dirty = true;
    }

    void BroadphaseNaive::UpdatePairs(IPairCallback* callback) {
        if (!callback) {
            return;
        }

        const ProxyID count = static_cast<ProxyID>(proxies_.size());
        for (ProxyID i = 0; i < count; ++i) {
            if (!IsActive(i)) {
                continue;
            }

            Proxy& proxyA = proxies_[i];
            for (ProxyID j = i + 1; j < count; ++j) {
                if (!IsActive(j)) {
                    continue;
                }

                Proxy& proxyB = proxies_[j];
                if (!FlatAABB::IntersectAABB(proxyA.aabb, proxyB.aabb)) {
                    continue;
                }
                callback->AddPair(proxyA.user_data, proxyB.user_data);
            }
        }

        for (Proxy& proxy : proxies_) {
            proxy.dirty = false;
        }
    }

    void BroadphaseNaive::Query(const FlatAABB& aabb, IQueryCallback& callback) {
        const ProxyID count = static_cast<ProxyID>(proxies_.size());
        for (ProxyID i = 0; i < count; ++i) {
            if (!IsActive(i)) {
                continue;
            }

            Proxy& proxy = proxies_[i];
            if (!FlatAABB::IntersectAABB(aabb, proxy.aabb)) {
                continue;
            }

            if (!callback.ReportProxy(i, proxy.user_data)) {
                break;
            }
        }
    }

    bool BroadphaseNaive::IsValid(ProxyID id) const {
        return id >= 0 && id < static_cast<ProxyID>(proxies_.size());
    }

    bool BroadphaseNaive::IsActive(ProxyID id) const {
        return IsValid(id) && proxies_[id].active;
    }

}