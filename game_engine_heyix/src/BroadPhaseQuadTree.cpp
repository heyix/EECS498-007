#include "BroadPhaseQuadTree.h"
#include <algorithm>
namespace FlatPhysics {
	namespace {
		constexpr float K_MIN_EXTENT = 0.5f;
		float ClampExtent(float value) {
			return std::max(value, K_MIN_EXTENT);
		}
		FlatAABB NormalizedBounds(const FlatAABB& src) {
			const Vector2 center{
				(src.min.x() + src.max.x()) * 0.5f,
				(src.min.y() + src.max.y()) * 0.5f
			};
			Vector2 half{
			  ClampExtent((src.max.x() - src.min.x()) * 0.5f),
			  ClampExtent((src.max.y() - src.min.y()) * 0.5f)
			};
			return FlatAABB(
				center.x() - half.x(), center.y() - half.y(),
				center.x() + half.x(), center.y() + half.y()
			);
		}
	}
	BroadPhaseQuadTree::BroadPhaseQuadTree(int max_depth, int max_leaf_capacity, float loose_factor)
		:max_depth_(max_depth),max_leaf_capacity_(max_leaf_capacity),loose_factor_(std::max(loose_factor,0.0f))
	{
	}
	ProxyID FlatPhysics::BroadPhaseQuadTree::CreateProxy(const FlatAABB& aabb, void* user_data)
	{
		Proxy proxy;
		proxy.tight_aabb = aabb;
		proxy.fat_aabb = MakeFatAABB(aabb);
		proxy.user_data = user_data;
		proxy.active = true;
		proxy.dirty = true;

		ProxyID id;
		if (!free_list_.empty()) {
			id = free_list_.back();
			free_list_.pop_back();
			proxies_[id] = proxy;
		}
		else {
			proxies_.push_back(proxy);
			id = static_cast<ProxyID>(proxies_.size() - 1);
		}
		dirty_list_.push_back(id);
		EnsureRoot(proxy.fat_aabb);
		return id;
	}

	void FlatPhysics::BroadPhaseQuadTree::DestroyProxy(ProxyID id)
	{
		if (!IsActive(id)) {
			return;
		}
		RemoveFromOwner(id);
		proxies_[id] = Proxy{};
		proxies_[id].active = false;
		free_list_.push_back(id);
		tree_dirty_ = true;
	}

	void FlatPhysics::BroadPhaseQuadTree::MoveProxy(ProxyID id, const FlatAABB& aabb, const Vector2& displacement)
	{
		if (!IsActive(id))return;
		Proxy& proxy = proxies_[id];
		proxy.tight_aabb = aabb;
		if (Contains(proxy.fat_aabb, proxy.tight_aabb)) {
			return;
		}
		proxy.fat_aabb = MakeFatAABB(proxy.tight_aabb);
		if (!proxy.dirty) {
			proxy.dirty = true;
			dirty_list_.push_back(id);
		}
		EnsureRoot(proxy.fat_aabb);
	}

	void FlatPhysics::BroadPhaseQuadTree::TouchProxy(ProxyID id)
	{
		if (!IsActive(id)) {
			return;
		}
		Proxy& proxy = proxies_[id];
		if (!proxy.dirty) {
			proxy.dirty = true;
			dirty_list_.push_back(id);
		}
	}

	void FlatPhysics::BroadPhaseQuadTree::UpdatePairs(IPairCallback* callback)
	{
		if (!callback) {
			return;
		}
		FlushDirty();
		if (!root_) {
			return;
		}
		const ProxyID count = static_cast<ProxyID>(proxies_.size());
		for (ProxyID i = 0; i < count; i++) {
			if (!IsActive(i)) {
				continue;
			}
			Proxy& proxyA = proxies_[i];
			QueryNode(root_.get(), proxyA.fat_aabb, [&](ProxyID otherId) {
				if (otherId <= i || !IsActive(otherId)) {
					return true;
				}
				Proxy& proxyB = proxies_[otherId];
				if (!FlatAABB::IntersectAABB(proxyA.tight_aabb, proxyB.tight_aabb)) {
					return true;
				}
				callback->AddPair(proxyA.user_data, proxyB.user_data);
				return true;
			}
			);
		}
	}

	void FlatPhysics::BroadPhaseQuadTree::Query(const FlatAABB& aabb, IQueryCallback& callback)
	{
		FlushDirty();
		if (!root_) {
			return;
		}
		bool continue_search = true;
		QueryNode(root_.get(), aabb, [&](ProxyID id) {
			if (!IsActive(id)) {
				return true;
			}
			Proxy& proxy = proxies_[id];
			if (!FlatAABB::IntersectAABB(aabb, proxy.tight_aabb)) {
				return true;
			}
			continue_search = callback.ReportProxy(id, proxy.user_data);
			return continue_search;
		}
		);
	}
	void BroadPhaseQuadTree::SetLooseFactor(float factor)
	{
		loose_factor_ = std::max(factor, 0.0f);
		tree_dirty_ = true;
	}
	float BroadPhaseQuadTree::GetLooseFactor() const
	{
		return loose_factor_;
	}
	bool BroadPhaseQuadTree::Node::IsLeaf() const
	{
		return !children[0];
	}
	bool BroadPhaseQuadTree::IsValid(ProxyID id) const
	{
		return id >= 0 && id < static_cast<ProxyID>(proxies_.size());
	}
	bool BroadPhaseQuadTree::IsActive(ProxyID id) const
	{
		return IsValid(id) && proxies_[id].active;
	}
	void BroadPhaseQuadTree::EnsureRoot(const FlatAABB& aabb)
	{
		if (!root_) {
			root_ = std::make_unique<Node>();
			root_->bounds = NormalizedBounds(aabb);
			root_->depth = 0;
			tree_dirty_ = true;
			return;
		}

		if (Contains(root_->bounds, aabb)) {
			return;
		}

		FlatAABB b = root_->bounds;

		while (!Contains(b, aabb)) {
			const float width = b.max.x() - b.min.x();
			const float height = b.max.y() - b.min.y();

			float minX = b.min.x();
			float maxX = b.max.x();
			float minY = b.min.y();
			float maxY = b.max.y();

			const float expandX = width * (root_expand_factor_ - 1.0f);
			const float expandY = height * (root_expand_factor_ - 1.0f);

			if (aabb.max.x() > maxX) {
				maxX += expandX;
			}
			else if (aabb.min.x() < minX) {
				minX -= expandX;
			}

			if (aabb.max.y() > maxY) {
				maxY += expandY;
			}
			else if (aabb.min.y() < minY) {
				minY -= expandY;
			}

			b = FlatAABB(minX, minY, maxX, maxY);
		}

		root_->bounds = NormalizedBounds(b);
		tree_dirty_ = true;
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
		if (loose_factor_ <= 0.0f) {
			return tight;
		}
		const Vector2 center{
			(tight.min.x() + tight.max.x()) * 0.5f,
			(tight.min.y() + tight.max.y()) * 0.5f
		};
		Vector2 half{
			(tight.max.x() - tight.min.x()) * 0.5f,
			(tight.max.y() - tight.min.y()) * 0.5f
		};
		half *= (1.0f + loose_factor_);

		return FlatAABB(
			center.x() - half.x(), center.y() - half.y(),
			center.x() + half.x(), center.y() + half.y()
		);
	}
}

