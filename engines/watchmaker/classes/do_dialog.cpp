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

#include "watchmaker/classes/do_dialog.h"
#include "watchmaker/globvar.h"
#include "watchmaker/define.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/message.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/schedule.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/main.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/classes/do_inv.h"
#include "watchmaker/ll/ll_string.h"
#include "watchmaker/classes/do_system.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/classes/do_string.h"
#include "watchmaker/renderer.h"

namespace Watchmaker {

// locals
int32 DebugSent = 0;
int16 NextDlg = dNULL;
int32 ic1, ic2;

/* -----------------03/06/98 10.11-------------------
 *                  doDialog
 * --------------------------------------------------*/
void doDialog(WGame &game) {
	Init &init = game.init;
	struct SItemCommand *ic;
	uint8 r;

	switch (TheMessage->event) {
	case ME_DIALOGSTART:
		if (TheMessage->wparam1 == dR391) {
			if (Character[ocCHIRURGO]->Mesh)
				t3dVectFill(&Character[ocCHIRURGO]->Mesh->Trasl, 0.0f);
			if (Character[ocVECCHIO]->Mesh)
				t3dVectFill(&Character[ocVECCHIO]->Mesh->Trasl, 0.0f);
			if (Character[ocOROLOGIAIO]->Mesh)
				t3dVectFill(&Character[ocOROLOGIAIO]->Mesh->Trasl, 0.0f);
			if (Character[ocTRADUTTORE]->Mesh)
				t3dVectFill(&Character[ocTRADUTTORE]->Mesh->Trasl, 0.0f);

			bSuperView = 1;
			game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);
		}

		//faccio apparire la scritta di descrizione della stanza di Darrell
		if ((TheMessage->wparam1 == dR000) && bShowRoomDescriptions) {
			t3dCurTime = 240;
			RoomInfo.name[0] = '\0';
			UpdateRoomInfo(game);
		}

		// I make darrell disappear
		if ((TheMessage->wparam1 == dRLOGHI) || (TheMessage->wparam1 == dR000)) {
			Character[ocDARRELL]->Flags |= T3D_CHARACTER_HIDE;  //I hide darrell to make sure he doesn't show while he loads the animations
		}

		if ((init.Dialog[TheMessage->wparam1].flags & DIALOG_ONCE) && (init.Dialog[TheMessage->wparam1].flags & DIALOG_DONE))
			return ;

		StopDiary(game, game._gameVars.getCurRoomId(), init.Dialog[TheMessage->wparam1].obj, 0);
		CurDialog = TheMessage->wparam1;
		bDialogActive = true;
		bDialogMenuActive = true;

		CurDlgItem = -1;
		CurMenu = mMAIN;
		InvStatus = INV_OFF;
		BigInvObj = iNULL;
		NextDlg = dNULL;
		ClearUseWith();
		ClearText();
		CharStop(ocCURPLAYER);

		if (bFirstPerson)
			game._renderer->setCurCameraViewport(74, bSuperView);

		bDialogMenuActive = false;
//DebugFile("DLG: StartDialog %d",CurDialog);
//			Se sono in un fullmotion
		if ((init.Dialog[CurDialog].flags & DIALOG_RTV2) && (init.Dialog[CurDialog].ItemIndex[mRTV2]))
			_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGCONTINUE, MP_DEFAULT, (int16)CurDialog, mRTV2, 0, nullptr, nullptr, nullptr);
		else if ((init.Dialog[CurDialog].flags & DIALOG_RTV3) && (init.Dialog[CurDialog].ItemIndex[mRTV3]))
			_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGCONTINUE, MP_DEFAULT, (int16)CurDialog, mRTV3, 0, nullptr, nullptr, nullptr);
		else if ((init.Dialog[CurDialog].flags & DIALOG_RTV) || (init.Dialog[CurDialog].ItemIndex[mRTV]))
			_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGCONTINUE, MP_DEFAULT, (int16)CurDialog, mRTV, 0, nullptr, nullptr, nullptr);
