/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
		characterSays(_putDownFirst[0], 0, -2);
		return 1;
	}
	if (queryGameFlag(0xF1)) {
		assert(_waitForAmulet);
		characterSays(_waitForAmulet[0], 0, -2);
		return 1;
	}
	if (!queryGameFlag(0x55+jewel)) {
		assert(_blackJewel);
		makeBrandonFaceMouse();
		drawJewelPress(jewel, 1);
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
				characterSays(_healingTip[0], 0, -2);
			}
			break;
		
		case 1:
			seq_makeBrandonInv();
			break;
		
		case 2:
			if (_brandonStatusBit & 1) {
				assert(_wispJewelStrings);
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

int KyraEngine::buttonMenuCallback(Button *caller) {
	warning("Menu not implemented yet!");
	return 0;
}

} // end of namespace Kyra
 
