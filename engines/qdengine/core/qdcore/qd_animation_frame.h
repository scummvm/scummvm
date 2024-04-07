#ifndef __QD_ANIMATION_FRAME_H__
#define __QD_ANIMATION_FRAME_H__

#include "qd_sprite.h"

//! Кадр анимации.
class qdAnimationFrame : public qdSprite
{
public:
	qdAnimationFrame();
	qdAnimationFrame(const qdAnimationFrame& frm);
	~qdAnimationFrame();

	qdAnimationFrame& operator = (const qdAnimationFrame& frm);

	qdAnimationFrame* clone() const;

	float start_time() const { return start_time_; }
	float end_time() const { return start_time_ + length_; }
	float length() const { return length_; }

	void set_start_time(float tm){ start_time_ = tm; }
	void set_length(float tm){ length_ = tm; }

	virtual void qda_load(class XStream& fh,int version = 100);
	virtual void qda_load(class XZipStream& fh,int version = 100);
	virtual void qda_save(class XStream& fh);

	bool load_resources();
	void free_resources();

private:
	float start_time_;
	float length_;
};

typedef std::list<qdAnimationFrame*> qdAnimationFrameList;

#endif /* __QD_ANIMATION_FRAME_H__ */

