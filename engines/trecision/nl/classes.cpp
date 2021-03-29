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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/trecision.h"

#include "common/config-manager.h"
#include "trecision/video.h"

namespace Trecision {

// inventory
#define COUNTERSTEP 6

bool ForceQuit = false;
int NlVer = 102;

/*-------------------------------------------------------------------------*/
/*                                DOACTION           					   */
/*-------------------------------------------------------------------------*/
void doAction() {
	if ((g_vm->_curMessage->_event == ME_MOUSEOPERATE) || (g_vm->_curMessage->_event == ME_MOUSEEXAMINE)) {
		// Action in the game area
		g_vm->_curObj = g_vm->_curMessage->_u32Param;
		if (g_vm->_curObj == oLASTLEV5)
			CharacterSay(2003);

		if ((!g_vm->_curObj) || (!(g_vm->_obj[g_vm->_curObj]._mode & OBJMODE_OBJSTATUS)))
			return;

		if (g_vm->_obj[g_vm->_curObj]._mode & OBJMODE_HIDDEN)
			g_vm->_obj[g_vm->_curObj]._mode &= ~OBJMODE_HIDDEN;

		if (FlagUseWithStarted) {
			if ((g_vm->_obj[g_vm->_curObj]._flag & (OBJFLAG_ROOMOUT | OBJFLAG_ROOMIN)) && !(g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_EXAMINE))
				return;
			FlagUseWithStarted = false;
			g_vm->_flagInventoryLocked = false;
			g_vm->_useWith[WITH] = g_vm->_curObj;
			g_vm->_useWithInv[WITH] = false;
			g_vm->_lightIcon = 0xFF;

			if (!g_vm->_useWithInv[USED] && (g_vm->_curObj == g_vm->_useWith[USED])) {
				g_vm->_useWith[USED] = 0;
				g_vm->_useWith[WITH] = 0;
				g_vm->_useWithInv[USED] = false;
				g_vm->_useWithInv[WITH] = false;
				FlagUseWithStarted = false;
				g_vm->clearText();
			} else
				doEvent(MC_ACTION, ME_USEWITH, MP_SYSTEM, 0, 0, 0, 0);
			g_vm->_curObj = 0;
			return;
		}

		if ((g_vm->_curMessage->_event == ME_MOUSEOPERATE) && (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_USEWITH)) {
			FlagUseWithStarted = true;
			g_vm->_flagInventoryLocked = true;
			g_vm->_useWith[USED] = g_vm->_curObj;
			g_vm->_useWith[WITH] = 0;
			g_vm->_useWithInv[USED] = false;
			g_vm->_useWithInv[WITH] = false;
			ShowObjName(g_vm->_curObj, true);
			return;
		}
	}

	switch (g_vm->_curMessage->_event) {
	case ME_MOUSEOPERATE:
		if (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_ROOMIN)
			doRoomIn(g_vm->_curObj);
		else if (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_PERSON)
			doMouseTalk(g_vm->_curObj);
		else if (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_ROOMOUT)
			doRoomOut(g_vm->_curObj);
		else if (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_TAKE)
			doMouseTake(g_vm->_curObj);
		else
			doMouseOperate(g_vm->_curObj);
		break;

	case ME_MOUSEEXAMINE:
		if (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_EXAMINE)
			doMouseExamine(g_vm->_curObj);
		else if (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_ROOMIN)
			doRoomIn(g_vm->_curObj);
		else if (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_PERSON)
			doMouseExamine(g_vm->_curObj);
		else if (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_ROOMOUT)
			doRoomOut(g_vm->_curObj);
		else
			doMouseExamine(g_vm->_curObj);
		break;

	case ME_INVOPERATE:
		doInvOperate();
		break;

	case ME_INVEXAMINE:
		doInvExamine();
		break;

	case ME_USEWITH:
		ShowObjName(0, false);
		doUseWith();
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                                  DOMOUSE           					   */
/*-------------------------------------------------------------------------*/
void doMouse() {
#define POSUP  0
#define POSGAME 1
#define POSINV 2

	switch (g_vm->_curMessage->_event) {
	case ME_MMOVE:
		int8 curpos;
		if (GAMEAREA(g_vm->_curMessage->_u16Param2))
			curpos = POSGAME;
		else if (isInventoryArea(g_vm->_curMessage->_u16Param2))
			curpos = POSINV;
		else
			curpos = POSUP;

		if (curpos == POSGAME) {
		// Game area
			if (FlagSomeOneSpeak || FlagDialogMenuActive || FlagDialogActive || FlagUseWithLocked)
				break;

			CheckMask(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2);
			// For the wheel in 2C
			if ((g_vm->_curObj >= oWHEEL1A2C) && (g_vm->_curObj <= oWHEEL12C2C))
				ShowObjName((oWHEEL1A2C % 3) + oWHEELA2C, true);
			// For the displacer
			else if (g_vm->_curRoom == r41D) {
				if ((g_vm->_curObj >= oPULSANTE1AD) && (g_vm->_curObj <= oPULSANTE33AD)) {
					if (!(g_vm->_obj[oROOM41 + g_vm->_obj[g_vm->_curObj]._goRoom - r41]._mode & OBJMODE_OBJSTATUS)) {
						for (int a = oROOM41; a <= oROOM4X; a++) {
							if (g_vm->_obj[a]._mode & OBJMODE_OBJSTATUS)
								g_vm->_obj[a]._mode &= ~OBJMODE_OBJSTATUS;
						}
						g_vm->_obj[oROOM45B]._mode &= ~OBJMODE_OBJSTATUS;

						if (((oROOM41 + g_vm->_obj[g_vm->_curObj]._goRoom - r41) == oROOM45) && (g_vm->_obj[od44ALLA45]._goRoom == r45S))
							g_vm->_obj[oROOM45B]._mode |= OBJMODE_OBJSTATUS;
						else
							g_vm->_obj[oROOM41 + g_vm->_obj[g_vm->_curObj]._goRoom - r41]._mode |= OBJMODE_OBJSTATUS;
						RegenRoom();
					}
				} else {
					for (int a = oROOM41; a <= oROOM4X; a++) {
						if (g_vm->_obj[a]._mode & OBJMODE_OBJSTATUS)
							g_vm->_obj[a]._mode &= ~OBJMODE_OBJSTATUS;
					}
					g_vm->_obj[oROOM45B]._mode &= ~OBJMODE_OBJSTATUS;

					RegenRoom();
				}
				ShowObjName(g_vm->_curObj, true);
			} else
			// not a wheel nor the displacer
				ShowObjName(g_vm->_curObj, true);

			if (g_vm->_inventoryStatus == INV_INACTION)
				doEvent(MC_INVENTORY, ME_CLOSE, MP_DEFAULT, 0, 0, 0, 0);
		} else if (curpos == POSINV) {
		// Inventory area
			if (!FlagCharacterExist && ((g_vm->_curRoom != r31P) && (g_vm->_curRoom != r35P)))
				break; // When it's in a room without a character, such as the map
			if ((FlagSomeOneSpeak && !FlagCharacterSpeak) || FlagDialogMenuActive || FlagDialogActive || FlagUseWithLocked)
				break;
			if (g_vm->_animMgr->_playingAnims[1])
				break;

			if (g_vm->_inventoryStatus == INV_OFF)
				doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			else if (g_vm->_inventoryStatus == INV_INACTION)
				doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, 0);
		}	else {
		// Up area
			if (g_vm->_curRoom == rSYS)
				break;

			g_vm->_curObj = 0;
			ShowObjName(g_vm->_curObj, true);

			if (FlagDialogMenuActive)
				UpdateChoices(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2);
		}
		break;

	case ME_MRIGHT:
	case ME_MLEFT:
		if (FlagSomeOneSpeak) {
			Flagskiptalk = Flagskipenable;
			break;
		}
		if (_actor._curAction > hWALKIN)
			break;
		if (FlagWalkNoInterrupt && (_actor._curAction != hSTAND))
			break;

		if ((FlagDialogActive) && (FlagDialogMenuActive)) {
			SelectChoice(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2);
			break;
		}
		//  for the displacer
		if ((g_vm->_curObj >= oPULSANTE1AD) && (g_vm->_curObj <= oPULSANTE33AD)) {
			if ((g_vm->_obj[g_vm->_curObj]._goRoom == r45) && (g_vm->_obj[od44ALLA45]._goRoom == r45S) &&
			    (g_vm->_obj[oEXIT41D]._goRoom == r45S) && (g_vm->_curMessage->_event == ME_MRIGHT))
				doEvent(MC_ACTION, ME_MOUSEOPERATE, MP_DEFAULT, 0, 0, 0, g_vm->_curObj);
			else if ((g_vm->_obj[g_vm->_curObj]._goRoom == r45) && (g_vm->_obj[od44ALLA45]._goRoom == r45S) &&
			         (g_vm->_obj[oEXIT41D]._goRoom != r45S) && (g_vm->_curMessage->_event == ME_MRIGHT)) {
				g_vm->_obj[oEXIT41D]._goRoom = r45S;
				g_vm->_inventoryObj[iDISLOCATORE]._flag |= OBJFLAG_EXTRA;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r45S, 0, 0, g_vm->_curObj);
			} else if (g_vm->_obj[oEXIT41D]._goRoom != g_vm->_obj[g_vm->_curObj]._goRoom && (g_vm->_curMessage->_event == ME_MRIGHT)) {
				g_vm->_obj[oEXIT41D]._goRoom = g_vm->_obj[g_vm->_curObj]._goRoom;
				g_vm->_inventoryObj[iDISLOCATORE]._flag |= OBJFLAG_EXTRA;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT41D]._goRoom, 0, 0, g_vm->_curObj);
			} else if ((g_vm->_curMessage->_event == ME_MLEFT) && g_vm->_curObj)
				doEvent(MC_ACTION, ME_MOUSEEXAMINE, MP_DEFAULT, 0, 0, 0, g_vm->_curObj);
			else if ((g_vm->_curMessage->_event == ME_MRIGHT) && g_vm->_curObj)
				doEvent(MC_ACTION, ME_MOUSEOPERATE, MP_DEFAULT, 0, 0, 0, g_vm->_curObj);
			break;
		}
		// end of displacer