//			Se c'e' un predialog attivo
		else if (init.Dialog[CurDialog].flags & (DIALOG_PRE1 | DIALOG_PRE2 | DIALOG_PRE3 | DIALOG_PRE4)) {
			if (init.Dialog[CurDialog].flags & DIALOG_PRE_RAND)
				while (!(init.Dialog[CurDialog].flags & (DIALOG_PRE1 << (r = (game._rnd->getRandomNumber(3))))));
			else if (init.Dialog[CurDialog].flags & DIALOG_PRE1) r = 0;
			else if (init.Dialog[CurDialog].flags & DIALOG_PRE2) r = 1;
			else if (init.Dialog[CurDialog].flags & DIALOG_PRE3) r = 2;
			else r = 3;

			if (init.Dialog[CurDialog].ItemIndex[mPREDIALOG1 + r])
				_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGCONTINUE, MP_DEFAULT, (int16)CurDialog, (int16)(mPREDIALOG1 + r), 0, nullptr, nullptr, nullptr);
			else
				bDialogMenuActive = true;
		} else
			bDialogMenuActive = true;

		break;

	case ME_DIALOGCONTINUE:
		CurDialog = TheMessage->wparam1;
		CurDlgItem = TheMessage->wparam2;
		bDialogMenuActive = false;

		ic = &init.DlgItem[init.Dialog[CurDialog].ItemIndex[CurDlgItem]].item[CurPlayer][TheMessage->lparam[0]];
//			Finche' non ci sono comandi che deve aspettare tempo, li fa tutti
		while (ic->com) {
			TheMessage->lparam[0] ++;
//				DebugLogFile("IC %d %d %d | %d %d %d",ic->com,ic->param1,ic->param2,CurDialog,CurDlgItem,TheMessage->lparam[0]);
			switch (ic->com) {
			case IC_NULL:                                                              // Esce dal dialogo
				_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGEND, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
				break;
			case IC_SET_PLAYER:                                                        // non fa niente
				break;
			case IC_ANIM:
//DebugFile("DLG: StartAnim %d",ic->param1);
				StartAnim(game, GetAlternateAnim(init, ic->param1));
				break;
			case IC_SET_CAMERA:
				ForcedCamera = GetAlternateCamera(init, (uint8)ic->param1);
				bCutCamera = true;
				bAllowCalcCamera = false;
				break;
			case IC_MOVE_CAMERA_TO:
				ForcedCamera = GetAlternateCamera(init, (uint8)ic->param1);
				bCutCamera = false;
				bAllowCalcCamera = true;
				break;
			case IC_SET_TARGET:
				CameraTargetObj = ic->param1;
				CameraTargetBone = ic->param2;
				break;
			case IC_SET_CHAR:
				CharSetPosition(ic->param1, GetAlternatePosition(init, (uint8)ic->param2), nullptr);
				CharStop(ic->param1);
				break;
			case IC_WALK_CHAR:
			case IC_RUN_CHAR:
			case IC_BACK_CHAR:
				CharGotoPosition(game, ic->param1, GetAlternatePosition(init, (uint8)ic->param2), (uint8)(ic->com - IC_WALK_CHAR), 0);
				TimeWalk = 0;
				break;
			case IC_HIDE_CHAR:
				Character[ic->param1]->Flags |= T3D_CHARACTER_HIDE;
				break;
			case IC_UNHIDE_CHAR:
				Character[ic->param1]->Flags &= ~T3D_CHARACTER_HIDE;
				break;
			case IC_CHANGE_ROOM:
				ChangeRoom(game, Common::String::format("%s.t3d", init.Room[ic->param1].name), 0, aNULL);
				break;
			case IC_EXPRESSION:
				if (Character[ic->param1])
					Character[ic->param1]->CurExpressionSet = ic->param2;
				break;
			case IC_CHANGE_PLAYER:
				UpdatePlayerStand(game, (uint8)(CurPlayer + ocDARRELL));
				CurPlayer = (ic->param1 == ocDARRELL) ? DARRELL : VICTORIA;
				Character[ocCURPLAYER] = Character[ic->param1];
				Player = Character[ocCURPLAYER];
				Player->Flags &= ~T3D_CHARACTER_HIDE;
				break;
			case IC_DEBUG:
				DebugSent = ic->param1;
				break;
			case IC_ITEM:
				init.DlgMenu[ic->param1].on = (uint8)ic->param2;
				break;
			case IC_SET_FLAGS:
				if (ic->param1 != dNULL)
					init.Dialog[ic->param1].flags |= ic->param2;
				else
					init.Dialog[CurDialog].flags |= ic->param2;
				break;
			case IC_CLR_FLAGS:
				if (ic->param1 != dNULL)
					init.Dialog[ic->param1].flags &= ~ic->param2;
				else
					init.Dialog[CurDialog].flags &= ~ic->param2;
				break;
			case IC_ATFRAME:
				init.Anim[aDUMMY].atframe[0].type = (uint8)ic->param1;
				init.Anim[aDUMMY].atframe[0].index = ic->param2;
				init.Anim[aDUMMY].active = 12;
				ProcessATF(game, aDUMMY, 0);
				init.Anim[aDUMMY].active = 0;
				break;
			case IC_NEXT_DLG:
				NextDlg = (int16)ic->param1;
				break;
			case IC_SET_CHAR2:
				if (CurAlternate[ic->param1]) break;
				CharSetPosition(ic->param1, GetAlternatePosition(init, (uint8)ic->param2), nullptr);
				CharStop(ic->param1);
				break;
			case IC_INTRO_TEXT1:
				ic1 = ic->param1;
				ic2 = ic->param2;
				_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND, FRAME_PER_SECOND, EFFECT_FADEOUT_T1, &ic1, nullptr, nullptr);
				if (ic2)
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_WAITA, FRAME_PER_SECOND, FRAME_PER_SECOND, EFFECT_MOVEIN_T1, &ic1, nullptr, &ic2);
				break;
			case IC_INTRO_TEXT2:
				ic1 = ic->param1;
				ic2 = ic->param2;
				_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND, FRAME_PER_SECOND, EFFECT_FADEOUT_T2, &ic1, nullptr, nullptr);
				if (ic2)
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_WAITA, FRAME_PER_SECOND, FRAME_PER_SECOND, EFFECT_MOVEIN_T2, &ic1, nullptr, &ic2);
				break;

			case IC_TIME_ANIM:
