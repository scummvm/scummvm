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
#include "xeen/dialogs_input.h"
#include "xeen/dialogs_query.h"
#include "xeen/character.h"
#include "xeen/events.h"
#include "xeen/party.h"
#include "xeen/xeen.h"

namespace Xeen {

PartyDialog::PartyDialog(XeenEngine *vm) : ButtonContainer(vm),
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
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
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

		Window &w = windows[11];
		w.open();
		setupFaces(startingChar, false);
		w.writeString(Common::String::format(Res.PARTY_DIALOG_TEXT, _partyDetails.c_str()));
		w.drawList(&_faceDrawStructs[0], 4);

		_uiSprites.draw(w, 0, Common::Point(16, 100));
		_uiSprites.draw(w, 2, Common::Point(52, 100));
		_uiSprites.draw(w, 4, Common::Point(87, 100));
		_uiSprites.draw(w, 6, Common::Point(122, 100));
		_uiSprites.draw(w, 8, Common::Point(157, 100));
		_uiSprites.draw(w, 10, Common::Point(192, 100));
		screen.loadPalette("mm4.pal");

		if (modeFlag) {
			windows[0].update();
			events.setCursor(0);
			screen.fadeIn();
		} else {
			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeOut();
				windows[0].update();
			}

			doScroll(false, false);
			events.setCursor(0);

			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeIn();
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
					ErrorScroll::show(_vm, Res.NO_ONE_TO_ADVENTURE_WITH);
				} else {
					if (_vm->_mode != MODE_0) {
						for (int idx = OBSCURITY_NONE; idx >= OBSCURITY_BLACK; --idx) {
							events.updateGameCounter();
							intf.obscureScene((Obscurity)idx);
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
							ErrorScroll::show(_vm, Res.YOUR_PARTY_IS_FULL);
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
				if (startingChar < ((int)_charList.size() - 4)) {
					startingChar += 4;
					startingCharChanged(startingChar);
				}
				break;

			case Common::KEYCODE_c:
				// Create
				if (_charList.size() == XEEN_TOTAL_CHARACTERS) {
					ErrorScroll::show(_vm, Res.YOUR_ROSTER_IS_FULL);
				} else {
					screen.fadeOut();
					w.close();

					createChar();

					party.copyPartyToRoster();
					_vm->_saves->writeCharFile();
					screen.fadeOut();
					modeFlag = true;
					breakFlag = true;
				}
				break;

			case Common::KEYCODE_d:
				// Delete character
				if (_charList.size() > 0) {
					int charButtonValue = selectCharacter(true, startingChar);
					if (charButtonValue != 0) {
						int charIndex = charButtonValue - Common::KEYCODE_1 + startingChar;
						Character &c = party._roster[_charList[charIndex]];
						if (c.hasSlayerSword()) {
							ErrorScroll::show(_vm, Res.HAS_SLAYER_SWORD);
						} else {
							Common::String msg = Common::String::format(Res.SURE_TO_DELETE_CHAR,
								c._name.c_str(), Res.CLASS_NAMES[c._class]);
							if (Confirm::show(_vm, msg)) {
								// If the character is in the party, remove it
								for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
									if (party._activeParty[idx]._rosterId == c._rosterId) {
										party._activeParty.remove_at(idx);
										break;
									}
								}

								// Empty the character in the roster
								c.clear();

								// Rebuild the character list
								_charList.clear();
								for (int idx = 0; idx < XEEN_TOTAL_CHARACTERS; ++idx) {
									Character &ch = party._roster[idx];
									if (!ch._name.empty() && ch._savedMazeId == party._priorMazeId) {
										_charList.push_back(idx);
									}
								}

								startingCharChanged(startingChar);
							}
						}
					}
				}
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
	addButton(Common::Rect(0, 0, 0, 0), Common::KEYCODE_ESCAPE);
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
	addButton(Common::Rect(16, 16, 48, 48), Common::KEYCODE_1);
	addButton(Common::Rect(117, 16, 149, 48), Common::KEYCODE_2);
	addButton(Common::Rect(59, 59, 91, 91), Common::KEYCODE_3);
	addButton(Common::Rect(117, 59, 151, 91), Common::KEYCODE_4);


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
		charNames[posIndex] = isInParty ? Res.IN_PARTY : ps._name;
		charRaces[posIndex] = Res.RACE_NAMES[ps._race];
		charSex[posIndex] = Res.SEX_NAMES[ps._sex];
		charClasses[posIndex] = Res.CLASS_NAMES[ps._class];
	}

	drawParty(updateFlag);

	// Set up the sprite set to use for each face
	for (posIndex = 0; posIndex < 4; ++posIndex) {
		if ((firstDisplayChar + posIndex) >= (int)_charList.size())
			_faceDrawStructs[posIndex]._sprites = nullptr;
		else
			_faceDrawStructs[posIndex]._sprites = party._roster[
				_charList[firstDisplayChar + posIndex]]._faceSprites;
	}

	_partyDetails = Common::String::format(Res.PARTY_DETAILS,
		charNames[0].c_str(), charRaces[0].c_str(), charSex[0].c_str(), charClasses[0].c_str(),
		charNames[1].c_str(), charRaces[1].c_str(), charSex[1].c_str(), charClasses[1].c_str(),
		charNames[2].c_str(), charRaces[2].c_str(), charSex[2].c_str(), charClasses[2].c_str(),
		charNames[3].c_str(), charRaces[3].c_str(), charSex[3].c_str(), charClasses[3].c_str()
		);
}

