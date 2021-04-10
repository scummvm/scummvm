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

#include "trecision/logic.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/graphics.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {

/*------------------------------------------------
                    refreshInventory()
--------------------------------------------------*/
void TrecisionEngine::refreshInventory(uint8 startIcon, uint8 startLine) {
	if (startLine > ICONDY)
		startLine = ICONDY;

	for (uint16 b = 0; b < ICONDY; b++)
		memset(_screenBuffer + (FIRSTLINE + b) * SCREENLEN, 0, SCREENLEN * 2);

	for (uint16 a = 0; a < ICONSHOWN; a++) {
		if (_inventory[a + startIcon] >= LASTICON) {
			for (uint16 b = 0; b < (ICONDY - startLine); b++)
				memcpy(_screenBuffer + (FIRSTLINE + b) * SCREENLEN + a * (ICONDX) + ICONMARGSX,
					  _icons + (_inventory[a + startIcon] - LASTICON + READICON + 1) * ICONDX * ICONDY + (b + startLine) * ICONDX, ICONDX * 2);
		} else if (_inventory[a + startIcon] != _lightIcon) {
			for (uint16 b = 0; b < (ICONDY - startLine); b++)
				memcpy(_screenBuffer + (FIRSTLINE + b) * SCREENLEN + a * (ICONDX) + ICONMARGSX,
					  _icons + _inventory[a + startIcon] * ICONDX * ICONDY + (b + startLine) * ICONDX, ICONDX * 2);
		}
	}

	// Arrows
	if (startIcon != 0) { // Copy left
		int16 leftArrow = ICONMARGSX * ICONDY * 3;
		for (uint16 b = 0; b < (ICONDY - startLine); b++) {
			memcpy(_screenBuffer + (FIRSTLINE + b) * SCREENLEN,
				  _arrows + leftArrow + (b + startLine) * ICONMARGSX, ICONMARGSX * 2);
		}
	}

	if ((startIcon + ICONSHOWN) < _inventorySize) { // Copy right
		int16 rightArrow = ICONMARGDX * ICONDY * 2;
		for (uint16 b = 0; b < (ICONDY - startLine); b++) {
			memcpy(_screenBuffer + (FIRSTLINE + b) * SCREENLEN + SCREENLEN - ICONMARGDX,
				  _arrows + rightArrow + ICONMARGSX * ICONDY * 2 + (b + startLine) * ICONMARGSX, ICONMARGSX * 2);
		}
	}

	_graphicsMgr->copyToScreen(0, FIRSTLINE, SCREENLEN, ICONDY);
}

