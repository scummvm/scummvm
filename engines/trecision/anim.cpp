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

#include "trecision/anim.h"
#include "trecision/3d.h"
#include "trecision/dialog.h"
#include "trecision/scheduler.h"
#include "trecision/text.h"
#include "trecision/trecision.h"

namespace Trecision {

#define ATF_WAITTEXT 1

AnimTypeManager::AnimTypeManager(TrecisionEngine *vm) : _vm(vm) {
	for (int i = 0; i < 3; ++i) {
		AnimType[i]._curFrame = 1;
		AnimType[i]._lastFrame = 0;
		AnimType[i]._object = 0;
		AnimType[i]._status = 0;
		AnimType[i]._curAnim = nullptr;
	}
}

AnimTypeManager::~AnimTypeManager() {

}

void AnimTypeManager::executeAtFrameDoit(ATFHandle *h, int doit, int obj) {
	switch (doit) {
	case fCLROBJSTATUS:
		_vm->setObjectVisible(obj, false);
		break;
	case fSETOBJSTATUS:
		_vm->setObjectVisible(obj, true);
		break;
	case fONETIME:
		_vm->setObjectAnim(obj, 0);
		break;
	case fCREPACCIO:
		if (_vm->_room[kRoom2E]._flag & kObjFlagExtra)
			_vm->_obj[oCREPACCIO2E]._position = 7;
		else
			_vm->_obj[oCREPACCIO2E]._position = 6;
		break;
	case fSERPVIA:
		_vm->_scheduler->doEvent(_vm->_snake52._class, _vm->_snake52._event, _vm->_snake52._priority, _vm->_snake52._u16Param1, _vm->_snake52._u16Param2, _vm->_snake52._u8Param, _vm->_snake52._u32Param);
		break;
	case fPIRANHA:
		_vm->setObjectAnim(oLUCCHETTO53, 0);
		_vm->setObjectAnim(oGRATAC53, 0);
		_vm->setObjectAnim(oGRATAA53, 0);
		_vm->_obj[oLUCCHETTO53]._action = 1240;
		_vm->_obj[oGRATAC53]._action = 1243;
		_vm->_obj[oGRATAA53]._action = 1246;
		_vm->_obj[oLAGO53]._examine = 1237;
		break;
	case fMOREAU:
		_vm->setObjectAnim(oWINDOWB58, 0);
		_vm->_obj[oWINDOWB58]._action = 1358;
		break;
	case fDOOR58:
		_vm->_scheduler->doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, 468, 180 + TOP, true, oDOOR58C55);
		break;
	case fHELLEN:
		_vm->_scheduler->doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, 336, 263 + TOP, true, 0);
		break;
	case fVALVEON34:
		if (!(_vm->_dialogMgr->_choice[616]._flag & kObjFlagDone) && // if the fmv is not done
			(_vm->isObjectVisible(oTUBOA34)) &&                      // if there's a cut pipe
			!(_vm->isObjectVisible(oTUBOFT34)))                      // if there's not tube outside
			_vm->_animMgr->smkVolumePan(0, 2, 1);
		break;
	case fVALVEOFF34:
		_vm->_animMgr->smkVolumePan(0, 2, 0);
		break;

	case fCHARACTEROFF:
		_vm->_flagCharacterExists = false;
		break;
	case fCHARACTERON:
		_vm->_flagCharacterExists = true;
		break;
	case fCHARACTERFOREGROUND:
		_vm->_forcedActorPos = BOX_FOREGROUND;
		break;
	case fCHARACTERBACKGROUND:
		_vm->_forcedActorPos = BOX_BACKGROUND;
		break;
	case fCHARACTERNORM:
		_vm->_forcedActorPos = BOX_NORMAL;
		break;
	case fSETEXTRA:
		_vm->_obj[obj]._flag |= kObjFlagExtra;
		break;
	case fCLREXTRA:
		_vm->_obj[obj]._flag &= ~kObjFlagExtra;
		break;

