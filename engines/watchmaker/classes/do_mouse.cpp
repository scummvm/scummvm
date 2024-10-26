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

#include "watchmaker/classes/do_mouse.h"
#include "watchmaker/t3d.h"
#include "watchmaker/globvar.h"
#include "watchmaker/message.h"
#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/define.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/schedule.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/main.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/classes/do_dialog.h"
#include "watchmaker/classes/do_inv.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/3d/animation.h"
#include "watchmaker/classes/do_string.h"
#include "watchmaker/classes/do_keyboard.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/ll/ll_string.h"
#include "watchmaker/t2d/t2d.h"
#include "watchmaker/walk/walk.h"
#include "watchmaker/renderer.h"

namespace Watchmaker {

// locals
t3dV3F LastClickPos;
uint8 LastFloorHit;

void doMouseButton(WGame &game) {
	uint8 cp;
	Init &init = game.init;
	Renderer &renderer = *game._renderer;

	if (bT2DActive) {
		doT2DMouse(game);
		return ;
	}
	if (bSomeOneSpeak && !bNotSkippableSent) {
		/*DebugLogWindow("skip");*/bSkipTalk = true;
		return;
	}
	if (bDialogActive) {
		UpdateDialogMenu(game, TheMessage->wparam1, TheMessage->wparam2, TheMessage->event);
		return;
	}
	if ((bPlayerInAnim) || (bNotSkippableWalk) || (bMovingCamera)) return;
	if (mHide) mHide = false;
	if ((bClock33) || (CurObj == o33LANCETTAMSX) || (CurObj == o33LANCETTAHSX) || (CurObj == o33LANCETTAMDX) || (CurObj == o33LANCETTAHDX)) return;
	CurInvObj = 0;
	CurObj = 0;

	// se sono su inventario
	if (InvStatus & INV_ON) {
		if ((InvStatus & INV_ON) && (InvStatus & INV_MODE1) && PlayerCanCall(game._gameVars)) {
			if (CheckRect(renderer, game._gameRect._callOtherPlayerRect, TheMessage->wparam1, TheMessage->wparam2)) {
				_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dRCALLOTHERPLAYER, 0, 0, nullptr, nullptr, nullptr);
				return;
			}
		}

		CurInvObj = WhatIcon(*game._renderer, TheMessage->wparam1, TheMessage->wparam2);

		if ((CurInvObj == iNULL) && (InvStatus & INV_MODE2) && CheckRect(renderer, game._gameRect._bigIconRect, TheMessage->wparam1, TheMessage->wparam2))
			CurInvObj = BigInvObj;

		if (CurInvObj == iNULL) {
			// se ho cliccato su icona grande
			if (InvStatus & INV_MODE2) {
				if (CheckRect(renderer, game._gameRect._inv1Up, TheMessage->wparam1, TheMessage->wparam2))
					InventoryUp();
				else if (CheckRect(renderer, game._gameRect._inv1Down, TheMessage->wparam1, TheMessage->wparam2))
					InventoryDown();
				else if (CheckRect(renderer, game._gameRect._closeInvRect, TheMessage->wparam1, TheMessage->wparam2)) {
					if (InvStatus & INV_MODE4) {
						CurPlayer ^= 1;
						ChangePlayer(game, (uint8)((CurPlayer ^ 1) + ocDARRELL));
					}

					InvStatus = INV_OFF;
					game._cameraMan->resetAngle();
					game._cameraMan->GetCameraTarget(init, &t3dCurCamera->Target);

					if (bFirstPerson)
						game._renderer->setCurCameraViewport(CAMERA_FOV_1ST, bSuperView);
					else
						game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);

					BigInvObj = iNULL;
				} else if (CheckRect(renderer, game._gameRect._quitGameRect, TheMessage->wparam1, TheMessage->wparam2))
					CloseSys(game);
				else if (CheckRect(renderer, game._gameRect._invSaveRect, TheMessage->wparam1, TheMessage->wparam2) && !(InvStatus & INV_MODE4) && PlayerCanSave())
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, MPX_START_T2D_SAVE, 0, tOPTIONS, nullptr, nullptr, nullptr);
				else if (CheckRect(renderer, game._gameRect._invLoadRect, TheMessage->wparam1, TheMessage->wparam2) && !(InvStatus & INV_MODE4))
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, MPX_START_T2D_LOAD, 0, tOPTIONS, nullptr, nullptr, nullptr);
				else if (CheckRect(renderer, game._gameRect._invOptionsRect, TheMessage->wparam1, TheMessage->wparam2) && !(InvStatus & INV_MODE4))
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, MPX_START_T2D_OPTIONS, 0, tOPTIONS, nullptr, nullptr, nullptr);
				else {
					ClearUseWith();
					if ((CheckRect(renderer, game._gameRect._playerInvRect, TheMessage->wparam1, TheMessage->wparam2)) || (InvStatus & INV_MODE4)) {
						if (PlayerCanSwitch(game._gameVars, 0)) {
							InvStatus ^= INV_MODE4;
							CurPlayer ^= VICTORIA;
							BigInvObj = iNULL;
							return;
						}
					}
				}
			} else if (CheckRect(renderer, game._gameRect._inv1Up, TheMessage->wparam1, TheMessage->wparam2))
				InventoryUp();
			else if (CheckRect(renderer, game._gameRect._inv1Down, TheMessage->wparam1, TheMessage->wparam2))
				InventoryDown();
			else {

				InvStatus = INV_OFF;
				ClearUseWith();
			}
			return;
		}

		if ((BigInvObj != CurInvObj) && !(bUseWith & UW_ON)) {
			t3dMatIdentity(&game.init._globals._invVars.BigIconM);
			BigInvObj = CurInvObj;
		}

		if (TheMessage->event == ME_MRIGHT)
			_vm->_messageSystem.doEvent(EventClass::MC_INVENTORY, ME_OPERATEICON, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, &CurInvObj, nullptr, nullptr);
		else if (bUseWith & UW_ON)
			_vm->_messageSystem.doEvent(EventClass::MC_INVENTORY, ME_OPERATEICON, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, &CurInvObj, nullptr, nullptr);
		else
			_vm->_messageSystem.doEvent(EventClass::MC_INVENTORY, ME_EXAMINEICON, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, &CurInvObj, nullptr, nullptr);

		//if( ( InvStatus & INV_MODE2 ) && CheckRect(BigIconRect,TheMessage->wparam1,TheMessage->wparam2) )
		//CurInvObj = iNULL;

		return;
	} else if ((bUseWith & UW_ON) && (bUseWith & UW_USEDI) && CheckRect(renderer, game._gameRect._useIconRect, TheMessage->wparam1, TheMessage->wparam2)) {
		ClearUseWith();
		ClearText();
		return;
	}

