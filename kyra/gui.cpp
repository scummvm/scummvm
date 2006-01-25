/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * $Header$
 *
 */

#include "kyra/kyra.h"
#include "kyra/screen.h"
#include "kyra/script.h"
#include "kyra/text.h"
#include "kyra/animator.h"

#include "common/savefile.h"
#include "common/system.h"

namespace Kyra {
Button *KyraEngine::initButton(Button *list, Button *newButton) {
	if (!newButton)
		return list;
	if (!list)
		return newButton;
	Button *cur = list;
	while (true) {
		if (!cur->nextButton) {
			break;
		}
		cur = cur->nextButton;
	}
	cur->nextButton = newButton;
	return list;
}

int KyraEngine::buttonInventoryCallback(Button *caller) {
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
			_screen->drawShape(0, _shapes[220+_itemInHand], _itemPosX[itemOffset], _itemPosY[itemOffset], 0, 0);
			setMouseItem(inventoryItem);
			updateSentenceCommand(_itemList[inventoryItem], _takenList[1], 179);
			_screen->showMouse();
			_currentCharacter->inventoryItems[itemOffset] = _itemInHand;
			_itemInHand = inventoryItem;
		} else {
			snd_playSoundEffect(0x32);
			_screen->hideMouse();
			_screen->drawShape(0, _shapes[220+_itemInHand], _itemPosX[itemOffset], _itemPosY[itemOffset], 0, 0);
			_screen->setMouseCursor(1, 1, _shapes[4]);
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

int KyraEngine::buttonAmuletCallback(Button *caller) {
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
		if (_features & GF_TALKIE) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(2000);
		}
		characterSays(_putDownFirst[0], 0, -2);
		return 1;
	}
	if (queryGameFlag(0xF1)) {
		assert(_waitForAmulet);
		if (_features & GF_TALKIE) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(2001);
		}
		characterSays(_waitForAmulet[0], 0, -2);
		return 1;
	}
	if (!queryGameFlag(0x55+jewel)) {
		assert(_blackJewel);
		makeBrandonFaceMouse();
		drawJewelPress(jewel, 1);
		if (_features & GF_TALKIE) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(2002);
		}
		characterSays(_blackJewel[0], 0, -2);
		return 1;
	}
	drawJewelPress(jewel, 0);
	drawJewelsFadeOutStart();
	drawJewelsFadeOutEnd(jewel);
	
	_scriptInterpreter->initScript(_scriptClick, _scriptClickData);
	_scriptClick->variables[3] = 0;
	_scriptClick->variables[6] = jewel;
	_scriptInterpreter->startScript(_scriptClick, 4);
	
	while (_scriptInterpreter->validScript(_scriptClick)) {
		_scriptInterpreter->runScript(_scriptClick);
	}
	
	if (_scriptClick->variables[3])
		return 1;
	
	_unkAmuletVar = 1;
	switch (jewel-1) {
		case 0:
			if (_brandonStatusBit & 1) {
				seq_brandonHealing2();
			} else if (_brandonStatusBit == 0) {
				seq_brandonHealing();
				assert(_healingTip);
				if (_features & GF_TALKIE) {
					snd_voiceWaitForFinish();
					snd_playVoiceFile(2003);
				}
				characterSays(_healingTip[0], 0, -2);
			}
			break;
		
		case 1:
			seq_makeBrandonInv();
			break;
		
		case 2:
			if (_brandonStatusBit & 1) {
				assert(_wispJewelStrings);
				if (_features & GF_TALKIE) {
					snd_voiceWaitForFinish();
					snd_playVoiceFile(2004);
				}
				characterSays(_wispJewelStrings[0], 0, -2);
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
			if (_features & GF_TALKIE) {
				snd_voiceWaitForFinish();
				snd_playVoiceFile(2007);
			}
			characterSays(_magicJewelString[0], 0, -2);
			break;
		
		default:
			break;
	}
	_unkAmuletVar = 0;
	// XXX clearKyrandiaButtonIO (!used before every return in this function!)
	return 1;
}

