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
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/define.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/trecision.h"
#include "trecision/video.h"
#include "trecision/logic.h"

namespace Trecision {

/*-------------------------------------------------------------------------*/
/*                                 doRoomIn            					   */
/*-------------------------------------------------------------------------*/
void doRoomIn(uint16 curObj) {
	g_vm->hideCursor();

	uint16 curAction = g_vm->_obj[curObj]._anim;
	uint16 curPos = g_vm->_obj[curObj]._ninv;

	doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[curObj]._goRoom, curAction, curPos, curObj);

	g_vm->_obj[curObj]._flag |= OBJFLAG_DONE;
}

/*-------------------------------------------------------------------------*/
/*                              doRoomOut         						   */
/*-------------------------------------------------------------------------*/
void doRoomOut(uint16 curObj) {
	g_vm->hideCursor();

	uint16 curAction, curPos;
	g_vm->_logicMgr->roomOut(curObj, &curAction, &curPos);
	
	if (curAction)
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, curAction, g_vm->_obj[curObj]._goRoom, curPos, curObj);

	g_vm->_obj[curObj]._flag |= OBJFLAG_DONE;
}

/*-------------------------------------------------------------------------*/
/*                            doMouseExamine           					   */
/*-------------------------------------------------------------------------*/
void doMouseExamine(uint16 curObj) {
	if (!curObj)
		warning("doMouseExamine - curObj not set");

	bool printSentence = g_vm->_logicMgr->mouseExamine(curObj);

	if (printSentence && (g_vm->_obj[curObj]._examine))
		CharacterSay(g_vm->_obj[curObj]._examine);
}

/*-------------------------------------------------------------------------*/
/*                            doMouseOperate           					   */
/*-------------------------------------------------------------------------*/
void doMouseOperate(uint16 curObj) {
	if (!curObj)
		warning("doMouseOperate - curObj not set");

	bool printSentence = g_vm->_logicMgr->mouseOperate(curObj);

	if (printSentence && (g_vm->_obj[curObj]._action))
		CharacterSay(g_vm->_obj[curObj]._action);
}

/*-------------------------------------------------------------------------*/
/*                           doMouseTake           						   */
/*-------------------------------------------------------------------------*/
void doMouseTake(uint16 curObj) {
	if (!curObj)
		warning("doMouseTake - curObj not set");

	// _flagMouseEnabled = false;

	bool del = g_vm->_logicMgr->mouseTake(curObj);
	
	uint16 curAction = g_vm->_obj[curObj]._anim;

	if (curAction)
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, curAction, 0, 0, curObj);

	// Remove object being taken
	if (del) {
		if (curAction) {
			for (uint16 j = 0; j < MAXATFRAME; j++) {
				if ((g_vm->_animMgr->_animTab[curAction]._atFrame[j]._type == ATFCLR) && (g_vm->_animMgr->_animTab[curAction]._atFrame[j]._index == curObj))
					break;

				if (g_vm->_animMgr->_animTab[curAction]._atFrame[j]._type == 0) {
					g_vm->_animMgr->_animTab[curAction]._atFrame[j]._child = 0;
					g_vm->_animMgr->_animTab[curAction]._atFrame[j]._numFrame = 1;
					g_vm->_animMgr->_animTab[curAction]._atFrame[j]._type = ATFCLR;
					g_vm->_animMgr->_animTab[curAction]._atFrame[j]._index = curObj;
					break;
				}
			}
		} else {
			g_vm->_obj[curObj]._mode &= ~OBJMODE_OBJSTATUS;
			RegenRoom();
		}
	}
	g_vm->addIcon(g_vm->_obj[g_vm->_curObj]._ninv);
}

/*-------------------------------------------------------------------------*/
/*                           doMouseTalk           						   */
/*-------------------------------------------------------------------------*/
void doMouseTalk(uint16 curObj) {
	if (!curObj)
		warning("doMouseTalk - curObj not set");

	bool printSentence = g_vm->_logicMgr->mouseTalk(curObj);

	if (printSentence)
		PlayDialog(g_vm->_obj[curObj]._goRoom);
}

/*-------------------------------------------------------------------------*/
/*                                doUseWith           					   */
/*-------------------------------------------------------------------------*/
void doUseWith() {
	if (g_vm->_useWithInv[USED]) {
		if (g_vm->_useWithInv[WITH])
			g_vm->doInventoryUseWithInventory();
		else
			g_vm->doInventoryUseWithScreen();
	} else
		doScreenUseWithScreen();

	g_vm->_useWith[USED] = 0;
	g_vm->_useWith[WITH] = 0;
	g_vm->_useWithInv[USED] = false;
	g_vm->_useWithInv[WITH] = false;
	g_vm->_flagUseWithStarted = false;
}

/*-------------------------------------------------------------------------*/
/*                          doScreenUseWithScreen         				   */
/*-------------------------------------------------------------------------*/
void doScreenUseWithScreen() {
	if (!g_vm->_useWith[USED] || !g_vm->_useWith[WITH])
		warning("doScreenUseWithScreen - _useWith not set properly");

//	stopSmkAnim(_inventoryObj[_useWith[USED]]._anim);
	if (_characterInMovement)
		return;

	bool printSentence = g_vm->_logicMgr->useScreenWithScreen();

	if (printSentence)
		CharacterSay(g_vm->_obj[g_vm->_useWith[USED]]._action);
}

