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

#include "watchmaker/classes/do_inv.h"
#include "watchmaker/define.h"
#include "watchmaker/types.h"
#include "watchmaker/globvar.h"
#include "watchmaker/message.h"
#include "watchmaker/schedule.h"
#include "watchmaker/ll/ll_string.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/classes/do_string.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/classes/do_inv_inv.h"
#include "watchmaker/classes/do_inv_scr.h"
#include "watchmaker/classes/do_scr_scr.h"
#include "watchmaker/classes/do_sound.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/renderer.h"

namespace Watchmaker {

/* -----------------03/04/98 10.39-------------------
 *                      IconInInv
 * --------------------------------------------------*/
uint8 IconInInv(Init &/*init*/, uint8 icon) {
	uint8 i;

	for (i = 0; i < MAX_ICONS_IN_INV; i++)
		if ((Inv[CurPlayer][i] == icon) || (Inv[CurPlayer ^ 1][i] == icon))
			break;
	if (i == MAX_ICONS_IN_INV)
		return false;
	else
		return true;
}

/* -----------------03/04/98 10.39-------------------
 *                      IconPos
 * --------------------------------------------------*/
uint8 IconPos(uint8 icon) {
	uint8 i;

	for (i = 0; i < MAX_ICONS_IN_INV; i++)
		if (Inv[CurPlayer][i] == icon)
			break;
	return i;
}

/* -----------------03/04/98 10.41-------------------
 *                  KillIcon
 * --------------------------------------------------*/
void KillIcon(Init &init, uint8 icon) {
	uint8 pos = IconPos(icon), op = CurPlayer;
	if (pos == MAX_ICONS_IN_INV) {
		CurPlayer ^= 1;
		if ((pos = IconPos(icon)) == MAX_ICONS_IN_INV) {
			CurPlayer = op;
			return;
		}
	}

	init.InvObj[icon].flags &= ~ON;
	Inv[CurPlayer][pos] = iNULL;
	for (; pos < InvLen[CurPlayer]; pos++)
		Inv[CurPlayer][pos] = Inv[CurPlayer][pos + 1];
	InvLen[CurPlayer] --;

	if (InvBase[CurPlayer])
		if ((InvLen[CurPlayer] > MAX_SHOWN_ICONS) && (Inv[CurPlayer][InvBase[CurPlayer] + MAX_SHOWN_ICONS] == iNULL))
			InvBase[CurPlayer] = InvLen[CurPlayer] - MAX_SHOWN_ICONS;

	if (CurInvObj == icon) CurInvObj = iNULL;
	if (BigInvObj == icon) BigInvObj = iNULL;

	CurPlayer = op;
}


/* -----------------03/04/98 10.50-------------------
 *                  AddIcon
 * --------------------------------------------------*/
void AddIcon(Init &init, uint8 icon) {
	if (IconInInv(init, icon))
		return;

	init.InvObj[icon].flags |= ON;
	Inv[CurPlayer][InvLen[CurPlayer]++] = icon;

	if (InvLen[CurPlayer] < MAX_ICONS_IN_INV)
		if (InvBase[CurPlayer] < InvLen[CurPlayer] - MAX_SHOWN_ICONS)
			InvBase[CurPlayer] = InvLen[CurPlayer] - MAX_SHOWN_ICONS;
}

/* -----------------04/06/98 16.34-------------------
 *                  ClearUseWith
 * --------------------------------------------------*/
void ClearUseWith() {
	bUseWith = UW_OFF;
	UseWith[USED] = 0;
	UseWith[WITH] = 0;
	CurMousePointer = MousePointerDefault;
}

/* -----------------25/09/00 17.40-------------------
 *                  SelectCurBigIcon
 * --------------------------------------------------*/
void SelectCurBigIcon(uint8 icon) {
	uint8 pos = IconPos(icon);
	if (pos == MAX_ICONS_IN_INV) {
		return;
	}

	CurInvObj = icon;
	BigInvObj = icon;
}

/* -----------------23/04/98 14.45-------------------
 *                  doUseWith
 * --------------------------------------------------*/
void doUseWith(WGame &game) {
	if (bUseWith & UW_USEDI) {
		if (bUseWith & UW_WITHI)
			doInvInvUseWith(game);
		else
			doInvScrUseWith(game);
//?     BigInvObj = UseWith[USED];
	} else {
		doScrScrUseWith(game);
		BigInvObj = iNULL;
	}

	ClearUseWith();
}

/* -----------------03/04/98 10.53-------------------
 *                  ReplaceIcon
 * --------------------------------------------------*/
void ReplaceIcon(Init &init, uint8 oldicon, uint8 newicon) {
	uint8 pos = IconPos(oldicon), op = CurPlayer;
	if (pos == MAX_ICONS_IN_INV) {
		CurPlayer ^= 1;
		if ((pos = IconPos(oldicon)) == MAX_ICONS_IN_INV) {
			CurPlayer = op;
			return;
		}
	}

	init.InvObj[oldicon].flags &= ~ON;
	init.InvObj[newicon].flags |= ON;
	Inv[CurPlayer][pos] = newicon;

	if (CurInvObj == oldicon) CurInvObj = newicon;
	if (BigInvObj == oldicon) BigInvObj = newicon;

	CurPlayer = op;
}


/* -----------------03/04/98 10.16-------------------
 *                  doInventory
 * --------------------------------------------------*/
void doInventory(WGame &game) {
	uint8 ci;
	Init &init = game.init;

	switch (TheMessage->event) {
	case ME_EXAMINEICON:
		CurInvObj = TheMessage->lparam[0];

		if (CurInvObj && (bUseWith & UW_ON)) {
			UseWith[WITH] = CurInvObj;

			bUseWith &= ~UW_ON;
			bUseWith |= UW_WITHI;
			ClearText();
			// fa l'usa con
			doUseWith(game);
		} else if (CurInvObj) {
			if (InvStatus & INV_MODE4) {
				BigInvObj = iNULL;
				if (
				    bSezioneLabirinto
				    || (CurInvObj == i00TELEFONO) || (CurInvObj == i00TELEFONOVIC)
				) {
					PlayerSpeak(game, init.Obj[o2ACOMPUTER].action[CurPlayer]);
					break;
				}

				ci = (uint8)CurInvObj;
				KillIcon(init, ci);
				CurPlayer ^= 1;
				AddIcon(init, ci);
				CurPlayer ^= 1;
				break;
			} else if (InvStatus & INV_MODE5) {
				BigInvObj = CurInvObj;
				ReplaceSaveLoadTexture(CurInvObj);
				break;
			} else if ((InvStatus & INV_MODE2) && ((CurInvObj == i00TELEFONO) || (CurInvObj == i00TELEFONOVIC))) {
				_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, tPDA, nullptr, nullptr, nullptr);
				break;
			}
			BigInvObj = CurInvObj;

//				Quando schiaccio sul nome dell'icona per aprire l'inventario grande non gli faccio dire la frase
			if ((InvStatus & INV_ON) && (InvStatus & INV_MODE2))
				PlayerSpeak(game, init.InvObj[CurInvObj].examine[CurPlayer]);
			else
				InvStatus = INV_ON | INV_MODE2;
		}
		break;

	case ME_OPERATEICON:
		CurInvObj = TheMessage->lparam[0];

		if (bUseWith & UW_ON) {
			UseWith[WITH] = CurInvObj;
			bUseWith &= ~UW_ON;
			bUseWith |= UW_WITHI;
			ClearText();
			// fa l'usa con
			doUseWith(game);
			break;
		}

		if (InvStatus & INV_MODE4) {
			InvStatus &= ~INV_MODE4;
			CurPlayer ^= 1;
			CurInvObj = BigInvObj = iNULL;
		} else if (init.InvObj[CurInvObj].flags & USEWITH) {
			if (!(InvStatus & INV_MODE2))
				InvStatus = INV_OFF;
			UseWith[USED] = CurInvObj;
			bUseWith = UW_ON | UW_USEDI;
			ShowInvObjName(init, CurInvObj);
			CurMousePointer = MousePointerPlus;
		} else if (CurInvObj) {
			if (init.InvObj[CurInvObj].anim[CurPlayer]) {
				_vm->_messageSystem.doEvent(EventClass::MC_INVENTORY, ME_INVOFF, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
				StartAnim(game, init.InvObj[CurInvObj].anim[CurPlayer]);
				break;
			} else {
				int an;
				if (CurInvObj == i28WALKMANOK) {
					if (init.InvObj[CurInvObj].flags & EXTRA)       an = a289_noinctime;
					else                                                                        an = a289;
					_vm->_messageSystem.doEvent(EventClass::MC_INVENTORY, ME_INVOFF, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
					StartAnim(game, an);
				}
			}
			InvStatus = INV_ON | INV_MODE2;

			if (CurInvObj == i22LIBROCHIUSODEPLIANT) {
				StartSound(game, w228);
				ReplaceIcon(init, i22LIBROCHIUSODEPLIANT, i22LIBROAPERTODEPLIANT);
			} else if (CurInvObj == i22LIBROAPERTODEPLIANT) {
				ReplaceIcon(init, i22LIBROAPERTODEPLIANT, i22DEPLIANT);
				AddIcon(init, i22LIBROAPERTO);
				IncCurTime(game, 10);
			} else if (CurInvObj == i22LIBROAPERTO)
				ReplaceIcon(init, i22LIBROAPERTO, i22LIBROCHIUSO);
			else if (CurInvObj == i22LIBROCHIUSO)
				ReplaceIcon(init, i22LIBROCHIUSO, i22LIBROAPERTO);
			else if (CurInvObj == i3bLASTRA2VOLTI) {
				StartSound(game, w3B22);
				ReplaceIcon(init, i3bLASTRA2VOLTI, i3bLASTRABIANCA);
				AddIcon(init, i3bLASTRANERA);
			} else
				PlayerSpeak(game, init.InvObj[CurInvObj].action[CurPlayer]);
		}
		break;

	case ME_INVOFF:
	case ME_INVMODE1:
	case ME_INVMODE2:
	case ME_INVMODE3:
	case ME_INVMODE4:
	case ME_INVSWITCH:

		ClearText();
		if (bSomeOneSpeak) bSkipTalk = true;

		PlayerPos[CurPlayer + ocDARRELL] = 0;
		PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
		game._messageSystem.removeEvent(EventClass::MC_PLAYER, ME_ALL);
		CharStop(ocCURPLAYER);

		if (TheMessage->event == ME_INVOFF)
			InvStatus = INV_OFF;
		else if (TheMessage->event == ME_INVSWITCH) {
			if (InvStatus & INV_ON) {
				if (InvStatus & INV_MODE4) {
					CurPlayer ^= 1;
					ChangePlayer(game, (uint8)((CurPlayer ^ 1) + ocDARRELL));
				}
				InvStatus = INV_OFF;
				game._cameraMan->resetAngle();
				game._cameraMan->GetCameraTarget(init, &t3dCurCamera->Target);
			} else {
				rGrabVideo("temp.tmp", 1);
				InvStatus = INV_ON | INV_MODE1;
			}
		} else
			InvStatus = INV_ON + (INV_MODE1 << (TheMessage->event - ME_INVMODE1));

		if (InvStatus & INV_ON) {
			if (TheMessage->event == ME_INVSWITCH) InvStatus |= INV_MODE1;
			ClearUseWith();
			mHide = false;
		}

		if (bFirstPerson)
			game._renderer->setCurCameraViewport(CAMERA_FOV_1ST, bSuperView);
		else
			game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);
		BigInvObj = TheMessage->wparam1;
		if (TheMessage->event == ME_INVMODE3) {
			bUseWith = UW_ON | (UW_USEDI * TheMessage->bparam);
			UseWith[USED] = TheMessage->wparam1;
			CurMousePointer = MousePointerPlus;
		}
		break;
	}
}

/* -----------------03/04/98 10.29-------------------
 *                  InventoryDown
 * --------------------------------------------------*/
void InventoryDown() {
//	Se si cambia la logica di incremento variarla anche in PaintInventory() nella gestione delle freccette

	if (InvBase[CurPlayer] < (InvLen[CurPlayer] - MAX_SHOWN_ICONS))
		InvBase[CurPlayer] ++;
}

/* -----------------03/04/98 10.29-------------------
 *                  InventoryUp
 * --------------------------------------------------*/
void InventoryUp() {
//	Se si cambia la logica di incremento variarla anche in PaintInventory() nella gestione delle freccette

	if (InvBase[CurPlayer] > 0)
		InvBase[CurPlayer] --;
}

/* -----------------03/04/98 10.34-------------------
 *                  WhatIcon
 * --------------------------------------------------*/
uint8 WhatIcon(Renderer &renderer, int16 invmx, int16 invmy) {
	struct SRect t;
	t.x1 = INV_MARG_SX;
	t.y1 = INV_MARG_UP;
	t.x2 = INV_MARG_DX;
	t.y2 = INV_MARG_DOWN;

	if (CheckRect(renderer, t, invmx, invmy))
		return Inv[CurPlayer][(InvBase[CurPlayer] + ((invmy - renderer.rFitY(INV_MARG_UP)) / (renderer.rFitY(ICON_DY))))];
	else
		return iNULL;
}

} // End of namespace Watchmaker
