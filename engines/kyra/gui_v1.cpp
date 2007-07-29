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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v1.h"
#include "kyra/screen.h"
#include "kyra/script.h"
#include "kyra/text.h"
#include "kyra/animator_v1.h"
#include "kyra/sound.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/events.h"
#include "common/system.h"

namespace Kyra {

void KyraEngine_v1::registerDefaultSettings() {
	// Most settings already have sensible defaults. This one, however, is
	// specific to the Kyra engine.
	ConfMan.registerDefault("walkspeed", 2);
	ConfMan.registerDefault("cdaudio", _flags.platform == Common::kPlatformFMTowns);
}

void KyraEngine_v1::readSettings() {
	int talkspeed = ConfMan.getInt("talkspeed");

	// The default talk speed is 60. This should be mapped to "Normal".

	if (talkspeed == 0)
		_configTextspeed = 3;	// Clickable
	if (talkspeed <= 50)
		_configTextspeed = 0;	// Slow
	else if (talkspeed <= 150)
		_configTextspeed = 1;	// Normal
	else
		_configTextspeed = 2;	// Fast

	_configWalkspeed = ConfMan.getInt("walkspeed");
	_configMusic = ConfMan.getBool("music_mute") ? 0 : ((ConfMan.getBool("cdaudio") && _flags.platform == Common::kPlatformFMTowns) ? 2 : 1);	
	_configSounds = ConfMan.getBool("sfx_mute") ? 0 : 1;

	_sound->enableMusic(_configMusic);
	_sound->enableSFX(_configSounds);

	bool speechMute = ConfMan.getBool("speech_mute");
	bool subtitles = ConfMan.getBool("subtitles");

	if (!speechMute && subtitles)
		_configVoice = 2;	// Voice & Text
	else if (!speechMute && !subtitles)
		_configVoice = 1;	// Voice only
	else
		_configVoice = 0;	// Text only

	setWalkspeed(_configWalkspeed);
}

void KyraEngine_v1::writeSettings() {
	bool speechMute, subtitles;
	int talkspeed;

	switch (_configTextspeed) {
	case 0:		// Slow
		talkspeed = 1;
		break;
	case 1:		// Normal
		talkspeed = 60;
		break;
	case 2:		// Fast
		talkspeed = 255;
		break;
	default:	// Clickable
		talkspeed = 0;
		break;
	}

	ConfMan.setInt("talkspeed", talkspeed);
	ConfMan.setInt("walkspeed", _configWalkspeed);
	ConfMan.setBool("music_mute", _configMusic == 0);
	ConfMan.setBool("cdaudio", _configMusic == 2);
	ConfMan.setBool("sfx_mute", _configSounds == 0);

	switch (_configVoice) {
	case 0:		// Text only
		speechMute = true;
		subtitles = true;
		break;
	case 1:		// Voice only
		speechMute = false;
		subtitles = false;
		break;
	default:	// Voice & Text
		speechMute = false;
		subtitles = true;
		break;
	}

	if (!_configMusic)
		_sound->beginFadeOut();

	_sound->enableMusic(_configMusic);
	_sound->enableSFX(_configSounds);

	ConfMan.setBool("speech_mute", speechMute);
	ConfMan.setBool("subtitles", subtitles);

	ConfMan.flushToDisk();
}

void KyraEngine_v1::initMainButtonList() {
	_haveScrollButtons = false;
	_buttonList = &_buttonData[0];
	for (int i = 0; _buttonDataListPtr[i]; ++i)
		_buttonList = initButton(_buttonList, _buttonDataListPtr[i]);
}

Button *KyraEngine_v1::initButton(Button *list, Button *newButton) {
	if (!newButton)
		return list;
	if (!list)
		return newButton;
	Button *cur = list;

	while (true) {
		if (!cur->nextButton)
			break;
		cur = cur->nextButton;
	}

	cur->nextButton = newButton;
	return list;
}

int KyraEngine_v1::buttonInventoryCallback(Button *caller) {
	int itemOffset = caller->specialValue - 2;
	uint8 inventoryItem = _currentCharacter->inventoryItems[itemOffset];
	if (_itemInHand == -1) {
		if (inventoryItem == 0xFF) {
			snd_playSoundEffect(0x36);
			return 0;
		} else {
			_screen->hideMouse();
			_screen->fillRect(_itemPosX[itemOffset], _itemPosY[itemOffset], _itemPosX[itemOffset] + 15, _itemPosY[itemOffset] + 15, 12);
			snd_playSoundEffect(0x35);
			setMouseItem(inventoryItem);
			updateSentenceCommand(_itemList[inventoryItem], _takenList[0], 179);
			_itemInHand = inventoryItem;
			_screen->showMouse();
			_currentCharacter->inventoryItems[itemOffset] = 0xFF;
		}
	} else {
		if (inventoryItem != 0xFF) {
			snd_playSoundEffect(0x35);
			_screen->hideMouse();
			_screen->fillRect(_itemPosX[itemOffset], _itemPosY[itemOffset], _itemPosX[itemOffset] + 15, _itemPosY[itemOffset] + 15, 12);
			_screen->drawShape(0, _shapes[216+_itemInHand], _itemPosX[itemOffset], _itemPosY[itemOffset], 0, 0);
			setMouseItem(inventoryItem);
			updateSentenceCommand(_itemList[inventoryItem], _takenList[1], 179);
			_screen->showMouse();
			_currentCharacter->inventoryItems[itemOffset] = _itemInHand;
			_itemInHand = inventoryItem;
		} else {
			snd_playSoundEffect(0x32);
			_screen->hideMouse();
			_screen->drawShape(0, _shapes[216+_itemInHand], _itemPosX[itemOffset], _itemPosY[itemOffset], 0, 0);
			_screen->setMouseCursor(1, 1, _shapes[0]);
			updateSentenceCommand(_itemList[_itemInHand], _placedList[0], 179);
			_screen->showMouse();
			_currentCharacter->inventoryItems[itemOffset] = _itemInHand;
			_itemInHand = -1;
		}
	}
	_screen->updateScreen();
	// XXX clearKyrandiaButtonIO
	return 0;
}

int KyraEngine_v1::buttonAmuletCallback(Button *caller) {
	if (!(_deathHandler & 8))
		return 1;
	int jewel = caller->specialValue - 0x14;
	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return 1;
	}
	if (!queryGameFlag(0x2D))
		return 1;
	if (_itemInHand != -1) {
		assert(_putDownFirst);
		characterSays(2000, _putDownFirst[0], 0, -2);
		return 1;
	}
	if (queryGameFlag(0xF1)) {
		assert(_waitForAmulet);
		characterSays(2001, _waitForAmulet[0], 0, -2);	
		return 1;
	}
	if (!queryGameFlag(0x55+jewel)) {
		assert(_blackJewel);
		_animator->makeBrandonFaceMouse();
		drawJewelPress(jewel, 1);
		characterSays(2002, _blackJewel[0], 0, -2);
		return 1;
	}
	drawJewelPress(jewel, 0);
	drawJewelsFadeOutStart();
	drawJewelsFadeOutEnd(jewel);
	
