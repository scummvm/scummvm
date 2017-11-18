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

#include "common/scummsys.h"
#include "xeen/dialogs_quests.h"
#include "xeen/events.h"
#include "xeen/party.h"
#include "xeen/xeen.h"

namespace Xeen {

#define MAX_DIALOG_LINES 128

void Quests::show(XeenEngine *vm) {
	Quests *dlg = new Quests(vm);
	dlg->execute();
	delete dlg;
}

void Quests::execute() {
	EventsManager &events = *_vm->_events;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Mode oldMode = _vm->_mode;
	int count = 0;
	bool headerShown = false;
	int topRow = 0;

	addButtons();
	loadQuestNotes();

	enum { QUEST_ITEMS, CURRENT_QUESTS, AUTO_NOTES } mode = QUEST_ITEMS;
	bool windowFlag;
	if (screen._windows[29]._enabled) {
		windowFlag = false;
	} else {
		screen._windows[29].open();
		screen._windows[30].open();
		windowFlag = true;
	}

	screen._windows[29].writeString(Res.QUESTS_DIALOG_TEXT);
	drawButtons(&screen);

	while (!_vm->shouldQuit()) {
		Common::String lines[MAX_DIALOG_LINES];

		switch (mode) {
		case QUEST_ITEMS:
			for (int idx = 0; idx < TOTAL_QUEST_ITEMS; ++idx)
				lines[idx] = "\b \b*";

			count = 0;
			headerShown = false;
			for (int idx = 0; idx < TOTAL_QUEST_ITEMS; ++idx) {
				if (party._questItems[idx]) {
					if (!count && !headerShown && idx < 35) {
						lines[count++] = Res.CLOUDS_OF_XEEN_LINE;
					}
					if (idx >= 35 && !headerShown) {
						lines[count++] = Res.DARKSIDE_OF_XEEN_LINE;
						headerShown = true;
					}

					switch (idx) {
					case 17:
					case 26:
					case 79:
					case 80:
					case 81:
					case 82:
					case 83:
					case 84:
						lines[count++] = Common::String::format("%d %s%c",
							party._questItems[idx], Res.QUEST_ITEM_NAMES[idx],
							party._questItems[idx] == 1 ? ' ' : 's');
						break;
					default:
						lines[count++] = Res.QUEST_ITEM_NAMES[idx];
						break;
					}
				}
			}

			if (count == 0) {
				screen._windows[30].writeString(Res.NO_QUEST_ITEMS);
			} else {
				screen._windows[30].writeString(Common::String::format(Res.QUEST_ITEMS_DATA,
					lines[topRow].c_str(), lines[topRow + 1].c_str(),
					lines[topRow + 2].c_str(), lines[topRow + 3].c_str(),
					lines[topRow + 4].c_str(), lines[topRow + 5].c_str(),
					lines[topRow + 6].c_str(), lines[topRow + 7].c_str(),
					lines[topRow + 8].c_str()
				));
			}
			break;

		case CURRENT_QUESTS:
			for (int idx = 0; idx < TOTAL_QUEST_ITEMS; ++idx)
				lines[idx] = "";

			count = 0;
			headerShown = false;
			for (int idx = 0; idx < TOTAL_QUEST_FLAGS; ++idx) {
				if (party._questFlags[(idx + 1) / 30][(idx + 1) % 30]) {
					if (!count && !headerShown && idx < 29) {
						lines[count++] = Res.CLOUDS_OF_XEEN_LINE;
					}
					if (idx > 28 && !headerShown) {
						lines[count++] = Res.DARKSIDE_OF_XEEN_LINE;
						headerShown = true;
					}

					lines[count++] = _questNotes[idx];
				}
			}

			if (count == 0)
				lines[1] = Res.NO_CURRENT_QUESTS;

			screen._windows[30].writeString(Common::String::format(Res.CURRENT_QUESTS_DATA,
				lines[topRow].c_str(), lines[topRow + 1].c_str(), lines[topRow + 2].c_str()));
			break;

		case AUTO_NOTES:
			for (int idx = 0; idx < MAX_DIALOG_LINES; ++idx)
				lines[idx] = "";

			count = 0;
			headerShown = false;
			for (int idx = 0; idx < MAX_DIALOG_LINES; ++idx) {
				if (party._worldFlags[idx]) {
					if (!count && !headerShown && idx < 72) {
						lines[count++] = Res.CLOUDS_OF_XEEN_LINE;
					}
					if (idx >= 72 && !headerShown) {
						lines[count++] = Res.DARKSIDE_OF_XEEN_LINE;
						headerShown = true;
					}

					lines[count++] = _questNotes[idx + 56];
				}
			}

			if (count == 0)
				lines[1] = Res.NO_AUTO_NOTES;

			screen._windows[30].writeString(Common::String::format(Res.AUTO_NOTES_DATA,
				lines[topRow].c_str(), lines[topRow + 1].c_str(),
				lines[topRow + 2].c_str(), lines[topRow + 3].c_str(),
				lines[topRow + 4].c_str(), lines[topRow + 5].c_str(),
				lines[topRow + 6].c_str(), lines[topRow + 7].c_str(),
				lines[topRow + 8].c_str()
			));
			break;
		}

		screen._windows[30].writeString("\v000\t000");
		screen._windows[24].update();

		// Key handling
		_buttonValue = 0;
		while (!_vm->shouldQuit() && !_buttonValue) {
			events.pollEventsAndWait();
			checkEvents(_vm);
		}

		if (_buttonValue == Common::KEYCODE_ESCAPE)
			break;

		switch (_buttonValue) {
		case Common::KEYCODE_a:
			mode = AUTO_NOTES;
			topRow = 0;
			break;
		case Common::KEYCODE_i:
			mode = QUEST_ITEMS;
			topRow = 0;
			break;
		case Common::KEYCODE_q:
			mode = CURRENT_QUESTS;
			topRow = 0;
			break;
		case Common::KEYCODE_HOME:
			topRow = 0;
			break;
		case Common::KEYCODE_END:
			topRow = count - 1;
			break;
		case Common::KEYCODE_PAGEUP:
			topRow = MAX(topRow - 3, 0);
			break;
		case Common::KEYCODE_PAGEDOWN:
			topRow = CLIP(topRow + 3, 0, count - 1);
			break;
		case Common::KEYCODE_UP:
		case Common::KEYCODE_KP8:
			topRow = MAX(topRow - 1, 0);
			break;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_KP2:
			topRow = CLIP(topRow + 1, 0, count - 1);
			break;
		default:
			break;
		}
	}

	if (windowFlag) {
		screen._windows[30].close();
		screen._windows[29].close();
	}
	_vm->_mode = oldMode;
}

void Quests::addButtons() {
	_iconSprites.load("quest.icn");


	addButton(Common::Rect(12, 109, 36, 129), Common::KEYCODE_i, &_iconSprites);
	addButton(Common::Rect(80, 109, 104, 129), Common::KEYCODE_q, &_iconSprites);
	addButton(Common::Rect(148, 109, 172, 129), Common::KEYCODE_a, &_iconSprites);
	addButton(Common::Rect(216, 109, 240, 129), Common::KEYCODE_UP, &_iconSprites);
	addButton(Common::Rect(250, 109, 274, 129), Common::KEYCODE_DOWN, &_iconSprites);
	addButton(Common::Rect(284, 109, 308, 129), Common::KEYCODE_ESCAPE, &_iconSprites);
}

void Quests::loadQuestNotes() {
	File f("qnotes.bin", _vm->getGameID() == GType_Clouds ? GAME_ARCHIVE : ALTSIDE_ARCHIVE);
	while (f.pos() < f.size())
		_questNotes.push_back(f.readString());
	f.close();
}

} // End of namespace Xeen