//DebugFile("DLG: StartTimeAnim %d %d",ic->param1,ic->param2);
				AnimAutoPush = ic->param2;
				StartAnim(game, GetAlternateAnim(init, ic->param1));
				TheMessage->flags |= MP_WAIT_ANIM;
				TimeAnim = GetAlternateAnim(init, ic->param1);
				ReEvent();
				return ;
			case IC_TIME_ANIM2:
//DebugFile("DLG: StartTimeAnim %d %d",ic->param1,ic->param2);
				StartAnim(game, GetAlternateAnim(init, ic->param1));
				TheMessage->flags |= MP_WAIT_ANIM;
				TimeAnim = GetAlternateAnim(init, ic->param1);
				ReEvent();
				return ;
			case IC_TIME_WALK_CHAR:
			case IC_TIME_RUN_CHAR:
			case IC_TIME_BACK_CHAR:
				if (!CharGotoPosition(game, ic->param1, GetAlternatePosition(init, (uint8)ic->param2), (uint8)(ic->com - IC_TIME_WALK_CHAR), 0)) break;
				TimeWalk = ic->param1;
				TheMessage->flags |= MP_WAIT_ACT;
				ReEvent();
				return ;
			case IC_TIME_WAIT_CAMERA:
				TheMessage->flags |= MP_WAIT_CAMERA;
				ReEvent();
				return ;
			case IC_TIME_WAIT:
				if (TheMessage->lparam[1] == 0) {
					TheMessage->lparam[1] = ic->param1;
					if (DebugSent && (LoaderFlags & T3D_DEBUGMODE)) PlayerSpeak(game, DebugSent);
				}
				if (TheMessage->lparam[1] > 1) {
					TheMessage->lparam[0] --;
					TheMessage->lparam[1] --;
					TheMessage->flags |= MP_WAIT_RETRACE;
					ReEvent();
					return;
				}
				TheMessage->lparam[1] = 0;
				bSkipTalk = false;
				DebugSent = 0;
				break;
			case IC_TIME_FADOUT:
				_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, ic->param1, 1, EFFECT_FADOUT, nullptr, nullptr, nullptr);
				TheMessage->flags |= MP_WAITA;
				ReEvent();
				return ;
			}
			ic = &init.DlgItem[init.Dialog[CurDialog].ItemIndex[CurDlgItem]].item[CurPlayer][TheMessage->lparam[0]];
		}
		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGEND, MP_DEFAULT, (int16)CurDialog, (int16)CurDlgItem, 0, nullptr, nullptr, nullptr);
		break;

	case ME_DIALOGEND:
	case ME_DIALOGEND_SKIPINTRO:
		if (TheMessage->wparam1 == dR391) {
			bSuperView = 0;
			game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);
			NextDlg = dR391_end;
		}

		CurDialog = TheMessage->wparam1;
		CurDlgItem = TheMessage->wparam2;
		game._messageSystem.removeEvent(EventClass::MC_DIALOG, ME_ALL);
