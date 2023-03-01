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
	Location::msgFocus(msg);
	return true;
}

void Training::draw() {
	Location::draw();

	setReduced(false);
	writeLine(0, STRING["enhdialogs.training.title"], ALIGN_MIDDLE);
	writeLine(1, STRING["enhdialogs.location.options_for"], ALIGN_MIDDLE);
	writeLine(3, camelCase(g_globals->_currCharacter->_name), ALIGN_MIDDLE);

	writeLine(10, STRING["enhdialogs.location.gold"]);
	writeLine(10, Common::String::format("%d",
		g_globals->_currCharacter->_gold), ALIGN_RIGHT);

	setReduced(true);
	writeString(5, 122, STRING["enhdialogs.training.train"]);
	writeString(43, 122, STRING["enhdialogs.training.esc"]);
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

void Training::checkCharacter() {
	Character &c = *g_globals->_currCharacter;

	_currLevel = c._level._base;
	if (_currLevel >= MAX_LEVEL)
		return;

	// Initialize fields
	_expTotal = 0;
	_remainingExp = 0;
	_expAmount = 0;
	_cost = _cost2 = 0;
	_canTrain = false;
	_canAfford = false;
	_class = c._class;

	if (_class == KNIGHT || _class == CLERIC || _class == ROBBER) {
		_expTotal = 1500;
		_expAmount = 150000;

		if (_currLevel != 0) {
			_cost = _currLevel >= 8 ? 3000 :
				TRAINING_COSTS1[_currLevel - 1];
		}
	} else {
		_expTotal = 2000;
		_expAmount = 200000;
		_cost = _currLevel >= 8 ? 4000 :
			TRAINING_COSTS2[_currLevel - 1];
	}

	for (int level = _currLevel - 1, ctr = 0; level > 0; --level) {
		_expTotal *= 16;

		if (++ctr >= 7) {
			while (--level > 0)
				_expTotal += _expAmount;
			break;
		}
	}

	_remainingExp = _expTotal - c._exp;
	_canTrain = _remainingExp <= 0;
	_canAfford = (int)c._gold >= _cost;
}

void Training::train() {
	Character &c = *g_globals->_currCharacter;

	if (c._condition) {
		Sound::sound(SOUND_3);
		clearSurface();
		writeString(8, 5, STRING["dialogs.training.condition"]);
		delaySeconds(3);

	} else if (!_canAfford) {
		notEnoughGold();

	} else {
		// Do the actual training
		c._gold -= _cost;
		Character::LevelIncrease lvl = c.increaseLevel();
		Sound::sound(SOUND_2);

		clearSurface();
		writeString(0, 3, STRING["dialogs.training.congrats"]);
		writeNumber(c._level._base);

		writeString(7, 5, Common::String::format(
			STRING["dialogs.training.hp"].c_str(), lvl._numHP));

		if (lvl._numSpells != 0)
			writeString(7, 6, STRING["dialogs.training.new_spells"]);

		Sound::sound(SOUND_2);
		delaySeconds(10);
	}
}

} // namespace Location
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