//			se sono su area gioco
	cp = 0;
	PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
	NextPortalObj = oNULL;
	NextPortalAnim = aNULL;
//			se gli ho gia' passato che oggetto deve trovare
	if ((CurObj = TheMessage->lparam[0])) {
//				se era per un passaggio di portale, cancella l'oggetto
		if (CurObj == oNEXTPORTAL) CurObj = oNULL;
//				Ripristina informazioni
		bFastWalk = (uint8)TheMessage->lparam[2];
		FloorHit = LastFloorHit;
		t3dVectCopy(&mPos, &LastClickPos);
	} else {
//				Cerca l'oggetto su cui ho cliccato
		CurObj = WhatObj(game, TheMessage->wparam1, TheMessage->wparam2, TheMessage->event);
//				Se sono su albero e clicco fuori dal nido, simulo un cambio portale
		if (bPlayerSuBasamento &&
		        (CurObj != oXT14ALBERO) && (CurObj != oXT14BASAMENTO) && (CurObj != oXT14NIDO_da_sopra_il_basamento) && (CurObj != oXT14OCCHIALI)) {
			NextPortalObj = CurObj;
			NextPortalAnim = a145;
		}

//				se non e' stato cliccato nessun oggetto e un oggetto era gia' selezionato dall'inventario il tasto
//				destro del mouse elimina questa selezione (nd Fox)
		if ((CurObj == oNULL) && (bUseWith) && (TheMessage->event == ME_MRIGHT)) {
			ClearUseWith();
			return;
		}

//				Salva informazioni
		LastFloorHit = FloorHit;
		t3dVectCopy(&LastClickPos, &mPos);
	}
	if (bFirstPerson && FromFirstPersonAnim && (!CurObj || (CurObj && (!(init.Obj[CurObj].flags & NOSWITCH))))) {
		if (CurObj)
			NextPortalObj = CurObj;
		else
			NextPortalObj = oNEXTPORTAL;
		NextPortalAnim = FromFirstPersonAnim;
		FromFirstPersonAnim = aNULL;
	}