//			Se c'e' un enddialog attivo
		if ((CurDlgItem == mQUIT) && (init.Dialog[CurDialog].flags & (DIALOG_END1 | DIALOG_END2 | DIALOG_END3))) {
			if (init.Dialog[CurDialog].flags & DIALOG_END_RAND)
				while (!(init.Dialog[CurDialog].flags & (DIALOG_END1 << (r = (game._rnd->getRandomNumber(2))))));
			else if (init.Dialog[CurDialog].flags & DIALOG_END1) r = 0;
			else if (init.Dialog[CurDialog].flags & DIALOG_END2) r = 1;
			else r = 2;

			if (init.Dialog[CurDialog].ItemIndex[mENDDIALOG1 + r]) {
				_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGCONTINUE, MP_DEFAULT, (int16)(CurDialog), (int16)(mENDDIALOG1 + r), 0, nullptr, nullptr, nullptr);
				return ;
			}
		}

		if ((CurDlgItem == mRTV) || (CurDlgItem == mRTV2) || (CurDlgItem == mRTV3) || (CurDlgItem == mQUIT) ||
		        (CurDlgItem == mENDDIALOG1) || (CurDlgItem == mENDDIALOG2) || (CurDlgItem == mENDDIALOG3)) {
//DebugFile("DLG: EndDialog %d",CurDialog);
			StopObjAnim(game, ocCURPLAYER);
			CharStop(ocCURPLAYER);   //evito che negli interrupt (in particolare) rimanga in memoria l'animazione di ascolta
			game._cameraMan->ResetCameraTarget();
			init.Dialog[CurDialog].flags |= DIALOG_DONE;
			CurDialog = dNULL;
			bDialogActive = false;
			bDialogMenuActive = false;
			bAnimWaitText = false;
			CurDlgItem = -1;
			CurMenu = mNULL;
			ForcedCamera = 0;
			bCutCamera = false;
			bAllowCalcCamera = false;
			CameraTargetObj = ocCURPLAYER;
			CameraTargetBone = 0;
			TimeWalk = ocCURPLAYER;
			TimeAnim = aNULL;
			bPlayerInAnim = false;
			if (NextDlg != dNULL)
				_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, NextDlg, 0, 0, nullptr, nullptr, nullptr);
			else {
				if ((init.Dialog[TheMessage->wparam1].obj) && (Character[init.Dialog[TheMessage->wparam1].obj]))
					StartDiary(game, game._gameVars.getCurRoomId(), &Character[init.Dialog[TheMessage->wparam1].obj]->Mesh->Trasl);
				else
					StartDiary(game, game._gameVars.getCurRoomId(), nullptr);

				DebugLogFile("EndDialog: resetto t3dLastCameraIndex");
				game._cameraMan->resetLastCameraIndex(); //forzo ProcessCamera() a cercare una nuova camera, in modo finito il dialogo non rimane qualche strana camera
			}
		} else
			bDialogMenuActive = true;

		if (TheMessage->event == ME_DIALOGEND_SKIPINTRO)
			if (! DataLoad(game, "WmStart.dat", 0)) {
				DebugLogFile("SkipIntro: DataLoad() Failed. Quitting ...");
				CloseSys(game);
			}

		if (TheMessage->wparam1 == dRLOGHI)
			ProcessATFDO(game, fSTART_MAIN_MENU);

		break;

	}
}

/* -----------------06/07/00 10.48-------------------
 *                  GetAlternatePosition
 * --------------------------------------------------*/
