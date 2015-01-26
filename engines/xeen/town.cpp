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
	_townMaxId = 0;
	_townActionId = 0;
	_townCurrent = 0;
	_v1 = 0;
	_v2 = 0;
	Common::fill(&_arr1[0], &_arr1[6], 0);
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

	Common::String title = createTownText();
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
		doTownOptions();
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

Common::String Town::createTownText() {
	error("TODO");
}

void Town::doTownOptions() {
	error("TODO: doTownOptions");
}


} // End of namespace Xeen