//			Se ha cliccato in un altro portale e c'e' un'animazione di link
	if ((NextPortalObj) && (NextPortalAnim)) {
//				Prende luce di posizione dall'animazione
		cp = init.Anim[NextPortalAnim].pos;
		PlayerGotoPos[CurPlayer + ocDARRELL] = GetLightPosition(&mPos, cp);
//				Simula un click per terra
		CurObj = oNULL;
		FloorHit = 1;
	} else if ((CurObj) && !(NextPortalObj) && (init.Obj[CurObj].pos != 0)) {
//				altrimenti prova a prendere la luce di posizione
		cp = init.Obj[CurObj].pos;
		PlayerGotoPos[CurPlayer + ocDARRELL] = GetLightPosition(&mPos, cp);
		FloorHit = 1;
	}
//			Se deve prendere sempre un click col destro
	if (init.Obj[CurObj].flags & FORCERIGHT) TheMessage->event = ME_MRIGHT;
//			Cammina solo; nessuno oggetto o nessuna posizione o posizione diversa e non sono in closeup
	if ((!CurObj) || ((PlayerGotoPos[CurPlayer + ocDARRELL]) && (PlayerGotoPos[CurPlayer + ocDARRELL] != PlayerPos[CurPlayer + ocDARRELL])) || (!PlayerGotoPos[CurPlayer + ocDARRELL])) {
		CheckCharacterWithBounds(game, ocCURPLAYER, &mPos, cp, 10);

//				Se ho cliccato in un altro portale dice di contiunare la camminata
		if ((NextPortalObj) && (NextPortalAnim))
			_vm->_messageSystem.doEvent(EventClass::MC_MOUSE, TheMessage->event, MP_WAIT_ANIM, 0, 0, 0, &NextPortalObj, nullptr, &bFastWalk);
		else if (NextPortalObj)
			_vm->_messageSystem.doEvent(EventClass::MC_MOUSE, TheMessage->event, MP_WAIT_PORTAL, 0, 0, 0, &NextPortalObj, nullptr, &bFastWalk);

//				Se sono in prima torno in terza, a meno che l'oggetto non mi obblighi a restare in prima
		if (bFirstPerson && !bNoFirstPersonSwitch && (!CurObj || (CurObj && (!(init.Obj[CurObj].flags & NOSWITCH) || (Player->Walk.Check & LONGPATH)))))
			_vm->_messageSystem.doEvent(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
//?             else if( bFirstPerson )
//?                 CharStop( ocCURPLAYER );
	}
//			Rimuove tutti gli eventi del personaggio
	game._messageSystem.removeEvent(EventClass::MC_PLAYER, ME_ALL);
	if (CurObj) {                        // se ho cliccato su un oggetto valido
		if (TheMessage->event == ME_MLEFT)   // se ho cliccato con sinistro
			_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTOEXAMINE, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, bFirstPerson, &CurObj, nullptr, nullptr);
		else if (TheMessage->event == ME_MRIGHT) {   // se ho cliccato con destro
			if ((init.Obj[CurObj].flags & USEWITH)  && !(bUseWith)) {      // se ho cliccato su un oggetto usacon
				Player->Walk.NumSteps = 0;          // Simula un PlayerStop
				_vm->_messageSystem.doEvent(EventClass::MC_ACTION, ME_MOUSEOPERATE, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, bFirstPerson, &CurObj, nullptr, nullptr);
			} else
				_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTOACTION, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, bFirstPerson, &CurObj, nullptr, nullptr);
		}
	} else                              // se non ho cliccato su niente
		_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTO, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, bFirstPerson, nullptr, &NextPortalAnim, nullptr);
}

