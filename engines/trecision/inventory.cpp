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
#include "logic.h"

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
			if (!(isInventoryArea(my)))
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
		clearText();
		lastobj = 0;
	}

	if (FlagUseWithStarted && !FlagUseWithLocked) {
		if (!showhide) {
			clearText();
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
			clearText();
		addText(posx, posy, locsent, COLOR_INVENTORY, MASKCOL);
	} else {
		if (obj == lastinv)
			return;

		if (!obj || !showhide) {
			clearText();
			lastinv = 0;
			return;
		}
		posx = ICONMARGSX + ((iconPos(_curInventory) - _iconBase) * (ICONDX)) + ICONDX / 2;
		posy = MAXY - CARHEI;
		lastinv = obj;
		LenText = TextLength(_objName[_inventoryObj[obj]._name], 0);

		posx = CLIP(posx - (LenText / 2), 2, SCREENLEN - 2 - LenText);

		if (lastinv)
			clearText();

		if (_inventoryObj[obj]._name)
			addText(posx, posy, _objName[_inventoryObj[obj]._name], COLOR_INVENTORY, MASKCOL);
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
	if (!_useWith[USED] || !_useWith[WITH])
		warning("doInventoryUseWithScreen - useWith not set properly");

	_animMgr->stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);
	if (_characterInMovement)
		return;
	
	bool refreshInventory, printSentence;
	_logicMgr->useWithScreen(&refreshInventory, &printSentence);

	if (printSentence)
		CharacterSay(_inventoryObj[_useWith[USED]]._action);

	if (refreshInventory)
		setInventoryStart(_iconBase, INVENTORY_SHOW);
}

} // End of namespace Trecision