	_scriptInterpreter->initScript(_scriptClick, _scriptClickData);
	_scriptClick->regs[3] = 0;
	_scriptClick->regs[6] = jewel;
	_scriptInterpreter->startScript(_scriptClick, 4);
	
	while (_scriptInterpreter->validScript(_scriptClick))
		_scriptInterpreter->runScript(_scriptClick);
	
	if (_scriptClick->regs[3])
		return 1;
	
	_unkAmuletVar = 1;
	switch (jewel-1) {
	case 0:
		if (_brandonStatusBit & 1) {
			seq_brandonHealing2();
		} else if (_brandonStatusBit == 0) {
			seq_brandonHealing();
			assert(_healingTip);
			characterSays(2003, _healingTip[0], 0, -2);
		}
		break;
		
	case 1:
		seq_makeBrandonInv();
		break;

	case 2:
		if (_brandonStatusBit & 1) {
			assert(_wispJewelStrings);
			characterSays(2004, _wispJewelStrings[0], 0, -2);
		} else {
			if (_brandonStatusBit & 2) {
				// XXX
				seq_makeBrandonNormal2();
				// XXX
			} else {
				// do not check for item in hand again as in the original since some strings are missing
				// in the cd version
				if (_currentCharacter->sceneId >= 109 && _currentCharacter->sceneId <= 198) {
					snd_playWanderScoreViaMap(1, 0);
					seq_makeBrandonWisp();
					snd_playWanderScoreViaMap(17, 0);
				} else {
					seq_makeBrandonWisp();
				}
				setGameFlag(0x9E);
			}
		}
		break;

	case 3:
		seq_dispelMagicAnimation();
		assert(_magicJewelString);
		characterSays(2007, _magicJewelString[0], 0, -2);
		break;
		
	default:
		break;
	}
	_unkAmuletVar = 0;
	// XXX clearKyrandiaButtonIO (!used before every return in this function!)
	return 1;
}

void KyraEngine_v1::processButtonList(Button *list) {
	if (_haveScrollButtons) {
		if (_mouseWheel < 0)
			gui_scrollUp(&_scrollUpButton);
		else if (_mouseWheel > 0)
			gui_scrollDown(&_scrollDownButton);
	}
	while (list) {
		if (list->flags & 8) {
			list = list->nextButton;
			continue;
		}
		
		int x = list->x;
		int y = list->y;
		assert(list->dimTableIndex < _screen->_screenDimTableCount);
		if (x < 0) {
			x += _screen->_screenDimTable[list->dimTableIndex].w << 3;
		}
		x += _screen->_screenDimTable[list->dimTableIndex].sx << 3;
		
		if (y < 0) {
			y += _screen->_screenDimTable[list->dimTableIndex].h;
		}
		y += _screen->_screenDimTable[list->dimTableIndex].sy;
		
		Common::Point mouse = getMousePos();
		if (mouse.x >= x && mouse.y >= y && x + list->width >= mouse.x && y + list->height >= mouse.y) {
			int processMouseClick = 0;
			if (list->flags & 0x400) {
				if (_mousePressFlag) {
					if (!(list->flags2 & 1)) {
						list->flags2 |= 1;
						list->flags2 |= 4;
						processButton(list);
						_screen->updateScreen();
					}
				} else {
					if (list->flags2 & 1) {
						list->flags2 &= 0xFFFE;
						processButton(list);
						processMouseClick = 1;
					}
				}
			} else if (_mousePressFlag) {
				processMouseClick = 1;
			}
				
			if (processMouseClick) {
				if (list->buttonCallback) {
					if ((this->*(list->buttonCallback))(list)) {
						break;
					}
				}
			}
		} else {
			if (list->flags2 & 1) {
				list->flags2 &= 0xFFFE;
				processButton(list);
			}
			if (list->flags2 & 4) {
				list->flags2 &= 0xFFFB;
				processButton(list);
				_screen->updateScreen();
			}
			list = list->nextButton;
			continue;
		}
		
		list = list->nextButton;
	}
}