/*-------------------------------------------------------------------------*/
/*                               doInvExamine          					   */
/*-------------------------------------------------------------------------*/
void doInvExamine() {
	if (!g_vm->_curInventory)
		warning("doInvExamine - _curInventory not set properly");

	if (g_vm->_inventoryObj[g_vm->_curInventory]._examine)
		CharacterSay(g_vm->_inventoryObj[g_vm->_curInventory]._examine);
}

/*-------------------------------------------------------------------------*/
/*                               doInvOperate          					   */
/*-------------------------------------------------------------------------*/
void doInvOperate() {
	if (!g_vm->_curInventory)
		warning("doInvOperate - _curInventory not set properly");

	bool printSentence = g_vm->_logicMgr->operateInventory();
	if (g_vm->_inventoryObj[g_vm->_curInventory]._action && printSentence)
		CharacterSay(g_vm->_inventoryObj[g_vm->_curInventory]._action);
}

/*-------------------------------------------------------------------------*/
/*                                 doDoing           					   */
/*-------------------------------------------------------------------------*/
void doDoing() {
	switch (g_vm->_curMessage->_event) {
	case ME_INITOPENCLOSE:
		if (g_vm->_actor->_curAction == hSTAND)
			REEVENT;
		else if (g_vm->_actor->_curFrame == 4)
			doEvent(g_vm->_curMessage->_class, ME_OPENCLOSE, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, g_vm->_curMessage->_u8Param, g_vm->_curMessage->_u32Param);
		else
			REEVENT;

		break;
	case ME_OPENCLOSE: {
		uint16 curObj = g_vm->_curMessage->_u16Param1;
		uint16 curAnim = g_vm->_curMessage->_u16Param2;
		g_vm->_obj[curObj]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
		if (curAnim)
			doEvent(MC_ANIMATION, ME_ADDANIM, MP_SYSTEM, curAnim, 0, 0, 0);

		g_vm->_curMessage->_event = ME_WAITOPENCLOSE;
		}
		// no break!
	case ME_WAITOPENCLOSE:
		RegenRoom();
		if (g_vm->_actor->_curAction == hSTAND)
			g_vm->showCursor();
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                                doScript           					   */
/*-------------------------------------------------------------------------*/
void doScript() {
	static uint32 pauseStartTime = 0;
	Message *message = g_vm->_curMessage;
	uint8 scope = message->_u8Param;
	uint16 index = message->_u16Param1;
	uint16 index2 = message->_u16Param2;
	uint32 value = message->_u32Param;
	SObject *obj = &g_vm->_obj[index];

	switch (message->_event) {
	case ME_PAUSE:
		if (!pauseStartTime) {
			pauseStartTime = TheTime;
			doEvent(message->_class, message->_event, message->_priority, message->_u16Param1, message->_u16Param2, message->_u8Param, message->_u32Param);
		} else if (TheTime >= (pauseStartTime + message->_u16Param1))
			pauseStartTime = 0;
		else
			doEvent(message->_class, message->_event, message->_priority, message->_u16Param1, message->_u16Param2, message->_u8Param, message->_u32Param);

		break;

	case ME_SETOBJ:
		switch (scope) {
		case C_ONAME:
			obj->_name = (uint16)value;
			break;
		case C_OEXAMINE:
			obj->_examine = (uint16)value;
			break;
		case C_OACTION:
			obj->_action = (uint16)value;
			break;
		case C_OGOROOM:
			obj->_goRoom = (uint8)value;
			break;
		case C_OMODE:
			if (value)
				obj->_mode |= (uint8)index2;
			else
				obj->_mode &= ~(uint8)index2;
			break;
		case C_OFLAG:
			if (value)
				obj->_flag |= (uint8)index2;
			else
				obj->_flag &= ~(uint8)index2;
			break;
		default:
			break;
		}
		break;

	case ME_SETINVOBJ:
		switch (scope) {
		case C_INAME:
			g_vm->_inventoryObj[index]._name = (uint16)value;
			break;
		case C_IEXAMINE:
			g_vm->_inventoryObj[index]._examine = (uint16)value;
			break;
		case C_IACTION:
			g_vm->_inventoryObj[index]._action = (uint16)value;
			break;
		case C_IFLAG:
			if (value)
				g_vm->_inventoryObj[index]._flag |= (uint8)index2;
			else
				g_vm->_inventoryObj[index]._flag &= ~(uint8)index2;
			break;
		default:
			break;
		}
		break;

	case ME_ADDICON:
		g_vm->addIcon(index);
		break;

	case ME_KILLICON:
		g_vm->removeIcon(index);
		break;

	case ME_PLAYDIALOG:
		PlayDialog(index);
		break;

	case ME_CHARACTERSAY:
		CharacterSay(message->_u32Param);
		break;

	case ME_PLAYSOUND:
		NLPlaySound(index);
		break;

	case ME_STOPSOUND:
		NLStopSound(index);
		break;

	case ME_REGENROOM:
		RegenRoom();
		break;

	case ME_CHANGER:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, index, index2, value, g_vm->_curObj);
		break;

	default:
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                               AtMouseClick           				   */
/*-------------------------------------------------------------------------*/
bool AtMouseClick(uint16 curObj) {
	bool retVal = g_vm->_logicMgr->mouseClick(curObj);

	return retVal;
}

/* -----------------26/11/97 10.37-------------------
 * 					AtEndChangeRoom
 * --------------------------------------------------*/
void AtEndChangeRoom() {
	//	Specific management of magnetic fields
	if ((g_vm->_curRoom == r2E) && (g_vm->_obj[oPULSANTEADS2D]._mode & OBJMODE_OBJSTATUS)) {
		g_vm->_animMgr->_animTab[aBKG2E]._flag |= SMKANIM_OFF1;
		g_vm->_obj[oCAMPO2E]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA2E]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS2E]._mode |= OBJMODE_OBJSTATUS;
	} else if ((g_vm->_curRoom == r2E) && (g_vm->_obj[oPULSANTEADA2D]._mode & OBJMODE_OBJSTATUS)) {
		g_vm->_animMgr->_animTab[aBKG2E]._flag &= ~SMKANIM_OFF1;
		g_vm->_obj[oCAMPO2E]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA2E]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS2E]._mode &= ~OBJMODE_OBJSTATUS;
	}
	if ((g_vm->_curRoom == r2C) && (g_vm->_obj[oPULSANTEACS2D]._mode & OBJMODE_OBJSTATUS)) {
		g_vm->_animMgr->_animTab[aBKG2C]._flag |= SMKANIM_OFF1;
		g_vm->_obj[oCAMPO2C]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA2C]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS2C]._mode |= OBJMODE_OBJSTATUS;
	} else if ((g_vm->_curRoom == r2C) && (g_vm->_obj[oPULSANTEACA2D]._mode & OBJMODE_OBJSTATUS)) {
		g_vm->_animMgr->_animTab[aBKG2C]._flag &= ~SMKANIM_OFF1;
		g_vm->_obj[oCAMPO2C]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS2C]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA2C]._mode |= OBJMODE_OBJSTATUS;
	}
	if ((g_vm->_curRoom == r28) && (g_vm->_obj[oPULSANTEBBS2D]._mode & OBJMODE_OBJSTATUS)) {
		g_vm->_animMgr->_animTab[aBKG28]._flag |= SMKANIM_OFF2;
		g_vm->_obj[oCESPUGLIO28]._anim = a283;
		g_vm->_obj[oSERPENTETM28]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oSERPENTEAM28]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
	} else if ((g_vm->_curRoom == r28) && (g_vm->_obj[oPULSANTEBBA2D]._mode & OBJMODE_OBJSTATUS)) {
		g_vm->_animMgr->_animTab[aBKG28]._flag &= ~SMKANIM_OFF2;
		g_vm->_obj[oCESPUGLIO28]._anim = a282;
		g_vm->_obj[oSERPENTETM28]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oSERPENTEAM28]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
	}
	if ((g_vm->_curRoom == r28) && (g_vm->_obj[oPULSANTEABS2D]._mode & OBJMODE_OBJSTATUS)) {
		g_vm->_animMgr->_animTab[aBKG28]._flag |= (SMKANIM_OFF1 | SMKANIM_OFF3);
		g_vm->_obj[oCAMPO28]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA28]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS28]._mode |= OBJMODE_OBJSTATUS;
		read3D("282.3d");
	} else if ((g_vm->_curRoom == r28) && (g_vm->_obj[oPULSANTEABA2D]._mode & OBJMODE_OBJSTATUS)) {
		g_vm->_animMgr->_animTab[aBKG28]._flag &= ~(SMKANIM_OFF1 | SMKANIM_OFF3);
		g_vm->_obj[oCAMPO28]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA28]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS28]._mode &= ~OBJMODE_OBJSTATUS;
	}
	// End of specific management for the magnetic fields

	// Actions
	if ((g_vm->_curRoom == r12) && ((g_vm->_oldRoom == r13) || (g_vm->_oldRoom == r16)))
		StartCharacterAction(a1213ARRIVACONASCENSORE, 0, 0, 0);
	else if ((g_vm->_curRoom == r16) && ((g_vm->_oldRoom == r13) || (g_vm->_oldRoom == r12)))
		StartCharacterAction(a1617SCENDECONASCENSORE, 0, 0, 0);
	else if ((g_vm->_curRoom == r13) && (g_vm->_oldRoom == r12))
		StartCharacterAction(a1315ARRIVAASCENSOREALTOENTRA, 0, 0, 0);
	else if ((g_vm->_curRoom == r13) && (g_vm->_oldRoom == r16))
		StartCharacterAction(a1314ARRIVAASCENSOREDABASSO, 0, 0, 0);
	else if ((g_vm->_curRoom == r14) && (g_vm->_oldRoom == r13))
		StartCharacterAction(a1414ENTERROOM, 0, 0, 0);
	else if ((g_vm->_curRoom == r18) && (g_vm->_oldRoom == r17) && !(g_vm->_room[r18]._flag & OBJFLAG_DONE))
		StartCharacterAction(a186GUARDAPIAZZA, 0, 0, 0);
	else if ((g_vm->_curRoom == r18) && (g_vm->_oldRoom == r19))
		StartCharacterAction(a189ENTRADALCLUB, 0, 0, 0);
	else if ((g_vm->_curRoom == r1A) && (g_vm->_oldRoom == r18) && (g_vm->_room[r1A]._flag & OBJFLAG_DONE)) {
		StartCharacterAction(a1A5ENTRA, 0, 0, 0);
		g_vm->_animMgr->_animTab[aBKG1A]._flag |= SMKANIM_OFF1;
	} else if ((g_vm->_curRoom == r1C) && (g_vm->_oldRoom == r1B))
		StartCharacterAction(a1C1ENTRACUNICOLO, 0, 0, 0);
	else if ((g_vm->_curRoom == r1B) && (g_vm->_oldRoom == r1C))
		StartCharacterAction(a1B11ESCETOMBINO, 0, 0, 0);
	else if ((g_vm->_curRoom == r1D) && (g_vm->_oldRoom == r1B) && (g_vm->_obj[oDONNA1D]._mode & OBJMODE_OBJSTATUS)) {
		StartCharacterAction(a1D1SCENDESCALE, 0, 1, 0);
		g_vm->_actor->_lim[0] = 60;
		g_vm->_actor->_lim[2] = 240;
	} else if ((g_vm->_curRoom == r1D) && (g_vm->_oldRoom == r1B) && !(g_vm->_obj[oDONNA1D]._mode & OBJMODE_OBJSTATUS)) {
		StartCharacterAction(a1D12SCENDESCALA, 0, 1, 0);
		g_vm->_actor->_lim[0] = 60;
		g_vm->_actor->_lim[2] = 240;
	} else if ((g_vm->_curRoom == r1B) && (g_vm->_oldRoom == r1D))
		StartCharacterAction(a1B4ESCEBOTOLA, 0, 0, 0);
	else if ((g_vm->_curRoom == r1B) && (g_vm->_oldRoom == r18) && (g_vm->_animMgr->_animTab[aBKG1B]._flag & SMKANIM_OFF1))
		StartCharacterAction(a1B12SCAPPATOPO, 0, 0, 0);
	else if ((g_vm->_curRoom == r24) && (g_vm->_oldRoom == r2H))
		StartCharacterAction(a242, 0, 0, 0);
	else if ((g_vm->_curRoom == r25) && (g_vm->_oldRoom == r2A))
		StartCharacterAction(a257, 0, 0, 0);
	else if ((g_vm->_curRoom == r28) && (g_vm->_oldRoom == r27))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r29) && (g_vm->_oldRoom == r2A))
		StartCharacterAction(a298ESCEBOTOLA, 0, 0, 0);
	else if ((g_vm->_curRoom == r29L) && (g_vm->_oldRoom == r2A))
		StartCharacterAction(a2910ESCEBOTOLA, 0, 0, 0);
	else if ((g_vm->_curRoom == r2A) && (g_vm->_oldRoom == r25))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r2A) && ((g_vm->_oldRoom == r29) || (g_vm->_oldRoom == r29L)))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r2B) && (g_vm->_oldRoom == r2A))
		StartCharacterAction(a2B2ESCEPOZZO, 0, 2, 0);
	else if ((g_vm->_curRoom == r2H) && (g_vm->_oldRoom == r24))
		StartCharacterAction(a2H1ARRIVA, 0, 0, 0);
	else if ((g_vm->_curRoom == r2E) && (g_vm->_oldRoom == r2F))
		StartCharacterAction(a2E5SECONDAPARRIVANDO, 0, 0, 0);
	else if ((g_vm->_curRoom == r2F) && (g_vm->_oldRoom == r2E))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r23B) && (g_vm->_oldRoom == r21))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r23A) && (g_vm->_oldRoom == r21) && (g_vm->_room[r23A]._flag & OBJFLAG_DONE))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r23A) && (g_vm->_oldRoom == r21) && (!(g_vm->_room[r23A]._flag & OBJFLAG_DONE)))
		g_vm->_flagShowCharacter = false;
	else if ((g_vm->_curRoom == r21) && ((g_vm->_oldRoom == r23A) || (g_vm->_oldRoom == r23B)))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r2BL) || (g_vm->_curRoom == r36F) || (g_vm->_curRoom == r41D) || (g_vm->_curRoom == r49M) || (g_vm->_curRoom == r4CT) ||
	         (g_vm->_curRoom == r58T) || (g_vm->_curRoom == r58M) || (g_vm->_curRoom == r59L) || (g_vm->_curRoom == rSYS) ||
	         (g_vm->_curRoom == r12CU) || (g_vm->_curRoom == r13CU)) { // Screens without inventory
		g_vm->_flagShowCharacter = false;
		g_vm->_flagCharacterExists = false;
		g_vm->_flagInventoryLocked = true;
	} else if ((g_vm->_curRoom == r31P) || (g_vm->_curRoom == r35P)) { // Screens with inventory
		g_vm->_flagShowCharacter = false;
		g_vm->_flagCharacterExists = false;
	} else if ((g_vm->_curRoom == r2F) && (g_vm->_oldRoom == r31))
		StartCharacterAction(a2F4ESCEASCENSORE, 0, 0, 0);
	else if ((g_vm->_curRoom == r31) && (g_vm->_oldRoom == r2F))
		StartCharacterAction(a3114ESCEPASSAGGIO, 0, 0, 0);
	else if ((g_vm->_curRoom == r31) && !(g_vm->_room[r31]._flag & OBJFLAG_DONE))
		setPosition(14);
	else if ((g_vm->_curRoom == r32) && (g_vm->_oldRoom == r31))
		StartCharacterAction(a321SALEMONTACARICHI, 0, 0, 0);
	else if ((g_vm->_curRoom == r33) && (g_vm->_oldRoom == r32) && (g_vm->_obj[oBRUCIATURA33]._mode & OBJMODE_OBJSTATUS))
		StartCharacterAction(a3311SALESCALE, 0, 0, 0);
	else if ((g_vm->_curRoom == r33) && (g_vm->_oldRoom == r32) && !(g_vm->_obj[oBRUCIATURA33]._mode & OBJMODE_OBJSTATUS))
		StartCharacterAction(a3313CHIUDEBOTOLA, 0, 0, 0);
	else if ((g_vm->_curRoom == r32) && (g_vm->_oldRoom == r33))
		StartCharacterAction(a325SCENDESCALE, 0, 0, 0);
	else if ((g_vm->_curRoom == r36) && (g_vm->_oldRoom == r35))
		StartCharacterAction(a361ESCEASCENSORE, 0, 0, 0);
	else if ((g_vm->_curRoom == r35) && (g_vm->_oldRoom == r36))
		StartCharacterAction(a3515ESCEASCENSORE, 0, 0, 0);
	else if ((g_vm->_curRoom == r44) && (g_vm->_oldRoom == r45 || g_vm->_oldRoom == r45S) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a445, 0, 0, 0);
	else if ((g_vm->_curRoom == r45 || g_vm->_curRoom == r45S) && (g_vm->_oldRoom == r44) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a457, 0, 0, 0);
	else if ((g_vm->_curRoom == r46) && (g_vm->_oldRoom == r47 || g_vm->_oldRoom == r48 || g_vm->_oldRoom == r49) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r47) && (g_vm->_oldRoom == r46) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a476, 0, 0, 0);
	else if ((g_vm->_curRoom == r48) && (g_vm->_oldRoom == r46) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a485, 0, 0, 0);
	else if ((g_vm->_curRoom == r49) && (g_vm->_oldRoom == r46) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a494, 0, 0, 0);
	else if ((g_vm->_curRoom == r54) && (g_vm->_oldRoom == r53)) {
		StartCharacterAction(a5411, 0, 11, 0);
		g_vm->_inventoryObj[iLASER35]._examine = 1599;
	} else if ((g_vm->_oldRoom == r41D) && (g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		setPosition(30);
		drawCharacter(CALCPOINTS);
	}

	// FullMotion
	if ((g_vm->_curRoom == r17) && (g_vm->_oldRoom == r16)) {
		PlayDialog(dF171);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r16) && (g_vm->_oldRoom == r17)) {
		PlayDialog(dF161);
		WaitSoundFadEnd();
		g_vm->removeIcon(iFOGLIO14);
	} else if ((g_vm->_curRoom == r21) && (g_vm->_oldRoom == r1C)) {
		PlayDialog(dF1C1);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r23B) && (g_vm->_oldRoom == r23A)) {
		PlayDialog(dF231);
		WaitSoundFadEnd();
		g_vm->removeIcon(iMAPPA17);
	} else if ((g_vm->_curRoom == r29L) && (g_vm->_oldRoom == r29)) {
		PlayDialog(dF291);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r2GV) && (g_vm->_oldRoom == r2G)) {
		PlayDialog(dF2G2);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r31) && (g_vm->_oldRoom == r32) && (g_vm->_room[r32]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF321);
		g_vm->_flagShowCharacter = false;
		WaitSoundFadEnd();
		g_vm->_room[r32]._flag &= ~OBJFLAG_EXTRA;
	} else if ((g_vm->_curRoom == r19) && !(g_vm->_room[r19]._flag & OBJFLAG_DONE)) {
		g_vm->playScript(s19EVA);
		g_vm->_flagNoPaintScreen = false;
		g_vm->clearText();
		g_vm->redrawString();
		WaitSoundFadEnd();
	}
	// CHECKME: This check is identical to the 3rd one, thus it's always false
	else if ((g_vm->_curRoom == r21) && (/*!( _room[g_vm->_curRoom]._flag & OBJFLAG_DONE ) || */ (g_vm->_oldRoom == r1C))) {
		setPosition(10);
		TendIn();
		g_vm->_flagNoPaintScreen = false;
		g_vm->clearText();
		g_vm->redrawString();
	} else if ((g_vm->_curRoom == r46) && (g_vm->_oldRoom == r43) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF431);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r45S) && (g_vm->_oldRoom == r45) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF451);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r4A) && (g_vm->_oldRoom == r49) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF491);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r4A) && (g_vm->_oldRoom == r41D) && (g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA) && (g_vm->_obj[ocHELLEN4A]._mode & OBJMODE_OBJSTATUS)) {
		WaitSoundFadEnd();
		PlayDialog(dC4A1);
	} else if ((g_vm->_curRoom == r4P) && (g_vm->_oldRoom == r4O) && !(g_vm->_room[r4P]._flag & OBJFLAG_DONE)) {
		PlayDialog(dF4PI);
		g_vm->_flagShowCharacter = false;
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r51) && (g_vm->_oldRoom == r4CT)) {
		PlayDialog(dF4C1);
		g_vm->_flagShowCharacter = false;
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r1A) && (g_vm->_oldRoom == r18)) {
		if (!(g_vm->_room[r1A]._flag & OBJFLAG_DONE)) {
			PlayDialog(dF1A1);
			WaitSoundFadEnd();
			g_vm->_obj[oTOPO1C]._flag |= OBJFLAG_TAKE;
			g_vm->_obj[oTOPO1C]._anim = a1C3RACCOGLIETOPO;
		} else {
			TendIn();
			g_vm->_flagNoPaintScreen = false;
			g_vm->redrawString();
		}
	} else {
		TendIn();
		g_vm->_flagNoPaintScreen = false;
		g_vm->clearText();
		g_vm->redrawString();
	}