void PartyDialog::startingCharChanged(int firstDisplayChar) {
	Windows &windows = *_vm->_windows;
	Window &w = windows[11];

	setupFaces(firstDisplayChar, true);
	w.writeString(Common::String::format(Res.PARTY_DIALOG_TEXT, _partyDetails.c_str()));
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
	EventsManager &events = *_vm->_events;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Windows &windows = *_vm->_windows;
	Window &w = windows[0];
	SpriteResource dice, icons;
	Common::Array<int> freeCharList;
	int classId;
	int selectedClass = 0;
	bool hasFadedIn = false;
	bool restartFlag = true;
	uint attribs[TOTAL_ATTRIBUTES];
	bool allowedClasses[TOTAL_CLASSES];
	Race race = HUMAN;
	Sex sex = MALE;
	Common::String msg;
	int charIndex = 0;

	Mode oldMode = _vm->_mode;
	_vm->_mode = MODE_4;
	dice.load("dice.vga");
	icons.load("create.raw");

	_dicePos[0] = Common::Point(20, 17);
	_dicePos[1] = Common::Point(112, 35);
	_dicePos[2] = Common::Point(61, 50);
	_diceFrame[0] = 0;
	_diceFrame[1] = 2;
	_diceFrame[2] = 4;
	_diceInc[0] = Common::Point(10, -10);
	_diceInc[1] = Common::Point(-10, -10);
	_diceInc[2] = Common::Point(-10, 10);

	// Add buttons
	saveButtons();
	addButton(Common::Rect(132, 98, 156, 118), Common::KEYCODE_r, &icons);
	addButton(Common::Rect(132, 128, 156, 148), Common::KEYCODE_c, &icons);
	addButton(Common::Rect(132, 158, 156, 178), Common::KEYCODE_ESCAPE, &icons);
	addButton(Common::Rect(86, 98, 110, 118), Common::KEYCODE_UP, &icons);
	addButton(Common::Rect(86, 120, 110, 140), Common::KEYCODE_DOWN, &icons);
	addButton(Common::Rect(168, 19, 192, 39), Common::KEYCODE_n, nullptr);
	addButton(Common::Rect(168, 43, 192, 63), Common::KEYCODE_i, nullptr);
	addButton(Common::Rect(168, 67, 192, 87), Common::KEYCODE_p, nullptr);
	addButton(Common::Rect(168, 91, 192, 111), Common::KEYCODE_e, nullptr);
	addButton(Common::Rect(168, 115, 192, 135), Common::KEYCODE_s, nullptr);
	addButton(Common::Rect(168, 139, 192, 159), Common::KEYCODE_a, nullptr);
	addButton(Common::Rect(168, 163, 192, 183), Common::KEYCODE_l, nullptr);
	addButton(Common::Rect(227, 19, 139, 29), 1000, nullptr);
	addButton(Common::Rect(227, 30, 139, 40), 1001, nullptr);
	addButton(Common::Rect(227, 41, 139, 51), 1002, nullptr);
	addButton(Common::Rect(227, 52, 139, 62), 1003, nullptr);
	addButton(Common::Rect(227, 63, 139, 73), 1004, nullptr);
	addButton(Common::Rect(227, 74, 139, 84), 1005, nullptr);
	addButton(Common::Rect(227, 85, 139, 95), 1006, nullptr);
	addButton(Common::Rect(227, 96, 139, 106), 1007, nullptr);
	addButton(Common::Rect(227, 107, 139, 117), 1008, nullptr);
	addButton(Common::Rect(227, 118, 139, 128), 1009, nullptr);

	// Load the background
	screen.loadBackground("create.raw");
	events.setCursor(0);

	while (!_vm->shouldQuit()) {
		classId = -1;

		if (restartFlag) {
			// Build up list of roster slot indexes that are free
			freeCharList.clear();
			for (uint idx = 0; idx < XEEN_TOTAL_CHARACTERS; ++idx) {
				if (party._roster[idx]._name.empty())
					freeCharList.push_back(idx);
			}
			charIndex = 0;
			//bool flag9 = true;

			if (freeCharList.size() == XEEN_TOTAL_CHARACTERS)
				break;

			// Get and race and sex for the given character
			race = (Race)((freeCharList[charIndex] / 4) % 5);
			sex = (Sex)(freeCharList[charIndex] & 1);

			// Randomly determine attributes, and which classes they allow
			throwDice(attribs, allowedClasses);

			// Set up display of the rolled character details
			selectedClass = newCharDetails(attribs, allowedClasses,
				race, sex, classId, selectedClass, msg);

			// Draw the screen
			icons.draw(w, 10, Common::Point(168, 19));
			icons.draw(w, 12, Common::Point(168, 43));
			icons.draw(w, 14, Common::Point(168, 67));
			icons.draw(w, 16, Common::Point(168, 91));
			icons.draw(w, 18, Common::Point(168, 115));
			icons.draw(w, 20, Common::Point(168, 139));
			icons.draw(w, 22, Common::Point(168, 163));
			for (int idx = 0; idx < 9; ++idx)
				icons.draw(w, 24 + idx * 2, Common::Point(227, 19 + 11 * idx));

			for (int idx = 0; idx < 7; ++idx)
				icons.draw(w, 50 + idx, Common::Point(195, 31 + 24 * idx));

			icons.draw(w, 57, Common::Point(62, 148));
			icons.draw(w, 58, Common::Point(62, 158));
			icons.draw(w, 59, Common::Point(62, 168));
			icons.draw(w, 61, Common::Point(220, 19));
			icons.draw(w, 64, Common::Point(220, 155));
			icons.draw(w, 65, Common::Point(220, 170));

			party._roster[freeCharList[charIndex]]._faceSprites->draw(
				w, 0, Common::Point(27, 102));

			icons.draw(w, 0, Common::Point(132, 98));
			icons.draw(w, 2, Common::Point(132, 128));
			icons.draw(w, 4, Common::Point(132, 158));
			icons.draw(w, 6, Common::Point(86, 98));
			icons.draw(w, 8, Common::Point(86, 120));

			w.writeString(msg);
			w.update();

			// Draw the arrow for the selected class, if applicable
			if (selectedClass != -1)
				printSelectionArrow(icons, selectedClass);

			// Draw the dice
			drawDice(dice);
			if (!hasFadedIn) {
				screen.fadeIn();
				hasFadedIn = true;
			}

			restartFlag = false;
		}

		// Animate the dice until a user action occurs
		_buttonValue = 0;
		while (!_vm->shouldQuit() && !_buttonValue)
			drawDice(dice);

		// Handling for different actions
		switch (_buttonValue) {
		case Common::KEYCODE_UP:
			if (charIndex == 0)
				continue;

			race = (Race)((freeCharList[charIndex] / 4) % 5);
			sex = (Sex)(freeCharList[charIndex] & 1);
			break;

		case Common::KEYCODE_DOWN:
			if (++charIndex == (int)freeCharList.size()) {
				--charIndex;
				continue;
			} else {
				race = (Race)((freeCharList[charIndex] / 4) % 5);
				sex = (Sex)(freeCharList[charIndex] & 1);
			}
			break;

		case Common::KEYCODE_PAGEUP:
			for (int tempClass = selectedClass - 1; tempClass >= 0; --tempClass) {
				if (allowedClasses[tempClass]) {
					selectedClass = tempClass;
					break;
				}
			}

			printSelectionArrow(icons, selectedClass);
			continue;

		case Common::KEYCODE_PAGEDOWN:
			break;

		case Common::KEYCODE_m:
		case Common::KEYCODE_i:
		case Common::KEYCODE_p:
		case Common::KEYCODE_e:
		case Common::KEYCODE_s:
		case Common::KEYCODE_a:
		case Common::KEYCODE_l: {
			Attribute srcAttrib, destAttrib;
			if (_buttonValue == Common::KEYCODE_m)
				srcAttrib = MIGHT;
			else if (_buttonValue == Common::KEYCODE_i)
				srcAttrib = INTELLECT;
			else if (_buttonValue == Common::KEYCODE_p)
				srcAttrib = PERSONALITY;
			else if (_buttonValue == Common::KEYCODE_e)
				srcAttrib = ENDURANCE;
			else if (_buttonValue == Common::KEYCODE_s)
				srcAttrib = SPEED;
			else if (_buttonValue == Common::KEYCODE_a)
				srcAttrib = ACCURACY;
			else
				srcAttrib = LUCK;

			_vm->_mode = MODE_86;
			icons.draw(w, srcAttrib * 2 + 11, Common::Point(
				_buttons[srcAttrib + 5]._bounds.left, _buttons[srcAttrib + 5]._bounds.top));
			w.update();

			int destAttribVal = exchangeAttribute(srcAttrib + 1);
			if (destAttribVal) {
				destAttrib = (Attribute)(destAttribVal - 1);
				icons.draw(w, destAttrib * 2 + 11, Common::Point(
					_buttons[destAttrib + 10]._bounds.left,
					_buttons[destAttrib + 10]._bounds.top));
				w.update();

				SWAP(attribs[srcAttrib], attribs[destAttrib]);
				checkClass(attribs, allowedClasses);
				classId = -1;
				selectedClass = newCharDetails(attribs, allowedClasses,
					race, sex, classId, selectedClass, msg);
			} else {
				icons.draw(w, srcAttrib * 2 + 10, Common::Point(
					_buttons[srcAttrib + 5]._bounds.left,
					_buttons[srcAttrib + 5]._bounds.top));
				w.update();
				_vm->_mode = MODE_SLEEPING;
				continue;
			}
			break;
		}

		case 1000:
		case 1001:
		case 1002:
		case 1003:
		case 1004:
		case 1005:
		case 1006:
		case 1007:
		case 1008:
		case 1009:
			if (allowedClasses[_buttonValue - 1000]) {
				selectedClass = classId = _buttonValue - 1000;
			}
			break;

		case Common::KEYCODE_c: {
			_vm->_mode = MODE_FF;
			bool result = saveCharacter(party._roster[freeCharList[charIndex]],
				classId, race, sex, attribs);
			_vm->_mode = MODE_4;

			if (result)
				restartFlag = true;
			continue;
		}

		case Common::KEYCODE_RETURN:
			classId = selectedClass;
			break;

		case Common::KEYCODE_SPACE:
		case Common::KEYCODE_r:
			// Re-roll the attributes
			throwDice(attribs, allowedClasses);
			classId = -1;
			break;

		default:
			// For all other keypresses, skip the code below the switch
			// statement, and go to wait for the next key
			continue;
		}

		if (_buttonValue != Common::KEYCODE_PAGEDOWN) {
			selectedClass = newCharDetails(attribs, allowedClasses,
				race, sex, classId, selectedClass, msg);

			for (int idx = 0; idx < 7; ++idx)
				icons.draw(w, 10 + idx * 2, Common::Point(168, 19 + idx * 24));
			for (int idx = 0; idx < 10; ++idx)
				icons.draw(w, 24 + idx * 2, Common::Point(227, 19 + idx * 11));
			for (int idx = 0; idx < 8; ++idx)
				icons.draw(w, 50 + idx, Common::Point(195, 31 + idx * 24));

			icons.draw(w, 57, Common::Point(62, 148));
			icons.draw(w, 58, Common::Point(62, 158));
			icons.draw(w, 59, Common::Point(62, 168));
			icons.draw(w, 61, Common::Point(220, 19));
			icons.draw(w, 64, Common::Point(220, 155));
			icons.draw(w, 65, Common::Point(220, 170));

			party._roster[freeCharList[charIndex]]._faceSprites->draw(w, 0,
				Common::Point(27, 102));

			icons.draw(w, 0, Common::Point(132, 98));
			icons.draw(w, 2, Common::Point(132, 128));
			icons.draw(w, 4, Common::Point(132, 158));
			icons.draw(w, 6, Common::Point(86, 98));
			icons.draw(w, 8, Common::Point(86, 120));

			w.writeString(msg);
			w.update();

			if (selectedClass != -1) {
				printSelectionArrow(icons, selectedClass);
				continue;
			}
		}

		// Move to next available class, or if the code block above resulted in
		// selectedClass being -1, move to select the first available class
		for (int tempClass = selectedClass + 1; tempClass <= CLASS_RANGER; ++tempClass) {
			if (allowedClasses[tempClass]) {
				selectedClass = tempClass;
				break;
			}
		}

		printSelectionArrow(icons, selectedClass);
	} while (!_vm->shouldQuit() && _buttonValue != Common::KEYCODE_ESCAPE);

	_vm->_mode = oldMode;
}

