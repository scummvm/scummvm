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

#include "prince/prince.h"

#include "prince/graphics.h"
#include "prince/hero.h"
#include "prince/script.h"
#include "prince/mhwanh.h"
#include "prince/variatxt.h"
#include "prince/option_text.h"
#include "prince/font.h"

namespace Prince {

void PrinceEngine::addInv(int heroId, int item, bool addItemQuiet) {
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _mainHero;
	} else if (heroId == 1) {
		hero = _secondHero;
	}
	if (hero != nullptr) {
		if (hero->_inventory.size() < kMaxItems) {
			if (item != 0x7FFF) {
				hero->_inventory.push_back(item);
			}
			if (!addItemQuiet) {
				addInvObj();
			}
			_interpreter->setResult(0);
		} else {
			_interpreter->setResult(1);
		}
	}
}

void PrinceEngine::remInv(int heroId, int item) {
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _mainHero;
	} else if (heroId == 1) {
		hero = _secondHero;
	}
	if (hero != nullptr) {
		for (uint i = 0; i < hero->_inventory.size(); i++) {
			if (hero->_inventory[i] == item) {
				hero->_inventory.remove_at(i);
				_interpreter->setResult(0);
				return;
			}
		}
	}
	_interpreter->setResult(1);
}

void PrinceEngine::clearInv(int heroId) {
	switch (heroId) {
	case 0:
		_mainHero->_inventory.clear();
		break;
	case 1:
		_secondHero->_inventory.clear();
		break;
	default:
		error("clearInv() - wrong hero slot");
		break;
	}
}

void PrinceEngine::swapInv(int heroId) {
	Common::Array<int> tempInv;
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _mainHero;
	} else if (heroId == 1) {
		hero = _secondHero;
	}
	if (hero != nullptr) {
		for (uint i = 0; i < hero->_inventory.size(); i++) {
			tempInv.push_back(hero->_inventory[i]);
		}
		hero->_inventory.clear();
		for (uint i = 0; i < hero->_inventory2.size(); i++) {
			hero->_inventory.push_back(hero->_inventory2[i]);
		}
		hero->_inventory2.clear();
		for (uint i = 0; i < tempInv.size(); i++) {
			hero->_inventory2.push_back(tempInv[i]);
		}
		tempInv.clear();
	}
}

void PrinceEngine::addInvObj() {
	changeCursor(0);
	prepareInventoryToView();

	_inventoryBackgroundRemember = true;
	drawScreen();

	Graphics::Surface *suitcase = _suitcaseBmp->getSurface();

	if (!_flags->getFlagValue(Flags::CURSEBLINK)) {

		loadSample(27, "PRZEDMIO.WAV");
		playSample(27, 0);

		_mst_shadow2 = 1;

		while (_mst_shadow2 < 512) {
			rememberScreenInv();
			_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);
			drawInvItems();
			_graph->update(_graph->_screenForInventory);
			_mst_shadow2 += 50;
			Common::Event event;
			Common::EventManager *eventMan = _system->getEventManager();
			eventMan->pollEvent(event);
			if (shouldQuit()) {
				return;
			}
			pausePrinceEngine();
		}
		while (_mst_shadow2 > 256) {
			rememberScreenInv();
			_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);
			drawInvItems();
			_graph->update(_graph->_screenForInventory);
			_mst_shadow2 -= 42;
			Common::Event event;
			Common::EventManager *eventMan = _system->getEventManager();
			eventMan->pollEvent(event);
			if (shouldQuit()) {
				return;
			}
			pausePrinceEngine();
		}
	} else {
		//CURSEBLINK:
		for (int i = 0; i < 3; i++) {
			_mst_shadow2 = 256;
			while (_mst_shadow2 < 512) {
				rememberScreenInv();
				_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);
				drawInvItems();
				_graph->update(_graph->_screenForInventory);
				_mst_shadow2 += 50;
				Common::Event event;
				Common::EventManager *eventMan = _system->getEventManager();
				eventMan->pollEvent(event);
				if (shouldQuit()) {
					return;
				}
				pausePrinceEngine();
			}
			while (_mst_shadow2 > 256) {
				rememberScreenInv();
				_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);
				drawInvItems();
				_graph->update(_graph->_screenForInventory);
				_mst_shadow2 -= 50;
				Common::Event event;
				Common::EventManager *eventMan = _system->getEventManager();
				eventMan->pollEvent(event);
				if (shouldQuit()) {
					return;
				}
				pausePrinceEngine();
			}
		}
	}
	_mst_shadow2 = 0;
	for (int i = 0; i < 20; i++) {
		rememberScreenInv();
		_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);
		drawInvItems();
		_graph->update(_graph->_screenForInventory);
		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		eventMan->pollEvent(event);
		if (shouldQuit()) {
			return;
		}
		pausePrinceEngine();
	}
}