void KyraEngine_v1::processButton(Button *button) {
	if (!button)
		return;
	
	int processType = 0;
	uint8 *shape = 0;
	Button::ButtonCallback callback = 0;
	
	int flags = (button->flags2 & 5);
	if (flags == 1) {
		processType = button->process2;
		if (processType == 1)
			shape = button->process2PtrShape;
		else if (processType == 4)
			callback = button->process2PtrCallback;
	} else if (flags == 4 || flags == 5) {
		processType = button->process1;
		if (processType == 1)
			shape = button->process1PtrShape;
		else if (processType == 4)
			callback = button->process1PtrCallback;
	} else {
		processType = button->process0;
		if (processType == 1)
			shape = button->process0PtrShape;
		else if (processType == 4)
			callback = button->process0PtrCallback;
	}
	
	int x = button->x;
	int y = button->y;
	assert(button->dimTableIndex < _screen->_screenDimTableCount);
	if (x < 0)
		x += _screen->_screenDimTable[button->dimTableIndex].w << 3;
	
	if (y < 0)
		y += _screen->_screenDimTable[button->dimTableIndex].h;
	
	if (processType == 1 && shape)
		_screen->drawShape(_screen->_curPage, shape, x, y, button->dimTableIndex, 0x10);
	else if (processType == 4 && callback)
		(this->*callback)(button);
}

void KyraEngine_v1::processAllMenuButtons() {
	if (!_menuButtonList)
		return;

	Button *cur = _menuButtonList;
	while (true) {
		if (!cur->nextButton)
			break;
		processMenuButton(cur);
		cur = cur->nextButton;
	}
	return;
}

void KyraEngine_v1::processMenuButton(Button *button) {
	if (!_displayMenu)
		return;

	if (!button || (button->flags & 8))
		return;

	if (button->flags2 & 1)
		button->flags2 &= 0xf7;
	else
		button->flags2 |= 8;

	button->flags2 &= 0xfc;

	if (button->flags2 & 4)
		button->flags2 |= 0x10;
	else
		button->flags2 &= 0xef;

	button->flags2 &= 0xfb;

	processButton(button);
}

int KyraEngine_v1::drawBoxCallback(Button *button) {
	if (!_displayMenu)
		return 0;

	_screen->hideMouse();
	_screen->drawBox(button->x + 1, button->y + 1, button->x + button->width - 1, button->y + button->height - 1, 0xf8);
	_screen->showMouse();

	return 0;
}

int KyraEngine_v1::drawShadedBoxCallback(Button *button) {
	if (!_displayMenu)
		return 0;
	
	_screen->hideMouse();
	_screen->drawShadedBox(button->x, button->y, button->x + button->width, button->y + button->height, 0xf9, 0xfa);
	_screen->showMouse();

	return 0;
}

void KyraEngine_v1::setGUILabels() {
	int offset = 0;
	int offsetOptions = 0;
	int offsetMainMenu = 0;
	int offsetOn = 0;

	int walkspeedGarbageOffset = 36;
	int menuLabelGarbageOffset = 0;
	
	if (_flags.isTalkie) {
		if (_flags.lang == Common::EN_ANY)
			offset = 52;
		else if (_flags.lang == Common::DE_DEU)
			offset = 30;
		else if (_flags.lang == Common::FR_FRA || _flags.lang == Common::IT_ITA)
			offset = 6;
		offsetOn = offsetMainMenu = offsetOptions = offset;
		walkspeedGarbageOffset = 48;
	} else if (_flags.lang == Common::ES_ESP) {
		offsetOn = offsetMainMenu = offsetOptions = offset = -4;
		menuLabelGarbageOffset = 72;
	} else if (_flags.lang == Common::DE_DEU) {
		offset = offsetMainMenu = offsetOn = offsetOptions = 24;
	} else if (_flags.platform == Common::kPlatformFMTowns) {
		offset = 1;
		offsetOptions = 10;
		offsetOn = 0;
		walkspeedGarbageOffset = 0;
	}

	assert(offset + 27 < _guiStringsSize);
		
	// The Legend of Kyrandia
	_menu[0].menuName = _guiStrings[0];
	// Load a Game
	_menu[0].item[0].itemString = _guiStrings[1];
	// Save a Game
	_menu[0].item[1].itemString = _guiStrings[2];
	// Game controls
	_menu[0].item[2].itemString = _guiStrings[3];
	// Quit playing
	_menu[0].item[3].itemString = _guiStrings[4];
	// Resume game
	_menu[0].item[4].itemString = _guiStrings[5];

	// Cancel
	_menu[2].item[5].itemString = _guiStrings[10];
	
	// Enter a description of your saved game:
	_menu[3].menuName = _guiStrings[11];
	// Save
	_menu[3].item[0].itemString = _guiStrings[12];
	// Cancel
	_menu[3].item[1].itemString = _guiStrings[10];

	// Rest in peace, Brandon
	_menu[4].menuName = _guiStrings[13];
	// Load a game
	_menu[4].item[0].itemString = _guiStrings[1];
	// Quit playing
	_menu[4].item[1].itemString = _guiStrings[4];
	
	// Game Controls
	_menu[5].menuName = _guiStrings[6];
	// Yes
	_menu[1].item[0].itemString = _guiStrings[22 + offset];
	// No
	_menu[1].item[1].itemString = _guiStrings[23 + offset];
		
	// Music is
	_menu[5].item[0].labelString = _guiStrings[26 + offsetOptions];
	// Sounds are
	_menu[5].item[1].labelString = _guiStrings[27 + offsetOptions];
	// Walk speed
	_menu[5].item[2].labelString = &_guiStrings[24 + offsetOptions][walkspeedGarbageOffset];
	// Text speed
	_menu[5].item[4].labelString = _guiStrings[25 + offsetOptions];
	// Main Menu
	_menu[5].item[5].itemString = &_guiStrings[19 + offsetMainMenu][menuLabelGarbageOffset];
	
	if (_flags.isTalkie)
		// Text & Voice
		_voiceTextString = _guiStrings[28 + offset];

	_textSpeedString = _guiStrings[25 + offsetOptions];
	_onString =  _guiStrings[20 + offsetOn];
	_offString =  _guiStrings[21 + offset];
	_onCDString = _guiStrings[21];
}