//	Sentence
	if ((g_vm->_curRoom == r17) && (g_vm->_oldRoom == r18) && !(g_vm->_room[r17]._flag & OBJFLAG_DONE) && (g_vm->_obj[oRETE17]._mode & OBJMODE_OBJSTATUS))
		CharacterSay(189);
	if (((g_vm->_curRoom == r12CU) || (g_vm->_curRoom == r13CU)) && g_vm->_closeUpObj && g_vm->_obj[g_vm->_closeUpObj]._examine)
		CharacterSay(g_vm->_obj[g_vm->_closeUpObj]._examine);
	else if ((g_vm->_curRoom == r23A) && (g_vm->_oldRoom == r21) && !(g_vm->_room[r23A]._flag & OBJFLAG_DONE)) {
		g_vm->_flagShowCharacter = true;
		StartCharacterAction(aWALKIN, 0, 0, 361);
	} else if ((g_vm->_curRoom == r24) && !(g_vm->_room[r24]._flag & OBJFLAG_DONE))
		CharacterSay(381);
	else if ((g_vm->_curRoom == r2G) && !(g_vm->_room[r2G]._flag & OBJFLAG_DONE))
		CharacterSay(688);
	else if ((g_vm->_curRoom == r4C) && (g_vm->_oldRoom == r4CT))
		CharacterSay(1163);
	else if ((g_vm->_curRoom == r41) && (g_vm->_oldRoom == r36) && !(g_vm->_room[r41]._flag & OBJFLAG_DONE))
		CharacterSay(900);
	else if ((g_vm->_curRoom == r58) && (g_vm->_obj[oGUARDIA58]._mode & OBJMODE_OBJSTATUS) && (g_vm->_obj[oGUARDIA58]._anim)) {
		g_vm->_curObj = oGUARDIA58;
		doEvent(MC_MOUSE, ME_MRIGHT, MP_DEFAULT, 372, 335 + TOP, 0, oGUARDIA58);
	} else if ((g_vm->_curRoom == r59L))
		CharacterSay(1394);
	else if ((g_vm->_curRoom == r58) && (g_vm->_oldRoom == r58T))
		CharacterSay(1368);
	else if ((g_vm->_curRoom == r5A) && !(g_vm->_room[r5A]._flag & OBJFLAG_DONE))
		CharacterSay(1408);
	else if ((g_vm->_curRoom == rSYS) && (g_vm->_oldRoom == rSYS))
		g_vm->_logicMgr->DoSys(o00LOAD);
	g_vm->_inventoryObj[iDISLOCATORE]._flag &= ~OBJFLAG_EXTRA;
}

