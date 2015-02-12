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
#include "xeen/dialogs_char_info.h"
#include "xeen/dialogs_party.h"
#include "xeen/character.h"
#include "xeen/events.h"
#include "xeen/party.h"
#include "xeen/xeen.h"

namespace Xeen {

PartyDialog::PartyDialog(XeenEngine *vm) : ButtonContainer(), 
		PartyDrawer(vm), _vm(vm) {
	initDrawStructs();
}

void PartyDialog::show(XeenEngine *vm) {
	PartyDialog *dlg = new PartyDialog(vm);
	dlg->execute();
	delete dlg;
}

void PartyDialog::execute() {
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	SoundManager &sound = *_vm->_sound;
	bool modeFlag = false;
	int startingChar = 0;

	loadButtons();
	setupBackground();

	while (!_vm->shouldQuit()) {
		_vm->_mode = MODE_1;

		// Build up a list of available characters in the Roster that are on the
		// same side of Xeen as the player is currently on
		_charList.clear();
		for (int i = 0; i < XEEN_TOTAL_CHARACTERS; ++i) {
			Character &player = party._roster[i];
			if (player._name.empty() || player._xeenSide != (map._loadDarkSide ? 1 : 0))
				continue;

			_charList.push_back(i);
		}

		Window &w = screen._windows[11];
		w.open();
		setupFaces(startingChar, false);
		w.writeString(Common::String::format(PARTY_DIALOG_TEXT, _partyDetails.c_str()));
		w.drawList(&_faceDrawStructs[0], 4);

		_uiSprites.draw(w, 0, Common::Point(16, 100));
		_uiSprites.draw(w, 2, Common::Point(52, 100));
		_uiSprites.draw(w, 4, Common::Point(87, 100));
		_uiSprites.draw(w, 6, Common::Point(122, 100));
		_uiSprites.draw(w, 8, Common::Point(157, 100));
		_uiSprites.draw(w, 10, Common::Point(192, 100));
		screen.loadPalette("mm4.pal");

		if (modeFlag) {
			screen._windows[0].update();
			events.setCursor(0);
			screen.fadeIn(4);
		} else {
			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeOut(4);
				screen._windows[0].update();
			}

			doScroll(_vm, false, false);
			events.setCursor(0);

			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeIn(4);
			}
		}

		bool breakFlag = false;
		while (!_vm->shouldQuit() && !breakFlag) {
			do {
				events.pollEventsAndWait();
				checkEvents(_vm);
			} while (!_vm->shouldQuit() && !_buttonValue);

			switch (_buttonValue) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_e:
			case Common::KEYCODE_x:
				if (party._activeParty.size() == 0) {
					ErrorScroll::show(_vm, NO_ONE_TO_ADVENTURE_WITH);
				} else {
					if (_vm->_mode != MODE_0) {
						for (int idx = 4; idx >= 0; --idx) {
							events.updateGameCounter();
							screen.frameWindow(idx);
							w.update();

							while (events.timeElapsed() < 1)
								events.pollEventsAndWait();
						}
					}

					w.close();
					party._mazeId = party._priorMazeId;

					party.copyPartyToRoster();
					_vm->_saves->writeCharFile();
					return;
				}
				break;

			case Common::KEYCODE_F1:
			case Common::KEYCODE_F2:
			case Common::KEYCODE_F3:
			case Common::KEYCODE_F4:
			case Common::KEYCODE_F5:
			case Common::KEYCODE_F6:
				// Show character info
				_buttonValue -= Common::KEYCODE_F1;
				if (_buttonValue < (int)party._activeParty.size())
					CharacterInfo::show(_vm, _buttonValue);
				break;

			case Common::KEYCODE_1:
			case Common::KEYCODE_2:
			case Common::KEYCODE_3:
			case Common::KEYCODE_4:
				_buttonValue -= Common::KEYCODE_1 - 7;
				if ((_buttonValue - 7 + startingChar) < (int)_charList.size()) {
					// Check if the selected character is already in the party
					uint idx = 0;
					for (; idx < party._activeParty.size(); ++idx) {
						if (_charList[_buttonValue - 7 + startingChar] ==
							party._activeParty[idx]._rosterId)
							break;
					}

					// Only add the character if they're not already in the party
					if (idx == party._activeParty.size()) {
						if (party._activeParty.size() == MAX_ACTIVE_PARTY) {
							sound.playFX(21);
							ErrorScroll::show(_vm, YOUR_PARTY_IS_FULL);
						} else {
							// Add the character to the active party
							party._activeParty.push_back(party._roster[
								_charList[_buttonValue - 7 + startingChar]]);
								startingCharChanged(startingChar);
						}
					}
				}
				break;

			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP8:
				// Up arrow
				if (startingChar > 0) {
					startingChar -= 4;
					startingCharChanged(startingChar);
				}
				break;

			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP2:
				// Down arrow
				if (startingChar < (_charList.size() - 4)) {
					startingChar += 4;
					startingCharChanged(startingChar);
				}
				break;

			case Common::KEYCODE_c:
				// Create
				if (_charList.size() == XEEN_TOTAL_CHARACTERS) {
					ErrorScroll::show(_vm, YOUR_ROSTER_IS_FULL);
				} else {
					screen.fadeOut(4);
					w.close();

					createChar();

					party.copyPartyToRoster();
					_vm->_saves->writeCharFile();
					screen.fadeOut(4);
					modeFlag = true;
					breakFlag = true;
				}
				break;
			case Common::KEYCODE_d:
				// Delete character
				break;
			case Common::KEYCODE_r:
				// Remove character
				if (party._activeParty.size() > 0) {
					int charButtonValue = selectCharacter(false, startingChar);
					if (charButtonValue != 0) {
						party.copyPartyToRoster();
						party._activeParty.remove_at(charButtonValue - Common::KEYCODE_F1);
					}
					startingCharChanged(startingChar);
				}
				break;
		
			default:
				break;
			}
		}
	}
}

