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

#include "qdengine/core/parser/qdscr_parser.h"
#include "qdengine/core/parser/xml_tag.h"
#include "qdengine/core/qdcore/qd_interface_background.h"

namespace QDEngine {


/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdInterfaceBackground::qdInterfaceBackground() {
	state_.set_owner(this);
}

qdInterfaceBackground::qdInterfaceBackground(const qdInterfaceBackground &bk) : qdInterfaceElement(bk) {
	state_.set_owner(this);
	state_ = bk.state_;
}

qdInterfaceBackground::~qdInterfaceBackground() {
	state_.unregister_resources();
}

qdInterfaceBackground &qdInterfaceBackground::operator = (const qdInterfaceBackground &bk) {
	if (this == &bk) return *this;

	*static_cast<qdInterfaceElement *>(this) = bk;

	state_ = bk.state_;

	return *this;
}

bool qdInterfaceBackground::save_script_body(XStream &fh, int indent) const {
	if (!state_.save_script(fh, indent)) return false;

	return true;
}

bool qdInterfaceBackground::load_script_body(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_INTERFACE_ELEMENT_STATE:
			if (!state_.load_script(&*it)) return false;
			break;
		}
	}

	return true;
}

bool qdInterfaceBackground::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	return true;
}

bool qdInterfaceBackground::keyboard_handler(int vkey) {
	return false;
}

bool qdInterfaceBackground::init(bool is_game_active) {
	return set_state(&state_);
}
} // namespace QDEngine
