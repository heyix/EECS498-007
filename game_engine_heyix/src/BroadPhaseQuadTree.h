#pragma once
#include "IBroadPhase.h"
namespace FlatPhysics {
	class BroadPhaseQuadTree : public IBroadPhase {
	public:
        ProxyID CreateProxy(const FlatAABB& aabb, void* user_data) override;
        void DestroyProxy(ProxyID id) override;
        void MoveProxy(ProxyID id, const FlatAABB& aabb, const Vector2& displacement) override;
        void TouchProxy(ProxyID id) override;
        void UpdatePairs(IPairCallback* callback) override;
        void Query(const FlatAABB& aabb, IQueryCallback& callback) override;
	};
}