		if (g_vm->_curRoom == r52) {
		// snake escape 52
			if (g_vm->_obj[oSNAKEU52]._mode & OBJMODE_OBJSTATUS) {
				if (GAMEAREA(g_vm->_curMessage->_u16Param2) && !FlagUseWithStarted && (g_vm->_curObj != oSNAKEU52)) {
					StartCharacterAction(a526, 0, 1, 0);
					g_vm->_obj[oSCAVO51]._anim = a516;
					memcpy(&g_vm->_snake52, g_vm->_curMessage, sizeof(g_vm->_snake52));
					break;
				}
			}
		} else if (g_vm->_curRoom == rSYS) {
		// Sys
			CheckMask(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2);
			DoSys(g_vm->_curObj);
			break;
		}

		// If it's in a room without a character, like a map or a book
		if (FlagCharacterExist == false) {
			if ((isInventoryArea(g_vm->_curMessage->_u16Param2)) && ((g_vm->_curRoom == r31P) || (g_vm->_curRoom == r35P))) {
				if (ICONAREA(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2) && (g_vm->whatIcon(g_vm->_curMessage->_u16Param1)) && (g_vm->_inventoryStatus == INV_INACTION)) {
					g_vm->_useWith[WITH] = 0;
					g_vm->_curObj = 0;
					g_vm->_lightIcon = 0xFF;
					g_vm->setInventoryStart(g_vm->_iconBase, INVENTORY_SHOW);
					if (g_vm->_curMessage->_event == ME_MRIGHT || FlagUseWithStarted)
						doEvent(MC_INVENTORY, ME_OPERATEICON, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, 0);
					else
						doEvent(MC_INVENTORY, ME_EXAMINEICON, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, 0);
				}
				break;
			}

			if ((g_vm->_curMessage->_event == ME_MLEFT) && (g_vm->_curObj))
				doEvent(MC_ACTION, ME_MOUSEEXAMINE, MP_DEFAULT, 0, 0, 0, g_vm->_curObj);
			else if ((g_vm->_curMessage->_event == ME_MRIGHT) && (g_vm->_curObj))
				doEvent(MC_ACTION, ME_MOUSEOPERATE, MP_DEFAULT, 0, 0, 0, g_vm->_curObj);

			break;
		}

