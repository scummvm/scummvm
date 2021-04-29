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

#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/proto.h"
#include "trecision/defines.h"
#include "trecision/graphics.h"
#include "trecision/logic.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {

void TrecisionEngine::refreshInventory(uint8 startIcon, uint8 startLine) {
	if (startLine > ICONDY)
		startLine = ICONDY;

	for (uint16 b = 0; b < ICONDY; b++)
		memset(_screenBuffer + (FIRSTLINE + b) * MAXX, 0, MAXX * 2);

	for (uint16 a = 0; a < ICONSHOWN; a++) {
		int index = a + startIcon;
		if (index >= _inventory.size())
			break;

		if (_inventory[index] >= LASTICON) {
			for (uint16 b = 0; b < (ICONDY - startLine); b++)
				memcpy(_screenBuffer + (FIRSTLINE + b) * MAXX + a * (ICONDX) + ICONMARGSX,
					   _icons + (_inventory[index] - LASTICON + READICON + 1) * ICONDX * ICONDY + (b + startLine) * ICONDX, ICONDX * 2);
		} else if (_inventory[index] != _lightIcon) {
			for (uint16 b = 0; b < (ICONDY - startLine); b++)
				memcpy(_screenBuffer + (FIRSTLINE + b) * MAXX + a * (ICONDX) + ICONMARGSX,
					   _icons + _inventory[index] * ICONDX * ICONDY + (b + startLine) * ICONDX, ICONDX * 2);
		}
	}

	// Arrows
	if (startIcon != 0) { // Copy left
		int16 leftArrow = ICONMARGSX * ICONDY * 3;
		for (uint16 b = 0; b < (ICONDY - startLine); b++) {
			memcpy(_screenBuffer + (FIRSTLINE + b) * MAXX,
				  _arrows + leftArrow + (b + startLine) * ICONMARGSX, ICONMARGSX * 2);
		}
	}

	if (startIcon + ICONSHOWN < _inventory.size()) { // Copy right
		int16 rightArrow = ICONMARGDX * ICONDY * 2;
		for (uint16 b = 0; b < (ICONDY - startLine); b++) {
			memcpy(_screenBuffer + (FIRSTLINE + b) * MAXX + MAXX - ICONMARGDX,
				  _arrows + rightArrow + ICONMARGSX * ICONDY * 2 + (b + startLine) * ICONMARGSX, ICONMARGSX * 2);
		}
	}

	_graphicsMgr->copyToScreen(0, FIRSTLINE, MAXX, ICONDY);
}

void TrecisionEngine::setInventoryStart(uint8 startIcon, uint8 startLine) {
	_inventoryRefreshStartIcon = startIcon;
	_inventoryRefreshStartLine = startLine;
}

void TrecisionEngine::moveInventoryLeft() {
	if (_iconBase < _inventory.size() - ICONSHOWN)
		_iconBase++;
	setInventoryStart(_iconBase, INVENTORY_SHOW);
}

void TrecisionEngine::moveInventoryRight() {
	if (_iconBase > 0)
		_iconBase--;
	setInventoryStart(_iconBase, INVENTORY_SHOW);
}

