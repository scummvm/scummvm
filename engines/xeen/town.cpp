/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "xeen/town.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

Town::Town(XeenEngine *vm) : _vm(vm) {
	Common::fill(&_arr1[0], &_arr1[6], 0);
	_townMaxId = 0;
	_townActionId = 0;
	_townCurrent = 0;
	_currentCharLevel = 0;
	_v1 = 0;
	_v2 = 0;
	_donation = 0;
	_healCost = 0;
	_v5 = _v6 = 0;
	_v10 = _v11 = 0;
	_v12 = _v13 = 0;
	_v14 = 0;
	_v20 = 0;
	_uncurseCost = 0;
	_flag1 = false;
	_nextExperienceLevel = 0;
}

void Town::loadStrings(const Common::String &name) {
	File f(name);
	_textStrings.clear();
	while (f.pos() < f.size())
		_textStrings.push_back(f.readString());
	f.close();
}

int Town::townAction(int actionId) {
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	SoundManager &sound = *_vm->_sound;
	bool isDarkCc = _vm->_files->_isDarkCc;

	if (actionId == 12) {
		pyramidEvent();
		return 0;
	}

	_townMaxId = TOWN_MAXES[_vm->_files->_isDarkCc][actionId];
	_townActionId = actionId;
	_townCurrent = 0;
	_v1 = 0;
	_townPos = Common::Point(8, 8);
	intf._overallFrame = 0;

	// This area sets up the GUI buttos and startup sample to play for the
	// given town action
	Common::String vocName = "hello1.voc";
	clearButtons();
	_icons1.clear();
	_icons2.clear();

	switch (actionId) {
	case 0:
		// Bank
		_icons1.load("bank.icn");
		_icons2.load("bank2.icn");
		addButton(Common::Rect(234, 108, 259, 128), Common::KEYCODE_d, &_icons1);
		addButton(Common::Rect(261, 108, 285, 128), Common::KEYCODE_w, &_icons1);
		addButton(Common::Rect(288, 108, 312, 128), Common::KEYCODE_ESCAPE, &_icons1);
		intf._overallFrame = 1;

		sound.playSample(nullptr, 0);
		vocName = isDarkCc ? "bank1.voc" : "banker.voc";
		break;
	
	case 1:
		// Blacksmith
		_icons1.load("esc.icn");
		addButton(Common::Rect(261, 100, 285, 120), Common::KEYCODE_ESCAPE, &_icons1, true);
		addButton(Common::Rect(234, 54, 308, 62), 0, &_icons1, false);
		addButton(Common::Rect(234, 64, 308, 72), Common::KEYCODE_b, &_icons1, false);
		addButton(Common::Rect(234, 74, 308, 82), 0, &_icons1, false);
		addButton(Common::Rect(234, 84, 308, 92), 0, &_icons1, false);

		sound.playSample(nullptr, 0);
		vocName = isDarkCc ? "see2.voc" : "whaddayo.voc";
		break;

	case 2:
		// Guild
		loadStrings("spldesc.bin");
		_icons1.load("esc.icn");
		addButton(Common::Rect(261, 100, 285, 120), Common::KEYCODE_ESCAPE, &_icons1, true);
		addButton(Common::Rect(234, 54, 308, 62), 0, &_icons1, false);
		addButton(Common::Rect(234, 64, 308, 72), Common::KEYCODE_b, &_icons1, false);
		addButton(Common::Rect(234, 74, 308, 82), Common::KEYCODE_s, &_icons1, false);
		addButton(Common::Rect(234, 84, 308, 92), 0, &_icons1, false);
		_vm->_mode = MODE_17;

		sound.playSample(nullptr, 0);
		vocName = isDarkCc ? "parrot1.voc" : "guild10.voc";
		break;

	case 3:
		// Tavern
		loadStrings("tavern.bin");
		_icons1.load("tavern.icn");
		addButton(Common::Rect(281, 108, 305, 128), Common::KEYCODE_ESCAPE, &_icons1, true);
		addButton(Common::Rect(242, 108, 266, 128), Common::KEYCODE_s, &_icons1, true);
		addButton(Common::Rect(234, 54, 308, 62), Common::KEYCODE_d, &_icons1, false);
		addButton(Common::Rect(234, 64, 308, 72), Common::KEYCODE_f, &_icons1, false);
		addButton(Common::Rect(234, 74, 308, 82), Common::KEYCODE_t, &_icons1, false);
		addButton(Common::Rect(234, 84, 308, 92), Common::KEYCODE_r, &_icons1, false);
		_vm->_mode = MODE_17;

		sound.playSample(nullptr, 0);
		vocName = isDarkCc ? "hello1.voc" : "hello.voc";
		break;

	case 4:
		// Temple
		_icons1.load("esc.icn");
		addButton(Common::Rect(261, 100, 285, 120), Common::KEYCODE_ESCAPE, &_icons1, true);
		addButton(Common::Rect(234, 54, 308, 62), Common::KEYCODE_h, &_icons1, false);
		addButton(Common::Rect(234, 64, 308, 72), Common::KEYCODE_d, &_icons1, false);
		addButton(Common::Rect(234, 74, 308, 82), Common::KEYCODE_u, &_icons1, false);
		addButton(Common::Rect(234, 84, 308, 92), 0, &_icons1, false);

		sound.playSample(nullptr, 0);
		vocName = isDarkCc ? "help2.voc" : "maywe2.voc";
		break;

	case 5:
		// Training
		Common::fill(&_arr1[0], &_arr1[6], 0);
		_v2 = 0;

		_icons1.load("train.icn");
		addButton(Common::Rect(281, 108, 305, 128), Common::KEYCODE_ESCAPE, &_icons1, true);
		addButton(Common::Rect(242, 108, 266, 128), Common::KEYCODE_t, &_icons1, false);

		sound.playSample(nullptr, 0);
		vocName = isDarkCc ? "training.voc" : "youtrn1.voc";
		break;

	case 6:
		// Arena event
		arenaEvent();
		return false;

	case 8:
		// Reaper event
		reaperEvent();
		return false;

	case 9:
		// Golem event
		golemEvent();
		return false;

	case 10:
	case 13:
		dwarfEvent();
		return false;

	case 11:
		sphinxEvent();
		return false;

	default:
		break;
	}

	sound.loadMusic(TOWN_ACTION_MUSIC[actionId], 223);

	_townSprites.clear();
	for (int idx = 0; idx < TOWN_ACTION_FILES[isDarkCc][actionId]; ++idx) {
		Common::String shapesName = Common::String::format("%s%d.twn",
			TOWN_ACTION_SHAPES[actionId], idx + 1);
		_townSprites[idx].load(shapesName);
	}

	Character *charP = &party._activeParty[0];
	Common::String title = createTownText(*charP);
	intf._face1UIFrame = intf._face2UIFrame = 0;
	intf._dangerSenseUIFrame = 0; 
	intf._spotDoorsUIFrame = 0;
	intf._batUIFrame = 0;

	_townSprites[_townCurrent / 8].draw(screen, _townCurrent % 8, _townPos);
	if (actionId == 0 && isDarkCc) {
		_townSprites[4].draw(screen, _vm->getRandomNumber(13, 18),
			Common::Point(8, 30));
	}

	intf.assembleBorder();

	// Open up the window and write the string
	screen._windows[10].open();
	screen._windows[10].writeString(title);
	drawButtons(&screen);

	screen._windows[0].update();
	intf.highlightChar(0);
	intf.drawTownAnim(1);

	if (actionId == 0)
		intf._overallFrame = 2;

	File voc(vocName);
	sound.playSample(&voc, 1);

	do {
		townWait();
		charP = doTownOptions(charP);
		screen._windows[10].writeString(title);
		drawButtons(&screen);
	} while (!_vm->shouldQuit() && _buttonValue != Common::KEYCODE_ESCAPE);

	switch (actionId) {
	case 1:
		// Leave blacksmith
		if (isDarkCc) {
			sound.playSample(nullptr, 0);
			File f("come1.voc");
			sound.playSample(&f, 1);
		}
		break;

	case 3: {
		// Leave Tavern
		sound.playSample(nullptr, 0);
		File f(isDarkCc ? "gdluck1.voc" : "goodbye.voc");
		sound.playSample(&f, 1);

		map.mazeData()._mazeNumber = party._mazeId;
		break;
	}
	default:
		break;
	}

	int result;
	if (party._mazeId != 0) {
		map.load(party._mazeId);
		_v1 += 1440;
		party.addTime(_v1);
		result = 0;
	} else {
		_vm->_saves->saveChars();
		result = 2;
	}

	for (uint idx = 0; idx < _townSprites.size(); ++idx)
		_townSprites[idx].clear();
	intf.mainIconsPrint();
	_buttonValue = 0;

	return result;
}

