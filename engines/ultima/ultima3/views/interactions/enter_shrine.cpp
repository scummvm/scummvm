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

#include "common/file.h"
#include "ultima/ultima3/views/interactions/enter_shrine.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr int ATTRIBUTES = 4;

// The names of the four shrines, and the most each race can gain at each of them
static const char *const SHRINE_NAMES[ATTRIBUTES] = {
	"    Strength\n\n", "   Dexterity\n\n", "  Intelligence\n\n", "     Wisdom\n\n"
};

static const byte ATTRIBUTE_LIMITS[Data::RACE_COUNT][ATTRIBUTES] = {
	{ 0x75, 0x75, 0x75, 0x75 }, { 0x75, 0x99, 0x75, 0x50 }, { 0x99, 0x75, 0x50, 0x75 },
	{ 0x75, 0x50, 0x75, 0x99 }, { 0x25, 0x99, 0x99, 0x75 }
};

EnterShrine::~EnterShrine() {
	_G(sceneShown) = false;
}

bool EnterShrine::keypress(const KeypressMessage &msg) {
	Data::Savegame &save = _G(savegame);

	if (!_offering) {
		PlayerChooser::Result result = _chooser.handleKey(msg);
		if (result == PlayerChooser::PENDING)
			return false;
		if (result == PlayerChooser::CANCELLED)
			return true;

		if (!save.partyMember(_chooser.slot()).isAlive()) {
			_G(messages).print("Incapacitated!\n");
			g_engine->playSoundEffect(0xFF);
			return true;
		}

		Common::File f;
		if (!f.open("SHRINE.IMG") || f.read(_G(scene), sizeof(_G(scene))) != sizeof(_G(scene)))
			error("Could not load SHRINE.IMG");

		_G(sceneShown) = true;
		_G(messages).print("\n Welcome to the\n   Shrine of\n");
		_G(messages).print(SHRINE_NAMES[save._posX & 3]);
		_G(messages).print("Offering*100-");
		_number.setup(2);
		_offering = true;
		return false;
	}

	if (!_number.handleKey(msg))
		return false;

	Data::RosterEntry &e = save.partyMember(_chooser.slot());
	const int attribute = save._posX & 3;
	const int offering = _number.hasLetters() ? 0 : _number.value();

	// The offering is made in hundreds of gold
	if (offering == 0) {
		_G(messages).print("\nThen be off!\n");
		return true;
	}

	if (offering * 100 > Data::fromBcdWord(e._gold)) {
		_G(messages).print("\nYou can't cheat\nthe Gods!\n");
		g_engine->playSoundEffect(0xFF);
		return true;
	}

	e._gold = Data::toBcdWord(Data::fromBcdWord(e._gold) - offering * 100);

	// Whichever attribute the shrine is for goes up by as much, to the most the race can reach
	byte *attributes[ATTRIBUTES] = { &e._strength, &e._dexterity, &e._intelligence, &e._wisdom };
	int race = Data::lookupIndex(e._race, Data::RACE_KEYS, Data::RACE_COUNT);
	int limit = Data::fromBcd(ATTRIBUTE_LIMITS[race][attribute]);

	*attributes[attribute] = Data::toBcd(MIN(Data::fromBcd(*attributes[attribute]) + offering, limit));

	_G(messages).print("\nShazam!\n");
	_G(effects).flashSlot(_chooser.slot());
	_G(effects).flashViewport();
	g_engine->playSoundEffect(0xFD);
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
