#pragma once
#include "IBroadPhase.h"
#include <memory>
#include <array>
#include<functional>
namespace FlatPhysics {
	class BroadPhaseQuadTree : public IBroadPhase {
    public:
        BroadPhaseQuadTree(int max_depth = 6, int max_leaf_capacity = 4);
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
    private:
        struct Node {
            FlatAABB bounds;
            std::array<std::unique_ptr<Node>, 4> children{};
            std::vector<ProxyID> items;
            int depth = 0;
            bool IsLeaf()const;
        };
        struct Proxy {
            FlatAABB tight_aabb{ 0,0,0,0 };
            FlatAABB fat_aabb{ 0,0,0,0 };
            void* user_data{ nullptr };
            Node* owner{ nullptr };
            bool dirty = false;
            bool active = false;
        };

        bool IsValid(ProxyID id)const;
        bool IsActive(ProxyID id)const;

        void EnsureRoot(const FlatAABB& aabb);
        void RebuildTree();
        void FlushDirty();
        void InsertIntoNode(Node* node, ProxyID id);
        void RemoveFromOwner(ProxyID id);
        void QueryNode(const Node* node, const FlatAABB& aabb, const std::function<bool(ProxyID)>& visitor)const;
        int SelectChild(const Node* node, const FlatAABB& aabb)const;
        bool Contains(const FlatAABB& outer, const FlatAABB& inner)const;
        FlatAABB ChildBounds(const FlatAABB& parent, int child_index)const;
        FlatAABB MakeFatAABB(const FlatAABB& tight)const;

    private:
        std::unique_ptr<Node> root_;
        std::vector<Proxy> proxies_;
        std::vector<ProxyID> free_list_;
        std::vector<ProxyID> dirty_list_;

        int max_depth_;
        int max_leaf_capacity_;
        float loose_factor_;
        bool tree_dirty_{ false };

	};

}