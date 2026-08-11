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
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_animation_set.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"


namespace QDEngine {

qdAnimationSet::qdAnimationSet() {
	_start_angle = 0.0f;
}

qdAnimationSet::qdAnimationSet(const qdAnimationSet &set) : qdNamedObject(set),
	_start_angle(set._start_angle),
	_animations(set._animations),
	_walk_sound_frequency(set._walk_sound_frequency),
	_static_animations(set._static_animations),
	_start_animations(set._start_animations),
	_stop_animations(set._stop_animations),
	_turn_animation(set._turn_animation) {
	_turn_animation.set_owner(this);

	for (int i = 0; i < size(); i++) {
		_animations[i].set_owner(this);
		_static_animations[i].set_owner(this);
		_static_animations[i].set_owner(this);
		_stop_animations[i].set_owner(this);
	}
}

qdAnimationSet::~qdAnimationSet() {
	_animations.clear();
	_static_animations.clear();
	_start_animations.clear();
	_stop_animations.clear();
}

qdAnimationSet &qdAnimationSet::operator = (const qdAnimationSet &set) {
	if (this == &set) return *this;

	*static_cast<qdNamedObject *>(this) = set;

	_start_angle = set._start_angle;

	_animations = set._animations;
	_static_animations = set._static_animations;
	_start_animations = set._stop_animations;
	_stop_animations = set._stop_animations;
	_walk_sound_frequency = set._walk_sound_frequency;

	_turn_animation = set._turn_animation;
	_turn_animation.set_owner(this);

	for (int i = 0; i < size(); i++) {
		_animations[i].set_owner(this);
		_static_animations[i].set_owner(this);
		_start_animations[i].set_owner(this);
		_stop_animations[i].set_owner(this);
	}

	return *this;
}

void qdAnimationSet::resize(int sz) {
	_animations.resize(sz);
	_static_animations.resize(sz);
	_start_animations.resize(sz);
	_stop_animations.resize(sz);
	_walk_sound_frequency.resize(sz, 1);

	for (int i = 0; i < size(); i++) {
		_animations[i].set_owner(this);
		_static_animations[i].set_owner(this);
	}
}

qdAnimationInfo *qdAnimationSet::get_animation_info(int index) {
	if (index >= 0 && index < size())
		return &_animations[index];

	return 0;
}

int qdAnimationSet::get_angle_index(float direction_angle, int dir_count) {
	if (direction_angle < 0.0f)
		direction_angle += 2.0f * M_PI;

	int index = round(direction_angle * float(dir_count) / (2.0f * M_PI));
	if (index >= dir_count) index -= dir_count;
	if (index < 0) index += dir_count;

	return index;
}

float qdAnimationSet::get_index_angle(int index, int dir_count) {
	return index * 2.f * M_PI / dir_count;
}

float qdAnimationSet::get_index_angle(int direction_index) const {
	return get_index_angle(direction_index, size()) + _start_angle;
}

int qdAnimationSet::get_angle_index(float direction_angle) const {
	return get_angle_index(direction_angle - _start_angle, size());
}

qdAnimationInfo *qdAnimationSet::get_animation_info(float direction_angle) {
	int index = get_angle_index(direction_angle);
	return get_animation_info(index);
}

qdAnimationInfo *qdAnimationSet::get_static_animation_info(int index) {
	if (index >= 0 && index < size())
		return &_static_animations[index];

	return 0;
}

qdAnimationInfo *qdAnimationSet::get_static_animation_info(float direction_angle) {
	int index = get_angle_index(direction_angle);
	return get_static_animation_info(index);
}

qdAnimationInfo *qdAnimationSet::get_start_animation_info(int index) {
	if (index >= 0 && index < size())
		return &_start_animations[index];

	return 0;
}

qdAnimationInfo *qdAnimationSet::get_start_animation_info(float direction_angle) {
	int index = get_angle_index(direction_angle);
	return get_start_animation_info(index);
}

qdAnimationInfo *qdAnimationSet::get_stop_animation_info(int index) {
	if (index >= 0 && index < size())
		return &_stop_animations[index];

	return 0;
}

qdAnimationInfo *qdAnimationSet::get_stop_animation_info(float direction_angle) {
	int index = get_angle_index(direction_angle);
	return get_stop_animation_info(index);
}

qdAnimation *qdAnimationSet::get_turn_animation() const {
	return _turn_animation.animation();
}

void qdAnimationSet::load_script(const xml::tag *p) {
	int index = 0;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_SIZE:
			resize(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_ANIMATION_INFO:
			if (index < size())
				_animations[index].load_script(&*it);
			else if (index < size() * 2)
				_static_animations[index - size()].load_script(&*it);
			else if (index < size() * 3)
				_start_animations[index - size() * 2].load_script(&*it);
			else
				_stop_animations[index - size() * 3].load_script(&*it);

			index++;
			break;
		case QDSCR_ANIMATION_SET_TURN:
			_turn_animation.set_animation_name(it->data());
			break;
		case QDSCR_ANIMATION_SET_START_ANGLE:
			xml::tag_buffer(*it) > _start_angle;
			break;
		case QDSCR_OBJECT_STATE_WALK_SOUND_FREQUENCY: {
			xml::tag_buffer buf(*it);
			_walk_sound_frequency.resize(it->data_size());
			for (int i = 0; i < it->data_size(); i++)
				buf > _walk_sound_frequency[i];
		}
		break;
		}
	}
}

