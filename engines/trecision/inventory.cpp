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
#include "nl/extern.h"

#include "trecision/graphics.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"
#include "video.h"

namespace Trecision {

void TrecisionEngine::initInventory() {
	_inventorySize = 0;
	_inventory[_inventorySize++] = iBANCONOTE;
	_inventory[_inventorySize++] = iSAM;
	_inventory[_inventorySize++] = iCARD03;
	_inventory[_inventorySize++] = iPEN;
	_inventory[_inventorySize++] = iKEY05;
}

/*------------------------------------------------
                    refreshInventory()
--------------------------------------------------*/
void TrecisionEngine::refreshInventory(uint8 StartIcon, uint8 StartLine) {
	if (StartLine > ICONDY)
		StartLine = ICONDY;

	for (uint16 b = 0; b < ICONDY; b++)
		wordset(_video2 + (FIRSTLINE + b) * CurRoomMaxX + CurScrollPageDx, 0, SCREENLEN);

	for (uint16 a = 0; a < ICONSHOWN; a++) {
		if (_inventory[a + StartIcon] >= LASTICON) {
			for (uint16 b = 0; b < (ICONDY - StartLine); b++)
				MCopy(_video2 + (FIRSTLINE + b) * CurRoomMaxX + a * (ICONDX) + ICONMARGSX + CurScrollPageDx,
					  Icone + (_inventory[a + StartIcon] - LASTICON + READICON + 1) * ICONDX * ICONDY + (b + StartLine) * ICONDX, ICONDX);
		} else if (_inventory[a + StartIcon] != _lightIcon) {
			for (uint16 b = 0; b < (ICONDY - StartLine); b++)
				MCopy(_video2 + (FIRSTLINE + b) * CurRoomMaxX + a * (ICONDX) + ICONMARGSX + CurScrollPageDx,
					  Icone + _inventory[a + StartIcon] * ICONDX * ICONDY + (b + StartLine) * ICONDX, ICONDX);
		}
	}

	// Arrows
	if (StartIcon != 0) { // Copy left
		int16 LeftArrow = ICONMARGSX * ICONDY * 3;
		for (uint16 b = 0; b < (ICONDY - StartLine); b++) {
			MCopy(_video2 + (FIRSTLINE + b) * CurRoomMaxX + CurScrollPageDx,
				  Arrows + LeftArrow + (b + StartLine) * ICONMARGSX, ICONMARGSX);
		}
	}

	if ((StartIcon + ICONSHOWN) < _inventorySize) { // Copy right
		int16 RightArrow = ICONMARGDX * ICONDY * 2;
		for (uint16 b = 0; b < (ICONDY - StartLine); b++) {
			MCopy(_video2 + (FIRSTLINE + b) * CurRoomMaxX + CurScrollPageDx + SCREENLEN - ICONMARGDX,
				  Arrows + RightArrow + ICONMARGSX * ICONDY * 2 + (b + StartLine) * ICONMARGSX, ICONMARGSX);
		}
	}

	//refreshSmkIcon( StartIcon, 2 );

	VMouseCopy();
	for (uint16 a = 0; a < ICONDY; a++) {
		_graphicsMgr->vCopy((FIRSTLINE + a) * VirtualPageLen + VideoScrollPageDx, _video2 + (FIRSTLINE + a) * CurRoomMaxX + CurScrollPageDx, SCREENLEN);
	}

	VMouseRestore();
	//VMouseON();
	_graphicsMgr->unlock();
}

/*-------------------------------------------------------------------------*/
/*                            setInventoryStart					  		   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::setInventoryStart(uint8 StartIcon, uint8 StartLine) {
	_inventoryRefreshStartIcon = StartIcon;
	_inventoryRefreshStartLine = StartLine;
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
		if ((!_flagInventoryLocked) && (_inventoryStatus == INV_OFF) && !(FlagDialogActive)) {
			_inventoryCounter = INVENTORY_HIDE;
			_inventorySpeedIndex = 0;
			_inventoryStatus = INV_PAINT;
		}
		break;

	case ME_CLOSE:
		if ((!_flagInventoryLocked) && (_inventoryStatus == INV_INACTION) && !(FlagDialogActive)) {
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
		_curInventory = WhatIcon(mx);
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
			}
			else {
				_animMgr->stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);
				ShowInvName(_curInventory, true);
				_lightIcon = _curInventory;
			}
		}
		else if (_inventoryObj[_curInventory]._flag & OBJFLAG_USEWITH) {
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
			ShowInvName(_curInventory, true);
		}
		else
			doEvent(MC_ACTION, ME_INVOPERATE, MP_DEFAULT, 0, 0, 0, _curInventory);
		break;

	case ME_EXAMINEICON:
		_curInventory = WhatIcon(mx);
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
			}
			else {
				_animMgr->stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);
				ShowInvName(_curInventory, true);
				_lightIcon = _curInventory;
			}
		}
		else
			doEvent(MC_ACTION, ME_INVEXAMINE, MP_DEFAULT, 0, 0, 0, _curInventory);
		break;

	case ME_SHOWICONNAME:
		if (ICONAREA(mx, my)) {
			if (_inventoryStatus != INV_ON)
				doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			_curInventory = WhatIcon(mx);
			ShowInvName(_curInventory, true);

			if (!FlagUseWithStarted && !FlagSomeOneSpeak) {
				setInventoryStart(_iconBase, INVENTORY_SHOW);
			}
		}
		else {
			if (!(INVAREA(my)))
				break;
			ShowInvName(NO_OBJECTS, true);
			if (!(FlagUseWithStarted)) {
				_lightIcon = 0xFF;
				setInventoryStart(_iconBase, INVENTORY_SHOW);
			}
		}
		break;
	}
}

} // End of namespace Trecision
