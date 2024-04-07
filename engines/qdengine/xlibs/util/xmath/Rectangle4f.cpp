#include "StdAfx.h"
#include "Rectangle4f.h"

const sRectangle4f sRectangle4f::ID(0.0f, 0.0f, 1.0f, 1.0f);

void sRectangle4f::serialize(Archive& ar)
{
	MergeBlocksAuto mergeBlock(ar);
	ar.serialize(min, "min", "min");
	ar.serialize(max, "max", "max");
}
