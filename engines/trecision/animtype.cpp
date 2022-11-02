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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "trecision/animmanager.h"
#include "trecision/animtype.h"
#include "trecision/dialog.h"
#include "trecision/logic.h"
#include "trecision/pathfinding3d.h"
#include "trecision/scheduler.h"
#include "trecision/text.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {

#define ATF_WAITTEXT 1

AnimTypeManager::AnimTypeManager(TrecisionEngine *vm) : _vm(vm) {
	for (int i = 0; i < 3; ++i) {
		_animType[i]._curFrame = 1;
		_animType[i]._lastFrame = 0;
		_animType[i]._object = 0;
		_animType[i]._status = 0;
		_animType[i]._curAnim = nullptr;
	}

	_oneSpeakDialogCount = 0;
}

AnimTypeManager::~AnimTypeManager() {

}

void AnimTypeManager::executeAtFrameDoit(ATFHandle *h, int doit, uint16 objectId) {
	SAnim *anim = &_vm->_animMgr->_animTab[_vm->_room[_vm->_curRoom]._bkgAnim];

	switch (doit) {
	case fCLROBJSTATUS:
		_vm->setObjectVisible(objectId, false);
		break;
	case fSETOBJSTATUS:
		_vm->setObjectVisible(objectId, true);
		break;
	case fONETIME:
		_vm->setObjectAnim(objectId, 0);
		break;
	case fCREPACCIO:
		if (_vm->_room[kRoom2E].hasExtra())
			_vm->_obj[oCRACK2E]._position = 7;
		else
			_vm->_obj[oCRACK2E]._position = 6;
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
		_vm->_scheduler->leftClick(468, 180 + TOP);
		break;
	case fHELLEN:
		_vm->_scheduler->leftClick(336, 263 + TOP);
		break;
	case fVALVEON34:
		if (!(_vm->_dialogMgr->isDialogFinished(616)) && // if the fmv is not done
			(_vm->isObjectVisible(oTUBOA34)) &&                      // if there's a cut pipe
			!(_vm->isObjectVisible(oTUBOFT34)))                      // if there's not tube outside
			_vm->_animMgr->smkToggleTrackAudio(0, 2, true);
		break;
	case fVALVEOFF34:
		_vm->_animMgr->smkToggleTrackAudio(0, 2, false);
		break;

	case fCHARACTEROFF:
		_vm->_flagShowCharacter = false;
		break;
	case fCHARACTERON:
		_vm->_flagShowCharacter = true;
		break;
	case fCHARACTERFOREGROUND:
		_vm->_pathFind->setForcedActorPos(BOX_FOREGROUND);
		break;
	case fCHARACTERBACKGROUND:
		_vm->_pathFind->setForcedActorPos(BOX_BACKGROUND);
		break;
	case fCHARACTERNORM:
		_vm->_pathFind->setForcedActorPos(BOX_NORMAL);
		break;
	case fSETEXTRA:
		_vm->_obj[objectId].setFlagExtra(true);
		break;
	case fCLREXTRA:
		_vm->_obj[objectId].setFlagExtra(false);
		break;

	case fANIMOFF1:
		anim->toggleAnimArea(1, false);
		if (_vm->_curRoom == kRoom11 ||
			_vm->_curRoom == kRoom1D ||
			_vm->_curRoom == kRoom14 ||
			_vm->_curRoom == kRoom22 ||
			_vm->_curRoom == kRoom48 ||
			_vm->_curRoom == kRoom4P)
			_vm->_animMgr->smkToggleTrackAudio(0, 1, false);
		break;
	case fANIMOFF2:
		anim->toggleAnimArea(2, false);
		if (_vm->_curRoom == kRoom2E)
			_vm->_animMgr->smkToggleTrackAudio(0, 2, false);
		break;
	case fANIMOFF3:
		anim->toggleAnimArea(3, false);
		break;
	case fANIMOFF4:
		anim->toggleAnimArea(4, false);
		if (_vm->_curRoom == kRoom28)
			_vm->_animMgr->smkToggleTrackAudio(0, 1, false);
		break;

	case fANIMON1:
		anim->toggleAnimArea(1, true);
		if (_vm->_curRoom == kRoom14 || _vm->_curRoom == kRoom1D || _vm->_curRoom == kRoom22 || _vm->_curRoom == kRoom48 || _vm->_curRoom == kRoom4P) {
			_vm->_animMgr->smkToggleTrackAudio(0, 1, true);
		}
		break;
	case fANIMON2:
		anim->toggleAnimArea(2, true);
		if (_vm->_curRoom == kRoom2E)
			_vm->_animMgr->smkToggleTrackAudio(0, 2, true);
		break;
	case fANIMON3:
		anim->toggleAnimArea(3, true);
		break;
	case fANIMON4:
		anim->toggleAnimArea(4, true);
		break;
	case fENDDEMO:
		_vm->demoOver();
		_vm->quitGame();
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
	const uint16 index = h->_curAnim->_atFrame[atf]._index;

	switch (type) {
	case ATFTEXT:
		_vm->_textMgr->characterSayInAction(index);
		break;
	case ATFTEXTACT:
		_vm->_textMgr->characterSayInAction(_vm->_obj[h->_object]._action);
		break;
	case ATFTEXTEX:
		_vm->_textMgr->characterSayInAction(_vm->_obj[h->_object]._examine);
		break;
	case ATFCLR:
		_vm->setObjectVisible(index, false);
		break;
	case ATFCLRI:
		_vm->removeIcon(index);
		break;
	case ATFCEX:
		_vm->_obj[h->_object]._examine = index;
		break;
	case ATFCACT:
		_vm->_obj[h->_object]._action = index;
		break;
	case ATFSET:
		_vm->setObjectVisible(index, true);
		break;
	case ATFSETI:
		_vm->addIcon(index);
		break;
	case ATFDO:
		executeAtFrameDoit(h, index, h->_object);
		break;
	case ATFROOM:
		_vm->changeRoom(index);
		break;
	case ATFSETPOS:
		_vm->_pathFind->setPosition(index);
		break;
	case ATFDIALOG:
		_vm->_dialogMgr->playDialog(index);
		break;
	case ATFCOBJANIM:
		_vm->_obj[h->_object]._anim = index;
		break;
	case ATFCOBJBOX:
		_vm->_obj[h->_object]._nbox = index;
		break;
	case ATFCOBJPOS:
		_vm->_obj[h->_object]._position = index;
		break;
	case ATFSETFORE:
		_vm->_obj[index]._nbox = BOX_FOREGROUND;
		break;
	case ATFSETBACK:
		_vm->_obj[index]._nbox = BOX_BACKGROUND;
		break;
	case ATFSWITCH:
		_vm->setObjectVisible(index, !_vm->isObjectVisible(index));
		break;
	case ATFSETROOMT:
		_vm->_logicMgr->setupAltRoom(index, true);
		break;
	case ATFSETROOMF:
		_vm->_logicMgr->setupAltRoom(index, false);
		break;
	case ATFREADBOX:
		switch (index) {
		case 1: {
			const Common::String filename = Common::String::format("%s.3d", _vm->_room[_vm->_curRoom]._baseName);
			_vm->read3D(filename);
			_vm->_room[_vm->_curRoom].setExtra(false);
			}
			break;
		case 2: {
			const Common::String filename = Common::String::format("%s2.3d", _vm->_room[_vm->_curRoom]._baseName);
			_vm->read3D(filename);
			_vm->_room[_vm->_curRoom].setExtra(true);
			if (_vm->_curRoom == kRoom37)
				_vm->_animMgr->smkToggleTrackAudio(0, 1, true);
			} break;
		default:
			break;
		}
		break;
	case ATFONESPEAK:
		switch (index) {
		case 1:	// Storekeeper's wife
			if (_vm->_room[kRoom1D].hasExtra())
				break;

			// Quotes spoken by the storekeeper's wife while she is in the cellar
			_vm->_textMgr->someoneSay(307 + _oneSpeakDialogCount, oDONNA1D);
			if (_oneSpeakDialogCount < 6)
				++_oneSpeakDialogCount;
			break;

		case 2:	// Storekeeper
			// Quote when you enter the liquor store: "Ah, it's you again... look round
			// if you want, but don't disturb me, I've got a lot to do"
			_vm->_textMgr->someoneSay(1788, ocNEGOZIANTE1A);
			break;
		default:
			break;
		}
		break;
	case ATFEND:
		_vm->demoOver();
		_vm->quitGame();
		break;
	default:
		break;
	}
}

void AnimTypeManager::init(uint16 an, uint16 obj) {
	SAnim *anim = &_vm->_animMgr->_animTab[an];
	ATFHandle *handle = &_animType[kAnimTypeCharacter];

	if (anim->_flag & SMKANIM_BKG)
		handle = &_animType[kAnimTypeBackground];
	if (anim->_flag & SMKANIM_ICON)
		handle = &_animType[kAnimTypeIcon];

	handle->_curAnim = anim;
	handle->_object = obj ? obj : _vm->_curObj;
	handle->_curFrame = 0;
	handle->_lastFrame = -1;
	handle->_status = 0;
}

void AnimTypeManager::next() {
	for (int i = 0; i < 3; ++i) {
		if (!(_animType[i]._status & ATF_WAITTEXT) || !_vm->_flagCharacterSpeak)
			++_animType[i]._curFrame;
	}
}

void AnimTypeManager::end(int type) {
	ATFHandle *h = &_animType[type];
	SAnim *anim = h->_curAnim;
	h->_curFrame = 0;

	// if this ATFrame has already been handled
	if (h->_curFrame == h->_lastFrame)
		return;

	h->_lastFrame = h->_curFrame;

	for (int32 i = 0; i < MAXATFRAME; ++i) {
		// if it's time to run this AtFrame
		if (anim->_atFrame[i]._numFrame == 0 && anim->_atFrame[i]._type) {
			const uint8 area = anim->_atFrame[i]._area;
			if ( area == 0 ||
				(area == 1 && anim->isAnimAreaShown(1)) ||
				(area == 2 && anim->isAnimAreaShown(2)) ||
				(area == 3 && anim->isAnimAreaShown(3)) ||
				(area == 4 && anim->isAnimAreaShown(4)))
				processAtFrame(h, anim->_atFrame[i]._type, i);
		}
	}

	h->_curAnim = nullptr;
}

void AnimTypeManager::handler(int type) {
	ATFHandle *h = &_animType[type];
	SAnim *anim = h->_curAnim;
	if (anim == nullptr)
		return;

	if (h->_curFrame == 0)
		++h->_curFrame;
	// if this ATFrame has already been applied
	if (h->_curFrame <= h->_lastFrame)
		return;

	for (int32 i = 0; i < MAXATFRAME; ++i) {
		// if it's time to run this AtFrame
		if (anim->_atFrame[i]._numFrame > h->_lastFrame &&
			anim->_atFrame[i]._numFrame <= h->_curFrame &&
			anim->_atFrame[i]._numFrame != 0) {
			const uint8 child = anim->_atFrame[i]._area;
			if ( child == 0 ||
				(child == 1 && anim->isAnimAreaShown(1)) ||
				(child == 2 && anim->isAnimAreaShown(2)) ||
				(child == 3 && anim->isAnimAreaShown(3)) ||
				(child == 4 && anim->isAnimAreaShown(4)))
				processAtFrame(h, anim->_atFrame[i]._type, i);
		}
	}

	// set _lastFrame
	h->_lastFrame = h->_curFrame;
}

} // End of namespace Trecision