	case fANIMOFF1:
		_vm->_animMgr->_animTab[_vm->_room[_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF1;
		if ((_vm->_curRoom == kRoom11) || (_vm->_curRoom == kRoom1D) || (_vm->_curRoom == kRoom14) || (_vm->_curRoom == kRoom22) || (_vm->_curRoom == kRoom48) || (_vm->_curRoom == kRoom4P))
			_vm->_animMgr->smkVolumePan(0, 1, 0);
		break;
	case fANIMOFF2:
		_vm->_animMgr->_animTab[_vm->_room[_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF2;
		if ((_vm->_curRoom == kRoom2E))
			_vm->_animMgr->smkVolumePan(0, 2, 0);
		break;
	case fANIMOFF3:
		_vm->_animMgr->_animTab[_vm->_room[_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF3;
		break;
	case fANIMOFF4:
		_vm->_animMgr->_animTab[_vm->_room[_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF4;
		if (_vm->_curRoom == kRoom28)
			_vm->_animMgr->smkVolumePan(0, 1, 0);
		break;

	case fANIMON1:
		_vm->_animMgr->_animTab[_vm->_room[_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF1;
		if ((_vm->_curRoom == kRoom14) || (_vm->_curRoom == kRoom1D) || (_vm->_curRoom == kRoom22) || (_vm->_curRoom == kRoom48) || (_vm->_curRoom == kRoom4P)) {
			_vm->_animMgr->smkVolumePan(0, 1, 1);
		}
		break;
	case fANIMON2:
		_vm->_animMgr->_animTab[_vm->_room[_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF2;
		if ((_vm->_curRoom == kRoom2E)) {
			_vm->_animMgr->smkVolumePan(0, 2, 1);
		}
		break;
	case fANIMON3:
		_vm->_animMgr->_animTab[_vm->_room[_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF3;
		break;
	case fANIMON4:
		_vm->_animMgr->_animTab[_vm->_room[_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF4;
		break;
	case fENDDEMO:
		_vm->demoOver();
		_vm->_scheduler->doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;
	case fSTOP2TXT:
		h->_status |= ATF_WAITTEXT;
		// Sets a flag that is always cleared when you finish speaking
		// if the flag is cleared the anim no longer plays
		// (to be done in the smacker player)
		// also the counters in next() stops
		break;
	default:
		break;
	}
}

void AnimTypeManager::processAtFrame(ATFHandle *h, int type, int atf) {
	static int dc = 0;

	switch (type) {
	case ATFTEXT:
		_vm->_textMgr->characterSayInAction(h->_curAnim->_atFrame[atf]._index);
		break;
	case ATFTEXTACT:
		_vm->_textMgr->characterSayInAction(_vm->_obj[h->_object]._action);
		break;
	case ATFTEXTEX:
		_vm->_textMgr->characterSayInAction(_vm->_obj[h->_object]._examine);
		break;
	case ATFCLR:
		_vm->setObjectVisible(h->_curAnim->_atFrame[atf]._index, false);
		break;
	case ATFCLRI:
		_vm->removeIcon(h->_curAnim->_atFrame[atf]._index);
		break;
	case ATFCEX:
		_vm->_obj[h->_object]._examine = h->_curAnim->_atFrame[atf]._index;
		break;
	case ATFCACT:
		_vm->_obj[h->_object]._action = h->_curAnim->_atFrame[atf]._index;
		break;
	case ATFSET:
		_vm->setObjectVisible(h->_curAnim->_atFrame[atf]._index, true);
		break;
	case ATFSETI:
		_vm->addIcon(h->_curAnim->_atFrame[atf]._index);
		break;
	case ATFDO:
		executeAtFrameDoit(h, h->_curAnim->_atFrame[atf]._index, h->_object);
		break;
	case ATFROOM:
		_vm->_scheduler->doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, h->_curAnim->_atFrame[atf]._index, 0, 0, h->_object);
		break;
	case ATFSETPOS:
		_vm->_pathFind->setPosition(h->_curAnim->_atFrame[atf]._index);
		break;
	case ATFDIALOG:
		_vm->_dialogMgr->playDialog(h->_curAnim->_atFrame[atf]._index);
		break;
	case ATFCOBJANIM:
		_vm->_obj[h->_object]._anim = h->_curAnim->_atFrame[atf]._index;
		break;
	case ATFCOBJBOX:
		_vm->_obj[h->_object]._nbox = h->_curAnim->_atFrame[atf]._index;
		break;
	case ATFCOBJPOS:
		_vm->_obj[h->_object]._position = h->_curAnim->_atFrame[atf]._index;
		break;
	case ATFSETFORE:
		_vm->_obj[h->_curAnim->_atFrame[atf]._index]._nbox = BOX_FOREGROUND;
		break;
	case ATFSETBACK:
		_vm->_obj[h->_curAnim->_atFrame[atf]._index]._nbox = BOX_BACKGROUND;
		break;
	case ATFSWITCH:
		_vm->setObjectVisible(h->_curAnim->_atFrame[atf]._index, !_vm->isObjectVisible(h->_curAnim->_atFrame[atf]._index));
		break;
	case ATFSETROOMT:
		_vm->setRoom(h->_curAnim->_atFrame[atf]._index, true);
		break;
	case ATFSETROOMF:
		_vm->setRoom(h->_curAnim->_atFrame[atf]._index, false);
		break;
	case ATFREADBOX:
		switch (h->_curAnim->_atFrame[atf]._index) {
		case 1: {
			Common::String filename = Common::String::format("%s.3d", _vm->_room[_vm->_curRoom]._baseName);
			_vm->read3D(filename);
			_vm->_room[_vm->_curRoom]._flag &= ~kObjFlagExtra;
			}
			break;
		case 2: {
			Common::String filename = Common::String::format("%s2.3d", _vm->_room[_vm->_curRoom]._baseName);
			_vm->read3D(filename);
			_vm->_room[_vm->_curRoom]._flag |= kObjFlagExtra;
			if (_vm->_curRoom == kRoom37)
				_vm->_animMgr->smkVolumePan(0, 1, 1);
			} break;
		default:
			break;
		}
		break;
	case ATFONESPEAK:
		switch (h->_curAnim->_atFrame[atf]._index) {
		case 1:
			if (_vm->_room[kRoom1D]._flag & kObjFlagExtra)
				break;

			_vm->_textMgr->someoneSay(307 + dc, oDONNA1D, 0);
			if (dc < 6)
				dc++;
			break;

		case 2:
			_vm->_textMgr->someoneSay(1788, ocNEGOZIANTE1A, 0);
			break;
		default:
			break;
		}
		break;
	case ATFEND:
		_vm->demoOver();
		_vm->_scheduler->doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void AnimTypeManager::init(uint16 an, uint16 obj) {
	SAnim *anim = &_vm->_animMgr->_animTab[an];
	ATFHandle *handle = &AnimType[kAnimTypeCharacter];
	
	if (anim->_flag & SMKANIM_BKG)
		handle = &AnimType[kAnimTypeBackground];
	if (anim->_flag & SMKANIM_ICON)
		handle = &AnimType[kAnimTypeIcon];

	handle->_curAnim = anim;
	handle->_object = obj ? obj : _vm->_curObj;
	handle->_curFrame = 0;
	handle->_lastFrame = -1;
	handle->_status = 0;
}

void AnimTypeManager::next() {
	for (int i = 0; i < 3; ++i) {
		if (!(AnimType[i]._status & ATF_WAITTEXT) || !_vm->_flagCharacterSpeak)
			AnimType[i]._curFrame++;
	}
}

void AnimTypeManager::end(int type) {
	ATFHandle *h = &AnimType[type];
	SAnim *anim = h->_curAnim;
	h->_curFrame = 0;

	// if this ATFrame has already been handled
	if (h->_curFrame == h->_lastFrame)
		return;

	h->_lastFrame = h->_curFrame;

	uint16 flag = _vm->_animMgr->_animTab[_vm->_room[_vm->_curRoom]._bkgAnim]._flag;

	for (int32 a = 0; a < MAXATFRAME; a++) {
		// if it's time to run this AtFrame
		if (anim->_atFrame[a]._numFrame == 0 && anim->_atFrame[a]._type) {
			const uint8 child = anim->_atFrame[a]._child;
			if (child == 0 || (child == 1 && !(flag & SMKANIM_OFF1))
			|| (child == 2 && !(flag & SMKANIM_OFF2)) || (child == 3 && !(flag & SMKANIM_OFF3))
			|| (child == 4 && !(flag & SMKANIM_OFF4)))
				processAtFrame(h, anim->_atFrame[a]._type, a);
		}
	}

	h->_curAnim = nullptr;
}

void AnimTypeManager::handler(int type) {
	ATFHandle *h = &AnimType[type];
	SAnim *anim = h->_curAnim;
	if (anim == nullptr)
		return;

	if (h->_curFrame == 0)
		h->_curFrame++;
	// if this ATFrame has already been applied
	if (h->_curFrame <= h->_lastFrame)
		return;

	uint16 flag = _vm->_animMgr->_animTab[_vm->_room[_vm->_curRoom]._bkgAnim]._flag;

	for (int32 a = 0; a < MAXATFRAME; a++) {
		// if it's time to run this AtFrame
		if ((anim->_atFrame[a]._numFrame > h->_lastFrame) && (anim->_atFrame[a]._numFrame <= h->_curFrame) && (anim->_atFrame[a]._numFrame != 0)) {
			if (anim->_atFrame[a]._child == 0 ||
				(anim->_atFrame[a]._child == 1 && !(flag & SMKANIM_OFF1)) ||
				(anim->_atFrame[a]._child == 2 && !(flag & SMKANIM_OFF2)) ||
				(anim->_atFrame[a]._child == 3 && !(flag & SMKANIM_OFF3)) ||
				(anim->_atFrame[a]._child == 4 && !(flag & SMKANIM_OFF4)))
				processAtFrame(h, anim->_atFrame[a]._type, a);
		}
	}

	// set _lastFrame
	h->_lastFrame = h->_curFrame;
}

} // End of namespace Trecision