void PartyDialog::loadButtons() {
	_uiSprites.load("inn.icn");
	addButton(Common::Rect(16, 100, 40, 120), Common::KEYCODE_UP, &_uiSprites);
	addButton(Common::Rect(52, 100, 76, 120), Common::KEYCODE_DOWN, &_uiSprites);
	addButton(Common::Rect(87, 100, 111, 120), Common::KEYCODE_d, &_uiSprites);
	addButton(Common::Rect(122, 100, 146, 120), Common::KEYCODE_r, &_uiSprites);
	addButton(Common::Rect(157, 100, 181, 120), Common::KEYCODE_c, &_uiSprites);
	addButton(Common::Rect(192, 100, 216, 120), Common::KEYCODE_x, &_uiSprites);
	addButton(Common::Rect(0, 0, 0, 0), Common::KEYCODE_ESCAPE, &_uiSprites, false);
}

void PartyDialog::initDrawStructs() {
	_faceDrawStructs[0] = DrawStruct(0, 0, 0);
	_faceDrawStructs[1] = DrawStruct(0, 101, 0);
	_faceDrawStructs[2] = DrawStruct(0, 0, 43);
	_faceDrawStructs[3] = DrawStruct(0, 101, 43);
}

void PartyDialog::setupBackground() {
	_vm->_screen->loadBackground("back.raw");
	_vm->_interface->assembleBorder();
}

/**
 * Sets up the faces from the avaialble roster for display in the party dialog
 */
void PartyDialog::setupFaces(int firstDisplayChar, bool updateFlag) {
	Party &party = *_vm->_party;
	Common::String charNames[4];
	Common::String charRaces[4];
	Common::String charSex[4];
	Common::String charClasses[4];
	int posIndex;
	int charId;

	// Reset the button areas for the display character images
	while (_buttons.size() > 7)
		_buttons.remove_at(7);
	addButton(Common::Rect(16, 16, 48, 48), Common::KEYCODE_1, &_uiSprites, false);
	addButton(Common::Rect(117, 16, 149, 48), Common::KEYCODE_2, &_uiSprites, false);
	addButton(Common::Rect(59, 59, 91, 91), Common::KEYCODE_3, &_uiSprites, false);
	addButton(Common::Rect(117, 59, 151, 91), Common::KEYCODE_4, &_uiSprites, false);


	for (posIndex = 0; posIndex < 4; ++posIndex) {
		charId = (firstDisplayChar + posIndex) >= (int)_charList.size() ? -1 :
			_charList[firstDisplayChar + posIndex];
		bool isInParty = party.isInParty(charId);

		if (charId == -1) {
			while ((int)_buttons.size() >(7 + posIndex))
				_buttons.remove_at(_buttons.size() - 1);
			break;
		}

		Common::Rect &b = _buttons[7 + posIndex]._bounds;
		b.moveTo((posIndex & 1) ? 117 : 16, b.top);
		Character &ps = party._roster[_charList[firstDisplayChar + posIndex]];
		charNames[posIndex] = isInParty ? IN_PARTY : ps._name;
		charRaces[posIndex] = RACE_NAMES[ps._race];
		charSex[posIndex] = SEX_NAMES[ps._sex];
		charClasses[posIndex] = CLASS_NAMES[ps._class];
	}

	drawParty(updateFlag);

	// Set up the sprite set to use for each face
	for (int posIndex = 0; posIndex < 4; ++posIndex) {
		if ((firstDisplayChar + posIndex) >= (int)_charList.size())
			_faceDrawStructs[posIndex]._sprites = nullptr;
		else
			_faceDrawStructs[posIndex]._sprites = party._roster[
				_charList[firstDisplayChar + posIndex]]._faceSprites;
	}

	_partyDetails = Common::String::format(PARTY_DETAILS,
		charNames[0].c_str(), charRaces[0].c_str(), charSex[0].c_str(), charClasses[0].c_str(),
		charNames[1].c_str(), charRaces[1].c_str(), charSex[1].c_str(), charClasses[1].c_str(),
		charNames[2].c_str(), charRaces[2].c_str(), charSex[2].c_str(), charClasses[2].c_str(),
		charNames[3].c_str(), charRaces[3].c_str(), charSex[3].c_str(), charClasses[3].c_str()
		);
}