void TrecisionEngine::doInventory() {
	switch (_curMessage->_event) {
	case ME_OPEN:
		if (!_flagInventoryLocked && (_inventoryStatus == INV_OFF) && !_flagDialogActive) {
			_inventoryCounter = INVENTORY_HIDE;
			_inventorySpeedIndex = 0;
			_inventoryStatus = INV_PAINT;
		}
		break;

	case ME_CLOSE:
		if (!_flagInventoryLocked && (_inventoryStatus == INV_INACTION) && !_flagDialogActive) {
			_inventoryCounter = INVENTORY_SHOW;
			_inventorySpeedIndex = 0;
			_inventoryStatus = INV_DEPAINT;
			_lightIcon = 0xFF;
		}
		break;

	case ME_ONELEFT:
		if (_inventoryStatus == INV_INACTION)
			moveInventoryLeft();
		break;

	case ME_ONERIGHT:
		if (_inventoryStatus == INV_INACTION)
			moveInventoryRight();
		break;

	case ME_OPERATEICON:
		_curInventory = whatIcon(_mouseX);
		if (_curInventory == 0)
			break;

		if (_flagUseWithStarted) {
			_flagInventoryLocked = false;
			_flagUseWithStarted = false;
			_useWith[WITH] = _curInventory;
			_useWithInv[WITH] = true;

			if (_useWith[USED] != _curInventory) {
				doEvent(MC_ACTION, ME_USEWITH, MP_DEFAULT, 0, 0, 0, 0);
				_lightIcon = 0xFF;
			} else {
				_animMgr->smkStop(kSmackerIcon);
				showInventoryName(_curInventory, true);
				_lightIcon = _curInventory;
			}
		} else if (_inventoryObj[_curInventory]._flag & kObjFlagUseWith) {
			if ((_curInventory == kItemFlare) && (_curRoom == kRoom29)) {
				CharacterSay(1565);
				return;
			}
			_animMgr->startSmkAnim(_inventoryObj[_curInventory]._anim);
			_lightIcon = _curInventory;
			setInventoryStart(_iconBase, INVENTORY_SHOW);
			_flagInventoryLocked = true;
			_flagUseWithStarted = true;
			_useWith[USED] = _curInventory;
			_useWithInv[USED] = true;
			showInventoryName(_curInventory, true);
		} else
			doEvent(MC_ACTION, ME_INVOPERATE, MP_DEFAULT, 0, 0, 0, _curInventory);
		break;

	case ME_EXAMINEICON:
		_curInventory = whatIcon(_mouseX);
		actorStop();
		nextStep();
		if (_flagUseWithStarted) {
			_flagInventoryLocked = false;
			_flagUseWithStarted = false;
			_useWith[WITH] = _curInventory;
			_useWithInv[WITH] = true;
			if (_useWith[USED] != _curInventory) {
				doEvent(MC_ACTION, ME_USEWITH, MP_DEFAULT, 0, 0, 0, 0);
				_lightIcon = 0xFF;
			} else {
				_animMgr->smkStop(kSmackerIcon);
				showInventoryName(_curInventory, true);
				_lightIcon = _curInventory;
			}
		} else
			doEvent(MC_ACTION, ME_INVEXAMINE, MP_DEFAULT, 0, 0, 0, _curInventory);
		break;

	case ME_SHOWICONNAME:
		if (ICONAREA(_mouseX, _mouseY)) {
			if (_inventoryStatus != INV_ON)
				doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			_curInventory = whatIcon(_mouseX);
			showInventoryName(_curInventory, true);

			if (!_flagUseWithStarted && !_flagSomeoneSpeaks) {
				setInventoryStart(_iconBase, INVENTORY_SHOW);
			}
		} else {
			if (!isInventoryArea(_mouseY))
				break;
			showInventoryName(NO_OBJECTS, true);
			if (!_flagUseWithStarted) {
				_lightIcon = 0xFF;
				setInventoryStart(_iconBase, INVENTORY_SHOW);
			}
		}
		break;
	}
}

uint8 TrecisionEngine::whatIcon(uint16 invmx) {
	if (invmx < ICONMARGSX || invmx > MAXX - ICONMARGDX)
		return 0;

	int index = _iconBase + ((invmx - ICONMARGSX) / (ICONDX));
	
	return index < _inventory.size() ? _inventory[index] : kItemNull;
}

int8 TrecisionEngine::iconPos(uint8 icon) {
	for (uint8 i = 0; i < _inventory.size(); i++) {
		if (_inventory[i] == icon)
			return i;
	}

	return -1;
}

