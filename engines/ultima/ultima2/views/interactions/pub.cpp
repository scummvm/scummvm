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

#include "ultima/ultima2/views/interactions/pub.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

static const char *const HINTS[10] = {
	"CAUGH...",
	"ISN'T THIS A GREAT GAME?",
	"HHMMM...",
	"SOME FIGHTERS WEAR MAGIC HELMS!",
	"AVIATORS USE SKULL KEYS!",
	"SAYLORS WEAR BLUE TASSLES!",
	"MAGES CARRY WANDS OR STAFFS!",
	"GUARDS CARRY KEYS!",
	"ANKHS OPEN SPACE!",
	"PLANES NEED BRASS BUTTONS!"
};

constexpr int DRINK_PRICE = 5;

Pub::Pub() : Interaction("Pub") {
}

bool Pub::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	_state = MENU;
	writeString("WELCOME TO THE PUB!\n1-BUY, 2-TIP --- ");

	return Interaction::msgFocus(msg);
}

void Pub::barkeepSays(int hint) {
	writeString("THE BARKEEP SAYS:\n%s\n", HINTS[hint]);
}

bool Pub::msgKeypress(const KeypressMessage &msg) {
	// Any other key walks out of the pub
	if (msg.ascii < '0' || msg.ascii > '9') {
		close();
		writeString("\n");
		_G(logic)->resumeTurn();
		return true;
	}

	int digit = msg.ascii - '0';
	writeString("%d\n", digit);

	if (_state == MENU) {
		if (digit == 2) {
			writeString("TIP HOW MUCH? ");
			_state = TIP;
			return true;
		}

		close();
		if (digit == 1 && _G(logic)->trySpendGold(DRINK_PRICE)) {
			writeString("HERE, HAVE A STRONG ONE!\n");
			g_engine->randByte();
			barkeepSays(1);
		}
	} else {
		close();
		if (_G(logic)->trySpendGold(digit)) {
			// A bigger tip moves the hint further along the list, with a
			// random -1/0/+1 wobble; running off either end gives the first
			int8 roll = (int8)g_engine->randByte();
			int wobble = (roll > 0) - (roll < 0);
			byte hint = (byte)(digit + wobble);
			barkeepSays(hint < 10 ? hint : 0);
		}
	}

	_G(logic)->resumeTurn();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
