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
#include "qdengine/qdcore/qd_game_end.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"

namespace QDEngine {

qdGameEnd::qdGameEnd() {
}

qdGameEnd::qdGameEnd(const qdGameEnd &end) : qdConditionalObject(end),
	_interface_screen(end._interface_screen) {
}

qdGameEnd::~qdGameEnd() {
}

qdGameEnd &qdGameEnd::operator = (const qdGameEnd &end) {
	if (this == &end) return *this;

	*static_cast<qdConditionalObject *>(this) = end;

	_interface_screen = end._interface_screen;

	return *this;
}

qdConditionalObject::trigger_start_mode qdGameEnd::trigger_start() {
	if (qdGameDispatcher *p = qdGameDispatcher::get_dispatcher()) {
		p->set_game_end(this);
		return qdConditionalObject::TRIGGER_START_ACTIVATE;
	}

	return qdConditionalObject::TRIGGER_START_FAILED;
}

bool qdGameEnd::load_script(const xml::tag *p) {
	load_conditions_script(p);

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_GAME_END_SCREEN:
			set_interface_screen(it->data());
			break;
		}
	}

	return true;
}

bool qdGameEnd::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<game_end");

	if (name()) {
		fh.writeString(Common::String::format(" name=\"%s\"", qdscr_XML_string(name())));
	}

	if (flags()) {
		fh.writeString(Common::String::format(" flags=\"%d\"", flags()));
	}

	if (!_interface_screen.empty()) {
		fh.writeString(Common::String::format(" end_screen=\"%s\"", qdscr_XML_string(_interface_screen.c_str())));
	}

	if (has_conditions()) {
		fh.writeString(">\r\n");

		save_conditions_script(fh, indent);
		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}

		fh.writeString("</game_end>\r\n");
	} else {
		fh.writeString("/>\r\n");
	}

	return true;
}

} // namespace QDEngine