/* -----------------26/11/97 10.38-------------------
 * 					GestioneATFrame
 * --------------------------------------------------*/
#define ATF_WAITTEXT 1

static struct ATFHandle {
	int16 curframe, lastframe;
	uint16 object;
	uint16 status;
	SAnim *curanim;
}            // 0->character 1->background 2->icon
AnimType[3] = {	{true}, {true}, {true}	};

/* -----------------11/07/97 11.43-------------------
					ExecuteATFDO
 --------------------------------------------------*/
void ExecuteATFDO(ATFHandle *h, int doit, int obj) {
	switch (doit) {
	case fCLROBJSTATUS:
		g_vm->_obj[obj]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case fSETOBJSTATUS:
		g_vm->_obj[obj]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case fONETIME:
		g_vm->_obj[obj]._anim = 0;
		break;
	case fCREPACCIO:
		if (g_vm->_room[r2E]._flag & OBJFLAG_EXTRA)
			g_vm->_obj[oCREPACCIO2E]._position = 7;
		else
			g_vm->_obj[oCREPACCIO2E]._position = 6;
		break;
	case fSERPVIA:
		doEvent(g_vm->_snake52._class, g_vm->_snake52._event, g_vm->_snake52._priority, g_vm->_snake52._u16Param1, g_vm->_snake52._u16Param2, g_vm->_snake52._u8Param, g_vm->_snake52._u32Param);
		break;
	case fPIRANHA:
		g_vm->_obj[oLUCCHETTO53]._anim = 0;
		g_vm->_obj[oGRATAC53]._anim = 0;
		g_vm->_obj[oGRATAA53]._anim = 0;
		g_vm->_obj[oLUCCHETTO53]._action = 1240;
		g_vm->_obj[oGRATAC53]._action = 1243;
		g_vm->_obj[oGRATAA53]._action = 1246;
		g_vm->_obj[oLAGO53]._examine = 1237;
		break;
	case fMOREAU:
		g_vm->_obj[oFINESTRAB58]._anim = 0;
		g_vm->_obj[oFINESTRAB58]._action = 1358;
		break;
	case fPORTA58:
		doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, 468, 180 + TOP, true, oPORTA58C55);
		break;
	case fHELLEN:
		doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, 336, 263 + TOP, true, 0);
		break;
	case fVALVEON34:
		if (!(g_vm->_choice[616]._flag & OBJFLAG_DONE) &&		// if the fmv is not done
		    (g_vm->_obj[oTUBOA34]._mode & OBJMODE_OBJSTATUS) && // if there's a cut pipe
		    !(g_vm->_obj[oTUBOFT34]._mode & OBJMODE_OBJSTATUS)) // if there's not tube outside
			g_vm->_animMgr->smkVolumePan(0, 2, 1);
		break;
	case fVALVEOFF34:
		g_vm->_animMgr->smkVolumePan(0, 2, 0);
		break;

	case fCHARACTEROFF:
		g_vm->_flagCharacterExists = false;
		break;
	case fCHARACTERON:
		g_vm->_flagCharacterExists = true;
		break;
	case fCHARACTERFOREGROUND:
		_forcedActorPos = FOREGROUND;
		break;
	case fCHARACTERBACKGROUND:
		_forcedActorPos = BACKGROUND;
		break;
	case fCHARACTERNORM:
		_forcedActorPos = 0;
		break;
	case fSETEXTRA:
		g_vm->_obj[obj]._flag |= OBJFLAG_EXTRA;
		break;
	case fCLREXTRA:
		g_vm->_obj[obj]._flag &= ~OBJFLAG_EXTRA;
		break;

	case fANIMOFF1:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF1;
		if ((g_vm->_curRoom == r11) || (g_vm->_curRoom == r1D) || (g_vm->_curRoom == r14) || (g_vm->_curRoom == r22) || (g_vm->_curRoom == r48) || (g_vm->_curRoom == r4P))
			g_vm->_animMgr->smkVolumePan(0, 1, 0);
		break;
	case fANIMOFF2:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF2;
		if ((g_vm->_curRoom == r2E))
			g_vm->_animMgr->smkVolumePan(0, 2, 0);
		break;
	case fANIMOFF3:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF3;
		break;
	case fANIMOFF4:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF4;
		if (g_vm->_curRoom == r28)
			g_vm->_animMgr->smkVolumePan(0, 1, 0);
		break;

	case fANIMON1:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF1;
		if ((g_vm->_curRoom == r14) || (g_vm->_curRoom == r1D) || (g_vm->_curRoom == r22) || (g_vm->_curRoom == r48) || (g_vm->_curRoom == r4P)) {
			g_vm->_animMgr->smkVolumePan(0, 1, 1);
		}
		break;
	case fANIMON2:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF2;
		if ((g_vm->_curRoom == r2E)) {
			g_vm->_animMgr->smkVolumePan(0, 2, 1);
		}
		break;
	case fANIMON3:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF3;
		break;
	case fANIMON4:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF4;
		break;
	case fENDDEMO:
		DemoOver();
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;
	case fSTOP2TXT:
		h->status |= ATF_WAITTEXT;
		// Sets a flag that is always cleared when you finish speaking
		// if the flag is cleared the anim no longer plays
		// (to be done in the smacker player)
		// also the counters in AtFrameNext stops
		break;
	}
}