int KyraEngine_v1::buttonMenuCallback(Button *caller) {
	_displayMenu = true;

	assert(_guiStrings);
	assert(_configStrings);
	
	/*
	for (int i = 0; i < _guiStringsSize; i++)
		debug("GUI string %i: %s", i, _guiStrings[i]);

	for (int i = 0; i < _configStringsSize; i++)
		debug("Config string %i: %s", i, _configStrings[i]);
	*/
	
	setGUILabels();
	if (_currentCharacter->sceneId == 210 && _deathHandler == 0xFF) {
		snd_playSoundEffect(0x36);
		return 0;
	}
	// XXX
	_screen->setPaletteIndex(0xFE, 60, 60, 0);
	for (int i = 0; i < 6; i++) {
		_menuButtonData[i].process0 = _menuButtonData[i].process1 = _menuButtonData[i].process2 = 4;
		_menuButtonData[i].process0PtrCallback = &KyraEngine_v1::drawShadedBoxCallback;
		_menuButtonData[i].process1PtrCallback = &KyraEngine_v1::drawBoxCallback;
		_menuButtonData[i].process2PtrCallback = &KyraEngine_v1::drawShadedBoxCallback;
	}

	_screen->savePageToDisk("SEENPAGE.TMP", 0);
	gui_fadePalette();

	for (int i = 0; i < 5; i++)
		calcCoords(_menu[i]);

	_menuRestoreScreen = true;
	_keyPressed.reset();
	_mousePressFlag = false;
	
	_toplevelMenu = 0;
	if (_menuDirectlyToLoad) {
		gui_loadGameMenu(0);
	} else {
		if (!caller)
			_toplevelMenu = 4;

		initMenu(_menu[_toplevelMenu]);
		processAllMenuButtons();
	}

	while (_displayMenu && !_quitFlag) {
		gui_processHighlights(_menu[_toplevelMenu]);
		processButtonList(_menuButtonList);
		gui_getInput();
	}

	if (_menuRestoreScreen) {
		gui_restorePalette();
		_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
		_animator->_updateScreen = true;
	} else {
		_screen->deletePageFromDisk(0);
	}

	return 0;
}

void KyraEngine_v1::initMenu(Menu &menu) {
	_menuButtonList = 0;

	_screen->hideMouse();

	int textX;
	int textY;

	int menu_x2 = menu.width  + menu.x - 1;
	int menu_y2 = menu.height + menu.y - 1;

	_screen->fillRect(menu.x + 2, menu.y + 2, menu_x2 - 2, menu_y2 - 2, menu.bgcolor);
	_screen->drawShadedBox(menu.x, menu.y, menu_x2, menu_y2, menu.color1, menu.color2);

	if (menu.field_10 != -1)
		textX = menu.x;
	else
		textX = _text->getCenterStringX(menu.menuName, menu.x, menu_x2);

	textY = menu.y + menu.field_12;

	_text->printText(menu.menuName, textX - 1, textY + 1, 12, 248, 0);
	_text->printText(menu.menuName, textX, textY, menu.textColor, 0, 0);

	int x1, y1, x2, y2;	
	for (int i = 0; i < menu.nrOfItems; i++) {
		if (!menu.item[i].enabled)
			continue;

		x1 = menu.x + menu.item[i].x;
		y1 = menu.y + menu.item[i].y;

		x2 = x1 + menu.item[i].width - 1;
		y2 = y1 + menu.item[i].height - 1;

		if (i < 6) {
			_menuButtonData[i].nextButton = 0;
			_menuButtonData[i].x = x1;
			_menuButtonData[i].y = y1;
			_menuButtonData[i].width  = menu.item[i].width  - 1;
			_menuButtonData[i].height = menu.item[i].height - 1;
			_menuButtonData[i].buttonCallback = menu.item[i].callback;
			_menuButtonData[i].specialValue = menu.item[i].field_1b;
			//_menuButtonData[i].field_6 = menu.item[i].field_25;
			//_menuButtonData[i].field_8 = 0;

			if (!_menuButtonList)
				_menuButtonList = &_menuButtonData[i];
			else
				_menuButtonList = initButton(_menuButtonList, &_menuButtonData[i]);
		}
		_screen->fillRect(x1, y1, x2, y2, menu.item[i].bgcolor);
		_screen->drawShadedBox(x1, y1, x2, y2, menu.item[i].color1, menu.item[i].color2);

		if (menu.item[i].itemString) {
			if (menu.item[i].field_12 != -1 && _flags.lang == Common::EN_ANY)
				textX = x1 + menu.item[i].field_12 + 3;
			else
				textX = _text->getCenterStringX(menu.item[i].itemString, x1, x2);

			textY = y1 + 2;
			_text->printText(menu.item[i].itemString, textX - 1, textY + 1,  12, 0, 0);

			if (i == menu.highlightedItem)
				_text->printText(menu.item[i].itemString, textX, textY, menu.item[i].highlightColor, 0, 0);
			else
				_text->printText(menu.item[i].itemString, textX, textY, menu.item[i].textColor, 0, 0);

			if (menu.item[i].labelString) {
				_text->printText(menu.item[i].labelString, menu.x + menu.item[i].labelX - 1, menu.y + menu.item[i].labelY + 1, 12, 0, 0);
				_text->printText(menu.item[i].labelString, menu.x + menu.item[i].labelX, menu.y + menu.item[i].labelY, 253, 0, 0);
			}
		}
	}

	if (menu.scrollUpBtnX != -1) {
		_haveScrollButtons = true;

		_scrollUpButton.x = menu.scrollUpBtnX + menu.x;
		_scrollUpButton.y = menu.scrollUpBtnY + menu.y;
		_scrollUpButton.buttonCallback = &KyraEngine_v1::gui_scrollUp;
		_scrollUpButton.nextButton = 0;
		_menuButtonList = initButton(_menuButtonList, &_scrollUpButton);
		processMenuButton(&_scrollUpButton);
		
		_scrollDownButton.x = menu.scrollDownBtnX + menu.x;
		_scrollDownButton.y = menu.scrollDownBtnY + menu.y;
		_scrollDownButton.buttonCallback = &KyraEngine_v1::gui_scrollDown;
		_scrollDownButton.nextButton = 0;
		_menuButtonList = initButton(_menuButtonList, &_scrollDownButton);
		processMenuButton(&_scrollDownButton);
	} else {
		_haveScrollButtons = false;
	}

	_screen->showMouse();
	_screen->updateScreen();
}

