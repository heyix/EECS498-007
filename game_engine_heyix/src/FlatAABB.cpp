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

}
