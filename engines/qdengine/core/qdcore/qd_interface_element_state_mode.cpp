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
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/parser/qdscr_parser.h"

#include "qd_interface_element_state_mode.h"


namespace QDEngine {


/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdInterfaceElementStateMode::qdInterfaceElementStateMode() : sound_(NULL),
	animation_flags_(0),
	animation_(NULL),
	contour_(qdContour::CONTOUR_POLYGON) {
}

qdInterfaceElementStateMode::qdInterfaceElementStateMode(const qdInterfaceElementStateMode &ev) : sound_file_(ev.sound_file_),
	sound_(ev.sound_),
	animation_file_(ev.animation_file_),
	animation_flags_(ev.animation_flags_),
	animation_(ev.animation_),
	contour_(ev.contour_) {
}

qdInterfaceElementStateMode::~qdInterfaceElementStateMode() {
}

qdInterfaceElementStateMode &qdInterfaceElementStateMode::operator = (const qdInterfaceElementStateMode &ev) {
	if (this == &ev) return *this;

	sound_file_ = ev.sound_file_;
	sound_ = ev.sound_;

	animation_file_ = ev.animation_file_;
	animation_flags_ = ev.animation_flags_;
	animation_ = ev.animation_;

	contour_ = ev.contour_;

	return *this;
}

void qdInterfaceElementStateMode::set_sound_file(const char *name) {
	if (name)
		sound_file_ = name;
	else
		sound_file_.clear();
}

void qdInterfaceElementStateMode::set_animation_file(const char *name) {
	if (name)
		animation_file_ = name;
	else
		animation_file_.clear();
}

bool qdInterfaceElementStateMode::save_script(Common::SeekableWriteStream &fh, int type_id, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<state_mode type=\"%d\"", type_id));

	if (has_animation()) {
		fh.writeString(Common::String::format(" animation=\"%s\"", qdscr_XML_string(animation_file())));
	}

	if (animation_flags_) {
		fh.writeString(Common::String::format(" animation_flags=\"%d\"", animation_flags_));
	}

	if (has_sound()) {
		fh.writeString(Common::String::format(" sound=\"%s\"", qdscr_XML_string(sound_file())));
	}

	if (has_contour()) {
		fh.writeString(">\r\n");
		contour_.save_script(fh, indent + 1);
		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("</state_mode>\r\n");
	} else {
		fh.writeString("/>\r\n");
	}

	return true;
}

bool qdInterfaceElementStateMode::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_SOUND:
			set_sound_file(it -> data());
			break;
		case QDSCR_INTERFACE_ANIMATION_FLAGS:
			xml::tag_buffer(*it) > animation_flags_;
			break;
		case QDSCR_ANIMATION:
			set_animation_file(it -> data());
			break;
		case QDSCR_CONTOUR_CIRCLE:
			contour_.set_contour_type(qdContour::CONTOUR_CIRCLE);
			contour_.load_script(&*it);
			break;
		case QDSCR_CONTOUR_RECTANGLE:
			contour_.set_contour_type(qdContour::CONTOUR_RECTANGLE);
			contour_.load_script(&*it);
			break;
		case QDSCR_CONTOUR_POLYGON:
			contour_.set_contour_type(qdContour::CONTOUR_POLYGON);
			contour_.load_script(&*it);
			break;
		}
	}

	return true;
}
} // namespace QDEngine
