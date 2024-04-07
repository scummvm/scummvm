/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#ifndef __QD_SYSLIB__
//#include "qdscr_parser.h"
#endif

#include "qd_animation_frame.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdAnimationFrame::qdAnimationFrame() : start_time_(0.0f),
	length_(0.0f)
{
}

qdAnimationFrame::qdAnimationFrame(const qdAnimationFrame& frm) : qdSprite(frm),
	start_time_(frm.start_time_),
	length_(frm.length_)
{
}

qdAnimationFrame::~qdAnimationFrame()
{
	free();
}

qdAnimationFrame& qdAnimationFrame::operator = (const qdAnimationFrame& frm)
{
	if(this == &frm) return *this;

	*static_cast<qdSprite*>(this) = frm;

	start_time_ = frm.start_time_;
	length_ = frm.length_;

	return *this;
}

qdAnimationFrame* qdAnimationFrame::clone() const
{
	return new qdAnimationFrame(*this);
}

void qdAnimationFrame::qda_save(class XStream& fh)
{
	fh < static_cast<int>(0) < start_time_ < length_;

	qdSprite::qda_save(fh);
}

void qdAnimationFrame::qda_load(class XStream& fh,int version)
{
	int fl;
	fh > fl > start_time_ > length_;

	qdSprite::qda_load(fh,version);
}

void qdAnimationFrame::qda_load(XZipStream& fh,int version)
{
	int fl;
	fh > fl > start_time_ > length_;

	qdSprite::qda_load(fh,version);
}

bool qdAnimationFrame::load_resources()
{
	if(!load()) return false;

	return true;
}

void qdAnimationFrame::free_resources()
{
	free();
}
