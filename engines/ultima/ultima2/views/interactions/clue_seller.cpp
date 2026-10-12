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

#include "ultima/ultima2/views/interactions/clue_seller.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

static const char *const CLUES[10] = {
	"Ask me no questions,\nI'll tell you no lies.",
	"The Queen is the King\nand the King is a spy.",
	"The evil men do\nis a horrible thing.",
	"But dispelling means\nthat you must have the Ring.",
	"Just where it is,\nI cannot say.",
	"But I'll tell you right now,\nthere is no other way.",
	"I hear in a town\nwhere the water runs free.",
	"There is an old man\nliving under a tree.",
	"He has not a name\nbut he does have a clue",
	"Mayhaps if you find him,\nhe'll tell it to you!"
};

ClueSeller::ClueSeller() : Interaction("ClueSeller") {
}

bool ClueSeller::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	writeString("Time and travel made me wise,\nPrecious gold, a clue it buys!How much will you pay *100? ");

	return Interaction::msgFocus(msg);
}

bool ClueSeller::msgKeypress(const KeypressMessage &msg) {
	close();

	// Any other key walks away
	if (msg.ascii < '0' || msg.ascii > '9') {
		writeString("\n");
	} else {
		int digit = msg.ascii - '0';
		writeString("%d\n", digit);

		if (_G(logic)->trySpendGold(digit * 100))
			writeString("%s\n", CLUES[digit]);
	}

	_G(logic)->resumeTurn();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