		// Special management for 2C wheels
		if ((g_vm->_obj[oBASEWHEELS2C]._mode & OBJMODE_OBJSTATUS) && (g_vm->_curRoom == r2C)) {
			if (CheckMask(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2)) {
				if ((g_vm->_curObj >= oWHEEL1A2C) && (g_vm->_curObj <= oWHEEL12C2C))
					g_vm->_wheel = (g_vm->_curObj - oWHEEL1A2C) % 3;
				else if (g_vm->_curObj == oPULSANTE2C) {
					extern uint16 *SmackImagePointer, *ImagePointer;
					if (g_vm->_curMessage->_event == ME_MLEFT) {
						doEvent(MC_ACTION, ME_MOUSEEXAMINE, MP_DEFAULT, 0, 0, 0, g_vm->_curObj);
						break;
					}
					g_vm->_animMgr->_animTab[aBKG2C]._flag &= ~SMKANIM_OFF1;
					g_vm->_obj[oBASEWHEELS2C]._mode &= ~OBJMODE_OBJSTATUS;
					g_vm->_obj[omWHEELS2C]._mode &= ~OBJMODE_OBJSTATUS;
					g_vm->_obj[oPULSANTE2C]._mode &= ~OBJMODE_OBJSTATUS;
					g_vm->_obj[g_vm->_wheelPos[0] * 3 + 0 + oWHEEL1A2C]._mode &= ~OBJMODE_OBJSTATUS;
					g_vm->_obj[g_vm->_wheelPos[1] * 3 + 1 + oWHEEL1A2C]._mode &= ~OBJMODE_OBJSTATUS;
					g_vm->_obj[g_vm->_wheelPos[2] * 3 + 2 + oWHEEL1A2C]._mode &= ~OBJMODE_OBJSTATUS;

					g_vm->_obj[oCAMPO2C]._mode |= OBJMODE_OBJSTATUS;
					g_vm->_obj[oTEMPIO2C]._mode |= OBJMODE_OBJSTATUS;
					g_vm->_obj[oLEONE2C]._mode |= OBJMODE_OBJSTATUS;
					//g_vm->_obj[od2CALLA2D]._mode |= OBJMODE_OBJSTATUS;
					g_vm->_obj[oSFINGE2C]._mode |= OBJMODE_OBJSTATUS;
					g_vm->_obj[oSTATUA2C]._mode |= OBJMODE_OBJSTATUS;
					g_vm->_obj[od2CALLA2E]._mode |= OBJMODE_OBJSTATUS;
					g_vm->_obj[oCARTELLOA2C]._mode |= OBJMODE_OBJSTATUS;
					g_vm->_obj[od2CALLA26]._mode |= OBJMODE_OBJSTATUS;
					g_vm->_obj[oWHEELS2C]._mode |= OBJMODE_OBJSTATUS;
					FlagShowCharacter = true;
					RegenRoom();
					memcpy(SmackImagePointer, ImagePointer, MAXX * AREA * 2);
					g_vm->_animMgr->startSmkAnim(g_vm->_room[g_vm->_curRoom]._bkgAnim);

					// right combination
					if ((g_vm->_wheelPos[0] == 7) && (g_vm->_wheelPos[1] == 5) && (g_vm->_wheelPos[2] == 11)) {
						doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2C6PREMEPULSANTEAPERTURA, 0, 0, g_vm->_curObj);
						g_vm->_obj[oSFINGE2C]._flag &= ~OBJFLAG_PERSON;
					} else
						doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2C6PREMEPULSANTE, 0, 0, g_vm->_curObj);

					break;
				} else
					break;

				if (g_vm->_curMessage->_event == ME_MLEFT)
					g_vm->_wheelPos[g_vm->_wheel] = (g_vm->_wheelPos[g_vm->_wheel] > 10) ? 0 : g_vm->_wheelPos[g_vm->_wheel] + 1;
				if (g_vm->_curMessage->_event == ME_MRIGHT)
					g_vm->_wheelPos[g_vm->_wheel] = (g_vm->_wheelPos[g_vm->_wheel] < 1) ? 11 : g_vm->_wheelPos[g_vm->_wheel] - 1;

