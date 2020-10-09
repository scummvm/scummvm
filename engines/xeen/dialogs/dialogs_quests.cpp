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
#include "xeen/dialogs/dialogs_quests.h"
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
	Windows &windows = *_vm->_windows;
	Mode oldMode = _vm->_mode;
	int count = 0;
	bool headerShown = false;
	int topRow = 0;
	const char **questItemNames = (g_vm->getGameID() == GType_Swords) ? Res.QUEST_ITEM_NAMES_SWORDS : Res.QUEST_ITEM_NAMES;
	int itemsCount = (g_vm->getGameID() == GType_Swords) ? TOTAL_QUEST_ITEMS_SWORDS : TOTAL_QUEST_ITEMS;
	const char *title1 = (g_vm->getGameID() == GType_Swords) ? Res.SWORDS_OF_XEEN_LINE : Res.CLOUDS_OF_XEEN_LINE;

	addButtons();
	loadQuestNotes();

	enum { QUEST_ITEMS, CURRENT_QUESTS, AUTO_NOTES } mode = QUEST_ITEMS;
	bool windowFlag;
	if (windows[29]._enabled) {
		windowFlag = false;
	} else {
		windows[29].open();
		windows[30].open();
		windowFlag = true;
	}

	windows[29].writeString(Res.QUESTS_DIALOG_TEXT);
	drawButtons(&windows[0]);

	while (!_vm->shouldExit()) {
		Common::String lines[MAX_DIALOG_LINES];

		switch (mode) {
		case QUEST_ITEMS:
			for (int idx = 0; idx < itemsCount; ++idx)
				lines[idx] = "\b \b*";

			count = 0;
			headerShown = false;
			for (int idx = 0; idx < itemsCount; ++idx) {
				if (party._questItems[idx]) {
					if (!count ) {
						if (_vm->getGameID() == GType_Swords)
							lines[count++] = Res.SWORDS_OF_XEEN_LINE;
						else if (idx < 35)
							lines[count++] = Res.CLOUDS_OF_XEEN_LINE;
					} else if (_vm->getGameID() != GType_Swords && idx >= 35 && !headerShown) {
						lines[count++] = Res.DARKSIDE_OF_XEEN_LINE;
						headerShown = true;
					}

					bool multiFlag = false;
					if (_vm->getGameID() == GType_Swords) {
						multiFlag = (idx == 20) || (idx == 27) || (idx == 41);
					} else {
						switch (idx) {
						case 17:
						case 26:
						case 79:
						case 80:
						case 81:
						case 82:
						case 83:
						case 84:
							multiFlag = true;
							break;
						default:
							break;
						}
					}

					if (multiFlag) {
						lines[count++] = Common::String::format("%d %s%c",
							party._questItems[idx], questItemNames[idx],
							party._questItems[idx] == 1 ? ' ' : 's');
					} else {
						lines[count++] = questItemNames[idx];
					}
				}
			}

			if (count == 0) {
				windows[30].writeString(Res.NO_QUEST_ITEMS);
			} else {
				windows[30].writeString(Common::String::format(Res.QUEST_ITEMS_DATA,
					lines[topRow].c_str(), lines[topRow + 1].c_str(),
					lines[topRow + 2].c_str(), lines[topRow + 3].c_str(),
					lines[topRow + 4].c_str(), lines[topRow + 5].c_str(),
					lines[topRow + 6].c_str(), lines[topRow + 7].c_str(),
					lines[topRow + 8].c_str()
				));
			}
			break;

		case CURRENT_QUESTS:
			for (int idx = 0; idx < itemsCount; ++idx)
				lines[idx] = "";

			count = 0;
			headerShown = false;
			for (int idx = 0; idx < TOTAL_QUEST_FLAGS; ++idx) {
				if (party._questFlags[idx + 1]) {
					if (!count && !headerShown && (_vm->getGameID() == GType_Swords || idx < 29)) {
						lines[count++] = title1;
					}
					if (_vm->getGameID() != GType_Swords && idx > 28 && !headerShown) {
						lines[count++] = Res.DARKSIDE_OF_XEEN_LINE;
						headerShown = true;
					}

					lines[count++] = _questNotes[idx];
				}
			}

			if (count == 0)
				lines[1] = Res.NO_CURRENT_QUESTS;

			windows[30].writeString(Common::String::format(Res.CURRENT_QUESTS_DATA,
				lines[topRow].c_str(), lines[topRow + 1].c_str(), lines[topRow + 2].c_str()));
			break;

		case AUTO_NOTES: {
			int max, offset;
			switch (_vm->getGameID()) {
			case GType_Swords:
				max = 49;
				offset = 51;
				break;
			case GType_Clouds:
				max = MAX_DIALOG_LINES;
				offset = 31;
				break;
			default:
				max = MAX_DIALOG_LINES;
				offset = 56;
				break;
			}

			for (int idx = 0; idx < max; ++idx)
				lines[idx] = "";

			count = 0;
			headerShown = false;
			for (int idx = 0; idx < max; ++idx) {
				if (party._worldFlags[idx + (_vm->getGameID() != GType_Swords ? 1 : 0)]) {
					if (!count && !headerShown && (_vm->getGameID() == GType_Swords || idx < 72)) {
						lines[count++] = title1;
					}
					if (idx >= 72 && !headerShown) {
						lines[count++] = Res.DARKSIDE_OF_XEEN_LINE;
						headerShown = true;
					}

					lines[count++] = _questNotes[idx + offset];
				}
			}

			if (count == 0)
				lines[1] = Res.NO_AUTO_NOTES;

			windows[30].writeString(Common::String::format(Res.AUTO_NOTES_DATA,
				lines[topRow].c_str(), lines[topRow + 1].c_str(),
				lines[topRow + 2].c_str(), lines[topRow + 3].c_str(),
				lines[topRow + 4].c_str(), lines[topRow + 5].c_str(),
				lines[topRow + 6].c_str(), lines[topRow + 7].c_str(),
				lines[topRow + 8].c_str()
			));
			break;
		}

		default:
			break;
		}

		windows[30].writeString("\v000\t000");
		windows[24].update();

		// Key handling
		_buttonValue = 0;
		while (!_vm->shouldExit() && !_buttonValue) {
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
			topRow = MAX(count - 1, 0);
			break;
		case Common::KEYCODE_PAGEUP:
			topRow = MAX(topRow - 3, 0);
			break;
		case Common::KEYCODE_PAGEDOWN:
			topRow = CLIP(topRow + 3, 0, MAX(count - 1, 0));
			break;
		case Common::KEYCODE_UP:
		case Common::KEYCODE_KP8:
		case Common::KEYCODE_u:
			topRow = MAX(topRow - 1, 0);
			break;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_KP2:
		case Common::KEYCODE_d:
			topRow = CLIP(topRow + 1, 0, MAX(count - 1, 0));
			break;
		default:
			break;
		}
	}

	if (windowFlag) {
		windows[30].close();
		windows[29].close();
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
	File f("qnotes.bin", 1);
	while (f.pos() < f.size())
		_questNotes.push_back(f.readString());
	f.close();
}

} // End of namespace Xeen
