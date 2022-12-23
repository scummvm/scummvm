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

namespace Efh {

int16 EfhEngine::sub1C219(Common::String str, int16 menuType, int16 displayOption, bool displayTeamWindowFl) {
	debug("sub1C219 %s %d %d %s", str.c_str(), menuType, displayOption, displayTeamWindowFl ? "True" : "False");

	int16 varA = 0xFF;
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

	drawColoredRect(minX, minY, maxX, maxY, 0);
	if (str.size())
		varA = script_parse(str, minX, minY, maxX, maxY, true);

	if (displayTeamWindowFl)
		displayLowStatusScreen(false);

	if (displayOption != 0) {
		displayFctFullScreen();
		if (_word2C87A)
			_word2C87A = false;
		else {
			drawColoredRect(minX, minY, maxX, maxY, 0);
			if (str.size())
				script_parse(str, minX, minY, maxX, maxY, false);
		}

		if (displayTeamWindowFl)
			displayLowStatusScreen(false);

		if (displayOption >= 2)
			getLastCharAfterAnimCount(_guessAnimationAmount);

		if (displayOption == 3)
			drawColoredRect(minX, minY, maxX, maxY, 0);
	}

	return varA;
}

bool EfhEngine::handleDeathMenu() {
	debug("handleDeathMenu");

	_saveAuthorized = false;

	displayAnimFrames(20, true);
	_imageSetSubFilesIdx = 213;
	drawScreen();

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

	for (bool found = false; !found;) {
		Common::KeyCode input = waitForKey();
		switch (input) {
		case Common::KEYCODE_l:
			//loadEfhGame();
			//TODO:
			//SaveEfhGame opens the GUI save/load screen. It's not possible to save at this point, which is fine, but it's possible to close the screen without loading.
			//Maybe adding the _saveAuthorized flag in the savegame would do the trick and could then be used tp keep found at false and loop on the input selection?
			//like: found = _saveAuthorized
			saveEfhGame();
			found = true;
			_saveAuthorized = true;
			break;
		case Common::KEYCODE_q:
			_shouldQuit = true;
			return true;
			break;
		case Common::KEYCODE_r:
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

	displayAnimFrames(0xFE, true);
	return false;
}

void EfhEngine::displayCombatMenu(int16 charId) {
	debug("displayCombatMenu %d", charId);

	Common::String buffer = _npcBuf[charId]._name;
	buffer += ":";
	setTextColorWhite();
	setTextPos(144, 7);
	displayStringAtTextPos(buffer);
	setTextPos(152, 79);
	displayStringAtTextPos("A");
	setTextColorRed();
	displayStringAtTextPos("ttack");
	setTextPos(195, 79);
	setTextColorWhite();
	displayStringAtTextPos("H");
	setTextColorRed();
	displayStringAtTextPos("ide");
	setTextPos(152, 88);
	setTextColorWhite();
	displayStringAtTextPos("D");
	setTextColorRed();
	displayStringAtTextPos("efend");
	setTextPos(195, 88);
	setTextColorWhite();
	displayStringAtTextPos("R");
	setTextColorRed();
	displayStringAtTextPos("un");
	setTextPos(152, 97);
	setTextColorWhite();
	displayStringAtTextPos("S");
	setTextColorRed();
	displayStringAtTextPos("tatus");
}

void EfhEngine::displayMenuItemString(int16 menuBoxId, int16 thisBoxId, int16 minX, int16 maxX, int16 minY, const char *str) {
	debugC(6, kDebugEngine, "displayMenuItemString %d %d %d->%d %d %s", menuBoxId, thisBoxId, minX, maxX, minY, str);

	if (menuBoxId == thisBoxId) {
		if (_menuDepth == 0)
			setTextColorWhite();
		else
			setTextColorGrey();

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

	int16 maxId = 0;
	int16 minId;
	_menuItemCounter = 0;

	switch (menuId) {
	case 5:
		minId = 26;
		maxId = 36;
		break;
	case 6:
		minId = 15;
		maxId = 25;
		break;
	case 7:
		minId = 0;
		maxId = 14;
		break;
	default:
		minId = -1;
		break;
	}

	if (minId == -1) {
		for (uint counter = 0; counter < 10; ++counter) {
			if (_npcBuf[charId]._inventory[counter]._ref != 0x7FFF) {
				_menuStatItemArr[_menuItemCounter++] = counter;
			}
		}
	} else {
		for (int16 counter = minId; counter < maxId; ++counter) {
			if (_npcBuf[charId]._activeScore[counter] != 0) {
				_menuStatItemArr[_menuItemCounter++] = counter;
			}
		}
	}
}

void EfhEngine::displayCharacterSummary(int16 curMenuLine, int16 npcId) {
	debugC(3, kDebugEngine, "displayCharacterSummary %d %d", curMenuLine, npcId);

	setTextColorRed();
	Common::String buffer1 = _npcBuf[npcId]._name;
	setTextPos(146, 27);
	displayStringAtTextPos("Name: ");
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Level: %d", getXPLevel(_npcBuf[npcId]._xp));
	setTextPos(146, 36);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("XP: %lu", _npcBuf[npcId]._xp);
	setTextPos(227, 36);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Speed: %d", _npcBuf[npcId]._speed);
	setTextPos(146, 45);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Defense: %d", getEquipmentDefense(npcId, false));
	setTextPos(146, 54);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Hit Points: %d", _npcBuf[npcId]._hitPoints);
	setTextPos(146, 63);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Max HP: %d", _npcBuf[npcId]._maxHP);
	setTextPos(227, 63);
	displayStringAtTextPos(buffer1);
	displayCenteredString("Inventory", 144, 310, 72);

	if (_menuItemCounter == 0) {
		if (curMenuLine != -1)
			setTextColorWhite();

		displayCenteredString("Nothing Carried", 144, 310, 117);
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
			if (_npcBuf[npcId]._inventory[_menuStatItemArr[counter]]._stat1 & 0x80) {
				setTextPos(146, textPosY);
				displayCharAtTextPos('E');
			}
		}

		setTextPos(152, textPosY);
		if (counter == curMenuLine) {
			buffer1 = Common::String::format("%c>", 'A' + counter);
		} else {
			buffer1 = Common::String::format("%c)", 'A' + counter);
		}
		displayStringAtTextPos(buffer1);

		if (itemId != 0x7FFF) {
			setTextPos(168, textPosY);
			buffer1 = Common::String::format("  %s", _items[itemId]._name);
			displayStringAtTextPos(buffer1);
			setTextPos(262, textPosY);

			if (_items[itemId]._defense > 0) {
				int16 stat2 = _npcBuf[npcId]._inventory[_menuStatItemArr[counter]]._stat2;
				if (stat2 != 0xFF) {
					buffer1 = Common::String::format("%d", 1 + stat2 / 8);
					displayStringAtTextPos(buffer1);
					setTextPos(286, textPosY);
					displayStringAtTextPos("Def");
				}
				// useless code removed.
				// else {
				//	var54 = _items[_npcBuf[npcId]._inventory[_menuStatItemArr[counter]]._ref]._defense;
				// {
			} else if (_items[itemId]._uses != 0x7F) {
				int16 stat1 = _npcBuf[npcId]._inventory[_menuStatItemArr[counter]]._stat1;
				if (stat1 != 0x7F) {
					buffer1 = Common::String::format("%d", stat1);
					displayStringAtTextPos(buffer1);
					setTextPos(286, textPosY);
					if (stat1 == 1)
						displayStringAtTextPos("Use");
					else
						displayStringAtTextPos("Uses");
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
	setTextPos(146, 27);
	displayStringAtTextPos("Name: ");
	displayStringAtTextPos(buffer);
	if (_menuItemCounter <= 0) {
		if (curMenuLine != -1)
			setTextColorWhite();
		displayCenteredString("No Skills To Select", 144, 310, 96);
		setTextColorRed();
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
		setTextPos(163, textPosY);
		displayStringAtTextPos(kSkillArray[_menuStatItemArr[counter]]);
		buffer = Common::String::format("%d", _npcBuf[charId]._activeScore[_menuStatItemArr[counter]]);
		setTextPos(278, textPosY);
		displayStringAtTextPos(buffer);
		setTextColorRed();
	}
}

void EfhEngine::displayStatusMenuActions(int16 menuId, int16 curMenuLine, int16 npcId) {
	debugC(6, kDebugEngine, "displayStatusMenuActions %d %d %d", menuId, curMenuLine, npcId);

	drawColoredRect(144, 15, 310, 184, 0);
	displayCenteredString("(ESCape Aborts)", 144, 310, 175);
	_textColor = 0x0E;
	switch (menuId) {
	case 0:
		displayCenteredString("Select Item to Equip", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case 1:
		displayCenteredString("Select Item to Use", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case 2:
		displayCenteredString("Select Item to Give", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case 3:
		displayCenteredString("Select Item to Trade", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case 4:
		displayCenteredString("Select Item to Drop", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case 5:
		displayCenteredString("Character Information", 144, 310, 15);
		displayCharacterInformationOrSkills(curMenuLine, npcId);
		break;
	case 6:
		displayCenteredString("Passive Skills", 144, 310, 15);
		displayCharacterInformationOrSkills(curMenuLine, npcId);
		break;
	case 7:
		displayCenteredString("Active Skills", 144, 310, 15);
		displayCharacterInformationOrSkills(curMenuLine, npcId);
		break;
	case 8:
	case 9:
		displayCenteredString("Character Summary", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	default:
		break;
	}
}

void EfhEngine::prepareStatusMenu(int16 windowId, int16 menuId, int16 curMenuLine, int16 charId, bool unusedFl, bool refreshFl) {
	debugC(6, kDebugEngine, "prepareStatusMenu %d %d %d %d %s", windowId, menuId, curMenuLine, charId, refreshFl ? "True" : "False");

	displayStatusMenu(windowId);

	prepareStatusRightWindowIndexes(menuId, charId);
	displayStatusMenuActions(menuId, curMenuLine, charId);

	if (refreshFl)
		displayFctFullScreen();
}

void EfhEngine::sub18E80(int16 charId, int16 windowId, int16 menuId, int16 curMenuLine) {
	debug("sub18E80 %d %d %d %d", charId, windowId, menuId, curMenuLine);

	for (int counter = 0; counter < 2; ++counter) {
		displayWindow(_menuBuf, 0, 0, _hiResImageBuf);
		prepareStatusMenu(windowId, menuId, curMenuLine, charId, true, false);

		if (counter == 0)
			displayFctFullScreen();
	}
}

} // End of namespace Efh