void PrinceEngine::rememberScreenInv() {
	_graph->_screenForInventory->copyFrom(*_graph->_frontScreen);
}

void PrinceEngine::inventoryFlagChange(bool inventoryState) {
	if (inventoryState) {
		_showInventoryFlag = true;
		_inventoryBackgroundRemember = true;
	} else {
		_showInventoryFlag = false;
	}
}

void PrinceEngine::prepareInventoryToView() {
	_invMobList.clear();
	int invItem = _mainHero->_inventory.size();
	_invLine =  invItem / 3;
	if (invItem % 3) {
		_invLine++;
	}
	if (_invLine < 4) {
		_invLine = 4;
	}
	_maxInvW = (374 - 2 * _invLine) / _invLine;
	_invLineW = _maxInvW - 2;

	int currInvX = _invLineX;
	int currInvY = _invLineY;

	Common::MemoryReadStream stream(_invTxt, _invTxtSize);
	byte c;

	uint item = 0;
	for (int i = 0; i < _invLines; i++) {
		for (int j = 0; j < _invLine; j++) {
			Mob tempMobItem;
			if (item < _mainHero->_inventory.size()) {
				int itemNr = _mainHero->_inventory[item];
				tempMobItem._visible = 0;
				tempMobItem._mask =  itemNr;
				tempMobItem._rect = Common::Rect(currInvX + _picWindowX, currInvY, currInvX + _picWindowX + _invLineW  - 1, currInvY + _invLineH - 1);
				tempMobItem._type = 0; // to work with checkMob()

				tempMobItem._name = "";
				tempMobItem._examText = "";
				int txtOffset = READ_LE_UINT32(&_invTxt[itemNr * 8]);
				int examTxtOffset = READ_LE_UINT32(&_invTxt[itemNr * 8 + 4]);

				stream.seek(txtOffset);
				while ((c = stream.readByte())) {
					tempMobItem._name += c;
				}

				stream.seek(examTxtOffset);
				while ((c = stream.readByte())) {
					tempMobItem._examText += c;
				}
				_invMobList.push_back(tempMobItem);
			}
			currInvX += _invLineW + _invLineSkipX;
			item++;
		}
		currInvX = _invLineX;
		currInvY += _invLineSkipY + _invLineH;
	}
}

void PrinceEngine::drawInvItems() {
	int currInvX = _invLineX;
	int currInvY = _invLineY;
	uint item = 0;
	for (int i = 0; i < _invLines; i++) {
		for (int j = 0; j < _invLine; j++) {
			if (item < _mainHero->_inventory.size()) {
				int itemNr = _mainHero->_inventory[item];
				_mst_shadow = 0;
				if (_mst_shadow2) {
					if (!_flags->getFlagValue(Flags::CURSEBLINK)) {
						if (item + 1 == _mainHero->_inventory.size()) { // last item in inventory
							_mst_shadow = 1;
						}
					} else if (itemNr == 1 || itemNr == 3 || itemNr == 4 || itemNr == 7) {
						_mst_shadow = 1;
					}
				}

				int drawX = currInvX;
				int drawY = currInvY;
				Graphics::Surface *itemSurface = nullptr;
				if (itemNr != 68) {
					itemSurface = _allInvList[itemNr].getSurface();
					if (itemSurface->h < _maxInvH) {
						drawY += (_maxInvH - itemSurface->h) / 2;
					}
				} else {
					// candle item:
					if (_candleCounter == 8) {
						_candleCounter = 0;
					}
					itemNr = _candleCounter;
					_candleCounter++;
					itemNr &= 7;
					itemNr += 71;
					itemSurface = _allInvList[itemNr].getSurface();
					drawY += _allInvList[itemNr]._y + (_maxInvH - 76) / 2 - 200;
				}
				if (itemSurface->w < _maxInvW) {
					drawX += (_maxInvW - itemSurface->w) / 2;
				}
				if (!_mst_shadow) {
					_graph->drawTransparentSurface(_graph->_screenForInventory, drawX, drawY, itemSurface);
				} else {
					_mst_shadow = _mst_shadow2;
					_graph->drawTransparentWithBlendSurface(_graph->_screenForInventory, drawX, drawY, itemSurface);
				}
			}
			currInvX += _invLineW + _invLineSkipX;
			item++;
		}
		currInvX = _invLineX;
		currInvY += _invLineSkipY + _invLineH;
	}
}

