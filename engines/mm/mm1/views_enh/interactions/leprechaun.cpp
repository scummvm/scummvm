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

#include "mm/mm1/views_enh/interactions/leprechaun.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Leprechaun::Leprechaun() : Interaction("Leprechaun", 15) {
	_title = STRING["maps.emap00.leprechaun_title"];
	addText(STRING["maps.emap00.leprechaun"]);
	addButton(STRING["stats.towns.1"], '1');
	addButton(STRING["stats.towns.2"], '2');
	addButton(STRING["stats.towns.3"], '3');
	addButton(STRING["stats.towns.4"], '4');
	addButton(STRING["stats.towns.5"], '5');
}

bool Leprechaun::msgFocus(const FocusMessage &msg) {
	Interaction::msgFocus(msg);
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	return true;
}

bool Leprechaun::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode >= Common::KEYCODE_1 && msg.keycode <= Common::KEYCODE_5) {
		teleportToTown(msg.ascii);
		return true;
	}

	return false;
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
