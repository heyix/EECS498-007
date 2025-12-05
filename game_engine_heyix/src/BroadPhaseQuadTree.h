#pragma once
#include "IBroadPhase.h"
#include <array>
#include <algorithm>
#include <deque>
namespace FlatPhysics {
    class BroadPhaseQuadTree : public IBroadPhase {
    public:
        BroadPhaseQuadTree(int max_depth = 8, int max_leaf_capacity = 4, float loose_factor = 0.2f);
    private:
        struct Node {
            std::array<Node*, 4> children{};
            std::vector<ProxyID> items;
            FlatAABB bounds;
            int depth = 0;
            bool IsLeaf()const;
        };
        struct Proxy {
            void* user_data{ nullptr };
            Node* owner{ nullptr };
            FlatAABB tight_aabb{ 0,0,0,0 };
            FlatAABB fat_aabb{ 0,0,0,0 };
            bool dirty = false;
            bool active = false;
            int   owner_index = -1;
        };
        struct NodePool {
            std::deque<Node> nodes;
            std::size_t used = 0;
            void Reset() {
                used = 0;
            }
            Node* Allocate() {
                if (used == nodes.size()){
                    nodes.emplace_back();
                }
                Node* node = &nodes[used++];
                node->bounds = FlatAABB{ 0,0,0,0 };
                node->items.clear();
                node->depth = 0;
                for (int i = 0; i < node->children.size(); i++) {
                    node->children[i] = nullptr;
                }
                return node;
            }
        };
	public:
        ProxyID CreateProxy(const FlatAABB& aabb, void* user_data) override;
        void DestroyProxy(ProxyID id) override;
        void MoveProxy(ProxyID id, const FlatAABB& aabb, const Vector2& displacement) override;
        void TouchProxy(ProxyID id) override;
        void UpdatePairs(IPairCallback* callback) override;
        void Query(const FlatAABB& aabb, IQueryCallback& callback) override;

    public:
        void SetLooseFactor(float factor);
        float GetLooseFactor()const;
        void SetRootExpandFactor(float factor) { root_expand_factor_ = std::max(factor, 1.01f); }
        float GetRootExpandFactor() { return root_expand_factor_; }
        void SetRootPaddingFactor(float f) { root_padding_factor_ = std::max(f, 1.0f); }
        float GetRootPaddingFactor() const { return root_padding_factor_; }
        int GetMaxDepth(const Node* node);
        void PrintLevelItemCounts() const;
    private:
       

        bool IsValid(ProxyID id)const;
        bool IsActive(ProxyID id)const;

        void EnsureRoot(const FlatAABB& aabb);
        void RebuildTree();
        void FlushDirty();
        void InsertIntoNode(Node* node, ProxyID id);
        void RemoveFromOwner(ProxyID id);
        template<typename Visitor>
        bool QueryNode(const Node* node, const FlatAABB& aabb, Visitor&& visitor) const;
        int SelectChild(const Node* node, const FlatAABB& aabb)const;
        FlatAABB ChildBounds(const FlatAABB& parent, int child_index)const;
        FlatAABB MakeFatAABB(const FlatAABB& tight)const;
        int GetActiveCount() const;
        int ComputeTargetMaxDepth() const;
        int ComputeTargetLeafCapacity() const;
    private:
        Node* root_ = nullptr;
        NodePool node_pool_;
        std::vector<Proxy> proxies_;
        std::vector<ProxyID> free_list_;
        std::vector<ProxyID> dirty_list_;

        int max_depth_;
        int max_leaf_capacity_;
        float loose_factor_;
        bool tree_dirty_{ false };
        float root_expand_factor_ = 2.0f;
        float root_padding_factor_ = 1.5f;
        int destroyed_since_rebuild_ = 0;
        float proportion_detroyed_objects_needed_to_build = 0.5f;
	};
    template<typename Visitor>
    bool BroadPhaseQuadTree::QueryNode(const Node* node, const FlatAABB& aabb, Visitor&& visitor) const
    {
        if (!node || !FlatAABB::IntersectAABB(node->bounds, aabb)) {
            return true;
        }

        for (ProxyID id : node->items) {
            if (!visitor(id)) {
                return false;
            }
        }

        if (node->IsLeaf()) {
            return true; 
        }

        for (const Node* child : node->children) {
            if (!child) continue;
            if (!QueryNode(child, aabb, visitor)) {
                return false; 
            }
        }

        return true;
    }
}