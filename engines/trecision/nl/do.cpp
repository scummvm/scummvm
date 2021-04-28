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
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/trecision.h"
#include "trecision/dialog.h"
#include "trecision/video.h"
#include "trecision/sound.h"

namespace Trecision {

/* -----------------26/11/97 10.38-------------------
 * 					GestioneATFrame
 * --------------------------------------------------*/
#define ATF_WAITTEXT 1

static struct ATFHandle {
	int16 _curFrame;
	int16 _lastFrame;
	uint16 _object;
	uint16 _status;
	SAnim *_curAnim;
}
AnimType[3] = {	{1}, {1}, {1}	};

void ExecuteAtFrameDoit(ATFHandle *h, int doit, int obj) {
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
		if (g_vm->_room[kRoom2E]._flag & kObjFlagExtra)
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
		g_vm->_obj[oWINDOWB58]._anim = 0;
		g_vm->_obj[oWINDOWB58]._action = 1358;
		break;
	case fDOOR58:
		doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, 468, 180 + TOP, true, oDOOR58C55);
		break;
	case fHELLEN:
		doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, 336, 263 + TOP, true, 0);
		break;
	case fVALVEON34:
		if (!(g_vm->_dialogMgr->_choice[616]._flag & kObjFlagDone) &&		// if the fmv is not done
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
		g_vm->_obj[obj]._flag |= kObjFlagExtra;
		break;
	case fCLREXTRA:
		g_vm->_obj[obj]._flag &= ~kObjFlagExtra;
		break;

	case fANIMOFF1:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF1;
		if ((g_vm->_curRoom == kRoom11) || (g_vm->_curRoom == kRoom1D) || (g_vm->_curRoom == kRoom14) || (g_vm->_curRoom == kRoom22) || (g_vm->_curRoom == kRoom48) || (g_vm->_curRoom == kRoom4P))
			g_vm->_animMgr->smkVolumePan(0, 1, 0);
		break;
	case fANIMOFF2:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF2;
		if ((g_vm->_curRoom == kRoom2E))
			g_vm->_animMgr->smkVolumePan(0, 2, 0);
		break;
	case fANIMOFF3:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF3;
		break;
	case fANIMOFF4:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF4;
		if (g_vm->_curRoom == kRoom28)
			g_vm->_animMgr->smkVolumePan(0, 1, 0);
		break;

	case fANIMON1:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF1;
		if ((g_vm->_curRoom == kRoom14) || (g_vm->_curRoom == kRoom1D) || (g_vm->_curRoom == kRoom22) || (g_vm->_curRoom == kRoom48) || (g_vm->_curRoom == kRoom4P)) {
			g_vm->_animMgr->smkVolumePan(0, 1, 1);
		}
		break;
	case fANIMON2:
		g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF2;
		if ((g_vm->_curRoom == kRoom2E)) {
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
		g_vm->demoOver();
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;
	case fSTOP2TXT:
		h->_status |= ATF_WAITTEXT;
		// Sets a flag that is always cleared when you finish speaking
		// if the flag is cleared the anim no longer plays
		// (to be done in the smacker player)
		// also the counters in AtFrameNext stops
		break;
	default:
		break;
	}
}

void ProcessAtFrame(ATFHandle *h, int type, int atf) {
	static int dc = 0;

	switch (type) {
	case ATFTEXT:
		CharacterTalkInAction(h->_curAnim->_atFrame[atf]._index);
		break;
	case ATFTEXTACT:
		CharacterTalkInAction(g_vm->_obj[h->_object]._action);
		break;
	case ATFTEXTEX:
		CharacterTalkInAction(g_vm->_obj[h->_object]._examine);
		break;
	case ATFCLR:
		g_vm->_obj[h->_curAnim->_atFrame[atf]._index]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFCLRI:
		g_vm->removeIcon(h->_curAnim->_atFrame[atf]._index);
		break;
	case ATFCEX:
		g_vm->_obj[h->_object]._examine = h->_curAnim->_atFrame[atf]._index;
		break;
	case ATFCACT:
		g_vm->_obj[h->_object]._action = h->_curAnim->_atFrame[atf]._index;
		break;
	case ATFSET:
		g_vm->_obj[h->_curAnim->_atFrame[atf]._index]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFSETI:
		g_vm->addIcon(h->_curAnim->_atFrame[atf]._index);
		break;
	case ATFDO:
		ExecuteAtFrameDoit(h, h->_curAnim->_atFrame[atf]._index, h->_object);
		break;
	case ATFROOM:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, h->_curAnim->_atFrame[atf]._index, 0, 0, h->_object);
		break;
	case ATFSETPOS:
		setPosition(h->_curAnim->_atFrame[atf]._index);
		break;
	case ATFDIALOG:
		g_vm->_dialogMgr->playDialog(h->_curAnim->_atFrame[atf]._index);
		break;
	case ATFCOBJANIM:
		g_vm->_obj[h->_object]._anim = h->_curAnim->_atFrame[atf]._index;
		RegenRoom();
		break;
	case ATFCOBJBOX:
		g_vm->_obj[h->_object]._nbox = h->_curAnim->_atFrame[atf]._index;
		RegenRoom();
		break;
	case ATFCOBJPOS:
		g_vm->_obj[h->_object]._position = h->_curAnim->_atFrame[atf]._index;
		RegenRoom();
		break;
	case ATFSETFORE:
		g_vm->_obj[h->_curAnim->_atFrame[atf]._index]._nbox = FOREGROUND;
		RegenRoom();
		break;
	case ATFSETBACK:
		g_vm->_obj[h->_curAnim->_atFrame[atf]._index]._nbox = BACKGROUND;
		RegenRoom();
		break;
	case ATFSWITCH:
		g_vm->_obj[h->_curAnim->_atFrame[atf]._index]._mode ^= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFSETROOMT:
		g_vm->setRoom(h->_curAnim->_atFrame[atf]._index, true);
		break;
	case ATFSETROOMF:
		g_vm->setRoom(h->_curAnim->_atFrame[atf]._index, false);
		break;
	case ATFREADBOX:
		switch (h->_curAnim->_atFrame[atf]._index) {
		case 1: {
			Common::String filename = Common::String::format("%s.3d", g_vm->_room[g_vm->_curRoom]._baseName);
			read3D(filename);
			g_vm->_room[g_vm->_curRoom]._flag &= ~kObjFlagExtra;
			}
			break;
		case 2: {
			Common::String filename = Common::String::format("%s2.3d", g_vm->_room[g_vm->_curRoom]._baseName);
			read3D(filename);
			g_vm->_room[g_vm->_curRoom]._flag |= kObjFlagExtra;
			if (g_vm->_curRoom == kRoom37)
				g_vm->_animMgr->smkVolumePan(0, 1, 1);
			}
			break;
		default:
			break;
		}
		break;
	case ATFONESPEAK:
		switch (h->_curAnim->_atFrame[atf]._index) {
		case 1:
			if (g_vm->_room[kRoom1D]._flag & kObjFlagExtra)
				break;

			SomeoneTalk(307 + dc, oDONNA1D, 0, false);
			if (dc < 6)
				dc ++;
			break;

		case 2:
			SomeoneTalk(1788, ocNEGOZIANTE1A, 0, false);
			break;
		default:
			break;
		}
		break;
	case ATFEND:
		g_vm->demoOver();
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;
	default:
		break;
	}

}