bool qdAnimationSet::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<animation_set name=\"%s\"", qdscr_XML_string(name())));

	if (_turn_animation.animation_name()) {
		fh.writeString(Common::String::format(" animation_turn=\"%s\"", qdscr_XML_string(_turn_animation.animation_name())));
	}

	fh.writeString(Common::String::format(" size=\"%d\"", size()));

	if (fabs(_start_angle) > FLT_EPS) {
		fh.writeString(Common::String::format(" start_angle=\"%f\"", _start_angle));
	}

	fh.writeString(">\r\n");

	for (auto &it : _animations) {
		it.save_script(fh, indent + 1);
	}

	for (auto &it : _static_animations) {
		it.save_script(fh, indent + 1);
	}

	for (auto &it : _start_animations) {
		it.save_script(fh, indent + 1);
	}

	for (auto &it : _stop_animations) {
		it.save_script(fh, indent + 1);
	}

	if (_walk_sound_frequency.size()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}

		fh.writeString(Common::String::format("<walk_sound_frequency>%u", _walk_sound_frequency.size()));
		for (uint i = 0; i < _walk_sound_frequency.size(); i++) {
			fh.writeString(Common::String::format(" %f", _walk_sound_frequency[i]));
		}

		fh.writeString("</walk_sound_frequency>\r\n");
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</animation_set>\r\n");

	return true;
}

bool qdAnimationSet::load_animations(const qdNamedObject *res_owner) {
	if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
		for (auto &it : _animations) {
			if (qdAnimation *p = it.animation()) {
				dp->load_resource(p, res_owner);
			}
		}

		for (auto &it : _static_animations) {
			if (qdAnimation *p = it.animation()) {
				dp->load_resource(p, res_owner);
			}
		}


		for (auto &it : _start_animations) {
			if (qdAnimation *p = it.animation()) {
				dp->load_resource(p, res_owner);
			}
		}

		for (auto &it : _stop_animations) {
			if (qdAnimation *p = it.animation()) {
				dp->load_resource(p, res_owner);
			}
		}

		if (qdAnimation *p = _turn_animation.animation())
			dp->load_resource(p, res_owner);

		return true;
	}

	return false;
}

