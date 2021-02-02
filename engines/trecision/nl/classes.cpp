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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"

#include "common/config-manager.h"

namespace Trecision {

extern int32 CurRoomMaxX;
extern uint16 _playingAnims[];

// inventory
#define COUNTERSTEP 6
//PELS:  e qui ci si gioca l'inv con accellerazione ***********
int16 InvSpeed[8] = { 20, 10, 5, 3, 2};
int InvSpeedIndex = 0;
uint32 ScrollInvTime;

// gestioni particolari
uint16 ruota = -1, ruotepos[3];

// gestione serpente via
message Serp52;
int FastWalk = false, FastWalkLocked = false;

int ForceQuit = 0;
int NlVer = 102;

/*-------------------------------------------------------------------------*/
/*                                DOACTION           					   */
/*-------------------------------------------------------------------------*/
void doAction() {
	if ((TheMessage->event == ME_MOUSEOPERATE) || (TheMessage->event == ME_MOUSEEXAMINE)) {
//		Azione sulla zona GAME
		_curObj = TheMessage->lparam;
		if (_curObj == oLASTLEV5) CharacterSay(2003);
		if ((!_curObj) || (!(_obj[_curObj]._mode & OBJMODE_OBJSTATUS))) return;

		if (_obj[_curObj]._mode & OBJMODE_HIDDEN)
			_obj[_curObj]._mode &= ~OBJMODE_HIDDEN;

		if (SemUseWithStarted) {
			if ((_obj[_curObj]._flag & (OBJFLAG_ROOMOUT | OBJFLAG_ROOMIN)) && !(_obj[_curObj]._flag & OBJFLAG_EXAMINE)) return;
			SemUseWithStarted = false;
			SemInventoryLocked = false;
			UseWith[WITH] = _curObj;
			UseWithInv[WITH] = false;
			LightIcon = 0xff;

			if ((!UseWithInv[USED]) && (_curObj == UseWith[USED])) {
				UseWith[USED] = 0;
				UseWith[WITH] = 0;
				UseWithInv[USED] = false;
				UseWithInv[WITH] = false;
				SemUseWithStarted = false;
				ClearText();
			} else
				doEvent(MC_ACTION, ME_USEWITH, MP_SYSTEM, 0, 0, 0, 0);
			_curObj = 0;
			return;
		}

		if ((TheMessage->event == ME_MOUSEOPERATE) && (_obj[_curObj]._flag & OBJFLAG_USEWITH)) {
			SemUseWithStarted = true;
			SemInventoryLocked = true;
			UseWith[USED] = _curObj;
			UseWith[WITH] = 0;
			UseWithInv[USED] = false;
			UseWithInv[WITH] = false;
			ShowObjName(_curObj, true);
			return;
		}
	}

	switch (TheMessage->event) {
	case ME_MOUSEOPERATE:
		if (_obj[_curObj]._flag & OBJFLAG_ROOMIN) doRoomIn(_curObj);
		else if (_obj[_curObj]._flag & OBJFLAG_PERSON) doMouseTalk(_curObj);
		else if (_obj[_curObj]._flag & OBJFLAG_ROOMOUT) doRoomOut(_curObj);
		else if (_obj[_curObj]._flag & OBJFLAG_TAKE) doMouseTake(_curObj);
		else doMouseOperate(_curObj);
		break;

	case ME_MOUSEEXAMINE:
		if (_obj[_curObj]._flag & OBJFLAG_EXAMINE) doMouseExamine(_curObj);
		else if (_obj[_curObj]._flag & OBJFLAG_ROOMIN) doRoomIn(_curObj);
		else if (_obj[_curObj]._flag & OBJFLAG_PERSON) doMouseExamine(_curObj);
		else if (_obj[_curObj]._flag & OBJFLAG_ROOMOUT) doRoomOut(_curObj);
		else doMouseExamine(_curObj);
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

	static int8 lastpos;
	int8 curpos;

	switch (TheMessage->event) {
	case ME_MMOVE:
		if (GAMEAREA(TheMessage->wparam2)) curpos = POSGAME;
		else if (INVAREA(TheMessage->wparam2)) curpos = POSINV;
		else curpos = POSUP;

//			Zona GAME
		if (curpos == POSGAME) {
			lastpos = POSGAME;
			if ((SemSomeOneSpeak) || (SemDialogMenuActive) || (SemDialogActive) || (SemUseWithLocked)) break;
			CheckMask(TheMessage->wparam1, TheMessage->wparam2);
//				per la ruota della 2C
			if ((_curObj >= oRUOTA1A2C) && (_curObj <= oRUOTA12C2C))
				ShowObjName((oRUOTA1A2C % 3) + oRUOTAA2C, true);
//				per il dislocatore
			else if (_curRoom == r41D) {
				if ((_curObj >= oPULSANTE1AD) && (_curObj <= oPULSANTE33AD)) {
					if (!(_obj[oROOM41 + _obj[_curObj]._goRoom - r41]._mode & OBJMODE_OBJSTATUS)) {
						int a;
						for (a = oROOM41; a <= oROOM4X; a++) {
							if (_obj[a]._mode & OBJMODE_OBJSTATUS)
								_obj[a]._mode &= ~OBJMODE_OBJSTATUS;
						}
						_obj[oROOM45B]._mode &= ~OBJMODE_OBJSTATUS;

						if (((oROOM41 + _obj[_curObj]._goRoom - r41) == oROOM45) && (_obj[od44ALLA45]._goRoom == r45S))
							_obj[oROOM45B]._mode |= OBJMODE_OBJSTATUS;
						else
							_obj[oROOM41 + _obj[_curObj]._goRoom - r41]._mode |= OBJMODE_OBJSTATUS;
						RegenRoom();
					}
				} else {
					int a;
					for (a = oROOM41; a <= oROOM4X; a++) {
						if (_obj[a]._mode & OBJMODE_OBJSTATUS)
							_obj[a]._mode &= ~OBJMODE_OBJSTATUS;
					}
					_obj[oROOM45B]._mode &= ~OBJMODE_OBJSTATUS;

					RegenRoom();
				}
				ShowObjName(_curObj, true);
			} else
//				fine ruota e dislocatore
				ShowObjName(_curObj, true);

			if (_inventoryStatus == INV_INACTION)
				doEvent(MC_INVENTORY, ME_CLOSE, MP_DEFAULT, 0, 0, 0, 0);
		}
//			Zona INVENTORY
		else if (curpos == POSINV) {
			lastpos = POSINV;
			if ((!SemCharacterExist) && ((_curRoom != r31P) && (_curRoom != r35P))) break;	// Se sono in stanze senza omino tipo la mappa
			if (((SemSomeOneSpeak) && !(SemCharacterSpeak)) || (SemDialogMenuActive) || (SemDialogActive) || (SemUseWithLocked)) break;
			if (_playingAnims[1]) break;

			if (_inventoryStatus == INV_OFF)
				doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			else if (_inventoryStatus == INV_INACTION)
				doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
		}
//			Zona UP
		else {
			if (_curRoom == rSYS) break;

			_curObj = 0;
			ShowObjName(_curObj, true);
//				if( lastpos != POSUP )
//					ShowObjName(0,false);
			lastpos = POSUP;

			if (SemDialogMenuActive)
				UpdateScelte(TheMessage->wparam1, TheMessage->wparam2);
		}
		break;

	case ME_MRIGHT:
	case ME_MLEFT:
//			if(Semskiptalk) break;
		if (SemSomeOneSpeak) {
			Semskiptalk = Semskipenable;
			break;
		}
		if (_actor._curAction > hWALKIN) break;
		if (SemWalkNoInterrupt && (_actor._curAction != hSTAND)) break;

		if ((SemDialogActive) && (SemDialogMenuActive)) {
			ScegliScelta(TheMessage->wparam1, TheMessage->wparam2);
			break;
		}
//			per il dislocatore
		if ((_curObj >= oPULSANTE1AD) && (_curObj <= oPULSANTE33AD)) {
			if ((_obj[_curObj]._goRoom == r45) && (_obj[od44ALLA45]._goRoom == r45S) &&
					(_obj[oEXIT41D]._goRoom == r45S) && (TheMessage->event == ME_MRIGHT))
				doEvent(MC_ACTION, ME_MOUSEOPERATE, MP_DEFAULT, 0, 0, 0, _curObj);
			else if ((_obj[_curObj]._goRoom == r45) && (_obj[od44ALLA45]._goRoom == r45S) &&
					 (_obj[oEXIT41D]._goRoom != r45S) && (TheMessage->event == ME_MRIGHT)) {
				_obj[oEXIT41D]._goRoom = r45S;
				InvObj[iDISLOCATORE]._flag |= OBJFLAG_EXTRA;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r45S, 0, 0, _curObj);
			} else if (_obj[oEXIT41D]._goRoom != _obj[_curObj]._goRoom  && (TheMessage->event == ME_MRIGHT)) {
				_obj[oEXIT41D]._goRoom = _obj[_curObj]._goRoom;
				InvObj[iDISLOCATORE]._flag |= OBJFLAG_EXTRA;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT41D]._goRoom, 0, 0, _curObj);
			} else if ((TheMessage->event == ME_MLEFT) && (_curObj))
				doEvent(MC_ACTION, ME_MOUSEEXAMINE, MP_DEFAULT, 0, 0, 0, _curObj);
			else if ((TheMessage->event == ME_MRIGHT) && (_curObj))
				doEvent(MC_ACTION, ME_MOUSEOPERATE, MP_DEFAULT, 0, 0, 0, _curObj);
			break;
		}
//			fine dislocatore
//			serpente scappa 52
		else if (_curRoom == r52) {
			if (_obj[oSERPENTEU52]._mode & OBJMODE_OBJSTATUS)
				if (GAMEAREA(TheMessage->wparam2)  && !SemUseWithStarted  && (_curObj != oSERPENTEU52)) {
					StartCharacterAction(a526, 0, 1, 0);
					_obj[oSCAVO51]._anim = a516;
					memcpy(&Serp52, TheMessage, sizeof(Serp52));
					break;
				}
		}
//			fine serpente scappa 52
//			sys
		else if (_curRoom == rSYS) {
			CheckMask(TheMessage->wparam1, TheMessage->wparam2);
			DoSys(_curObj);
			break;
		}
//			fine sys

// 			Se sono in stanze senza omino tipo la mappa o libro
		if ((SemCharacterExist == false)/* && (GAMEAREA(TheMessage->wparam2))*/) {
			if ((INVAREA(TheMessage->wparam2)) && ((_curRoom == r31P) || (_curRoom == r35P))) {
				if (ICONAREA(TheMessage->wparam1, TheMessage->wparam2) && (WhatIcon(TheMessage->wparam1)) && (_inventoryStatus == INV_INACTION)) {
					UseWith[WITH] = 0;
					_curObj = 0;
					LightIcon = 0xFF;
					RegenInv(TheIconBase, INVENTORY_SHOW);
					if (TheMessage->event == ME_MRIGHT)
						doEvent(MC_INVENTORY, ME_OPERATEICON, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
					else if (SemUseWithStarted)
						doEvent(MC_INVENTORY, ME_OPERATEICON, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
					else
						doEvent(MC_INVENTORY, ME_EXAMINEICON, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
				}
				break;
			}

			if ((TheMessage->event == ME_MLEFT) && (_curObj))
				doEvent(MC_ACTION, ME_MOUSEEXAMINE, MP_DEFAULT, 0, 0, 0, _curObj);
			else if ((TheMessage->event == ME_MRIGHT) && (_curObj))
				doEvent(MC_ACTION, ME_MOUSEOPERATE, MP_DEFAULT, 0, 0, 0, _curObj);

			break;
		}

//			gestione particolare ruote 2C
		if ((_obj[oBASERUOTE2C]._mode & OBJMODE_OBJSTATUS) && (_curRoom == r2C)) {
			if (CheckMask(TheMessage->wparam1, TheMessage->wparam2)) {
				if ((_curObj >= oRUOTA1A2C) && (_curObj <= oRUOTA12C2C))
					ruota = (_curObj - oRUOTA1A2C) % 3;
				else if (_curObj == oPULSANTE2C) {
					extern uint16 *SmackImagePointer, *ImagePointer;
					if (TheMessage->event == ME_MLEFT) {
						doEvent(MC_ACTION, ME_MOUSEEXAMINE, MP_DEFAULT, 0, 0, 0, _curObj);
						break;
					}
					AnimTab[aBKG2C].flag &= ~SMKANIM_OFF1;
					_obj[oBASERUOTE2C]._mode &= ~OBJMODE_OBJSTATUS;
					_obj[omRUOTE2C]._mode &= ~OBJMODE_OBJSTATUS;
					_obj[oPULSANTE2C]._mode &= ~OBJMODE_OBJSTATUS;
					_obj[ruotepos[0] * 3 + 0 + oRUOTA1A2C]._mode &= ~OBJMODE_OBJSTATUS;
					_obj[ruotepos[1] * 3 + 1 + oRUOTA1A2C]._mode &= ~OBJMODE_OBJSTATUS;
					_obj[ruotepos[2] * 3 + 2 + oRUOTA1A2C]._mode &= ~OBJMODE_OBJSTATUS;

					_obj[oCAMPO2C]._mode |= OBJMODE_OBJSTATUS;
					_obj[oTEMPIO2C]._mode |= OBJMODE_OBJSTATUS;
					_obj[oLEONE2C]._mode |= OBJMODE_OBJSTATUS;
					//_obj[od2CALLA2D]._mode |= OBJMODE_OBJSTATUS;
					_obj[oSFINGE2C]._mode |= OBJMODE_OBJSTATUS;
					_obj[oSTATUA2C]._mode |= OBJMODE_OBJSTATUS;
					_obj[od2CALLA2E]._mode |= OBJMODE_OBJSTATUS;
					_obj[oCARTELLOA2C]._mode |= OBJMODE_OBJSTATUS;
					_obj[od2CALLA26]._mode |= OBJMODE_OBJSTATUS;
					_obj[oRUOTE2C]._mode |= OBJMODE_OBJSTATUS;
					SemShowHomo = true;
					RegenRoom();
					memcpy(SmackImagePointer, ImagePointer, MAXX * AREA * 2);
					StartSmackAnim(Room[_curRoom]._bkgAnim);

					// combinazione giusta
					if ((ruotepos[0] == 7) && (ruotepos[1] == 5) && (ruotepos[2] == 11)) {
						doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2C6PREMEPULSANTEAPERTURA, 0, 0, _curObj);
						_obj[oSFINGE2C]._flag &= ~OBJFLAG_PERSON;
					} else
						doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2C6PREMEPULSANTE, 0, 0, _curObj);

					break;
				} else
					break;

				if (TheMessage->event == ME_MLEFT)
					ruotepos[ruota] = (ruotepos[ruota] > 10) ? 0 : ruotepos[ruota] + 1;
				if (TheMessage->event == ME_MRIGHT)
					ruotepos[ruota] = (ruotepos[ruota] < 1) ? 11 : ruotepos[ruota] - 1;

				NLPlaySound(wRUOTE2C);
				_obj[_curObj]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[ruotepos[ruota] * 3 + ruota + oRUOTA1A2C]._mode |= OBJMODE_OBJSTATUS;
				RegenRoom();
			}
			break;
		}
//			fine gestione particolare

//			Zona GAME
		if (GAMEAREA(TheMessage->wparam2) && (!_playingAnims[1])) {
			int pmousex, pmousey;

			if (Semscriptactive) _curObj = TheMessage->lparam;

			pmousex  = TheMessage->wparam1;
			pmousey  = TheMessage->wparam2;
			if (!(AtMouseClick(_curObj))) {
				if (CheckMask(TheMessage->wparam1, TheMessage->wparam2)) {
					if ((_obj[_curObj]._lim[2] - _obj[_curObj]._lim[0]) < MAXX / 7) {
						pmousex = (_obj[_curObj]._lim[0] + _obj[_curObj]._lim[2]) / 2;
						pmousey = ((_obj[_curObj]._lim[1] + _obj[_curObj]._lim[3]) / 2) + TOP;
					}
				}
				whereIs(pmousex, pmousey);
				findPath();
			}
			InitQueue(&Homo);

			if (CheckMask(TheMessage->wparam1, TheMessage->wparam2) && (!SemDialogActive)) {
				if ((_curRoom == r1D) && !(Room[r1D]._flag & OBJFLAG_EXTRA) && (_curObj != oSCALA1D))
					_curObj = oDONNA1D;
				else if ((_curRoom == r2B) && (Room[r2B]._flag & OBJFLAG_EXTRA) && (_curObj != oCARTELLO2B) && (_curObj != od2BALLA28)) {
					ClearText();
					_curObj = oPORTA2B;
					StartCharacterAction(a2B1PROVAAPRIREPORTA, 0, 0, 0);
					if (SemUseWithStarted) {
						if (UseWithInv[USED]) {
							LightIcon = 0xFF;
							StopSmackAnim(InvObj[UseWith[USED]]._anim);
							RegenInv(RegenInvStartIcon, INVENTORY_HIDE);
							SemInventoryLocked = false;
						}
						UseWith[USED] = 0;
						UseWith[WITH] = 0;
						UseWithInv[USED] = false;
						UseWithInv[WITH] = false;
						SemUseWithStarted = false;
						ClearText();
					}
					break;
				} else if ((_curRoom == r35) && !(Room[r35]._flag & OBJFLAG_EXTRA) && ((_curObj == oSPORTELLOC35) || (_curObj == oSPORTELLOA35) || (_curObj == oASCENSORE35) || (_curObj == oMONITOR35) || (_curObj == oSEDIA35) || (_curObj == oRIBELLEA35) || (_curObj == oCOMPUTER35) || (_curObj == oGIORNALE35))) {
					_curObj = oLASTLEV5;
					doEvent(MC_HOMO, ME_HOMOGOTOEXAMINE, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, _curObj);
					if (SemUseWithStarted) {
						if (UseWithInv[USED]) {
							LightIcon = 0xFF;
							StopSmackAnim(InvObj[UseWith[USED]]._anim);
							RegenInv(RegenInvStartIcon, INVENTORY_HIDE);
							SemInventoryLocked = false;
						}
						UseWith[USED] = 0;
						UseWith[WITH] = 0;
						UseWithInv[USED] = false;
						UseWithInv[WITH] = false;
						SemUseWithStarted = false;
						ClearText();
					}
					break;
				} else if ((TheMessage->event == ME_MLEFT) &&
						   ((!(Room[_curRoom]._flag & OBJFLAG_EXTRA) && ((_curObj == oENTRANCE2E) || (_curObj == od24ALLA26) || (_curObj == od21ALLA23 && !(_obj[_curObj]._flag & OBJFLAG_EXAMINE)))) ||
							((Room[_curRoom]._flag & OBJFLAG_EXTRA) && ((_curObj == od2EALLA2C) || (_curObj == od24ALLA23) || (_curObj == od21ALLA22 && !(_obj[_curObj]._flag & OBJFLAG_EXAMINE)) || (_curObj == od2GVALLA26))))) {
					doEvent(MC_HOMO, ME_HOMOGOTO, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
					break;
				}

				if (TheMessage->event == ME_MRIGHT) {
					if (!(_obj[_curObj]._flag & OBJFLAG_EXAMINE)  && (_curObj != 0)) {
						if (SemUseWithStarted) {
							doEvent(MC_HOMO, ME_HOMOGOTO, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
							return;
						} else if (_obj[_curObj]._flag & OBJFLAG_ROOMIN)
							doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[_curObj]._goRoom, _obj[_curObj]._anim, _obj[_curObj]._ninv, _curObj);
						else if (_obj[_curObj]._flag & OBJFLAG_ROOMOUT)
							doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[_curObj]._goRoom, 0, _obj[_curObj]._ninv, _curObj);
						actorStop();
						nextStep();
						_obj[_curObj]._flag |= OBJFLAG_DONE;
					} else if (_obj[_curObj]._flag & OBJFLAG_USEWITH) {
						_homoGoToPosition = -1;
						actorStop();
						nextStep();
						doEvent(MC_ACTION, ME_MOUSEOPERATE, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, _curObj);
					} else
						doEvent(MC_HOMO, ME_HOMOGOTOACTION, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, _curObj);
				} else
					doEvent(MC_HOMO, ME_HOMOGOTOEXAMINE, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, _curObj);
			} else if ((SemUseWithStarted) && !(UseWith[WITH]))
				doEvent(MC_HOMO, ME_HOMOGOTO, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
			else
				doEvent(MC_HOMO, ME_HOMOGOTO, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
		}
//			Zona INVENTORY
		else if (INVAREA(TheMessage->wparam2)) {
			if (_playingAnims[1] || SemDialogActive) break;
			if (_curRoom == rSYS) break;

			if (ICONAREA(TheMessage->wparam1, TheMessage->wparam2) && (WhatIcon(TheMessage->wparam1)) && (_inventoryStatus == INV_INACTION)) {
				InitQueue(&Homo);
				actorStop();
				nextStep();
				doEvent(MC_HOMO, ME_HOMOGOTOACTION, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
				UseWith[WITH] = 0;
				_curObj = 0;
				LightIcon = 0xFF;
				RegenInv(TheIconBase, INVENTORY_SHOW);
				if (TheMessage->event == ME_MRIGHT)
					doEvent(MC_INVENTORY, ME_OPERATEICON, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
				else if (SemUseWithStarted)
					doEvent(MC_INVENTORY, ME_OPERATEICON, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
				else
					doEvent(MC_INVENTORY, ME_EXAMINEICON, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, 0);
			}
		}
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                                DOINVENTORY          					   */
/*-------------------------------------------------------------------------*/
void doInventory() {
	switch (TheMessage->event) {
	case ME_OPEN:
		if ((!SemInventoryLocked) && (_inventoryStatus == INV_OFF) && !(SemDialogActive)) {
			_inventoryCounter = INVENTORY_HIDE;
			InvSpeedIndex = 0;
			_inventoryStatus = INV_PAINT;
		}
		break;

	case ME_CLOSE:
		if ((!SemInventoryLocked) && (_inventoryStatus == INV_INACTION) && !(SemDialogActive)) {
			_inventoryCounter = INVENTORY_SHOW;
			InvSpeedIndex = 0;
			_inventoryStatus = INV_DEPAINT;
			LightIcon = 0xFF;
		}
		break;

	case ME_ONELEFT:
		if (_inventoryStatus == INV_INACTION) OneLeftInventory();
		break;

	case ME_ONERIGHT:
		if (_inventoryStatus == INV_INACTION) OneRightInventory();
		break;

	case ME_OPERATEICON:
		CurInventory = WhatIcon(mx);
		if (CurInventory == 0) break;

		if (SemUseWithStarted) {
			SemInventoryLocked = false;
			SemUseWithStarted = false;
			UseWith[WITH] = CurInventory;
			UseWithInv[WITH] = true;

			if (UseWith[USED] != CurInventory) {
				doEvent(MC_ACTION, ME_USEWITH, MP_DEFAULT, 0, 0, 0, 0);
				LightIcon = 0xFF;
			} else {
				StopSmackAnim(InvObj[UseWith[USED]]._anim);
				ShowInvName(CurInventory, true);
				LightIcon = CurInventory;
			}
		} else if (InvObj[CurInventory]._flag & OBJFLAG_USEWITH) {
			if ((CurInventory == iCANDELOTTO) && (_curRoom == r29)) {
				CharacterSay(1565);
				return;
			}
			StartSmackAnim(InvObj[CurInventory]._anim);
			LightIcon = CurInventory;
			RegenInv(TheIconBase, INVENTORY_SHOW);
			SemInventoryLocked = true;
			SemUseWithStarted = true;
			UseWith[USED] = CurInventory;
			UseWithInv[USED] = true;
			ShowInvName(CurInventory, true);
		} else doEvent(MC_ACTION, ME_INVOPERATE, MP_DEFAULT, 0, 0, 0, CurInventory);
		break;

	case ME_EXAMINEICON:
		CurInventory = WhatIcon(mx);
		actorStop();
		nextStep();
		if (SemUseWithStarted) {
			SemInventoryLocked = false;
			SemUseWithStarted = false;
			UseWith[WITH] = CurInventory;
			UseWithInv[WITH] = true;
			if (UseWith[USED] != CurInventory) {
				doEvent(MC_ACTION, ME_USEWITH, MP_DEFAULT, 0, 0, 0, 0);
				LightIcon = 0xFF;
			} else {
				StopSmackAnim(InvObj[UseWith[USED]]._anim);
				ShowInvName(CurInventory, true);
				LightIcon = CurInventory;
			}
		} else doEvent(MC_ACTION, ME_INVEXAMINE, MP_DEFAULT, 0, 0, 0, CurInventory);
		break;

	case ME_SHOWICONNAME:
		if (ICONAREA(mx, my)) {
			if (_inventoryStatus != INV_ON) doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			CurInventory = WhatIcon(mx);
			ShowInvName(CurInventory, true);

			if (!SemUseWithStarted && !SemSomeOneSpeak) {
				RegenInv(TheIconBase, INVENTORY_SHOW);
			}
		} else {
			if (!(INVAREA(my))) break;
			ShowInvName(NO_OBJECTS, true);
			if (!(SemUseWithStarted)) {
				LightIcon = 0xFF;
				RegenInv(TheIconBase, INVENTORY_SHOW);
			}
		}
		break;
	}
}

/* -----------------09/07/97 17.52-------------------
					StartCharacterAction
 --------------------------------------------------*/
void StartCharacterAction(uint16 Act, uint16 NewRoom, uint8 NewPos, uint16 sent) {
	InitQueue(&Homo);

	SemInventoryLocked = false;
	if (Act > hLAST) {
		StartSmackAnim(Act);
		InitAtFrameHandler(Act, _curObj);
		SemMouseEnabled = false;
		SemShowHomo = 0;
		doEvent(MC_HOMO, ME_HOMOCONTINUEACTION, MP_DEFAULT, Act, NewRoom, NewPos, _curObj);
	} else {
		if ((Act == aWALKIN) || (Act == aWALKOUT))
			_curObj = 0;
		SemMouseEnabled = false;
		actorDoAction(Act);
		nextStep();
	}

	if (sent)
		CharacterTalkInAction(sent);
	else
		ClearText();
}

/*-------------------------------------------------------------------------*/
/*                                   DOHOMO                  			   */
/*-------------------------------------------------------------------------*/
void doHomo() {
	switch (TheMessage->event) {
	case ME_HOMODOACTION:
	case ME_HOMOGOTOACTION:
	case ME_HOMOGOTOEXAMINE:
	case ME_HOMOGOTOEXIT:
	case ME_HOMOGOTO:

		if (nextStep()) {
			_homoInMovement = false;
			_homoGoToPosition = -1;
			SemWaitRegen = true;
		} else
			_homoInMovement = true;

		if (FastWalk) {
			if (nextStep()) {
				_homoInMovement = false;
				_homoGoToPosition = -1;
				SemWaitRegen = true;
			} else
				_homoInMovement = true;
		}

		SemPaintHomo = true;

		if (_homoInMovement)
			REEVENT;
		else {
			SemMouseEnabled = true;

			if (TheMessage->event == ME_HOMOGOTOACTION)
				doEvent(MC_ACTION, ME_MOUSEOPERATE, TheMessage->priority, TheMessage->wparam1, TheMessage->wparam2, 0, TheMessage->lparam);
			else if (TheMessage->event == ME_HOMOGOTOEXAMINE)
				doEvent(MC_ACTION, ME_MOUSEEXAMINE, TheMessage->priority, TheMessage->wparam1, TheMessage->wparam2, 0, TheMessage->lparam);
			else if (TheMessage->event == ME_HOMOGOTOEXIT) {
				SemShowHomo = 0;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, TheMessage->priority, TheMessage->wparam1, TheMessage->wparam2, TheMessage->bparam, TheMessage->lparam);
			} else if (TheMessage->event == ME_HOMODOACTION) {
				extern uint16 lastobj;
				lastobj = 0;
				ShowObjName(_curObj, true);
				RegenInventory(RegenInvStartIcon, RegenInvStartLine);
			}
		}
		break;

	case ME_HOMOACTION:
		if (SemWaitRegen)
			REEVENT;
		InitQueue(&Homo);
		RegenInvStartLine = INVENTORY_HIDE;
		RegenInventory(RegenInvStartIcon, INVENTORY_HIDE);
		_inventoryStatus = INV_OFF;
		if (TheMessage->wparam1 > hLAST) {
			StartSmackAnim(TheMessage->wparam1);
			InitAtFrameHandler(TheMessage->wparam1, TheMessage->lparam);
			SemMouseEnabled = false;
			doEvent(MC_HOMO, ME_HOMOCONTINUEACTION, TheMessage->priority, TheMessage->wparam1, TheMessage->wparam2, TheMessage->bparam, TheMessage->lparam);
		} else
			actorDoAction(TheMessage->wparam1);

		ClearText();
		break;

	case ME_HOMOCONTINUEACTION:
		SemShowHomo = 0;
		AtFrameHandler(HOMO_ANIM);
		//	If the animation is over
		if (!_playingAnims[1]) {
			extern uint16 lastobj;
			SemMouseEnabled = true;
			SemShowHomo = 1;
			_homoInMovement = false;
			InitQueue(&Homo);
			AtFrameEnd(HOMO_ANIM);
			SemWaitRegen = true;
			lastobj = 0;
			ShowObjName(_curObj, true);
			//	If the room changes at the end
			if (TheMessage->wparam2) {
				SemShowHomo = 0;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, TheMessage->wparam2, 0, TheMessage->bparam, TheMessage->lparam);
			} else if (TheMessage->bparam)
				setPosition(TheMessage->bparam);

			if (TheMessage->wparam1 == _obj[oCANCELLATA1B]._anim)
				if (!(_obj[oBOTTIGLIA1D]._mode & OBJMODE_OBJSTATUS) && !(_obj[oRETE17]._mode & OBJMODE_OBJSTATUS)) {
					PlayDialog(dF181);
					SemMouseEnabled = false;
					setPosition(1);
				}
		} else
			REEVENT;
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                                  DOSYSTEM           					   */
/*-------------------------------------------------------------------------*/
void doSystem() {
	switch (TheMessage->event) {
	case ME_START:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _curRoom, 0, 0, _curObj);
		break;

	case ME_REDRAWROOM:
		RedrawRoom();
		break;

	case ME_CHANGEROOM:
		// se oggetto e' sbagliato
		if (/*( _curObj == 0 ) ||*/ (_curRoom == 0)) {
			return ;
		}
		// se deve ancora fare regen
		if (SemWaitRegen)
			REEVENT;

		if ((_curRoom == r41D) && (OldRoom != TheMessage->wparam1))
			NlDissolve(30);

		OldRoom = _curRoom;
		_curRoom = TheMessage->wparam1;
		InitQueue(&Game);
		InitQueue(&Anim);
		InitQueue(&Homo);
		_lastCurInventory = 0;
		_lastLightIcon = 0xFF;
		_inventoryStatus = INV_OFF;
		LightIcon = 0xFF;
		SemInventoryLocked = false;
		RegenInvStartLine = INVENTORY_HIDE;
		_inventoryCounter = INVENTORY_HIDE;
		RegenInv(RegenInvStartIcon, INVENTORY_HIDE);
		SemCharacterExist = true;
		SemShowHomo = true;
		CurRoomMaxX = SCREENLEN;
		StopSmackAnim(InvObj[UseWith[USED]]._anim);
		UseWith[USED] = 0;
		UseWith[WITH] = 0;
		UseWithInv[USED] = false;
		UseWithInv[WITH] = false;
		SemUseWithStarted = false;
		SemUseWithLocked = false;
		LightIcon = 0xFF;
		SemCharacterSpeak = false;
		SemSomeOneSpeak = false;
		actorStop();
		nextStep();

//			Gestione exit veloci in stanze doppie livello 2
		if (Room[OldRoom]._flag & OBJFLAG_EXTRA) {
			if (_curObj == od2EALLA2C)	SetRoom(r2E, false);
			if (_curObj == od24ALLA23)	SetRoom(r24, false);
			if (_curObj == od21ALLA22)	SetRoom(r21, false);
			if (_curObj == od2GVALLA26)	SetRoom(r2GV, false);
		} else {
			if (_curObj == oENTRANCE2E)	SetRoom(r2E, true);
			if (_curObj == od24ALLA26)	SetRoom(r24, true);
			if (_curObj == od21ALLA23)	SetRoom(r21, true);
		}

		if ((_curRoom == r12) && (OldRoom == r11))
			AnimTab[aBKG11].flag |= SMKANIM_OFF1;
		else if ((OldRoom == r2BL) || (OldRoom == r36F))
			OldRoom = _curRoom;
		else if (_curRoom == rSYS) {
			bool SpeechON = !ConfMan.getBool("speech_mute");
			bool TextON = ConfMan.getBool("subtitles");
			int SpeechVol = ConfMan.getInt("speech_volume");
			int MusicVol = ConfMan.getInt("music_volume");
			int SFxVol = ConfMan.getInt("sfx_volume");

			if (SpeechON) _obj[o00SPEECHON]._mode |= OBJMODE_OBJSTATUS;
			else _obj[o00SPEECHOFF]._mode |= OBJMODE_OBJSTATUS;
			if (TextON)   _obj[o00TEXTON]._mode |= OBJMODE_OBJSTATUS;
			else _obj[o00TEXTOFF]._mode |= OBJMODE_OBJSTATUS;
			_obj[o00SPEECH1D + ((SpeechVol) / 51) * 2]._mode |= OBJMODE_OBJSTATUS;
			_obj[o00MUSIC1D + ((MusicVol) / 51) * 2]._mode |= OBJMODE_OBJSTATUS;
			_obj[o00SOUND1D + ((SFxVol) / 51) * 2]._mode |= OBJMODE_OBJSTATUS;
			if (SpeechVol < 256) _obj[o00SPEECH1D + ((SpeechVol) / 51) * 2 + 1]._mode |= OBJMODE_OBJSTATUS;
			if (MusicVol  < 256) _obj[o00MUSIC1D + ((MusicVol) / 51) * 2 + 1]._mode |= OBJMODE_OBJSTATUS;
			if (SFxVol    < 256) _obj[o00SOUND1D + ((SFxVol) / 51) * 2 + 1]._mode |= OBJMODE_OBJSTATUS;
		}

		ReadLoc();
		SemMouseEnabled = true;

		if ((_curRoom == r21) && ((OldRoom == r23A) || (OldRoom == r23B)))			SetRoom(r21, true);
		else if ((_curRoom == r21) && (OldRoom == r22))									SetRoom(r21, false);
		else if ((_curRoom == r24) && ((OldRoom == r23A) || (OldRoom == r23B)))		SetRoom(r24, false);
		else if ((_curRoom == r24) && (OldRoom == r26))									SetRoom(r24, true);
		else if ((_curRoom == r2A) && (OldRoom == r25))									SetRoom(r2A, true);
		else if ((_curRoom == r2A) && ((OldRoom == r2B) || (OldRoom == r29) || (OldRoom == r29L)))	SetRoom(r2A, false);
		else if ((_curRoom == r2B) && (OldRoom == r28))									SetRoom(r2B, true);
		else if ((_curRoom == r2B) && (OldRoom == r2A))									SetRoom(r2B, false);
//			for save/load
		else if ((_curRoom == r15) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("152.3d");
		else if ((_curRoom == r17) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("172.3d");
		else if ((_curRoom == r1D) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("1d2.3d");
		else if ((_curRoom == r21) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("212.3d");
		else if ((_curRoom == r24) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("242.3d");
		else if ((_curRoom == r28) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("282.3d");
		else if ((_curRoom == r2A) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("2A2.3d");
		else if ((_curRoom == r2B) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("2B2.3d");
		else if ((_curRoom == r2E) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("2E2.3d");
		else if ((_curRoom == r2GV) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("2GV2.3d");
		else if ((_curRoom == r35) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("352.3d");
		else if ((_curRoom == r37) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("372.3d");
		else if ((_curRoom == r4P) && (Room[_curRoom]._flag & OBJFLAG_EXTRA)) read3D("4P2.3d");
//			end save/load

		setPosition(TheMessage->bparam);
		actorStop();

		if (TheMessage->wparam2)
			StartCharacterAction(TheMessage->wparam2, 0, 0, 0);

		AtEndChangeRoom();

		Room[_curRoom]._flag |= OBJFLAG_DONE;  // visitata
		drawCharacter(CALCPOINTS);			// for right _actorPos entrance

		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                              DOSCROLLINVENTORY         				   */
/*-------------------------------------------------------------------------*/
void doScrollInventory(uint16 mousex) {
	if ((_inventoryStatus == INV_PAINT) || (_inventoryStatus == INV_DEPAINT))
		return;

	if ((mousex <= ICONMARGSX) && (TheIconBase))
		doEvent(MC_INVENTORY, ME_ONERIGHT, MP_DEFAULT, 0, 0, 0, 0);

	else if (BETWEEN(SCREENLEN - ICONMARGDX, mousex, SCREENLEN) && (TheIconBase + ICONSHOWN < _inventorySize))
		doEvent(MC_INVENTORY, ME_ONELEFT, MP_DEFAULT, 0, 0, 0, 0);
}

/*-------------------------------------------------------------------------*/
/*                                ROLLINVENTORY             			   */
/*-------------------------------------------------------------------------*/
void RollInventory(uint8 status) {
	if (status == INV_PAINT) {
		_inventoryCounter -= InvSpeed[InvSpeedIndex++];
		if ((_inventoryCounter <= INVENTORY_SHOW) || (InvSpeedIndex > 5)) {
			InvSpeedIndex = 0;
			RegenInv(TheIconBase, INVENTORY_SHOW);
			_inventoryStatus = INV_INACTION;
			_inventoryCounter = INVENTORY_SHOW;
			if (!(INVAREA(my))) doEvent(MC_INVENTORY, ME_CLOSE, MP_DEFAULT, 0, 0, 0, 0);
			RepaintString();
			return ;
		}
	} else if (status == INV_DEPAINT) {
		_inventoryCounter += InvSpeed[InvSpeedIndex++];

		if ((_inventoryCounter > INVENTORY_HIDE) || (InvSpeedIndex > 5)) {
			InvSpeedIndex = 0;
			RegenInv(TheIconBase, INVENTORY_HIDE);
			_inventoryStatus = INV_OFF;
			_inventoryCounter = INVENTORY_HIDE;
			if ((INVAREA(my)) && !(SemDialogActive || SemDialogMenuActive)) doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			else RepaintString();
			return ;
		}
	}
	RegenInv(TheIconBase, _inventoryCounter);
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
		if (!SemDialogActive  && !SemDialogMenuActive) {
			if (QuitGame())
				doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		} else { //if( _curDialog == dINTRO )
			extern int FullEnd;
			extern uint16 _curAnimFrame[];

			if (FullEnd != _curAnimFrame[1])
				FullEnd = _curAnimFrame[1] + 1;
		}
		break;

	// Skip
	case 0x1B:
		if (SemDialogActive) {
			extern int FullEnd;
			extern uint16 _curAnimFrame[];

			if (FullEnd != _curAnimFrame[1])
				FullEnd = _curAnimFrame[1] + 1;
		} else if (!SemSomeOneSpeak && !Semscriptactive && !SemDialogActive && !SemDialogMenuActive && (_actor._curAction < hWALKIN) && !SemUseWithStarted && _playingAnims[1] == 0 && SemShowHomo) {
			actorStop();
			nextStep();
			Mouse(1);
			SemMouseEnabled = true;
			_obj[o00EXIT]._goRoom = _curRoom;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, rSYS, 0, 0, c);
			SemShowHomo = false;
			SemCharacterExist = false;
			IconSnapShot();
		}
		break;

	// Sys
	case 0x3B:
		if (!SemSomeOneSpeak && !Semscriptactive && !SemDialogActive && !SemDialogMenuActive && (_actor._curAction < hWALKIN) && !SemUseWithStarted && _playingAnims[1] == 0 && SemShowHomo) {
			actorStop();
			nextStep();
			Mouse(1);
			SemMouseEnabled = true;
			_obj[o00EXIT]._goRoom = _curRoom;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, rSYS, 0, 0, c);
			SemShowHomo = false;
			SemCharacterExist = false;
			IconSnapShot();
		}
		break;

	// Save
	case 0x3C:
		if (!SemSomeOneSpeak && !Semscriptactive && !SemDialogActive && !SemDialogMenuActive && (_actor._curAction < hWALKIN) && !SemUseWithStarted && _playingAnims[1] == 0 && SemShowHomo) {
			IconSnapShot();
			SemSaveInventory = true;
			DataSave();
			ShowInvName(NO_OBJECTS, false);
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, mx, my, 0, 0);
			RegenInventory(RegenInvStartIcon, RegenInvStartLine);
		}
		break;

	// Load
	case 0x3D:
		if (!SemSomeOneSpeak && !Semscriptactive && !SemDialogActive && !SemDialogMenuActive && (_actor._curAction < hWALKIN) && !SemUseWithStarted && _playingAnims[1] == 0 && SemShowHomo) {
			IconSnapShot();
			if (!DataLoad()) {
				ShowInvName(NO_OBJECTS, false);
				doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, mx, my, 0, 0);
				RegenInventory(RegenInvStartIcon, RegenInvStartLine);
			}
		}
		break;
	}

	if (GAMEAREA(my))
		if ((_inventoryStatus == INV_ON) || (_inventoryStatus == INV_INACTION)) doEvent(MC_INVENTORY, ME_CLOSE, MP_SYSTEM, 0, 0, 0, 0);

	if (ScrollInvTime > TheTime) ScrollInvTime = TheTime;
	if ((INVAREA(my)) && (TheTime > (INVSCROLLSP + ScrollInvTime))) {
		doScrollInventory(mx);
		ScrollInvTime = TheTime;
	}

	if (ForceQuit && !SemDialogActive  && !SemDialogMenuActive)
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);

}

} // End of namespace Trecision
