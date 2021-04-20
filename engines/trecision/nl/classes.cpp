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
#include "graphics/scaler.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/define.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/trecision.h"
#include "trecision/logic.h"
#include "trecision/video.h"

namespace Trecision {

// inventory
#define COUNTERSTEP 6

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

		if (g_vm->_flagUseWithStarted) {
			if ((g_vm->_obj[g_vm->_curObj]._flag & (OBJFLAG_ROOMOUT | OBJFLAG_ROOMIN)) && !(g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_EXAMINE))
				return;
			g_vm->_flagUseWithStarted = false;
			g_vm->_flagInventoryLocked = false;
			g_vm->_useWith[WITH] = g_vm->_curObj;
			g_vm->_useWithInv[WITH] = false;
			g_vm->_lightIcon = 0xFF;

			if (!g_vm->_useWithInv[USED] && (g_vm->_curObj == g_vm->_useWith[USED])) {
				g_vm->_useWith[USED] = 0;
				g_vm->_useWith[WITH] = 0;
				g_vm->_useWithInv[USED] = false;
				g_vm->_useWithInv[WITH] = false;
				g_vm->_flagUseWithStarted = false;
				g_vm->clearText();
			} else
				doEvent(MC_ACTION, ME_USEWITH, MP_SYSTEM, 0, 0, 0, 0);
			g_vm->_curObj = 0;
			return;
		}

		if ((g_vm->_curMessage->_event == ME_MOUSEOPERATE) && (g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_USEWITH)) {
			g_vm->_flagUseWithStarted = true;
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
			if (g_vm->_flagSomeoneSpeaks || g_vm->_flagDialogMenuActive || g_vm->_flagDialogActive)
				break;

			CheckMask(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2);
			g_vm->_logicMgr->doMouseGame();
		} else if (curpos == POSINV) {
			if (g_vm->_logicMgr->doMouseInventory())
				break;
			if ((g_vm->_flagSomeoneSpeaks && !g_vm->_flagCharacterSpeak) || g_vm->_flagDialogMenuActive || g_vm->_flagDialogActive)
				break;
			if (g_vm->_animMgr->_playingAnims[kSmackerAction])
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

			if (g_vm->_flagDialogMenuActive)
				UpdateChoices(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2);
		}
		break;

	case ME_MRIGHT:
	case ME_MLEFT:
		if (g_vm->_flagSomeoneSpeaks) {
			g_vm->_flagSkipTalk = g_vm->_flagSkipEnable;
			break;
		}
		if (g_vm->_actor->_curAction > hWALKIN)
			break;

		if (g_vm->_flagDialogActive && g_vm->_flagDialogMenuActive) {
			SelectChoice(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2);
			break;
		}