bool qdAnimationSet::free_animations(const qdNamedObject *res_owner) {
	if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
		for (auto &it : _animations) {
			if (qdAnimation *p = it.animation()) {
				dp->release_resource(p, res_owner);
			}
		}

		for (auto &it : _static_animations) {
			if (qdAnimation *p = it.animation()) {
				dp->release_resource(p, res_owner);
			}
		}

		for (auto &it : _start_animations) {
			if (qdAnimation *p = it.animation()) {
				dp->release_resource(p, res_owner);
			}
		}

		for (auto &it : _stop_animations) {
			if (qdAnimation *p = it.animation()) {
				dp->release_resource(p, res_owner);
			}
		}

		if (qdAnimation *p = _turn_animation.animation())
			dp->release_resource(p, res_owner);

		return true;
	}

	return false;
}

bool qdAnimationSet::scale_animations(float coeff_x, float coeff_y) {
	bool res = true;

	for (auto &it : _animations) {
		qdAnimation *p = it.animation();
		if (p)
			if (!p->scale(coeff_x, coeff_y)) res = false;
	}
	for (auto &it : _static_animations) {
		qdAnimation *p = it.animation();
		if (p)
			if (!p->scale(coeff_x, coeff_y)) res = false;
	}
	for (auto &it : _start_animations) {
		qdAnimation *p = it.animation();
		if (p)
			if (!p->scale(coeff_x, coeff_y)) res = false;
	}
	for (auto &it : _stop_animations) {
		qdAnimation *p = it.animation();
		if (p)
			if (!p->scale(coeff_x, coeff_y)) res = false;
	}

	if (qdAnimation *p = _turn_animation.animation())
		if (!p->scale(coeff_x, coeff_y)) res = false;

	return res;
}

bool qdAnimationSet::register_resources(const qdNamedObject *res_owner) {
	if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
		for (auto &it : _animations) {
			if (qdAnimation *p = it.animation())
				dp->register_resource(p, res_owner);
		}
		for (auto &it : _static_animations) {
			if (qdAnimation *p = it.animation())
				dp->register_resource(p, res_owner);
		}
		for (auto &it : _start_animations) {
			if (qdAnimation *p = it.animation())
				dp->register_resource(p, res_owner);
		}
		for (auto &it : _stop_animations) {
			if (qdAnimation *p = it.animation())
				dp->register_resource(p, res_owner);
		}
		if (qdAnimation *p = _turn_animation.animation())
			dp->register_resource(p, res_owner);
		return true;
	}

	return false;
}

bool qdAnimationSet::unregister_resources(const qdNamedObject *res_owner) {
	if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
		for (auto &it : _animations) {
			if (qdAnimation *p = it.animation())
				dp->unregister_resource(p, res_owner);
		}
		for (auto &it : _static_animations) {
			if (qdAnimation *p = it.animation())
				dp->unregister_resource(p, res_owner);
		}
		for (auto &it : _start_animations) {
			if (qdAnimation *p = it.animation())
				dp->unregister_resource(p, res_owner);
		}
		for (auto &it : _stop_animations) {
			if (qdAnimation *p = it.animation())
				dp->unregister_resource(p, res_owner);
		}
		if (qdAnimation *p = _turn_animation.animation())
			dp->unregister_resource(p, res_owner);
		return true;
	}

	return false;
}

float qdAnimationSet::adjust_angle(float angle) const {
	int dir = get_angle_index(angle);
	if (dir == -1) return 0.0f;

	return get_index_angle(dir);
}

float qdAnimationSet::walk_sound_frequency(int direction_index) const {
	if (direction_index < 0 || direction_index >= (int)_walk_sound_frequency.size())
		return 1;
	else
		return _walk_sound_frequency[direction_index];
}

float qdAnimationSet::walk_sound_frequency(float direction_angle) const {
	int index = get_angle_index(direction_angle);
	return walk_sound_frequency(index);
}

void qdAnimationSet::set_walk_sound_frequency(int direction_index, float freq) {
	assert(direction_index >= 0);

	if (direction_index >= (int)_walk_sound_frequency.size())
		_walk_sound_frequency.resize(direction_index + 1, 1);

	_walk_sound_frequency[direction_index] = freq;
}
} // namespace QDEngine