void KyraEngine::processButtonList(Button *list) {
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
		
		if (_mouseX >= x && _mouseY >= y && x + list->width >= _mouseX && y + list->height >= _mouseY) {
			int processMouseClick = 0;
			if (list->flags & 0x400) {
				if (_mousePressFlag) {
					if (!(list->flags2 & 1)) {
						list->flags2 |= 1;
						processButton(list);
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
			list = list->nextButton;
			continue;
		}
		
		list = list->nextButton;
	}
}

void KyraEngine::processButton(Button *button) {
	if (!button)
		return;
	
	int processType = 0;
	uint8 *shape = 0;
	Button::ButtonCallback callback = 0;
	
	int flags = (button->flags2 & 5);
	if (flags == 1) {
		processType = button->process2;
		if (processType == 1) {
			shape = button->process2PtrShape;
		} else if (processType == 4) {
			callback = button->process2PtrCallback;
		}
	} else if (flags == 4 || flags == 5) {
		processType = button->process1;
		if (processType == 1) {
			shape = button->process1PtrShape;
		} else if (processType == 4) {
			callback = button->process1PtrCallback;
		}
	} else {
		processType = button->process0;
		if (processType == 1) {
			shape = button->process0PtrShape;
		} else if (processType == 4) {
			callback = button->process0PtrCallback;
		}
	}
	
	int x = button->x;
	int y = button->y;
	assert(button->dimTableIndex < _screen->_screenDimTableCount);
	if (x < 0) {
		x += _screen->_screenDimTable[button->dimTableIndex].w << 3;
	}
	
	if (y < 0) {
		y += _screen->_screenDimTable[button->dimTableIndex].h;
	}
	
	if (processType == 1 && shape) {
		_screen->drawShape(_screen->_curPage, shape, x, y, button->dimTableIndex, 0x10);
	} else if (processType == 4 && callback) {
		(this->*callback)(button);
	}
}

void KyraEngine::processAllMenuButtons() {
	if (!_menuButtonList)
		return;

	Button *cur = _menuButtonList;
	while (true) {
		if (!cur->nextButton) {
			break;
		}
		processMenuButton(cur);
		cur = cur->nextButton;
	}
	return;
}

void KyraEngine::processMenuButton(Button *button) {
	if (!_displayMenu)
		return;

	//_screen->hideMouse();

	if ( !button || (button->flags & 8))
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

	//_screen->showMouse();
}

int KyraEngine::drawBoxCallback(Button *button) {
	if (!_displayMenu)
		return 0;

	_screen->hideMouse();
	_screen->drawBox(button->x + 1, button->y + 1, button->x + button->width - 1, button->y + button->height - 1, 0xf8);
	_screen->showMouse();

	return 0;
}

int KyraEngine::drawShadedBoxCallback(Button *button) {

	if (!_displayMenu)
		return 0;
	
	_screen->hideMouse();
	_screen->drawShadedBox(button->x, button->y, button->x + button->width, button->y + button->height, 0xf9, 0xfa);
	_screen->showMouse();

	return 0;
}

int KyraEngine::buttonMenuCallback(Button *caller) {
	_displayMenu = true;

	// XXX setLabels
	if (_currentCharacter->sceneId == 210 || caller == 0) {
		snd_playSoundEffect(0x36);
		return 0;
	}
	// XXX

	for (int i = 0; i < 6; i++) {
		_menuButtonData[i].process0 = _menuButtonData[i].process1 = _menuButtonData[i].process2 = 4;
		_menuButtonData[i].process0PtrCallback = &KyraEngine::drawShadedBoxCallback;
		_menuButtonData[i].process1PtrCallback = &KyraEngine::drawBoxCallback;
		_menuButtonData[i].process2PtrCallback = &KyraEngine::drawShadedBoxCallback;
	}

	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	calcCoords(_menu[0]);
	calcCoords(_menu[1]);
	calcCoords(_menu[2]);

	initMenu(_menu[0]);
	processAllMenuButtons();

	_menuRestoreScreen = true;
	while (_displayMenu) {
		gui_processHighlights(_menu[0]);
		processButtonList(_menuButtonList);
		gui_getInput();
	}

	if (_menuRestoreScreen) {
		_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
		_animator->_updateScreen = true;
	}
	else
		_screen->deletePageFromDisk(0);

	return 0;
}

void KyraEngine::initMenu(Menu menu) {
	int menu_x2 = menu.width  + menu.x - 1;
	int menu_y2 = menu.height + menu.y - 1;

	_menuButtonList = 0;

	_screen->hideMouse();
	_screen->fillRect(menu.x + 2, menu.y + 2, menu_x2 - 2, menu_y2 - 2, menu.bgcolor);
	_screen->drawShadedBox(menu.x, menu.y, menu_x2, menu_y2, menu.color1, menu.color2);

	int textX;
	int textY;

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

		if (menu.item[i].field_12 != -1)
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
			_text->printText(menu.item[i].labelString, menu.x + menu.item[i].field_21 - 1, menu.y + menu.item[i].field_23 + 1, 12, 0, 0);
			_text->printText(menu.item[i].labelString, menu.x + menu.item[i].field_21, menu.y + menu.item[i].field_23, 253, 0, 0);
		}
	}

	if (menu.scrollUpBtnX != -1) {
		_scrollUpButton.x = menu.scrollUpBtnX + menu.x;
		_scrollUpButton.y = menu.scrollUpBtnY + menu.y;
		_scrollUpButton.buttonCallback = &KyraEngine::gui_scrollUp;
		_scrollUpButton.nextButton = 0;
		_menuButtonList = initButton(_menuButtonList, &_scrollUpButton);
		processMenuButton(&_scrollUpButton);
		
		_scrollDownButton.x = menu.scrollDownBtnX + menu.x;
		_scrollDownButton.y = menu.scrollDownBtnY + menu.y;
		_scrollDownButton.buttonCallback = &KyraEngine::gui_scrollDown;
		_scrollDownButton.nextButton = 0;
		_menuButtonList = initButton(_menuButtonList, &_scrollDownButton);
		processMenuButton(&_scrollDownButton);
	}

	_screen->showMouse();
	_screen->updateScreen();
}

