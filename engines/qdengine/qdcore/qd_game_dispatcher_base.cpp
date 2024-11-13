/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "qdengine/qd_fwd.h"

#include "qdengine/parser/xml_tag.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_sound.h"
#include "qdengine/qdcore/qd_animation.h"
#include "qdengine/qdcore/qd_animation_set.h"
#include "qdengine/qdcore/qd_game_dispatcher_base.h"

namespace QDEngine {

qdGameDispatcherBase::qdGameDispatcherBase() {
}

qdGameDispatcherBase::~qdGameDispatcherBase() {
}

void qdGameDispatcherBase::load_script_body(const xml::tag *p) {
	qdAnimation *ap;
	qdAnimationSet *asp;
	qdSound *snd;

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_OBJ_SCALE_INFO: {
			qdScaleInfo sp;
			sp.load_script(&*it);
			add_scale_info(&sp);
		}
		break;
		case QDSCR_ANIMATION:
			ap = new qdAnimation;
			ap->load_script(&*it);
			add_animation(ap);
			break;
		case QDSCR_ANIMATION_SET:
			asp = new qdAnimationSet;
			asp->load_script(&*it);
			add_animation_set(asp);
			break;
		case QDSCR_SOUND:
			snd = new qdSound;
			snd->load_script(&*it);
			add_sound(snd);
			break;
		}
	}
}

bool qdGameDispatcherBase::save_script_body(Common::WriteStream &fh, int indent) const {
	for (auto &it : _scale_infos) {
		it.save_script(fh, indent + 1);
	}

	for (auto &it : sound_list()) {
		it->save_script(fh, indent + 1);
	}

	for (auto &it : animation_list()) {
		it->save_script(fh, indent + 1);
	}

	for (auto &it : animation_set_list()) {
		it->save_script(fh, indent + 1);
	}

	return true;
}

bool qdGameDispatcherBase::get_object_scale(const char *p, float &sc) {
	qdScaleInfo *si = get_scale_info(p);
	if (si) {
		sc = si->scale();
		return true;
	}

	return false;
}

qdScaleInfo *qdGameDispatcherBase::get_scale_info(const char *p) {
	for (scale_info_container_t::iterator it = _scale_infos.begin(); it != _scale_infos.end(); ++it) {
		if (!strcmp(it->name(), p)) {
			return &*it;
		}
	}

	return NULL;
}

bool qdGameDispatcherBase::set_object_scale(const char *p, float sc) {
	for (scale_info_container_t::iterator it = _scale_infos.begin(); it != _scale_infos.end(); ++it) {
		if (!strcmp(it->name(), p)) {
			it->set_scale(sc);
			return true;
		}
	}

	qdScaleInfo scl;
	scl.set_name(p);
	scl.set_scale(sc);

	add_scale_info(&scl);

	return true;
}

int qdGameDispatcherBase::load_resources() {
	return 0;
}

void qdGameDispatcherBase::free_resources() {
	for (auto &ia : animation_list()) {
		ia->free_resources();
	}

	for (auto &is : sound_list()) {
		is->free_resource();
	}
}

int qdGameDispatcherBase::get_resources_size() {
	return 0;
}

void qdGameDispatcherBase::show_loading_progress(int sz) {
	_loading_progress.show_progress(sz);
}

bool qdGameDispatcherBase::add_sound(qdSound *p) {
	if (_sounds.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcherBase::remove_sound(const char *name) {
	return _sounds.remove_object(name);
}

bool qdGameDispatcherBase::remove_sound(qdSound *p) {
	return _sounds.remove_object(p);
}

qdSound *qdGameDispatcherBase::get_sound(const char *name) {
	return _sounds.get_object(name);
}

bool qdGameDispatcherBase::is_sound_in_list(const char *name) {
	return _sounds.is_in_list(name);
}

bool qdGameDispatcherBase::is_sound_in_list(qdSound *p) {
	return _sounds.is_in_list(p);
}

bool qdGameDispatcherBase::add_animation(qdAnimation *p) {
	if (_animations.add_object(p)) {
		p->set_owner(this);
		return true;
	}
	return false;
}

bool qdGameDispatcherBase::remove_animation(const char *name) {
	return _animations.remove_object(name);
}

bool qdGameDispatcherBase::remove_animation(qdAnimation *p) {
	return _animations.remove_object(p);
}

qdAnimation *qdGameDispatcherBase::get_animation(const char *name) {
	return _animations.get_object(name);
}

bool qdGameDispatcherBase::is_animation_in_list(const char *name) {
	return _animations.is_in_list(name);
}

bool qdGameDispatcherBase::is_animation_in_list(qdAnimation *p) {
	return _animations.is_in_list(p);
}

bool qdGameDispatcherBase::add_animation_set(qdAnimationSet *p) {
	if (_animation_sets.add_object(p)) {
		p->set_owner(this);
		return true;
	}
	return false;
}

bool qdGameDispatcherBase::remove_animation_set(const char *name) {
	return _animation_sets.remove_object(name);
}

bool qdGameDispatcherBase::remove_animation_set(qdAnimationSet *p) {
	return _animation_sets.remove_object(p);
}

qdAnimationSet *qdGameDispatcherBase::get_animation_set(const char *name) {
	return _animation_sets.get_object(name);
}

bool qdGameDispatcherBase::is_animation_set_in_list(const char *name) {
	return _animation_sets.is_in_list(name);
}

bool qdGameDispatcherBase::is_animation_set_in_list(qdAnimationSet *p) {
	return _animation_sets.is_in_list(p);
}

#ifdef __QD_DEBUG_ENABLE__
bool qdGameDispatcherBase::get_resources_info(qdResourceInfoContainer &infos) const {
	for (qdSoundList::const_iterator it = sound_list().begin(); it != sound_list().end(); ++it) {
		if ((*it)->is_resource_loaded())
			infos.push_back(qdResourceInfo(*it, *it));
	}

	for (qdAnimationList::const_iterator it = animation_list().begin(); it != animation_list().end(); ++it) {
		if ((*it)->is_resource_loaded())
			infos.push_back(qdResourceInfo(*it, *it));
	}

	return true;
}
#endif

} // namespace QDEngine
