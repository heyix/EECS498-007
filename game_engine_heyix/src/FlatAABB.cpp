#include "FlatAABB.h"

bool FlatPhysics::FlatAABB::IntersectAABB(const FlatAABB& a, const FlatAABB& b)
{
	if (a.max.x() <= b.min.x() || b.max.x() <= a.min.x()||
		a.max.y() <= b.min.y() || b.max.y() <= a.min.y()) {
		return false;
	}
	return true;
}
