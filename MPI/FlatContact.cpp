#include "FlatContact.h"
namespace FlatPhysics {
	FlatContactEdge* ContactEdgePool::Allocate()
	{
		if (!free_list.empty()) {
			int idx = free_list.back();
			free_list.pop_back();
			FlatContactEdge* e = &edges[idx];
			e->pool_index = idx;
			e->other = nullptr;
			e->contact_index = -1;
			e->prev = nullptr;
			e->next = nullptr;
			return e;
		}
		int idx = static_cast<int>(edges.size());
		edges.push_back({});
		FlatContactEdge* e = &edges.back();
		e->pool_index = idx;
		e->other = nullptr;
		e->contact_index = -1;
		e->prev = nullptr;
		e->next = nullptr;
		return e;
	}
	void ContactEdgePool::Free(FlatContactEdge* e)
	{
		if (!e) return;

		int idx = e->pool_index;
		if (idx < 0) {
			return;
		}

		free_list.push_back(idx);
		e->pool_index = -1;

		e->other = nullptr;
		e->contact_index = -1;
		e->prev = nullptr;
		e->next = nullptr;
	}
	void ContactEdgePool::Clear()
	{
		edges.clear();
		free_list.clear();
	}
}