void PartyDialog::startingCharChanged(int firstDisplayChar) {
	Window &w = _vm->_screen->_windows[11];

	setupFaces(firstDisplayChar, true);
	w.writeString(Common::String::format(PARTY_DIALOG_TEXT, _partyDetails.c_str()));
	w.drawList(_faceDrawStructs, 4);

	_uiSprites.draw(w, 0, Common::Point(16, 100));
	_uiSprites.draw(w, 2, Common::Point(52, 100));
	_uiSprites.draw(w, 4, Common::Point(87, 100));
	_uiSprites.draw(w, 6, Common::Point(122, 100));
	_uiSprites.draw(w, 8, Common::Point(157, 100));
	_uiSprites.draw(w, 10, Common::Point(192, 100));

	w.update();
}

void PartyDialog::createChar() {
	error("TODO: createChar");
}

int PartyDialog::selectCharacter(bool isDelete, int firstDisplayChar) {
	EventsManager &events = *_vm->_events;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Window &w = screen._windows[28];

	SpriteResource iconSprites;
	iconSprites.load("esc.icn");

	w.setBounds(Common::Rect(50, isDelete ? 112 : 76, 266, isDelete ? 148 : 112));
	w.open();
	w.writeString(Common::String::format(REMOVE_OR_DELETE_WHICH,
		REMOVE_DELETE[isDelete ? 1 : 0]));
	iconSprites.draw(w, 0, Common::Point(225, isDelete ? 120 : 84));
	w.update();

	saveButtons();
	addButton(Common::Rect(225, isDelete ? 120 : 84, 249, isDelete ? 140 : 104), 
		Common::KEYCODE_ESCAPE, &iconSprites);
	addButton(Common::Rect(16, 16, 48, 48), Common::KEYCODE_1, &iconSprites, false);
	addButton(Common::Rect(117, 16, 149, 48), Common::KEYCODE_2, &iconSprites, false);
	addButton(Common::Rect(16, 59, 48, 91), Common::KEYCODE_3, &iconSprites, false);
	addButton(Common::Rect(117, 59, 149, 91), Common::KEYCODE_4, &iconSprites, false);
	addPartyButtons(_vm);

	int result = -1, v;
	while (!_vm->shouldQuit() && result == -1) {
		_buttonValue = 0;
		while (!_vm->shouldQuit() && !_buttonValue) {
			events.pollEventsAndWait();
			checkEvents(_vm);
		}

		switch (_buttonValue) {
		case Common::KEYCODE_ESCAPE:
			result = 0;
			break;

		case Common::KEYCODE_F1:
		case Common::KEYCODE_F2:
		case Common::KEYCODE_F3:
		case Common::KEYCODE_F4:
		case Common::KEYCODE_F5:
		case Common::KEYCODE_F6:
			if (!isDelete) {
				v = _buttonValue - Common::KEYCODE_F1;
				if (v < (int)party._activeParty.size())
					result = _buttonValue;
			}
			break;

		case Common::KEYCODE_1:
		case Common::KEYCODE_2:
		case Common::KEYCODE_3:
		case Common::KEYCODE_4:
			if (isDelete) {
				v = _buttonValue - Common::KEYCODE_1;
				if ((firstDisplayChar + v) < (int)_charList.size())
					result = _buttonValue;
			}
			break;

		default:
			break;
		}
	} 

	w.close();
	restoreButtons();
	return result == -1 ? 0 : result;
}

} // End of namespace Xeen
