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

#include "ultima/ultima3/views/interactions/insert_card.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr byte MACHINE_CELL = 0x7C;
constexpr byte FLASH_CELL = 0xF0;
constexpr byte EMPTY_CELL = 0x20;
constexpr int FIRST_SLOT_X = 0x1E;
constexpr int CARD_COUNT = 4;
constexpr int FLASH_COUNT = 5;
constexpr int FLASH_STEP_FRAMES = 2;
constexpr int VIEWPORT_LEFT = 8, VIEWPORT_RIGHT = 184;

// The word that goes with each card, in the order they go in
static const char CARD_WORDS[CARD_COUNT + 1] = "LSMD";

// Which pixel columns each step of the ending inverts: 3 for both, 2 for the
// odd ones and 1 for the even ones
static const byte SHATTER_STEPS[21] = {
	3, 2, 1, 3, 2, 3, 1, 3, 2, 1, 3, 2, 3, 1, 3, 2, 1, 3, 2, 3, 1
};

static const char *const VICTORY_LINES[10] = {
	"And so it came to", "pass  that  on  this", "day EXODUS,hell-born",
	"incarnate  of  evil,", "was vanquished  from", "Sosaria.    What now",
	"lies  ahead  in  the", "ULTIMA saga can only", "be pure speculation!",
	"Onward to ULTIMA IV!"
};

static void noEffect() {
	_G(messages).print("No effect!\n");
}

void InsertCard::start(int slot) {
	_slot = slot;
	_stage = DIRECTION;
	_G(messages).print("Direct? ");
}

bool InsertCard::chooseCell() {
	const Data::Savegame &save = _G(savegame);
	Direction dir = _directions.direction();

	_targetX = (save._posX + (dir == DIR_EAST ? 1 : (dir == DIR_WEST ? -1 : 0))) & (Data::MAP_SIZE - 1);
	_targetY = (save._posY + (dir == DIR_SOUTH ? 1 : (dir == DIR_NORTH ? -1 : 0))) & (Data::MAP_SIZE - 1);

	if (_G(map).cell(_targetX, _targetY) != MACHINE_CELL) {
		noEffect();
		return true;
	}

	static const char *const WORDS[6] = { "Moons", "Death", "Love", "Sol", "Quit", "Quit" };
	_G(messages).print("D, S, L, M:\n");
	_menu.setup("MDLSQ\033", WORDS, 6);
	_stage = CHOOSE_WORD;
	return false;
}

bool InsertCard::tryWord() {
	Data::RosterEntry &e = _G(savegame).partyMember(_slot);
	int progress = _G(exodusProgress);
	char key = _menu.key();

	_G(messages).print("\n");
	if (key == 'Q' || key == '\033')
		return true;

	// The card for this slot has to be in this person's hands
	if (!(e._marksAndCards & (1 << progress))) {
		noEffect();
		return true;
	}

	if (_targetX != FIRST_SLOT_X + progress || key != CARD_WORDS[progress]) {
		// Anything else is fatal
		_G(effects).flashSlot(_slot);
		g_engine->playSoundEffect(0xF7);
		e._hitPoints = 0;
		Logic().damageCharacter(_slot, 0xFF);
		return true;
	}

	++_G(exodusProgress);
	_step = 0;
	_frames = 0;
	_stage = FLASH;
	return false;
}

void InsertCard::startVictory() {
	const Data::Savegame &save = _G(savegame);

	_G(messages).print("\nCongratulations!\n   Thou hast\n   compleated\nExodus: Ultima 3\n       in\n");
	_G(messages).print(Common::String::format("%02X%02X%02X%02X moves\n", save._moveCount[3],
		save._moveCount[2], save._moveCount[1], save._moveCount[0]).c_str());
	_G(messages).print("Report thy feat!");

	_step = 0;
	_stage = SHATTER;
}

bool InsertCard::keypress(const KeypressMessage &msg) {
	switch (_stage) {
	case DIRECTION: {
		DirectionChooser::Result result = _directions.handleKey(msg);
		if (result == DirectionChooser::PENDING)
			return false;
		if (result == DirectionChooser::CANCELLED)
			return true;

		return chooseCell();
	}

	case CHOOSE_WORD:
		return _menu.handleKey(msg) ? tryWord() : false;

	case DONE:
		if (!isModifierKey(msg.keycode)) {
			_G(soundEnabled) = true;
			g_events->replaceView("Title", true);
		}
		return false;

	default:
		return false;
	}
}

bool InsertCard::timeout() {
	if (_stage == FLASH) {
		if (++_frames < FLASH_STEP_FRAMES)
			return false;
		_frames = 0;

		// The slot lights up and goes dark again, and then stays empty
		if (_step < FLASH_COUNT * 2) {
			_G(map).setCell(_targetX, _targetY, (_step & 1) ? MACHINE_CELL : FLASH_CELL);
			g_engine->playSoundEffect(0xF7);
			++_step;
			return false;
		}

		_G(map).setCell(_targetX, _targetY, EMPTY_CELL);
		if (_G(exodusProgress) != CARD_COUNT)
			return true;

		startVictory();
	} else if (_stage == SHATTER) {
		g_engine->playSoundEffect(0xF7);
		byte step = SHATTER_STEPS[_step];
		_invertEven ^= (step & 1) != 0;
		_invertOdd ^= (step & 2) != 0;

		if (++_step == ARRAYSIZE(SHATTER_STEPS))
			_stage = TEXT;
	} else if (_stage == TEXT) {
		_stage = DONE;
	}

	return false;
}

void InsertCard::draw(GfxSurface &s) {
	if (_stage == SHATTER) {
		for (int y = VIEWPORT_LEFT; y < VIEWPORT_RIGHT; ++y) {
			byte *p = (byte *)s.getBasePtr(VIEWPORT_LEFT, y);

			for (int x = VIEWPORT_LEFT; x < VIEWPORT_RIGHT; ++x, ++p) {
				if ((x & 1) ? _invertOdd : _invertEven)
					*p ^= 3;
			}
		}

		s.addDirtyRect(Common::Rect(VIEWPORT_LEFT, VIEWPORT_LEFT, VIEWPORT_RIGHT, VIEWPORT_RIGHT));
	} else if (_stage == TEXT || _stage == DONE) {
		s.fillRect(Common::Rect(VIEWPORT_LEFT, VIEWPORT_LEFT, VIEWPORT_RIGHT, VIEWPORT_RIGHT), 0);

		for (int i = 0; i < ARRAYSIZE(VICTORY_LINES); ++i)
			s.writeString(Common::Point(2, 3 + i * 2), VICTORY_LINES[i]);
	}
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
