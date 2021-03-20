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

#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/graphics.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

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
			}
			else {
				_animMgr->stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);
				showInventoryName(_curInventory, true);
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
			showInventoryName(_curInventory, true);
		}
		else
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
			}
			else {
				_animMgr->stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);
				showInventoryName(_curInventory, true);
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
			_curInventory = whatIcon(mx);
			showInventoryName(_curInventory, true);

			if (!FlagUseWithStarted && !FlagSomeOneSpeak) {
				setInventoryStart(_iconBase, INVENTORY_SHOW);
			}
		}
		else {
			if (!(INVAREA(my)))
				break;
			showInventoryName(NO_OBJECTS, true);
			if (!(FlagUseWithStarted)) {
				_lightIcon = 0xFF;
				setInventoryStart(_iconBase, INVENTORY_SHOW);
			}
		}
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                                WHATICON           					   */
/*-------------------------------------------------------------------------*/
uint8 TrecisionEngine::whatIcon(uint16 invmx) {
	if (invmx < ICONMARGSX || invmx > SCREENLEN - ICONMARGDX)
		return 0;

	return _inventory[_iconBase + ((invmx - ICONMARGSX) / (ICONDX))];
}

/*-------------------------------------------------------------------------*/
/*                                 ICONPOS           					   */
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
/*                                 SHOWINVNAME           				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::showInventoryName(uint16 obj, bool showhide) {
	uint16 posx;
	uint16 posy;
	uint16 LenText;

	static const char *dunno = "?";

	if ((_curRoom == r2BL) || (_curRoom == r36F) || (_curRoom == r41D) || (_curRoom == r49M)
	 || (_curRoom == r4CT) || (_curRoom == r58T) || (_curRoom == r58M) || (_curRoom == r59L)
	 || (_curRoom == rSYS) || (_curRoom == r12CU) || (_curRoom == r13CU))
		return;

	if (FlagSomeOneSpeak)
		return;

	if (lastobj) {
		ClearText();
		lastobj = 0;
	}

	if (FlagUseWithStarted && !FlagUseWithLocked) {
		if (!showhide) {
			ClearText();
			lastinv = 0;
			return;
		}
		if ((obj | 0x8000) == lastinv)
			return;

		char locsent[256];
		strcpy(locsent, _sysText[23]);
		if (_useWithInv[USED]) {
			strcat(locsent, _objName[_inventoryObj[_useWith[USED]]._name]);
			strcat(locsent, _sysText[24]);
			if (obj && (_inventoryObj[_useWith[USED]]._name != _inventoryObj[obj]._name))
				strcat(locsent, _objName[_inventoryObj[obj]._name]);
		} else {
			if (_obj[_useWith[USED]]._mode & OBJMODE_HIDDEN)
				strcat(locsent, dunno);
			else
				strcat(locsent, _objName[_obj[_useWith[USED]]._name]);
			strcat(locsent, _sysText[24]);
			if (obj && (_obj[_useWith[USED]]._name != _inventoryObj[obj]._name))
				strcat(locsent, _objName[_inventoryObj[obj]._name]);
		}

		LenText = TextLength(locsent, 0);
		posx = CLIP(320 - (LenText / 2), 2, SCREENLEN - 2 - LenText);
		posy = MAXY - CARHEI;

		lastinv = (obj | 0x8000);
		if (lastinv)
			ClearText();
		Text(posx, posy, locsent, COLOR_INVENTORY, MASKCOL);
	} else {
		if (obj == lastinv)
			return;

		if (!obj || !showhide) {
			ClearText();
			lastinv = 0;
			return;
		}
		posx = ICONMARGSX + ((iconPos(_curInventory) - _iconBase) * (ICONDX)) + ICONDX / 2;
		posy = MAXY - CARHEI;
		lastinv = obj;
		LenText = TextLength(_objName[_inventoryObj[obj]._name], 0);

		posx = CLIP(posx - (LenText / 2), 2, SCREENLEN - 2 - LenText);

		if (lastinv)
			ClearText();

		if (_inventoryObj[obj]._name)
			Text(posx, posy, _objName[_inventoryObj[obj]._name], COLOR_INVENTORY, MASKCOL);
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

	RepaintString();
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
	RepaintString();
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
	bool updateInv = true;
	bool printSentence = true;

	if (!_useWith[USED] || !_useWith[WITH])
		warning("doInventoryUseWithInventory");

	_animMgr->stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);

	switch (_useWith[USED]) {
	case iSTAGNOLA:
		if (_useWith[WITH] == iFUSE) {
			removeIcon(iSTAGNOLA);
			replaceIcon(iFUSE, iFUSES);
			StartCharacterAction(hUSEGG, 0, 0, 1441);
			printSentence = false;
		}
		break;

	case iFUSE:
		if (_useWith[WITH] == iSTAGNOLA) {
			removeIcon(iSTAGNOLA);
			replaceIcon(iFUSE, iFUSES);
			StartCharacterAction(hUSEGG, 0, 0, 1441);
			printSentence = false;
		}
		break;

	case iTOPO1C:
		if (_useWith[WITH] == iPATTINO) {
			removeIcon(iPATTINO);
			removeIcon(iTOPO1C);
			addIcon(iTOPO1D);
			StartCharacterAction(hUSEGG, 0, 0, 1497);
			printSentence = false;
		}
		break;

	case iPATTINO:
		if (_useWith[WITH] == iTOPO1C) {
			removeIcon(iPATTINO);
			removeIcon(iTOPO1C);
			addIcon(iTOPO1D);
			StartCharacterAction(hUSEGG, 0, 0, 1497);
			printSentence = false;
		}
		break;

	case iBAR11:
		if (_useWith[WITH] == iMAGNETE) {
			removeIcon(iBAR11);
			replaceIcon(iMAGNETE, iSBARRA21);
			StartCharacterAction(hUSEGG, 0, 0, 1438);
			printSentence = false;
		}
		break;

	case iMAGNETE:
		if (_useWith[WITH] == iBAR11) {
			removeIcon(iBAR11);
			replaceIcon(iMAGNETE, iSBARRA21);
			StartCharacterAction(hUSEGG, 0, 0, 1533);
			printSentence = false;
		}
		break;

	case iSIGARO:
		if (_useWith[WITH] == iSCOPA27) {
			removeIcon(iSCOPA27);
			replaceIcon(iSIGARO, iTORCIA32);
			StartCharacterAction(hUSEGG, 0, 0, 1575);
			printSentence = false;
		}
		break;

	case iSCOPA27:
		if (_useWith[WITH] == iSIGARO) {
			removeIcon(iSCOPA27);
			replaceIcon(iSIGARO, iTORCIA32);
			StartCharacterAction(hUSEGG, 0, 0, 1546);
			printSentence = false;
		}
		break;

	case iPROIETTORE31:
		if (_useWith[WITH] == iTRIPLA) {
			removeIcon(iTRIPLA);
			replaceIcon(iPROIETTORE31, iPROIETTORE35);
			StartCharacterAction(hUSEGG, 0, 0, 0);
			printSentence = false;
		}
		break;

	case iTRIPLA:
		if (_useWith[WITH] == iPROIETTORE31) {
			removeIcon(iTRIPLA);
			replaceIcon(iPROIETTORE31, iPROIETTORE35);
			StartCharacterAction(hUSEGG, 0, 0, 0);
			printSentence = false;
		}
		break;

	case iSALNITRO:
	case iCARBONE:
	case iZOLFO:
	case iCARSAL:
	case iCARZOL:
	case iSALZOL:
		if ((_useWith[WITH] == iSALNITRO) || (_useWith[WITH] == iCARBONE) || (_useWith[WITH] == iZOLFO) ||
			(_useWith[WITH] == iCARSAL) || (_useWith[WITH] == iCARZOL) || (_useWith[WITH] == iSALZOL)) {
			removeIcon(_useWith[USED]);
			removeIcon(_useWith[WITH]);

			if (((_useWith[USED] == iSALNITRO) && (_useWith[WITH] == iCARBONE)) ||
				((_useWith[WITH] == iSALNITRO) && (_useWith[USED] == iCARBONE)))
				addIcon(iCARSAL);
			if (((_useWith[USED] == iZOLFO) && (_useWith[WITH] == iCARBONE)) ||
				((_useWith[WITH] == iZOLFO) && (_useWith[USED] == iCARBONE)))
				addIcon(iCARZOL);
			if (((_useWith[USED] == iZOLFO) && (_useWith[WITH] == iSALNITRO)) ||
				((_useWith[WITH] == iZOLFO) && (_useWith[USED] == iSALNITRO)))
				addIcon(iSALZOL);

			if ((_useWith[USED] == iZOLFO) || (_useWith[WITH] == iZOLFO))
				addIcon(iBARATTOLO);
			if ((_useWith[USED] >= iCARSAL) || (_useWith[WITH] >= iCARSAL))
				addIcon(iPOLVERE48);
			StartCharacterAction(hUSEGG, 0, 0, 1663);
			printSentence = false;
		}
		break;

	case iPISTOLA4B:
		if (_useWith[WITH] == iPOLVERE48) {
			replaceIcon(iPOLVERE48, iPOLVERE4P);
			replaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1676);
			printSentence = false;
		} else if (_useWith[WITH] == iPOLVERE4P) {
			removeIcon(iPOLVERE4P);
			replaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1700);
			printSentence = false;
		}
		break;

	case iPOLVERE48:
		if (_useWith[WITH] == iPISTOLA4B) {
			replaceIcon(iPOLVERE48, iPOLVERE4P);
			replaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1676);
			printSentence = false;
		}
		break;

	case iPOLVERE4P:
		if (_useWith[WITH] == iPISTOLA4B) {
			removeIcon(iPOLVERE4P);
			replaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1700);
			printSentence = false;
		}
		break;

	case iBIGLIAA:
	case iBIGLIAB:
		if ((_useWith[WITH] == iPISTOLA4PC) && !(_inventoryObj[iPISTOLA4PC]._flag & OBJFLAG_EXTRA)) {
			removeIcon(_useWith[USED]);
			replaceIcon(iPISTOLA4PC, iPISTOLA4PD);
			StartCharacterAction(hUSEGG, 0, 0, 1683);
			_inventoryObj[iPISTOLA4PC]._flag |= OBJFLAG_EXTRA;
			printSentence = false;
		} else if (_useWith[WITH] == iPISTOLA4PC) {
			CharacterSay(1688);
			printSentence = false;
		} else if (_useWith[WITH] == iPISTOLA4B) {
			CharacterSay(2009);
			printSentence = false;
		}
		break;

	case iBIGLIA4U:
		if (_useWith[WITH] == iPISTOLA4PC) {
			removeIcon(iBIGLIA4U);
			replaceIcon(iPISTOLA4PC, iPISTOLA4PD);
			StartCharacterAction(hUSEGG, 0, 0, 1718);
			_inventoryObj[iPISTOLA4PD]._flag |= OBJFLAG_EXTRA;
			printSentence = false;
		} else if (_useWith[WITH] == iPISTOLA4B) {
			CharacterSay(2011);
			printSentence = false;
		}
		break;

	case iSIRINGA37:
		if (_useWith[WITH] == iFIALE) {
			removeIcon(iSIRINGA37);
			replaceIcon(iFIALE, iSIRINGA59);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printSentence = false;
		}
		break;

	case iFIALE:
		if (_useWith[WITH] == iSIRINGA37) {
			removeIcon(iSIRINGA37);
			replaceIcon(iFIALE, iSIRINGA59);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printSentence = false;
		}
		break;

	case iFILO:
		if (_useWith[WITH] == iGUANTI57) {
			removeIcon(iFILO);
			replaceIcon(iGUANTI57, iGUANTI5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printSentence = false;
		} else if (_useWith[WITH] == iSIRINGA59) {
			removeIcon(iFILO);
			replaceIcon(iSIRINGA59, iSIRINGA5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printSentence = false;
		}
		break;

	case iGUANTI57:
		if (_useWith[WITH] == iFILO) {
			removeIcon(iFILO);
			replaceIcon(iGUANTI57, iGUANTI5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printSentence = false;
		} else if (_useWith[WITH] == iSIRINGA5A) {
			removeIcon(iSIRINGA5A);
			replaceIcon(iGUANTI57, iARMAEVA);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printSentence = false;
		}
		break;

	case iSIRINGA59:
		if (_useWith[WITH] == iFILO) {
			removeIcon(iFILO);
			replaceIcon(iSIRINGA59, iSIRINGA5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printSentence = false;
		} else if (_useWith[WITH] == iGUANTI5A) {
			removeIcon(iSIRINGA59);
			replaceIcon(iGUANTI5A, iARMAEVA);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printSentence = false;
		}
		break;

	case iGUANTI5A:
		if (_useWith[WITH] == iSIRINGA59) {
			removeIcon(iSIRINGA59);
			replaceIcon(iGUANTI5A, iARMAEVA);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printSentence = false;
		}
		break;

	case iSIRINGA5A:
		if (_useWith[WITH] == iGUANTI57) {
			removeIcon(iSIRINGA5A);
			replaceIcon(iGUANTI57, iARMAEVA);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printSentence = false;
		}
		break;

	default:
		updateInv = false;
		break;
	}

	if (printSentence)
		CharacterSay(_inventoryObj[_useWith[USED]]._action);
	if (updateInv)
		setInventoryStart(_iconBase, INVENTORY_SHOW);
}

/*-------------------------------------------------------------------------*/
/*                          USE WITH / INV - SCR         				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::doInventoryUseWithScreen() {
	bool updateinv = true, printsent = true;

	if (!_useWith[USED] || !_useWith[WITH])
		warning("doInventoryUseWithScreen");

	_animMgr->stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);
	if (_characterInMovement)
		return;
	switch (_useWith[USED]) {
	case iBANCONOTE:
		if (_useWith[WITH] == oDISTRIBUTORE13 && !(_obj[oDISTRIBUTORE13]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a131USABANCONOTA, 0, 0, _useWith[WITH]);
			printsent = false;
			_obj[oDISTRIBUTORE13]._flag |= OBJFLAG_EXTRA;
		} else if ((_useWith[WITH] == oDISTRIBUTORE13) && (_obj[oDISTRIBUTORE13]._flag & OBJFLAG_EXTRA) && (_obj[oLATTINA13]._mode & OBJMODE_OBJSTATUS)) {
			CharacterSay(1410);
			printsent = false;
		} else if ((_useWith[WITH] == oDISTRIBUTORE13) && (_obj[oDISTRIBUTORE13]._flag & OBJFLAG_EXTRA)) {
			if (!(_obj[oSCOMPARTO13]._flag & OBJFLAG_EXTRA)) {
				_obj[oSCOMPARTO13]._flag |= OBJFLAG_EXTRA;
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1311DABOTTADISTRIBUTORE, 0, 0, _useWith[WITH]);
			} else
				CharacterSay(1411);

			printsent = false;
		} else if (_useWith[WITH] == oTICKETOFFICE16) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a165USABANCONOTA, 0, 0, _useWith[WITH]);
			_inventoryObj[iBANCONOTE]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if ((_useWith[WITH] == ocPOLIZIOTTO16) && (_inventoryObj[iBANCONOTE]._flag & OBJFLAG_EXTRA)) {
			_choice[62]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dPOLIZIOTTO16);
			printsent = false;
		}
		break;
	case iLETTER12:
		if (_useWith[WITH] == oPENPADA13) {
			if (_room[r14]._flag & OBJFLAG_DONE)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1312METTELETTERARICALCA, r14, 14, _useWith[WITH]);
			else {
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a135METTELETTERA, 0, 0, _useWith[WITH]);
				_obj[oLETTERA13]._mode |= OBJMODE_OBJSTATUS;
				_inventoryObj[iLETTER12]._flag |= OBJFLAG_EXTRA;
				removeIcon(iLETTER12);
				RegenRoom();
			}
			printsent = false;
		}
		break;
	case iFUSES:
		if (_useWith[WITH] == oPANELA12) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a127USEWRAPPEDFUSED, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;
	case iFUSE:
		if (_useWith[WITH] == oPANELA12) {
			CharacterSay(62);
			printsent = false;
		}
		break;
	case iKEY05:
		if (_useWith[WITH] == oBOXES12) {
			if (!(_obj[oBOXES12]._flag & OBJFLAG_EXTRA)) {
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a121BOXTEST, 0, 0, _useWith[WITH]);
				_obj[oBOXES12]._flag |= OBJFLAG_EXTRA;
				printsent = false;
			} else {
				CharacterSay(1426);
				printsent = false;
			}
		} else if (_useWith[WITH] == oBOX12 && !(_inventoryObj[iLETTER12]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a122APREBOX, 0, 0, oBOX12);
			_inventoryObj[iLETTER12]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if ((_useWith[WITH] == oBOX12) && (_inventoryObj[iLETTER12]._flag & OBJFLAG_EXTRA)) {
			CharacterSay(1429);
			printsent = false;
		} else if ((_useWith[WITH] == oCARA11) || (_useWith[WITH] == oCARB11) || (_useWith[WITH] == oTAKE12) || (_useWith[WITH] == oSTRONGBOXC15) || (_useWith[WITH] == oDOOR18) || (_useWith[WITH] == oPADLOCK1B) || (_useWith[WITH] == oDOORC21) || (_useWith[WITH] == oPANELC23) || (_useWith[WITH] == oDOOR2A) || (_useWith[WITH] == oDOORC33) || (_useWith[WITH] == oFRONTOFFICEC35) || (_useWith[WITH] == oCASSETTOC36) || (_useWith[WITH] == oPORTAC54) || (_useWith[WITH] == oPORTA57C55) || (_useWith[WITH] == oPORTA58C55) || (_useWith[WITH] == oPORTAS56) || (_useWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1426);
		}
		break;
	case iCARD03:
		if (_useWith[WITH] == oSLOT12 || _useWith[WITH] == oSLOT13 || _useWith[WITH] == oSLOT16) {
			printsent = false;
			_inventoryObj[iCARD03]._flag |= OBJFLAG_EXTRA;
			_obj[oSLOT12]._flag |= OBJFLAG_PERSON;
			_obj[oLIFTA12]._flag |= OBJFLAG_PERSON;
			doMouseTalk(_useWith[WITH]);
		} else if ((_useWith[WITH] == oTICKETOFFICE16) || (_useWith[WITH] == oSLOT23) || (_useWith[WITH] == oFRONTOFFICEA35) || (_useWith[WITH] == oSLOTA58) || (_useWith[WITH] == oSLOTB58)) {
			printsent = false;
			CharacterSay(1419);
		}
		break;
	case iPEN:
		printsent = false;
		if (((_useWith[WITH] == oPENPADA13) || (_useWith[WITH] == oLETTERA13)) && (_obj[oLETTERA13]._mode & OBJMODE_OBJSTATUS))
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a137RICALCAFIRMA, r14, 14, _useWith[WITH]);
		else if ((_useWith[WITH] == oPENPADA13) && (_room[r14]._flag & OBJFLAG_DONE))
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1312METTELETTERARICALCA, r14, 14, _useWith[WITH]);
		else if (_useWith[WITH] == oPENPADA13) {
			if (!(_obj[oBOX12]._mode & OBJMODE_OBJSTATUS)) {
				printsent = false;
				CharacterSay(2005);
			} else
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a134USAMAGNETICPEN, 0, 0, _useWith[WITH]);
		} else
			printsent = true;
		break;

	case iACIDO15:
		if (_useWith[WITH] == oBAR11) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a113USAFIALA, 0, 0, _useWith[WITH]);
			replaceIcon(iACIDO15, iFIALAMETA);
			//_animMgr->_animTab[a113USAFIALA]._atFrame[0]._index = 1483;
			printsent = false;
		} else if (_useWith[WITH] == oPADLOCK1B) {
			if (_obj[oTOMBINOA1B]._mode & OBJMODE_OBJSTATUS)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2AVERSAACIDO, 0, 0, _useWith[WITH]);
			else
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2VERSAACIDO, 0, 0, _useWith[WITH]);
			_obj[oBOTOLAC1B]._anim = a1B3APREBOTOLA;
			replaceIcon(iACIDO15, iFIALAMETA);
			_animMgr->_animTab[a113USAFIALA]._atFrame[0]._index = 1483;
			printsent = false;
		} else if ((_useWith[WITH] == ocGUARD18) || (_useWith[WITH] == oMANHOLEC1B)) {
			printsent = false;
			CharacterSay(1476);
		}
		break;

	case iFIALAMETA:
		if (_useWith[WITH] == oBAR11) {
			_animMgr->_animTab[a113USAFIALA]._atFrame[0]._index = 1483;
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a113USAFIALA, 0, 0, _useWith[WITH]);
			removeIcon(iFIALAMETA);
			printsent = false;
		} else if (_useWith[WITH] == oPADLOCK1B) {
			_animMgr->_animTab[a1B2AVERSAACIDO]._atFrame[2]._index = 1483;
			_animMgr->_animTab[a1B2VERSAACIDO]._atFrame[2]._index = 1483;
			if (_obj[oTOMBINOA1B]._mode & OBJMODE_OBJSTATUS)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2AVERSAACIDO, 0, 0, _useWith[WITH]);
			else
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2VERSAACIDO, 0, 0, _useWith[WITH]);
			_obj[oBOTOLAC1B]._anim = a1B3APREBOTOLA;
			removeIcon(iFIALAMETA);
			printsent = false;
		} else if ((_useWith[WITH] == oDOOR2A) || (_useWith[WITH] == oPORTA2B)) {
			printsent = false;
			CharacterSay(1508);
		}
		break;

	case iKEY15:
		if (_useWith[WITH] == oSTRONGBOXC15) {
			PlayDialog(dF151);
			_obj[oSTRONGBOXC15]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCASSAFORTEA15]._mode |= OBJMODE_OBJSTATUS;
			_obj[oSLOT13]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oASCENSOREA13]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oASCENSOREC13]._mode |= OBJMODE_OBJSTATUS;
			addIcon(iPISTOLA15);
			addIcon(iACIDO15);
			addIcon(iRAPPORTO);
			removeIcon(iKEY15);
			printsent = false;
			updateinv = false;
		} else if ((_useWith[WITH] == oCARA11) || (_useWith[WITH] == oCARB11) || (_useWith[WITH] == oTAKE12) || (_useWith[WITH] == oBOX12) || (_useWith[WITH] == oDOOR18) || (_useWith[WITH] == oPADLOCK1B) || (_useWith[WITH] == oDOORC21) || (_useWith[WITH] == oPANELC23) || (_useWith[WITH] == oDOOR2A) || (_useWith[WITH] == oDOORC33) || (_useWith[WITH] == oFRONTOFFICEC35) || (_useWith[WITH] == oCASSETTOC36) || (_useWith[WITH] == oPORTAC54) || (_useWith[WITH] == oPORTA57C55) || (_useWith[WITH] == oPORTA58C55) || (_useWith[WITH] == oPORTAS56) || (_useWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1469);
		}
		break;

	case iBAR11:
		if (_useWith[WITH] == oMANHOLEC1B) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B1USASBARRA, 0, 0, _useWith[WITH]);
			_obj[oBOTOLAA1B]._anim = a1B6ASCENDEBOTOLA;
			//_obj[oBOTOLAC1B]._anim = a1B3AAPREBOTOLA;
			printsent = false;
		} else if (_useWith[WITH] == oCATENAT21) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a216, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if (_useWith[WITH] == oALTOPARLANTE25) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a251, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if (_useWith[WITH] == oDOORC33) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a333LOSEBAR, 0, 0, _useWith[WITH]);
			removeIcon(iBAR11);
			printsent = false;
		} else if (_obj[_useWith[WITH]]._flag & OBJFLAG_PERSON) {
			CharacterSay(1436);
			printsent = false;
		} else if ((_useWith[WITH] == oTAKE12) || (_useWith[WITH] == oSTRONGBOXC15) || (_useWith[WITH] == oDOOR18) || (_useWith[WITH] == oPADLOCK1B) || (_useWith[WITH] == oDOORC21) || (_useWith[WITH] == oPANELC23) || (_useWith[WITH] == oDOOR2A) || (_useWith[WITH] == oPORTA2B)) {
			printsent = false;
			CharacterSay(1435);
		}
		break;

	case iCARD14:
		if ((_useWith[WITH] == oTICKETOFFICE16) && (_obj[oMAPPA16]._flag & OBJFLAG_EXTRA)) {
			if (_choice[49]._flag & OBJFLAG_DONE) {
				CharacterSay(1457);
				printsent = false;
			} else {
				_choice[46]._flag |= DLGCHOICE_HIDE;
				_choice[47]._flag |= DLGCHOICE_HIDE;
				_choice[48]._flag |= DLGCHOICE_HIDE;
				_choice[49]._flag &= ~DLGCHOICE_HIDE;
				PlayScript(s16CARD);
				//					doMouseTalk( _useWith[WITH] );
				printsent = false;
			}
		} else if (_useWith[WITH] == oSLOT23) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2311, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oSLOT12) || (_useWith[WITH] == oSLOT13) || (_useWith[WITH] == oSLOT16) || (_useWith[WITH] == oFRONTOFFICEA35) || (_useWith[WITH] == oSLOTA58) || (_useWith[WITH] == oSLOTB58)) {
			printsent = false;
			CharacterSay(1419);
		}
		break;

	case iMONETA13:
		if ((_useWith[WITH] == oTICKETOFFICE16) && (_obj[oMAPPA16]._flag & OBJFLAG_EXTRA)) {
			_choice[46]._flag |= DLGCHOICE_HIDE;
			_choice[47]._flag |= DLGCHOICE_HIDE;
			_choice[48]._flag |= DLGCHOICE_HIDE;
			_choice[50]._flag &= ~DLGCHOICE_HIDE;
			PlayScript(S16MONEY);
			//				doMouseTalk( _useWith[WITH] );
			printsent = false;
			removeIcon(iMONETA13);
			_obj[oFINGERPADP16]._flag |= OBJFLAG_ROOMOUT;
		} else if (_useWith[WITH] == oTICKETOFFICE16) {
			CharacterSay(146);
			printsent = false;
		}
		break;

	case iPLASTICA:
		if (_useWith[WITH] == oTELEFAXF17) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a172USAPLASTICA, 0, 0, _useWith[WITH]);
			_obj[oTELEFAXF17]._examine = 1486;
			printsent = false;
		}
		break;

	case iFOTO:
		if ((_useWith[WITH] == ocBARBONE17) && (_choice[81]._flag & OBJFLAG_DONE)) {
			CharacterSay(1463);
			printsent = false;
		} else if ((_useWith[WITH] == ocBARBONE17) && (_choice[91]._flag & OBJFLAG_DONE)) {
			_obj[ocBARBONE17]._action = 1462;
			CharacterSay(_obj[ocBARBONE17]._action);
			printsent = false;
		} else if ((_useWith[WITH] == ocBARBONE17) && (!(_choice[78]._flag & OBJFLAG_DONE) || ((_choice[79]._flag & OBJFLAG_DONE) || (_choice[83]._flag & OBJFLAG_DONE) && !(_choice[92]._flag & OBJFLAG_DONE)))) {
			_choice[78]._flag &= ~DLGCHOICE_HIDE;
			if ((_choice[79]._flag & OBJFLAG_DONE) || (_choice[83]._flag & OBJFLAG_DONE)) {
				_choice[92]._flag &= ~DLGCHOICE_HIDE;
				if (_choice[80]._flag & OBJFLAG_DONE)
					_choice[121]._flag &= ~DLGCHOICE_HIDE;
				else
					_choice[122]._flag &= ~DLGCHOICE_HIDE;
			} else
				_choice[91]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dBARBONE171);
			updateinv = false;
			printsent = false;
		} else if (_useWith[WITH] == ocPOLIZIOTTO16) {
			_obj[ocPOLIZIOTTO16]._flag |= OBJFLAG_EXTRA;
			CharacterSay(1461);
			if ((_choice[61]._flag & OBJFLAG_DONE) && (_choice[62]._flag & OBJFLAG_DONE) && (_obj[ocPOLIZIOTTO16]._flag & OBJFLAG_EXTRA))
				_obj[ocPOLIZIOTTO16]._mode &= ~OBJMODE_OBJSTATUS;
			printsent = false;
		} else if (_useWith[WITH] == ocGUARD18) {
			if (_choice[152]._flag & OBJFLAG_DONE)
				CharacterSay(1465);
			else
				CharacterSay(1464);
			printsent = false;
		} else if (_useWith[WITH] == ocNEGOZIANTE1A) {
			CharacterSay(1466);
			printsent = false;
		} else if (_useWith[WITH] == ocEVA19) {
			CharacterSay(1465);
			printsent = false;
		}

		break;

	case iLATTINA13:
		if (_useWith[WITH] == ocBARBONE17) {
			if ((_choice[79]._flag & OBJFLAG_DONE) || (_choice[83]._flag & OBJFLAG_DONE)) {
				_choice[80]._flag &= ~DLGCHOICE_HIDE;
				if (_choice[81]._flag & OBJFLAG_DONE) {
					_choice[81]._flag &= ~DLGCHOICE_HIDE;
					_choice[80]._flag &= ~DLGCHOICE_EXITDLG;
				}
				PlayDialog(dBARBONE171);
				updateinv = false;
				removeIcon(iLATTINA13);
				printsent = false;
			} else { // if( !(_choice[79]._flag & OBJFLAG_DONE) )
				_choice[79]._flag &= ~DLGCHOICE_HIDE;
				if (!(_choice[78]._flag & OBJFLAG_DONE)) {
					_choice[106]._flag &= ~DLGCHOICE_HIDE;
					_obj[ocBARBONE17]._action = 213;
				} else {
					_choice[107]._flag &= ~DLGCHOICE_HIDE;
					if (_choice[80]._flag & OBJFLAG_DONE)
						_choice[121]._flag &= ~DLGCHOICE_HIDE;
					else
						_choice[122]._flag &= ~DLGCHOICE_HIDE;
				}
				PlayDialog(dBARBONE171);
				updateinv = false;
				removeIcon(iLATTINA13);
				printsent = false;
			}
		}

		break;

	case iBOTTIGLIA14:
		if (_useWith[WITH] == ocBARBONE17) {
			if ((_choice[79]._flag & OBJFLAG_DONE) || (_choice[83]._flag & OBJFLAG_DONE)) {
				_choice[80]._flag &= ~DLGCHOICE_HIDE;
				if (_choice[81]._flag & OBJFLAG_DONE) {
					_choice[81]._flag &= ~DLGCHOICE_HIDE;
					_choice[80]._flag &= ~DLGCHOICE_EXITDLG;
				}
				PlayDialog(dBARBONE171);
				updateinv = false;
				removeIcon(iBOTTIGLIA14);
				printsent = false;
			} else {
				_choice[83]._flag &= ~DLGCHOICE_HIDE;
				if (!(_choice[78]._flag & OBJFLAG_DONE)) {
					_choice[106]._flag &= ~DLGCHOICE_HIDE;
					_obj[ocBARBONE17]._action = 213;
				} else {
					_choice[107]._flag &= ~DLGCHOICE_HIDE;
					if (_choice[80]._flag & OBJFLAG_DONE)
						_choice[121]._flag &= ~DLGCHOICE_HIDE;
					else
						_choice[122]._flag &= ~DLGCHOICE_HIDE;
				}
				PlayDialog(dBARBONE171);
				updateinv = false;
				removeIcon(iBOTTIGLIA14);
				printsent = false;
			}
		}

		break;

	case iBOTTIGLIA1D:
		if (_useWith[WITH] == ocNEGOZIANTE1A) {
			printsent = false;
			if (_choice[183]._flag & OBJFLAG_DONE) {
				_choice[185]._flag &= ~DLGCHOICE_HIDE;
				_dialog[dNEGOZIANTE1A]._startLen = 0;
				PlayDialog(dNEGOZIANTE1A);
				updateinv = false;
				_dialog[dNEGOZIANTE1A]._startLen = 1;
				replaceIcon(iBOTTIGLIA1D, iTESSERA);
			} else
				CharacterSay(2006);
		}
		break;

	case iTESSERA:
		if ((_useWith[WITH] == ocGUARD18) && !(_choice[155]._flag & OBJFLAG_DONE)) {
			_choice[155]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dGUARDIANO18);
			updateinv = false;
			_obj[ocGUARD18]._flag &= ~OBJFLAG_PERSON;
			_obj[oPORTAC18]._flag |= OBJFLAG_ROOMOUT;
			printsent = false;
		} else if (_useWith[WITH] == ocGUARD18) {
			CharacterSay(1494);
			printsent = false;
		}
		break;

	case iTOPO1D:
		if ((_useWith[WITH] == oDONNA1D) &&
			((mx >= _obj[oDONNA1D]._lim[0]) &&
			 (my >= _obj[oDONNA1D]._lim[1] + TOP) &&
			 (mx <= _obj[oDONNA1D]._lim[2]) &&
			 (my <= _obj[oDONNA1D]._lim[3] + TOP))) {
			PlayDialog(dF1D1);
			updateinv = false;
			removeIcon(iTOPO1D);
			read3D("1d2.3d"); // after skate
			_obj[oDONNA1D]._mode &= ~OBJMODE_OBJSTATUS;
			_room[_curRoom]._flag |= OBJFLAG_EXTRA;
			_animMgr->_animTab[aBKG1D]._flag |= SMKANIM_OFF1;
			printsent = false;
		}
		break;

	case iPISTOLA15:
		if ((_useWith[WITH] == oDOORC21) && !(_room[r21]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a211, 0, 0, _useWith[WITH]);
			_inventoryObj[iPISTOLA15]._examine = 1472;
			_inventoryObj[iPISTOLA15]._action = 1473;
			printsent = false;
		}
		break;

	case iCACCIAVITE:
		if (_useWith[WITH] == oESSE21) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a213, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if (_useWith[WITH] == oCOPERCHIOC31) {
			NLPlaySound(wCOVER31);
			_obj[oCOPERCHIOC31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCOPERCHIOA31]._mode |= OBJMODE_OBJSTATUS;
			RegenRoom();
			printsent = false;
		} else if (_useWith[WITH] == oCOPERCHIOA31) {
			_obj[oCOPERCHIOA31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCOPERCHIOC31]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPANNELLOM31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oPANNELLOMA31]._mode |= OBJMODE_OBJSTATUS;
			if (_obj[oFILOTC31]._mode & OBJMODE_OBJSTATUS) {
				_obj[oPANNELLOM31]._anim = 0;
				_obj[oPANNELLOM31]._examine = 715;
				_obj[oPANNELLOM31]._action = 716;
				_obj[oPANNELLOM31]._flag &= ~OBJFLAG_ROOMOUT;
			}
			NLPlaySound(wCOVER31);
			RegenRoom();
			printsent = false;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r31, a3118CHIUDEPANNELLO, 3, _curObj);
		} else if (_useWith[WITH] == oPANNELLO55) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a5512, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if (_useWith[WITH] == oPANNELLOC56) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a568, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oSLOT23) || (_useWith[WITH] == oPRESA35) || (_useWith[WITH] == oSERRATURA33)) {
			printsent = false;
			CharacterSay(1520);
		}
		break;

	case iESSE:
		if (_useWith[WITH] == oCATENA21) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a214, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iTANICHETTA27:
		if (_useWith[WITH] == oMANIGLIONE22) {
			if (_obj[oARMADIETTORC22]._mode & OBJMODE_OBJSTATUS)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a228, 0, 0, _useWith[WITH]);
			else
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a228A, 0, 0, _useWith[WITH]);
			removeIcon(iTANICHETTA27);
			addIcon(iTANICHETTA22);
			if (_inventoryObj[iLATTINA28]._flag & OBJFLAG_EXTRA) {
				removeIcon(iTANICHETTA22);
				replaceIcon(iLATTINA28, iLATTINE);
			}
			printsent = false;
		} else
			break;

	case iKEY22:
		if (_useWith[WITH] == oDOOR2A) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2A2USEKEY, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oPANELC23) || (_useWith[WITH] == oDOORC33) || (_useWith[WITH] == oFRONTOFFICEC35) || (_useWith[WITH] == oCASSETTOC36) || (_useWith[WITH] == oPORTAC54) || (_useWith[WITH] == oPORTA57C55) || (_useWith[WITH] == oPORTA58C55) || (_useWith[WITH] == oPORTAS56) || (_useWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1512);
		}
		break;

	case iLAMPADINA29:
		if ((_useWith[WITH] == oPORTALAMPADE2B) && !(_obj[_useWith[WITH]]._anim)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B6METTELAMPADINA, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iPIPEWRENCH:
		if (_useWith[WITH] == oPANELC23) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a233, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oDOORC33) || (_useWith[WITH] == oFRONTOFFICEC35) || (_useWith[WITH] == oCASSETTOC36) || (_useWith[WITH] == oPORTAC54) || (_useWith[WITH] == oPORTA57C55) || (_useWith[WITH] == oPORTA58C55) || (_useWith[WITH] == oPORTAS56) || (_useWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1525);
		}
		break;

	case iCAVI:
		if ((_useWith[WITH] == oCONTATTI23) && (_obj[oLEVAS23]._mode & OBJMODE_OBJSTATUS)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a236, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if (_useWith[WITH] == oLEVAG23) {
			CharacterSay(2015);
			printsent = false;
		}
		break;

	case iTRONCHESE:
		if ((_useWith[WITH] == oCAVO2H) && (_obj[oCARTELLONE2H]._mode & OBJMODE_OBJSTATUS)) {
			PlayDialog(dF2H1);
			_obj[oPASSAGE24]._mode |= OBJMODE_OBJSTATUS;
			_obj[omPASSAGGIO24]._mode |= OBJMODE_OBJSTATUS;
			_obj[oCARTELLONE24]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCARTELLONE2H]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oPASSERELLA24]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oMACERIE24]._mode &= ~OBJMODE_OBJSTATUS;
			//_obj[oPASSERELLA24]._flag &= ~OBJFLAG_ROOMOUT;
			//_obj[oPASSERELLA24]._anim = 0;
			printsent = false;
		} else if ((_useWith[WITH] == oTUBOT34) && (_obj[oVALVOLAC34]._mode & OBJMODE_OBJSTATUS)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a341USAPINZE, 0, 0, _useWith[WITH]);
			//_obj[oVALVOLAC34]._anim = 0;
			printsent = false;
		} else if ((_useWith[WITH] == oTUBOT34) && (_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS)) {
			CharacterSay(2007);
			printsent = false;
		} else
			printsent = true;
		break;

	case iLATTINA28:
		if ((_useWith[WITH] == oSERBATOIOA2G) && !(_inventoryObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2G4VERSALATTINA, 0, 0, _useWith[WITH]);
			_inventoryObj[iLATTINA28]._flag |= OBJFLAG_EXTRA;
			_inventoryObj[iLATTINA28]._examine = 1537;
			if (iconPos(iTANICHETTA22) != MAXICON) {
				removeIcon(iTANICHETTA22);
				replaceIcon(iLATTINA28, iLATTINE);
			}
			if ((_inventoryObj[iBOMBOLA]._flag & OBJFLAG_EXTRA) && (_inventoryObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
				_obj[oSERBATOIOA2G]._examine = 670;
				_obj[oSERBATOIOA2G]._action = 671;
			} else {
				_obj[oSERBATOIOA2G]._examine = 667;
				_obj[oSERBATOIOA2G]._action = 669;
			}
			printsent = false;
		}
		break;

	case iBOMBOLA:
		if (_useWith[WITH] == oSERBATOIOA2G) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2G5METTEBOMBOLA, 0, 0, _useWith[WITH]);
			_inventoryObj[iBOMBOLA]._flag |= OBJFLAG_EXTRA;
			if ((_inventoryObj[iBOMBOLA]._flag & OBJFLAG_EXTRA) && (_inventoryObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
				_obj[oSERBATOIOA2G]._examine = 670;
				_obj[oSERBATOIOA2G]._action = 671;
			} else {
				_obj[oSERBATOIOA2G]._examine = 668;
				_obj[oSERBATOIOA2G]._action = 669;
			}
			printsent = false;
		}
		break;

	case iCANDELOTTO:
		if ((_useWith[WITH] == oSERBATOIOA2G) && (_inventoryObj[iBOMBOLA]._flag & OBJFLAG_EXTRA) && (_inventoryObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r2GV, 0, 0, _useWith[WITH]);
			removeIcon(iCANDELOTTO);
			printsent = false;
		}
		break;

	case iFUCILE:
		if (_useWith[WITH] == oDINOSAURO2E) {
			PlayDialog(dF2E2);
			_obj[oDINOSAURO2E]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCATWALKA2E]._anim = a2E2PRIMAPALLONTANANDO;
			_animMgr->_animTab[aBKG2E]._flag |= SMKANIM_OFF2;
			printsent = false;
		}
		break;

	case iPINZA:
	case iSBARRA21:
		if (_useWith[WITH] == oCATENAT21) {
			if (_room[_curRoom]._flag & OBJFLAG_EXTRA) {
				if (_useWith[USED] == iPINZA)
					PlayDialog(dF212B);
				else
					PlayDialog(dF212);
				printsent = false;
			} else {
				if (_useWith[USED] == iPINZA)
					PlayDialog(dF213B);
				else
					PlayDialog(dF213);
				printsent = false;
			}
		} else if ((_useWith[WITH] == oDOORC33) && (_useWith[USED] == iSBARRA21)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a333LOSEBAR, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oSNAKEU52) && (_useWith[USED] == iPINZA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a527, 0, 0, _useWith[WITH]);
			_obj[oSCAVO51]._anim = a516;
			printsent = false;
		} else if ((_useWith[WITH] == oSERPENTEA52) && (_useWith[USED] == iPINZA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a528, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oSERPENTEB52) && (_useWith[USED] == iPINZA) && (iconPos(iSERPENTEA) == MAXICON)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a523, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iLAMPADINA2B:
		if (_useWith[WITH] == oPORTALAMPADE29) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a293AVVITALAMPADINA, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iLATTINA27:
		if ((_useWith[WITH] == oBRACIERES28) && (_obj[oBRACIERES28]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a286, 0, 0, _useWith[WITH]);
			_obj[oBRACIERES28]._examine = 456;
			printsent = false;
		} else
			printsent = true;
		break;

	case iTELECOMANDO2G:
		if (_useWith[WITH] == oTASTO2F) {
			if (_obj[oASCENSORE2F]._mode & OBJMODE_OBJSTATUS) {
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2F10PANNELLOSICHIUDE, 0, 0, _useWith[WITH]);
				_obj[oBIDONE2F]._anim = a2F5CFRUGABIDONE;
			} else {
				if (!(_inventoryObj[iTELECOMANDO2G]._flag & OBJFLAG_EXTRA))
					doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2F9PPANNELLOSIAPRE, 0, 0, _useWith[WITH]);
				else
					doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2F9PANNELLOSIAPRE, 0, 0, _useWith[WITH]);
				_inventoryObj[iTELECOMANDO2G]._flag |= OBJFLAG_EXTRA;
				_obj[oBIDONE2F]._anim = a2F5FRUGABIDONE;
			}
			printsent = false;
		}
		break;

	case iSAMROTTO:
		if (_useWith[WITH] == oSERRATURA33) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a338POSASAM, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iTORCIA32:
		if ((_useWith[WITH] == oSENSOREV32) && (_obj[oFILOTC31]._mode & OBJMODE_OBJSTATUS)) {
			_obj[oPANNELLOMA31]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPANNELLOM31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oPANNELLOMA31]._examine = 717;
			_obj[oPANNELLOMA31]._action = 718;
			_obj[oCORPO31]._mode |= OBJMODE_OBJSTATUS;
			_obj[od31ALLA35]._mode |= OBJMODE_OBJSTATUS;
			_obj[omd31ALLA35]._mode |= OBJMODE_OBJSTATUS;
			_obj[oMONTACARICHI31]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPANNELLO31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oPANNELLON31]._mode &= ~OBJMODE_OBJSTATUS;
			_room[r32]._flag |= OBJFLAG_EXTRA;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r31, 0, 11, _useWith[WITH]);

			printsent = false;
		}
		break;

	case iPROIETTORE31:
		if (_useWith[WITH] == oPRESA35) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a351PROVAPROIETTOREPRESA, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if (_useWith[WITH] == oTRIPLA35) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a355ATTPROIETTORETRIPLAEPRESA, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oRIBELLEA35)) {
			printsent = false;
			CharacterSay(1578);
		}
		break;

	case iPROIETTORE35:
		if (_useWith[WITH] == oPRESA35) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a352ATTPROIETTOREETRIPLAPRESA, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oRIBELLEA35)) {
			printsent = false;
			CharacterSay(1590);
		}
		break;

	case iTRIPLA:
		if (_useWith[WITH] == oPRESA35) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a353ATTACCATRIPLAPRESA, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iLASER35:
		if (_useWith[WITH] == oFRONTOFFICEC35) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a3511APRESPORTELLO, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if (_useWith[WITH] == oSNAKEU52) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a522, 0, 0, _useWith[WITH]);
			_obj[oSCAVO51]._anim = a516;
			printsent = false;
		} else if (_useWith[WITH] == oLUCCHETTO53) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a532, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oPORTAMC36) || (_useWith[WITH] == oPORTALC36) || (_useWith[WITH] == oSCANNERMA36) || (_useWith[WITH] == oSCANNERLA36) || (_useWith[WITH] == oCASSETTOC36) || (_useWith[WITH] == oRETE52) || (_useWith[WITH] == oTELECAMERA52) || (_useWith[WITH] == oSERPENTET52) || (_useWith[WITH] == oLAGO53)) {
			printsent = false;
			CharacterSay(1597);
		}
		break;

	case iKEY35:
		if (_useWith[WITH] == oCASSETTOC36) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a364APRECASSETTO, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oFRONTOFFICEC35) || (_useWith[WITH] == oPORTAC54) || (_useWith[WITH] == oPORTA57C55) || (_useWith[WITH] == oPORTA58C55) || (_useWith[WITH] == oPORTAS56) || (_useWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1594);
		}
		break;

	case iSTETOSCOPIO:
		if (_useWith[WITH] == oPORTALC36) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a368USASTETOSCOPIO, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iCARD36:
		if ((_useWith[WITH] == oFRONTOFFICEA35) && !(_obj[oFRONTOFFICEA35]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r35P, 0, 10, _useWith[WITH]);
			removeIcon(iCARD36);
			printsent = false;
		} else if ((_useWith[WITH] == oFRONTOFFICEA35) && (_obj[oFRONTOFFICEA35]._flag & OBJFLAG_EXTRA)) {
			CharacterSay(1844);
			printsent = false;
		} else if ((_useWith[WITH] == oSLOTA58) || (_useWith[WITH] == oSLOTB58)) {
			printsent = false;
			CharacterSay(1602);
		}
		break;

	case iMONETA4L:
		if ((_useWith[WITH] == oFESSURA41) &&
			((_obj[oFUCILE42]._anim != 0) && (_obj[oFUCILE42]._anim != a428) && (_obj[oFUCILE42]._anim != a429))) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a412, 0, 0, _useWith[WITH]);
			if (_obj[oZAMPA41]._mode & OBJMODE_OBJSTATUS)
				_obj[oSLOT41]._anim = a417;
			else if (_slotMachine41Counter <= 2)
				_obj[oSLOT41]._anim = a414;
			else
				CharacterSay(2015);
			_slotMachine41Counter++;
			printsent = false;
		} else if ((_useWith[WITH] == oFESSURA41) && ((_obj[oFUCILE42]._anim == 0) || (_obj[oFUCILE42]._anim == a428) || (_obj[oFUCILE42]._anim == a429))) {
			CharacterSay(2010);
			printsent = false;
		} else if (_useWith[WITH] == oFESSURA42) {
			CharacterSay(924);
			printsent = false;
		} else if (_useWith[WITH] == oCAMPANA4U) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U3, 0, 0, _useWith[WITH]);
			_obj[oCAMPANA4U]._flag |= OBJFLAG_EXTRA;
			if ((_obj[oCAMPANA4U]._flag & OBJFLAG_EXTRA) && (_inventoryObj[iBIGLIAA]._flag & OBJFLAG_EXTRA)) {
				_obj[oCAMPANA4U]._examine = 1202;
				_obj[oCAMPANA4U]._action = 1203;
			} else
				_obj[oCAMPANA4U]._examine = 1200;
			printsent = false;
		}
		break;

	case iMARTELLO:
		if ((_useWith[WITH] == oRAGNO41) && !(_obj[oRAGNO41]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a416, 0, 0, _useWith[WITH]);
			_obj[oRAGNO41]._flag |= OBJFLAG_EXTRA;
			_obj[oRAGNO41]._anim = 0;
			if (_obj[oSLOT41]._anim == a414)
				_obj[oSLOT41]._anim = a417;
			printsent = false;
		} else if ((_useWith[WITH] == oSLOT41) || (_useWith[WITH] == oVETRINETTA42) || (_useWith[WITH] == oTAMBURO43) || (_useWith[WITH] == oSFIATO45) || (_useWith[WITH] == oPORTAC4A) || (_useWith[WITH] == oPORTAC4B) || (_useWith[WITH] == oSERRATURA4B) || (_useWith[WITH] == oLICANTROPO4P)) {
			printsent = false;
			CharacterSay(1619);
		}
		break;

	case iMONETE:
		if ((_useWith[WITH] == oFESSURA42) && (_obj[oFUCILE42]._anim == a427)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a421, 0, 0, _useWith[WITH]);
			if (_obj[oPOLTIGLIA42]._mode & OBJMODE_OBJSTATUS)
				_obj[oFUCILE42]._anim = a429;
			else
				_obj[oFUCILE42]._anim = a428;
			printsent = false;
		}
		break;

	case iPOLTIGLIA:
		if (_useWith[WITH] == oGUIDE42) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a423, 0, 0, _useWith[WITH]);
			if (_obj[oFUCILE42]._anim != a427)
				_obj[oFUCILE42]._anim = a429;
			printsent = false;
		}
		break;

	case iMAZZA:
		if ((_useWith[WITH] == oTAMBURO43) && !(_obj[oTAMBURO43]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a435, 0, 0, _useWith[WITH]);
			_obj[oTAMBURO43]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if ((_useWith[WITH] == oPORTAC4B) || (_useWith[WITH] == oSERRATURA4B) || (_useWith[WITH] == oLICANTROPO4P)) {
			printsent = false;
			CharacterSay(1679);
		}
		break;

	case iPUPAZZO:
		if (_useWith[WITH] == oCASSETTOAA44) {
			replaceIcon(iPUPAZZO, iTELECOMANDO44);
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a442, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iSTRACCIO:
		if (_useWith[WITH] == oMANOPOLAR45) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r45S, 0, 2, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oCAMPANA4U) && (_inventoryObj[iBIGLIAA]._flag & OBJFLAG_EXTRA) && (_obj[oCAMPANA4U]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U5, 0, 0, _useWith[WITH]);
			_obj[oCAMPANA4U]._examine = 1204;
			_obj[oCAMPANA4U]._action = 1205;
			printsent = false;
		} else if (_useWith[WITH] == oCAMPANA4U) {
			CharacterSay(1713);
			printsent = false;
		} else if ((_useWith[WITH] == oRAGNO46) || (_useWith[WITH] == oLICANTROPO4P)) {
			printsent = false;
			CharacterSay(1711);
		}
		break;

	case iTESCHIO:
		if (_useWith[WITH] == oPIASTRELLA48) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4810, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iTORCIA47:
		if (_useWith[WITH] == oTESCHIO48) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4811, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oCALDAIAS45) || (_useWith[WITH] == oRAGNO46)) {
			printsent = false;
			CharacterSay(1640);
		}
		break;

	case iFIAMMIFERO:
		if (_useWith[WITH] == oTORCIAS48) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4812, 0, 0, _useWith[WITH]);
			_obj[oTORCIAS48]._lim[0] = 0;
			_obj[oTORCIAS48]._lim[1] = 0;
			_obj[oTORCIAS48]._lim[2] = 0;
			_obj[oTORCIAS48]._lim[3] = 0;
			printsent = false;
		}
		break;

	case iASTA:
		if (_useWith[WITH] == oMERIDIANA49) {
			removeIcon(iASTA);
			StartCharacterAction(a491, r49M, 1, 0);
			printsent = false;
		}
		break;

	case iPISTOLA4PD:
		if ((_useWith[WITH] == oLICANTROPO4P) && (_inventoryObj[iPISTOLA4PD]._flag & OBJFLAG_EXTRA)) {
			replaceIcon(iPISTOLA4PD, iPISTOLA4B);
			_obj[oLICANTROPO4P]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oLICANTROPOM4P]._mode |= OBJMODE_OBJSTATUS;
			_obj[oSANGUE4P]._mode |= OBJMODE_OBJSTATUS;
			_room[r4P]._flag |= OBJFLAG_EXTRA;
			read3D("4p2.3d"); // after werewolf
			_animMgr->_animTab[aBKG4P]._flag |= SMKANIM_OFF1;
			PlayDialog(dF4P2);
			printsent = false;
		} else if (_useWith[WITH] == oLICANTROPO4P) {
			replaceIcon(iPISTOLA4PD, iPISTOLA4B);
			PlayDialog(dF4P1);
			printsent = false;
		} else if ((_useWith[WITH] == oRAGNO46) || (_useWith[WITH] == oPORTAC4B) || (_useWith[WITH] == oSERRATURA4B)) {
			printsent = false;
			CharacterSay(1706);
		}
		break;

	case iBARATTOLO:
		if ((_useWith[WITH] == oSANGUE4P) || (_useWith[WITH] == oLICANTROPOM4P)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4P7, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iCAMPANA:
		if (_useWith[WITH] == oPOZZA4U) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U2, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iBIGLIAA:
	case iBIGLIAB:
		if ((_useWith[WITH] == oCAMPANA4U) && (_inventoryObj[iBIGLIAA]._flag & OBJFLAG_EXTRA)) {
			CharacterSay(1684);
			printsent = false;
		} else if (_useWith[WITH] == oCAMPANA4U) {
			removeIcon(_useWith[USED]);
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U4, 0, 0, _useWith[WITH]);
			_inventoryObj[iBIGLIAA]._flag |= OBJFLAG_EXTRA;
			if ((_obj[oCAMPANA4U]._flag & OBJFLAG_EXTRA) && (_inventoryObj[iBIGLIAA]._flag & OBJFLAG_EXTRA)) {
				_obj[oCAMPANA4U]._examine = 1202;
				_obj[oCAMPANA4U]._action = 1203;
			} else
				_obj[oCAMPANA4U]._examine = 1201;
			printsent = false;
		}
		break;

	case iPAPAVERINA:
		if ((_useWith[WITH] == oCIOCCOLATINI4A) && ((_animMgr->_curAnimFrame[0] < 370) || (_animMgr->_curAnimFrame[0] > 480))) {
			PlayScript(s4AHELLEN);
			_obj[oPULSANTE4A]._anim = a4A3;
			printsent = false;
		} else if (_useWith[WITH] == oCIOCCOLATINI4A) {
			_obj[oPULSANTE4A]._anim = a4A3;
			printsent = false;
			_obj[oCIOCCOLATINI4A]._flag |= OBJFLAG_EXTRA;
		}
		break;

	case iSANGUE:
		if (_useWith[WITH] == oSERRATURA4B) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B4, 0, 0, _useWith[WITH]);
			_obj[oPORTAC4B]._anim = a4B5;
			printsent = false;
		}
		break;

	case iDIAPA4B:
		if ((_useWith[WITH] == oPROIETTORE4B) && (_obj[oPROIETTORE4B]._anim < a4B9A)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B6A, 0, 0, _useWith[WITH]);
			_obj[oPROIETTORE4B]._anim = a4B9A;
			printsent = false;
		}
		break;

	case iDIAPB4B:
		if ((_useWith[WITH] == oPROIETTORE4B) && (_obj[oPROIETTORE4B]._anim < a4B9A)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B6B, 0, 0, _useWith[WITH]);
			_obj[oPROIETTORE4B]._anim = a4B9B;
			printsent = false;
		}
		break;

	case iDIAP4C:
		if ((_useWith[WITH] == oPROIETTORE4B) && (_obj[oPROIETTORE4B]._anim < a4B9A)) {
			//
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B6C, 0, 0, _useWith[WITH]);
			_obj[oPROIETTORE4B]._anim = a4B9C;
			printsent = false;
		}
		break;

	case iUOVO:
		if ((_useWith[WITH] == oRETE52) || (_useWith[WITH] == oSERPENTET52)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a521, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iSERPENTEA:
		if (_useWith[WITH] == oLAGO53) {
			printsent = false;
			if (!(_obj[oLUCCHETTO53]._mode & OBJMODE_OBJSTATUS)) {
				StartCharacterAction(a533, r54, 11, 0);
				removeIcon(_useWith[USED]);
			} else if (_useWith[USED] != iSERPENTEB) {
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a534, 0, 0, _useWith[WITH]);
				removeIcon(_useWith[USED]);
				_obj[oLAGO53]._examine = 1237;
			} else
				CharacterSay(1740);
		}
		break;

	case iSERPENTEB:
		if (_useWith[WITH] == oLAGO53) {
			printsent = false;
			if (!(_obj[oLUCCHETTO53]._mode & OBJMODE_OBJSTATUS)) {
				StartCharacterAction(a533C, r54, 11, 0);
				removeIcon(_useWith[USED]);
			} else if (_useWith[USED] != iSERPENTEB) {
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a534, 0, 0, _useWith[WITH]);
				removeIcon(_useWith[USED]);
				_obj[oLAGO53]._examine = 1237;
			} else
				CharacterSay(1740);
		}
		break;

	case iSAPONE:
		if (_useWith[WITH] == oSECCHIOA54) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a543, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iLATTINE:
		if (_useWith[WITH] == oLAVATRICEF54) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a546, 0, 0, _useWith[WITH]);
			_obj[oLAVATRICEL54]._anim = a547;
			printsent = false;
		}
		break;

	case iCHIAVI:
		if (_useWith[WITH] == oPORTAS56) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a563, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if (_useWith[WITH] == oPORTA57C55) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a551, r57, 17, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oPORTA58C55) && (!(_choice[871]._flag & OBJFLAG_DONE) || (_choice[901]._flag & OBJFLAG_DONE))) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a552, 0, 0, _useWith[WITH]);
			printsent = false;
		} else if ((_useWith[WITH] == oPORTA58C55) && (_choice[871]._flag & OBJFLAG_DONE)) {
			CharacterSay(1287);
			printsent = false;
		}
		break;

	case iMDVD:
		if ((_useWith[WITH] == oTASTIERA56) && (_choice[260]._flag & OBJFLAG_DONE) && !(_choice[262]._flag & OBJFLAG_DONE) && (_inventoryObj[iMDVD]._examine != 1752)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a566, 0, 0, _useWith[WITH]);
			_choice[262]._flag &= ~DLGCHOICE_HIDE;
			_inventoryObj[iMDVD]._examine = 1752;
			printsent = false;
		} else if ((_useWith[WITH] == oTASTIERA56) && (_inventoryObj[iMDVD]._examine == 1752)) {
			CharacterSay(1753);
			printsent = false;
		} else
			printsent = true;
		break;

	case iTESTER:
		if ((_useWith[WITH] == oPANNELLOA) && (_choice[856]._flag & OBJFLAG_DONE)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a569, 0, 0, _useWith[WITH]);
			_obj[oPANNELLOA]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		}
		break;

	case iCUTTER:
		if ((_useWith[WITH] == oPANNELLOA) && (_obj[oPANNELLOA]._flag & OBJFLAG_EXTRA)) {
			PlayDialog(dF562);
			_obj[oPANNELLOA]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCAVOTAGLIATO56]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPORTA58C55]._mode |= OBJMODE_OBJSTATUS;
			setPosition(6);
			removeIcon(iCUTTER);
			printsent = false;
		} else if (_useWith[WITH] == oPANNELLOA) {
			CharacterSay(2012);
			printsent = false;
		}
		break;

	case iGUANTI59:
		if (_useWith[WITH] == oBOMBOLA57) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a575, 0, 0, _useWith[WITH]);
			printsent = false;
		}
		break;

	case iARMAEVA:
		if ((_useWith[WITH] == oFINESTRAA5A) && (_choice[871]._flag & OBJFLAG_DONE) && !(_choice[286]._flag & OBJFLAG_DONE)) {
			removeIcon(iARMAEVA);
			PlayDialog(dC5A1);
			_obj[oFINESTRAA58]._anim = a587;
			printsent = false;
			_room[r5A]._flag |= OBJFLAG_EXTRA;
		} else if ((_useWith[WITH] == oFINESTRAA5A) && (_choice[871]._flag & OBJFLAG_DONE)) {
			removeIcon(iARMAEVA);
			PlayDialog(dF5A1);
			_obj[oFINESTRAA58]._anim = a587;
			printsent = false;
		}
		break;

	default:
		updateinv = false;
		break;
	}

	if (printsent)
		CharacterSay(_inventoryObj[_useWith[USED]]._action);

	if (updateinv)
		setInventoryStart(_iconBase, INVENTORY_SHOW);
}

} // End of namespace Trecision
