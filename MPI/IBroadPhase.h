#pragma once
#include  "FlatDefs.h"
#include "FlatCallbacks.h"
#include "FlatAABB.h"
namespace FlatPhysics {
	class IBroadPhase {
	public:
		virtual ~IBroadPhase() = default;

		virtual ProxyID CreateProxy(const FlatAABB& aabb, void* user_data) = 0;
		virtual void DestroyProxy(ProxyID id) = 0;
		virtual void MoveProxy(ProxyID id, const FlatAABB& aabb, const Vector2& displacement) = 0;
		virtual void TouchProxy(ProxyID id) = 0;
		virtual void UpdatePairs(IPairCallback* callback) = 0;
		virtual void Query(const FlatAABB& aabb, IQueryCallback& callback) = 0;

	};
}