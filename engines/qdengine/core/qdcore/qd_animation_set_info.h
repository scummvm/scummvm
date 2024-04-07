
#ifndef __QD_ANIMATION_SET_INFO_H__
#define __QD_ANIMATION_SET_INFO_H__

#include "qd_named_object.h"

class qdAnimationSet;

//! Информация о наборе анимаций.
class qdAnimationSetInfo : public qdNamedObject
{
public:
	qdAnimationSetInfo(){ }
	~qdAnimationSetInfo(){ }

	int named_object_type() const { return QD_NAMED_OBJECT_ANIMATION_SET_INFO; }

	qdAnimationSet* animation_set() const;
};

#endif /* __QD_ANIMATION_SET_INFO_H__ */