uint8 GetAlternatePosition(Init &init, uint8 pos) {
	struct SDialog *d = &init.Dialog[CurDialog];
	int32 alt;

	if (!d || !d->obj || !(alt = CurAlternate[d->obj])) return pos;

	alt --;
	if (d->AltPosSco[alt]) return pos + d->AltPosSco[alt];

	return pos;
}

/* -----------------06/07/00 10.49-------------------
 *                  GetAlternateCamera
 * --------------------------------------------------*/
uint8 GetAlternateCamera(Init &init, uint8 cam) {
	struct SDialog *d = &init.Dialog[CurDialog];
	int32 alt;

	if (!d || !d->obj || !(alt = CurAlternate[d->obj])) return cam;

	alt --;
	if (d->AltCamSco[alt]) return cam + d->AltCamSco[alt];

	return cam;
}

/* -----------------06/07/00 10.49-------------------
 *                  GetAlternateAnim
 * --------------------------------------------------*/
int32 GetAlternateAnim(Init &init, int32 an) {
	struct SDialog *d = &init.Dialog[CurDialog];
	int32 a, alt;

	if (!d || !d->obj || !(alt = CurAlternate[d->obj])) return an;

	alt --;
	for (a = 0; a < MAX_ALT_ANIMS; a++)
		if ((d->AltAnims[alt][a][0] == an) && (d->AltAnims[alt][a][1] != aNULL))
			return d->AltAnims[alt][a][1];

	return an;
}

/* -----------------03/06/98 11.31-------------------
 *                  UpdateDialogMenu
 * --------------------------------------------------*/
void UpdateDialogMenu(WGame &game, int16 dmx, int16 dmy, uint8 db) {
	struct SRect t;
	int32 a, ca1, ca2;
	Init &init = game.init;
	Renderer &renderer = *game._renderer;

	if ((bDialogActive == FALSE) || (CurDialog == dNULL) || (bDialogMenuActive == FALSE))
		return ;

	if ((db == ME_MLEFT) && (CurDlgItem) && (init.DlgMenu[CurDlgItem].parent == mMAIN)) {
		CurMenu = CurDlgItem;
		Diag2Base = 0;
	} else if ((db == ME_MLEFT) && (CurDlgItem) && (CurDlgItem != -1)) { // TODO: The original didn't have a check for -1 here
		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGCONTINUE, MP_DEFAULT, (int16)CurDialog, (int16)CurDlgItem, 0, nullptr, nullptr, nullptr);
		UsedDlgMenu[CurPlayer][CurObj][CurDlgItem] = 1;
	} else if ((db == ME_MRIGHT) && (CurMenu == mMAIN))
		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGEND, MP_DEFAULT, (int16)CurDialog, (int16)mQUIT, 0, nullptr, nullptr, nullptr);
	else if (db == ME_MRIGHT) {
		CurMenu = mMAIN;
		Diag2Base = 0;
	}

//if( ( db == ME_MLEFT ) || ( db == ME_MRIGHT ) ) DebugFile("DLG: Click %d %d",CurMenu,CurDlgItem);

	for (a = 0, ca1 = 0, ca2 = 0; a < MAX_DLG_MENUS; a++) {
		if (!(init.DlgMenu[a].on)) continue;

		// se la servetta mi ha gia' dato la banconota devo fare in modo che non me la dia ancora
		if ((a == mCOSEFATTI8) && (CurObj == ocSERVETTA) && (IconInInv(init, i2cBANCONOTA1)))   continue;

		CurDlgItem = a;
		if (init.DlgMenu[a].parent == mMAIN) {
			t.x1 = DIAG1_MARG_SX;
			t.x2 = DIAG1_MARG_DX;
			t.y1 = DIAG1_MARG_UP + DIAG_DY * ca1;
			t.y2 = DIAG1_MARG_UP + DIAG_DY * (ca1 + 1);

			ca1++;
			if (CheckRect(renderer, t, dmx, dmy)) return;
		} else if (init.DlgMenu[a].parent == CurMenu) {
			t.x1 = DIAG2_MARG_SX;
			t.x2 = DIAG2_MARG_DX;
			t.y1 = DIAG2_MARG_UP + DIAG2_DY * (ca2 - Diag2Base);
			t.y2 = DIAG2_MARG_UP + DIAG2_DY * ((ca2 - Diag2Base) + 1);

			ca2++;
			if ((ca2 < Diag2Base) || (ca2 > (Diag2Base + MAX_DIAG2_ITEMS))) continue;
			if (CheckRect(renderer, t, dmx, dmy)) return;
		}
	}
	CurDlgItem = -1;
	if ((db == ME_MLEFT) || (db == ME_MRIGHT)) {
		//  Se si cambia la logica di incremento variarla anche in PaintDialog() nella gestione delle freccette
		if (CheckRect(renderer, game._gameRect._diag2Up, dmx, dmy))
			Diag2Base = ((Diag2Base - 1) < 0) ? 0 : Diag2Base - 1;
		else if (CheckRect(renderer, game._gameRect._diag2Down, dmx, dmy))
			if ((Diag2Base + 1 + MAX_DIAG2_ITEMS) <= ca2)
				Diag2Base ++;
	}
}

