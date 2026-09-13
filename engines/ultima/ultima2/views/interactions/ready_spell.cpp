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

#include "ultima/ultima2/views/interactions/ready_spell.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

ReadySpell::ReadySpell() : Interaction("ReadySpell") {
}

bool ReadySpell::msgFocus(const FocusMessage &msg) {
	writeString("MAGIC SPELL READY #");
	return Interaction::msgFocus(msg);
}

bool ReadySpell::msgKeypress(const KeypressMessage &msg) {
	if (msg.ascii < '0' || msg.ascii > '9')
		return true;

	int digit = msg.ascii - '0';
	_G(savegame)._readiedSpell = (Data::SpellType)digit;
	writeString("%d\nSPELL READY => %s\n", digit, Data::SPELL_NAMES[digit]);

	close();
	_G(logic)->resumeTurn();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