/* -----------------11/07/97 11.42-------------------
					ProcessATF
 --------------------------------------------------*/
void ProcessATF(ATFHandle *h, int type, int atf) {
	static int dc = 0;

	switch (type) {
	case ATFTEXT:
		CharacterTalkInAction(h->curanim->_atFrame[atf]._index);
		break;
	case ATFTEXTACT:
		CharacterTalkInAction(g_vm->_obj[h->object]._action);
		break;
	case ATFTEXTEX:
		CharacterTalkInAction(g_vm->_obj[h->object]._examine);
		break;
	case ATFCLR:
		g_vm->_obj[h->curanim->_atFrame[atf]._index]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFCLRI:
		g_vm->removeIcon(h->curanim->_atFrame[atf]._index);
		break;
	case ATFCEX:
		g_vm->_obj[h->object]._examine = h->curanim->_atFrame[atf]._index;
		break;
	case ATFCACT:
		g_vm->_obj[h->object]._action = h->curanim->_atFrame[atf]._index;
		break;
	case ATFSET:
		g_vm->_obj[h->curanim->_atFrame[atf]._index]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFSETI:
		g_vm->addIcon(h->curanim->_atFrame[atf]._index);
		break;
	case ATFDO:
		ExecuteATFDO(h, h->curanim->_atFrame[atf]._index, h->object);
		break;
	case ATFROOM:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, h->curanim->_atFrame[atf]._index, 0, 0, h->object);
		break;
	case ATFSETPOS:
		setPosition(h->curanim->_atFrame[atf]._index);
		break;
	case ATFDIALOG:
		PlayDialog(h->curanim->_atFrame[atf]._index);
		break;
	case ATFCOBJANIM:
		g_vm->_obj[h->object]._anim = h->curanim->_atFrame[atf]._index;
		RegenRoom();
		break;
	case ATFCOBJBOX:
		g_vm->_obj[h->object]._nbox = h->curanim->_atFrame[atf]._index;
		RegenRoom();
		break;
	case ATFCOBJPOS:
		g_vm->_obj[h->object]._position = h->curanim->_atFrame[atf]._index;
		RegenRoom();
		break;
	case ATFSETFORE:
		g_vm->_obj[h->curanim->_atFrame[atf]._index]._nbox = FOREGROUND;
		RegenRoom();
		break;
	case ATFSETBACK:
		g_vm->_obj[h->curanim->_atFrame[atf]._index]._nbox = BACKGROUND;
		RegenRoom();
		break;
	case ATFSWITCH:
		g_vm->_obj[h->curanim->_atFrame[atf]._index]._mode ^= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFSETROOMT:
		g_vm->setRoom(h->curanim->_atFrame[atf]._index, true);
		break;
	case ATFSETROOMF:
		g_vm->setRoom(h->curanim->_atFrame[atf]._index, false);
		break;
	case ATFREADBOX:
		if (h->curanim->_atFrame[atf]._index == 1) {
			Common::String filename = Common::String::format("%s.3d", g_vm->_room[g_vm->_curRoom]._baseName);
			read3D(filename);
			g_vm->_room[g_vm->_curRoom]._flag &= ~OBJFLAG_EXTRA;
		} else if (h->curanim->_atFrame[atf]._index == 2) {
			Common::String filename = Common::String::format("%s2.3d", g_vm->_room[g_vm->_curRoom]._baseName);
			read3D(filename);
			g_vm->_room[g_vm->_curRoom]._flag |= OBJFLAG_EXTRA;
			if (g_vm->_curRoom == r37) {
				g_vm->_animMgr->smkVolumePan(0, 1, 1);
			}
		}
		break;
	case ATFONESPEAK:
		switch (h->curanim->_atFrame[atf]._index) {
		case 1:
			if (g_vm->_room[r1D]._flag & OBJFLAG_EXTRA)
				break;

			SomeOneTalk(307 + dc, oDONNA1D, 0, 0);
			if (dc < 6)
				dc ++;
			break;

		case 2:
			SomeOneTalk(1788, ocNEGOZIANTE1A, 0, 0);
			break;
		}
		break;
	case ATFEND:
		DemoOver();
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;

	}

}