		g_vm->_logicMgr->doMouseLeftRight();
		break;
	default:
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
		g_vm->hideCursor();
		g_vm->_flagShowCharacter = false;
		doEvent(MC_CHARACTER, ME_CHARACTERCONTINUEACTION, MP_DEFAULT, Act, NewRoom, NewPos, g_vm->_curObj);
	} else {
		if ((Act == aWALKIN) || (Act == aWALKOUT))
			g_vm->_curObj = 0;
		g_vm->hideCursor();
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
			g_vm->_flagWaitRegen = true;
		} else
			_characterInMovement = true;

		if (g_vm->_fastWalk) {
			if (nextStep()) {
				_characterInMovement = false;
				_characterGoToPosition = -1;
				g_vm->_flagWaitRegen = true;
			} else
				_characterInMovement = true;
		}

		g_vm->_flagPaintCharacter = true;

		if (_characterInMovement)
			REEVENT;
		else {
			g_vm->showCursor();

			if (g_vm->_curMessage->_event == ME_CHARACTERGOTOACTION)
				doEvent(MC_ACTION, ME_MOUSEOPERATE, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, g_vm->_curMessage->_u32Param);
			else if (g_vm->_curMessage->_event == ME_CHARACTERGOTOEXAMINE)
				doEvent(MC_ACTION, ME_MOUSEEXAMINE, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, 0, g_vm->_curMessage->_u32Param);
			else if (g_vm->_curMessage->_event == ME_CHARACTERGOTOEXIT) {
				g_vm->_flagShowCharacter = false;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, g_vm->_curMessage->_u8Param, g_vm->_curMessage->_u32Param);
			} else if (g_vm->_curMessage->_event == ME_CHARACTERDOACTION) {
				g_vm->_lastObj = 0;
				ShowObjName(g_vm->_curObj, true);
				g_vm->refreshInventory(g_vm->_inventoryRefreshStartIcon, g_vm->_inventoryRefreshStartLine);
			}
		}
		break;

	case ME_CHARACTERACTION:
		if (g_vm->_flagWaitRegen)
			REEVENT;
		g_vm->_characterQueue.initQueue();
		g_vm->_inventoryRefreshStartLine = INVENTORY_HIDE;
		g_vm->refreshInventory(g_vm->_inventoryRefreshStartIcon, INVENTORY_HIDE);
		g_vm->_inventoryStatus = INV_OFF;
		if (g_vm->_curMessage->_u16Param1 > hLAST) {
			g_vm->_animMgr->startSmkAnim(g_vm->_curMessage->_u16Param1);
			InitAtFrameHandler(g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u32Param);
			g_vm->hideCursor();
			doEvent(MC_CHARACTER, ME_CHARACTERCONTINUEACTION, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, g_vm->_curMessage->_u8Param, g_vm->_curMessage->_u32Param);
		} else
			actorDoAction(g_vm->_curMessage->_u16Param1);

		g_vm->clearText();
		break;

	case ME_CHARACTERCONTINUEACTION:
		g_vm->_flagShowCharacter = false;
		AtFrameHandler(CHARACTER_ANIM);
		//	If the animation is over
		if (!g_vm->_animMgr->_playingAnims[kSmackerAction]) {
			g_vm->showCursor();
			g_vm->_flagShowCharacter = true;
			_characterInMovement = false;
			g_vm->_characterQueue.initQueue();
			AtFrameEnd(CHARACTER_ANIM);
			g_vm->_flagWaitRegen = true;
			g_vm->_lastObj = 0;
			ShowObjName(g_vm->_curObj, true);
			//	If the room changes at the end
			if (g_vm->_curMessage->_u16Param2) {
				g_vm->_flagShowCharacter = false;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_curMessage->_u16Param2, 0, g_vm->_curMessage->_u8Param, g_vm->_curMessage->_u32Param);
			} else if (g_vm->_curMessage->_u8Param)
				setPosition(g_vm->_curMessage->_u8Param);

			if ((g_vm->_curMessage->_u16Param1 == g_vm->_obj[oCANCELLATA1B]._anim)
			&& !(g_vm->_obj[oBOTTIGLIA1D]._mode & OBJMODE_OBJSTATUS)
			&& !(g_vm->_obj[oRETE17]._mode & OBJMODE_OBJSTATUS)) {
				PlayDialog(dF181);
				g_vm->hideCursor();
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
			return;

		// if regen still has to occur
		if (g_vm->_flagWaitRegen)
			REEVENT;

		g_vm->_logicMgr->doSystemChangeRoom();

		setPosition(g_vm->_curMessage->_u8Param);
		actorStop();

		if (g_vm->_curMessage->_u16Param2)
			StartCharacterAction(g_vm->_curMessage->_u16Param2, 0, 0, 0);

		g_vm->_logicMgr->endChangeRoom();

		g_vm->_room[g_vm->_curRoom]._flag |= OBJFLAG_DONE; // Visited
		drawCharacter(CALCPOINTS);			// for right _actorPos entrance

		break;
	default:
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                                  doIdle            					   */
/*-------------------------------------------------------------------------*/
void doIdle() {
	char c = g_vm->GetKey();
	switch (c) {
	// Quit
	case 'q':
	case 'Q':
		if (!g_vm->_flagDialogActive && !g_vm->_flagDialogMenuActive) {
			if (QuitGame())
				doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		}
		break;

	// Skip
	case 0x1B:
		if (!g_vm->_flagSomeoneSpeaks && !g_vm->_flagscriptactive && !g_vm->_flagDialogActive && !g_vm->_flagDialogMenuActive
		&& (g_vm->_actor->_curAction < hWALKIN) && !g_vm->_flagUseWithStarted && g_vm->_flagShowCharacter && !g_vm->_animMgr->_playingAnims[kSmackerAction]) {
			actorStop();
			nextStep();
			g_vm->showCursor();
			g_vm->_obj[o00EXIT]._goRoom = g_vm->_curRoom;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, rSYS, 0, 0, c);
			g_vm->_flagShowCharacter = false;
			g_vm->_flagCharacterExists = false;
			::createThumbnailFromScreen(&g_vm->_thumbnail);
		}
		break;

	// Sys
	case 0x3B:
		if (!g_vm->_flagSomeoneSpeaks && !g_vm->_flagscriptactive && !g_vm->_flagDialogActive && !g_vm->_flagDialogMenuActive
		&& (g_vm->_actor->_curAction < hWALKIN) && !g_vm->_flagUseWithStarted && g_vm->_flagShowCharacter
		&& !g_vm->_animMgr->_playingAnims[kSmackerAction]) {
			actorStop();
			nextStep();
			g_vm->showCursor();
			g_vm->_obj[o00EXIT]._goRoom = g_vm->_curRoom;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, rSYS, 0, 0, c);
			g_vm->_flagShowCharacter = false;
			g_vm->_flagCharacterExists = false;
			::createThumbnailFromScreen(&g_vm->_thumbnail);
		}
		break;

	// Save
	case 0x3C:
		if (!g_vm->_flagSomeoneSpeaks && !g_vm->_flagscriptactive && !g_vm->_flagDialogActive && !g_vm->_flagDialogMenuActive
		&& (g_vm->_actor->_curAction < hWALKIN) && !g_vm->_flagUseWithStarted && g_vm->_flagShowCharacter
		&& !g_vm->_animMgr->_playingAnims[kSmackerAction]) {
			::createThumbnailFromScreen(&g_vm->_thumbnail);
			DataSave();
			g_vm->showInventoryName(NO_OBJECTS, false);
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, g_vm->_mouseX, g_vm->_mouseY, 0, 0);
			g_vm->refreshInventory(g_vm->_inventoryRefreshStartIcon, g_vm->_inventoryRefreshStartLine);
		}
		break;

	// Load
	case 0x3D:
		if (!g_vm->_flagSomeoneSpeaks && !g_vm->_flagscriptactive && !g_vm->_flagDialogActive && !g_vm->_flagDialogMenuActive
		&& (g_vm->_actor->_curAction < hWALKIN) && !g_vm->_flagUseWithStarted && g_vm->_flagShowCharacter
		&& !g_vm->_animMgr->_playingAnims[kSmackerAction]) {
			::createThumbnailFromScreen(&g_vm->_thumbnail);
			if (!DataLoad()) {
				g_vm->showInventoryName(NO_OBJECTS, false);
				doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, g_vm->_mouseX, g_vm->_mouseY, 0, 0);
				g_vm->refreshInventory(g_vm->_inventoryRefreshStartIcon, g_vm->_inventoryRefreshStartLine);
			}
		}
		break;
	default:
		break;
	}

	if (GAMEAREA(g_vm->_mouseY) && ((g_vm->_inventoryStatus == INV_ON) || (g_vm->_inventoryStatus == INV_INACTION)))
		doEvent(MC_INVENTORY, ME_CLOSE, MP_SYSTEM, 0, 0, 0, 0);

	if (g_vm->_inventoryScrollTime > TheTime)
		g_vm->_inventoryScrollTime = TheTime;

	if (isInventoryArea(g_vm->_mouseY) && (TheTime > (INVSCROLLSP + g_vm->_inventoryScrollTime))) {
		g_vm->doScrollInventory(g_vm->_mouseX);
		g_vm->_inventoryScrollTime = TheTime;
	}

	if (g_engine->shouldQuit() && !g_vm->_flagDialogActive && !g_vm->_flagDialogMenuActive)
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
}

} // End of namespace Trecision
