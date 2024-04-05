/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <stdlib.h>

#include "xml_tag.h"
#include "qdscr_parser.h"

#include "qd_sound.h"
#include "qd_animation.h"
#include "qd_animation_set.h"
#include "qd_game_dispatcher_base.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdGameDispatcherBase::qdGameDispatcherBase()
{
}

qdGameDispatcherBase::~qdGameDispatcherBase()
{
}

void qdGameDispatcherBase::load_script_body(const xml::tag* p)
{
	qdAnimation* ap;
	qdAnimationSet* asp;
	qdSound* snd;

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_OBJ_SCALE_INFO: {
					qdScaleInfo sp;
					sp.load_script(&*it);
					add_scale_info(&sp);
				}
				break;
			case QDSCR_ANIMATION:
				ap = new qdAnimation;
				ap -> load_script(&*it);
				add_animation(ap);
				break;
			case QDSCR_ANIMATION_SET:
				asp = new qdAnimationSet;
				asp -> load_script(&*it);
				add_animation_set(asp);
				break;
			case QDSCR_SOUND:
				snd = new qdSound;
				snd -> load_script(&*it);
				add_sound(snd);
				break;
		}
	}
}

bool qdGameDispatcherBase::save_script_body(XStream& fh,int indent) const
{
	for(scale_info_container_t::const_iterator it = scale_infos.begin(); it != scale_infos.end(); ++it)
		it -> save_script(fh,indent + 1);

	for(qdSoundList::const_iterator it = sound_list().begin(); it != sound_list().end(); ++it)
		(*it) -> save_script(fh,indent + 1);

	for(qdAnimationList::const_iterator it = animation_list().begin(); it != animation_list().end(); ++it)
		(*it) -> save_script(fh,indent + 1);

	for(qdAnimationSetList::const_iterator it = animation_set_list().begin(); it != animation_set_list().end(); ++it)
		(*it) -> save_script(fh,indent + 1);

	return true;
}

bool qdGameDispatcherBase::get_object_scale(const char* p,float& sc)
{
	qdScaleInfo* si = get_scale_info(p);
	if(si){
		sc = si -> scale();
		return true;
	}

	return false;
}

bool qdGameDispatcherBase::remove_object_scale(const char* p)
{
	for(scale_info_container_t::iterator it = scale_infos.begin(); it != scale_infos.end(); ++it){
		if(!strcmp(it -> name(),p)){
			scale_infos.erase(it);
			return true;
		}
	}

	return false;
}

qdScaleInfo* qdGameDispatcherBase::get_scale_info(const char* p)
{
	for(scale_info_container_t::iterator it = scale_infos.begin(); it != scale_infos.end(); ++it){
		if(!strcmp(it -> name(),p)){
			return &*it;
		}
	}

	return NULL;
}

bool qdGameDispatcherBase::set_object_scale(const char* p,float sc)
{
	for(scale_info_container_t::iterator it = scale_infos.begin(); it != scale_infos.end(); ++it){
		if(!strcmp(it -> name(),p)){
			it -> set_scale(sc);
			return true;
		}
	}

	qdScaleInfo scl;
	scl.set_name(p);
	scl.set_scale(sc);

	add_scale_info(&scl);

	return true;
}

int qdGameDispatcherBase::load_resources()
{
	return 0;
}

void qdGameDispatcherBase::free_resources()
{
	qdAnimationList::const_iterator ia;
	FOR_EACH(animation_list(),ia){
		(*ia) -> free_resources();
	}

	qdSoundList::const_iterator is;
	FOR_EACH(sound_list(),is){
		(*is) -> free_resource();
	}
}

int qdGameDispatcherBase::get_resources_size()
{
	return 0;
}

void qdGameDispatcherBase::show_loading_progress(int sz)
{
	loading_progress_.show_progress(sz);
}

bool qdGameDispatcherBase::add_sound(qdSound* p)
{ 
	if(sounds.add_object(p)){
		p -> set_owner(this);
		return true;
	} 

	return false; 
}

bool qdGameDispatcherBase::remove_sound(const char* name)
{
	return sounds.remove_object(name);
}

bool qdGameDispatcherBase::remove_sound(qdSound* p)
{
	return sounds.remove_object(p);
}

qdSound* qdGameDispatcherBase::get_sound(const char* name)
{
	return sounds.get_object(name);
}

bool qdGameDispatcherBase::is_sound_in_list(const char* name)
{
	return sounds.is_in_list(name);
}

bool qdGameDispatcherBase::is_sound_in_list(qdSound* p)
{
	return sounds.is_in_list(p);
}

bool qdGameDispatcherBase::add_animation(qdAnimation* p)
{
	if(animations.add_object(p)){ p -> set_owner(this); return true; } 
	return false; 
}

bool qdGameDispatcherBase::remove_animation(const char* name)
{ 
	return animations.remove_object(name); 
}

bool qdGameDispatcherBase::remove_animation(qdAnimation* p)
{ 
	return animations.remove_object(p); 
}

qdAnimation* qdGameDispatcherBase::get_animation(const char* name)
{ 
	return animations.get_object(name); 
}

bool qdGameDispatcherBase::is_animation_in_list(const char* name)
{ 
	return animations.is_in_list(name); 
}

bool qdGameDispatcherBase::is_animation_in_list(qdAnimation* p)
{ 
	return animations.is_in_list(p); 
}

bool qdGameDispatcherBase::add_animation_set(qdAnimationSet* p)
{ 
	if(animation_sets.add_object(p)){ p -> set_owner(this); return true; } 
	return false;
}

bool qdGameDispatcherBase::remove_animation_set(const char* name)
{ 
	return animation_sets.remove_object(name); 
}

bool qdGameDispatcherBase::remove_animation_set(qdAnimationSet* p)
{ 
	return animation_sets.remove_object(p); 
}

qdAnimationSet* qdGameDispatcherBase::get_animation_set(const char* name)
{ 
	return animation_sets.get_object(name); 
}

bool qdGameDispatcherBase::is_animation_set_in_list(const char* name)
{ 
	return animation_sets.is_in_list(name); 
}

bool qdGameDispatcherBase::is_animation_set_in_list(qdAnimationSet* p)
{ 
	return animation_sets.is_in_list(p); 
}

#ifdef __QD_DEBUG_ENABLE__
bool qdGameDispatcherBase::get_resources_info(qdResourceInfoContainer& infos) const
{
	for(qdSoundList::const_iterator it = sound_list().begin(); it != sound_list().end(); ++it){
		if((*it) -> is_resource_loaded())
			infos.push_back(qdResourceInfo(*it,*it));
	}

	for(qdAnimationList::const_iterator it = animation_list().begin(); it != animation_list().end(); ++it){
		if((*it) -> is_resource_loaded())
			infos.push_back(qdResourceInfo(*it,*it));
	}

	return true;
}
#endif
