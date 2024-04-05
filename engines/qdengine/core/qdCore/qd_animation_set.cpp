/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag_buffer.h"

#include "qdscr_parser.h"
#include "qd_animation_set.h"
#include "qd_game_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdAnimationSet::qdAnimationSet()
{
	start_angle_ = 0.0f;
}

qdAnimationSet::qdAnimationSet(const qdAnimationSet& set) : qdNamedObject(set),
	start_angle_(set.start_angle_),
	animations_(set.animations_),
	walk_sound_frequency_(set.walk_sound_frequency_),
	static_animations_(set.static_animations_),
	start_animations_(set.start_animations_),
	stop_animations_(set.stop_animations_),
	turn_animation_(set.turn_animation_)
{
	turn_animation_.set_owner(this);

	for(int i = 0; i < size(); i ++){
		animations_[i].set_owner(this);
		static_animations_[i].set_owner(this);
		start_animations_[i].set_owner(this);
		stop_animations_[i].set_owner(this);
	}
}

qdAnimationSet::~qdAnimationSet()
{
	animations_.clear();
	static_animations_.clear();
	start_animations_.clear();
	stop_animations_.clear();
}

qdAnimationSet& qdAnimationSet::operator = (const qdAnimationSet& set)
{
	if(this == &set) return *this;

	*static_cast<qdNamedObject*>(this) = set;

	start_angle_ = set.start_angle_;

	animations_ = set.animations_;
	static_animations_ = set.static_animations_;
	start_animations_ = set.start_animations_;
	stop_animations_ = set.stop_animations_;
	walk_sound_frequency_ = set.walk_sound_frequency_;

	turn_animation_ = set.turn_animation_;
	turn_animation_.set_owner(this);

	for(int i = 0; i < size(); i ++){
		animations_[i].set_owner(this);
		static_animations_[i].set_owner(this);
		start_animations_[i].set_owner(this);
		stop_animations_[i].set_owner(this);
	}

	return *this;
}

void qdAnimationSet::resize(int sz)
{
	animations_.resize(sz);
	static_animations_.resize(sz);
	start_animations_.resize(sz);
	stop_animations_.resize(sz);
	walk_sound_frequency_.resize(sz,1);

	for(int i = 0; i < size(); i ++){
		animations_[i].set_owner(this);
		static_animations_[i].set_owner(this);
	}
}

qdAnimationInfo* qdAnimationSet::get_animation_info(int index)
{
	if(index >= 0 && index < size())
		return &animations_[index];

	return 0;
}

int qdAnimationSet::get_angle_index(float direction_angle,int dir_count)
{
	if(direction_angle < 0.0f)
		direction_angle += 2.0f * M_PI;
	
	int index = round(direction_angle * float(dir_count) / (2.0f * M_PI));
	if(index >= dir_count) index -= dir_count;
	if(index < 0) index += dir_count;
	
	return index;
}

float qdAnimationSet::get_index_angle(int index, int dir_count)
{
#ifdef _QUEST_EDITOR
	if (!dir_count)
		return 0.f;
#endif
	return index*2.f*M_PI/dir_count;
}

float qdAnimationSet::get_index_angle(int direction_index) const
{
	return get_index_angle(direction_index,size()) + start_angle_;
}

int qdAnimationSet::get_angle_index(float direction_angle) const
{
	return get_angle_index(direction_angle - start_angle_,size());
}

qdAnimationInfo* qdAnimationSet::get_animation_info(float direction_angle)
{
	int index = get_angle_index(direction_angle);
	return get_animation_info(index);
}

qdAnimationInfo* qdAnimationSet::get_static_animation_info(int index)
{
	if(index >= 0 && index < size())
		return &static_animations_[index];

	return 0;
}

qdAnimationInfo* qdAnimationSet::get_static_animation_info(float direction_angle)
{
	int index = get_angle_index(direction_angle);
	return get_static_animation_info(index);
}

qdAnimationInfo* qdAnimationSet::get_start_animation_info(int index)
{
	if(index >= 0 && index < size())
		return &start_animations_[index];

	return 0;
}

qdAnimationInfo* qdAnimationSet::get_start_animation_info(float direction_angle)
{
	int index = get_angle_index(direction_angle);
	return get_start_animation_info(index);
}

