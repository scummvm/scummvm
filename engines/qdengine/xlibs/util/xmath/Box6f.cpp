#include "StdAfx.h"
#include "Box6f.h"

void sBox6f::serialize(Archive& ar)
{
	MergeBlocksAuto mergeBlock(ar);
	ar.serialize(min, "min", "min");
	ar.serialize(max, "max", "max");
}

bool sBox6f::isCrossOrInside(const Se3f& pose, const Vect3f& p0, const Vect3f& p1) const
{
	Vect3f pointBegin;
	Vect3f pointEnd(center());
	pose.xformPoint(pointEnd);
	pointBegin.sub(p0, pointEnd);
	pose.invXformVect(pointBegin);
	pointEnd.sub(p1, pointEnd);
	pose.invXformVect(pointEnd);

	float t_begin, t_end;
	bool inited = false;

	for(int i = 0; i < 3; ++i)
	{
		float begin = pointBegin[i], end = pointEnd[i];
		float halfSize = (max[i] - min[i]) / 2.f;

		if(begin < end) {
			if(begin > halfSize || end < -halfSize)
				return false;
			float shift = end - begin;
			if(shift > FLT_EPS) {
				float beginTemp = (-halfSize - begin) / shift;
				if(!inited || beginTemp > t_begin)
					t_begin = beginTemp;
				float endTemp = (halfSize - begin) / shift;
				if(!inited || endTemp < t_end)
					t_end = endTemp;
				inited = true;
			}

		}
		else {
			if(end > halfSize || begin < -halfSize)
				return false;
			float shift = end - begin;
			if(shift < -FLT_EPS) {
				float beginTemp = (halfSize - begin) / shift;
				if(!inited || beginTemp > t_begin)
					t_begin = beginTemp;
				float endTemp = (-halfSize - begin) / shift;
				if(!inited || endTemp < t_end)
					t_end = endTemp;
				inited = true;
			}
		}
	}

	return !inited || t_end >= t_begin;
}