void KyraEngine_v1::calcCoords(Menu &menu) {
	assert(menu.nrOfItems < 7);

	int widthBackup = _screen->_charWidth;
	_screen->_charWidth = -2;
	
	menu.x = (320 - menu.width)/2;

	int menu_x2 = menu.width  + menu.x - 1;	
	int maxOffset = 0;
	int x1, x2, y1, y2;	

	for (int i = 0; i < menu.nrOfItems; i++) {
		if (menu.item[i].x == -1)
			menu.item[i].x = (menu.width - menu.item[i].width)/2;

		if (menu.item[i].labelString) {
			x1 = menu.x + menu.item[i].x + 25;
			y1 = (200 - menu.height)/2 + menu.item[i].y;

			x2 = x1 + menu.item[i].width;
			y2 = y1 + menu.item[i].height;

			int textWidth = _screen->getTextWidth(menu.item[i].labelString) + 25;
			int textX = menu.item[i].labelX + menu.x;

			if (textWidth + textX > x1) {
				int offset = ((textWidth + textX) - x1);
				if (maxOffset < offset)
					maxOffset = offset;
			}
		}
		
		if (menu.item[i].itemString) {
			int textWidth = _screen->getTextWidth(menu.item[i].itemString) + 15;

			if (menu.item[i].width < textWidth) {
				menu.item[i].width = textWidth;
				
				if ( menu.x + menu.item[i].x + menu.item[i].width > menu_x2)
					menu.item[i].x -= (menu.x + menu.item[i].x + menu.item[i].width) - menu_x2 + 10;
			}
		}	
		
	}

	if (maxOffset > 0) {
		maxOffset = maxOffset/2;
		for (int i = 0; i < menu.nrOfItems; i++) {
			menu.item[i].x += maxOffset + 10;
			menu.item[i].labelX -= maxOffset;
		}
		menu.width += maxOffset;
	}
	
	if (menu.menuName != 0) {
		int menuNameLength = _screen->getTextWidth(menu.menuName);
		if (menuNameLength  > menu.width)
			menu.width = menuNameLength;
	}

	if (menu.width > 310)
		menu.width = 310;

	menu.x = (320 - menu.width)/2;
			
	if (menu.y == -1)
		menu.y = (200 - menu.height)/2;
		
	_screen->_charWidth = widthBackup;
}

void KyraEngine_v1::gui_getInput() {
	Common::Event event;
	static uint32 lastScreenUpdate = 0;
	uint32 now = _system->getMillis();

	_mouseWheel = 0;
	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
			quitGame();
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mousePressFlag = true;
			break;
		case Common::EVENT_LBUTTONUP:
			_mousePressFlag = false;
			break;
		case Common::EVENT_MOUSEMOVE:
			_system->updateScreen();
			lastScreenUpdate = now;
			break;
		case Common::EVENT_WHEELUP:
			_mouseWheel = -1;
			break;
		case Common::EVENT_WHEELDOWN:
			_mouseWheel = 1;
			break;
		case Common::EVENT_KEYDOWN:
			_keyPressed = event.kbd;
			break;
		default:
			break;
		}
	}

	if (now - lastScreenUpdate > 50) {
		_system->updateScreen();
		lastScreenUpdate = now;
	}

	_system->delayMillis(3);
}

int KyraEngine_v1::gui_resumeGame(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_resumeGame()");
	processMenuButton(button);
	_displayMenu = false;

	return 0;
}

const char *KyraEngine_v1::getSavegameFilename(int num) {
	static char saveLoadSlot[12];

	sprintf(saveLoadSlot, "%s.%.3d", _targetName.c_str(), num);
	return saveLoadSlot;
}

int KyraEngine_v1::getNextSavegameSlot() {
	Common::InSaveFile *in;

	for (int i = 1; i < 1000; i++) {
		if ((in = _saveFileMan->openForLoading(getSavegameFilename(i))))
			delete in;
		else
			return i;
	}
	warning("Didn't save: Ran out of savegame filenames");
	return 0;
}