qdAnimationInfo* qdAnimationSet::get_stop_animation_info(int index)
{
	if(index >= 0 && index < size())
		return &stop_animations_[index];

	return 0;
}

qdAnimationInfo* qdAnimationSet::get_stop_animation_info(float direction_angle)
{
	int index = get_angle_index(direction_angle);
	return get_stop_animation_info(index);
}

qdAnimation* qdAnimationSet::get_turn_animation() const
{
	return turn_animation_.animation();
}

void qdAnimationSet::load_script(const xml::tag* p)
{
	int index = 0;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_SIZE:
			resize(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_ANIMATION_INFO:
			if(index < size())
				animations_[index].load_script(&*it);
			else if(index < size()*2)
				static_animations_[index - size()].load_script(&*it);
			else if(index < size()*3)
				start_animations_[index - size()*2].load_script(&*it);
			else
				stop_animations_[index - size()*3].load_script(&*it);

			index ++;
			break;
		case QDSCR_ANIMATION_SET_TURN:
			turn_animation_.set_animation_name(it->data());
			break;
		case QDSCR_ANIMATION_SET_START_ANGLE:
			xml::tag_buffer(*it) > start_angle_;
			break;
		case QDSCR_OBJECT_STATE_WALK_SOUND_FREQUENCY: {
				xml::tag_buffer buf(*it);
				walk_sound_frequency_.resize(it -> data_size());
				for(int i = 0; i < it -> data_size(); i++)
					buf > walk_sound_frequency_[i];
			}
			break;
		}
	}
}

bool qdAnimationSet::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i++) fh < "\t";
	fh < "<animation_set name=\"" < qdscr_XML_string(name()) < "\"";

	if(turn_animation_.animation_name())
		fh < " animation_turn=\"" < qdscr_XML_string(turn_animation_.animation_name()) < "\"";

	fh < " size=\"" <= size() < "\"";

	if(fabs(start_angle_) > FLT_EPS)
		fh < " start_angle=\"" <= start_angle_ < "\"";

	fh < ">\r\n";

	for(qdAnimationInfoVector::const_iterator it = animations_.begin(); it != animations_.end(); ++it)
		it -> save_script(fh,indent + 1);

	for(qdAnimationInfoVector::const_iterator it = static_animations_.begin(); it != static_animations_.end(); ++it)
		it -> save_script(fh,indent + 1);

	for(qdAnimationInfoVector::const_iterator it = start_animations_.begin(); it != start_animations_.end(); ++it)
		it -> save_script(fh,indent + 1);

	for(qdAnimationInfoVector::const_iterator it = stop_animations_.begin(); it != stop_animations_.end(); ++it)
		it -> save_script(fh,indent + 1);

	if(walk_sound_frequency_.size()){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<walk_sound_frequency>" <= walk_sound_frequency_.size();
		for(int i = 0; i < walk_sound_frequency_.size(); i++)
			fh < " " <= walk_sound_frequency_[i];
		fh < "</walk_sound_frequency>\r\n";
	}

	for(int i = 0; i < indent; i++) fh < "\t";
	fh < "</animation_set>\r\n";

	return true;
}

bool qdAnimationSet::load_animations(const qdNamedObject* res_owner)
{
	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
		qdAnimationInfoVector::iterator it;
		FOR_EACH(animations_,it){
			if(qdAnimation* p = it -> animation()){
				dp -> load_resource(p,res_owner);
			}
		}

		FOR_EACH(static_animations_,it){
			if(qdAnimation* p = it -> animation()){
				dp -> load_resource(p,res_owner);
			}
		}

		FOR_EACH(start_animations_,it){
			if(qdAnimation* p = it -> animation()){
				dp -> load_resource(p,res_owner);
			}
		}

		FOR_EACH(stop_animations_,it){
			if(qdAnimation* p = it -> animation()){
				dp -> load_resource(p,res_owner);
			}
		}

		if(qdAnimation* p = turn_animation_.animation())
			dp -> load_resource(p,res_owner);

		return true;
	}

	return false;
}

