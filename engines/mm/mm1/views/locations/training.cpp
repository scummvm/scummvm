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

#include "mm/mm1/views/locations/training.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

Training::Training() : Location("Training") {
}

bool Training::msgFocus(const FocusMessage &msg) {
	send("View", GameMessage("LOCATION", LOC_TRAINING));
	changeCharacter(0);

	return true;
}

void Training::changeCharacter(uint index) {
	Location::changeCharacter(index);
	checkCharacter();
}

void Training::timeout() {
	checkCharacter();
	Location::timeout();
}

void Training::checkCharacter() {
	Character &c = *g_globals->_currCharacter;

	_currLevel = c._level._base;
	if (_currLevel >= MAX_LEVEL) {
		_canTrain = false;
		return;
	}

	// Initialize fields
	_expTotal = 0;
	_remainingExp = 0;
	_expAmount = 0;
	_cost = _cost2 = 0;
	_canTrain = false;
	_canAfford = false;
	_class = c._class;

	if (_class == KNIGHT || _class == CLERIC || _class == ROBBER) {
		if (_currLevel != 0) {
			_cost = _currLevel >= 8 ? 3000 :
				TRAINING_COSTS1[_currLevel - 1];
		}
	} else {
		_cost = _currLevel >= 8 ? 4000 :
			TRAINING_COSTS2[_currLevel - 1];
	}

	_expTotal = c.experienceForNextLevel();
	_remainingExp = (int)_expTotal - (int)c._exp;
	_canTrain = _remainingExp <= 0;
	_canAfford = (int)c._gold >= _cost;
}

void Training::draw() {
	Location::draw();

	writeString(18, 1, "Training for level ");
	writeNumber(_currLevel + 1);

	if (_currLevel >= MAX_LEVEL) {
		writeString(24, 3, "No way!");

	} else if (_remainingExp > 0) {
		writeString(21, 3, Common::String::format(
			"Need %d more", _remainingExp));
		writeString(20, 5, "experience points");

	} else {
		writeString(21, 3, Common::String::format(
			"Cost %d gold", _cost));
		writeString(18, 5, "Cost %d gold");
	}
}

bool Training::msgKeypress(const KeypressMessage &msg) {
	// If a delay is active, end it
	if (endDelay())
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_a:
		if (_canTrain)
			train();
		break;
	case Common::KEYCODE_g:
		g_globals->_currCharacter->gatherGold();
		redraw();
		break;
	case Common::KEYCODE_1:
	case Common::KEYCODE_2:
	case Common::KEYCODE_3:
	case Common::KEYCODE_4:
	case Common::KEYCODE_5:
	case Common::KEYCODE_6:
		changeCharacter(msg.keycode - Common::KEYCODE_1);
		break;
	default:
		break;
	}

	return true;
}

bool Training::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	if (msg._action == KEYBIND_ESCAPE) {
		leave();
		return true;
	}

	return false;
}

void Training::train() {
	Character &c = *g_globals->_currCharacter;

	if (c._condition) {
		Sound::sound(SOUND_3);
		clearSurface();
		writeString(8, 5, "*** Check condition ***");
		delaySeconds(3);

	} else if (!_canAfford) {
		notEnoughGold();

	} else {
		// Do the actual training
		c._gold -= _cost;
		Character::LevelIncrease lvl = c.increaseLevel();
		Sound::sound(SOUND_2);

		clearSurface();
		writeString(0, 3, "Congratulations! you are now level ");
		writeNumber(c._level._base);

		writeString(7, 5, Common::String::format(
			"You gained %d hit points", lvl._numHP));

		if (lvl._numSpells != 0)
			writeString(7, 6, "You gained new spells!");

		Sound::sound(SOUND_2);
		delaySeconds(10);
	}
}

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM
