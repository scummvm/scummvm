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

#include "watchmaker/classes/do_action.h"
#include "watchmaker/game.h"
#include "watchmaker/globvar.h"
#include "watchmaker/message.h"
#include "watchmaker/ll/ll_string.h"
#include "watchmaker/define.h"
#include "watchmaker/schedule.h"
#include "watchmaker/classes/do_string.h"
#include "watchmaker/classes/do_inv.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/classes/do_operate.h"

namespace Watchmaker {

/* -----------------05/06/00 11.10-------------------
 *                  CheckRoomPuzzle
 * --------------------------------------------------*/
bool CheckRoomPuzzle(WGame &game, uint8 ev) {
	if ((game._gameVars.getCurRoomId() == r2G) && (WhichRoomChar(game.init, ocGIARDINIERE) == r2G) && ((CurObj != o2Gp2H) && (CurObj != ocGIARDINIERE))) {
		ClearText();
		ClearUseWith();

		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dGIARDINIERE_INTERRUPT2, 0, 0, nullptr, nullptr, nullptr);
		return true;
	} else if ((game._gameVars.getCurRoomId() == r2G) && (WhichRoomChar(game.init, ocCUSTODE) == r2G) && ((CurObj != o2Gp2H) && (CurObj != ocCUSTODE))) {
		ClearText();
		ClearUseWith();

		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dCUSTODE_INTERRUPT5, 0, 0, nullptr, nullptr, nullptr);
		return true;
	}
	return false;
}

/* -----------------19/05/98 16.40-------------------
 *                  doDoor
 * --------------------------------------------------*/
void doDoor(WGame &game, int32 obj) {
	int32 anim = aNULL;
	Init &init = game.init;

	if (!obj) return;

	anim = init.Obj[obj].anim[CurPlayer];
	switch (obj) {
	case 0: // Quiet VS C4065 warning
	default:
		anim = init.Obj[obj].anim[CurPlayer];
		if (init.Obj[obj].goroom)
			game._gameVars.setCurRoomId(init.Obj[obj].goroom);
		break;
	}

	if (anim != aNULL) StartAnim(game, anim);
}

/* -----------------19/05/98 16.40-------------------
 *                  doTake
 * --------------------------------------------------*/
void doTake(WGame &game, int32 obj) {
	int32 anim = aNULL;
	//uint8 del = TRUE;

	if (!obj) return;

	anim = game.init.Obj[obj].anim[CurPlayer];
	switch (obj) {
	case o25CHIAVI:
//			if ( Anim[aMOSTRAFOTOACUOCO].active == 0 )
//				return ;
		break;

	default:
		//del = TRUE;
		break;
	}

	if (anim) StartAnim(game, anim);

	// spegne oggetto che viene preso
//	if(del) Obj[obj].flags &= ~ON;
	// aggiunge icona
	AddIcon(game.init, game.init.Obj[obj].ninv);
}

/* -----------------19/05/98 16.40-------------------
 *                  doExamine
 * --------------------------------------------------*/
void doExamine(WGame &game, int32 obj) {
	int32 anim = aNULL;
	uint8 sent = FALSE;
	int32 log_item = lNULL;
	Init &init = game.init;

	if (!obj) return;

	if (init.Obj[obj].flags & EXAMINEACT)
		anim = init.Obj[obj].anim2[CurPlayer];
	switch (obj) {
	case o1DVALIGETTACH:
		UpdateSpecial(game, r1D);
		anim = aTO1PERSON;
		break;

	case o31BAULECH:
		UpdateSpecial(game, r31);
		anim = aTO1PERSON;
		break;

	case oXT1ETARGHETTA:
		log_item = lPDA2_MENU5_RAUL_ITEM2;
		break;

	case o13STATUETTE:
		log_item = lPDA2_MENU6_CUS_ITEM3;
		break;

	case o29TARGA1:
	case o29TARGA2:
	case o29TARGA3:
	case o29TARGA4:
	case o29COPPA:
		log_item = lPDA2_MENU8_SUP_ITEM2;
		break;

	case o46PRIMODIAGRAMMA:
		log_item = lPDA6_MENU31_ITEM1;
		break;

	case o46SECONDODIAGRAMMA:
//			se il flag EXTRA non e' settato vuol dire che non si e' ancora entrati nella r45 e non si
//			puo' conoscere il significato del disegno
		if (!(init.Obj[o46SECONDODIAGRAMMA].flags & EXTRA)) {
			PlayerSpeak(game, init.Obj[o13QUADRO1].action[CurPlayer]);
			return;
		} else    log_item = lPDA6_MENU31_ITEM2;
		break;

	default:
		sent = TRUE;
		break;
	}

	if (log_item != lNULL) {
		if (!(init.PDALog[log_item].flags & PDA_ON)) {
			init.PDALog[log_item].flags |= (PDA_ON | PDA_UPDATE);
			init.PDALog[log_item].time = t3dCurTime;
			_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND * 3, 0, EFFECT_DISPLAY_NEWLOGIMG, nullptr, nullptr, nullptr);
		}
		sent = TRUE;
	}

	if (anim != aNULL) StartAnim(game, anim);
	else if ((sent) && (init.Obj[obj].examine[CurPlayer])) PlayerSpeak(game, init.Obj[obj].examine[CurPlayer]);
}

/* -----------------19/03/98 16.31-------------------
 *                  doAction
 * --------------------------------------------------*/
void doAction(WGame &game) {
	Init &init = game.init;
	switch (TheMessage->event) {
	case ME_MOUSEEXAMINE:
		CurObj = TheMessage->lparam[0];

		if (CheckRoomPuzzle(game, TheMessage->event))
			break;

		if (bUseWith & UW_ON) {
			UseWith[WITH] = CurObj;
			bUseWith &= ~UW_ON;
			ClearText();
			// fa l'usa con
			doUseWith(game);
			break;
		}

		doExamine(game, CurObj);
		break;

	case ME_MOUSEOPERATE:
		CurObj = TheMessage->lparam[0];

		if (CheckRoomPuzzle(game, TheMessage->event))
			break;

		if (bUseWith & UW_ON) {
			UseWith[WITH] = CurObj;
			bUseWith &= ~UW_ON;
			ClearText();
			// fa l'usa con
			doUseWith(game);
			break;
		}

		if (init.Obj[CurObj].flags & USEWITH) {
			UseWith[USED] = CurObj;
			bUseWith = UW_ON;
			ShowObjName(init, CurObj);
			CurMousePointer = MousePointerPlus;
		} else if (init.Obj[CurObj].flags & CHARACTER) {
			if (bFirstPerson) {
				_vm->_messageSystem.doEvent(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
				_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_CAMERA, init.Obj[CurObj].goroom, 0, 0, nullptr, nullptr, nullptr);
			} else
				_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, init.Obj[CurObj].goroom, 0, 0, nullptr, nullptr, nullptr);
		} else if (init.Obj[CurObj].flags & TAKE)
			doTake(game, CurObj);
		else if (init.Obj[CurObj].flags & ROOM)
			doDoor(game, CurObj);
		else
			doOperate(game, CurObj);

		break;
	}
}

} // End of namespace Watchmaker
