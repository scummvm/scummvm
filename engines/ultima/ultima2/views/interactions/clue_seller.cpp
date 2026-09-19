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
	"ASK ME NO QUESTIONS,\nI'LL TELL YOU NO LIES.",
	"THE QUEEN IS THE KING\nAND THE KING IS A SPY.",
	"THE EVIL MEN DO\nIS A HORRIBLE THING.",
	"BUT DISPELLING MEANS\nTHAT YOU MUST HAVE THE RING.",
	"JUST WHERE IT IS,\nI CANNOT SAY.",
	"BUT I'LL TELL YOU RIGHT NOW,\nTHERE IS NO OTHER WAY.",
	"I HEAR IN A TOWN\nWHERE THE WATER RUNS FREE.",
	"THERE IS AN OLD MAN\nLIVING UNDER A TREE.",
	"HE HAS NOT A NAME\nBUT HE DOES HAVE A CLUE",
	"MAYHAPS IF YOU FIND HIM,\nHE'LL TELL IT TO YOU!"
};

ClueSeller::ClueSeller() : Interaction("ClueSeller") {
}

bool ClueSeller::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	writeString("TIME AND TRAVEL MADE ME WISE,\nPRECIOUS GOLD, A CLUE IT BUYS!HOW MUCH WILL YOU PAY *100? ");

	return Interaction::msgFocus(msg);
}

bool ClueSeller::msgKeypress(const KeypressMessage &msg) {
	if (msg.ascii < '0' || msg.ascii > '9')
		return true;

	int digit = msg.ascii - '0';
	writeString("%d\n", digit);
	close();

	if (_G(logic)->trySpendGold(digit * 100))
		writeString("%s\n", CLUES[digit]);

	_G(logic)->resumeTurn();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