/* -----------------11/07/97 11.42-------------------
					InitAtFrameHandler
 --------------------------------------------------*/
void InitAtFrameHandler(uint16 an, uint16 obj) {
	SAnim *anim = &g_vm->_animMgr->_animTab[an];

	ATFHandle *handle = &AnimType[0];
	if (anim->_flag & SMKANIM_BKG)
		handle = &AnimType[1];
	if (anim->_flag & SMKANIM_ICON)
		handle = &AnimType[2];

	handle->curanim = anim;
	if (obj)
		handle->object = obj;
	else
		handle->object = g_vm->_curObj;
	handle->curframe = 0;
	handle->lastframe = -1;
	handle->status = 0;
}

/* -----------------11/07/97 11.41-------------------
					AtFrameNext
 --------------------------------------------------*/
void AtFrameNext() {
	if (!((AnimType[0].status & ATF_WAITTEXT) && g_vm->_flagCharacterSpeak))
		AnimType[0].curframe++;
	if (!((AnimType[1].status & ATF_WAITTEXT) && g_vm->_flagCharacterSpeak))
		AnimType[1].curframe++;
	if (!((AnimType[2].status & ATF_WAITTEXT) && g_vm->_flagCharacterSpeak))
		AnimType[2].curframe++;
}

/* -----------------11/07/97 11.42-------------------
					AtFrameEnd
 --------------------------------------------------*/