bool TrecisionEngine::isCloseupOrControlRoom() {
	return _curRoom == kRoom2BL ||
		   _curRoom == kRoom36F ||
		   _curRoom == kRoom41D ||
		   _curRoom == kRoom49M ||
		   _curRoom == kRoom4CT ||
		   _curRoom == kRoom58T ||
		   _curRoom == kRoom58M ||
		   _curRoom == kRoom59L ||
		   _curRoom == kRoomControlPanel ||
		   _curRoom == kRoom12CU ||
		   _curRoom == kRoom13CU;
}

void TrecisionEngine::showInventoryName(uint16 obj, bool showhide) {
	static const char *dunno = "?";

	if (isCloseupOrControlRoom() || _flagSomeoneSpeaks)
		return;

	if (_lastObj) {
		clearText();
		_lastObj = 0;
	}

	if (_flagUseWithStarted) {
		if (!showhide) {
			clearText();
			_lastInv = 0;
			return;
		}
		if ((obj | 0x8000) == _lastInv)
			return;

		char locsent[256];
		strcpy(locsent, _sysText[kMessageUse]);
		if (_useWithInv[USED]) {
			strcat(locsent, _objName[_inventoryObj[_useWith[USED]]._name]);
			strcat(locsent, _sysText[kMessageWith]);
			if (obj && (_inventoryObj[_useWith[USED]]._name != _inventoryObj[obj]._name))
				strcat(locsent, _objName[_inventoryObj[obj]._name]);
		} else {
			if (_obj[_useWith[USED]]._mode & OBJMODE_HIDDEN)
				strcat(locsent, dunno);
			else
				strcat(locsent, _objName[_obj[_useWith[USED]]._name]);
			strcat(locsent, _sysText[kMessageWith]);
			if (obj && (_obj[_useWith[USED]]._name != _inventoryObj[obj]._name))
				strcat(locsent, _objName[_inventoryObj[obj]._name]);
		}

		uint16 lenText = textLength(locsent, 0);
		uint16 posX = CLIP(320 - (lenText / 2), 2, MAXX - 2 - lenText);
		uint16 posY = MAXY - CARHEI;

		_lastInv = (obj | 0x8000);
		if (_lastInv)
			clearText();
		addText(posX, posY, locsent, COLOR_INVENTORY, MASKCOL);
	} else {
		if (obj == _lastInv)
			return;

		if (!obj || !showhide) {
			clearText();
			_lastInv = 0;
			return;
		}
		uint16 posX = ICONMARGSX + ((iconPos(_curInventory) - _iconBase) * (ICONDX)) + ICONDX / 2;
		uint16 posY = MAXY - CARHEI;
		_lastInv = obj;
		uint16 lenText = textLength(_objName[_inventoryObj[obj]._name], 0);

		posX = CLIP(posX - (lenText / 2), 2, MAXX - 2 - lenText);

		if (_lastInv)
			clearText();

		if (_inventoryObj[obj]._name)
			addText(posX, posY, _objName[_inventoryObj[obj]._name], COLOR_INVENTORY, MASKCOL);
	}
}

void TrecisionEngine::removeIcon(uint8 icon) {
	int8 pos = iconPos(icon);
	if (pos == -1)
		return;

	_inventory.remove_at(pos);
	_iconBase = _inventory.size() <= ICONSHOWN ? 0 : _inventory.size() - ICONSHOWN; 

	redrawString();
}

void TrecisionEngine::addIcon(uint8 icon) {
	if (iconPos(icon) != -1)
		return;

	_inventory.push_back(icon);
	_iconBase = _inventory.size() <= ICONSHOWN ? 0 : _inventory.size() - ICONSHOWN; 

	//	To show the icon that enters the inventory
	//	doEvent(MC_INVENTORY,ME_OPEN,MP_DEFAULT,0,0,0,0);
	//	FlagForceRegenInventory = true;
	redrawString();
}

void TrecisionEngine::replaceIcon(uint8 oldIcon, uint8 newIcon) {
	int8 pos = iconPos(oldIcon);
	if (pos >= 0)
		_inventory[pos] = newIcon;
}