bool qdAnimationSet::free_animations(const qdNamedObject* res_owner)
{
	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
		qdAnimationInfoVector::iterator it;
		FOR_EACH(animations_,it){
			if(qdAnimation* p = it -> animation()){
				dp -> release_resource(p,res_owner);
			}
		}

		FOR_EACH(static_animations_,it){
			if(qdAnimation* p = it -> animation()){
				dp -> release_resource(p,res_owner);
			}
		}

		FOR_EACH(start_animations_,it){
			if(qdAnimation* p = it -> animation()){
				dp -> release_resource(p,res_owner);
			}
		}

		FOR_EACH(stop_animations_,it){
			if(qdAnimation* p = it -> animation()){
				dp -> release_resource(p,res_owner);
			}
		}

		if(qdAnimation* p = turn_animation_.animation())
			dp -> release_resource(p,res_owner);

		return true;
	}

	return false;
}

bool qdAnimationSet::scale_animations(float coeff_x,float coeff_y)
{
	bool res = true;

	qdAnimationInfoVector::iterator it;
	FOR_EACH(animations_,it){
		qdAnimation* p = it -> animation();
		if(p)
			if(!p -> scale(coeff_x,coeff_y)) res = false;
	}
	FOR_EACH(static_animations_,it){
		qdAnimation* p = it -> animation();
		if(p)
			if(!p -> scale(coeff_x,coeff_y)) res = false;
	}
	FOR_EACH(start_animations_,it){
		qdAnimation* p = it -> animation();
		if(p)
			if(!p -> scale(coeff_x,coeff_y)) res = false;
	}
	FOR_EACH(stop_animations_,it){
		qdAnimation* p = it -> animation();
		if(p)
			if(!p -> scale(coeff_x,coeff_y)) res = false;
	}

	if(qdAnimation* p = turn_animation_.animation())
		if(!p -> scale(coeff_x,coeff_y)) res = false;

	return res;
}

bool qdAnimationSet::register_resources(const qdNamedObject* res_owner)
{
	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
		qdAnimationInfoVector::iterator it;
		FOR_EACH(animations_,it){
			if(qdAnimation* p = it -> animation())
				dp -> register_resource(p,res_owner);
		}
		FOR_EACH(static_animations_,it){
			if(qdAnimation* p = it -> animation())
				dp -> register_resource(p,res_owner);
		}
		FOR_EACH(start_animations_,it){
			if(qdAnimation* p = it -> animation())
				dp -> register_resource(p,res_owner);
		}
		FOR_EACH(stop_animations_,it){
			if(qdAnimation* p = it -> animation())
				dp -> register_resource(p,res_owner);
		}
		if(qdAnimation* p = turn_animation_.animation())
			dp -> register_resource(p,res_owner);
		return true;
	}

	return false;
}

bool qdAnimationSet::unregister_resources(const qdNamedObject* res_owner)
{
	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
		qdAnimationInfoVector::iterator it;
		FOR_EACH(animations_,it){
			if(qdAnimation* p = it -> animation())
				dp -> unregister_resource(p,res_owner);
		}
		FOR_EACH(static_animations_,it){
			if(qdAnimation* p = it -> animation())
				dp -> unregister_resource(p,res_owner);
		}
		FOR_EACH(start_animations_,it){
			if(qdAnimation* p = it -> animation())
				dp -> unregister_resource(p,res_owner);
		}
		FOR_EACH(stop_animations_,it){
			if(qdAnimation* p = it -> animation())
				dp -> unregister_resource(p,res_owner);
		}
		if(qdAnimation* p = turn_animation_.animation())
			dp -> unregister_resource(p,res_owner);
		return true;
	}

	return false;
}

float qdAnimationSet::adjust_angle(float angle) const
{
	int dir = get_angle_index(angle);
	if(dir == -1) return 0.0f;

	return get_index_angle(dir);
}

const float qdAnimationSet::walk_sound_frequency(int direction_index) const
{
	if(direction_index < 0 || direction_index >= walk_sound_frequency_.size())
		return 1;
	else
		return walk_sound_frequency_[direction_index];
}

const float qdAnimationSet::walk_sound_frequency(float direction_angle) const
{
	int index = get_angle_index(direction_angle);
	return walk_sound_frequency(index);
}

void qdAnimationSet::set_walk_sound_frequency(int direction_index,float freq)
{
	assert(direction_index >= 0);

	if(direction_index >= walk_sound_frequency_.size())
		walk_sound_frequency_.resize(direction_index + 1,1);

	walk_sound_frequency_[direction_index] = freq;
}
