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

#include "trecision/actor.h"
#include "trecision/animmanager.h"
#include "trecision/defines.h"
#include "trecision/graphics.h"
#include "trecision/logic.h"
#include "trecision/pathfinding3d.h"
#include "trecision/text.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {

void TrecisionEngine::refreshInventory(uint8 startIcon, uint8 startLine) {
	if (startLine > ICONDY)
		return;

	_graphicsMgr->clearScreenBufferInventory();

	for (uint8 iconSlot = 0; iconSlot < ICONSHOWN; iconSlot++) {
		uint8 i = iconSlot + startIcon;
		if (i >= _inventory.size())
			break;
		const byte iconIndex = _inventory[i];
		if (iconIndex == _lightIcon)
			continue;

		if (iconIndex <= EMPTYSLOT)
			_graphicsMgr->drawInventoryIcon(iconIndex - 1, iconSlot, startLine);
		else
			_graphicsMgr->drawSaveSlotThumbnail(iconIndex - EMPTYSLOT - 1, iconSlot, startLine);
	}

	if (startIcon != 0)
		_graphicsMgr->drawLeftInventoryArrow(startLine);

	if (startIcon + ICONSHOWN < (int)_inventory.size())
		_graphicsMgr->drawRightInventoryArrow(startLine);

	_graphicsMgr->copyToScreen(0, FIRSTLINE, MAXX, ICONDY);
}

void TrecisionEngine::setInventoryStart(uint8 startIcon, uint8 startLine) {
	_inventoryRefreshStartIcon = startIcon;
	_inventoryRefreshStartLine = startLine;
}

void TrecisionEngine::moveInventoryLeft() {
	if (_iconBase < _inventory.size() - ICONSHOWN)
		++_iconBase;
	setInventoryStart(_iconBase, INVENTORY_SHOW);
}

void TrecisionEngine::moveInventoryRight() {
	if (_iconBase > 0)
		--_iconBase;
	setInventoryStart(_iconBase, INVENTORY_SHOW);
}

void TrecisionEngine::showIconName() {
	if (isIconArea(_mousePos)) {
		if (_inventoryStatus != INV_ON)
			openInventory();
		_curInventory = whatIcon(_mousePos);
		showInventoryName(_curInventory, true);

		if (!_flagUseWithStarted && !_flagSomeoneSpeaks) {
			setInventoryStart(_iconBase, INVENTORY_SHOW);
		}
	} else if (isInventoryArea(_mousePos)) {
		showInventoryName(NO_OBJECTS, true);
		if (!_flagUseWithStarted) {
			_lightIcon = 0xFF;
			setInventoryStart(_iconBase, INVENTORY_SHOW);
		}
	}
}

void TrecisionEngine::openInventory() {
	if (!_flagInventoryLocked && (_inventoryStatus == INV_OFF) && !_flagDialogActive) {
		_inventoryCounter = INVENTORY_HIDE;
		_inventorySpeedIndex = 0;
		_inventoryStatus = INV_PAINT;
	}
}

void TrecisionEngine::closeInventory() {
	if (!_flagInventoryLocked && (_inventoryStatus == INV_INACTION) && !_flagDialogActive) {
		_inventoryCounter = INVENTORY_SHOW;
		_inventorySpeedIndex = 0;
		_inventoryStatus = INV_DEPAINT;
		_lightIcon = 0xFF;
	}
}

void TrecisionEngine::closeInventoryImmediately() {
	_inventoryStatus = INV_OFF;
	_lightIcon = 0xFF;
	_flagInventoryLocked = false;
	_inventoryRefreshStartLine = INVENTORY_HIDE;
	_inventoryCounter = INVENTORY_HIDE;
	setInventoryStart(_inventoryRefreshStartIcon, INVENTORY_HIDE);
	refreshInventory(_inventoryRefreshStartIcon, _inventoryRefreshStartLine);
}