void TrecisionEngine::doInventoryUseWithInventory() {
	if (!_useWith[USED] || !_useWith[WITH])
		warning("doInventoryUseWithInventory - _useWith not set properly");

	_animMgr->smkStop(kSmackerIcon);

	bool refreshInventory, printSentence;
	_logicMgr->useInventoryWithInventory(&refreshInventory, &printSentence);

	if (printSentence)
		CharacterSay(_inventoryObj[_useWith[USED]]._action);
	if (refreshInventory)
		setInventoryStart(_iconBase, INVENTORY_SHOW);
}

void TrecisionEngine::doInventoryUseWithScreen() {
	if (!_useWith[USED] || !_useWith[WITH])
		warning("doInventoryUseWithScreen - _useWith not set properly");

	_animMgr->smkStop(kSmackerIcon);
	if (_characterInMovement)
		return;

	bool refreshInventory, printSentence;
	_logicMgr->useInventoryWithScreen(&refreshInventory, &printSentence);

	if (printSentence)
		CharacterSay(_inventoryObj[_useWith[USED]]._action);

	if (refreshInventory)
		setInventoryStart(_iconBase, INVENTORY_SHOW);
}

void TrecisionEngine::rollInventory(uint8 status) {
	if (status == INV_PAINT) {
		_inventoryCounter -= _inventorySpeed[_inventorySpeedIndex++];
		if (_inventoryCounter <= INVENTORY_SHOW || _inventorySpeedIndex > 5) {
			_inventorySpeedIndex = 0;
			setInventoryStart(_iconBase, INVENTORY_SHOW);
			_inventoryStatus = INV_INACTION;
			_inventoryCounter = INVENTORY_SHOW;
			if (!isInventoryArea(_mouseY))
				doEvent(MC_INVENTORY, ME_CLOSE, MP_DEFAULT, 0, 0, 0, 0);
			redrawString();
			return;
		}
	} else if (status == INV_DEPAINT) {
		_inventoryCounter += _inventorySpeed[_inventorySpeedIndex++];

		if (_inventoryCounter > INVENTORY_HIDE || _inventorySpeedIndex > 5) {
			_inventorySpeedIndex = 0;
			setInventoryStart(_iconBase, INVENTORY_HIDE);
			_inventoryStatus = INV_OFF;
			_inventoryCounter = INVENTORY_HIDE;
			if (isInventoryArea(_mouseY) && !(_flagDialogActive || _flagDialogMenuActive))
				doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			else
				redrawString();
			return;
		}
	}
	setInventoryStart(_iconBase, _inventoryCounter);
}

void TrecisionEngine::doScrollInventory(uint16 mouseX) {
	if (_inventoryStatus == INV_PAINT || _inventoryStatus == INV_DEPAINT)
		return;

	if (mouseX <= ICONMARGSX && _iconBase)
		doEvent(MC_INVENTORY, ME_ONERIGHT, MP_DEFAULT, 0, 0, 0, 0);
	else if (BETWEEN(MAXX - ICONMARGDX, mouseX, MAXX) && (_iconBase + ICONSHOWN < _inventory.size()))
		doEvent(MC_INVENTORY, ME_ONELEFT, MP_DEFAULT, 0, 0, 0, 0);
}

void TrecisionEngine::syncInventory(Common::Serializer &ser) {
	if (ser.isLoading()) {
		_inventory.clear();
		_cyberInventory.clear();
	}

	for (int which = 0; which <= 1; which++) {
		for (int i = 0; i < MAXICON; i++) {
			byte val = 0;
			if (ser.isSaving()) {
				if (which == 0)
					val = i < _inventory.size() ? _inventory[i] : 0;
				else
					val = i < _cyberInventory.size() ? _cyberInventory[i] : 0;
				ser.syncAsByte(val);
			} else {
				ser.syncAsByte(val);
				if (val != kItemNull) {
					if (which == 0)
						_inventory.push_back(val);
					else
						_cyberInventory.push_back(val);
				}
			}
		}
	}
}

} // End of namespace Trecision