				NLPlaySound(wWHEELS2C);
				g_vm->_obj[g_vm->_curObj]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[g_vm->_wheelPos[g_vm->_wheel] * 3 + g_vm->_wheel + oWHEEL1A2C]._mode |= OBJMODE_OBJSTATUS;
				RegenRoom();
			}
			break;
		}

		//	Game area
		if (GAMEAREA(g_vm->_curMessage->_u16Param2) && (!g_vm->_animMgr->_playingAnims[1])) {
			if (g_vm->_flagscriptactive)
				g_vm->_curObj = g_vm->_curMessage->_u32Param;

			int pmousex = g_vm->_curMessage->_u16Param1;
			int pmousey = g_vm->_curMessage->_u16Param2;
			if (!(AtMouseClick(g_vm->_curObj))) {
				if (CheckMask(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2)) {
					if ((g_vm->_obj[g_vm->_curObj]._lim[2] - g_vm->_obj[g_vm->_curObj]._lim[0]) < MAXX / 7) {
						pmousex = (g_vm->_obj[g_vm->_curObj]._lim[0] + g_vm->_obj[g_vm->_curObj]._lim[2]) / 2;
						pmousey = ((g_vm->_obj[g_vm->_curObj]._lim[1] + g_vm->_obj[g_vm->_curObj]._lim[3]) / 2) + TOP;
					}
				}
				whereIs(pmousex, pmousey);
				findPath();
			}
			g_vm->_characterQueue.initQueue();

			if (CheckMask(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2) && (!FlagDialogActive)) {
				if ((g_vm->_curRoom == r1D) && !(g_vm->_room[r1D]._flag & OBJFLAG_EXTRA) && (g_vm->_curObj != oSCALA1D))
					g_vm->_curObj = oDONNA1D;
				else if ((g_vm->_curRoom == r2B) && (g_vm->_room[r2B]._flag & OBJFLAG_EXTRA) && (g_vm->_curObj != oCARTELLO2B) && (g_vm->_curObj != od2BALLA28)) {
					g_vm->clearText();
					g_vm->_curObj = oPORTA2B;
					StartCharacterAction(a2B1PROVAAPRIREPORTA, 0, 0, 0);
					if (FlagUseWithStarted) {
						if (g_vm->_useWithInv[USED]) {
							g_vm->_lightIcon = 0xFF;
							g_vm->_animMgr->stopSmkAnim(g_vm->_inventoryObj[g_vm->_useWith[USED]]._anim);
							g_vm->setInventoryStart(g_vm->_inventoryRefreshStartIcon, INVENTORY_HIDE);
							g_vm->_flagInventoryLocked = false;
						}
						g_vm->_useWith[USED] = 0;
						g_vm->_useWith[WITH] = 0;
						g_vm->_useWithInv[USED] = false;
						g_vm->_useWithInv[WITH] = false;
						FlagUseWithStarted = false;
						g_vm->clearText();
					}
					break;
				} else if ((g_vm->_curRoom == r35) && !(g_vm->_room[r35]._flag & OBJFLAG_EXTRA) && ((g_vm->_curObj == oFRONTOFFICEC35) || (g_vm->_curObj == oFRONTOFFICEA35) || (g_vm->_curObj == oASCENSORE35) || (g_vm->_curObj == oMONITOR35) || (g_vm->_curObj == oSEDIA35) || (g_vm->_curObj == oRIBELLEA35) || (g_vm->_curObj == oCOMPUTER35) || (g_vm->_curObj == oGIORNALE35))) {
					g_vm->_curObj = oLASTLEV5;
					doEvent(MC_CHARACTER, ME_CHARACTERGOTOEXAMINE, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, g_vm->_curObj);
					if (FlagUseWithStarted) {
						if (g_vm->_useWithInv[USED]) {
							g_vm->_lightIcon = 0xFF;
							g_vm->_animMgr->stopSmkAnim(g_vm->_inventoryObj[g_vm->_useWith[USED]]._anim);
							g_vm->setInventoryStart(g_vm->_inventoryRefreshStartIcon, INVENTORY_HIDE);
							g_vm->_flagInventoryLocked = false;
						}
						g_vm->_useWith[USED] = 0;
						g_vm->_useWith[WITH] = 0;
						g_vm->_useWithInv[USED] = false;
						g_vm->_useWithInv[WITH] = false;
						FlagUseWithStarted = false;
						g_vm->clearText();
					}
					break;
				} else if ((g_vm->_curMessage->_event == ME_MLEFT) &&
				           ((!(g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA) && ((g_vm->_curObj == oENTRANCE2E) || (g_vm->_curObj == od24TO26) || (g_vm->_curObj == od21TO23 && !(g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_EXAMINE)))) ||
				            ((g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA) && ((g_vm->_curObj == od2ETO2C) || (g_vm->_curObj == od24TO23) || (g_vm->_curObj == od21TO22 && !(g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_EXAMINE)) || (g_vm->_curObj == od2GVALLA26))))) {
					doEvent(MC_CHARACTER, ME_CHARACTERGOTO, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, 0);
					break;
				}

				if (g_vm->_curMessage->_event == ME_MRIGHT) {
					if (!(g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_EXAMINE) && (g_vm->_curObj != 0)) {
						if (FlagUseWithStarted) {
							doEvent(MC_CHARACTER, ME_CHARACTERGOTO, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, 0);
							return;
						}
						if (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_ROOMIN)
							doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[g_vm->_curObj]._goRoom, g_vm->_obj[g_vm->_curObj]._anim, g_vm->_obj[g_vm->_curObj]._ninv, g_vm->_curObj);
						else if (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_ROOMOUT)
							doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[g_vm->_curObj]._goRoom, 0, g_vm->_obj[g_vm->_curObj]._ninv, g_vm->_curObj);
						actorStop();
						nextStep();
						g_vm->_obj[g_vm->_curObj]._flag |= OBJFLAG_DONE;
					} else if (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_USEWITH) {
						_characterGoToPosition = -1;
						actorStop();
						nextStep();
						doEvent(MC_ACTION, ME_MOUSEOPERATE, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, g_vm->_curObj);
					} else
						doEvent(MC_CHARACTER, ME_CHARACTERGOTOACTION, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, g_vm->_curObj);
				} else
					doEvent(MC_CHARACTER, ME_CHARACTERGOTOEXAMINE, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, g_vm->_curObj);
			} else
				doEvent(MC_CHARACTER, ME_CHARACTERGOTO, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, 0);
		} else if (isInventoryArea(g_vm->_curMessage->_u16Param2)) {
			// Inventory area
			if (g_vm->_animMgr->_playingAnims[1] || FlagDialogActive || g_vm->_curRoom == rSYS)
				break;

			if (ICONAREA(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2) && g_vm->whatIcon(g_vm->_curMessage->_u16Param1) && (g_vm->_inventoryStatus == INV_INACTION)) {
				g_vm->_characterQueue.initQueue();
				actorStop();
				nextStep();
				doEvent(MC_CHARACTER, ME_CHARACTERGOTOACTION, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, 0);
				g_vm->_useWith[WITH] = 0;
				g_vm->_curObj = 0;
				g_vm->_lightIcon = 0xFF;
				g_vm->setInventoryStart(g_vm->_iconBase, INVENTORY_SHOW);
				if (g_vm->_curMessage->_event == ME_MRIGHT || FlagUseWithStarted)
					doEvent(MC_INVENTORY, ME_OPERATEICON, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, 0);
				else
					doEvent(MC_INVENTORY, ME_EXAMINEICON, MP_DEFAULT, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, 0);
			}
		}
		break;
	}
}

