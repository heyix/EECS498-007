#include "BroadPhaseQuadTree.h"
namespace FlatPhysics {
	BroadPhaseQuadTree::BroadPhaseQuadTree(int max_depth, int max_leaf_capacity)
	{
	}
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
	void BroadPhaseQuadTree::SetLooseFactor(float factor)
	{
	}
	float BroadPhaseQuadTree::GetLooseFactor() const
	{
		return 0.0f;
	}
	bool BroadPhaseQuadTree::Node::IsLeaf() const
	{
		return false;
	}
	bool BroadPhaseQuadTree::IsValid(ProxyID id) const
	{
		return false;
	}
	bool BroadPhaseQuadTree::IsActive(ProxyID id) const
	{
		return false;
	}
	void BroadPhaseQuadTree::EnsureRoot(const FlatAABB& aabb)
	{
	}
	void BroadPhaseQuadTree::RebuildTree()
	{
	}
	void BroadPhaseQuadTree::FlushDirty()
	{
	}
	void BroadPhaseQuadTree::InsertIntoNode(Node* node, ProxyID id)
	{
	}
	void BroadPhaseQuadTree::RemoveFromOwner(ProxyID id)
	{
	}
	void BroadPhaseQuadTree::QueryNode(const Node* node, const FlatAABB& aabb, const std::function<bool(ProxyID)>& visitor) const
	{
	}
	int BroadPhaseQuadTree::SelectChild(const Node* node, const FlatAABB& aabb) const
	{
		return 0;
	}
	bool BroadPhaseQuadTree::Contains(const FlatAABB& outer, const FlatAABB& inner) const
	{
		return false;
	}
	FlatAABB BroadPhaseQuadTree::ChildBounds(const FlatAABB& parent, int child_index) const
	{
		return FlatAABB();
	}
	FlatAABB BroadPhaseQuadTree::MakeFatAABB(const FlatAABB& tight) const
	{
		return FlatAABB();
	}
}