int PartyDialog::selectCharacter(bool isDelete, int firstDisplayChar) {
	EventsManager &events = *_vm->_events;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	Window &w = windows[28];

	SpriteResource iconSprites;
	iconSprites.load("esc.icn");

	w.setBounds(Common::Rect(50, isDelete ? 112 : 76, 266, isDelete ? 148 : 112));
	w.open();
	w.writeString(Common::String::format(Res.REMOVE_OR_DELETE_WHICH,
		Res.REMOVE_DELETE[isDelete ? 1 : 0]));
	iconSprites.draw(w, 0, Common::Point(225, isDelete ? 120 : 84));
	w.update();

	saveButtons();
	addButton(Common::Rect(225, isDelete ? 120 : 84, 249, isDelete ? 140 : 104),
		Common::KEYCODE_ESCAPE, &iconSprites);
	addButton(Common::Rect(16, 16, 48, 48), Common::KEYCODE_1);
	addButton(Common::Rect(117, 16, 149, 48), Common::KEYCODE_2);
	addButton(Common::Rect(16, 59, 48, 91), Common::KEYCODE_3);
	addButton(Common::Rect(117, 59, 149, 91), Common::KEYCODE_4);
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

void PartyDialog::throwDice(uint attribs[TOTAL_ATTRIBUTES], bool allowedClasses[TOTAL_CLASSES]) {
	bool repeat = true;
	do {
		// Default all the attributes to zero
		Common::fill(&attribs[0], &attribs[TOTAL_ATTRIBUTES], 0);

		// Assign random amounts to each attribute
		for (int idx1 = 0; idx1 < 3; ++idx1) {
			for (int idx2 = 0; idx2 < TOTAL_ATTRIBUTES; ++idx2) {
				attribs[idx1] += _vm->getRandomNumber(10, 79) / 10;
			}
		}

		// Check which classes are allowed based on the rolled attributes
		checkClass(attribs, allowedClasses);

		// Only exit if the attributes allow for at least one class
		for (int idx = 0; idx < TOTAL_CLASSES; ++idx) {
			if (allowedClasses[idx])
				repeat = false;
		}
	} while (repeat);
}

void PartyDialog::checkClass(const uint attribs[TOTAL_ATTRIBUTES], bool allowedClasses[TOTAL_CLASSES]) {
	allowedClasses[CLASS_KNIGHT] = attribs[MIGHT] >= 15;
	allowedClasses[CLASS_PALADIN] = attribs[MIGHT] >= 13
		&& attribs[PERSONALITY] >= 13 && attribs[ENDURANCE] >= 13;
	allowedClasses[CLASS_ARCHER] = attribs[INTELLECT] >= 13 && attribs[ACCURACY] >= 13;
	allowedClasses[CLASS_CLERIC] = attribs[PERSONALITY] >= 13;
	allowedClasses[CLASS_SORCERER] = attribs[INTELLECT] >= 13;
	allowedClasses[CLASS_ROBBER] = attribs[LUCK] >= 13;
	allowedClasses[CLASS_NINJA] = attribs[SPEED] >= 13 && attribs[ACCURACY] >= 13;
	allowedClasses[CLASS_BARBARIAN] = attribs[ENDURANCE] >= 15;
	allowedClasses[CLASS_DRUID] = attribs[INTELLECT] >= 15 && attribs[PERSONALITY] >= 15;
	allowedClasses[CLASS_RANGER] = attribs[INTELLECT] >= 12 && attribs[PERSONALITY] >= 12
		&& attribs[ENDURANCE] >= 12 && attribs[SPEED] >= 12;
}

int PartyDialog::newCharDetails(const uint attribs[TOTAL_ATTRIBUTES],
		bool allowedClasses[TOTAL_CLASSES], Race race, Sex sex, int classId,
		int selectedClass, Common::String &msg) {
	int foundClass = -1;
	Common::String skillStr, classStr, raceSkillStr;

	// If a selected class is provided, set the default skill for that class
	if (classId != -1 && Res.NEW_CHAR_SKILLS[classId] != -1) {
		const char *skillP = Res.SKILL_NAMES[Res.NEW_CHAR_SKILLS[classId]];
		skillStr = Common::String(skillP, skillP + Res.NEW_CHAR_SKILLS_LEN[classId]);
	}

	// If a class is provided, set the class name
	if (classId != -1) {
		classStr = Common::String::format("\t062\v168%s", Res.CLASS_NAMES[classId]);
	}

	// Set up default skill for the race, if any
	if (Res.NEW_CHAR_RACE_SKILLS[race] != -1) {
		raceSkillStr = Res.SKILL_NAMES[Res.NEW_CHAR_RACE_SKILLS[race]];
	}

	// Set up color to use for each skill string to be displayed, based
	// on whether each class is allowed or not for the given attributes
	int classColors[TOTAL_CLASSES];
	Common::fill(&classColors[0], &classColors[TOTAL_CLASSES], 0);
	for (int classNum = CLASS_KNIGHT; classNum <= CLASS_RANGER; ++classNum) {
		if (allowedClasses[classNum]) {
			if (classId == -1 && (foundClass == -1 || foundClass < classNum))
				foundClass = classNum;
			classColors[classNum] = 4;
		}
	}

	// Return stats details and character class
	msg = Common::String::format(Res.NEW_CHAR_STATS, Res.RACE_NAMES[race], Res.SEX_NAMES[sex],
		attribs[MIGHT], attribs[INTELLECT], attribs[PERSONALITY],
		attribs[ENDURANCE], attribs[SPEED], attribs[ACCURACY], attribs[LUCK],
		classColors[CLASS_KNIGHT], classColors[CLASS_PALADIN],
		classColors[CLASS_ARCHER], classColors[CLASS_CLERIC],
		classColors[CLASS_SORCERER], classColors[CLASS_ROBBER],
		classColors[CLASS_NINJA], classColors[CLASS_BARBARIAN],
		classColors[CLASS_DRUID], classColors[CLASS_RANGER],
		skillStr.c_str(), raceSkillStr.c_str(), classStr.c_str()
	);
	return classId == -1 ? foundClass : selectedClass;
}

void PartyDialog::printSelectionArrow(SpriteResource &icons, int selectedClass) {
	Windows &windows = *_vm->_windows;
	Window &w = windows[0];
	icons.draw(w, 61, Common::Point(220, 19));
	icons.draw(w, 63, Common::Point(220, selectedClass * 11 + 21));
	w.update();
}

void PartyDialog::drawDice(SpriteResource &dice) {
	EventsManager &events = *_vm->_events;
	Windows &windows = *_vm->_windows;
	Window &w = windows[32];
	dice.draw(w, 7, Common::Point(12, 11));

	for (int diceNum = 0; diceNum < 3; ++diceNum) {
		_diceFrame[diceNum] = (_diceFrame[diceNum] + 1) % 7;
		_dicePos[diceNum] += _diceInc[diceNum];

		if (_dicePos[diceNum].x < 13) {
			_dicePos[diceNum].x = 13;
			_diceInc[diceNum].x *= -1;
		} else if (_dicePos[diceNum].x >= 163) {
			_dicePos[diceNum].x = 163;
			_diceInc[diceNum].x *= -1;
		}

		if (_dicePos[diceNum].y < 12) {
			_dicePos[diceNum].y = 12;
			_diceInc[diceNum].y *= -1;
		} else if (_dicePos[diceNum].y >= 93) {
			_dicePos[diceNum].y = 93;
			_diceInc[diceNum].y *= -1;
		}

		dice.draw(w, _diceFrame[diceNum], _dicePos[diceNum]);
	}

	w.update();

	// Wait for keypress
	events.wait(1);
	checkEvents(_vm);
}

int PartyDialog::exchangeAttribute(int srcAttr) {
	EventsManager &events = *_vm->_events;
	Windows &windows = *_vm->_windows;
	SpriteResource icons;
	icons.load("create2.icn");

	saveButtons();
	addButton(Common::Rect(118, 58, 142, 78), Common::KEYCODE_ESCAPE, &icons);
	addButton(Common::Rect(168, 19, 192, 39), Common::KEYCODE_m);
	addButton(Common::Rect(168, 43, 192, 63), Common::KEYCODE_i);
	addButton(Common::Rect(168, 67, 192, 87), Common::KEYCODE_p);
	addButton(Common::Rect(168, 91, 192, 111), Common::KEYCODE_e);
	addButton(Common::Rect(168, 115, 192, 135), Common::KEYCODE_s);
	addButton(Common::Rect(168, 139, 192, 159), Common::KEYCODE_a);
	addButton(Common::Rect(168, 163, 192, 183), Common::KEYCODE_l);

	Window &w = windows[26];
	w.open();
	w.writeString(Common::String::format(Res.EXCHANGE_ATTR_WITH, Res.STAT_NAMES[srcAttr - 1]));
	icons.draw(w, 0, Common::Point(118, 58));
	w.update();

	int result = 0;
	bool breakFlag = false;
	while (!_vm->shouldQuit() && !breakFlag) {
		// Wait for an action
		do {
			events.pollEventsAndWait();
			checkEvents(_vm);
		} while (!_vm->shouldQuit() && !_buttonValue);

		Attribute destAttr;
		switch (_buttonValue) {
		case Common::KEYCODE_m:
			destAttr = MIGHT;
			break;
		case Common::KEYCODE_i:
			destAttr = INTELLECT;
			break;
		case Common::KEYCODE_p:
			destAttr = PERSONALITY;
			break;
		case Common::KEYCODE_e:
			destAttr = ENDURANCE;
			break;
		case Common::KEYCODE_s:
			destAttr = SPEED;
			break;
		case Common::KEYCODE_a:
			destAttr = ACCURACY;
			break;
		case Common::KEYCODE_l:
			destAttr = LUCK;
			break;
		case Common::KEYCODE_ESCAPE:
			result = 0;
			breakFlag = true;
			continue;
		default:
			continue;
		}

		if ((srcAttr - 1) != destAttr) {
			result = destAttr + 1;
			break;
		}
	}

	w.close();
	_buttonValue = 0;
	restoreButtons();

	return result;
}

bool PartyDialog::saveCharacter(Character &c, int classId,
		Race race, Sex sex, uint attribs[TOTAL_ATTRIBUTES]) {
	if (classId == -1) {
		ErrorScroll::show(_vm, Res.SELECT_CLASS_BEFORE_SAVING);
		return false;
	}

	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	Window &w = windows[6];
	Common::String name;
	int result;
	bool isDarkCc = _vm->_files->_isDarkCc;

	saveButtons();
	w.writeString(Res.NAME_FOR_NEW_CHARACTER);

	result = Input::show(_vm, &w, name, 10, 200);
	w.close();
	restoreButtons();
	if (!result)
		return false;

	// Save new character details
	c.clear();
	c._name = name;
	c._savedMazeId = party._priorMazeId;
	c._xeenSide = map._loadDarkSide;
	c._sex = sex;
	c._race = race;
	c._class = (CharacterClass)classId;
	c._level._permanent = isDarkCc ? 5 : 1;

	c._might._permanent = attribs[MIGHT];
	c._intellect._permanent = attribs[INTELLECT];
	c._personality._permanent = attribs[PERSONALITY];
	c._endurance._permanent = attribs[ENDURANCE];
	c._speed._permanent = attribs[SPEED];
	c._accuracy._permanent = attribs[ACCURACY];
	c._luck._permanent = attribs[LUCK];

	c._magicResistence._permanent = Res.RACE_MAGIC_RESISTENCES[race];
	c._fireResistence._permanent = Res.RACE_FIRE_RESISTENCES[race];
	c._electricityResistence._permanent = Res.RACE_ELECTRIC_RESISTENCES[race];
	c._coldResistence._permanent = Res.RACE_COLD_RESISTENCES[race];
	c._energyResistence._permanent = Res.RACE_ENERGY_RESISTENCES[race];
	c._poisonResistence._permanent = Res.RACE_POISON_RESISTENCES[race];

	c._birthYear = party._year - 18;
	c._birthDay = party._day;
	c._hasSpells = false;
	c._currentSpell = -1;

	// Set up any default spells for the character's class
	for (int idx = 0; idx < 4; ++idx) {
		if (Res.NEW_CHARACTER_SPELLS[c._class][idx] != -1) {
			c._hasSpells = true;
			c._currentSpell = Res.NEW_CHARACTER_SPELLS[c._class][idx];
			c._spells[c._currentSpell] = 1;
		}
	}

	int classSkill = Res.NEW_CHAR_SKILLS[c._class];
	if (classSkill != -1)
		c._skills[classSkill] = 1;

	int raceSkill = Res.NEW_CHAR_RACE_SKILLS[c._race];
	if (raceSkill != -1)
		c._skills[raceSkill] = 1;

	c._currentHp = c.getMaxHP();
	c._currentSp = c.getMaxSP();
	return true;
}

} // End of namespace Xeen