/* -----------------03/06/98 10.47-------------------
 *                  PaintDialog
 * --------------------------------------------------*/
void PaintDialog(WGame &game) {
	int32 a, ca1, ca2;
	int32 tx = 0, ty = 0;
	FontColor tc;
	Init &init = game.init;

	if ((bDialogActive == FALSE) || (CurDialog == dNULL) || (bDialogMenuActive == FALSE))
		return ;

	game._renderer->_2dStuff.displayDDBitmap(ConsoleD1, 7, 366, 0, 0, 0, 0);
	DisplayD3DRect(*game._renderer, 15, 373, 187, 211, 18, 25, 18, 128);
	game._renderer->_2dStuff.displayDDBitmap(ConsoleD2, 223, 515, 0, 0, 0, 0);
	DisplayD3DRect(*game._renderer, 223 + 6, 515 + 6, 536, 62, 18, 25, 18, 128);
	DisplayD3DRect(*game._renderer, 223 + 546, 515 + 22, 16, 30, 18, 25, 18, 128);
	DisplayD3DRect(*game._renderer, 223 + 542, 515 + 18, 4, 38, 18, 25, 18, 128);

	for (a = 0, ca1 = 0, ca2 = 0; a < MAX_DLG_MENUS; a++) {
		if (!(init.DlgMenu[a].on) || ((init.DlgMenu[a].parent != mMAIN) && (init.DlgMenu[a].parent != CurMenu))) continue;

		// se la servetta mi ha gia' dato la banconota devo fare in modo che non me la dia ancora
		if ((a == mCOSEFATTI8) && (CurObj == ocSERVETTA) && (IconInInv(init, i2cBANCONOTA1)))   continue;

		tc = WHITE_FONT;
		if (init.DlgMenu[a].parent == mMAIN) {
			tx = DIAG1_MARG_SX;
			ty = DIAG1_MARG_UP + DIAG_DY * ca1;
			ca1++;

			if (CurMenu == a) tc = YELLOW_FONT;
		} else if (init.DlgMenu[a].parent == CurMenu) {
			tx = DIAG2_MARG_SX;
			ty = DIAG2_MARG_UP + DIAG2_DY * (ca2 - Diag2Base);
			ca2++;
			if ((ca2 <= Diag2Base) || (ca2 > (Diag2Base + MAX_DIAG2_ITEMS))) continue;

			if (UsedDlgMenu[CurPlayer][CurObj][a])  tc = GRAY_FONT;
		}
		if (CurDlgItem == a) tc = RED_FONT;

		game._renderer->_2dStuff.displayDDText(Sentence[init.DlgMenu[a].titolo], FontKind::Standard, tc, tx, ty, 0, 0, 0, 0);
	}


//	disegno le frecce
	if (Diag2Base > 0)
		game._renderer->_2dStuff.displayDDBitmap(ConsoleFrecciaSu, 223 + 550, 515 + 0, 0, 0, 0, 0);

	if ((Diag2Base + 1 + MAX_DIAG2_ITEMS) <= ca2)
		game._renderer->_2dStuff.displayDDBitmap(ConsoleFrecciaGiu, 223 + 550, 515 + 56, 0, 0, 0, 0);
}

} // End of namespace Watchmaker