/* -----------------09/07/97 17.52-------------------
					StartCharacterAction
 --------------------------------------------------*/
void StartCharacterAction(uint16 Act, uint16 NewRoom, uint8 NewPos, uint16 sent) {
	g_vm->_characterQueue.initQueue();

	g_vm->_flagInventoryLocked = false;
	if (Act > hLAST) {
		g_vm->_animMgr->startSmkAnim(Act);
		InitAtFrameHandler(Act, g_vm->_curObj);
		g_vm->_flagMouseEnabled = false;
		FlagShowCharacter = false;
		doEvent(MC_CHARACTER, ME_CHARACTERCONTINUEACTION, MP_DEFAULT, Act, NewRoom, NewPos, g_vm->_curObj);
	} else {
		if ((Act == aWALKIN) || (Act == aWALKOUT))
			g_vm->_curObj = 0;
		g_vm->_flagMouseEnabled = false;
		actorDoAction(Act);
		nextStep();
	}

	if (sent)
		CharacterTalkInAction(sent);
	else
		g_vm->clearText();
}

/*-------------------------------------------------------------------------*/
/*                                DOCHARACTER                  			   */
/*-------------------------------------------------------------------------*/
void doCharacter() {
	switch (g_vm->_curMessage->_event) {
	case ME_CHARACTERDOACTION:
	case ME_CHARACTERGOTOACTION:
	case ME_CHARACTERGOTOEXAMINE:
	case ME_CHARACTERGOTOEXIT:
	case ME_CHARACTERGOTO:

		if (nextStep()) {
			_characterInMovement = false;
			_characterGoToPosition = -1;
			FlagWaitRegen = true;
		} else
			_characterInMovement = true;

		if (g_vm->_fastWalk) {
			if (nextStep()) {
				_characterInMovement = false;
				_characterGoToPosition = -1;
				FlagWaitRegen = true;
			} else
				_characterInMovement = true;
		}

		FlagPaintCharacter = true;

		if (_characterInMovement)
			REEVENT;
		else {
			g_vm->_flagMouseEnabled = true;

			if (g_vm->_curMessage->_event == ME_CHARACTERGOTOACTION)
				doEvent(MC_ACTION, ME_MOUSEOPERATE, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, g_vm->_curMessage->_u32Param);
			else if (g_vm->_curMessage->_event == ME_CHARACTERGOTOEXAMINE)
				doEvent(MC_ACTION, ME_MOUSEEXAMINE, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, g_vm->_curMessage->_u32Param);
			else if (g_vm->_curMessage->_event == ME_CHARACTERGOTOEXIT) {
				FlagShowCharacter = false;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, g_vm->_curMessage->_u8Param, g_vm->_curMessage->_u32Param);
			} else if (g_vm->_curMessage->_event == ME_CHARACTERDOACTION) {
				g_vm->lastobj = 0;
				ShowObjName(g_vm->_curObj, true);
				g_vm->refreshInventory(g_vm->_inventoryRefreshStartIcon, g_vm->_inventoryRefreshStartLine);
			}
		}
		break;

	case ME_CHARACTERACTION:
		if (FlagWaitRegen)
			REEVENT;
		g_vm->_characterQueue.initQueue();
		g_vm->_inventoryRefreshStartLine = INVENTORY_HIDE;
		g_vm->refreshInventory(g_vm->_inventoryRefreshStartIcon, INVENTORY_HIDE);
		g_vm->_inventoryStatus = INV_OFF;
		if (g_vm->_curMessage->_u16Param1 > hLAST) {
			g_vm->_animMgr->startSmkAnim(g_vm->_curMessage->_u16Param1);
			InitAtFrameHandler(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u32Param);
			g_vm->_flagMouseEnabled = false;
			doEvent(MC_CHARACTER, ME_CHARACTERCONTINUEACTION, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, g_vm->_curMessage->_u8Param, g_vm->_curMessage->_u32Param);
		} else
			actorDoAction(g_vm->_curMessage->_u16Param1);

		g_vm->clearText();
		break;

	case ME_CHARACTERCONTINUEACTION:
		FlagShowCharacter = false;
		AtFrameHandler(CHARACTER_ANIM);
		//	If the animation is over
		if (!g_vm->_animMgr->_playingAnims[1]) {
			g_vm->_flagMouseEnabled = true;
			FlagShowCharacter = true;
			_characterInMovement = false;
			g_vm->_characterQueue.initQueue();
			AtFrameEnd(CHARACTER_ANIM);
			FlagWaitRegen = true;
			g_vm->lastobj = 0;
			ShowObjName(g_vm->_curObj, true);
			//	If the room changes at the end
			if (g_vm->_curMessage->_u16Param2) {
				FlagShowCharacter = false;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_curMessage->_u16Param2, 0, g_vm->_curMessage->_u8Param, g_vm->_curMessage->_u32Param);
			} else if (g_vm->_curMessage->_u8Param)
				setPosition(g_vm->_curMessage->_u8Param);

			if ((g_vm->_curMessage->_u16Param1 == g_vm->_obj[oCANCELLATA1B]._anim)
			&& !(g_vm->_obj[oBOTTIGLIA1D]._mode & OBJMODE_OBJSTATUS)
			&& !(g_vm->_obj[oRETE17]._mode & OBJMODE_OBJSTATUS)) {
				PlayDialog(dF181);
				g_vm->_flagMouseEnabled = false;
				setPosition(1);
			}
		} else
			REEVENT;
		break;
	default:
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                                  DOSYSTEM           					   */
/*-------------------------------------------------------------------------*/
void doSystem() {
	switch (g_vm->_curMessage->_event) {
	case ME_START:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_curRoom, 0, 0, g_vm->_curObj);
		break;

	case ME_REDRAWROOM:
		RedrawRoom();
		break;

	case ME_CHANGEROOM:
		if (g_vm->_curRoom == 0)
			return ;

		// if regen still has to occur
		if (FlagWaitRegen)
			REEVENT;

		if ((g_vm->_curRoom == r41D) && (g_vm->_oldRoom != g_vm->_curMessage->_u16Param1))
			NlDissolve(30);

		g_vm->_oldRoom = g_vm->_curRoom;
		g_vm->_curRoom = g_vm->_curMessage->_u16Param1;
		g_vm->_gameQueue.initQueue();
		g_vm->_animQueue.initQueue();
		g_vm->_characterQueue.initQueue();
		g_vm->_lastCurInventory = 0;
		g_vm->_lastLightIcon = 0xFF;
		g_vm->_inventoryStatus = INV_OFF;
		g_vm->_lightIcon = 0xFF;
		g_vm->_flagInventoryLocked = false;
		g_vm->_inventoryRefreshStartLine = INVENTORY_HIDE;
		g_vm->_inventoryCounter = INVENTORY_HIDE;
		g_vm->setInventoryStart(g_vm->_inventoryRefreshStartIcon, INVENTORY_HIDE);
		FlagCharacterExist = true;
		FlagShowCharacter = true;
		CurRoomMaxX = SCREENLEN;
		g_vm->_animMgr->stopSmkAnim(g_vm->_inventoryObj[g_vm->_useWith[USED]]._anim);
		g_vm->_useWith[USED] = 0;
		g_vm->_useWith[WITH] = 0;
		g_vm->_useWithInv[USED] = false;
		g_vm->_useWithInv[WITH] = false;
		FlagUseWithStarted = false;
		FlagUseWithLocked = false;
		g_vm->_lightIcon = 0xFF;
		FlagCharacterSpeak = false;
		FlagSomeOneSpeak = false;
		actorStop();
		nextStep();

		// Handle exit velocity in dual rooms level 2
		if (g_vm->_room[g_vm->_oldRoom]._flag & OBJFLAG_EXTRA) {
			if (g_vm->_curObj == od2ETO2C)
				g_vm->setRoom(r2E, false);
			if (g_vm->_curObj == od24TO23)
				g_vm->setRoom(r24, false);
			if (g_vm->_curObj == od21TO22)
				g_vm->setRoom(r21, false);
			if (g_vm->_curObj == od2GVALLA26)
				g_vm->setRoom(r2GV, false);
		} else {
			if (g_vm->_curObj == oENTRANCE2E)
				g_vm->setRoom(r2E, true);
			if (g_vm->_curObj == od24TO26)
				g_vm->setRoom(r24, true);
			if (g_vm->_curObj == od21TO23)
				g_vm->setRoom(r21, true);
		}

		if ((g_vm->_curRoom == r12) && (g_vm->_oldRoom == r11))
			g_vm->_animMgr->_animTab[aBKG11]._flag |= SMKANIM_OFF1;
		else if ((g_vm->_oldRoom == r2BL) || (g_vm->_oldRoom == r36F))
			g_vm->_oldRoom = g_vm->_curRoom;
		else if (g_vm->_curRoom == rSYS) {
			bool SpeechON = !ConfMan.getBool("speech_mute");
			bool TextON = ConfMan.getBool("subtitles");
			int SpeechVol = ConfMan.getInt("speech_volume");
			int MusicVol = ConfMan.getInt("music_volume");
			int SFxVol = ConfMan.getInt("sfx_volume");

			if (SpeechON)
				g_vm->_obj[o00SPEECHON]._mode |= OBJMODE_OBJSTATUS;
			else
				g_vm->_obj[o00SPEECHOFF]._mode |= OBJMODE_OBJSTATUS;
			if (TextON)
				g_vm->_obj[o00TEXTON]._mode |= OBJMODE_OBJSTATUS;
			else
				g_vm->_obj[o00TEXTOFF]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[o00SPEECH1D + ((SpeechVol) / 51) * 2]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[o00MUSIC1D + ((MusicVol) / 51) * 2]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[o00SOUND1D + ((SFxVol) / 51) * 2]._mode |= OBJMODE_OBJSTATUS;
			if (SpeechVol < 256)
				g_vm->_obj[o00SPEECH1D + ((SpeechVol) / 51) * 2 + 1]._mode |= OBJMODE_OBJSTATUS;
			if (MusicVol < 256)
				g_vm->_obj[o00MUSIC1D + ((MusicVol) / 51) * 2 + 1]._mode |= OBJMODE_OBJSTATUS;
			if (SFxVol < 256)
				g_vm->_obj[o00SOUND1D + ((SFxVol) / 51) * 2 + 1]._mode |= OBJMODE_OBJSTATUS;
		}

		ReadLoc();
		g_vm->_flagMouseEnabled = true;

		if ((g_vm->_curRoom == r21) && ((g_vm->_oldRoom == r23A) || (g_vm->_oldRoom == r23B)))
			g_vm->setRoom(r21, true);
		else if ((g_vm->_curRoom == r21) && (g_vm->_oldRoom == r22))
			g_vm->setRoom(r21, false);
		else if ((g_vm->_curRoom == r24) && ((g_vm->_oldRoom == r23A) || (g_vm->_oldRoom == r23B)))
			g_vm->setRoom(r24, false);
		else if ((g_vm->_curRoom == r24) && (g_vm->_oldRoom == r26))
			g_vm->setRoom(r24, true);
		else if ((g_vm->_curRoom == r2A) && (g_vm->_oldRoom == r25))
			g_vm->setRoom(r2A, true);
		else if ((g_vm->_curRoom == r2A) && ((g_vm->_oldRoom == r2B) || (g_vm->_oldRoom == r29) || (g_vm->_oldRoom == r29L)))
			g_vm->setRoom(r2A, false);
		else if ((g_vm->_curRoom == r2B) && (g_vm->_oldRoom == r28))
			g_vm->setRoom(r2B, true);
		else if ((g_vm->_curRoom == r2B) && (g_vm->_oldRoom == r2A))
			g_vm->setRoom(r2B, false);
		//			for save/load
		else if ((g_vm->_curRoom == r15) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("152.3d");
		else if ((g_vm->_curRoom == r17) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("172.3d");
		else if ((g_vm->_curRoom == r1D) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("1d2.3d");
		else if ((g_vm->_curRoom == r21) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("212.3d");
		else if ((g_vm->_curRoom == r24) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("242.3d");
		else if ((g_vm->_curRoom == r28) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("282.3d");
		else if ((g_vm->_curRoom == r2A) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("2A2.3d");
		else if ((g_vm->_curRoom == r2B) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("2B2.3d");
		else if ((g_vm->_curRoom == r2E) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("2E2.3d");
		else if ((g_vm->_curRoom == r2GV) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("2GV2.3d");
		else if ((g_vm->_curRoom == r35) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("352.3d");
		else if ((g_vm->_curRoom == r37) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("372.3d");
		else if ((g_vm->_curRoom == r4P) && (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA))
			read3D("4P2.3d");
		//			end save/load

		setPosition(g_vm->_curMessage->_u8Param);
		actorStop();

		if (g_vm->_curMessage->_u16Param2)
			StartCharacterAction(g_vm->_curMessage->_u16Param2, 0, 0, 0);

		AtEndChangeRoom();

		g_vm->_room[g_vm->_curRoom]._flag |= OBJFLAG_DONE; // Visited
		drawCharacter(CALCPOINTS);			// for right _actorPos entrance

		break;
	default:
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                              DOSCROLLINVENTORY         				   */
/*-------------------------------------------------------------------------*/
void doScrollInventory(uint16 mousex) {
	if ((g_vm->_inventoryStatus == INV_PAINT) || (g_vm->_inventoryStatus == INV_DEPAINT))
		return;

	if ((mousex <= ICONMARGSX) && g_vm->_iconBase)
		doEvent(MC_INVENTORY, ME_ONERIGHT, MP_DEFAULT, 0, 0, 0, 0);
	else if (BETWEEN(SCREENLEN - ICONMARGDX, mousex, SCREENLEN) && (g_vm->_iconBase + ICONSHOWN < g_vm->_inventorySize))
		doEvent(MC_INVENTORY, ME_ONELEFT, MP_DEFAULT, 0, 0, 0, 0);
}

/*-------------------------------------------------------------------------*/
/*                                ROLLINVENTORY             			   */
/*-------------------------------------------------------------------------*/
void RollInventory(uint8 status) {
	if (status == INV_PAINT) {
		g_vm->_inventoryCounter -= g_vm->_inventorySpeed[g_vm->_inventorySpeedIndex++];
		if (g_vm->_inventoryCounter <= INVENTORY_SHOW || g_vm->_inventorySpeedIndex > 5) {
			g_vm->_inventorySpeedIndex = 0;
			g_vm->setInventoryStart(g_vm->_iconBase, INVENTORY_SHOW);
			g_vm->_inventoryStatus = INV_INACTION;
			g_vm->_inventoryCounter = INVENTORY_SHOW;
			if (!(isInventoryArea(my)))
				doEvent(MC_INVENTORY, ME_CLOSE, MP_DEFAULT, 0, 0, 0, 0);
			g_vm->redrawString();
			return ;
		}
	} else if (status == INV_DEPAINT) {
		g_vm->_inventoryCounter += g_vm->_inventorySpeed[g_vm->_inventorySpeedIndex++];

		if (g_vm->_inventoryCounter > INVENTORY_HIDE || g_vm->_inventorySpeedIndex > 5) {
			g_vm->_inventorySpeedIndex = 0;
			g_vm->setInventoryStart(g_vm->_iconBase, INVENTORY_HIDE);
			g_vm->_inventoryStatus = INV_OFF;
			g_vm->_inventoryCounter = INVENTORY_HIDE;
			if (isInventoryArea(my) && !(FlagDialogActive || FlagDialogMenuActive))
				doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			else
				g_vm->redrawString();
			return ;
		}
	}
	g_vm->setInventoryStart(g_vm->_iconBase, g_vm->_inventoryCounter);
}

/*-------------------------------------------------------------------------*/
/*                                  DOIDLE            					   */
/*-------------------------------------------------------------------------*/
void doIdle() {
	char c = GetKey();
	switch (c) {
	// Quit
	case 'q':
	case 'Q':
		if (!FlagDialogActive  && !FlagDialogMenuActive) {
			if (QuitGame())
				doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		} else { //if( _curDialog == dINTRO )
			if (g_vm->_animMgr->_fullMotionEnd != g_vm->_animMgr->_curAnimFrame[1])
				g_vm->_animMgr->_fullMotionEnd = g_vm->_animMgr->_curAnimFrame[1] + 1;
		}
		break;

	// Skip
	case 0x1B:
		if (FlagDialogActive) {
			if (g_vm->_animMgr->_fullMotionEnd != g_vm->_animMgr->_curAnimFrame[1])
				g_vm->_animMgr->_fullMotionEnd = g_vm->_animMgr->_curAnimFrame[1] + 1;
		} else if (!FlagSomeOneSpeak && !g_vm->_flagscriptactive && !FlagDialogActive && !FlagDialogMenuActive && (_actor._curAction < hWALKIN) && !FlagUseWithStarted && g_vm->_animMgr->_playingAnims[1] == 0 && FlagShowCharacter) {
			actorStop();
			nextStep();
			Mouse(MCMD_ON);
			g_vm->_flagMouseEnabled = true;
			g_vm->_obj[o00EXIT]._goRoom = g_vm->_curRoom;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, rSYS, 0, 0, c);
			FlagShowCharacter = false;
			FlagCharacterExist = false;
			IconSnapShot();
		}
		break;

	// Sys
	case 0x3B:
		if (!FlagSomeOneSpeak && !g_vm->_flagscriptactive && !FlagDialogActive && !FlagDialogMenuActive && (_actor._curAction < hWALKIN) && !FlagUseWithStarted && g_vm->_animMgr->_playingAnims[1] == 0 && FlagShowCharacter) {
			actorStop();
			nextStep();
			Mouse(MCMD_ON);
			g_vm->_flagMouseEnabled = true;
			g_vm->_obj[o00EXIT]._goRoom = g_vm->_curRoom;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, rSYS, 0, 0, c);
			FlagShowCharacter = false;
			FlagCharacterExist = false;
			IconSnapShot();
		}
		break;

	// Save
	case 0x3C:
		if (!FlagSomeOneSpeak && !g_vm->_flagscriptactive && !FlagDialogActive && !FlagDialogMenuActive && (_actor._curAction < hWALKIN) && !FlagUseWithStarted && g_vm->_animMgr->_playingAnims[1] == 0 && FlagShowCharacter) {
			IconSnapShot();
			DataSave();
			g_vm->showInventoryName(NO_OBJECTS, false);
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, mx, my, 0, 0);
			g_vm->refreshInventory(g_vm->_inventoryRefreshStartIcon, g_vm->_inventoryRefreshStartLine);
		}
		break;

	// Load
	case 0x3D:
		if (!FlagSomeOneSpeak && !g_vm->_flagscriptactive && !FlagDialogActive && !FlagDialogMenuActive && (_actor._curAction < hWALKIN) && !FlagUseWithStarted && g_vm->_animMgr->_playingAnims[1] == 0 && FlagShowCharacter) {
			IconSnapShot();
			if (!DataLoad()) {
				g_vm->showInventoryName(NO_OBJECTS, false);
				doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, mx, my, 0, 0);
				g_vm->refreshInventory(g_vm->_inventoryRefreshStartIcon, g_vm->_inventoryRefreshStartLine);
			}
		}
		break;
	default:
		break;
	}

	if (GAMEAREA(my) && ((g_vm->_inventoryStatus == INV_ON) || (g_vm->_inventoryStatus == INV_INACTION)))
		doEvent(MC_INVENTORY, ME_CLOSE, MP_SYSTEM, 0, 0, 0, 0);

	if (g_vm->_inventoryScrollTime > TheTime)
		g_vm->_inventoryScrollTime = TheTime;

	if (isInventoryArea(my) && (TheTime > (INVSCROLLSP + g_vm->_inventoryScrollTime))) {
		doScrollInventory(mx);
		g_vm->_inventoryScrollTime = TheTime;
	}

	if (ForceQuit && !FlagDialogActive  && !FlagDialogMenuActive)
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
}

} // End of namespace Trecision
