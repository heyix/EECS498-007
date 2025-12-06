#include "FlatAABB.h"
namespace FlatPhysics {
	bool FlatAABB::Overlaps(const FlatAABB& other) const
	{
		return IntersectAABB(*this, other);
	}

	bool FlatAABB::Contains(const FlatAABB& inner) const
	{
		return min.x() <= inner.min.x() &&
			min.y() <= inner.min.y() &&
			max.x() >= inner.max.x() &&
			max.y() >= inner.max.y();
	}

	void FlatAABB::UnionWith(const FlatAABB& other)
	{
		min.x() = std::min(min.x(), other.min.x());
		min.y() = std::min(min.y(), other.min.y());
		max.x() = std::max(max.x(), other.max.x());
		max.y() = std::max(max.y(), other.max.y());
	}

	bool FlatAABB::IntersectAABB(const FlatAABB& a, const FlatAABB& b)
	{
		if (a.max.x() <= b.min.x() || b.max.x() <= a.min.x() ||
			a.max.y() <= b.min.y() || b.max.y() <= a.min.y()) {
			return false;
		}
		return true;
	}

	FlatAABB FlatAABB::Union(const FlatAABB& a, const FlatAABB& b)
	{
		return FlatAABB(
			std::min(a.min.x(), b.min.x()),
			std::min(a.min.y(), b.min.y()),
			std::max(a.max.x(), b.max.x()),
			std::max(a.max.y(), b.max.y())
		);
	}

	FlatAABB FlatAABB::ExpandAroundCenter(const FlatAABB& src, float factor)
	{
		const Vector2 center{
		(src.min.x() + src.max.x()) * 0.5f,
		(src.min.y() + src.max.y()) * 0.5f
		};
		Vector2 half{
			(src.max.x() - src.min.x()) * 0.5f,
			(src.max.y() - src.min.y()) * 0.5f
		};
		half *= factor;

		return FlatAABB(
			center.x() - half.x(), center.y() - half.y(),
			center.x() + half.x(), center.y() + half.y()
		);
	}

}