void AtFrameEnd(int type) {
	ATFHandle *h = &AnimType[type];
	SAnim *anim = h->curanim;
	h->curframe = 0;

	// if this ATFrame has already been handled
	if (h->curframe == h->lastframe)
		return;

	h->lastframe = h->curframe;

	for (int32 a = 0; a < MAXATFRAME; a++) {
		// if it's time to run this AtFrame
		if ((anim->_atFrame[a]._numFrame == 0) && (anim->_atFrame[a]._type)) {
			if ((anim->_atFrame[a]._child == 0) ||
			    ((anim->_atFrame[a]._child == 1) && !(g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF1)) ||
			    ((anim->_atFrame[a]._child == 2) && !(g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF2)) ||
			    ((anim->_atFrame[a]._child == 3) && !(g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF3)) ||
			    ((anim->_atFrame[a]._child == 4) && !(g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF4)))
				ProcessATF(h, anim->_atFrame[a]._type, a);
		}
	}

	h->curanim = nullptr;
}

/* -----------------11/07/97 11.44-------------------
					AtFrameHandler
 --------------------------------------------------*/
void AtFrameHandler(int type) {
	ATFHandle *h = &AnimType[type];
	SAnim *anim = h->curanim;
	if (anim == nullptr)
		return;

	if (h->curframe == 0)
		h->curframe++;
	// if this ATFrame has already been applied
	if (h->curframe <= h->lastframe)
		return;

	for (int32 a = 0; a < MAXATFRAME; a++) {
		// if it's time to run this AtFrame
		if ((anim->_atFrame[a]._numFrame > h->lastframe) && (anim->_atFrame[a]._numFrame <= h->curframe) && (anim->_atFrame[a]._numFrame != 0)) {
			if ((anim->_atFrame[a]._child == 0) ||
			    ((anim->_atFrame[a]._child == 1) && !(g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF1)) ||
			    ((anim->_atFrame[a]._child == 2) && !(g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF2)) ||
			    ((anim->_atFrame[a]._child == 3) && !(g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF3)) ||
			    ((anim->_atFrame[a]._child == 4) && !(g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF4)))
				ProcessATF(h, anim->_atFrame[a]._type, a);
		}
	}

	// set lastframe
	h->lastframe = h->curframe;
}

} // End of namespace Trecision
