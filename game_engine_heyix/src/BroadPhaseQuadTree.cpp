#include "BroadPhaseQuadTree.h"
namespace FlatPhysics {
	ProxyID FlatPhysics::BroadPhaseQuadTree::CreateProxy(const FlatAABB& aabb, void* user_data)
	{
		return ProxyID();
	}

	void FlatPhysics::BroadPhaseQuadTree::DestroyProxy(ProxyID id)
	{
	}

	void FlatPhysics::BroadPhaseQuadTree::MoveProxy(ProxyID id, const FlatAABB& aabb, const Vector2& displacement)
	{
	}

	void FlatPhysics::BroadPhaseQuadTree::TouchProxy(ProxyID id)
	{
	}

	void FlatPhysics::BroadPhaseQuadTree::UpdatePairs(IPairCallback* callback)
	{
	}

	void FlatPhysics::BroadPhaseQuadTree::Query(const FlatAABB& aabb, IQueryCallback& callback)
	{
	}
}

