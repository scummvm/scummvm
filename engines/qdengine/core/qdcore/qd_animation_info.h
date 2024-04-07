#ifndef __QD_ANIMATION_INFO_H__
#define __QD_ANIMATION_INFO_H__

#include "xml_fwd.h"
#include "qd_named_object.h"

class XStream;
class qdAnimation;

//! Информация об анимации.
class qdAnimationInfo : public qdNamedObject
{
public:
	qdAnimationInfo();
	qdAnimationInfo(const qdAnimationInfo& p);
	~qdAnimationInfo();

	qdAnimationInfo& operator = (const qdAnimationInfo& p);

	int named_object_type() const { return QD_NAMED_OBJECT_ANIMATION_INFO; }

	float speed() const { return speed_; }
	void set_speed(float sp){ speed_ = sp; }

	float animation_speed() const { return animation_speed_; }
	void set_animation_speed(float sp){ animation_speed_ = sp; }

	qdAnimation* animation() const;
	const char* animation_name() const { return name(); }

	void load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	void set_animation_name(const char* name){ set_name(name); }
	void free_animation_name(){ set_name(0); }

private:

	float speed_;
	float animation_speed_;
};

typedef std::vector<qdAnimationInfo> qdAnimationInfoVector;

#endif /* __QD_ANIMATION_INFO_H__ */

