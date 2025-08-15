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

#include "efh/efh.h"

#include "backends/keymapper/keymapper.h"

namespace Efh {

int16 EfhEngine::displayBoxWithText(const Common::String &str, int16 menuType, int16 displayOption, bool displayTeamWindowFl, bool voiceText) {
	debugC(3, kDebugEngine, "displayBoxWithText %s %d %d %s", str.c_str(), menuType, displayOption, displayTeamWindowFl ? "True" : "False");

	int16 retVal = 0xFF;
	int16 minX, maxX, minY, maxY;

	switch (menuType) {
	case 0:
		minX = 129;
		minY = 9;
		maxX = 302;
		maxY = 18;
		break;
	case 1:
		minX = 129;
		minY = 9;
		maxX = 302;
		maxY = 110;
		break;
	case 2:
		minX = 129;
		minY = 112;
		maxX = 302;
		maxY = 132;
		break;
	case 3:
		minX = 129;
		minY = 79;
		maxX = 303;
		maxY = 107;
		break;
	default:
		minX = minY = 0;
		maxX = 320;
		maxY = 200;
		break;
	}

	if (voiceText) {
		Common::String ttsMessage(str);

		// Some messages have a ^ followed by a few numbers at the end
		uint32 caretPosition = ttsMessage.find('^');
		if (caretPosition != Common::String::npos) {
			ttsMessage.erase(caretPosition, Common::String::npos);
		}

		ttsMessage.replace('|', '\n');

		sayText(ttsMessage, kNoRestriction);
	}

	drawColoredRect(minX, minY, maxX, maxY, 0);
	if (!str.empty())
		retVal = script_parse(str, minX, minY, maxX, maxY, true);

	if (displayTeamWindowFl)
		displayLowStatusScreen(false);

	if (displayOption != 0) {
		displayFctFullScreen();
		if (_textBoxDisabledByScriptFl)
			_textBoxDisabledByScriptFl = false;
		else {
			drawColoredRect(minX, minY, maxX, maxY, 0);
			if (!str.empty())
				script_parse(str, minX, minY, maxX, maxY, false);
		}

		if (displayTeamWindowFl)
			displayLowStatusScreen(false);

		if (displayOption >= 2)
			getLastCharAfterAnimCount(_guessAnimationAmount);

		if (displayOption == 3)
			drawColoredRect(minX, minY, maxX, maxY, 0);
	}

	return retVal;
}

bool EfhEngine::handleDeathMenu() {
	debugC(3, kDebugEngine, "handleDeathMenu");

	_saveAuthorized = false;

	displayAnimFrames(20, true);
	_imageSetSubFilesIdx = 213;
	drawScreen();

	sayText("Darkness prevails... death has taken you!", kNoRestriction, Common::TextToSpeechManager::INTERRUPT);
	sayText("Load last saved game", kNoRestriction);
	sayText("Restart from beginning", kNoRestriction);
	sayText("Quit for now", kNoRestriction);

	for (uint counter = 0; counter < 2; ++counter) {
		clearBottomTextZone(0);
		displayCenteredString("Darkness Prevails...Death Has Taken You!", 24, 296, 153);
		setTextPos(100, 162);
		setTextColorWhite();
		displayCharAtTextPos('L');
		setTextColorRed();
		displayStringAtTextPos("oad last saved game");
		setTextPos(100, 171);
		setTextColorWhite();
		displayCharAtTextPos('R');
		setTextColorRed();
		displayStringAtTextPos("estart from beginning");
		setTextPos(100, 180);
		setTextColorWhite();
		displayCharAtTextPos('Q');
		setTextColorRed();
		displayStringAtTextPos("uit for now");
		if (counter == 0)
			displayFctFullScreen();
	}

	setKeymap(kKeymapDeathMenu);

	for (bool found = false; !found && !shouldQuit();) {
		Common::CustomEventType input = waitForKey();
		switch (input) {
		case kActionLoad:
			// If the user actually loads a savegame, it'll get _saveAuthorized from the savegame (always true) and will set 'found' to true.
			// If 'found' remains false, it means the user cancelled the loading and still needs to take a decision
			// Original is calling loadEfhGame() because there's only one savegame, so it's not ambiguous
			loadGameDialog();
			found = _saveAuthorized;
			break;
		case kActionQuit:
			quitGame();
			return true;
			break;
		case kActionreset:
			loadEfhGame();
			resetGame();
			found = true;
			_saveAuthorized = true;
			break;
		case Common::KEYCODE_x: // mysterious and unexpected keycode ?
			found = true;
			break;
		default:
			break;
		}
	}

	setKeymap(kKeymapDefault);

	displayAnimFrames(0xFE, true);
	return false;
}

void EfhEngine::displayCombatMenu(int16 charId) {
	debugC(6, kDebugEngine, "displayCombatMenu %d", charId);

	Common::String buffer = Common::String::format("%s:", _npcBuf[charId]._name);
	setTextColorWhite();
	setTextPos(144, 7);
	displayStringAtTextPos(buffer);
	setTextPos(152, 79);
	displayStringAtTextPos("A");
	setTextColorRed();
	displayStringAtTextPos("ttack");
	sayText("Attack", kMenu);
	setTextPos(195, 79);
	setTextColorWhite();
	displayStringAtTextPos("H");
	setTextColorRed();
	displayStringAtTextPos("ide");
	sayText("Hide", kMenu);
	setTextPos(152, 88);
	setTextColorWhite();
	displayStringAtTextPos("D");
	setTextColorRed();
	displayStringAtTextPos("efend");
	sayText("Defend", kMenu);
	setTextPos(195, 88);
	setTextColorWhite();
	displayStringAtTextPos("R");
	setTextColorRed();
	displayStringAtTextPos("un");
	sayText("Run", kMenu);
	setTextPos(152, 97);
	setTextColorWhite();
	displayStringAtTextPos("S");
	setTextColorRed();
	displayStringAtTextPos("tatus");
	sayText("Status", kMenu);
}

void EfhEngine::displayMenuItemString(int16 menuBoxId, int16 thisBoxId, int16 minX, int16 maxX, int16 minY, const char *str) {
	debugC(6, kDebugEngine, "displayMenuItemString %d %d %d->%d %d %s", menuBoxId, thisBoxId, minX, maxX, minY, str);

	if (menuBoxId == thisBoxId) {
		if (_menuDepth == 0)
			setTextColorWhite();
		else
			setTextColorGrey();

		if (_selectedMenuBox != thisBoxId) {
			sayText(str, kNoRestriction, Common::TextToSpeechManager::INTERRUPT);
			_selectedMenuBox = thisBoxId;
		}

		Common::String buffer = Common::String::format("> %s <", str);
		displayCenteredString(buffer, minX, maxX, minY);
		setTextColorRed();
	} else {
		if (_menuDepth == 0)
			setTextColorRed();
		else
			setTextColorGrey();

		displayCenteredString(str, minX, maxX, minY);
	}
}

void EfhEngine::displayStatusMenu(int16 windowId) {
	debugC(3, kDebugEngine, "displayStatusMenu %d", windowId);

	for (uint counter = 0; counter < 9; ++counter) {
		drawColoredRect(80, 39 + 14 * counter, 134, 47 + 14 * counter, 0);
	}

	if (_menuDepth != 0)
		setTextColorGrey();

	displayMenuItemString(windowId, 0, 80, 134, 39, "EQUIP");
	displayMenuItemString(windowId, 1, 80, 134, 53, "USE");
	displayMenuItemString(windowId, 2, 80, 134, 67, "GIVE");
	displayMenuItemString(windowId, 3, 80, 134, 81, "TRADE");
	displayMenuItemString(windowId, 4, 80, 134, 95, "DROP");
	displayMenuItemString(windowId, 5, 80, 134, 109, "INFO.");
	displayMenuItemString(windowId, 6, 80, 134, 123, "PASSIVE");
	displayMenuItemString(windowId, 7, 80, 134, 137, "ACTIVE");
	displayMenuItemString(windowId, 8, 80, 134, 151, "LEAVE");

	setTextColorRed();
}

void EfhEngine::prepareStatusRightWindowIndexes(int16 menuId, int16 charId) {
	debugC(6, kDebugEngine, "prepareStatusRightWindowIndexes %d %d", menuId, charId);

	_menuItemCounter = 0;

	switch (menuId) {
	case kEfhMenuInfo:
		for (int16 counter = 0; counter <= 10; ++counter) {
			if (_npcBuf[charId]._infoScore[counter] != 0) {
				_menuStatItemArr[_menuItemCounter++] = counter + 26;
			}
		}
		break;
	case kEfhMenuPassive:
		for (int16 counter = 0; counter <= 10; ++counter) {
			if (_npcBuf[charId]._passiveScore[counter] != 0) {
				_menuStatItemArr[_menuItemCounter++] = counter + 15;
			}
		}
		break;
	case kEfhMenuActive:
		for (int16 counter = 0; counter <= 14; ++counter) {
			if (_npcBuf[charId]._activeScore[counter] != 0) {
				_menuStatItemArr[_menuItemCounter++] = counter;
			}
		}
		break;
	default:
		for (uint counter = 0; counter < 10; ++counter) {
			if (_npcBuf[charId]._inventory[counter]._ref != 0x7FFF) {
				_menuStatItemArr[_menuItemCounter++] = counter;
			}
		}
		break;
	}
}

void EfhEngine::displayCharacterSummary(int16 curMenuLine, int16 npcId) {
	debugC(3, kDebugEngine, "displayCharacterSummary %d %d", curMenuLine, npcId);

	setTextColorRed();
	Common::String buffer1 = _npcBuf[npcId]._name;
	setTextPos(146, 27);
	buffer1 = "Name: " + buffer1;
	Common::String ttsMessage = buffer1;
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Level: %d", getXPLevel(_npcBuf[npcId]._xp));
	ttsMessage += '\n' + buffer1;
	setTextPos(146, 36);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("XP: %u", _npcBuf[npcId]._xp);
	ttsMessage += '\n' + buffer1;
	setTextPos(227, 36);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Speed: %d", _npcBuf[npcId]._speed);
	ttsMessage += '\n' + buffer1;
	setTextPos(146, 45);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Defense: %d", getEquipmentDefense(npcId));
	ttsMessage += '\n' + buffer1;
	setTextPos(146, 54);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Hit Points: %d", _npcBuf[npcId]._hitPoints);
	ttsMessage += '\n' + buffer1;
	setTextPos(146, 63);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Max HP: %d", _npcBuf[npcId]._maxHP);
	ttsMessage += '\n' + buffer1;
	setTextPos(227, 63);
	displayStringAtTextPos(buffer1);

	if (curMenuLine == -1) {
		sayText(ttsMessage, kMenu);
	}

	displayCenteredString("Inventory", 144, 310, 72);
	sayText("Inventory", kMenu);

	if (_menuItemCounter == 0) {
		if (curMenuLine != -1)
			setTextColorWhite();

		displayCenteredString("Nothing Carried", 144, 310, 117);
		sayText("Nothing Carried", kMenu);
		setTextColorRed();
		return;
	}

	for (int counter = 0; counter < _menuItemCounter; ++counter) {
		if (_menuDepth == 0)
			setTextColorGrey();
		else {
			if (counter == curMenuLine)
				setTextColorWhite();
		}
		int16 textPosY = 81 + counter * 9;
		int16 itemId = _npcBuf[npcId]._inventory[_menuStatItemArr[counter]]._ref;
		if (itemId != 0x7FFF) {
			if (_npcBuf[npcId]._inventory[_menuStatItemArr[counter]].isEquipped()) {
				setTextPos(146, textPosY);
				displayCharAtTextPos('E');
				sayText("E", kMenu);
			}
		}

		setTextPos(152, textPosY);
		if (counter == curMenuLine) {
			buffer1 = Common::String::format("%c>", 'A' + counter);
		} else {
			buffer1 = Common::String::format("%c)", 'A' + counter);
		}
		displayStringAtTextPos(buffer1);
		sayText(buffer1, kMenu);

		if (itemId != 0x7FFF) {
			setTextPos(168, textPosY);
			buffer1 = Common::String::format("  %s", _items[itemId]._name);
			displayStringAtTextPos(buffer1);
			setTextPos(262, textPosY);
			sayText(buffer1, kMenu);

			if (_items[itemId]._defense > 0) {
				int16 curHitPoints = _npcBuf[npcId]._inventory[_menuStatItemArr[counter]]._curHitPoints;
				if (curHitPoints != 0xFF) {
					buffer1 = Common::String::format("%d", 1 + curHitPoints / 8);
					displayStringAtTextPos(buffer1);
					setTextPos(286, textPosY);
					displayStringAtTextPos("Def");
					sayText(buffer1 + " Defense", kMenu);
				}
			} else if (_items[itemId]._uses != 0x7F) {
				int16 stat1 = _npcBuf[npcId]._inventory[_menuStatItemArr[counter]].getUsesLeft();
				if (stat1 != 0x7F) {
					buffer1 = Common::String::format("%d", stat1);
					displayStringAtTextPos(buffer1);
					setTextPos(286, textPosY);
					if (stat1 == 1) {
						displayStringAtTextPos("Use");
						sayText(buffer1 + " Use", kMenu);
					} else {
						displayStringAtTextPos("Uses");
						sayText(buffer1 + " Uses", kMenu);
					}
				}
			}
		}
		setTextColorRed();
	}
}

void EfhEngine::displayCharacterInformationOrSkills(int16 curMenuLine, int16 charId) {
	debugC(3, kDebugEngine, "displayCharacterInformationOrSkills %d %d", curMenuLine, charId);

	setTextColorRed();
	Common::String buffer = _npcBuf[charId]._name;
	buffer = "Name: " + buffer;
	setTextPos(146, 27);
	displayStringAtTextPos(buffer);

	sayText(buffer, kMenu);

	if (_menuItemCounter <= 0) {
		if (curMenuLine != -1)
			setTextColorWhite();
		displayCenteredString("No Skills To Select", 144, 310, 96);
		setTextColorRed();

		sayText("No Skills To Select", kMenu);

		return;
	}

	for (int counter = 0; counter < _menuItemCounter; ++counter) {
		if (counter == curMenuLine)
			setTextColorWhite();
		int16 textPosY = 38 + counter * 9;
		setTextPos(146, textPosY);
		if (counter == curMenuLine) {
			buffer = Common::String::format("%c>", 'A' + counter);
		} else {
			buffer = Common::String::format("%c)", 'A' + counter);
		}

		displayStringAtTextPos(buffer);
		sayText(buffer, kMenu);
		setTextPos(163, textPosY);
		int16 scoreId = _menuStatItemArr[counter];
		displayStringAtTextPos(kSkillArray[scoreId]);
		sayText(kSkillArray[scoreId], kMenu);
		if (scoreId < 15)
			buffer = Common::String::format("%d", _npcBuf[charId]._activeScore[_menuStatItemArr[counter]]);
		else if (scoreId < 26)
			buffer = Common::String::format("%d", _npcBuf[charId]._passiveScore[_menuStatItemArr[counter] - 15]);
		else if (scoreId < 37)
			buffer = Common::String::format("%d", _npcBuf[charId]._infoScore[_menuStatItemArr[counter] - 26]);
		setTextPos(278, textPosY);
		displayStringAtTextPos(buffer);
		sayText(buffer, kMenu);
		setTextColorRed();
	}
}

void EfhEngine::displayStatusMenuActions(int16 menuId, int16 curMenuLine, int16 npcId) {
	debugC(6, kDebugEngine, "displayStatusMenuActions %d %d %d", menuId, curMenuLine, npcId);

	drawColoredRect(144, 15, 310, 184, 0);
	displayCenteredString("(ESCape Aborts)", 144, 310, 175);
	_textColor = 0x0E;
	switch (menuId) {
	case kEfhMenuEquip:
		displayCenteredString("Select Item to Equip", 144, 310, 15);
		sayText("Select Item to Equip", kMenu);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case kEfhMenuUse:
		displayCenteredString("Select Item to Use", 144, 310, 15);
		sayText("Select Item to Use", kMenu);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case kEfhMenuGive:
		displayCenteredString("Select Item to Give", 144, 310, 15);
		sayText("Select Item to Give", kMenu);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case kEfhMenuTrade:
		displayCenteredString("Select Item to Trade", 144, 310, 15);
		sayText("Select Item to Trade", kMenu);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case kEfhMenuDrop:
		displayCenteredString("Select Item to Drop", 144, 310, 15);
		sayText("Select Item to Drop", kMenu);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case kEfhMenuInfo:
		displayCenteredString("Character Information", 144, 310, 15);
		sayText("Character Information", kMenu);
		displayCharacterInformationOrSkills(curMenuLine, npcId);
		break;
	case kEfhMenuPassive:
		displayCenteredString("Passive Skills", 144, 310, 15);
		sayText("Passive Skills", kMenu);
		displayCharacterInformationOrSkills(curMenuLine, npcId);
		break;
	case kEfhMenuActive:
		displayCenteredString("Active Skills", 144, 310, 15);
		sayText("Active Skills", kMenu);
		displayCharacterInformationOrSkills(curMenuLine, npcId);
		break;
	case kEfhMenuLeave:
	case kEfhMenuInvalid:
		displayCenteredString("Character Summary", 144, 310, 15);
		sayText("Character Summary", kMenu);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	default:
		break;
	}

	sayText("Escape Aborts", kMenu);
	_sayMenu = false;
}

void EfhEngine::prepareStatusMenu(int16 windowId, int16 menuId, int16 curMenuLine, int16 charId, bool refreshFl) {
	debugC(6, kDebugEngine, "prepareStatusMenu %d %d %d %d %s", windowId, menuId, curMenuLine, charId, refreshFl ? "True" : "False");

	displayStatusMenu(windowId);

	prepareStatusRightWindowIndexes(menuId, charId);
	displayStatusMenuActions(menuId, curMenuLine, charId);

	if (refreshFl)
		displayFctFullScreen();
}

void EfhEngine::displayWindowAndStatusMenu(int16 charId, int16 windowId, int16 menuId, int16 curMenuLine) {
	debugC(6, kDebugEngine, "displayWindowAndStatusMenu %d %d %d %d", charId, windowId, menuId, curMenuLine);

	for (int counter = 0; counter < 2; ++counter) {
		displayWindow(_menuBuf, 0, 0, _decompBuf);
		prepareStatusMenu(windowId, menuId, curMenuLine, charId, false);

		if (counter == 0)
			displayFctFullScreen();
	}
}

int16 EfhEngine::displayStringInSmallWindowWithBorder(const Common::String &str, bool delayFl, int16 charId, int16 windowId, int16 menuId, int16 curMenuLine) {
	debugC(3, kDebugEngine, "displayStringInSmallWindowWithBorder %s %s %d %d %d %d", str.c_str(), delayFl ? "True" : "False", charId, windowId, menuId, curMenuLine);

	sayText(str, kNoRestriction);
	int16 retVal = 0;

	for (uint counter = 0; counter < 2; ++counter) {
		prepareStatusMenu(windowId, menuId, curMenuLine, charId, false);
		displayWindow(_windowWithBorderBuf, 19, 113, _decompBuf);

		if (counter == 0) {
			script_parse(str, 28, 122, 105, 166, false);
		} else {
			retVal = script_parse(str, 28, 122, 105, 166, true);
		}
		// The original is only calling displayFctFullScreen when counter = 0, but it's related to the screen buffers which aren't used in ScummVM implementation
		// Calling it once fixes the (almost) unreadable text displayed otherwise.
		// Maybe a refactoring to remove those 0..1 loop would be useful at some point.
		displayFctFullScreen();
	}

	if (delayFl) {
		getLastCharAfterAnimCount(_guessAnimationAmount);
		displayWindowAndStatusMenu(charId, windowId, menuId, curMenuLine);
	}

	return retVal;
}

int16 EfhEngine::handleStatusMenu(int16 gameMode, int16 charId) {
	debugC(3, kDebugEngine, "handleStatusMenu %d %d", gameMode, charId);

	int16 menuId = kEfhMenuInvalid;
	int16 selectedLine = -1;
	int16 windowId = -1;
	int16 curMenuLine = -1;
	bool selectionDoneFl = false;
	bool var2 = false;
	_selectedMenuBox = 0;

	saveAnimImageSetId();

	_statusMenuActive = true;
	_menuDepth = 0;

	Common::Keymapper *keymapper = g_system->getEventManager()->getKeymapper();

	displayWindowAndStatusMenu(charId, windowId, menuId, curMenuLine);

	for (;;) {
		if (windowId != -1)
			prepareStatusMenu(windowId, menuId, curMenuLine, charId, true);
		else
			windowId = kEfhMenuEquip;

		keymapper->disableAllGameKeymaps();
		keymapper->getKeymap("status-menu-navigation")->setEnabled(true);
		if (_menuDepth == 0)
			keymapper->getKeymap("status-menu")->setEnabled(true);
		else
			keymapper->getKeymap("status-menu-submenu")->setEnabled(true);
		do {
			Common::CustomEventType input = handleAndMapInput(false);
			if (_menuDepth == 0) {
				switch (input) {
				case kActionActive:
					windowId = kEfhMenuActive;
					input = kActionSelect;
					break;
				case kActionDrop:
					windowId = kEfhMenuDrop;
					input = kActionSelect;
					break;
				case kActionEquip:
					windowId = kEfhMenuEquip;
					input = kActionSelect;
					break;
				case kActionGive:
					windowId = kEfhMenuGive;
					input = kActionSelect;
					break;
				case kActionInfo:
					windowId = kEfhMenuInfo;
					input = kActionSelect;
					break;
				case kActionExitStatusMenu:
					stopTextToSpeech();
					windowId = kEfhMenuLeave;
					input = kActionSelect;
					break;
				case kActionPassive:
					windowId = kEfhMenuPassive;
					input = kActionSelect;
					break;
				case kActionTrade:
					windowId = kEfhMenuTrade;
					input = kActionSelect;
					break;
				case kActionUse:
					windowId = kEfhMenuUse;
					input = kActionSelect;
					break;
				default:
					debugC(9, kDebugEngine, "handleStatusMenu - unhandled keys");
					break;
				}

				if (input == Common::KEYCODE_RETURN) {
					_selectedMenuBox = -1;
				}
			} else if (_menuDepth == 1) {
				// in the sub-menus, only a list of selectable items is displayed
				for (int i = 0; i < ARRAYSIZE(_efhStatusMenuSubMenuCodes); ++i) {
					if (input == _efhStatusMenuSubMenuCodes[i]._action) {
						if (_efhStatusMenuSubMenuCodes[i]._id < _menuItemCounter) {
							curMenuLine = _efhStatusMenuSubMenuCodes[i]._id;
							input = kActionSelect;
						}
					}
				}
			}

			switch (input) {
			case kActionSelect:
				// case 0xFA: Joystick button 1
				if (_menuDepth == 0) {
					menuId = windowId;
					if (menuId >= kEfhMenuLeave)
						selectionDoneFl = true;
					else {
						_sayMenu = true;
						_menuDepth = 1;
						curMenuLine = 0;
						keymapper->getKeymap("status-menu")->setEnabled(false);
						keymapper->getKeymap("status-menu-submenu")->setEnabled(true);
					}
				} else if (_menuDepth == 1) {
					if (_menuItemCounter == 0) {
						_menuDepth = 0;
						curMenuLine = -1;
						menuId = kEfhMenuInvalid;
						_sayMenu = true;
						prepareStatusMenu(windowId, menuId, curMenuLine, charId, true);
						keymapper->getKeymap("status-menu-submenu")->setEnabled(false);
						keymapper->getKeymap("status-menu")->setEnabled(true);
					} else {
						selectedLine = curMenuLine;
						selectionDoneFl = true;
					}
				}
				break;
			case kActionExitSubMenu:
				_menuDepth = 0;
				curMenuLine = -1;
				menuId = kEfhMenuInvalid;
				stopTextToSpeech();
				_sayMenu = true;
				prepareStatusMenu(windowId, menuId, curMenuLine, charId, true);
				keymapper->getKeymap("status-menu-submenu")->setEnabled(false);
				keymapper->getKeymap("status-menu")->setEnabled(true);
				break;
			case kActionScrollDown:
				// Original checks joystick axis: case 0xCC, 0xCF
				if (_menuDepth == 0) {
					if (++windowId > kEfhMenuLeave)
						windowId = kEfhMenuEquip;
				} else if (_menuDepth == 1) {
					if (_menuItemCounter != 0) {
						++curMenuLine;
						if (curMenuLine > _menuItemCounter - 1)
							curMenuLine = 0;
					}
				}
				break;
			case kActionScrollUp:
				// Original checks joystick axis: case 0xC7, 0xCA
				if (_menuDepth == 0) {
					if (--windowId < kEfhMenuEquip)
						windowId = kEfhMenuLeave;
				} else if (_menuDepth == 1) {
					if (_menuItemCounter != 0) {
						--curMenuLine;
						if (curMenuLine < 0)
							curMenuLine = _menuItemCounter - 1;
					}
				}
				break;
			default:
				break;
			}

			prepareStatusMenu(windowId, menuId, curMenuLine, charId, true);

		} while (!selectionDoneFl && !shouldQuit()); // Loop until a menu entry is confirmed by the user by pressing the enter key

		setKeymap(kKeymapDefault);

		bool validationFl = true;

		int16 objectId;
		int16 itemId;
		switch (menuId) {
		case kEfhMenuEquip:
			objectId = _menuStatItemArr[selectedLine];
			itemId = _npcBuf[charId]._inventory[objectId]._ref; // CHECKME: Useless?
			sayText(_items[itemId]._name, kNoRestriction, Common::TextToSpeechManager::INTERRUPT);
			tryToggleEquipped(charId, objectId, windowId, menuId, curMenuLine);
			if (gameMode == 2) {
				restoreAnimImageSetId();
				_statusMenuActive = false;
				return 0x7D00;
			}
			break;
		case kEfhMenuUse:
			objectId = _menuStatItemArr[selectedLine];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			sayText(_items[itemId]._name, kNoRestriction, Common::TextToSpeechManager::INTERRUPT);
			if (gameMode == 2) {
				restoreAnimImageSetId();
				_statusMenuActive = false;
				return objectId;
			}

			if (handleInteractionText(_mapPosX, _mapPosY, charId, itemId, 2, -1)) {
				_statusMenuActive = false;
				return -1;
			}

			useObject(charId, objectId, windowId, menuId, curMenuLine, 2);
			break;
		case kEfhMenuGive:
			objectId = _menuStatItemArr[selectedLine];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			sayText(_items[itemId]._name, kNoRestriction, Common::TextToSpeechManager::INTERRUPT);
			_initiatedTalkByMenu = true;
			if (hasObjectEquipped(charId, objectId) && isItemCursed(itemId)) {
				displayStringInSmallWindowWithBorder("The item is cursed!  IT IS EVIL!!!!!!!!", true, charId, windowId, menuId, curMenuLine);
			} else {
				if (hasObjectEquipped(charId, objectId)) {
					displayStringInSmallWindowWithBorder("Item is Equipped!  Give anyway?", false, charId, windowId, menuId, curMenuLine);
					if (!getValidationFromUser())
						validationFl = false;
					stopTextToSpeech();
					displayWindowAndStatusMenu(charId, windowId, menuId, curMenuLine);
				}
				if (validationFl) {
					if (gameMode == 2) {
						displayStringInSmallWindowWithBorder("Not a Combat Option !", true, charId, windowId, menuId, curMenuLine);
					} else {
						removeObject(charId, objectId);
						if (handleInteractionText(_mapPosX, _mapPosY, charId, itemId, 3, -1)) {
							_statusMenuActive = false;
							return -1;
						}
					}
				}
			}

			break;
		case kEfhMenuTrade:
			objectId = _menuStatItemArr[selectedLine];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			sayText(_items[itemId]._name, kNoRestriction, Common::TextToSpeechManager::INTERRUPT);
			if (hasObjectEquipped(charId, objectId) && isItemCursed(itemId)) {
				displayStringInSmallWindowWithBorder("The item is cursed!  IT IS EVIL!!!!!!!!", true, charId, windowId, menuId, curMenuLine);
				break;
			}

			if (hasObjectEquipped(charId, objectId)) {
				displayStringInSmallWindowWithBorder("Item is Equipped!  Trade anyway?", false, charId, windowId, menuId, curMenuLine);
				if (!getValidationFromUser())
					validationFl = false;
				stopTextToSpeech();
				displayWindowAndStatusMenu(charId, windowId, menuId, curMenuLine);
			}

			if (validationFl) {
				bool givenFl;
				int16 destCharId;
				do {
					if (_teamChar[2]._id != -1) {
						displayStringInSmallWindowWithBorder("Who will you give the item to?", false, charId, windowId, menuId, curMenuLine);
						destCharId = selectOtherCharFromTeam();
						stopTextToSpeech();
						var2 = false;
					} else if (_teamChar[1]._id == -1) {
						destCharId = 0x1A;
						var2 = false;
					} else {
						var2 = true;
						if (_teamChar[0]._id == charId)
							destCharId = 1;
						else
							destCharId = 0;
					}

					if (destCharId != 0x1A && destCharId != 0x1B) {
						givenFl = giveItemTo(_teamChar[destCharId]._id, objectId, charId);
						sayText(_npcBuf[_teamChar[destCharId]._id]._name, kNoRestriction);
						if (!givenFl) {
							displayStringInSmallWindowWithBorder("That character cannot carry anymore!", false, charId, windowId, menuId, curMenuLine);
							getLastCharAfterAnimCount(_guessAnimationAmount);
						}
					} else {
						if (destCharId == 0x1A) {
							displayStringInSmallWindowWithBorder("No one to trade with!", false, charId, windowId, menuId, curMenuLine);
							getLastCharAfterAnimCount(_guessAnimationAmount);
							destCharId = 0x1B;
						}
						givenFl = false;
					}
				} while (!givenFl && !var2 && destCharId != 0x1B && !shouldQuit());

				if (givenFl) {
					removeObject(charId, objectId);
					if (gameMode == 2) {
						restoreAnimImageSetId();
						_statusMenuActive = false;
						return 0x7D00;
					}
				}

				displayWindowAndStatusMenu(charId, windowId, menuId, curMenuLine);
			}
			break;
		case kEfhMenuDrop:
			objectId = _menuStatItemArr[selectedLine];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			sayText(_items[itemId]._name, kNoRestriction, Common::TextToSpeechManager::INTERRUPT);
			if (hasObjectEquipped(charId, objectId) && isItemCursed(itemId)) {
				displayStringInSmallWindowWithBorder("The item is cursed!  IT IS EVIL!!!!!!!!", true, charId, windowId, menuId, curMenuLine);
			} else {
				if (hasObjectEquipped(charId, objectId)) {
					displayStringInSmallWindowWithBorder("Item Is Equipped!  Drop Anyway?", false, charId, windowId, menuId, curMenuLine);
					if (!getValidationFromUser())
						validationFl = false;
					displayWindowAndStatusMenu(charId, windowId, menuId, curMenuLine);
				}
				if (validationFl) {
					stopTextToSpeech();
					_sayMenu = true;
					removeObject(charId, objectId);
					if (gameMode == 2) {
						restoreAnimImageSetId();
						_statusMenuActive = false;
						return 0x7D00;
					}

					if (handleInteractionText(_mapPosX, _mapPosY, charId, itemId, 1, -1)) {
						_statusMenuActive = false;
						return -1;
					}
				}
			}
			break;
		case kEfhMenuInfo:
		case kEfhMenuPassive:
		case kEfhMenuActive:
			stopTextToSpeech();
			objectId = _menuStatItemArr[selectedLine];
			sayText(kSkillArray[objectId], kNoRestriction, Common::TextToSpeechManager::INTERRUPT);
			if (gameMode == 2) {
				displayStringInSmallWindowWithBorder("Not a Combat Option!", true, charId, windowId, menuId, curMenuLine);
			} else if (handleInteractionText(_mapPosX, _mapPosY, charId, objectId, 4, -1)) {
				_statusMenuActive = false;
				return -1;
			}
			break;
		default:
			break;
		}

		if (menuId != kEfhMenuLeave) {
			selectionDoneFl = false;
			_menuDepth = 0;
			_sayMenu = true;
			menuId = kEfhMenuInvalid;
			selectedLine = -1;
			curMenuLine = -1;
		}

		if (menuId == kEfhMenuLeave) {
			restoreAnimImageSetId();
			_statusMenuActive = false;
			_sayMenu = true;
			return 0x7FFF;
		}

		if (shouldQuit()) {
			return 0;
		}
	}

	return 0;
}

void EfhEngine::unequipItem(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine) {
	debugC(6,kDebugEngine, "unequipItem %d %d %d %d %d", charId, objectId, windowId, menuId, curMenuLine);

	int16 itemId = _npcBuf[charId]._inventory[objectId]._ref;

	if (!isItemCursed(itemId)) {
		_npcBuf[charId]._inventory[objectId]._stat1 &= 0x7F;
	} else {
		// Original message. "Cursed item can't be unequipped" would make more sense, imho
		displayStringInSmallWindowWithBorder("Cursed Item Already Equipped!", true, charId, windowId, menuId, curMenuLine);
	}
}

void EfhEngine::tryToggleEquipped(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine) {
	debugC(3, kDebugEngine, "tryToggleEquipped %d %d %d %d %d", charId, objectId, windowId, menuId, curMenuLine);

	int16 itemId = _npcBuf[charId]._inventory[objectId]._ref;

	if (hasObjectEquipped(charId, objectId)) {
		unequipItem(charId, objectId, windowId, menuId, curMenuLine);
	} else {
		int16 curType = _items[itemId]._exclusiveType;
		if (curType != 4) {
			for (uint counter = 0; counter < 10; ++counter) {
				int16 curItemId = _npcBuf[charId]._inventory[counter]._ref;
				if (curItemId == 0x7FFF) {
					warning("CHECKME : hack");
					continue;
				}
				if (curType == _items[curItemId]._exclusiveType)
					unequipItem(charId, counter, windowId, menuId, curMenuLine);
			}
		}

		if (curType != 0) {
			// Set item as Equipped
			_npcBuf[charId]._inventory[objectId]._stat1 |= 0x80;
		}
	}
}

int16 EfhEngine::useObject(int16 charId, int16 objectId, int16 teamMonsterId, int16 menuId, int16 curMenuLine, int16 gameMode) {
	debugC(3, kDebugEngine, "useObject %d %d %d %d %d %s", charId, objectId, teamMonsterId, menuId, curMenuLine, gameMode == 3 ? "Combat" : "Normal");

	Common::String buffer1 = "";

	bool objectUsedFl = false;
	bool retVal = false;

	int16 itemId = _npcBuf[charId]._inventory[objectId]._ref;
	switch (_items[itemId]._specialEffect - 1) {
	case 0: // "Demonic Powers", "MindDomination", "Guilt Trip", "Sleep Grenade", "SleepGrenader"
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("The item emits a low droning hum...", false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			int16 victims = 0;
			_messageToBePrinted += "  The item emits a low droning hum...";
			if (getRandom(100) < 50) {
				for (uint ctrMobsterId = 0; ctrMobsterId < 9; ++ctrMobsterId) {
					if (isMonsterActive(teamMonsterId, ctrMobsterId)) {
						++victims;
						_teamMonster[teamMonsterId]._mobsterStatus[ctrMobsterId]._type = kEfhStatusSleeping;
						_teamMonster[teamMonsterId]._mobsterStatus[ctrMobsterId]._duration = getRandom(8);
					}
				}
			} else {
				int16 NumberOfTargets = getRandom(9);
				for (uint ctrMobsterId = 0; ctrMobsterId < 9; ++ctrMobsterId) {
					if (NumberOfTargets == 0)
						break;

					if (isMonsterActive(teamMonsterId, ctrMobsterId)) {
						++victims;
						--NumberOfTargets;
						_teamMonster[teamMonsterId]._mobsterStatus[ctrMobsterId]._type = kEfhStatusSleeping;
						_teamMonster[teamMonsterId]._mobsterStatus[ctrMobsterId]._duration = getRandom(8);
					}
				}
			}
			// The original was duplicating this code in each branch of the previous random check.
			if (victims > 1) {
				buffer1 = Common::String::format("%d %ss fall asleep!", victims, kEncounters[_mapMonsters[_techId][_teamMonster[teamMonsterId]._id]._monsterRef]._name);
			} else {
				buffer1 = Common::String::format("%d %s falls asleep!", victims, kEncounters[_mapMonsters[_techId][_teamMonster[teamMonsterId]._id]._monsterRef]._name);
			}
			_messageToBePrinted += buffer1;
		}

		objectUsedFl = true;
		break;
	case 1: // "Chilling Touch", "Guilt", "Petrify Rod", "Elmer's Gun"
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("The item grows very cold for a moment...", false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += "  The item emits a blue beam...";
			int16 victim = 0;
			if (getRandom(100) < 50) {
				for (uint ctrEffectId = 0; ctrEffectId < 9; ++ctrEffectId) {
					if (isMonsterActive(teamMonsterId, ctrEffectId)) {
						++victim;
						_teamMonster[teamMonsterId]._mobsterStatus[ctrEffectId]._type = kEfhStatusFrozen;
						_teamMonster[teamMonsterId]._mobsterStatus[ctrEffectId]._duration = getRandom(8);
					}
				}
			} else {
				int16 varAC = getRandom(9);
				for (uint ctrMobsterId = 0; ctrMobsterId < 9; ++ctrMobsterId) {
					if (varAC == 0)
						break;

					if (isMonsterActive(teamMonsterId, ctrMobsterId)) {
						++victim;
						--varAC;
						_teamMonster[teamMonsterId]._mobsterStatus[ctrMobsterId]._type = kEfhStatusFrozen;
						_teamMonster[teamMonsterId]._mobsterStatus[ctrMobsterId]._duration = getRandom(8);
					}
				}
			}
			// <CHECKME>: This part is only present in the original in the case < 50, but for me
			// it's missing in the other case as there's an effect (frozen enemies) but no feedback to the player
			if (victim > 1) {
				buffer1 = Common::String::format("%d %ss are frozen in place!", victim, kEncounters[_mapMonsters[_techId][_teamMonster[teamMonsterId]._id]._monsterRef]._name);
			} else {
				buffer1 = Common::String::format("%d %s is frozen in place!", victim, kEncounters[_mapMonsters[_techId][_teamMonster[teamMonsterId]._id]._monsterRef]._name);
			}
			_messageToBePrinted += buffer1;
			// </CHECKME>
		}

		objectUsedFl = true;
		break;
	case 2:
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("A serene feeling passes through the air...", false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += "  The combat pauses...as there is a moment of forgiveness...";
			_alertDelay = 0;
		}

		objectUsedFl = true;
		break;
	case 4: // "Unholy Sinwave", "Holy Water"
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("A dark sense fills your soul...then fades!", false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += "  A dark gray fiery whirlwind surrounds the poor victim...the power fades and death abounds!";
			if (getRandom(100) < 50) {
				for (uint counter = 0; counter < 9; ++counter) {
					if (getRandom(100) < 50) {
						_mapMonsters[_techId][_teamMonster[teamMonsterId]._id]._hitPoints[counter] = 0;
					}
				}
			} else {
				for (uint counter = 0; counter < 9; ++counter) {
					if (isMonsterActive(teamMonsterId, counter)) {
						if (getRandom(100) < 50) {
							_mapMonsters[_techId][_teamMonster[teamMonsterId]._id]._hitPoints[counter] = 0;
						}
						break;
					}
				}
			}
		}
		objectUsedFl = true;
		break;
	case 5: // "Lucifer'sTouch", "Book of Death", "Holy Cross"
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("A dark sense fills your soul...then fades!", false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			if (getRandom(100) < 50) {
				_messageToBePrinted += "  A dark fiery whirlwind surrounds the poor victim...the power fades and all targeted die!";
				for (uint counter = 0; counter < 9; ++counter) {
					_mapMonsters[_techId][_teamMonster[teamMonsterId]._id]._hitPoints[counter] = 0;
				}
			} else {
				_messageToBePrinted += "  A dark fiery whirlwind surrounds the poor victim...the power fades and one victim dies!";
				for (uint counter = 0; counter < 9; ++counter) {
					if (isMonsterActive(teamMonsterId, counter)) {
						_mapMonsters[_techId][_teamMonster[teamMonsterId]._id]._hitPoints[counter] = 0;
					}
				}
			}
		}

		objectUsedFl = true;
		break;
	case 12: // "Terror Gaze", "Servitude Rod", "Despair Ankh", "ConfusionPrism", "Pipe of Peace", "Red Cape", "Peace Symbol", "Hell Badge"
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("There is no apparent affect!", false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += "  The magic sparkles brilliant hues in the air!";
			setMapMonsterAggressivenessAndMovementType(teamMonsterId, _items[itemId]._defenseType);
		}
		objectUsedFl = true;
		break;
	case 14: { // "Feathered Cap"
		int16 teamCharId;
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("Who will use the item?", false, charId, teamMonsterId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else {
			teamCharId = teamMonsterId;
		}

		if (teamCharId != 0x1B) { // Escape code, which means the user cancelled the selection
			buffer1 = "  The magic makes the user as quick and agile as a bird!";
			if (gameMode == 2) {
				displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
			}
			_teamChar[teamCharId]._pctDodgeMiss -= 50;
			if (_teamChar[teamCharId]._pctDodgeMiss < 0)
				_teamChar[teamCharId]._pctDodgeMiss = 0;
		}

		objectUsedFl = true;
	} break;
	case 15: { // "Regal Crown"
		int16 teamCharId;
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("Who will use the item?", false, charId, teamMonsterId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else {
			teamCharId = teamMonsterId;
		}

		if (teamCharId != 0x1B) {
			buffer1 = "  The magic makes the user invisible!";
			if (gameMode == 2) {
				displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
			}

			_teamChar[teamCharId]._pctVisible -= 50;
			if (_teamChar[teamCharId]._pctVisible < 0)
				_teamChar[teamCharId]._pctVisible = 0;
		}

		objectUsedFl = true;
	} break;
	case 16: { // Fairy Dust
		_mapPosX = getRandom(_largeMapFlag ? 63 : 23);
		_mapPosY = getRandom(_largeMapFlag ? 63 : 23);
		int16 tileFactId = getTileFactId(_mapPosX, _mapPosY);

		if (_tileFact[tileFactId]._status == 0) {
			totalPartyKill();
			buffer1 = "The entire party vanishes in a flash... only to appear in stone !";
			if (gameMode == 2) {
				displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
				retVal = true;
			}
		} else {
			if (tileFactId == 0 || tileFactId == 0x48) {
				buffer1 = "The entire party vanishes in a flash...but re-appears, as if nothing happened!";
				if (gameMode == 2) {
					displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
				} else {
					_messageToBePrinted += buffer1;
					retVal = true;
				}
			} else {
				buffer1 = "The entire party vanishes in a flash...only to appear elsewhere!";
				if (gameMode == 2) {
					displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
				} else {
					_messageToBePrinted += buffer1;
					retVal = true;
				}
			}
		}

		objectUsedFl = true;
	} break;
	case 17: { // "Devil Dust"
		_mapPosX = _items[itemId]._field19_mapPosX_or_maxDeltaPoints;
		_mapPosY = _items[itemId]._mapPosY;
		int16 tileFactId = getTileFactId(_mapPosX, _mapPosY);
		if (_tileFact[tileFactId]._status == 0) {
			totalPartyKill();
			buffer1 = "The entire party vanishes in a flash... only to appear in stone !";
			if (gameMode == 2) {
				displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
				retVal = true;
			}
		} else {
			if (tileFactId == 0 || tileFactId == 0x48) {
				buffer1 = "The entire party vanishes in a flash...but re-appears, as if nothing happened!";
				if (gameMode == 2) {
					displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
				} else {
					_messageToBePrinted += buffer1;
					retVal = true;
				}
			} else {
				buffer1 = "The entire party vanishes in a flash...only to appear elsewhere!";
				if (gameMode == 2) {
					displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
				} else {
					_messageToBePrinted += buffer1;
					retVal = true;
				}
			}
		}

		objectUsedFl = true;
	} break;
	case 18:
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("The item makes a loud noise!", false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			int16 teamCharId = teamMonsterId;
			if (teamCharId != 0x1B) {
				if (_teamChar[teamCharId]._status._type == kEfhStatusFrozen) {
					// The message is weird because it's a duplicate of case 28, which is about "curing" sleep... But it's about being frozen.
					// We could improve the description, but that's the way the original deals with it
					_messageToBePrinted += "  The item makes a loud noise, awakening the character!";
					_teamChar[teamCharId]._status._type = kEfhStatusNormal;
					_teamChar[teamCharId]._status._duration = 0;
				} else {
					_messageToBePrinted += "  The item makes a loud noise, but has no effect!";
				}
			}
		}

		objectUsedFl = true;
		break;
	case 19: // "Junk"
		buffer1 = "  * The item breaks!";
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += buffer1;
		}
		setCharacterObjectToBroken(charId, objectId);
		objectUsedFl = true;
		break;
	case 23: // "Divining Rod"
		buffer1 = Common::String::format("The %s says, '", _items[itemId]._name);
		if (_items[itemId]._field19_mapPosX_or_maxDeltaPoints < _mapPosX) {
			if (_items[itemId]._mapPosY < _mapPosY) {
				buffer1 += "North West!";
			} else if (_items[itemId]._mapPosY > _mapPosY) {
				buffer1 += "South West!";
			} else {
				buffer1 += "West!";
			}
		} else if (_items[itemId]._field19_mapPosX_or_maxDeltaPoints > _mapPosX) {
			if (_items[itemId]._mapPosY < _mapPosY) {
				buffer1 += "North East!";
			} else if (_items[itemId]._mapPosY > _mapPosY) {
				buffer1 += "South East!";
			} else {
				buffer1 += "East!";
			}
		} else { // equals _mapPosX
			if (_items[itemId]._mapPosY < _mapPosY) {
				buffer1 += "North!";
			} else if (_items[itemId]._mapPosY > _mapPosY) {
				buffer1 += "South!";
			} else {
				buffer1 += "Here!!!";
			}
		}
		buffer1 += "'";
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += buffer1;
			retVal = true;
		}

		objectUsedFl = true;
		break;
	case 24: {
		int16 teamCharId;
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("Who will use this item?", false, charId, teamMonsterId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else
			teamCharId = teamMonsterId;

		if (teamCharId != 0x1B) {
			uint8 varAE = _items[itemId]._defenseType;
			uint8 effectPoints = getRandom(_items[itemId]._field19_mapPosX_or_maxDeltaPoints);
			_npcBuf[_teamChar[teamCharId]._id]._activeScore[varAE] += effectPoints;
			if (_npcBuf[_teamChar[teamCharId]._id]._activeScore[varAE] > 20) {
				_npcBuf[_teamChar[teamCharId]._id]._activeScore[varAE] = 20;
			}
			if (effectPoints > 1)
				buffer1 = Common::String::format("%s increased %d points!", kSkillArray[varAE], effectPoints);
			else
				buffer1 = Common::String::format("%s increased 1 point!", kSkillArray[varAE]);

			if (gameMode == 2) {
				displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
				retVal = true;
			}
		}

		objectUsedFl = true;
	} break;
	case 25: {
		int16 teamCharId;
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("Who will use this item?", false, charId, teamMonsterId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else
			teamCharId = teamMonsterId;

		if (teamCharId != 0x1B) {
			uint8 varAE = _items[itemId]._defenseType;
			uint8 effectPoints = getRandom(_items[itemId]._field19_mapPosX_or_maxDeltaPoints);
			_npcBuf[_teamChar[teamCharId]._id]._activeScore[varAE] -= effectPoints;
			if (_npcBuf[_teamChar[teamCharId]._id]._activeScore[varAE] > 20) {
				_npcBuf[_teamChar[teamCharId]._id]._activeScore[varAE] = 1;
			}
			if (effectPoints > 1)
				buffer1 = Common::String::format("%s lowered %d points!", kSkillArray[varAE], effectPoints);
			else
				buffer1 = Common::String::format("%s lowered 1 point!", kSkillArray[varAE]);

			if (gameMode == 2) {
				displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
				retVal = true;
			}
		}

		objectUsedFl = true;
	} break;
	case 26: // "Black Sphere"
		buffer1 = "The entire party collapses, dead!!!";
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += buffer1;
			retVal = true;
		}
		totalPartyKill();
		objectUsedFl = true;
		break;
	case 27: { // "Magic Pyramid", "Razor Blade"
		int16 teamCharId;
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("Who will use the item?", false, charId, teamMonsterId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else {
			teamCharId = teamMonsterId;
		}

		if (teamCharId != 0x1B) {
			_npcBuf[_teamChar[teamCharId]._id]._hitPoints = 0;
			buffer1 = Common::String::format("%s collapses, dead!!!", _npcBuf[_teamChar[teamCharId]._id]._name);
			if (gameMode == 2) {
				displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
				retVal = true;
			}
		}

		objectUsedFl = true;
	} break;
	case 28: // "Bugle"
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("The item makes a loud noise!", false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			int16 teamCharId = teamMonsterId;
			if (teamCharId != 0x1B) {
				if (_teamChar[teamCharId]._status._type == kEfhStatusSleeping) {
					_messageToBePrinted += "  The item makes a loud noise, awakening the character!";
					_teamChar[teamCharId]._status._type = kEfhStatusNormal;
					_teamChar[teamCharId]._status._duration = 0;
				} else {
					_messageToBePrinted += "  The item makes a loud noise, but has no effect!";
				}
			}
		}

		objectUsedFl = true;
		break;
	case 29: { // "Healing Spray", "Healing Elixir", "Curing Potion", "Magic Potion"
		int16 teamCharId;
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("Who will use the item?", false, charId, teamMonsterId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else {
			teamCharId = teamMonsterId;
		}

		if (teamCharId != 0x1B) {
			int16 effectPoints = getRandom(_items[itemId]._defenseType);
			_npcBuf[_teamChar[teamCharId]._id]._hitPoints += effectPoints;
			if (_npcBuf[_teamChar[teamCharId]._id]._hitPoints > _npcBuf[_teamChar[teamCharId]._id]._maxHP)
				_npcBuf[_teamChar[teamCharId]._id]._hitPoints = _npcBuf[_teamChar[teamCharId]._id]._maxHP;

			if (effectPoints > 1)
				buffer1 = Common::String::format("%s is healed %d points!", _npcBuf[_teamChar[teamCharId]._id]._name, effectPoints);
			else
				buffer1 = Common::String::format("%s is healed 1 point!", _npcBuf[_teamChar[teamCharId]._id]._name);
		}

		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += buffer1;
			retVal = true;
		}

		objectUsedFl = true;
	} break;
	case 30: {
		int16 teamCharId;
		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder("Who will use the item?", false, charId, teamMonsterId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else {
			teamCharId = teamMonsterId;
		}

		if (teamCharId != 0x1B) {
			int16 effectPoints = getRandom(_items[itemId]._defenseType);
			_npcBuf[_teamChar[teamCharId]._id]._hitPoints -= effectPoints;
			if (_npcBuf[_teamChar[teamCharId]._id]._hitPoints < 0)
				_npcBuf[_teamChar[teamCharId]._id]._hitPoints = 0;

			if (effectPoints > 1)
				buffer1 = Common::String::format("%s is harmed for %d points!", _npcBuf[_teamChar[teamCharId]._id]._name, effectPoints);
			else
				buffer1 = Common::String::format("%s is harmed for 1 point!", _npcBuf[_teamChar[teamCharId]._id]._name);
		}

		if (gameMode == 2) {
			displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += buffer1;
			retVal = true;
		}

		objectUsedFl = true;

	} break;
	case 3:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 13:
	case 20:
	case 21:
	case 22:
	default:
		break;
	}

	if (objectUsedFl) {
		int16 usesLeft = _npcBuf[charId]._inventory[objectId].getUsesLeft();
		if (usesLeft != 0x7F) {
			--usesLeft;
			if (usesLeft <= 0) {
				buffer1 = "  * The item breaks!";
				if (gameMode == 2) {
					getLastCharAfterAnimCount(_guessAnimationAmount);
					displayStringInSmallWindowWithBorder(buffer1, false, charId, teamMonsterId, menuId, curMenuLine);
				} else {
					_messageToBePrinted += buffer1;
				}
				setCharacterObjectToBroken(charId, objectId);
			} else {
				// Keep the Equipped bit and set the new number of uses
				_npcBuf[charId]._inventory[objectId]._stat1 &= 0x80;
				_npcBuf[charId]._inventory[objectId]._stat1 |= usesLeft;
			}
		}

		if (gameMode == 2) {
			getLastCharAfterAnimCount(_guessAnimationAmount);
			displayWindowAndStatusMenu(charId, teamMonsterId, menuId, curMenuLine);
		}
	}

	return retVal;
}

} // End of namespace Efh