void PrinceEngine::inventoryLeftMouseButton() {
	if (!_mouseFlag) {
		_textSlots[0]._time = 0;
		_textSlots[0]._str = nullptr;
		stopSample(28);
	}

	if (_optionsFlag == 1) {
		if (_selectedMob != -1)  {
			if (_optionEnabled < _invOptionsNumber) {
				_optionsFlag = 0;
			} else {
				return;
			}
		} else {
			error("PrinceEngine::inventoryLeftMouseButton() - optionsFlag = 1, selectedMob = 0");
			if (_currentPointerNumber == 2) {
				changeCursor(1);
				_currentPointerNumber = 1;
				_selectedMob = -1;
				_optionsMob = -1;
				return;
			} else {
				return;
			}
		}
	} else {
		if (_selectedMob != -1) {
			if (_currentPointerNumber != 2) {
				if (_invMobList[_selectedMob]._mask != 29) {
					_optionEnabled = 0;
				} else {
					// map item
					_optionEnabled = 1;
				}
			} else {
				//use_item_on_item
				int invObjUU = _script->scanMobEventsWithItem(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjUU, _selectedItem);
				if (invObjUU == -1) {
					int textNr = 80011; // "I can't do it."
					if (_selectedItem == 31 || _invMobList[_selectedMob]._mask == 31) {
						textNr = 80020; // "Nothing is happening."
					}
					_interpreter->setCurrentString(textNr);
					printAt(0, 216, (char *)_variaTxt->getString(textNr - 80000), kNormalWidth / 2, 100);
					setVoice(0, 28, 1);
					playSample(28, 0);
					_selectedMob = -1;
					_optionsMob = -1;
					return;
				} else {
					_interpreter->storeNewPC(invObjUU);
					_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
					_showInventoryFlag = false;
				}
			}
		} else {
			return;
		}
	}
	//do_option
	if (_optionEnabled == 0) {
		int invObjExamEvent = _script->scanMobEvents(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjExam);
		if (invObjExamEvent == -1) {
			static char buf[256];
			strncpy(buf, _invMobList[_selectedMob]._examText.c_str(), 256);
			printAt(0, 216, buf, kNormalWidth / 2, _invExamY);
			_interpreter->setCurrentString(_invMobList[_selectedMob]._mask + 70000);
			setVoice(0, 28, 1);
			playSample(28, 0);
			// disableuseuse
			changeCursor(0);
			_currentPointerNumber = 1;
		} else {
			_interpreter->storeNewPC(invObjExamEvent);
			_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
			_showInventoryFlag = false;
		}
	} else if (_optionEnabled == 1) {
		// not_examine
		int invObjUse = _script->scanMobEvents(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjUse);
		if (invObjUse == -1) {
			// do_standard_use
			_selectedMode = 0;
			_selectedItem = _invMobList[_selectedMob]._mask;
			makeInvCursor(_invMobList[_selectedMob]._mask);
			_currentPointerNumber = 2;
			changeCursor(2);
		} else {
			_interpreter->storeNewPC(invObjUse);
			_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
			_showInventoryFlag = false;
		}
	} else if (_optionEnabled == 4) {
		// do_standard_give
		_selectedMode = 1;
		_selectedItem = _invMobList[_selectedMob]._mask;
		makeInvCursor(_invMobList[_selectedMob]._mask);
		_currentPointerNumber = 2;
		changeCursor(2);
	} else {
		// use_item_on_item
		int invObjUU = _script->scanMobEventsWithItem(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjUU, _selectedItem);
		if (invObjUU == -1) {
			int textNr = 80011; // "I can't do it."
			if (_selectedItem == 31 || _invMobList[_selectedMob]._mask == 31) {
				textNr = 80020; // "Nothing is happening."
			}
			_interpreter->setCurrentString(textNr);
			printAt(0, 216, (char *)_variaTxt->getString(textNr - 80000), kNormalWidth / 2, 100);
			setVoice(0, 28, 1);
			playSample(28, 0);
		} else {
			_interpreter->storeNewPC(invObjUU);
			_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
			_showInventoryFlag = false;
		}
	}
	_selectedMob = -1;
	_optionsMob = -1;
}