void KyraEngine::calcCoords(Menu &menu) {
	if (menu.x == -1)
		menu.x = (320 - menu.width)/2;

	if (menu.y == -1)
		menu.y = (200 - menu.height)/2;

	assert(menu.nrOfItems < 7);
	for (int i = 0; i < menu.nrOfItems; i++)
		if (menu.item[i].x == -1)
			menu.item[i].x = (menu.width - menu.item[i].width)/2;
}

void KyraEngine::gui_getInput() {
	OSystem::Event event;

	_mousePressFlag = false;
	while (_system->pollEvent(event)) {
		switch (event.type) {
			case OSystem::EVENT_QUIT:
				quitGame();
				break;
			case OSystem::EVENT_LBUTTONUP:
				_mousePressFlag = true;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				_system->updateScreen();
				break;
			default:
				break;
		}
	}
	_system->delayMillis(10);
}

int KyraEngine::gui_resumeGame(Button *button) {
	debug(9, "KyraEngine::gui_resumeGame()");
	processMenuButton(button);
	_displayMenu = false;

	return 0;
}

const char *KyraEngine::getSavegameName(int num) {
	static char saveLoadSlot[12];

	sprintf(saveLoadSlot, "%s.%.3d", _targetName.c_str(), num);
	return saveLoadSlot;
}

