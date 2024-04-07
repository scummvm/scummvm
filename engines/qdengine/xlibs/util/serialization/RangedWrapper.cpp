#include "stdafx.h"
#include "RangedWrapper.h"

#include "Serialization\Serialization.h"

void RangedWrapperf::clip()
{
	value_ = range_.clip(value_);
}

bool RangedWrapperf::serialize(Archive& ar, const char* name, const char* nameAlt)
{
	const char* typeName = typeid(RangedWrapperf).name();

	if(ar.inPlace())
		return ar.serialize(*valuePointer_, name, nameAlt);

	bool nodeExists;
	if(ar.isEdit()){
		if(ar.openStruct(*this, name, nameAlt)){
			ar.serialize(value_, "value", 0);
			ar.serialize(range_, "range", 0);
			ar.serialize(step_, "step", 0);
			ar.closeStruct(name);
		}
		if(ar.isOutput())
			clip();
		return true;
	}
	else
		nodeExists = ar.serialize(value_, name, nameAlt);

	return nodeExists;
}

void RangedWrapperi::clip()
{
	value_ = range_.clip(value_);
}

bool RangedWrapperi::serialize(Archive& ar, const char* name, const char* nameAlt)
{
	const char* typeName = typeid(RangedWrapperi).name();

	if(ar.inPlace())
		return ar.serialize(*valuePointer_, name, nameAlt);

	bool nodeExists;
	if(ar.isEdit()){
		if(ar.openStruct(*this, name, nameAlt)){
			ar.serialize(value_, "value", 0);
			ar.serialize(range_, "range", 0);
			ar.serialize(step_, "step", 0);
			ar.closeStruct(name);
		}
		if(ar.isOutput())
			clip();
		return true;
	}
	else
		nodeExists = ar.serialize(value_, name, nameAlt);

	return nodeExists;
}