void Town::townWait() {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;

	while (!_vm->shouldQuit()) {
		events.updateGameCounter();
		while (!_vm->shouldQuit() && !_buttonValue && events.timeElapsed() < 3) {
			checkEvents(_vm);
		}
		if (_buttonValue)
			return;

		intf.drawTownAnim(!_vm->_screen->_windows[11]._enabled);
	}
}

void Town::pyramidEvent() {
	error("TODO: pyramidEvent");
}

void Town::arenaEvent() {
	error("TODO: arenaEvent");
}

void Town::reaperEvent() {
	error("TODO: repearEvent");
}

void Town::golemEvent() {
	error("TODO: golemEvent");
}

void Town::sphinxEvent() {
	error("TODO: sphinxEvent");
}

void Town::dwarfEvent() {
	error("TODO: dwarfEvent");
}

Common::String Town::createTownText(Character &ch) {
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Common::String msg;

	switch (_townActionId) {
	case 0:
		// Bank
		return Common::String::format(BANK_TEXT,
			XeenEngine::printMil(party._bankGold).c_str(),
			XeenEngine::printMil(party._bankGems).c_str(),
			XeenEngine::printMil(party._gold).c_str(),
			XeenEngine::printMil(party._gems).c_str());
	case 1:
		// Blacksmith
		return Common::String::format(BLACKSMITH_TEXT,
			XeenEngine::printMil(party._gold));

	case 2:
		// Guild
		return !ch.guildMember() ? GUILD_NOT_MEMBER_TEXT :
			Common::String::format(GUILD_TEXT, ch._name.c_str());

	case 3:
		// Tavern
		return Common::String::format(TAVERN_TEXT, ch._name,
			FOOD_AND_DRINK, XeenEngine::printMil(party._gold).c_str());

	case 4:
		// Temple
		_donation = 0;
		_uncurseCost = 0;
		_v5 = 0;
		_v6 = 0;
		_healCost = 0;

		if (party._mazeId == (_vm->_files->_isDarkCc ? 29 : 28)) {
			_v10 = _v11 = _v12 = _v13 = 0;
			_v14 = 10;
		} else if (party._mazeId == (_vm->_files->_isDarkCc ? 31 : 30)) {
			_v13 = 10;
			_v12 = 50;
			_v11 = 500;
			_v10 = 100;
			_v14 = 25;
		} else if (party._mazeId == (_vm->_files->_isDarkCc ? 37 : 73)) {
			_v13 = 20;
			_v12 = 100;
			_v11 = 1000;
			_v10 = 200;
			_v14 = 50;
		} else if (_vm->_files->_isDarkCc || party._mazeId == 49) {
			_v13 = 100;
			_v12 = 500;
			_v11 = 5000;
			_v10 = 300;
			_v14 = 100;
		}

		_currentCharLevel = ch.getCurrentLevel();
		if (ch._currentHp < ch.getMaxHP()) {
			_healCost = _currentCharLevel * 10 + _v13;
		}

		for (int attrib = HEART_BROKEN; attrib <= UNCONSCIOUS; ++attrib) {
			if (ch._conditions[attrib])
				_healCost += _currentCharLevel * 10;
		}

		_v6 = 0;
		if (ch._conditions[DEAD]) {
			_v6 += (_currentCharLevel * 100) + (ch._conditions[DEAD] * 50) + _v12;
		}
		if (ch._conditions[STONED]) {
			_v6 += (_currentCharLevel * 100) + (ch._conditions[STONED] * 50) + _v12;
		}
		if (ch._conditions[ERADICATED]) {
			_v5 = (_currentCharLevel * 1000) + (ch._conditions[ERADICATED] * 500) + _v11;
		}

		for (int idx = 0; idx < 9; ++idx) {
			_uncurseCost |= ch._weapons[idx]._bonusFlags & 0x40;
			_uncurseCost |= ch._armor[idx]._bonusFlags & 0x40;
			_uncurseCost |= ch._accessories[idx]._bonusFlags & 0x40;
			_uncurseCost |= ch._misc[idx]._bonusFlags & 0x40;
		}

		if (_uncurseCost || ch._conditions[CURSED])
			_v5 = (_currentCharLevel * 20) + _v10;

		_donation = _flag1 ? 0 : _v14;
		_healCost += _v6 + _v5;

		return Common::String::format(TEMPLE_TEXT, ch._name.c_str(),
			_healCost, _donation, XeenEngine::printK(_uncurseCost).c_str(),
			XeenEngine::printMil(party._gold).c_str());

	case 5:
		// Training
		if (_vm->_files->_isDarkCc) {
			switch (party._mazeId) {
			case 29:
				_v20 = 30;
				break;
			case 31:
				_v20 = 50;
				break;
			case 37:
				_v20 = 200;
				break;
			default:
				_v20 = 100;
				break;
			}
		} else {
			switch (party._mazeId) {
			case 28:
				_v20 = 10;
				break;
			case 30:
				_v20 = 15;
				break;
			default:
				_v20 = 20;
				break;
			}
		}

		_nextExperienceLevel = ch.nextExperienceLevel();

		if (_nextExperienceLevel >= 0x10000 && ch._level._permanent < _v20) {
			int nextLevel = ch._level._permanent + 1;
			return Common::String::format(EXPERIENCE_FOR_LEVEL,
				ch._name.c_str(), _nextExperienceLevel, nextLevel);
		} else if (ch._level._permanent >= 20) {
			_nextExperienceLevel = 1;
			msg = Common::String::format(LEARNED_ALL, ch._name.c_str());
		} else {
			msg = Common::String::format(ELIGIBLE_FOR_LEVEL,
				ch._name.c_str(), ch._level._permanent + 1);
		}

		return Common::String::format(TRAINING_TEXT,
			XeenEngine::printMil(party._gold).c_str());

	default:
		return "";
	}
}

Character *Town::doTownOptions(Character *charP) {
	Common::String result;

	error("TODO: doTownOptions");
}


} // End of namespace Xeen
