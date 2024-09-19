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

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_animation.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"

#include "qd_interface_element_state_mode.h"

namespace QDEngine {

qdInterfaceElementStateMode::qdInterfaceElementStateMode() : _sound(NULL),
	_animation_flags(0),
	_animation(NULL),
	_contour(qdContour::CONTOUR_POLYGON) {
}

qdInterfaceElementStateMode::qdInterfaceElementStateMode(const qdInterfaceElementStateMode &ev) : _sound_file(ev._sound_file),
	_sound(ev._sound),
	_animation_file(ev._animation_file),
	_animation_flags(ev._animation_flags),
	_animation(ev._animation),
	_contour(ev._contour) {
}

qdInterfaceElementStateMode::~qdInterfaceElementStateMode() {
}

qdInterfaceElementStateMode &qdInterfaceElementStateMode::operator = (const qdInterfaceElementStateMode &ev) {
	if (this == &ev) return *this;

	_sound_file = ev._sound_file;
	_sound = ev._sound;

	_animation_file = ev._animation_file;
	_animation_flags = ev._animation_flags;
	_animation = ev._animation;

	_contour = ev._contour;

	return *this;
}

void qdInterfaceElementStateMode::set_sound_file(const Common::Path name) {
	_sound_file = name;
}

void qdInterfaceElementStateMode::set_animation_file(const Common::Path name) {
	_animation_file = name;
}

bool qdInterfaceElementStateMode::save_script(Common::WriteStream &fh, int type_id, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<state_mode type=\"%d\"", type_id));

	if (has_animation()) {
		fh.writeString(Common::String::format(" animation=\"%s\"", qdscr_XML_string(animation_file().toString('\\'))));
	}

	if (_animation_flags) {
		if (debugChannelSet(-1, kDebugLog))
			fh.writeString(Common::String::format(" animation_flags=\"%s\"", qdAnimation::flag2str(_animation_flags).c_str()));
		else
			fh.writeString(Common::String::format(" animation_flags=\"%d\"", _animation_flags));
	}

	if (has_sound()) {
		fh.writeString(Common::String::format(" sound=\"%s\"", qdscr_XML_string(sound_file().toString('\\'))));
	}

	if (has_contour()) {
		fh.writeString(">\r\n");
		_contour.save_script(fh, indent + 1);
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
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_SOUND:
			set_sound_file(Common::Path(it->data(), '\\'));
			break;
		case QDSCR_INTERFACE_ANIMATION_FLAGS:
			xml::tag_buffer(*it) > _animation_flags;
			break;
		case QDSCR_ANIMATION:
			set_animation_file(Common::Path(it->data(), '\\'));
			break;
		case QDSCR_CONTOUR_CIRCLE:
			_contour.set_contour_type(qdContour::CONTOUR_CIRCLE);
			_contour.load_script(&*it);
			break;
		case QDSCR_CONTOUR_RECTANGLE:
			_contour.set_contour_type(qdContour::CONTOUR_RECTANGLE);
			_contour.load_script(&*it);
			break;
		case QDSCR_CONTOUR_POLYGON:
			_contour.set_contour_type(qdContour::CONTOUR_POLYGON);
			_contour.load_script(&*it);
			break;
		}
	}

	return true;
}
} // namespace QDEngine