void PrinceEngine::inventoryRightMouseButton() {
	if (_textSlots[0]._str == nullptr) {
		enableOptions(false);
	}
}

void PrinceEngine::enableOptions(bool checkType) {
	if (_optionsFlag != 1) {
		changeCursor(1);
		_currentPointerNumber = 1;
		if (_selectedMob != -1) {
			if (checkType) {
				if (_mobList[_selectedMob]._type & 0x100) {
					return;
				}
			}
			Common::Point mousePos = _system->getEventManager()->getMousePos();
			int x1 = mousePos.x - _optionsWidth / 2;
			int x2 = mousePos.x + _optionsWidth / 2;
			if (x1 < 0) {
				x1 = 0;
				x2 = _optionsWidth;
			} else if (x2 >= kNormalWidth) {
				x1 = kNormalWidth - _optionsWidth;
				x2 = kNormalWidth;
			}
			int y1 = mousePos.y - 10;
			if (y1 < 0) {
				y1 = 0;
			}
			if (y1 + _optionsHeight >= kNormalHeight) {
				y1 = kNormalHeight - _optionsHeight;
			}
			_optionsMob = _selectedMob;
			_optionsX = x1;
			_optionsY = y1;
			_optionsFlag = 1;
		}
	}
}

void PrinceEngine::checkOptions() {
	if (_optionsFlag) {
		Common::Rect optionsRect(_optionsX, _optionsY, _optionsX + _optionsWidth, _optionsY + _optionsHeight);
		Common::Point mousePos = _system->getEventManager()->getMousePos();
		if (!optionsRect.contains(mousePos)) {
			_optionsFlag = 0;
			_selectedMob = -1;
			return;
		}
		_graph->drawAsShadowSurface(_graph->_frontScreen, _optionsX, _optionsY, _optionsPic, _graph->_shadowTable50);

		_optionEnabled = -1;
		int optionsYCord = mousePos.y - (_optionsY + 16);
		if (optionsYCord >= 0) {
			int selectedOptionNr = optionsYCord / _optionsStep;
			if (selectedOptionNr < _optionsNumber) {
				_optionEnabled = selectedOptionNr;
			}
		}
		int optionsColor;
		int textY = _optionsY + 16;
		for (int i = 0; i < _optionsNumber; i++) {
			if (i != _optionEnabled) {
				optionsColor = _optionsColor1;
			} else {
				optionsColor = _optionsColor2;
			}
			Common::String optText;
			switch(getLanguage()) {
			case Common::PL_POL:
				optText = optionsTextPL[i];
				break;
			case Common::DE_DEU:
				optText = optionsTextDE[i];
				break;
			case Common::EN_ANY:
				optText = optionsTextEN[i];
				break;
			case Common::RU_RUS:
                if (getFeatures() & GF_RUSPROJEDITION) {
                    optText = optionsTextRU2[i];
                } else {
                    optText = optionsTextRU[i];
                }
                break;
			default:
				break;
			};
			uint16 textW = getTextWidth(optText.c_str());
			uint16 textX = _optionsX + _optionsWidth / 2 - textW / 2;
			_font->drawString(_graph->_frontScreen, optText, textX, textY, textW, optionsColor);
			textY += _optionsStep;
		}
	}
}

