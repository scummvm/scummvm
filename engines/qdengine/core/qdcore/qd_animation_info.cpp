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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"

#ifndef __QD_SYSLIB__
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/parser/qdscr_parser.h"

#include "qdengine/core/qdcore/qd_game_scene.h"
#include "qdengine/core/qdcore/qd_game_dispatcher.h"
#endif

#include "qdengine/core/qdcore/qd_named_object.h"
#include "qdengine/core/qdcore/qd_animation_info.h"
#include "qdengine/core/qdcore/qd_animation.h"


namespace QDEngine {


/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdAnimationInfo::qdAnimationInfo() : speed_(0.0f), animation_speed_(1.0f) {
}

qdAnimationInfo::qdAnimationInfo(const qdAnimationInfo &p) : qdNamedObject(p),
	speed_(p.speed_),
	animation_speed_(p.animation_speed_) {
}

qdAnimationInfo::~qdAnimationInfo() {
}

void qdAnimationInfo::load_script(const xml::tag *p) {
#ifndef __QD_SYSLIB__
	int fl;
	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
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

bool qdAnimationInfo::save_script(Common::SeekableWriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<animation_info");

	if (flags()) {
		fh.writeString(Common::String::format(" flags=\"%d\"", flags()));
	}

	if (speed_ > 0.01f) {
		fh.writeString(Common::String::format(" speed=\"%f\"", speed_));
	}

	if (animation_speed_ != 1.0f) {
		fh.writeString(Common::String::format(" animation_speed=\"%f\"", animation_speed_));
	}

	if (animation_name()) {
		fh.writeString(Common::String::format(" animation=\"%s\"", qdscr_XML_string(animation_name())));
	}

	fh.writeString("/>\r\n");
	return true;

}

bool qdAnimationInfo::save_script(XStream &fh, int indent) const {
	warning("STUB: qdAnimationInfo::save_script(XStream)");
}

qdAnimationInfo &qdAnimationInfo::operator = (const qdAnimationInfo &p) {
	if (this == &p) return *this;

	*static_cast<qdNamedObject *>(this) = p;

	speed_ = p.speed_;
	animation_speed_ = p.animation_speed_;

	return *this;
}

qdAnimation *qdAnimationInfo::animation() const {
#ifndef __QD_SYSLIB__
	if (animation_name()) {
		if (qdGameScene * p = static_cast<qdGameScene * >(owner(QD_NAMED_OBJECT_SCENE))) {
			if (qdAnimation * anm = p -> get_animation(animation_name()))
				return anm;
		}

		if (qdGameDispatcher * p = qd_get_game_dispatcher())
			return p -> get_animation(animation_name());
	}
#endif
	return NULL;
}

} // namespace QDEngine
