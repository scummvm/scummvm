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

#include "mm/mm1/views_enh/locations/training.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/shared/utils/strings.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

Training::Training() : Location("Training", LOC_TRAINING) {
	_trainSprite.load("train.icn");
	addButton(&_trainSprite, Common::Point(5, 100), 2, Common::KEYCODE_t);
	addButton(&_escSprite, Common::Point(40, 100), 0, KEYBIND_ESCAPE);
}

bool Training::msgFocus(const FocusMessage &msg) {
	if (!g_globals->_currCharacter && !g_globals->_party.empty())
		g_globals->_currCharacter = &g_globals->_party[0];

	checkCharacter();
	Location::msgFocus(msg);
	return true;
}

void Training::draw() {
	Location::draw();
	const Character &c = *g_globals->_currCharacter;

	setReduced(false);
	writeLine(0, "Training", ALIGN_MIDDLE);
	writeLine(1, "options", ALIGN_MIDDLE);

	if (_currLevel >= MAX_LEVEL) {
		writeLine(3, c._name, ALIGN_MIDDLE);
		writeLine(5, "No way!", ALIGN_MIDDLE);

	} else if (_remainingExp > 0) {
		writeLine(3, Common::String::format(
			"%s\nneeds %d experience for level %d.",
			c._name, _remainingExp, _currLevel + 1), ALIGN_MIDDLE);

	} else {
		writeLine(3, Common::String::format(
			"%s\nis eligible for level %d.",
			c._name, _currLevel + 1), ALIGN_MIDDLE);
	}

	writeLine(10, "Gold");
	writeLine(10, Common::String::format("%d", c._gold), ALIGN_RIGHT);

	setReduced(true);
	writeString(5, 122, "\x01""37Train");
	writeString(43, 122, "ESC");
}

bool Training::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE")
		checkCharacter();
	Location::msgGame(msg);
	return true;
}

void Training::charSwitched(Character *priorChar) {
	checkCharacter();
	PartyView::charSwitched(priorChar);
}

bool Training::msgKeypress(const KeypressMessage &msg) {
	// If a delay is active, end it
	if (endDelay())
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_t:
		if (_canTrain)
			train();
		break;
	case Common::KEYCODE_g:
		g_globals->_currCharacter->gatherGold();
		redraw();
		break;
	default:
		return Location::msgKeypress(msg);
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

	return Location::msgAction(msg);
}

void Training::checkCharacter() {
	assert(g_globals->_currCharacter);
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

void Training::train() {
	Character &c = *g_globals->_currCharacter;

	if (c._condition) {
		// Having a condition prevents characters from training
		Sound::sound(SOUND_3);
		displayMessage("*** Check condition ***");

	} else if (!_canAfford) {
		// Can't afford training
		notEnoughGold();

	} else {
		// Do the actual training
		c._gold -= _cost;
		Character::LevelIncrease lvl = c.increaseLevel();
		Sound::sound(SOUND_2);

		Common::String msg = Common::String::format("Congratulations! you are now level %d",
			c._level._base);

		msg = Common::String::format("You gained %d hit points",
			lvl._numHP);

		if (lvl._numSpells != 0) {
			msg += ". ";
			msg += "You gained new spells!";
		}

		displayMessage(msg);
	}
}

void Training::messageShown() {
	checkCharacter();
	redraw();
}

} // namespace Location
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