void TrecisionEngine::examineItem() {
	_curInventory = whatIcon(_mousePos);
	_actor->actorStop();
	_pathFind->nextStep();
	if (_flagUseWithStarted) {
		endUseWith();
	} else
		doInvExamine();
}

void TrecisionEngine::useItem() {
	_curInventory = whatIcon(_mousePos);
	if (_curInventory == 0)
		return;

	if (_flagUseWithStarted) {
		endUseWith();
	} else if (_inventoryObj[_curInventory].isUseWith()) {
		if (_curInventory == kItemFlare && _curRoom == kRoom29) {
			_textMgr->characterSay(kSentenceOnlyGotOne);
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
		doInvOperate();
}

void TrecisionEngine::endUseWith() {
	_flagInventoryLocked = false;
	_flagUseWithStarted = false;
	_useWith[WITH] = _curInventory;
	_useWithInv[WITH] = true;
	_lightIcon = 0xFF;

	if (_useWith[USED] != _curInventory) {
		doUseWith();
	} else {
		_animMgr->smkStop(kSmackerIcon);
		showInventoryName(_curInventory, true);
	}
}

void TrecisionEngine::clearUseWith() {
	if (_flagUseWithStarted) {
		if (_useWithInv[USED]) {
			_lightIcon = 0xFF;
			_animMgr->smkStop(kSmackerIcon);
			setInventoryStart(_inventoryRefreshStartIcon, INVENTORY_HIDE);
			_flagInventoryLocked = false;
		}
		_useWith[USED] = 0;
		_useWith[WITH] = 0;
		_useWithInv[USED] = false;
		_useWithInv[WITH] = false;
		_flagUseWithStarted = false;
		_textMgr->clearLastText();
	}
}

uint8 TrecisionEngine::whatIcon(Common::Point pos) {
	if (pos.x < ICONMARGSX || pos.x > MAXX - ICONMARGDX)
		return 0;

	int index = _iconBase + ((pos.x - ICONMARGSX) / (ICONDX));

	return index < (int)_inventory.size() ? _inventory[index] : 0;
}

int8 TrecisionEngine::iconPos(uint8 icon) {
	for (uint8 i = 0; i < _inventory.size(); i++) {
		if (_inventory[i] == icon)
			return i;
	}

	return -1;
}

void TrecisionEngine::showInventoryName(uint16 obj, bool showhide) {
	if (_logicMgr->isCloseupOrControlRoom() || _flagSomeoneSpeaks)
		return;

	if (_lastObj) {
		_textMgr->clearLastText();
		_lastObj = 0;
	}

	if (_flagUseWithStarted) {
		if (!showhide) {
			_textMgr->clearLastText();
			_lastInv = 0;
			return;
		}
		if ((obj | 0x8000) == _lastInv)
			return;

		Common::String desc = _sysText[kMessageUse];
		if (_useWithInv[USED]) {
			desc += _objName[_inventoryObj[_useWith[USED]]._name];
			desc += _sysText[kMessageWith];
			if (obj && (_inventoryObj[_useWith[USED]]._name != _inventoryObj[obj]._name))
				desc += _objName[_inventoryObj[obj]._name];
		} else {
			if (_obj[_useWith[USED]].isModeHidden())
				desc += "?";	// dunno
			else
				desc += _objName[_obj[_useWith[USED]]._name];
			desc += _sysText[kMessageWith];
			if (obj && (_obj[_useWith[USED]]._name != _inventoryObj[obj]._name))
				desc += _objName[_inventoryObj[obj]._name];
		}

		const uint16 lenText = textLength(desc);
		Common::Point pos(CLIP(320 - (lenText / 2), 2, MAXX - 2 - lenText), MAXY - CARHEI);

		_lastInv = (obj | 0x8000);
		if (_lastInv)
			_textMgr->clearLastText();
		_textMgr->addText(pos, desc.c_str(), COLOR_INVENTORY);
	} else {
		if (obj == _lastInv)
			return;

		if (!obj || !showhide) {
			_textMgr->clearLastText();
			_lastInv = 0;
			return;
		}

		const uint16 lenText = textLength(_objName[_inventoryObj[obj]._name]);
		uint16 posX = ICONMARGSX + ((iconPos(_curInventory) - _iconBase) * (ICONDX)) + ICONDX / 2;
		posX = CLIP(posX - (lenText / 2), 2, MAXX - 2 - lenText);
		Common::Point pos(posX, MAXY - CARHEI);
		
		_lastInv = obj;

		if (_lastInv)
			_textMgr->clearLastText();

		if (_inventoryObj[obj]._name)
			_textMgr->addText(pos, _objName[_inventoryObj[obj]._name], COLOR_INVENTORY);
	}
}

void TrecisionEngine::removeIcon(uint8 icon) {
	const int8 pos = iconPos(icon);
	if (pos == -1)
		return;

	_inventory.remove_at(pos);
	_iconBase = _inventory.size() <= ICONSHOWN ? 0 : _inventory.size() - ICONSHOWN;

	_textMgr->redrawString();
}

void TrecisionEngine::addIcon(uint8 icon) {
	if (iconPos(icon) != -1)
		return;

	_inventory.push_back(icon);
	_iconBase = _inventory.size() <= ICONSHOWN ? 0 : _inventory.size() - ICONSHOWN;

	//	To show the icon that enters the inventory
	//	doEvent(MC_INVENTORY,ME_OPEN,MP_DEFAULT,0,0,0,0);
	//	FlagForceRegenInventory = true;
	_textMgr->redrawString();
}

void TrecisionEngine::replaceIcon(uint8 oldIcon, uint8 newIcon) {
	int8 pos = iconPos(oldIcon);
	if (pos >= 0)
		_inventory[pos] = newIcon;
}

void TrecisionEngine::rollInventory(uint8 status) {
	static const int16 inventorySpeed[8] = { 20, 10, 5, 3, 2, 0, 0, 0 };

	if (status == INV_PAINT) {
		_inventoryCounter -= inventorySpeed[_inventorySpeedIndex++];
		if (_inventoryCounter <= INVENTORY_SHOW || _inventorySpeedIndex > 5) {
			_inventorySpeedIndex = 0;
			setInventoryStart(_iconBase, INVENTORY_SHOW);
			_inventoryStatus = INV_INACTION;
			_inventoryCounter = INVENTORY_SHOW;
			if (!isInventoryArea(_mousePos))
				closeInventory();
			_textMgr->redrawString();
			return;
		}
	} else if (status == INV_DEPAINT) {
		_inventoryCounter += inventorySpeed[_inventorySpeedIndex++];

		if (_inventoryCounter > INVENTORY_HIDE || _inventorySpeedIndex > 5) {
			_inventorySpeedIndex = 0;
			setInventoryStart(_iconBase, INVENTORY_HIDE);
			_inventoryStatus = INV_OFF;
			_inventoryCounter = INVENTORY_HIDE;
			if (isInventoryArea(_mousePos) && !(_flagDialogActive || _flagDialogMenuActive))
				openInventory();
			else
				_textMgr->redrawString();
			return;
		}
	}
	setInventoryStart(_iconBase, _inventoryCounter);
}

void TrecisionEngine::doScrollInventory(Common::Point pos) {
	if (_inventoryStatus != INV_INACTION)
		return;

	if (pos.x <= ICONMARGSX && _iconBase)
		moveInventoryRight();
	else if (isBetween(MAXX - ICONMARGDX, pos.x, MAXX) && (_iconBase + ICONSHOWN < (int)_inventory.size()))
		moveInventoryLeft();
}

void TrecisionEngine::syncInventory(Common::Serializer &ser) {
	if (ser.isLoading()) {
		_inventory.clear();
		_cyberInventory.clear();
	}

	for (uint which = 0; which <= 1; which++) {
		for (uint i = 0; i < MAXICON; i++) {
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
