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
#include "qdengine/xmath.h"

#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"

#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"

#include "qdengine/qdcore/qd_named_object.h"
#include "qdengine/qdcore/qd_animation_info.h"
#include "qdengine/qdcore/qd_animation.h"


namespace QDEngine {

qdAnimationInfo::qdAnimationInfo() : _speed(0.0f), _animation_speed(1.0f) {
}

qdAnimationInfo::qdAnimationInfo(const qdAnimationInfo &p) : qdNamedObject(p),
	_speed(p._speed),
	_animation_speed(p._animation_speed) {
}

qdAnimationInfo::~qdAnimationInfo() {
}

void qdAnimationInfo::load_script(const xml::tag *p) {
	int fl;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_SPEED:
			xml::tag_buffer(*it) > _speed;
			break;
		case QDSCR_ANIMATION_SPEED:
			xml::tag_buffer(*it) > _animation_speed;
			break;
		case QDSCR_ANIMATION:
			set_animation_name(it->data());
			break;
		case QDSCR_FLAG:
			xml::tag_buffer(*it) > fl;
			set_flag(fl);
			break;
		}
	}
}

bool qdAnimationInfo::save_script(Common::WriteStream &fh, int indent) const {
	Common::String res;

	if (flags())
		res += Common::String::format(" flags=\"%d\"", flags());

	if (_speed > 0.01f)
		res += Common::String::format(" speed=\"%f\"", _speed);

	if (_animation_speed != 1.0f)
		res += Common::String::format(" animation_speed=\"%f\"", _animation_speed);

	if (animation_name())
		res += Common::String::format(" animation=\"%s\"", qdscr_XML_string(animation_name()));

	if (!res.empty()) {
		for (int i = 0; i < indent; i++)
			fh.writeString("\t");

		fh.writeString("<animation_info");
		fh.writeString(res);
		fh.writeString("/>\r\n");
	}

	return true;
}

qdAnimationInfo &qdAnimationInfo::operator = (const qdAnimationInfo &p) {
	if (this == &p)
		return *this;

	*static_cast<qdNamedObject *>(this) = p;

	_speed = p._speed;
	_animation_speed = p._animation_speed;

	return *this;
}

qdAnimation *qdAnimationInfo::animation() const {
	if (animation_name()) {
		if (qdGameScene *p = static_cast<qdGameScene *>(owner(QD_NAMED_OBJECT_SCENE))) {
			if (qdAnimation *anm = p->get_animation(animation_name()))
				return anm;
		}

		if (qdGameDispatcher *p = qd_get_game_dispatcher())
			return p->get_animation(animation_name());
	}
	return NULL;
}

} // namespace QDEngine