void KyraEngine::setupSavegames(Menu &menu, int num) {
	Common::InSaveFile *in;
	static char savenames[5][31];

	assert(num <= 5);

	for (int i = 0; i < num; i++) {
		if ((in = _saveFileMan->openForLoading(getSavegameName(i + _savegameOffset)))) {
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

int KyraEngine::gui_loadGameMenu(Button *button) {
	debug(9, "KyraEngine::gui_loadGameMenu()");
	processMenuButton(button);
	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	_savegameOffset = 0;
	setupSavegames(_menu[2], 5);
	initMenu(_menu[2]);
	processAllMenuButtons();

	_displayLoadGameMenu = true;
	_cancelLoadGameMenu = false;

	while (_displayLoadGameMenu) {
		gui_getInput();
		gui_processHighlights(_menu[2]);
		processButtonList(_menuButtonList);
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	if (_cancelLoadGameMenu) {
		initMenu(_menu[0]);
		processAllMenuButtons();
	} else {
		loadGame(getSavegameName(_gameToLoad));
		_displayMenu = false;
		_menuRestoreScreen = false;
	}
	return 0;
}

int KyraEngine::gui_loadGame(Button *button) {
	debug(9, "KyraEngine::gui_loadGame()");
	processMenuButton(button);
	_displayLoadGameMenu = false;
	_gameToLoad = button->specialValue;

	return 0;
}

int KyraEngine::gui_cancelLoadGameMenu(Button *button) {
	debug(9, "KyraEngine::gui_cancelLoadGameMenu()");
	processMenuButton(button);
	_displayLoadGameMenu = false;
	_cancelLoadGameMenu = true;

	return 0;
}

int KyraEngine::gui_quitPlaying(Button *button) {
	debug(9, "KyraEngine::gui_quitPlaying()");

	processMenuButton(button);
	if (gui_quitConfirm("Are you sure you want to quit playing?"))
		quitGame();
	else {
		initMenu(_menu[0]);
		processAllMenuButtons();
	}

	return 0;
}

bool KyraEngine::gui_quitConfirm(const char *str) {
	debug(9, "KyraEngine::gui_quitConfirm()");

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	_menu[1].menuName = str;
	initMenu(_menu[1]);

	_displayQuitConfirmDialog = true;
	_quitConfirmed = false;

	while (_displayQuitConfirmDialog) {
		gui_getInput();
		gui_processHighlights(_menu[1]);
		processButtonList(_menuButtonList);
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	return _quitConfirmed;
}

int KyraEngine::gui_quitConfirmYes(Button *button) {
	debug(9, "KyraEngine::gui_quitConfirmYes()");
	processMenuButton(button);
	_displayQuitConfirmDialog = false;
	_quitConfirmed = true;

	return 0;
}

int KyraEngine::gui_quitConfirmNo(Button *button) {
	debug(9, "KyraEngine::gui_quitConfirmNo()");
	processMenuButton(button);
	_displayQuitConfirmDialog = false;
	_quitConfirmed = false;

	return 0;
}

int KyraEngine::gui_scrollUp(Button *button) {
	debug(9, "KyraEngine::gui_scrollUp()");
	processMenuButton(button);

	if (_savegameOffset > 0) {
		_savegameOffset--;
		setupSavegames(_menu[2], 5);
		initMenu(_menu[2]);
	}
	return 0;
}

int KyraEngine::gui_scrollDown(Button *button) {
	debug(9, "KyraEngine::gui_scrollDown()");
	processMenuButton(button);

	_savegameOffset++;
	setupSavegames(_menu[2], 5);
	initMenu(_menu[2]);

	return 0;
}

void KyraEngine::gui_processHighlights(Menu &menu) {
	int x1, y1, x2, y2;

	for (int i = 0; i < menu.nrOfItems; i++) {
		if (!menu.item[i].enabled)
			continue;

		x1 = menu.x + menu.item[i].x;
		y1 = menu.y + menu.item[i].y;

		x2 = x1 + menu.item[i].width;
		y2 = y1 + menu.item[i].height;

		if (_mouseX > x1 && _mouseX < x2 &&
			_mouseY > y1 && _mouseY < y2) {
			
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

void KyraEngine::gui_redrawText(Menu menu) {
	int textX;
	int i = menu.highlightedItem;

	int x1 = menu.x + menu.item[i].x;
	int y1 = menu.y + menu.item[i].y;

	int x2 = x1 + menu.item[i].width - 1;

	if (menu.item[i].field_12 != -1)
		textX = x1 + menu.item[i].field_12 + 3;
	else
		textX = _text->getCenterStringX(menu.item[i].itemString, x1, x2);

	int textY = y1 + 2;
	_text->printText(menu.item[i].itemString, textX - 1, textY + 1,  12, 0, 0);
	_text->printText(menu.item[i].itemString, textX, textY, menu.item[i].textColor, 0, 0);
}

void KyraEngine::gui_redrawHighlight(Menu menu) {
	int textX;
	int i = menu.highlightedItem;

	int x1 = menu.x + menu.item[i].x;
	int y1 = menu.y + menu.item[i].y;

	int x2 = x1 + menu.item[i].width - 1;

	if (menu.item[i].field_12 != -1)
		textX = x1 + menu.item[i].field_12 + 3;
	else
		textX = _text->getCenterStringX(menu.item[i].itemString, x1, x2);

	int textY = y1 + 2;
	_text->printText(menu.item[i].itemString, textX - 1, textY + 1,  12, 0, 0);
	_text->printText(menu.item[i].itemString, textX, textY, menu.item[i].highlightColor, 0, 0);
}

} // end of namespace Kyra
 