void doMouseUpdate(WGame &game) {
	Init &init = game.init;
	Renderer &renderer = *game._renderer;

	if (bT2DActive) {
		doT2DMouse(game);
		return ;
	}
	if (bDialogActive) {
		UpdateDialogMenu(game, TheMessage->wparam1, TheMessage->wparam2, TheMessage->event);
		return;
	}
	if (/*( mHide ) ||*/ (bSomeOneSpeak)) return;
	if (!TheMessage->wparam1 && !TheMessage->wparam2) return;
	if ((bPlayerInAnim) || (bNotSkippableWalk) || (mHide) || (bMovingCamera)) return;

	CurObj = 0;
	CurInvObj = 0;
	//DebugFile("MM %d %d",TheMessage->wparam1, TheMessage->wparam2 );

	// se sono su inventario
	if (InvStatus & INV_ON) {
		if ((bLPressed) && (InvStatus & INV_MODE2)) {
			t3dM3X3F t;
			t3dMatRot(&t, ((t3dF32)(TheMessage->lparam[1]) / (t3dF32)(game._gameRect._bigIconRect.y2 - game._gameRect._bigIconRect.y1))*T3D_PI * 2.0f,
			          ((t3dF32)(TheMessage->lparam[0]) / (t3dF32)(game._gameRect._bigIconRect.x1 - game._gameRect._bigIconRect.x2))*T3D_PI * 2.0f, 0.0f);
			t3dMatMul(&game.init._globals._invVars.BigIconM, &t, &game.init._globals._invVars.BigIconM);
			CurInvObj = BigInvObj;
		} else {
			CurInvObj = WhatIcon(*game._renderer, TheMessage->wparam1, TheMessage->wparam2);
			if ((CurInvObj == iNULL) && CheckRect(renderer, game._gameRect._bigIconRect, TheMessage->wparam1, TheMessage->wparam2))
				CurInvObj = BigInvObj;
			ShowInvObjName(init, CurInvObj);
		}
		return;
	}

	// se sono su area gioco
	CurObj = WhatObj(game, TheMessage->wparam1, TheMessage->wparam2, TheMessage->event);

	if ((bLPressed || bRPressed) && ((bClock33) || (CurObj == o33LANCETTAMSX) || (CurObj == o33LANCETTAHSX) || (CurObj == o33LANCETTAMDX) || (CurObj == o33LANCETTAHDX)))
		doClock33(game, CurObj, &mPos);
	else {
		if (bClock33) {
			if ((Comb33[0] == 7) && (Comb33[1] == 2) && (Comb33[2] == 9) && (Comb33[3] == 11) && IconInInv(init, i19FOGLIO1)) {
				_vm->_messageSystem.doEvent(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
				_vm->_messageSystem.doEvent(EventClass::MC_ANIM, ME_STARTANIM, MP_WAIT_CAMERA, a336, 0, 0, nullptr, nullptr, nullptr);

				if (!(init.Obj[o33LANCETTAHSX].flags & EXTRA2)) {
					IncCurTime(game, 15);
					init.Obj[o33LANCETTAHSX].flags |= EXTRA2;
				}
			}
			bClock33 = false;
		}
		ShowObjName(init, CurObj);
	}
}

/* -----------------17/03/98 17.16-------------------
 *                      doMouse
 * --------------------------------------------------*/
void doMouse(WGame &game) {
	//se ci sono i crediti ritorna
	if (bTitoliCodaStatic || bTitoliCodaScrolling)  return;

	switch (TheMessage->event) {
	case ME_MRIGHT:
	case ME_MLEFT:
		doMouseButton(game);
		break;
	case ME_MOUSEUPDATE :
		doMouseUpdate(game);
		break;
	case ME_MOUSEHIDE :
		if ((mCounter++ > 20) && !(mHide))
			mHide = 1;
		ClearText();
		break;
	case ME_MOUSEUNHIDE :
		mHide = 0;
		break;
	}
}

} // End of namespace Watchmaker