void InitAtFrameHandler(uint16 an, uint16 obj) {
	SAnim *anim = &g_vm->_animMgr->_animTab[an];

	ATFHandle *handle = &AnimType[kAnimTypeCharacter];
	if (anim->_flag & SMKANIM_BKG)
		handle = &AnimType[kAnimTypeBackground];
	if (anim->_flag & SMKANIM_ICON)
		handle = &AnimType[kAnimTypeIcon];

	handle->_curAnim = anim;
	handle->_object = obj ? obj : g_vm->_curObj;
	handle->_curFrame = 0;
	handle->_lastFrame = -1;
	handle->_status = 0;
}

void AtFrameNext() {
	for (int i = 0; i < 3; ++i) {
		if (!(AnimType[i]._status & ATF_WAITTEXT) || !g_vm->_flagCharacterSpeak)
			AnimType[i]._curFrame++;
	}
}

void AtFrameEnd(int type) {
	ATFHandle *h = &AnimType[type];
	SAnim *anim = h->_curAnim;
	h->_curFrame = 0;

	// if this ATFrame has already been handled
	if (h->_curFrame == h->_lastFrame)
		return;

	h->_lastFrame = h->_curFrame;

	uint16 flag = g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag;

	for (int32 a = 0; a < MAXATFRAME; a++) {
		// if it's time to run this AtFrame
		if (anim->_atFrame[a]._numFrame == 0 && anim->_atFrame[a]._type) {
			if (anim->_atFrame[a]._child == 0 ||
			    (anim->_atFrame[a]._child == 1 && !(flag & SMKANIM_OFF1)) ||
				(anim->_atFrame[a]._child == 2 && !(flag & SMKANIM_OFF2)) ||
			    (anim->_atFrame[a]._child == 3 && !(flag & SMKANIM_OFF3)) ||
			    (anim->_atFrame[a]._child == 4 && !(flag & SMKANIM_OFF4)))
				ProcessAtFrame(h, anim->_atFrame[a]._type, a);
		}
	}

	h->_curAnim = nullptr;
}

void AtFrameHandler(int type) {
	ATFHandle *h = &AnimType[type];
	SAnim *anim = h->_curAnim;
	if (anim == nullptr)
		return;

	if (h->_curFrame == 0)
		h->_curFrame++;
	// if this ATFrame has already been applied
	if (h->_curFrame <= h->_lastFrame)
		return;

	uint16 flag = g_vm->_animMgr->_animTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag;

	for (int32 a = 0; a < MAXATFRAME; a++) {
		// if it's time to run this AtFrame
		if ((anim->_atFrame[a]._numFrame > h->_lastFrame) && (anim->_atFrame[a]._numFrame <= h->_curFrame) && (anim->_atFrame[a]._numFrame != 0)) {
			if (anim->_atFrame[a]._child == 0 ||
			    (anim->_atFrame[a]._child == 1 && !(flag & SMKANIM_OFF1)) ||
			    (anim->_atFrame[a]._child == 2 && !(flag & SMKANIM_OFF2)) ||
			    (anim->_atFrame[a]._child == 3 && !(flag & SMKANIM_OFF3)) ||
			    (anim->_atFrame[a]._child == 4 && !(flag & SMKANIM_OFF4)))
				ProcessAtFrame(h, anim->_atFrame[a]._type, a);
		}
	}

	// set _lastFrame
	h->_lastFrame = h->_curFrame;
}

} // End of namespace Trecision