/*-------------------------------------------------------------------------*/
/*                            setInventoryStart					  		   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::setInventoryStart(uint8 startIcon, uint8 startLine) {
	_inventoryRefreshStartIcon = startIcon;
	_inventoryRefreshStartLine = startLine;
}

/*-------------------------------------------------------------------------*/
/*                            moveInventoryLeft          				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::moveInventoryLeft() {
	if (_iconBase < _inventorySize - ICONSHOWN)
		_iconBase++;
	setInventoryStart(_iconBase, INVENTORY_SHOW);
}

/*-------------------------------------------------------------------------*/
/*                            moveInventoryRight          				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::moveInventoryRight() {
	if (_iconBase > 0)
		_iconBase--;
	setInventoryStart(_iconBase, INVENTORY_SHOW);
}

/*-------------------------------------------------------------------------*/
/*                                doInventory          					   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::doInventory() {
	switch (_curMessage->_event) {
	case ME_OPEN:
		if (!_flagInventoryLocked && (_inventoryStatus == INV_OFF) && !FlagDialogActive) {
			_inventoryCounter = INVENTORY_HIDE;
			_inventorySpeedIndex = 0;
			_inventoryStatus = INV_PAINT;
		}
		break;

	case ME_CLOSE:
		if (!_flagInventoryLocked && (_inventoryStatus == INV_INACTION) && !FlagDialogActive) {
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
		_curInventory = whatIcon(mx);
		if (_curInventory == 0)
			break;

		if (FlagUseWithStarted) {
			_flagInventoryLocked = false;
			FlagUseWithStarted = false;
			_useWith[WITH] = _curInventory;
			_useWithInv[WITH] = true;

			if (_useWith[USED] != _curInventory) {
				doEvent(MC_ACTION, ME_USEWITH, MP_DEFAULT, 0, 0, 0, 0);
				_lightIcon = 0xFF;
			} else {
				_animMgr->stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);
				showInventoryName(_curInventory, true);
				_lightIcon = _curInventory;
			}
		} else if (_inventoryObj[_curInventory]._flag & OBJFLAG_USEWITH) {
			if ((_curInventory == iCANDELOTTO) && (_curRoom == r29)) {
				CharacterSay(1565);
				return;
			}
			_animMgr->startSmkAnim(_inventoryObj[_curInventory]._anim);
			_lightIcon = _curInventory;
			setInventoryStart(_iconBase, INVENTORY_SHOW);
			_flagInventoryLocked = true;
			FlagUseWithStarted = true;
			_useWith[USED] = _curInventory;
			_useWithInv[USED] = true;
			showInventoryName(_curInventory, true);
		} else
			doEvent(MC_ACTION, ME_INVOPERATE, MP_DEFAULT, 0, 0, 0, _curInventory);
		break;

	case ME_EXAMINEICON:
		_curInventory = whatIcon(mx);
		actorStop();
		nextStep();
		if (FlagUseWithStarted) {
			_flagInventoryLocked = false;
			FlagUseWithStarted = false;
			_useWith[WITH] = _curInventory;
			_useWithInv[WITH] = true;
			if (_useWith[USED] != _curInventory) {
				doEvent(MC_ACTION, ME_USEWITH, MP_DEFAULT, 0, 0, 0, 0);
				_lightIcon = 0xFF;
			} else {
				_animMgr->stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);
				showInventoryName(_curInventory, true);
				_lightIcon = _curInventory;
			}
		} else
			doEvent(MC_ACTION, ME_INVEXAMINE, MP_DEFAULT, 0, 0, 0, _curInventory);
		break;

	case ME_SHOWICONNAME:
		if (ICONAREA(mx, my)) {
			if (_inventoryStatus != INV_ON)
				doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			_curInventory = whatIcon(mx);
			showInventoryName(_curInventory, true);

			if (!FlagUseWithStarted && !FlagSomeOneSpeak) {
				setInventoryStart(_iconBase, INVENTORY_SHOW);
			}
		} else {
			if (!isInventoryArea(my))
				break;
			showInventoryName(NO_OBJECTS, true);
			if (!FlagUseWithStarted) {
				_lightIcon = 0xFF;
				setInventoryStart(_iconBase, INVENTORY_SHOW);
			}
		}
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                                whatIcon           					   */
/*-------------------------------------------------------------------------*/
uint8 TrecisionEngine::whatIcon(uint16 invmx) {
	if (invmx < ICONMARGSX || invmx > SCREENLEN - ICONMARGDX)
		return 0;

	return _inventory[_iconBase + ((invmx - ICONMARGSX) / (ICONDX))];
}

/*-------------------------------------------------------------------------*/
/*                                 iconPos           					   */
/*-------------------------------------------------------------------------*/
uint8 TrecisionEngine::iconPos(uint8 icon) {
	uint8 i;

	for (i = 0; i < MAXICON; i++) {
		if (_inventory[i] == icon)
			break;
	}

	return i;
}

