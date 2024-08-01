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

#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag.h"
#include "qdengine/qdcore/qd_interface_background.h"

namespace QDEngine {

qdInterfaceBackground::qdInterfaceBackground() {
	_state.set_owner(this);
}

qdInterfaceBackground::qdInterfaceBackground(const qdInterfaceBackground &bk) : qdInterfaceElement(bk) {
	_state.set_owner(this);
	_state = bk._state;
}

qdInterfaceBackground::~qdInterfaceBackground() {
	_state.unregister_resources();
}

qdInterfaceBackground &qdInterfaceBackground::operator = (const qdInterfaceBackground &bk) {
	if (this == &bk) return *this;

	*static_cast<qdInterfaceElement *>(this) = bk;

	_state = bk._state;

	return *this;
}

bool qdInterfaceBackground::save_script_body(Common::WriteStream &fh, int indent) const {
	if (!_state.save_script(fh, indent)) {
		return false;
	}

	return true;
}

bool qdInterfaceBackground::load_script_body(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_INTERFACE_ELEMENT_STATE:
			if (!_state.load_script(&*it)) return false;
			break;
		}
	}

	return true;
}

bool qdInterfaceBackground::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	return true;
}

bool qdInterfaceBackground::keyboard_handler(Common::KeyCode vkey) {
	return false;
}

bool qdInterfaceBackground::init(bool is_game_active) {
	return set_state(&_state);
}

} // namespace QDEngine
