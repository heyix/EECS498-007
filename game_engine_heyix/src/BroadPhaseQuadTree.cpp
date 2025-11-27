#include "BroadPhaseQuadTree.h"
#include <algorithm>
#include "FlatHelper.h"
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
		: max_depth_(max_depth)
		, max_leaf_capacity_(max_leaf_capacity)
		, loose_factor_(std::max(loose_factor, 0.0f))
		, root_(nullptr)
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

		destroyed_since_rebuild_++;
		int total_slots = proxies_.size();
		int free_slots = free_list_.size();
		int active_count = GetActiveCount();

		int total_count = active_count + destroyed_since_rebuild_;
		if (total_count > 0 &&
			destroyed_since_rebuild_ >= proportion_detroyed_objects_needed_to_build * total_count) {
			tree_dirty_ = true;
		}
	}

	void FlatPhysics::BroadPhaseQuadTree::MoveProxy(ProxyID id, const FlatAABB& aabb, const Vector2& displacement)
	{
		if (!IsActive(id))return;
		Proxy& proxy = proxies_[id];
		proxy.tight_aabb = aabb;
		if (proxy.fat_aabb.Contains(proxy.tight_aabb)) {
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

	void BroadPhaseQuadTree::UpdatePairs(IPairCallback* callback)
	{
		if (!callback) return;

		int target_capacity = ComputeTargetLeafCapacity();
		if (target_capacity != max_leaf_capacity_) {
			max_leaf_capacity_ = target_capacity;
			tree_dirty_ = true;
		}
		int target_depth = ComputeTargetMaxDepth();
		if (target_depth != max_depth_) {
			max_depth_ = target_depth;
			tree_dirty_ = true;
		}

		FlushDirty();
		if (!root_) return;

		//MeasureTime("Broadphase", [this,&callback]() {
			const ProxyID count = static_cast<ProxyID>(proxies_.size());

			bool print_info = false;
			std::vector<int> scanned;
			using UserPair = std::pair<void*, void*>;
#pragma omp parallel for schedule(dynamic,64)
			for (ProxyID i = 0; i < count; i++) {
				if (!IsActive(i))continue;
				Proxy& proxyA = proxies_[i];
				QueryNode(root_, proxyA.fat_aabb, [&](ProxyID otherId) {
					if (print_info) scanned[i]++;
					if (otherId <= i || !IsActive(otherId)) {
						return true;
					}

					Proxy& proxyB = proxies_[otherId];
					if (!FlatAABB::IntersectAABB(proxyA.tight_aabb, proxyB.tight_aabb)) {
						return true;
					}

					callback->AddPair(proxyA.user_data, proxyB.user_data);
					return true;
					});
			}
			if (print_info) {
				long long total_scanned = 0;
				int active_proxies = 0;
				int max_scanned = 0;
				ProxyID max_proxy = -1;

				for (ProxyID i = 0; i < count; ++i) {
					if (!IsActive(i)) continue;

					total_scanned += scanned[i];
					active_proxies++;

					if (scanned[i] > max_scanned) {
						max_scanned = scanned[i];
						max_proxy = i;
					}
				}

				double avg = (active_proxies > 0) ? (double)total_scanned / active_proxies : 0.0;

				printf("Broadphase scan summary:\n");
				printf("  Active proxies    = %d\n", active_proxies);
				printf("  Total scanned     = %lld\n", total_scanned);
				printf("  Average per proxy = %.2f\n", avg);
				if (max_proxy >= 0)
					printf("  Max scanned       = %d (proxy %d)\n", max_scanned, max_proxy);
				printf("----------------------------------\n");
			}
		//});
		
	}

	void BroadPhaseQuadTree::Query(const FlatAABB& aabb, IQueryCallback& callback)
	{
		FlushDirty();
		if (!root_) {
			return;
		}

		QueryNode(root_, aabb, [&](ProxyID id) {
			if (!IsActive(id)) {
				return true;
			}
			Proxy& proxy = proxies_[id];
			if (!FlatAABB::IntersectAABB(aabb, proxy.tight_aabb)) {
				return true;
			}
			return callback.ReportProxy(id, proxy.user_data);
			});
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
	int BroadPhaseQuadTree::GetMaxDepth(const Node* node)
	{
		if (!node) return 0;

		int maxDepth = node->depth;

		if (node->IsLeaf()) {
			return maxDepth;
		}

		for (int i = 0; i < 4; ++i) {
			if (node->children[i]) {
				int childDepth = GetMaxDepth(node->children[i]);
				if (childDepth > maxDepth) {
					maxDepth = childDepth;
				}
			}
		}
		return maxDepth;
	}
	void BroadPhaseQuadTree::PrintLevelItemCounts() const
	{
		if (!root_) {
			printf("Tree is empty.\n");
			return;
		}

		struct LevelInfo {
			int nodes_with_items = 0;
			int total_items = 0;
		};

		std::unordered_map<int, LevelInfo> levelInfo;

		std::deque<const Node*> q;
		q.push_back(root_);

		while (!q.empty()) {
			const Node* node = q.front();
			q.pop_front();

			int itemCount = static_cast<int>(node->items.size());
			if (itemCount > 0) {
				LevelInfo& info = levelInfo[node->depth];
				info.nodes_with_items++;   // only count nodes that have ≥1 item
				info.total_items += itemCount;
			}

			if (!node->IsLeaf()) {
				for (int i = 0; i < 4; ++i) {
					if (node->children[i]) {
						q.push_back(node->children[i]);
					}
				}
			}
		}

		// Sort depths
		std::vector<int> depths;
		depths.reserve(levelInfo.size());
		for (auto& p : levelInfo) depths.push_back(p.first);
		std::sort(depths.begin(), depths.end());

		printf("===== QuadTree Nodes With Items =====\n");
		for (int d : depths) {
			const LevelInfo& info = levelInfo[d];
			printf("Depth %d : nodes=%d  total_items=%d\n",
				d, info.nodes_with_items, info.total_items);
		}
		printf("=====================================\n");
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
			Node* root = node_pool_.Allocate();
			FlatAABB base = NormalizedBounds(aabb);
			FlatAABB expanded = FlatAABB::ExpandAroundCenter(base, root_padding_factor_);
			root->bounds = NormalizedBounds(expanded);
			root->depth = 0;
			root_ = root;
			tree_dirty_ = true;
			return;
		}
		
		if (root_->bounds.Contains(aabb)) {
			return;
		}

		FlatAABB b = root_->bounds;

		while (!b.Contains(aabb)) {
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
		node_pool_.Reset();

		FlatAABB combined;
		bool has_proxy = false;
		for (Proxy& proxy : proxies_) {
			if (!proxy.active) {
				continue;
			}
			proxy.fat_aabb = MakeFatAABB(proxy.tight_aabb);
			if (!has_proxy) {
				combined = proxy.fat_aabb;
				has_proxy = true;
			}
			else {
				combined.UnionWith(proxy.fat_aabb);
			}
		}
		if (!has_proxy) {
			root_ = nullptr; 
			destroyed_since_rebuild_ = 0;
			return;
		}

		Node* root = node_pool_.Allocate(); 
		FlatAABB expanded = FlatAABB::ExpandAroundCenter(combined, root_padding_factor_);
		root->bounds = NormalizedBounds(expanded);
		root->depth = 0;
		root_ = root; 

		for (ProxyID id = 0; id < static_cast<ProxyID>(proxies_.size()); id++) {
			if (!IsActive(id)) {
				continue;
			}
			Proxy& proxy = proxies_[id];
			proxy.owner = nullptr;
			proxy.dirty = false;
			InsertIntoNode(root_, id);  
		}
		destroyed_since_rebuild_ = 0;
	}
	void BroadPhaseQuadTree::FlushDirty()
	{
		if (tree_dirty_) {
			RebuildTree();
			tree_dirty_ = false;
			dirty_list_.clear();
			return;
		}
		if (!root_) {
			dirty_list_.clear();
			return;
		}
		for (ProxyID id : dirty_list_) {
			if (!IsActive(id)) {
				continue;
			}
			Proxy& proxy = proxies_[id];
			if (!proxy.dirty) {//it's possible: mark dirty->destroy->reused->mark dirty, then two copies of same id will exist in the dirty list
				continue;
			}
			RemoveFromOwner(id);
			proxy.dirty = false;
			InsertIntoNode(root_, id);
		}
		dirty_list_.clear();
	}
	void BroadPhaseQuadTree::InsertIntoNode(Node* node, ProxyID id)
	{
		if (!node) {
			return;
		}
		Proxy& proxy = proxies_[id];
		if (node->IsLeaf()) {
			if (node->items.size() < static_cast<size_t>(max_leaf_capacity_) || node->depth >= max_depth_) {
				node->items.push_back(id);
				proxy.owner = node;
				return;
			}
			for (int i = 0; i < 4; i++) {
				Node* child = node_pool_.Allocate();
				child->bounds = ChildBounds(node->bounds, i);
				child->depth = node->depth + 1;
				node->children[i] = child;
			}
			std::vector<ProxyID> to_reinsert = std::move(node->items);
			node->items.clear();
			for (ProxyID stored_id : to_reinsert) {
				proxies_[stored_id].owner = nullptr;
				InsertIntoNode(node, stored_id);
			}
		}
		int child_index = SelectChild(node, proxy.fat_aabb);
		if (child_index == -1) {
			node->items.push_back(id);
			proxy.owner = node;
		}
		else {
			InsertIntoNode(node->children[child_index], id);
		}

	}
	void BroadPhaseQuadTree::RemoveFromOwner(ProxyID id)
	{
		if (!IsActive(id)) {
			return;
		}
		Proxy& proxy = proxies_[id];
		Node* owner = proxy.owner;
		if (!owner) {
			return;
		}
		auto& items = owner->items;
		auto it = std::find(items.begin(), items.end(), id);
		if (it != items.end()) {
			*it = items.back();
			items.pop_back();
		}
		proxy.owner = nullptr;
	}
	int BroadPhaseQuadTree::SelectChild(const Node* node, const FlatAABB& aabb) const
	{
		for (int i = 0; i < 4; ++i) {
			const Node* child = node->children[i];
			if (!child) continue;

			if (child->bounds.Contains(aabb)) {
				return i;
			}
		}
		return -1;
	}
	FlatAABB BroadPhaseQuadTree::ChildBounds(const FlatAABB& parent, int child_index) const
	{
		const Vector2 center{
			(parent.min.x() + parent.max.x()) * 0.5f,
			(parent.min.y() + parent.max.y()) * 0.5f
		};

		switch (child_index) {
		case 0:
			return FlatAABB(parent.min.x(), center.y(), center.x(), parent.max.y());
		case 1:
			return FlatAABB(center.x(), center.y(), parent.max.x(), parent.max.y());
		case 2:
			return FlatAABB(parent.min.x(), parent.min.y(), center.x(), center.y());
		default:
			return FlatAABB(center.x(), parent.min.y(), parent.max.x(), center.y());
		}
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
	int BroadPhaseQuadTree::GetActiveCount() const
	{
		int total_slots = static_cast<int>(proxies_.size());
		int free_slots = static_cast<int>(free_list_.size());
		return total_slots - free_slots;
	}

	int BroadPhaseQuadTree::ComputeTargetMaxDepth() const
	{
		int active = GetActiveCount();
		if (active <= 0) {
			return max_depth_;
		}

		if (active < 1000) {
			return 8; 
		}
		else if (active < 3000) {
			return 9;
		}
		else if (active < 10000) {
			return 10;
		}
		else {
			return 11;
		}
	}

	int BroadPhaseQuadTree::ComputeTargetLeafCapacity() const
	{
		int active = GetActiveCount();

		if (active < 1000) {
			return 8; 
		}
		else if (active < 5000) {
			return 12;
		}
		else {
			return 14;
		}
	}

}

