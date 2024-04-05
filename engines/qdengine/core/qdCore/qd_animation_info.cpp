/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#ifndef __QD_SYSLIB__
#include "xml_tag_buffer.h"
#include "qdscr_parser.h"

#include "qd_game_scene.h"
#include "qd_game_dispatcher.h"
#endif

#include "qd_named_object.h"
#include "qd_animation_info.h"
#include "qd_animation.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdAnimationInfo::qdAnimationInfo() : speed_(0.0f), animation_speed_(1.0f)
{ 
}

qdAnimationInfo::qdAnimationInfo(const qdAnimationInfo& p) : qdNamedObject(p),
	speed_(p.speed_),
	animation_speed_(p.animation_speed_)
{
}

qdAnimationInfo::~qdAnimationInfo()
{
}

void qdAnimationInfo::load_script(const xml::tag* p)
{
#ifndef __QD_SYSLIB__
	int fl;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_SPEED:
			xml::tag_buffer(*it) > speed_;
			break;
		case QDSCR_ANIMATION_SPEED:
			xml::tag_buffer(*it) > animation_speed_;
			break;
		case QDSCR_ANIMATION:
			set_animation_name(it -> data());
			break;
		case QDSCR_FLAG:
			xml::tag_buffer(*it) > fl;
			set_flag(fl);
			break;
		}
	}
#endif
}

bool qdAnimationInfo::save_script(XStream& fh,int indent) const
{
#ifndef __QD_SYSLIB__
	for(int i = 0; i < indent; i ++) fh < "\t";

	fh < "<animation_info";

	if(flags())
		fh < " flags=\"" <= flags() < "\"";

	if(speed_ > 0.01f)
		fh < " speed=\"" <= speed_ < "\"";

	if(animation_speed_ != 1.0f)
		fh < " animation_speed=\"" <= animation_speed_ < "\"";

	if(animation_name())
		fh < " animation=\"" < qdscr_XML_string(animation_name()) < "\"";

	fh < "/>\r\n";
#endif
	return true;
}

qdAnimationInfo& qdAnimationInfo::operator = (const qdAnimationInfo& p)
{
	if(this == &p) return *this;

	*static_cast<qdNamedObject*>(this) = p;

	speed_ = p.speed_; 
	animation_speed_ = p.animation_speed_;

	return *this;
}

qdAnimation* qdAnimationInfo::animation() const
{
#ifndef __QD_SYSLIB__
	if(animation_name()){
		if(qdGameScene* p = static_cast<qdGameScene*>(owner(QD_NAMED_OBJECT_SCENE))){
			if(qdAnimation* anm = p -> get_animation(animation_name()))
				return anm;
		}

		if(qdGameDispatcher* p = qd_get_game_dispatcher())
			return p -> get_animation(animation_name());
	}
#endif
	return NULL;
}