void PrinceEngine::checkInvOptions() {
	if (_optionsFlag) {
		Common::Rect optionsRect(_optionsX, _optionsY, _optionsX + _invOptionsWidth, _optionsY + _invOptionsHeight);
		Common::Point mousePos = _system->getEventManager()->getMousePos();
		if (!optionsRect.contains(mousePos)) {
			_optionsFlag = 0;
			_selectedMob = -1;
			return;
		}
		_graph->drawAsShadowSurface(_graph->_screenForInventory, _optionsX, _optionsY, _optionsPicInInventory, _graph->_shadowTable50);

		_optionEnabled = -1;
		int optionsYCord = mousePos.y - (_optionsY + 16);
		if (optionsYCord >= 0) {
			int selectedOptionNr = optionsYCord / _invOptionsStep;
			if (selectedOptionNr < _invOptionsNumber) {
				_optionEnabled = selectedOptionNr;
			}
		}
		int optionsColor;
		int textY = _optionsY + 16;
		for (int i = 0; i < _invOptionsNumber; i++) {
			if (i != _optionEnabled) {
				optionsColor = _optionsColor1;
			} else {
				optionsColor = _optionsColor2;
			}
			Common::String invText;
			switch(getLanguage()) {
			case Common::PL_POL:
				invText = invOptionsTextPL[i];
				break;
			case Common::DE_DEU:
				invText = invOptionsTextDE[i];
				break;
			case Common::EN_ANY:
				invText = invOptionsTextEN[i];
				break;
			case Common::RU_RUS:
                if (getFeatures() & GF_RUSPROJEDITION) {
                    invText = invOptionsTextRU2[i];
                } else {
                    invText = invOptionsTextRU[i];
                }
                break;
			default:
				error("Unknown game language %d", getLanguage());
				break;
			};
			uint16 textW = getTextWidth(invText.c_str());
			uint16 textX = _optionsX + _invOptionsWidth / 2 - textW / 2;
			_font->drawString(_graph->_screenForInventory, invText, textX, textY, _graph->_screenForInventory->w, optionsColor);
			textY += _invOptionsStep;
		}
	}
}

void PrinceEngine::displayInventory() {

	_mainHero->freeOldMove();
	_secondHero->freeOldMove();

	_interpreter->setFgOpcodePC(0);

	stopAllSamples();

	prepareInventoryToView();

	while (!shouldQuit()) {

		if (_textSlots[0]._str != nullptr) {
			changeCursor(0);
		} else {
			changeCursor(_currentPointerNumber);

			Common::Rect inventoryRect(_invX1, _invY1, _invX1 + _invWidth, _invY1 + _invHeight);
			Common::Point mousePos = _system->getEventManager()->getMousePos();

			if (!_invCurInside && inventoryRect.contains(mousePos)) {
				_invCurInside = true;
			}

			if (_invCurInside && !inventoryRect.contains(mousePos)) {
				inventoryFlagChange(false);
				_invCurInside = false;
				break;
			}
		}

		rememberScreenInv();

		Graphics::Surface *suitcase = _suitcaseBmp->getSurface();
		_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase);

		drawInvItems();

		showTexts(_graph->_screenForInventory);

		if (!_optionsFlag && _textSlots[0]._str == nullptr) {
			_selectedMob = checkMob(_graph->_screenForInventory, _invMobList, false);
		}

		checkInvOptions();

		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				keyHandler(event);
				break;
			case Common::EVENT_LBUTTONDOWN:
				inventoryLeftMouseButton();
				break;
			case Common::EVENT_RBUTTONDOWN:
				inventoryRightMouseButton();
				break;
			default:
				break;
			}
		}

		if (!_showInventoryFlag) {
			break;
		}

		if (shouldQuit())
			return;

		_graph->update(_graph->_screenForInventory);
		pausePrinceEngine();
	}

	if (_currentPointerNumber == 2) {
		_flags->setFlagValue(Flags::SELITEM, _selectedItem);
	} else {
		_flags->setFlagValue(Flags::SELITEM, 0);
	}
}

void PrinceEngine::openInventoryCheck() {
	if (!_optionsFlag) {
		if (_mouseFlag == 1 || _mouseFlag == 2) {
			if (_mainHero->_visible) {
				if (!_flags->getFlagValue(Flags::INVALLOWED)) {
					// 29 - Basement, 50 - Map
					if (_locationNr != 29 && _locationNr != 50) {
						Common::Point mousePos = _system->getEventManager()->getMousePos();
						if (mousePos.y < 4 && !_showInventoryFlag) {
							_invCounter++;
						} else {
							_invCounter = 0;
						}
						if (_invCounter >= _invMaxCount) {
							inventoryFlagChange(true);
						}
					}
				}
			}
		}
	}
}

} // End of namespace Prince