void KyraEngine_v1::setupSavegames(Menu &menu, int num) {
	Common::InSaveFile *in;
	static char savenames[5][31];
	uint8 startSlot;
	assert(num <= 5);

	if (_savegameOffset == 0) {
		menu.item[0].itemString = _specialSavegameString;
		menu.item[0].enabled = 1;
		menu.item[0].field_1b = 0;
		startSlot = 1;
	} else {
		startSlot = 0;
	}

	for (int i = startSlot; i < num; i++) {
		if ((in = _saveFileMan->openForLoading(getSavegameFilename(i + _savegameOffset)))) {
			in->skip(8);
			in->read(savenames[i], 31);
			menu.item[i].itemString = savenames[i];
			menu.item[i].enabled = 1;
			menu.item[i].field_1b = i + _savegameOffset;
			delete in;
		} else {
			menu.item[i].enabled = 0;
			//menu.item[i].itemString = "";
			//menu.item[i].field_1b = -1;
		}
	}
}

int KyraEngine_v1::gui_saveGameMenu(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_saveGameMenu()");
	processMenuButton(button);
	_menu[2].item[5].enabled = true;

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	_menu[2].menuName = _guiStrings[8]; // Select a position to save to:
	_specialSavegameString = _guiStrings[9]; // [ EMPTY SLOT ]
	for (int i = 0; i < 5; i++)
		_menu[2].item[i].callback = &KyraEngine_v1::gui_saveGame;

	_savegameOffset = 0;
	setupSavegames(_menu[2], 5);

	initMenu(_menu[2]);
	processAllMenuButtons();

	_displaySubMenu = true;
	_cancelSubMenu = false;

	while (_displaySubMenu && !_quitFlag) {
		gui_getInput();
		gui_processHighlights(_menu[2]);
		processButtonList(_menuButtonList);
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	if (_cancelSubMenu) {
		initMenu(_menu[0]);
		processAllMenuButtons();
	} else {
		_displayMenu = false;
	}
	return 0;
}

int KyraEngine_v1::gui_loadGameMenu(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_loadGameMenu()");
	if (_menuDirectlyToLoad) {
		_menu[2].item[5].enabled = false;
	} else {
		processMenuButton(button);
		_menu[2].item[5].enabled = true;
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	_specialSavegameString = _newGameString[0]; //[ START A NEW GAME ]
	_menu[2].menuName = _guiStrings[7]; // Which game would you like to reload?
	for (int i = 0; i < 5; i++)
		_menu[2].item[i].callback = &KyraEngine_v1::gui_loadGame;

	_savegameOffset = 0;
	setupSavegames(_menu[2], 5);

	initMenu(_menu[2]);
	processAllMenuButtons();

	_displaySubMenu = true;
	_cancelSubMenu = false;

	while (_displaySubMenu && !_quitFlag) {
		gui_getInput();
		gui_processHighlights(_menu[2]);
		processButtonList(_menuButtonList);
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	if (_cancelSubMenu) {
		initMenu(_menu[_toplevelMenu]);
		processAllMenuButtons();
	} else {
		gui_restorePalette();
		loadGame(getSavegameFilename(_gameToLoad));
		_displayMenu = false;
		_menuRestoreScreen = false;
	}
	return 0;
}

void KyraEngine_v1::gui_redrawTextfield() {
	_screen->fillRect(38, 91, 287, 102, 250);
	_text->printText(_savegameName, 38, 92, 253, 0, 0);

	_screen->_charWidth = -2;
	int width = _screen->getTextWidth(_savegameName);
	_screen->fillRect(39 + width, 93, 45 + width, 100, 254);
	_screen->_charWidth = 0;

	_screen->updateScreen();
}

void KyraEngine_v1::gui_updateSavegameString() {
	int length;

	if (_keyPressed.keycode) {
		length = strlen(_savegameName);

		if (_keyPressed.ascii > 31 && _keyPressed.ascii < 127) {
			if (length < 31) {
				_savegameName[length] = _keyPressed.ascii;
				_savegameName[length+1] = 0;
				gui_redrawTextfield();
			}
		} else if (_keyPressed.keycode == Common::KEYCODE_BACKSPACE ||
		           _keyPressed.keycode == Common::KEYCODE_DELETE) {
			if (length > 0) {
				_savegameName[length-1] = 0;
				gui_redrawTextfield();
			}
		} else if (_keyPressed.keycode == Common::KEYCODE_RETURN ||
		           _keyPressed.keycode == Common::KEYCODE_KP_ENTER) {
			_displaySubMenu = false;
		}
	}

	_keyPressed.reset();
}

int KyraEngine_v1::gui_saveGame(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_saveGame()");
	processMenuButton(button);
	_gameToLoad = button->specialValue;

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	initMenu(_menu[3]);
	processAllMenuButtons();

	_displaySubMenu = true;
	_cancelSubMenu = false;

	if (_savegameOffset == 0 && _gameToLoad == 0) {
		_savegameName[0] = 0;
	} else {
		for (int i = 0; i < 5; i++) {
			if (_menu[2].item[i].field_1b == _gameToLoad) {
				strncpy(_savegameName, _menu[2].item[i].itemString, 31);
				break;
			}
		}
	}
	gui_redrawTextfield();

	while (_displaySubMenu && !_quitFlag) {
		gui_getInput();
		gui_updateSavegameString();
		gui_processHighlights(_menu[3]);
		processButtonList(_menuButtonList);
	}

	if (_cancelSubMenu) {
		_displaySubMenu = true;
		_cancelSubMenu = false;
		initMenu(_menu[2]);
		processAllMenuButtons();
	} else {
		if (_savegameOffset == 0 && _gameToLoad == 0)
			_gameToLoad = getNextSavegameSlot();
		if (_gameToLoad > 0)
			saveGame(getSavegameFilename(_gameToLoad), _savegameName);
	}

	return 0;
}

int KyraEngine_v1::gui_savegameConfirm(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_savegameConfirm()");
	processMenuButton(button);
	_displaySubMenu = false;

	return 0;
}

int KyraEngine_v1::gui_loadGame(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_loadGame()");
	processMenuButton(button);
	_displaySubMenu = false;
	_gameToLoad = button->specialValue;

	return 0;
}

int KyraEngine_v1::gui_cancelSubMenu(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_cancelLoadGameMenu()");
	processMenuButton(button);
	_displaySubMenu = false;
	_cancelSubMenu = true;

	return 0;
}

int KyraEngine_v1::gui_quitPlaying(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_quitPlaying()");
	processMenuButton(button);

	if (gui_quitConfirm(_guiStrings[14])) { // Are you sure you want to quit playing?
		quitGame();
	} else {
		initMenu(_menu[_toplevelMenu]);
		processAllMenuButtons();
	}

	return 0;
}

bool KyraEngine_v1::gui_quitConfirm(const char *str) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_quitConfirm()");

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	_menu[1].menuName = str;
	calcCoords(_menu[1]);
	initMenu(_menu[1]);
	
	_displaySubMenu = true;
	_cancelSubMenu = true;

	while (_displaySubMenu && !_quitFlag) {
		gui_getInput();
		gui_processHighlights(_menu[1]);
		processButtonList(_menuButtonList);
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	return !_cancelSubMenu;
}

int KyraEngine_v1::gui_quitConfirmYes(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_quitConfirmYes()");
	processMenuButton(button);
	_displaySubMenu = false;
	_cancelSubMenu = false;

	return 0;
}

int KyraEngine_v1::gui_quitConfirmNo(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_quitConfirmNo()");
	processMenuButton(button);
	_displaySubMenu = false;
	_cancelSubMenu = true;

	return 0;
}

int KyraEngine_v1::gui_gameControlsMenu(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_gameControlsMenu()");

	readSettings();

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	if (_flags.isTalkie) {
		//_menu[5].width = 230;

		for (int i = 0; i < 5; i++) {
			//_menu[5].item[i].labelX = 24;
			//_menu[5].item[i].x = 115;
			//_menu[5].item[i].width = 94;
		}

		_menu[5].item[3].labelString = _voiceTextString; //"Voice / Text "
		_menu[5].item[3].callback = &KyraEngine_v1::gui_controlsChangeVoice;

	} else {
		//_menu[5].height = 136;
		//_menu[5].item[5].y = 110;
		_menu[5].item[4].enabled = 0;
		_menu[5].item[3].labelString = _textSpeedString; // "Text speed "
		_menu[5].item[3].callback = &KyraEngine_v1::gui_controlsChangeText;
	}

	gui_setupControls(_menu[5]);

	processAllMenuButtons();

	_displaySubMenu = true;
	_cancelSubMenu = false;

	while (_displaySubMenu && !_quitFlag) {
		gui_getInput();
		gui_processHighlights(_menu[5]);
		processButtonList(_menuButtonList);
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	if (_cancelSubMenu) {
		initMenu(_menu[_toplevelMenu]);
		processAllMenuButtons();
	}
	return 0;
}

void KyraEngine_v1::gui_setupControls(Menu &menu) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_setupControls()");

	switch (_configMusic) {
		case 0:
			menu.item[0].itemString = _offString; //"Off"
			break;
		case 1:
			menu.item[0].itemString = _onString; //"On"
			break;
		case 2:
			menu.item[0].itemString = _onCDString; //"On + CD"
			break;
	}

	if (_configSounds)
		menu.item[1].itemString = _onString; //"On"
	else
		menu.item[1].itemString = _offString; //"Off"


	switch (_configWalkspeed) {
	case 0:
		menu.item[2].itemString = _configStrings[0]; //"Slowest"
		break;
	case 1:
		menu.item[2].itemString = _configStrings[1]; //"Slow"
		break;
	case 2:
		menu.item[2].itemString = _configStrings[2]; //"Normal"
		break;
	case 3:
		menu.item[2].itemString = _configStrings[3]; //"Fast"
		break;
	case 4:
		menu.item[2].itemString = _configStrings[4]; //"Fastest"
		break;
	default:
		menu.item[2].itemString = "ERROR";
		break;
	}

	int textControl = 3;
	int clickableOffset = 8;
	if (_flags.isTalkie) {
		textControl = 4;
		clickableOffset = 11;
		
		if (_configVoice == 0)
			_menu[5].item[4].enabled = 1;
		else
			_menu[5].item[4].enabled = 0;

		switch (_configVoice) {
		case 0:
			menu.item[3].itemString = _configStrings[5]; //"Text only"
			break;
		case 1:
			menu.item[3].itemString = _configStrings[6]; //"Voice only"
			break;
		case 2:
			menu.item[3].itemString = _configStrings[7]; //"Voice & Text"
			break;
		default:
			menu.item[3].itemString = "ERROR";
			break;
		}
	}

	switch (_configTextspeed) {
	case 0:
		menu.item[textControl].itemString = _configStrings[1]; //"Slow"
		break;
	case 1:
		menu.item[textControl].itemString = _configStrings[2]; //"Normal"
		break;
	case 2:
		menu.item[textControl].itemString = _configStrings[3]; //"Fast"
		break;
	case 3:
		menu.item[textControl].itemString = _configStrings[clickableOffset]; //"Clickable"
		break;
	default:
		menu.item[textControl].itemString = "ERROR";
		break;
	}

	calcCoords(menu);
	initMenu(menu);
}

int KyraEngine_v1::gui_controlsChangeMusic(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_controlsChangeMusic()");
	processMenuButton(button);

	_configMusic = ++_configMusic % (_flags.platform == Common::kPlatformFMTowns ? 3 : 2);
	gui_setupControls(_menu[5]);
	return 0;
}

int KyraEngine_v1::gui_controlsChangeSounds(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_controlsChangeSounds()");
	processMenuButton(button);

	_configSounds = !_configSounds;
	gui_setupControls(_menu[5]);
	return 0;
}

int KyraEngine_v1::gui_controlsChangeWalk(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_controlsChangeWalk()");
	processMenuButton(button);

	_configWalkspeed = ++_configWalkspeed % 5;
	setWalkspeed(_configWalkspeed);
	gui_setupControls(_menu[5]);
	return 0;
}

int KyraEngine_v1::gui_controlsChangeText(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_controlsChangeText()");
	processMenuButton(button);

	_configTextspeed = ++_configTextspeed % 4;
	gui_setupControls(_menu[5]);
	return 0;
}

int KyraEngine_v1::gui_controlsChangeVoice(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_controlsChangeVoice()");
	processMenuButton(button);

	_configVoice = ++_configVoice % 3;
	gui_setupControls(_menu[5]);
	return 0;
}

int KyraEngine_v1::gui_controlsApply(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_controlsApply()");
	writeSettings();
	return gui_cancelSubMenu(button);
}

int KyraEngine_v1::gui_scrollUp(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_scrollUp()");
	processMenuButton(button);

	if (_savegameOffset > 0) {
		_savegameOffset--;
		setupSavegames(_menu[2], 5);
		initMenu(_menu[2]);
	}
	return 0;
}

int KyraEngine_v1::gui_scrollDown(Button *button) {
	debugC(9, kDebugLevelGUI, "KyraEngine_v1::gui_scrollDown()");
	processMenuButton(button);

	_savegameOffset++;
	setupSavegames(_menu[2], 5);
	initMenu(_menu[2]);

	return 0;
}

void KyraEngine_v1::gui_processHighlights(Menu &menu) {
	int x1, y1, x2, y2;

	Common::Point mouse = getMousePos();
	for (int i = 0; i < menu.nrOfItems; i++) {
		if (!menu.item[i].enabled)
			continue;

		x1 = menu.x + menu.item[i].x;
		y1 = menu.y + menu.item[i].y;

		x2 = x1 + menu.item[i].width;
		y2 = y1 + menu.item[i].height;

		if (mouse.x > x1 && mouse.x < x2 &&
			mouse.y > y1 && mouse.y < y2) {
			
			if (menu.highlightedItem != i) {
				if (menu.item[menu.highlightedItem].enabled )
					gui_redrawText(menu);

				menu.highlightedItem = i;
				gui_redrawHighlight(menu);
				_screen->updateScreen();
			}
		}
	}
}

void KyraEngine_v1::gui_redrawText(Menu menu) {
	int textX;
	int i = menu.highlightedItem;

	int x1 = menu.x + menu.item[i].x;
	int y1 = menu.y + menu.item[i].y;

	int x2 = x1 + menu.item[i].width - 1;

	if (menu.item[i].field_12 != -1 &&_flags.lang == Common::EN_ANY)
		textX = x1 + menu.item[i].field_12 + 3;
	else
		textX = _text->getCenterStringX(menu.item[i].itemString, x1, x2);

	int textY = y1 + 2;
	_text->printText(menu.item[i].itemString, textX - 1, textY + 1,  12, 0, 0);
	_text->printText(menu.item[i].itemString, textX, textY, menu.item[i].textColor, 0, 0);
}

void KyraEngine_v1::gui_redrawHighlight(Menu menu) {
	int textX;
	int i = menu.highlightedItem;

	int x1 = menu.x + menu.item[i].x;
	int y1 = menu.y + menu.item[i].y;

	int x2 = x1 + menu.item[i].width - 1;

	if (menu.item[i].field_12 != -1 &&_flags.lang == Common::EN_ANY)
		textX = x1 + menu.item[i].field_12 + 3;
	else
		textX = _text->getCenterStringX(menu.item[i].itemString, x1, x2);

	int textY = y1 + 2;
	_text->printText(menu.item[i].itemString, textX - 1, textY + 1,  12, 0, 0);
	_text->printText(menu.item[i].itemString, textX, textY, menu.item[i].highlightColor, 0, 0);
}

void KyraEngine_v1::gui_fadePalette() {
	if (_flags.platform == Common::kPlatformAmiga)
		return;

	static int16 menuPalIndexes[] = {248, 249, 250, 251, 252, 253, 254, -1};
	int index = 0;

	memcpy(_screen->getPalette(2), _screen->_currentPalette, 768);

	for (int i = 0; i < 768; i++)
		_screen->_currentPalette[i] >>= 1;

	while (menuPalIndexes[index] != -1) {
		memcpy(&_screen->_currentPalette[menuPalIndexes[index]*3], &_screen->getPalette(2)[menuPalIndexes[index]*3], 3);
		index++;
	}

	_screen->fadePalette(_screen->_currentPalette, 2);
}

void KyraEngine_v1::gui_restorePalette() {
	if (_flags.platform == Common::kPlatformAmiga)
		return;

	memcpy(_screen->_currentPalette, _screen->getPalette(2), 768);
	_screen->fadePalette(_screen->_currentPalette, 2);
}

} // end of namespace Kyra
 