/*-------------------------------------------------------------------------*/
/*                            showInventoryName           				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::showInventoryName(uint16 obj, bool showhide) {
	static const char *dunno = "?";

	if ((_curRoom == r2BL) || (_curRoom == r36F) || (_curRoom == r41D) || (_curRoom == r49M)
	 || (_curRoom == r4CT) || (_curRoom == r58T) || (_curRoom == r58M) || (_curRoom == r59L)
	 || (_curRoom == rSYS) || (_curRoom == r12CU) || (_curRoom == r13CU))
		return;

	if (FlagSomeOneSpeak)
		return;

	if (_lastObj) {
		clearText();
		_lastObj = 0;
	}

	if (FlagUseWithStarted && !FlagUseWithLocked) {
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

		uint16 lenText = TextLength(locsent, 0);
		uint16 posX = CLIP(320 - (lenText / 2), 2, SCREENLEN - 2 - lenText);
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
		uint16 lenText = TextLength(_objName[_inventoryObj[obj]._name], 0);

		posX = CLIP(posX - (lenText / 2), 2, SCREENLEN - 2 - lenText);

		if (_lastInv)
			clearText();

		if (_inventoryObj[obj]._name)
			addText(posX, posY, _objName[_inventoryObj[obj]._name], COLOR_INVENTORY, MASKCOL);
	}
}

/*-------------------------------------------------------------------------*/
/*                                removeIcon           					   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::removeIcon(uint8 icon) {
	uint8 pos = iconPos(icon);

	if (pos == MAXICON)
		return;
	_inventory[pos] = iNULL;
	for (; pos < _inventorySize; pos++)
		_inventory[pos] = _inventory[pos + 1];
	_inventorySize--;

	if (_inventorySize < ICONSHOWN)
		_iconBase = 0;

	if (_iconBase && (_inventorySize > ICONSHOWN) && (_inventory[_iconBase + ICONSHOWN] == iNULL))
		_iconBase = _inventorySize - ICONSHOWN;

	redrawString();
}

/*-------------------------------------------------------------------------*/
/*                                 addIcon           					   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::addIcon(uint8 icon) {
	if (iconPos(icon) != MAXICON)
		return;
	_inventory[_inventorySize++] = icon;
	if (_inventorySize >= MAXICON)
		warning("addIcon overflow");

	if (_iconBase < _inventorySize - ICONSHOWN)
		_iconBase = _inventorySize - ICONSHOWN;

	//	To show the icon that enters the inventory
	//	doEvent(MC_INVENTORY,ME_OPEN,MP_DEFAULT,0,0,0,0);
	//	FlagForceRegenInventory = true;
	redrawString();
}

/*-------------------------------------------------------------------------*/
/*                             replaceIcon          					   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::replaceIcon(uint8 oldIcon, uint8 newIcon) {
	uint8 pos = iconPos(oldIcon);

	_inventory[pos] = newIcon;
}

/*-------------------------------------------------------------------------*/
/*                          USE WITH / INV - INV         				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::doInventoryUseWithInventory() {
	if (!_useWith[USED] || !_useWith[WITH])
		warning("doInventoryUseWithInventory - _useWith not set properly");

	_animMgr->stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);

	bool refreshInventory, printSentence;
	_logicMgr->useInventoryWithInventory(&refreshInventory, &printSentence);

	if (printSentence)
		CharacterSay(_inventoryObj[_useWith[USED]]._action);
	if (refreshInventory)
		setInventoryStart(_iconBase, INVENTORY_SHOW);
}

/*-------------------------------------------------------------------------*/
/*                          USE WITH / INV - SCR         				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::doInventoryUseWithScreen() {
	if (!_useWith[USED] || !_useWith[WITH])
		warning("doInventoryUseWithScreen - _useWith not set properly");

	_animMgr->stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);
	if (_characterInMovement)
		return;
	
	bool refreshInventory, printSentence;
	_logicMgr->useInventoryWithScreen(&refreshInventory, &printSentence);

	if (printSentence)
		CharacterSay(_inventoryObj[_useWith[USED]]._action);

	if (refreshInventory)
		setInventoryStart(_iconBase, INVENTORY_SHOW);
}

/*-------------------------------------------------------------------------*/
/*                                ROLLINVENTORY             			   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::rollInventory(uint8 status) {
	if (status == INV_PAINT) {
		_inventoryCounter -= _inventorySpeed[_inventorySpeedIndex++];
		if (_inventoryCounter <= INVENTORY_SHOW || _inventorySpeedIndex > 5) {
			_inventorySpeedIndex = 0;
			setInventoryStart(_iconBase, INVENTORY_SHOW);
			_inventoryStatus = INV_INACTION;
			_inventoryCounter = INVENTORY_SHOW;
			if (!(isInventoryArea(my)))
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
			if (isInventoryArea(my) && !(FlagDialogActive || FlagDialogMenuActive))
				doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			else
				redrawString();
			return;
		}
	}
	setInventoryStart(_iconBase, _inventoryCounter);
}

/*-------------------------------------------------------------------------*/
/*                              doScrollInventory         				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::doScrollInventory(uint16 mouseX) {
	if ((_inventoryStatus == INV_PAINT) || (_inventoryStatus == INV_DEPAINT))
		return;

	if ((mouseX <= ICONMARGSX) && _iconBase)
		doEvent(MC_INVENTORY, ME_ONERIGHT, MP_DEFAULT, 0, 0, 0, 0);
	else if (BETWEEN(SCREENLEN - ICONMARGDX, mouseX, SCREENLEN) && (_iconBase + ICONSHOWN < _inventorySize))
		doEvent(MC_INVENTORY, ME_ONELEFT, MP_DEFAULT, 0, 0, 0, 0);
}

} // End of namespace Trecision
