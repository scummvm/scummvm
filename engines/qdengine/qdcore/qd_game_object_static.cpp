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
#include "qdengine/parser/xml_tag.h"
#include "qdengine/parser/qdscr_parser.h"

#include "qdengine/qdcore/qd_game_object_static.h"
#include "qdengine/qdcore/qd_game_scene.h"

namespace QDEngine {

qdGameObjectStatic::qdGameObjectStatic() {
}

qdGameObjectStatic::~qdGameObjectStatic() {
}

void qdGameObjectStatic::redraw(int offs_x, int offs_y) const {
	debugC(2, kDebugGraphics, "qdGameObjectStatic::redraw([%d, %d]), name: '%s'", offs_x, offs_y, transCyrillic(name()));

	Vect2i scrCoord = screen_pos() + Vect2i(offs_x, offs_y);
	_sprite.redraw(scrCoord.x, scrCoord.y, screen_depth(), 0);
}

bool qdGameObjectStatic::load_script(const xml::tag *p) {
	return load_script_body(p);
}

bool qdGameObjectStatic::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString(Common::String::format("<static_object name=\"%s\">\r\n", qdscr_XML_string(name())));

	save_script_body(fh, indent);

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</static_object>\r\n");
	return true;
}

bool qdGameObjectStatic::load_script_body(const xml::tag *p) {
	qdGameObject::load_script_body(p);

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_FILE:
			_sprite.set_file(Common::Path(it->data(), '\\'));
			break;
		}
	}

	return true;
}

bool qdGameObjectStatic::save_script_body(Common::WriteStream &fh, int indent) const {
	qdGameObject::save_script_body(fh, indent);

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}

	if (!_sprite.file().empty()) {
		fh.writeString(Common::String::format("<file>%s</file>\r\n", qdscr_XML_string(_sprite.file().toString('\\'))));
	}

	return true;
}

bool qdGameObjectStatic::load_resources() {
	return _sprite.load();
}

void qdGameObjectStatic::free_resources() {
	_sprite.free();
}

bool qdGameObjectStatic::hit(int x, int y) const {
	return false;
}

void qdGameObjectStatic::draw_contour(uint32 color) const {
	Vect2s pos = screen_pos();
	_sprite.draw_contour(pos.x, pos.y, color);
}

} // namespace QDEngine
