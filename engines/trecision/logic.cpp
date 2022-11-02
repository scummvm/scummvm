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

#include "common/config-manager.h"

#include "trecision/actor.h"
#include "trecision/animmanager.h"
#include "trecision/defines.h"
#include "trecision/dialog.h"
#include "trecision/graphics.h"
#include "trecision/logic.h"
#include "trecision/pathfinding3d.h"
#include "trecision/renderer3d.h"
#include "trecision/struct.h"
#include "trecision/scheduler.h"
#include "trecision/sound.h"
#include "trecision/text.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {
LogicManager::LogicManager(TrecisionEngine *vm) : _vm(vm) {
	for (int i = 0; i < 7; ++i)
		_comb35[i] = 0;
	_count35 = 0;

	for (int i = 0; i < 4; ++i)
		_comb49[i] = 0;

	for (int i = 0; i < 6; ++i) {
		_comb58[i] = 0;
		_comb4CT[i] = 0;
	}
	_count58 = 0;

	_wheel = 0xFFFF;
	for (int i = 0; i < 3; ++i)
		_wheelPos[i] = 0;

	_slotMachine41Counter = 0;

	initInventory();
}
LogicManager::~LogicManager() {}

void LogicManager::syncGameStream(Common::Serializer &ser) {
	for (int i = 0; i < 7; ++i)
		ser.syncAsUint16LE(_comb35[i]);
	for (int i = 0; i < 4; ++i)
		ser.syncAsUint16LE(_comb49[i]);
	for (int i = 0; i < 6; ++i)
		ser.syncAsUint16LE(_comb4CT[i]);
	for (int i = 0; i < 6; ++i)
		ser.syncAsUint16LE(_comb58[i]);
	for (int i = 0; i < 3; ++i)
		ser.syncAsUint16LE(_wheelPos[i]);
	ser.syncAsUint16LE(_wheel);
	ser.syncAsUint16LE(_count35);
	ser.syncAsUint16LE(_count58);
	ser.syncAsUint16LE(_slotMachine41Counter);
}

void LogicManager::setupAltRoom(uint16 room, bool altRoomFl) {
	switch (room) {
	case kRoom21:
		if (!altRoomFl) {
			_vm->read3D("21.3d");
			_vm->_room[kRoom21].setExtra(false);
			_vm->_pathFind->setPosition(14);
			_vm->_obj[oCHAIN21]._position = 5;
			_vm->_obj[oEXIT21]._position = 11;

			// if we can go beyond
			if (((_vm->iconPos(kItemMagneticBar) != -1) && (_vm->_dialogMgr->isDialogFinished(436) || _vm->_dialogMgr->isDialogFinished(466)))
				|| (_vm->_dialogMgr->isDialogFinished(451) || _vm->_dialogMgr->isDialogFinished(481))) {
				_vm->_obj[od21TO23].setFlagRoomOut(true);
				_vm->_obj[od21TO23].setFlagExamine(false);
			} else {
				_vm->_obj[od21TO23].setFlagRoomOut(false);
				_vm->_obj[od21TO23].setFlagExamine(true);
			}

			_vm->setObjectAnim(od21TO23, 0);
			_vm->setObjectVisible(oEXIT21, true);

			_vm->_obj[od21TO22].setFlagRoomOut(true);
			_vm->_obj[od21TO22].setFlagExamine(false);
			_vm->setObjectAnim(od21TO22, aWALKOUT);
			_vm->setObjectAnim(oDOORA21, a212);
			_vm->setObjectAnim(oDOORC21, a219);

			_vm->setObjectVisible(oCUNICLE21, true);
			_vm->setObjectVisible(oBILLBOARD21, true);
		} else {
			_vm->read3D("212.3d");
			_vm->_room[kRoom21].setExtra(true);
			_vm->_pathFind->setPosition(15);
			_vm->_obj[oCHAIN21]._position = 6;
			_vm->_obj[oEXIT21]._position = 21;

			_vm->_obj[od21TO23].setFlagRoomOut(true);
			_vm->_obj[od21TO23].setFlagExamine(false);
			_vm->setObjectAnim(od21TO23, aWALKOUT);
			_vm->setObjectVisible(oEXIT21, true);

			// If we can go beyond
			if (((_vm->iconPos(kItemMagneticBar) != -1) && (_vm->_dialogMgr->isDialogFinished(436) || _vm->_dialogMgr->isDialogFinished(466)))
				|| (_vm->_dialogMgr->isDialogFinished(451) || _vm->_dialogMgr->isDialogFinished(481))) {
				_vm->_obj[od21TO22].setFlagRoomOut(true);
				_vm->_obj[od21TO22].setFlagExamine(false);
			} else {
				_vm->_obj[od21TO22].setFlagRoomOut(false);
				_vm->_obj[od21TO22].setFlagExamine(true);
			}
			_vm->setObjectAnim(od21TO22, 0);
			_vm->_obj[od21TO22]._examine = 335;
			_vm->_obj[od21TO22]._action = 335;
			_vm->setObjectAnim(oDOORA21, 0);
			_vm->setObjectAnim(oDOORC21, 0);

			_vm->setObjectVisible(oCUNICLE21, false);
			_vm->setObjectVisible(oBILLBOARD21, false);
		}
		break;
	case kRoom24:
		if (!altRoomFl) {
			_vm->read3D("24.3d");
			_vm->_room[kRoom24].setExtra(false);
			_vm->_obj[oPASSAGE24]._position = 3;
			_vm->_obj[oRUINS24]._position = 3;
			_vm->setObjectVisible(oDUMMY24, false);
			_vm->setObjectVisible(oDUMMY24A, true);
		} else {
			_vm->read3D("242.3d");
			_vm->_room[kRoom24].setExtra(true);
			_vm->setObjectVisible(od24TO26, true);
			_vm->_obj[oPASSAGE24]._position = 4;
			_vm->_obj[oRUINS24]._position = 4;
			_vm->setObjectVisible(oDUMMY24A, false);
			_vm->setObjectVisible(oDUMMY24, true);
		}
		break;

	case kRoom2A:
		if (!altRoomFl) {
			_vm->read3D("2A.3d");
			_vm->_room[kRoom2A].setExtra(false);
			_vm->setObjectVisible(oDUMMY2A2, true);
			_vm->setObjectVisible(oDUMMY2A, false);
		} else {
			_vm->read3D("2A2.3d");
			_vm->_room[kRoom2A].setExtra(true);
			_vm->setObjectVisible(oDUMMY2A, true);
			_vm->setObjectVisible(oDUMMY2A2, false);
		}
		break;
	case kRoom2B:
		if (!altRoomFl) {
			_vm->read3D("2B.3d");
			_vm->_room[kRoom2B].setExtra(false);
			_vm->setObjectVisible(oDOOR2B, true);
			_vm->setObjectVisible(od2BTO28, false);
		} else {
			_vm->read3D("2B2.3d");
			_vm->_room[kRoom2B].setExtra(true);
			_vm->setObjectVisible(oDOOR2B, false);
			_vm->setObjectVisible(od2BTO28, true);
		}
		break;
	case kRoom2E:
		if (!altRoomFl) {
			_vm->_obj[oCATWALKA2E]._nbox = BOX_BACKGROUND;
			_vm->_obj[oCATWALKA2E]._position = 2;
			_vm->setObjectAnim(oCATWALKA2E, a2E2PRIMAPALLONTANANDO);
			_vm->read3D("2E.3d");
			_vm->_room[kRoom2E].setExtra(false);
			_vm->setObjectVisible(oDUMMY2E, false);
			_vm->_obj[oENTRANCE2E].setFlagExamine(false);
			_vm->setObjectVisible(oCATWALKB2E, false);
			_vm->setObjectVisible(oCRATER2E, false);
			_vm->setObjectVisible(oSHRUBS2E, false);
			_vm->_obj[oCRACK2E]._position = 6;
		} else {
			_vm->_obj[oCATWALKA2E]._position = 3;
			_vm->setObjectAnim(oCATWALKA2E, a2E3FIRSTAPPROACH);
			_vm->read3D("2E2.3d");
			_vm->_room[kRoom2E].setExtra(true);
			_vm->setObjectVisible(oDUMMY2E, true);
			_vm->setObjectVisible(oENTRANCE2E, true);
			_vm->_obj[oENTRANCE2E].setFlagExamine(true);
			_vm->setObjectVisible(oCATWALKB2E, true);
			_vm->setObjectVisible(oCRATER2E, true);
			_vm->setObjectVisible(oSHRUBS2E, true);
			_vm->_obj[oCRACK2E]._position = 7;
			_vm->_obj[oCATWALKA2E]._nbox = BOX_FOREGROUND;
		}
		break;
	case kRoom2GV:
		if (!altRoomFl) {
			_vm->_obj[oVIADUCT2GV]._position = 7;
			_vm->setObjectAnim(oVIADUCT2GV, a2G7CROSSBRIDGE);
			_vm->read3D("2GV.3d");
			_vm->_room[kRoom2GV].setExtra(false);
			_vm->setObjectVisible(oDUMMY2GV, false);
			_vm->setObjectVisible(oGIRLS2GV, false);
			_vm->setObjectVisible(oCROCODILE2GV, false);
		}
		break;
	default:
		break;
	}
}

struct CharacterAnimation {
	uint16 _curRoom;
	uint16 _oldRoom;
	uint16 _action;
};

void LogicManager::startCharacterAnimations() {
	static const CharacterAnimation characterAnimations[] = {
		{ kRoom12,  kRoom13,  a1213ARRIVACONASCENSORE },
		{ kRoom12,  kRoom16,  a1213ARRIVACONASCENSORE },
		{ kRoom16,  kRoom12,  a1617SCENDECONASCENSORE },
		{ kRoom16,  kRoom13,  a1617SCENDECONASCENSORE },
		{ kRoom13,  kRoom12,  a1315ARRIVAASCENSOREALTOENTRA },
		{ kRoom13,  kRoom16,  a1314ARRIVAASCENSOREDABASSO },
		{ kRoom14,  kRoom13,  a1414ENTERROOM },
		{ kRoom18,  kRoom19,  a189ENTRADALCLUB },
		{ kRoom1A,  kRoom18,  a1A5ENTRA },
		{ kRoom1C,  kRoom1B,  a1C1ENTRACUNICOLO },
		{ kRoom1B,  kRoom1C,  a1B11ESCETOMBINO },
		{ kRoom1B,  kRoom1D,  a1B4ESCEBOTOLA },
		{ kRoom24,  kRoom2H,  a242 },
		{ kRoom25,  kRoom2A,  a257 },
		{ kRoom28,  kRoom27,  aWALKIN },
		{ kRoom29,  kRoom2A,  a298ESCEBOTOLA },
		{ kRoom29L, kRoom2A,  a2910ESCEBOTOLA },
		{ kRoom2A,  kRoom25,  aWALKIN },
		{ kRoom2A,  kRoom29,  aWALKIN },
		{ kRoom2A,  kRoom29L, aWALKIN },
		{ kRoom2H,  kRoom24,  a2H1ARRIVA },
		{ kRoom2E,  kRoom2F,  a2E5SECONDAPARRIVANDO },
		{ kRoom2F,  kRoom2E,  aWALKIN },
		{ kRoom23B, kRoom21,  aWALKIN },
		{ kRoom21,  kRoom23A, aWALKIN },
		{ kRoom21,  kRoom23B, aWALKIN },
		{ kRoom2F,  kRoom31,  a2F4ESCEASCENSORE },
		{ kRoom31,  kRoom2F,  a3114ESCEPASSAGGIO },
		{ kRoom32,  kRoom31,  a321SALEMONTACARICHI },
		{ kRoom32,  kRoom33,  a325SCENDESCALE },
		{ kRoom36,  kRoom35,  a361ESCEASCENSORE },
		{ kRoom35,  kRoom36,  a3515ESCEASCENSORE },
		{ kRoom44,  kRoom45,  a445 },
		{ kRoom44,  kRoom45S, a445 },
		{ kRoom45,  kRoom44,  a457 },
		{ kRoom45S, kRoom44,  a457 },
		{ kRoom46,  kRoom47,  aWALKIN },
		{ kRoom46,  kRoom48,  aWALKIN },
		{ kRoom46,  kRoom49,  aWALKIN },
		{ kRoom47,  kRoom46,  a476 },
		{ kRoom48,  kRoom46,  a485 },
		{ kRoom49,  kRoom46,  a494 },
		{       0,        0,  0 }
	};

	int i = 0;

	do {
		const CharacterAnimation anim = characterAnimations[i];
		const bool positionerRoom = _vm->_curRoom >= kRoom44 && _vm->_curRoom <= kRoom49;
		const bool checkPositioner = !positionerRoom || !(_vm->_inventoryObj[kItemPositioner].isFlagExtra());
		if (_vm->_curRoom == anim._curRoom && _vm->_oldRoom == anim._oldRoom && checkPositioner) {
			_vm->startCharacterAction(anim._action, 0, 0, 0);
			break;
		}
	
		++i;
	} while (characterAnimations[i]._curRoom != 0);

	if (_vm->_curRoom == kRoom18 && _vm->_oldRoom == kRoom17 && !_vm->_room[kRoom18].isDone()) {
		_vm->startCharacterAction(a186GUARDAPIAZZA, 0, 0, 0);
	} else if (_vm->_curRoom == kRoom1A && _vm->_oldRoom == kRoom18 && _vm->_room[kRoom1A].isDone()) {
		_vm->startCharacterAction(a1A5ENTRA, 0, 0, 0);
		_vm->_animMgr->_animTab[aBKG1A].toggleAnimArea(1, false);
	} else if (_vm->_curRoom == kRoom1D && _vm->_oldRoom == kRoom1B) {
		const uint16 cellarAction = _vm->isObjectVisible(oDONNA1D) ? a1D1SCENDESCALE : a1D12SCENDESCALA;
		_vm->startCharacterAction(cellarAction, 0, 1, 0);
		_vm->_actor->_area[0] = 60;
		_vm->_actor->_area[2] = 240;
	} else if (_vm->_curRoom == kRoom1B && _vm->_oldRoom == kRoom18 && _vm->_animMgr->_animTab[aBKG1B].isAnimAreaShown(1))
		_vm->startCharacterAction(a1B12SCAPPATOPO, 0, 0, 0);
	else if (_vm->_curRoom == kRoom2B && _vm->_oldRoom == kRoom2A)
		_vm->startCharacterAction(a2B2ESCEPOZZO, 0, 2, 0);
	else if (_vm->_curRoom == kRoom23A && _vm->_oldRoom == kRoom21) {
		if (_vm->_room[kRoom23A].isDone())
			_vm->startCharacterAction(aWALKIN, 0, 0, 0);
		else
			_vm->_flagShowCharacter = false;
	} else if (_vm->_curRoom == kRoom33 && _vm->_oldRoom == kRoom32) {
		const uint16 roofAction = _vm->isObjectVisible(oBRUCIATURA33) ? a3311SALESCALE : a3313CHIUDEBOTOLA;
		_vm->startCharacterAction(roofAction, 0, 0, 0);
	} else if (_vm->_curRoom == kRoom54 && (_vm->_oldRoom == kRoom53)) {
		_vm->startCharacterAction(a5411, 0, 11, 0);
		_vm->_inventoryObj[kItemLaserGun]._examine = 1599;
	}
}

bool LogicManager::startPlayDialog() {
	bool dialogHandled = true;

	if (_vm->_curRoom == kRoom17 && _vm->_oldRoom == kRoom16) {
		_vm->_dialogMgr->playDialog(dF171);
	} else if (_vm->_curRoom == kRoom16 && (_vm->_oldRoom == kRoom17)) {
		_vm->_dialogMgr->playDialog(dF161);
		_vm->removeIcon(kItemNoteRotmall17);
	} else if (_vm->_curRoom == kRoom21 && (_vm->_oldRoom == kRoom1C)) {
		_vm->_dialogMgr->playDialog(dF1C1);
	} else if (_vm->_curRoom == kRoom23B && (_vm->_oldRoom == kRoom23A)) {
		_vm->_dialogMgr->playDialog(dF231);
		_vm->removeIcon(kItemSubwayMap);
	} else if (_vm->_curRoom == kRoom29L && (_vm->_oldRoom == kRoom29)) {
		_vm->_dialogMgr->playDialog(dF291);
	} else if (_vm->_curRoom == kRoom2GV && (_vm->_oldRoom == kRoom2G)) {
		_vm->_dialogMgr->playDialog(dF2G2);
	} else if (_vm->_curRoom == kRoom31 && (_vm->_oldRoom == kRoom32) && _vm->_room[kRoom32].hasExtra()) {
		_vm->_dialogMgr->playDialog(dF321);
		_vm->_flagShowCharacter = false;
		_vm->_room[kRoom32].setExtra(false);
	} else if (_vm->_curRoom == kRoom46 && (_vm->_oldRoom == kRoom43) && !_vm->_inventoryObj[kItemPositioner].isFlagExtra()) {
		_vm->_dialogMgr->playDialog(dF431);
	} else if (_vm->_curRoom == kRoom45S && (_vm->_oldRoom == kRoom45) && !_vm->_inventoryObj[kItemPositioner].isFlagExtra()) {
		_vm->_dialogMgr->playDialog(dF451);
	} else if (_vm->_curRoom == kRoom4A && (_vm->_oldRoom == kRoom49) && !_vm->_inventoryObj[kItemPositioner].isFlagExtra()) {
		_vm->_dialogMgr->playDialog(dF491);
	} else if (_vm->_curRoom == kRoom4A && (_vm->_oldRoom == kRoom41D) && _vm->_inventoryObj[kItemPositioner].isFlagExtra() && _vm->isObjectVisible(ocHELLEN4A)) {
		_vm->_dialogMgr->playDialog(dC4A1);
	} else if (_vm->_curRoom == kRoom4P && (_vm->_oldRoom == kRoom4O) && !_vm->_room[kRoom4P].isDone()) {
		_vm->_dialogMgr->playDialog(dF4PI);
		_vm->_flagShowCharacter = false;
	} else if (_vm->_curRoom == kRoom51 && (_vm->_oldRoom == kRoom4CT)) {
		_vm->_dialogMgr->playDialog(dF4C1);
		_vm->_flagShowCharacter = false;
	} else if (_vm->_curRoom == kRoom1A && (_vm->_oldRoom == kRoom18)) {
		if (!_vm->_room[kRoom1A].isDone()) {
			_vm->_dialogMgr->playDialog(dF1A1);
			_vm->_obj[oTOPO1C].setFlagTake(true);
			_vm->setObjectAnim(oTOPO1C, a1C3RACCOGLIETOPO);
		} else {
			_vm->tendIn();
			_vm->_flagNoPaintScreen = false;
			_vm->_textMgr->redrawString();
		}
	} else if (_vm->_curRoom == kRoomIntro) {
		_vm->_dialogMgr->playDialog(dFLOG);
	} else {
		dialogHandled = false;
	}

	return dialogHandled;
}

void LogicManager::handleChangeRoomObjects() {
	switch (_vm->_curRoom) {
	case kRoom2E:
		if (_vm->isObjectVisible(oPULSANTEADS2D) || _vm->isObjectVisible(oPULSANTEADA2D)) {
			const bool flag = _vm->isObjectVisible(oPULSANTEADS2D);
			_vm->_animMgr->_animTab[aBKG2E].toggleAnimArea(1, !flag);
			_vm->setObjectVisible(oCAMPO2E, !flag);
			_vm->setObjectVisible(oCARTELLOA2E, !flag);
			_vm->setObjectVisible(oCARTELLOS2E, flag);
		}
		break;
	case kRoom2C:
		if (_vm->isObjectVisible(oPULSANTEACS2D) || _vm->isObjectVisible(oPULSANTEACA2D)) {
			const bool flag = _vm->isObjectVisible(oPULSANTEACS2D);
			_vm->_animMgr->_animTab[aBKG2C].toggleAnimArea(1, !flag);
			_vm->setObjectVisible(oCAMPO2C, !flag);
			_vm->setObjectVisible(oCARTELLOA2C, !flag);
			_vm->setObjectVisible(oCARTELLOS2C, flag);
		}
		break;
	case kRoom28:
		if (_vm->isObjectVisible(oPULSANTEBBS2D) || _vm->isObjectVisible(oPULSANTEBBA2D)) {
			const bool flag = _vm->isObjectVisible(oPULSANTEBBS2D);
			_vm->_animMgr->_animTab[aBKG28].toggleAnimArea(2, !flag);
			_vm->setObjectAnim(oCESPUGLIO28, flag ? a283 : a282);
			_vm->setObjectVisible(oSERPENTETM28, flag);
			_vm->setObjectVisible(oSERPENTEAM28, flag);
		}

		if (_vm->isObjectVisible(oPULSANTEABS2D) || _vm->isObjectVisible(oPULSANTEABA2D)) {
			const bool flag = _vm->isObjectVisible(oPULSANTEABS2D);
			_vm->_animMgr->_animTab[aBKG28].toggleAnimArea(1, !flag);
			_vm->_animMgr->_animTab[aBKG28].toggleAnimArea(3, !flag);
			_vm->setObjectVisible(oCAMPO28, !flag);
			_vm->setObjectVisible(oCARTELLOA28, !flag);
			_vm->setObjectVisible(oCARTELLOS28, flag);
			if (flag)
				_vm->read3D("282.3d");
		}
		break;
	}
}

void LogicManager::endChangeRoom() {
	handleChangeRoomObjects();

	// Actions
	startCharacterAnimations();

	if (isCloseupOrControlRoom()) { // Screens without inventory
		_vm->_flagShowCharacter = false;
		_vm->_flagInventoryLocked = true;
	} else if (_vm->_curRoom == kRoom31P || _vm->_curRoom == kRoom35P) { // Screens with inventory
		_vm->_flagShowCharacter = false;
	} else if (_vm->_curRoom == kRoom31 && !_vm->_room[kRoom31].isDone())
		_vm->_pathFind->setPosition(14);
	else if (_vm->_oldRoom == kRoom41D && _vm->_inventoryObj[kItemPositioner].isFlagExtra()) {
		_vm->_pathFind->setPosition(30);
		_vm->_renderer->drawCharacter(CALCPOINTS);
	}

	// FullMotion
	const bool dialogHandled = startPlayDialog();

	if (_vm->_curRoom == kRoom19 && !_vm->_room[kRoom19].isDone()) {
		_vm->playScript(s19EVA);
		_vm->_flagNoPaintScreen = false;
		_vm->_textMgr->clearLastText();
		_vm->_textMgr->redrawString();
	} else if (!dialogHandled) {
		_vm->tendIn();
		_vm->_flagNoPaintScreen = false;
		_vm->_textMgr->clearLastText();
		_vm->_textMgr->redrawString();
	}

	//	Sentence
	if (_vm->_curRoom == kRoom17 && (_vm->_oldRoom == kRoom18) && !_vm->_room[kRoom17].isDone() && _vm->isObjectVisible(oRETE17))
		_vm->_textMgr->characterSay(189);

	if (_vm->_curRoom == kRoom12CU) {
		const uint16 closeupObjectId = _vm->isObjectVisible(oFUSIBILE12) ? oFUSIBILE12 : oPANELA12;
		_vm->_textMgr->characterSay(_vm->_obj[closeupObjectId]._examine);
	} else if (_vm->_curRoom == kRoom13CU) {
		const uint16 closeupObjectId = _vm->isObjectVisible(oLETTERA13) ? oLETTERA13 : oPENPADA13;
		_vm->_textMgr->characterSay(_vm->_obj[closeupObjectId]._examine);
	} else if (_vm->_curRoom == kRoom23A && _vm->_oldRoom == kRoom21 && !_vm->_room[kRoom23A].isDone()) {
		_vm->_flagShowCharacter = true;
		_vm->startCharacterAction(aWALKIN, 0, 0, 361);
	} else if (_vm->_curRoom == kRoom24 && !_vm->_room[kRoom24].isDone())
		_vm->_textMgr->characterSay(381);
	else if (_vm->_curRoom == kRoom2G && !_vm->_room[kRoom2G].isDone())
		_vm->_textMgr->characterSay(688);
	else if (_vm->_curRoom == kRoom4C && (_vm->_oldRoom == kRoom4CT))
		_vm->_textMgr->characterSay(1163);
	else if (_vm->_curRoom == kRoom41 && (_vm->_oldRoom == kRoom36) && !_vm->_room[kRoom41].isDone())
		_vm->_textMgr->characterSay(900);
	else if (_vm->_curRoom == kRoom58 && _vm->isObjectVisible(oGUARDIA58) && (_vm->_obj[oGUARDIA58]._anim)) {
		_vm->_curObj = oGUARDIA58;
		_vm->_scheduler->rightClick(372, 335 + TOP);
	} else if (_vm->_curRoom == kRoom59L)
		_vm->_textMgr->characterSay(1394);
	else if (_vm->_curRoom == kRoom58 && (_vm->_oldRoom == kRoom58T))
		_vm->_textMgr->characterSay(1368);
	else if (_vm->_curRoom == kRoom5A && !_vm->_room[kRoom5A].isDone())
		_vm->_textMgr->characterSay(1408);
	else if (_vm->_curRoom == kRoomControlPanel && (_vm->_oldRoom == kRoomControlPanel))
		_vm->_logicMgr->handleClickControlPanel(o00LOAD);

	if (_vm->_curRoom == kRoom41D)
		_vm->readPositionerSnapshots();

	_vm->_inventoryObj[kItemPositioner].setFlagExtra(false);
}

void LogicManager::initInventory() {
	_vm->_inventory.clear();
	_vm->_inventory.push_back(kItemBankNote);
	_vm->_inventory.push_back(kItemMinicom);
	_vm->_inventory.push_back(kItemLiftCard);
	_vm->_inventory.push_back(kItemPen);
	_vm->_inventory.push_back(kItemLetterboxKey);
}

void LogicManager::useInventoryWithInventory() {
	bool itemUsed = false;

	if (!_vm->_useWith[USED] || !_vm->_useWith[WITH])
		warning("useInventoryWithInventory - _useWith not set properly");

	_vm->_animMgr->smkStop(kSmackerIcon);
	
	switch (_vm->_useWith[USED]) {
	case kItemFoilPaper:
		if (_vm->_useWith[WITH] == kItemBurnedFuse) {
			_vm->removeIcon(kItemFoilPaper);
			_vm->replaceIcon(kItemBurnedFuse, kItemRepairedFuse);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1441);
			itemUsed = true;
		}
		break;

	case kItemBurnedFuse:
		if (_vm->_useWith[WITH] == kItemFoilPaper) {
			_vm->removeIcon(kItemFoilPaper);
			_vm->replaceIcon(kItemBurnedFuse, kItemRepairedFuse);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1441);
			itemUsed = true;
		}
		break;

	case kItemDeadRat:
		if (_vm->_useWith[WITH] == kItemSkate) {
			_vm->removeIcon(kItemSkate);
			_vm->removeIcon(kItemDeadRat);
			_vm->addIcon(kItemRatOnSkate);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1497);
			itemUsed = true;
		}
		break;

	case kItemSkate:
		if (_vm->_useWith[WITH] == kItemDeadRat) {
			_vm->removeIcon(kItemSkate);
			_vm->removeIcon(kItemDeadRat);
			_vm->addIcon(kItemRatOnSkate);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1497);
			itemUsed = true;
		}
		break;

	case kItemBar:
		if (_vm->_useWith[WITH] == kItemMagnet) {
			_vm->removeIcon(kItemBar);
			_vm->replaceIcon(kItemMagnet, kItemMagneticBar);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1438);
			itemUsed = true;
		}
		break;

	case kItemMagnet:
		if (_vm->_useWith[WITH] == kItemBar) {
			_vm->removeIcon(kItemBar);
			_vm->replaceIcon(kItemMagnet, kItemMagneticBar);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1533);
			itemUsed = true;
		}
		break;

	case kItemMatch:
		if (_vm->_useWith[WITH] == kItemBroom) {
			_vm->removeIcon(kItemBroom);
			_vm->replaceIcon(kItemMatch, kItemMakeshiftTorch);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1575);
			itemUsed = true;
		}
		break;

	case kItemBroom:
		if (_vm->_useWith[WITH] == kItemMatch) {
			_vm->removeIcon(kItemBroom);
			_vm->replaceIcon(kItemMatch, kItemMakeshiftTorch);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1546);
			itemUsed = true;
		}
		break;

	case kItemProjector:
		if (_vm->_useWith[WITH] == kItemThreeWayAdapter) {
			_vm->removeIcon(kItemThreeWayAdapter);
			_vm->replaceIcon(kItemProjector, kItemProjector2);
			_vm->startCharacterAction(hUSEGG, 0, 0, 0);
			itemUsed = true;
		}
		break;

	case kItemThreeWayAdapter:
		if (_vm->_useWith[WITH] == kItemProjector) {
			_vm->removeIcon(kItemThreeWayAdapter);
			_vm->replaceIcon(kItemProjector, kItemProjector2);
			_vm->startCharacterAction(hUSEGG, 0, 0, 0);
			itemUsed = true;
		}
		break;

	case kItemSaltpetre:
	case kItemCharcoalSlack:
	case kItemJarWithSulfurPowder:
	case kItemCharcoalSaltpetrePowder:
	case kItemCharcoalSulfurPowder:
	case kItemSaltpetreSulfurPowder:
		if ((_vm->_useWith[WITH] == kItemSaltpetre) || (_vm->_useWith[WITH] == kItemCharcoalSlack) || (_vm->_useWith[WITH] == kItemJarWithSulfurPowder) ||
			(_vm->_useWith[WITH] == kItemCharcoalSaltpetrePowder) || (_vm->_useWith[WITH] == kItemCharcoalSulfurPowder) || (_vm->_useWith[WITH] == kItemSaltpetreSulfurPowder)) {
			_vm->removeIcon(_vm->_useWith[USED]);
			_vm->removeIcon(_vm->_useWith[WITH]);

			if (((_vm->_useWith[USED] == kItemSaltpetre) && (_vm->_useWith[WITH] == kItemCharcoalSlack)) ||
				((_vm->_useWith[WITH] == kItemSaltpetre) && (_vm->_useWith[USED] == kItemCharcoalSlack)))
				_vm->addIcon(kItemCharcoalSaltpetrePowder);
			if (((_vm->_useWith[USED] == kItemJarWithSulfurPowder) && (_vm->_useWith[WITH] == kItemCharcoalSlack)) ||
				((_vm->_useWith[WITH] == kItemJarWithSulfurPowder) && (_vm->_useWith[USED] == kItemCharcoalSlack)))
				_vm->addIcon(kItemCharcoalSulfurPowder);
			if (((_vm->_useWith[USED] == kItemJarWithSulfurPowder) && (_vm->_useWith[WITH] == kItemSaltpetre)) ||
				((_vm->_useWith[WITH] == kItemJarWithSulfurPowder) && (_vm->_useWith[USED] == kItemSaltpetre)))
				_vm->addIcon(kItemSaltpetreSulfurPowder);

			if ((_vm->_useWith[USED] == kItemJarWithSulfurPowder) || (_vm->_useWith[WITH] == kItemJarWithSulfurPowder))
				_vm->addIcon(kItemEmptyGlassJar);
			if ((_vm->_useWith[USED] >= kItemCharcoalSaltpetrePowder) || (_vm->_useWith[WITH] >= kItemCharcoalSaltpetrePowder))
				_vm->addIcon(kItemGunpowder);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1663);
			itemUsed = true;
		}
		break;

	case kItemPiratePistol:
		if (_vm->_useWith[WITH] == kItemGunpowder) {
			_vm->replaceIcon(kItemGunpowder, kItemLeftoverGunpowder);
			_vm->replaceIcon(kItemPiratePistol, kItemPistolWithGunpowder);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1676);
			itemUsed = true;
		} else if (_vm->_useWith[WITH] == kItemLeftoverGunpowder) {
			_vm->removeIcon(kItemLeftoverGunpowder);
			_vm->replaceIcon(kItemPiratePistol, kItemPistolWithGunpowder);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1700);
			itemUsed = true;
		}
		break;

	case kItemGunpowder:
		if (_vm->_useWith[WITH] == kItemPiratePistol) {
			_vm->replaceIcon(kItemGunpowder, kItemLeftoverGunpowder);
			_vm->replaceIcon(kItemPiratePistol, kItemPistolWithGunpowder);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1676);
			itemUsed = true;
		}
		break;

	case kItemLeftoverGunpowder:
		if (_vm->_useWith[WITH] == kItemPiratePistol) {
			_vm->removeIcon(kItemLeftoverGunpowder);
			_vm->replaceIcon(kItemPiratePistol, kItemPistolWithGunpowder);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1700);
			itemUsed = true;
		}
		break;

	case kItemIronBullet1:
	case kItemIronBullet2:
		if ((_vm->_useWith[WITH] == kItemPistolWithGunpowder) && !_vm->_inventoryObj[kItemPistolWithGunpowder].isFlagExtra()) {
			_vm->removeIcon(_vm->_useWith[USED]);
			_vm->replaceIcon(kItemPistolWithGunpowder, kItemLoadedPistol);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1683);
			_vm->_inventoryObj[kItemPistolWithGunpowder].setFlagExtra(true);
			itemUsed = true;
		} else if (_vm->_useWith[WITH] == kItemPistolWithGunpowder) {
			_vm->_textMgr->characterSay(1688);
			itemUsed = true;
		} else if (_vm->_useWith[WITH] == kItemPiratePistol) {
			_vm->_textMgr->characterSay(2009);
			itemUsed = true;
		}
		break;

	case kItemSilverBullet:
		if (_vm->_useWith[WITH] == kItemPistolWithGunpowder) {
			_vm->removeIcon(kItemSilverBullet);
			_vm->replaceIcon(kItemPistolWithGunpowder, kItemLoadedPistol);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1718);
			_vm->_inventoryObj[kItemLoadedPistol].setFlagExtra(true);
			itemUsed = true;
		} else if (_vm->_useWith[WITH] == kItemPiratePistol) {
			_vm->_textMgr->characterSay(2011);
			itemUsed = true;
		}
		break;

	case kItemEmptySyringe:
		if (_vm->_useWith[WITH] == kItemTubes) {
			_vm->removeIcon(kItemEmptySyringe);
			_vm->replaceIcon(kItemTubes, kItemPoisonSyringe);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1756);
			itemUsed = true;
		}
		break;

	case kItemTubes:
		if (_vm->_useWith[WITH] == kItemEmptySyringe) {
			_vm->removeIcon(kItemEmptySyringe);
			_vm->replaceIcon(kItemTubes, kItemPoisonSyringe);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1756);
			itemUsed = true;
		}
		break;

	case kItemDentalFloss:
		if (_vm->_useWith[WITH] == kItemInflatedGloves) {
			_vm->removeIcon(kItemDentalFloss);
			_vm->replaceIcon(kItemInflatedGloves, kItemInflatedGlovesWithFloss);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1756);
			itemUsed = true;
		} else if (_vm->_useWith[WITH] == kItemPoisonSyringe) {
			_vm->removeIcon(kItemDentalFloss);
			_vm->replaceIcon(kItemPoisonSyringe, kItemPoisonSyringeWithFloss);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1756);
			itemUsed = true;
		}
		break;

	case kItemInflatedGloves:
		if (_vm->_useWith[WITH] == kItemDentalFloss) {
			_vm->removeIcon(kItemDentalFloss);
			_vm->replaceIcon(kItemInflatedGloves, kItemInflatedGlovesWithFloss);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1756);
			itemUsed = true;
		} else if (_vm->_useWith[WITH] == kItemPoisonSyringeWithFloss) {
			_vm->removeIcon(kItemPoisonSyringeWithFloss);
			_vm->replaceIcon(kItemInflatedGloves, kItemFloatingPoisonSyringe);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1756);
			itemUsed = true;
		}
		break;

	case kItemPoisonSyringe:
		if (_vm->_useWith[WITH] == kItemDentalFloss) {
			_vm->removeIcon(kItemDentalFloss);
			_vm->replaceIcon(kItemPoisonSyringe, kItemPoisonSyringeWithFloss);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1756);
			itemUsed = true;
		} else if (_vm->_useWith[WITH] == kItemInflatedGlovesWithFloss) {
			_vm->removeIcon(kItemPoisonSyringe);
			_vm->replaceIcon(kItemInflatedGlovesWithFloss, kItemFloatingPoisonSyringe);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1756);
			itemUsed = true;
		}
		break;

	case kItemInflatedGlovesWithFloss:
		if (_vm->_useWith[WITH] == kItemPoisonSyringe) {
			_vm->removeIcon(kItemPoisonSyringe);
			_vm->replaceIcon(kItemInflatedGlovesWithFloss, kItemFloatingPoisonSyringe);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1756);
			itemUsed = true;
		}
		break;

	case kItemPoisonSyringeWithFloss:
		if (_vm->_useWith[WITH] == kItemInflatedGloves) {
			_vm->removeIcon(kItemPoisonSyringeWithFloss);
			_vm->replaceIcon(kItemInflatedGloves, kItemFloatingPoisonSyringe);
			_vm->startCharacterAction(hUSEGG, 0, 0, 1756);
			itemUsed = true;
		}
		break;

	default:
		break;
	}

	if (!itemUsed)
		_vm->_textMgr->characterSay(_vm->_inventoryObj[_vm->_useWith[USED]]._action);
	else
		_vm->setInventoryStart(_vm->_iconBase, INVENTORY_SHOW);
}

void LogicManager::useInventoryWithScreen() {
	bool updateInventory = true;
	bool printSentence = true;

	if (!_vm->_useWith[USED] || !_vm->_useWith[WITH])
		warning("useInventoryWithScreen - _useWith not set properly");

	_vm->_animMgr->smkStop(kSmackerIcon);
	if (_vm->_pathFind->_characterInMovement)
		return;

	switch (_vm->_useWith[USED]) {
	case kItemBankNote:
		if (_vm->_useWith[WITH] == oDISTRIBUTORE13 && !_vm->_obj[oDISTRIBUTORE13].isFlagExtra()) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a131USABANCONOTA, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
			_vm->_obj[oDISTRIBUTORE13].setFlagExtra(true);
		} else if ((_vm->_useWith[WITH] == oDISTRIBUTORE13) && _vm->_obj[oDISTRIBUTORE13].isFlagExtra() && _vm->isObjectVisible(oLATTINA13)) {
			_vm->_textMgr->characterSay(1410);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oDISTRIBUTORE13) && _vm->_obj[oDISTRIBUTORE13].isFlagExtra()) {
			if (!_vm->_obj[oSCOMPARTO13].isFlagExtra()) {
				_vm->_obj[oSCOMPARTO13].setFlagExtra(true);
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1311DABOTTADISTRIBUTORE, 0, 0, _vm->_useWith[WITH]);
			} else
				_vm->_textMgr->characterSay(1411);

			printSentence = false;
		} else if (_vm->_useWith[WITH] == oTICKETOFFICE16) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a165USABANCONOTA, 0, 0, _vm->_useWith[WITH]);
			_vm->_inventoryObj[kItemBankNote].setFlagExtra(true);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == ocPOLIZIOTTO16) && _vm->_inventoryObj[kItemBankNote].isFlagExtra()) {
			_vm->_dialogMgr->toggleChoice(62, true);
			_vm->_dialogMgr->playDialog(dPOLIZIOTTO16);
			printSentence = false;
		}
		break;
	case kItemLetter:
		if (_vm->_useWith[WITH] == oPENPADA13) {
			if (_vm->_room[kRoom14].isDone())
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1312METTELETTERARICALCA, kRoom14, 14, _vm->_useWith[WITH]);
			else {
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a135METTELETTERA, 0, 0, _vm->_useWith[WITH]);
				_vm->setObjectVisible(oLETTERA13, true);
				_vm->_inventoryObj[kItemLetter].setFlagExtra(true);
				_vm->removeIcon(kItemLetter);
			}
			printSentence = false;
		}
		break;
	case kItemRepairedFuse:
		if (_vm->_useWith[WITH] == oPANELA12) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a127USEWRAPPEDFUSED, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;
	case kItemBurnedFuse:
		if (_vm->_useWith[WITH] == oPANELA12) {
			_vm->_textMgr->characterSay(62);
			printSentence = false;
		}
		break;
	case kItemLetterboxKey:
		if (_vm->_useWith[WITH] == oBOXES12) {
			if (!_vm->_obj[oBOXES12].isFlagExtra()) {
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, kAnim121TryOpenLetterboxes, 0, 0, _vm->_useWith[WITH]);
				_vm->_obj[oBOXES12].setFlagExtra(true);
				printSentence = false;
			} else {
				_vm->_textMgr->characterSay(1426);
				printSentence = false;
			}
		} else if (_vm->_useWith[WITH] == oBOX12 && !_vm->_inventoryObj[kItemLetter].isFlagExtra()) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, kAnim122OpenLetterbox17, 0, 0, oBOX12);
			_vm->_inventoryObj[kItemLetter].setFlagExtra(true);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oBOX12) && _vm->_inventoryObj[kItemLetter].isFlagExtra()) {
			_vm->_textMgr->characterSay(1429);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oCARA11) || (_vm->_useWith[WITH] == oCARB11) || (_vm->_useWith[WITH] == oTAKE12) || (_vm->_useWith[WITH] == oSTRONGBOXC15) || (_vm->_useWith[WITH] == oDOOR18) || (_vm->_useWith[WITH] == oPADLOCK1B) || (_vm->_useWith[WITH] == oDOORC21) || (_vm->_useWith[WITH] == oPANELC23) || (_vm->_useWith[WITH] == oDOOR2A) || (_vm->_useWith[WITH] == oDOORC33) || (_vm->_useWith[WITH] == oFRONTOFFICEC35) || (_vm->_useWith[WITH] == oCASSETTOC36) || (_vm->_useWith[WITH] == oDOORC54) || (_vm->_useWith[WITH] == oDOOR57C55) || (_vm->_useWith[WITH] == oDOOR58C55) || (_vm->_useWith[WITH] == oDOORS56) || (_vm->_useWith[WITH] == oDOORS57)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1426);
		}
		break;
	case kItemLiftCard:
		if (_vm->_useWith[WITH] == oSLOT12 || _vm->_useWith[WITH] == oSLOT13 || _vm->_useWith[WITH] == oSLOT16) {
			printSentence = false;
			_vm->_inventoryObj[kItemLiftCard].setFlagExtra(true);
			_vm->_obj[oSLOT12].setFlagPerson(true);
			_vm->_obj[oLIFTA12].setFlagPerson(true);
			_vm->doMouseTalk(_vm->_useWith[WITH]);
		} else if ((_vm->_useWith[WITH] == oTICKETOFFICE16) || (_vm->_useWith[WITH] == oSLOT23) || (_vm->_useWith[WITH] == oFRONTOFFICEA35) || (_vm->_useWith[WITH] == oSLOTA58) || (_vm->_useWith[WITH] == oSLOTB58)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1419);
		}
		break;
	case kItemPen:
		printSentence = false;
		if (((_vm->_useWith[WITH] == oPENPADA13) || (_vm->_useWith[WITH] == oLETTERA13)) && _vm->isObjectVisible(oLETTERA13))
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a137RICALCAFIRMA, kRoom14, 14, _vm->_useWith[WITH]);
		else if ((_vm->_useWith[WITH] == oPENPADA13) && _vm->_room[kRoom14].isDone())
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1312METTELETTERARICALCA, kRoom14, 14, _vm->_useWith[WITH]);
		else if (_vm->_useWith[WITH] == oPENPADA13) {
			if (!_vm->isObjectVisible(oBOX12)) {
				printSentence = false;
				_vm->_textMgr->characterSay(2005);
			} else
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a134USAMAGNETICPEN, 0, 0, _vm->_useWith[WITH]);
		} else
			printSentence = true;
		break;

	case kItemTubeOfAcidFull:
		if (_vm->_useWith[WITH] == oBAR11) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, kAnim113UseVial, 0, 0, _vm->_useWith[WITH]);
			_vm->replaceIcon(kItemTubeOfAcidFull, kItemTubeOfAcidHalfFull);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oPADLOCK1B) {
			if (_vm->isObjectVisible(oTOMBINOA1B))
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2AVERSAACIDO, 0, 0, _vm->_useWith[WITH]);
			else
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2VERSAACIDO, 0, 0, _vm->_useWith[WITH]);
			_vm->setObjectAnim(oBOTOLAC1B, a1B3APREBOTOLA);
			_vm->replaceIcon(kItemTubeOfAcidFull, kItemTubeOfAcidHalfFull);
			_vm->_animMgr->_animTab[kAnim113UseVial]._atFrame[0]._index = 1483;
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == ocGUARD18) || (_vm->_useWith[WITH] == oMANHOLEC1B)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1476);
		}
		break;

	case kItemTubeOfAcidHalfFull:
		if (_vm->_useWith[WITH] == oBAR11) {
			_vm->_animMgr->_animTab[kAnim113UseVial]._atFrame[0]._index = 1483;
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, kAnim113UseVial, 0, 0, _vm->_useWith[WITH]);
			_vm->removeIcon(kItemTubeOfAcidHalfFull);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oPADLOCK1B) {
			_vm->_animMgr->_animTab[a1B2AVERSAACIDO]._atFrame[2]._index = 1483;
			_vm->_animMgr->_animTab[a1B2VERSAACIDO]._atFrame[2]._index = 1483;
			if (_vm->isObjectVisible(oTOMBINOA1B))
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2AVERSAACIDO, 0, 0, _vm->_useWith[WITH]);
			else
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2VERSAACIDO, 0, 0, _vm->_useWith[WITH]);
			_vm->setObjectAnim(oBOTOLAC1B, a1B3APREBOTOLA);
			_vm->removeIcon(kItemTubeOfAcidHalfFull);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oDOOR2A) || (_vm->_useWith[WITH] == oDOOR2B)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1508);
		}
		break;

	case kItemRubysSafeboxKey:
		if (_vm->_useWith[WITH] == oSTRONGBOXC15) {
			if (_vm->isDemo()) {
				_vm->demoOver();
			} else {
				_vm->_dialogMgr->playDialog(dF151);
				_vm->setObjectVisible(oSTRONGBOXC15, false);
				_vm->setObjectVisible(oCASSAFORTEA15, true);
				_vm->setObjectVisible(oSLOT13, false);
				_vm->setObjectVisible(oASCENSOREA13, false);
				_vm->setObjectVisible(oASCENSOREC13, true);
				_vm->addIcon(kItemPistol);
				_vm->addIcon(kItemTubeOfAcidFull);
				_vm->addIcon(kItemRubysReport);
				_vm->removeIcon(kItemRubysSafeboxKey);
				printSentence = false;
				updateInventory = false;
			}
		} else if ((_vm->_useWith[WITH] == oCARA11) || (_vm->_useWith[WITH] == oCARB11) || (_vm->_useWith[WITH] == oTAKE12) || (_vm->_useWith[WITH] == oBOX12) || (_vm->_useWith[WITH] == oDOOR18) || (_vm->_useWith[WITH] == oPADLOCK1B) || (_vm->_useWith[WITH] == oDOORC21) || (_vm->_useWith[WITH] == oPANELC23) || (_vm->_useWith[WITH] == oDOOR2A) || (_vm->_useWith[WITH] == oDOORC33) || (_vm->_useWith[WITH] == oFRONTOFFICEC35) || (_vm->_useWith[WITH] == oCASSETTOC36) || (_vm->_useWith[WITH] == oDOORC54) || (_vm->_useWith[WITH] == oDOOR57C55) || (_vm->_useWith[WITH] == oDOOR58C55) || (_vm->_useWith[WITH] == oDOORS56) || (_vm->_useWith[WITH] == oDOORS57)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1469);
		}
		break;

	case kItemBar:
		if (_vm->_useWith[WITH] == oMANHOLEC1B) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B1USASBARRA, 0, 0, _vm->_useWith[WITH]);
			_vm->setObjectAnim(oBOTOLAA1B, a1B6ASCENDEBOTOLA);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oCHAIN21) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a216, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oALTOPARLANTE25) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a251, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oDOORC33) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a333LOSEBAR, 0, 0, _vm->_useWith[WITH]);
			_vm->removeIcon(kItemBar);
			printSentence = false;
		} else if (_vm->_obj[_vm->_useWith[WITH]].isFlagPerson()) {
			_vm->_textMgr->characterSay(1436);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oTAKE12) || (_vm->_useWith[WITH] == oSTRONGBOXC15) || (_vm->_useWith[WITH] == oDOOR18) || (_vm->_useWith[WITH] == oPADLOCK1B) || (_vm->_useWith[WITH] == oDOORC21) || (_vm->_useWith[WITH] == oPANELC23) || (_vm->_useWith[WITH] == oDOOR2A) || (_vm->_useWith[WITH] == oDOOR2B)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1435);
		}
		break;

	case kItemSubwayCard:
		if ((_vm->_useWith[WITH] == oTICKETOFFICE16) && _vm->_obj[oMAPPA16].isFlagExtra()) {
			if (_vm->_dialogMgr->isDialogFinished(49)) {
				_vm->_textMgr->characterSay(1457);
				printSentence = false;
			} else {
				_vm->_dialogMgr->toggleChoice(46, false);
				_vm->_dialogMgr->toggleChoice(47, false);
				_vm->_dialogMgr->toggleChoice(48, false);
				_vm->_dialogMgr->toggleChoice(49, true);
				_vm->playScript(s16CARD);
				printSentence = false;
			}
		} else if (_vm->_useWith[WITH] == oSLOT23) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2311, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oSLOT12) || (_vm->_useWith[WITH] == oSLOT13) || (_vm->_useWith[WITH] == oSLOT16) || (_vm->_useWith[WITH] == oFRONTOFFICEA35) || (_vm->_useWith[WITH] == oSLOTA58) || (_vm->_useWith[WITH] == oSLOTB58)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1419);
		}
		break;

	case kItemFiveCreditCoin:
		if ((_vm->_useWith[WITH] == oTICKETOFFICE16) && _vm->_obj[oMAPPA16].isFlagExtra()) {
			_vm->_dialogMgr->toggleChoice(46, false);
			_vm->_dialogMgr->toggleChoice(47, false);
			_vm->_dialogMgr->toggleChoice(48, false);
			_vm->_dialogMgr->toggleChoice(50, true);
			_vm->playScript(S16MONEY);
			printSentence = false;
			_vm->removeIcon(kItemFiveCreditCoin);
			_vm->_obj[oFINGERPADP16].setFlagRoomOut(true);
		} else if (_vm->_useWith[WITH] == oTICKETOFFICE16) {
			_vm->_textMgr->characterSay(146);
			printSentence = false;
		}
		break;

	case kItemPlasticPlate:
		if (_vm->_useWith[WITH] == oTELEFAXF17) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a172USAPLASTICA, 0, 0, _vm->_useWith[WITH]);
			_vm->_obj[oTELEFAXF17]._examine = 1486;
			printSentence = false;
		}
		break;

	case kItemRubysPhoto:
		if ((_vm->_useWith[WITH] == ocTRAMP17) && (_vm->_dialogMgr->isDialogFinished(81))) {
			_vm->_textMgr->characterSay(1463);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == ocTRAMP17) && (_vm->_dialogMgr->isDialogFinished(91))) {
			_vm->_obj[ocTRAMP17]._action = 1462;
			_vm->_textMgr->characterSay(_vm->_obj[ocTRAMP17]._action);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == ocTRAMP17) && (!_vm->_dialogMgr->isDialogFinished(78) || (_vm->_dialogMgr->isDialogFinished(79) || (_vm->_dialogMgr->isDialogFinished(83) && !_vm->_dialogMgr->isDialogFinished(92))))) {
			_vm->_dialogMgr->toggleChoice(78, true);
			if (_vm->_dialogMgr->isDialogFinished(79) || _vm->_dialogMgr->isDialogFinished(83)) {
				_vm->_dialogMgr->toggleChoice(92, true);
				if (_vm->_dialogMgr->isDialogFinished(80))
					_vm->_dialogMgr->toggleChoice(121, true);
				else
					_vm->_dialogMgr->toggleChoice(122, true);
			} else
				_vm->_dialogMgr->toggleChoice(91, true);
			_vm->_dialogMgr->playDialog(dTRAMP171);
			updateInventory = false;
			printSentence = false;
		} else if (_vm->_useWith[WITH] == ocPOLIZIOTTO16) {
			_vm->_obj[ocPOLIZIOTTO16].setFlagExtra(true);
			_vm->_textMgr->characterSay(1461);
			if (_vm->_dialogMgr->isDialogFinished(61) && _vm->_dialogMgr->isDialogFinished(62) && _vm->_obj[ocPOLIZIOTTO16].isFlagExtra())
				_vm->setObjectVisible(ocPOLIZIOTTO16, false);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == ocGUARD18) {
			if (_vm->_dialogMgr->isDialogFinished(152))
				_vm->_textMgr->characterSay(1465);
			else
				_vm->_textMgr->characterSay(1464);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == ocNEGOZIANTE1A) {
			_vm->_textMgr->characterSay(1466);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == ocEVA19) {
			_vm->_textMgr->characterSay(1465);
			printSentence = false;
		}

		break;

	case kItemBeerCan:
		if (_vm->_useWith[WITH] == ocTRAMP17) {
			if (_vm->_dialogMgr->isDialogFinished(79) || _vm->_dialogMgr->isDialogFinished(83)) {
				_vm->_dialogMgr->toggleChoice(80, true);
				if (_vm->_dialogMgr->isDialogFinished(81)) {
					_vm->_dialogMgr->toggleChoice(81, true);
					_vm->_dialogMgr->clearExitFlag(80);
				}
				_vm->_dialogMgr->playDialog(dTRAMP171);
				updateInventory = false;
				_vm->removeIcon(kItemBeerCan);
				printSentence = false;
			} else {
				_vm->_dialogMgr->toggleChoice(79, true);
				if (!_vm->_dialogMgr->isDialogFinished(78)) {
					_vm->_dialogMgr->toggleChoice(106, true);
					_vm->_obj[ocTRAMP17]._action = 213;
				} else {
					_vm->_dialogMgr->toggleChoice(107, true);
					if (_vm->_dialogMgr->isDialogFinished(80))
						_vm->_dialogMgr->toggleChoice(121, true);
					else
						_vm->_dialogMgr->toggleChoice(122, true);
				}
				_vm->_dialogMgr->playDialog(dTRAMP171);
				updateInventory = false;
				_vm->removeIcon(kItemBeerCan);
				printSentence = false;
			}
		}

		break;

	case kItemWineBottle:
		if (_vm->_useWith[WITH] == ocTRAMP17) {
			if (_vm->_dialogMgr->isDialogFinished(79) || _vm->_dialogMgr->isDialogFinished(83)) {
				_vm->_dialogMgr->toggleChoice(80, true);
				if (_vm->_dialogMgr->isDialogFinished(81)) {
					_vm->_dialogMgr->toggleChoice(81, true);
					_vm->_dialogMgr->clearExitFlag(80);
				}
				_vm->_dialogMgr->playDialog(dTRAMP171);
				updateInventory = false;
				_vm->removeIcon(kItemWineBottle);
				printSentence = false;
			} else {
				_vm->_dialogMgr->toggleChoice(83, true);
				if (!_vm->_dialogMgr->isDialogFinished(78)) {
					_vm->_dialogMgr->toggleChoice(106, true);
					_vm->_obj[ocTRAMP17]._action = 213;
				} else {
					_vm->_dialogMgr->toggleChoice(107, true);
					if (_vm->_dialogMgr->isDialogFinished(80))
						_vm->_dialogMgr->toggleChoice(121, true);
					else
						_vm->_dialogMgr->toggleChoice(122, true);
				}
				_vm->_dialogMgr->playDialog(dTRAMP171);
				updateInventory = false;
				_vm->removeIcon(kItemWineBottle);
				printSentence = false;
			}
		}

		break;

	case kItemBottleOfChateau:
		if (_vm->_useWith[WITH] == ocNEGOZIANTE1A) {
			printSentence = false;
			if (_vm->_dialogMgr->isDialogFinished(183)) {
				_vm->_dialogMgr->toggleChoice(185, true);
				_vm->_dialogMgr->playDialog(dSHOPKEEPER1A);
				updateInventory = false;
				_vm->replaceIcon(kItemBottleOfChateau, kItemMembershipCard);
			} else
				_vm->_textMgr->characterSay(2006);
		}
		break;

	case kItemMembershipCard:
		if ((_vm->_useWith[WITH] == ocGUARD18) && !(_vm->_dialogMgr->isDialogFinished(155))) {
			_vm->_dialogMgr->toggleChoice(155, true);
			_vm->_dialogMgr->playDialog(dGUARDIAN18);
			updateInventory = false;
			_vm->_obj[ocGUARD18].setFlagPerson(false);
			_vm->_obj[oDOORC18].setFlagRoomOut(true);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == ocGUARD18) {
			_vm->_textMgr->characterSay(1494);
			printSentence = false;
		}
		break;

	case kItemRatOnSkate:
		if ((_vm->_useWith[WITH] == oDONNA1D) && (_vm->_mousePos.x >= _vm->_obj[oDONNA1D]._area.left && (_vm->_mousePos.x >= _vm->_obj[oDONNA1D]._area.top + TOP) && (_vm->_mousePos.x <= _vm->_obj[oDONNA1D]._area.right) && (_vm->_mousePos.y <= _vm->_obj[oDONNA1D]._area.bottom + TOP))) {
			_vm->_animMgr->_animTab[aBKG1D].toggleAnimArea(1, false);
			_vm->_dialogMgr->playDialog(dF1D1);
			updateInventory = false;
			_vm->removeIcon(kItemRatOnSkate);
			_vm->read3D("1d2.3d"); // after skate
			_vm->setObjectVisible(oDONNA1D, false);
			_vm->_room[_vm->_curRoom].setExtra(true);
			printSentence = false;
		}
		break;

	case kItemPistol:
		if ((_vm->_useWith[WITH] == oDOORC21) && !(_vm->_room[kRoom21].hasExtra())) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a211, 0, 0, _vm->_useWith[WITH]);
			_vm->_inventoryObj[kItemPistol]._examine = 1472;
			_vm->_inventoryObj[kItemPistol]._action = 1473;
			printSentence = false;
		}
		break;

	case kItemScrewdriver:
		if (_vm->_useWith[WITH] == oESSE21) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a213, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oCOPERCHIOC31) {
			_vm->_soundMgr->play(wCOVER31);
			_vm->setObjectVisible(oCOPERCHIOC31, false);
			_vm->setObjectVisible(oCOPERCHIOA31, true);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oCOPERCHIOA31) {
			_vm->setObjectVisible(oCOPERCHIOA31, false);
			_vm->setObjectVisible(oCOPERCHIOC31, true);
			_vm->setObjectVisible(oPANNELLOM31, false);
			_vm->setObjectVisible(oPANNELLOMA31, true);
			if (_vm->isObjectVisible(oFILOTC31)) {
				_vm->setObjectAnim(oPANNELLOM31, 0);
				_vm->_obj[oPANNELLOM31]._examine = 715;
				_vm->_obj[oPANNELLOM31]._action = 716;
				_vm->_obj[oPANNELLOM31].setFlagRoomOut(false);
			}
			_vm->_soundMgr->play(wCOVER31);
			printSentence = false;
			_vm->changeRoom(kRoom31, a3118CHIUDEPANNELLO, 3);
		} else if (_vm->_useWith[WITH] == oPANNELLO55) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a5512, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oPANNELLOC56) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a568, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oSLOT23) || (_vm->_useWith[WITH] == oPRESA35) || (_vm->_useWith[WITH] == oSERRATURA33)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1520);
		}
		break;

	case kItemMetallicS:
		if (_vm->_useWith[WITH] == oCATENA21) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a214, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemTinCanWithOil:
		if (_vm->_useWith[WITH] == oMANIGLIONE22) {
			if (_vm->isObjectVisible(oARMADIETTORC22))
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a228, 0, 0, _vm->_useWith[WITH]);
			else
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a228A, 0, 0, _vm->_useWith[WITH]);
			_vm->removeIcon(kItemTinCanWithOil);
			_vm->addIcon(kItemEmptyTinCan);
			if (_vm->_inventoryObj[kItemCanWithFuel].isFlagExtra()) {
				_vm->removeIcon(kItemEmptyTinCan);
				_vm->replaceIcon(kItemCanWithFuel, kItemLargeCans);
			}
			printSentence = false;
			// FIXME: is fallthrough intended here?
		} else
			break;
		// fall through
	case kItemBookstoreKey:
		if (_vm->_useWith[WITH] == oDOOR2A) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2A2USEKEY, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oPANELC23) || (_vm->_useWith[WITH] == oDOORC33) || (_vm->_useWith[WITH] == oFRONTOFFICEC35) || (_vm->_useWith[WITH] == oCASSETTOC36) || (_vm->_useWith[WITH] == oDOORC54) || (_vm->_useWith[WITH] == oDOOR57C55) || (_vm->_useWith[WITH] == oDOOR58C55) || (_vm->_useWith[WITH] == oDOORS56) || (_vm->_useWith[WITH] == oDOORS57)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1512);
		}
		break;

	case kItemFaultyBulb:
		if ((_vm->_useWith[WITH] == oPORTALAMPADE2B) && !_vm->_obj[_vm->_useWith[WITH]]._anim) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B6METTELAMPADINA, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemSocketSpanner:
		if (_vm->_useWith[WITH] == oPANELC23) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a233, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oDOORC33) || (_vm->_useWith[WITH] == oFRONTOFFICEC35) || (_vm->_useWith[WITH] == oCASSETTOC36) || (_vm->_useWith[WITH] == oDOORC54) || (_vm->_useWith[WITH] == oDOOR57C55) || (_vm->_useWith[WITH] == oDOOR58C55) || (_vm->_useWith[WITH] == oDOORS56) || (_vm->_useWith[WITH] == oDOORS57)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1525);
		}
		break;

	case kItemCopperLeads:
		if ((_vm->_useWith[WITH] == oCONTATTI23) && _vm->isObjectVisible(oLEVAS23)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a236, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oLEVAG23) {
			_vm->_textMgr->characterSay(2015);
			printSentence = false;
		}
		break;

	case kItemPliers:
		if ((_vm->_useWith[WITH] == oCAVO2H) && _vm->isObjectVisible(oCARTELLONE2H)) {
			_vm->_dialogMgr->playDialog(dF2H1);
			_vm->setObjectVisible(oPASSAGE24, true);
			_vm->setObjectVisible(omPASSAGGIO24, true);
			_vm->setObjectVisible(oCARTELLONE24, false);
			_vm->setObjectVisible(oCARTELLONE2H, false);
			_vm->setObjectVisible(oPASSERELLA24, false);
			_vm->setObjectVisible(oRUINS24, false);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oTUBOT34) && _vm->isObjectVisible(oVALVOLAC34)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a341USAPINZE, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oTUBOT34) && _vm->isObjectVisible(oVALVOLA34)) {
			_vm->_textMgr->characterSay(2007);
			printSentence = false;
		} else
			printSentence = true;
		break;

	case kItemCanWithFuel:
		if ((_vm->_useWith[WITH] == oSERBATOIOA2G) && !_vm->_inventoryObj[kItemCanWithFuel].isFlagExtra()) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2G4VERSALATTINA, 0, 0, _vm->_useWith[WITH]);
			_vm->_inventoryObj[kItemCanWithFuel].setFlagExtra(true);
			_vm->_inventoryObj[kItemCanWithFuel]._examine = 1537;
			if (_vm->iconPos(kItemEmptyTinCan) != -1) {
				_vm->removeIcon(kItemEmptyTinCan);
				_vm->replaceIcon(kItemCanWithFuel, kItemLargeCans);
			}
			if (_vm->_inventoryObj[kItemGasCylinder].isFlagExtra() && _vm->_inventoryObj[kItemCanWithFuel].isFlagExtra()) {
				_vm->_obj[oSERBATOIOA2G]._examine = 670;
				_vm->_obj[oSERBATOIOA2G]._action = 671;
			} else {
				_vm->_obj[oSERBATOIOA2G]._examine = 667;
				_vm->_obj[oSERBATOIOA2G]._action = 669;
			}
			printSentence = false;
		}
		break;

	case kItemGasCylinder:
		if (_vm->_useWith[WITH] == oSERBATOIOA2G) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2G5METTEBOMBOLA, 0, 0, _vm->_useWith[WITH]);
			_vm->_inventoryObj[kItemGasCylinder].setFlagExtra(true);
			if (_vm->_inventoryObj[kItemGasCylinder].isFlagExtra() && _vm->_inventoryObj[kItemCanWithFuel].isFlagExtra()) {
				_vm->_obj[oSERBATOIOA2G]._examine = 670;
				_vm->_obj[oSERBATOIOA2G]._action = 671;
			} else {
				_vm->_obj[oSERBATOIOA2G]._examine = 668;
				_vm->_obj[oSERBATOIOA2G]._action = 669;
			}
			printSentence = false;
		}
		break;

	case kItemFlare:
		if ((_vm->_useWith[WITH] == oSERBATOIOA2G) && _vm->_inventoryObj[kItemGasCylinder].isFlagExtra() && _vm->_inventoryObj[kItemCanWithFuel].isFlagExtra()) {
			_vm->changeRoom(kRoom2GV);
			_vm->removeIcon(kItemFlare);
			printSentence = false;
		}
		break;

	case kItemMicrowaveGun:
		if (_vm->_useWith[WITH] == oDINOSAURO2E) {
			_vm->_animMgr->_animTab[aBKG2E].toggleAnimArea(2, false);
			_vm->_dialogMgr->playDialog(dF2E2);
			_vm->setObjectVisible(oDINOSAURO2E, false);
			_vm->setObjectAnim(oCATWALKA2E, a2E2PRIMAPALLONTANANDO);
			printSentence = false;
		}
		break;

	case kItemPincers:
	case kItemMagneticBar:
		if (_vm->_useWith[WITH] == oCHAIN21) {
			if (_vm->_room[_vm->_curRoom].hasExtra()) {
				if (_vm->_useWith[USED] == kItemPincers)
					_vm->_dialogMgr->playDialog(dF212B);
				else
					_vm->_dialogMgr->playDialog(dF212);
				printSentence = false;
			} else {
				if (_vm->_useWith[USED] == kItemPincers)
					_vm->_dialogMgr->playDialog(dF213B);
				else
					_vm->_dialogMgr->playDialog(dF213);
				printSentence = false;
			}
		} else if ((_vm->_useWith[WITH] == oDOORC33) && (_vm->_useWith[USED] == kItemMagneticBar)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a333LOSEBAR, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oSNAKEU52) && (_vm->_useWith[USED] == kItemPincers)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a527, 0, 0, _vm->_useWith[WITH]);
			_vm->setObjectAnim(oSCAVO51, a516);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oSERPENTEA52) && (_vm->_useWith[USED] == kItemPincers)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a528, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oSERPENTEB52) && (_vm->_useWith[USED] == kItemPincers) && (_vm->iconPos(kItemSnakeHead) == -1)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a523, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemWorkingBulb:
		if (_vm->_useWith[WITH] == oPORTALAMPADE29) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a293AVVITALAMPADINA, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemEmptyBeerCan:
		if ((_vm->_useWith[WITH] == oBRACIERES28) && _vm->_obj[oBRACIERES28].isFlagExtra()) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a286, 0, 0, _vm->_useWith[WITH]);
			_vm->_obj[oBRACIERES28]._examine = 456;
			printSentence = false;
		} else
			printSentence = true;
		break;

	case kItemElevatorRemoteControl:
		if (_vm->_useWith[WITH] == oTASTO2F) {
			if (_vm->isObjectVisible(oASCENSORE2F)) {
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2F10PANNELLOSICHIUDE, 0, 0, _vm->_useWith[WITH]);
				_vm->setObjectAnim(oBIDONE2F, a2F5CFRUGABIDONE);
			} else {
				if (!_vm->_inventoryObj[kItemElevatorRemoteControl].isFlagExtra())
					_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2F9PPANNELLOSIAPRE, 0, 0, _vm->_useWith[WITH]);
				else
					_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2F9PANNELLOSIAPRE, 0, 0, _vm->_useWith[WITH]);
				_vm->_inventoryObj[kItemElevatorRemoteControl].setFlagExtra(true);
				_vm->setObjectAnim(oBIDONE2F, a2F5FRUGABIDONE);
			}
			printSentence = false;
		}
		break;

	case kItemDamagedMinicom:
		if (_vm->_useWith[WITH] == oSERRATURA33) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a338POSASAM, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemMakeshiftTorch:
		if ((_vm->_useWith[WITH] == oSENSOREV32) && _vm->isObjectVisible(oFILOTC31)) {
			_vm->setObjectVisible(oPANNELLOMA31, true);
			_vm->setObjectVisible(oPANNELLOM31, false);
			_vm->_obj[oPANNELLOMA31]._examine = 717;
			_vm->_obj[oPANNELLOMA31]._action = 718;
			_vm->setObjectVisible(oCORPO31, true);
			_vm->setObjectVisible(od31TO35, true);
			_vm->setObjectVisible(omd31TO35, true);
			_vm->setObjectVisible(oMONTACARICHI31, true);
			_vm->setObjectVisible(oPANNELLO31, false);
			_vm->setObjectVisible(oPANNELLON31, false);
			_vm->_room[kRoom32].setExtra(true);
			_vm->changeRoom(kRoom31, 0, 11);

			printSentence = false;
		}
		break;

	case kItemProjector:
		if (_vm->_useWith[WITH] == oPRESA35) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a351PROVAPROIETTOREPRESA, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oTRIPLA35) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a355ATTPROIETTORETRIPLAEPRESA, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oRIBELLEA35) {
			printSentence = false;
			_vm->_textMgr->characterSay(1578);
		}
		break;

	case kItemProjector2:
		if (_vm->_useWith[WITH] == oPRESA35) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a352ATTPROIETTOREETRIPLAPRESA, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oRIBELLEA35) {
			printSentence = false;
			_vm->_textMgr->characterSay(1590);
		}
		break;

	case kItemThreeWayAdapter:
		if (_vm->_useWith[WITH] == oPRESA35) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a353ATTACCATRIPLAPRESA, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemLaserGun:
		if (_vm->_useWith[WITH] == oFRONTOFFICEC35) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a3511APRESPORTELLO, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oSNAKEU52) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a522, 0, 0, _vm->_useWith[WITH]);
			_vm->setObjectAnim(oSCAVO51, a516);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oLUCCHETTO53) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a532, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oDOORMC36) || (_vm->_useWith[WITH] == oPORTALC36) || (_vm->_useWith[WITH] == oSCANNERMA36) || (_vm->_useWith[WITH] == oSCANNERLA36) || (_vm->_useWith[WITH] == oCASSETTOC36) || (_vm->_useWith[WITH] == oRETE52) || (_vm->_useWith[WITH] == oTELECAMERA52) || (_vm->_useWith[WITH] == oSERPENTET52) || (_vm->_useWith[WITH] == oLAGO53)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1597);
		}
		break;

	case kItemDeskKey:
		if (_vm->_useWith[WITH] == oCASSETTOC36) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a364APRECASSETTO, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oFRONTOFFICEC35) || (_vm->_useWith[WITH] == oDOORC54) || (_vm->_useWith[WITH] == oDOOR57C55) || (_vm->_useWith[WITH] == oDOOR58C55) || (_vm->_useWith[WITH] == oDOORS56) || (_vm->_useWith[WITH] == oDOORS57)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1594);
		}
		break;

	case kItemStethoscope:
		if (_vm->_useWith[WITH] == oPORTALC36) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a368USASTETOSCOPIO, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemSecurityCard:
		if ((_vm->_useWith[WITH] == oFRONTOFFICEA35) && !_vm->_obj[oFRONTOFFICEA35].isFlagExtra()) {
			_vm->changeRoom(kRoom35P, 0, 10);
			_vm->removeIcon(kItemSecurityCard);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oFRONTOFFICEA35) && _vm->_obj[oFRONTOFFICEA35].isFlagExtra()) {
			_vm->_textMgr->characterSay(1844);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oSLOTA58) || (_vm->_useWith[WITH] == oSLOTB58)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1602);
		}
		break;

	case kItemHalfCreditCoin:
		if ((_vm->_useWith[WITH] == oFESSURA41) &&
			((_vm->_obj[oFUCILE42]._anim != 0) && (_vm->_obj[oFUCILE42]._anim != a428) && (_vm->_obj[oFUCILE42]._anim != a429))) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a412, 0, 0, _vm->_useWith[WITH]);
			if (_vm->isObjectVisible(oZAMPA41))
				_vm->setObjectAnim(oSLOT41, a417);
			else if (_slotMachine41Counter <= 2)
				_vm->setObjectAnim(oSLOT41, a414);
			else
				_vm->_textMgr->characterSay(2015);
			++_slotMachine41Counter;
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oFESSURA41) && ((_vm->_obj[oFUCILE42]._anim == 0) || (_vm->_obj[oFUCILE42]._anim == a428) || (_vm->_obj[oFUCILE42]._anim == a429))) {
			_vm->_textMgr->characterSay(2010);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oFESSURA42) {
			_vm->_textMgr->characterSay(924);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oCAMPANA4U) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U3, 0, 0, _vm->_useWith[WITH]);
			_vm->_obj[oCAMPANA4U].setFlagExtra(true);
			if (_vm->_obj[oCAMPANA4U].isFlagExtra() && _vm->_inventoryObj[kItemIronBullet1].isFlagExtra()) {
				_vm->_obj[oCAMPANA4U]._examine = 1202;
				_vm->_obj[oCAMPANA4U]._action = 1203;
			} else
				_vm->_obj[oCAMPANA4U]._examine = 1200;
			printSentence = false;
		}
		break;

	case kItemHammer:
		if ((_vm->_useWith[WITH] == oRAGNO41) && !_vm->_obj[oRAGNO41].isFlagExtra()) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a416, 0, 0, _vm->_useWith[WITH]);
			_vm->_obj[oRAGNO41].setFlagExtra(true);
			_vm->setObjectAnim(oRAGNO41, 0);
			if (_vm->_obj[oSLOT41]._anim == a414)
				_vm->setObjectAnim(oSLOT41, a417);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oSLOT41) || (_vm->_useWith[WITH] == oVETRINETTA42) || (_vm->_useWith[WITH] == oTAMBURO43) || (_vm->_useWith[WITH] == oSFIATO45) || (_vm->_useWith[WITH] == oDOORC4A) || (_vm->_useWith[WITH] == oDOORC4B) || (_vm->_useWith[WITH] == oSERRATURA4B) || (_vm->_useWith[WITH] == oLICANTROPO4P)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1619);
		}
		break;

	case kItemHalfCreditCoins:
		if ((_vm->_useWith[WITH] == oFESSURA42) && (_vm->_obj[oFUCILE42]._anim == a427)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a421, 0, 0, _vm->_useWith[WITH]);
			if (_vm->isObjectVisible(oPOLTIGLIA42))
				_vm->setObjectAnim(oFUCILE42, a429);
			else
				_vm->setObjectAnim(oFUCILE42, a428);
			printSentence = false;
		}
		break;

	case kItemMud:
		if (_vm->_useWith[WITH] == oGUIDE42) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a423, 0, 0, _vm->_useWith[WITH]);
			if (_vm->_obj[oFUCILE42]._anim != a427)
				_vm->setObjectAnim(oFUCILE42, a429);
			printSentence = false;
		}
		break;

	case kItemClub:
		if ((_vm->_useWith[WITH] == oTAMBURO43) && !_vm->_obj[oTAMBURO43].isFlagExtra()) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a435, 0, 0, _vm->_useWith[WITH]);
			_vm->_obj[oTAMBURO43].setFlagExtra(true);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oDOORC4B) || (_vm->_useWith[WITH] == oSERRATURA4B) || (_vm->_useWith[WITH] == oLICANTROPO4P)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1679);
		}
		break;

	case kItemPuppet:
		if (_vm->_useWith[WITH] == oCASSETTOAA44) {
			_vm->replaceIcon(kItemPuppet, kItemPuppetRemoteControl);
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a442, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemAsbestosCloth:
		if (_vm->_useWith[WITH] == oMANOPOLAR45) {
			_vm->changeRoom(kRoom45S, 0, 2);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oCAMPANA4U) && _vm->_inventoryObj[kItemIronBullet1].isFlagExtra() && _vm->_obj[oCAMPANA4U].isFlagExtra()) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U5, 0, 0, _vm->_useWith[WITH]);
			_vm->_obj[oCAMPANA4U]._examine = 1204;
			_vm->_obj[oCAMPANA4U]._action = 1205;
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oCAMPANA4U) {
			_vm->_textMgr->characterSay(1713);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oSPIDER46) || (_vm->_useWith[WITH] == oLICANTROPO4P)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1711);
		}
		break;

	case kItemSkull:
		if (_vm->_useWith[WITH] == oTILE48) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4810, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemTorch:
		if (_vm->_useWith[WITH] == oSKULL48) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4811, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oBOILERS45) || (_vm->_useWith[WITH] == oSPIDER46)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1640);
		}
		break;

	case kItemMatchInPacket:
		if (_vm->_useWith[WITH] == oTORCHS48) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4812, 0, 0, _vm->_useWith[WITH]);
			_vm->_obj[oTORCHS48]._area = Common::Rect(0, 0, 0, 0);
			printSentence = false;
		}
		break;

	case kItemShaft:
		if (_vm->_useWith[WITH] == oSUNDIAL49) {
			_vm->removeIcon(kItemShaft);
			_vm->startCharacterAction(a491, kRoom49M, 1, 0);
			printSentence = false;
		}
		break;

	case kItemLoadedPistol:
		if ((_vm->_useWith[WITH] == oLICANTROPO4P) && _vm->_inventoryObj[kItemLoadedPistol].isFlagExtra()) {
			_vm->replaceIcon(kItemLoadedPistol, kItemPiratePistol);
			_vm->setObjectVisible(oLICANTROPO4P, false);
			_vm->setObjectVisible(oLICANTROPOM4P, true);
			_vm->setObjectVisible(oSANGUE4P, true);
			_vm->_room[kRoom4P].setExtra(true);
			_vm->read3D("4p2.3d"); // after werewolf
			_vm->_animMgr->_animTab[aBKG4P].toggleAnimArea(1, false);
			_vm->_dialogMgr->playDialog(dF4P2);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oLICANTROPO4P) {
			_vm->replaceIcon(kItemLoadedPistol, kItemPiratePistol);
			_vm->_dialogMgr->playDialog(dF4P1);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oSPIDER46) || (_vm->_useWith[WITH] == oDOORC4B) || (_vm->_useWith[WITH] == oSERRATURA4B)) {
			printSentence = false;
			_vm->_textMgr->characterSay(1706);
		}
		break;

	case kItemEmptyGlassJar:
		if ((_vm->_useWith[WITH] == oSANGUE4P) || (_vm->_useWith[WITH] == oLICANTROPOM4P)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4P7, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemBell:
		if (_vm->_useWith[WITH] == oPOZZA4U) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U2, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemIronBullet1:
	case kItemIronBullet2:
		if ((_vm->_useWith[WITH] == oCAMPANA4U) && _vm->_inventoryObj[kItemIronBullet1].isFlagExtra()) {
			_vm->_textMgr->characterSay(1684);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oCAMPANA4U) {
			_vm->removeIcon(_vm->_useWith[USED]);
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U4, 0, 0, _vm->_useWith[WITH]);
			_vm->_inventoryObj[kItemIronBullet1].setFlagExtra(true);
			if (_vm->_obj[oCAMPANA4U].isFlagExtra() && _vm->_inventoryObj[kItemIronBullet1].isFlagExtra()) {
				_vm->_obj[oCAMPANA4U]._examine = 1202;
				_vm->_obj[oCAMPANA4U]._action = 1203;
			} else
				_vm->_obj[oCAMPANA4U]._examine = 1201;
			printSentence = false;
		}
		break;

	case kItemBagWithPapaverine:
		if ((_vm->_useWith[WITH] == oCHOCOLATES4A) && (_vm->_animMgr->smkCurFrame(kSmackerBackground) < 370 || _vm->_animMgr->smkCurFrame(kSmackerBackground) > 480)) {
			_vm->playScript(s4AHELLEN);
			_vm->setObjectAnim(oPULSANTE4A, a4A3);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oCHOCOLATES4A) {
			_vm->setObjectAnim(oPULSANTE4A, a4A3);
			printSentence = false;
			_vm->_obj[oCHOCOLATES4A].setFlagExtra(true);
		}
		break;

	case kItemJarWithWerewolfBlood:
		if (_vm->_useWith[WITH] == oSERRATURA4B) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B4, 0, 0, _vm->_useWith[WITH]);
			_vm->setObjectAnim(oDOORC4B, a4B5);
			printSentence = false;
		}
		break;

	case kItemMeetingRoomSlide1:
		if ((_vm->_useWith[WITH] == oPROIETTORE4B) && (_vm->_obj[oPROIETTORE4B]._anim < a4B9A)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B6A, 0, 0, _vm->_useWith[WITH]);
			_vm->setObjectAnim(oPROIETTORE4B, a4B9A);
			printSentence = false;
		}
		break;

	case kItemMeetingRoomSlide2:
		if ((_vm->_useWith[WITH] == oPROIETTORE4B) && (_vm->_obj[oPROIETTORE4B]._anim < a4B9A)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B6B, 0, 0, _vm->_useWith[WITH]);
			_vm->setObjectAnim(oPROIETTORE4B, a4B9B);
			printSentence = false;
		}
		break;

	case kItemHugsOfficeSlide:
		if ((_vm->_useWith[WITH] == oPROIETTORE4B) && (_vm->_obj[oPROIETTORE4B]._anim < a4B9A)) {
			//
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B6C, 0, 0, _vm->_useWith[WITH]);
			_vm->setObjectAnim(oPROIETTORE4B, a4B9C);
			printSentence = false;
		}
		break;

	case kItemTurtleEgg:
		if ((_vm->_useWith[WITH] == oRETE52) || (_vm->_useWith[WITH] == oSERPENTET52)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a521, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemSnakeHead:
		if (_vm->_useWith[WITH] == oLAGO53) {
			printSentence = false;
			if (!_vm->isObjectVisible(oLUCCHETTO53)) {
				_vm->startCharacterAction(a533, kRoom54, 11, 0);
				_vm->removeIcon(_vm->_useWith[USED]);
			} else if (_vm->_useWith[USED] != kItemSnakeTail) {
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a534, 0, 0, _vm->_useWith[WITH]);
				_vm->removeIcon(_vm->_useWith[USED]);
				_vm->_obj[oLAGO53]._examine = 1237;
			} else
				_vm->_textMgr->characterSay(1740);
		}
		break;

	case kItemSnakeTail:
		if (_vm->_useWith[WITH] == oLAGO53) {
			printSentence = false;
			if (!_vm->isObjectVisible(oLUCCHETTO53)) {
				_vm->startCharacterAction(a533C, kRoom54, 11, 0);
				_vm->removeIcon(_vm->_useWith[USED]);
			} else if (_vm->_useWith[USED] != kItemSnakeTail) {
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a534, 0, 0, _vm->_useWith[WITH]);
				_vm->removeIcon(_vm->_useWith[USED]);
				_vm->_obj[oLAGO53]._examine = 1237;
			} else
				_vm->_textMgr->characterSay(1740);
		}
		break;

	case kItemSoapPowder:
		if (_vm->_useWith[WITH] == oSECCHIOA54) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a543, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemLargeCans:
		if (_vm->_useWith[WITH] == oLAVATRICEF54) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a546, 0, 0, _vm->_useWith[WITH]);
			_vm->setObjectAnim(oLAVATRICEL54, a547);
			printSentence = false;
		}
		break;

	case kItemGuardKeys:
		if (_vm->_useWith[WITH] == oDOORS56) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a563, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oDOOR57C55) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a551, kRoom57, 17, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oDOOR58C55) && (!_vm->_dialogMgr->isDialogFinished(871) || _vm->_dialogMgr->isDialogFinished(901))) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a552, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oDOOR58C55) && _vm->_dialogMgr->isDialogFinished(871)) {
			_vm->_textMgr->characterSay(1287);
			printSentence = false;
		}
		break;

	case kItemVideoRecorder:
		if ((_vm->_useWith[WITH] == oKEYBOARD56) && _vm->_dialogMgr->isDialogFinished(260) && !_vm->_dialogMgr->isDialogFinished(262) && (_vm->_inventoryObj[kItemVideoRecorder]._examine != 1752)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a566, 0, 0, _vm->_useWith[WITH]);
			_vm->_dialogMgr->toggleChoice(262, true);
			_vm->_inventoryObj[kItemVideoRecorder]._examine = 1752;
			printSentence = false;
		} else if ((_vm->_useWith[WITH] == oKEYBOARD56) && (_vm->_inventoryObj[kItemVideoRecorder]._examine == 1752)) {
			_vm->_textMgr->characterSay(1753);
			printSentence = false;
		} else
			printSentence = true;
		break;

	case kItemTester:
		if ((_vm->_useWith[WITH] == oPANNELLOA) && _vm->_dialogMgr->isDialogFinished(856)) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a569, 0, 0, _vm->_useWith[WITH]);
			_vm->_obj[oPANNELLOA].setFlagExtra(true);
			printSentence = false;
		}
		break;

	case kItemParallelCutter:
		if ((_vm->_useWith[WITH] == oPANNELLOA) && _vm->_obj[oPANNELLOA].isFlagExtra()) {
			_vm->_dialogMgr->playDialog(dF562);
			_vm->setObjectVisible(oPANNELLOA, false);
			_vm->setObjectVisible(oCAVOTAGLIATO56, true);
			_vm->setObjectVisible(oDOOR58C55, true);
			_vm->_pathFind->setPosition(6);
			_vm->removeIcon(kItemParallelCutter);
			printSentence = false;
		} else if (_vm->_useWith[WITH] == oPANNELLOA) {
			_vm->_textMgr->characterSay(2012);
			printSentence = false;
		}
		break;

	case kItemSurgicalGloves:
		if (_vm->_useWith[WITH] == oBOMBOLA57) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a575, 0, 0, _vm->_useWith[WITH]);
			printSentence = false;
		}
		break;

	case kItemFloatingPoisonSyringe:
		if ((_vm->_useWith[WITH] == oWINDOWA5A) && _vm->_dialogMgr->isDialogFinished(871) && !_vm->_dialogMgr->isDialogFinished(286)) {
			_vm->removeIcon(kItemFloatingPoisonSyringe);
			_vm->_dialogMgr->playDialog(dC5A1);
			_vm->setObjectAnim(oWINDOWA58, a587);
			printSentence = false;
			_vm->_room[kRoom5A].setExtra(true);
		} else if ((_vm->_useWith[WITH] == oWINDOWA5A) && _vm->_dialogMgr->isDialogFinished(871)) {
			_vm->removeIcon(kItemFloatingPoisonSyringe);
			_vm->_dialogMgr->playDialog(dF5A1);
			_vm->setObjectAnim(oWINDOWA58, a587);
			printSentence = false;
		}
		break;

	default:
		updateInventory = false;
		break;
	}

	if (printSentence)
		_vm->_textMgr->characterSay(_vm->_inventoryObj[_vm->_useWith[USED]]._action);

	if (updateInventory)
		_vm->setInventoryStart(_vm->_iconBase, INVENTORY_SHOW);
}

bool LogicManager::useScreenWithScreen() {
	bool printSentence = true;

	switch (_vm->_useWith[USED]) {
	case oRAMPINO21:
		if (_vm->_useWith[WITH] == oTUBO21) {
			_vm->_animMgr->_animTab[aBKG21].toggleAnimArea(1, true);
			_vm->_dialogMgr->playDialog(dF211);
			_vm->_logicMgr->setupAltRoom(kRoom21, true);
			_vm->setObjectVisible(oRAMPINO21, false);
			_vm->setObjectVisible(oTUBO21, false);
			_vm->setObjectVisible(oCHAIN21, true);
			printSentence = false;
		}
		break;

	case oCAVIE23:
		if (_vm->_useWith[WITH] == oCAMPO23) {
			_vm->changeRoom(kRoom23B);
			printSentence = false;
		} else {
			_vm->startCharacterAction(hBOH, 0, 0, 0);
			printSentence = false;
		}
		break;

	case oTUBOP33:
		if (_vm->_useWith[WITH] == oTUBOF33) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a337PRENDETUBO, 0, 0, _vm->_useWith[WITH]);
			_vm->_obj[oTUBOF34]._examine = 1832;
			_vm->_obj[oTUBOFT34]._examine = 773;
			printSentence = false;
		}
		break;

	case oTUBET33:
		if (((_vm->_useWith[WITH] == oSAMA33) || (_vm->_useWith[WITH] == oSERRATURA33 && _vm->isObjectVisible(oSAMA33))) && _vm->isObjectVisible(oVALVOLA34)) {
			_vm->_dialogMgr->playDialog(dF331);
			_vm->_pathFind->setPosition(10);
			_vm->setObjectVisible(oBOTOLAC33, false);
			_vm->setObjectVisible(oSERRATURA33, false);
			_vm->setObjectVisible(oBOTOLAA33, true);
			_vm->setObjectVisible(oBRUCIATURA33, true);
			_vm->setObjectVisible(oSERRATURAF33, true);
			_vm->setObjectVisible(oSAMD33, true);
			_vm->setObjectVisible(oTUBOS33, true);

			_vm->setObjectVisible(oBOTOLAC32, false);
			_vm->setObjectVisible(oBOTOLAB32, true);
			_vm->setObjectVisible(oBOTOLAA32, true);
			_vm->setObjectAnim(oPULSANTI32, 0);

			_vm->setObjectVisible(oSIGARO31, true);
			_vm->setObjectAnim(oPANNELLOM31, a314APREPANNELLO);
			_vm->_obj[oPANNELLOM31]._examine = 713;
			_vm->_obj[oPANNELLOM31]._action = 714;
			_vm->_obj[oPANNELLOM31].setFlagRoomOut(true);
			_vm->setObjectVisible(oPANNELLON31, true);

			_vm->setObjectVisible(oMANOMETROC34, true);
			_vm->setObjectVisible(oMANOMETRO34, false);
			_vm->_obj[oMANOMETROC34]._examine = 804;
			_vm->setObjectAnim(oVALVOLAC34, 0);
			_vm->setObjectAnim(oVALVOLA34, 0);
			_vm->_obj[oVALVOLAC34]._action = 1831;
			_vm->_obj[oVALVOLA34]._action = 1831;

			_vm->_obj[oTUBOF34]._examine = 1832;
			_vm->_obj[oTUBOFT34]._examine = 784;

			printSentence = false;
		}
		break;

	case oFILOS31:
		if (_vm->_useWith[WITH] == oCONTATTOP31) {
			_vm->setObjectVisible(oFILOS31, false);
			_vm->setObjectVisible(oCONTATTOP31, false);
			_vm->setObjectVisible(oFILOTC31, true);
			_vm->_textMgr->characterSay(746);
			printSentence = false;
		}
		break;
	default:
		break;
	}

	return printSentence;
}

void LogicManager::roomOut(uint16 curObj, uint16 *action, uint16 *pos) {
	*action = 0;
	*pos = 0;
	if (curObj == oSCALA32 && _vm->isObjectVisible(oBOTOLAC32)) {
		_vm->_textMgr->characterSay(_vm->_obj[curObj]._action);
		_vm->_graphicsMgr->showCursor();
	} else if (_vm->isDemo() && curObj == oFINGERPADP16) {
		_vm->demoOver();
	} else {
		*action = _vm->_obj[curObj]._anim;
		*pos = _vm->_obj[curObj]._ninv;
	}
}

bool LogicManager::mouseExamine(uint16 curObj) {
	bool retVal = false;

	switch (curObj) {
	case oMAPPA12:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1211OSSERVAMAPPAPALAZZO, 0, 0, curObj);
		if (!_vm->isDemo()) {
			_vm->_dialogMgr->toggleChoice(4, true);
			_vm->_dialogMgr->toggleChoice(18, true);
			_vm->_dialogMgr->toggleChoice(33, true);
		}
		retVal = false;
		break;

	case oDOORA13:
		_vm->setObjectVisible(oBOX12, true);
		retVal = true;
		break;

	case oPANELA12:
	case oFUSIBILE12:
		_vm->setObjectVisible(oFUSE12CU, _vm->isObjectVisible(oFUSIBILE12));
		_vm->changeRoom(kRoom12CU);
		break;

	case oLETTERA13:
	case oPENPADA13:
		_vm->setObjectVisible(oLETTER13CU, _vm->isObjectVisible(oLETTERA13));
		_vm->changeRoom(kRoom13CU);
		break;

	case oCUCININO14:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1413OSSERVAPIANOCOTTURA, 0, 0, curObj);
		retVal = false;
		break;

	case oSCAFFALE14:
		if (!_vm->isObjectVisible(oDOORR14))
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a145ESAMINASCAFFALE, 0, 0, curObj);
		else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a145CESAMINASCAFFALE, 0, 0, curObj);
		retVal = false;
		break;

	case oTAVOLINOPP14:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1412SPOSTASEDIA, 0, 0, curObj);
		retVal = false;
		break;

	case oBOCCETTE15:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1510TOCCABOCCETTE, 0, 0, curObj);
		retVal = false;
		break;

	case oSPECCHIO15:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1511SISPECCHIA, 0, 0, curObj);
		retVal = false;
		break;

	case oMONITORSA16:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1614GUARDAMONITORS14, 0, 0, curObj);
		retVal = false;
		break;

	case oWINDOWA15:
		if (_vm->isObjectVisible(oTAPPARELLAA15))
			_vm->_textMgr->characterSay(1999);
		else
			retVal = true;
		break;

	case oMONITORSP16:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1615GUARDAMONITORS15, 0, 0, curObj);
		retVal = false;
		break;

	case oCARTACCE16:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1613GUARDACARTACCE, 0, 0, curObj);
		retVal = false;
		break;

	case oMAPPA16:
		if (_vm->iconPos(kItemNoteRotmall17) != -1)
			_vm->_obj[oMAPPA16].setFlagExtra(true);
		retVal = true;
		break;

	case oSCATOLONE17:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a177SICHINA, 0, 0, curObj);
		retVal = false;
		break;

	case oMURALES17:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a179MUOVETESTA, 0, 0, curObj);
		retVal = false;
		break;

	case oSCHERMO18:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a181ESAMINACARTELLONE, 0, 0, curObj);
		retVal = false;
		break;

	case oVETRINA1A:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1A1ESAMINAVETRINA, 0, 0, curObj);
		retVal = false;
		break;

	case oTESSERA1A:
		if ((_vm->_dialogMgr->isDialogFinished(151) || _vm->_dialogMgr->isDialogFinished(152)) && !_vm->_dialogMgr->isDialogFinished(183))
			_vm->_dialogMgr->toggleChoice(183, true);
		_vm->_obj[oTESSERA1A].setFlagExtra(true);
		retVal = true;
		break;

	case oCARTACCE1B:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B8FRUGACARTACCIE, 0, 0, curObj);
		retVal = false;
		break;

	case oBIDONE1B:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B10GUARDABIDONE, 0, 0, curObj);
		retVal = false;
		break;

	case oGRATA1C:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1C2GUARDAGRATA, 0, 0, curObj);
		retVal = false;
		break;

	case oSCAFFALE1D:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1D8SALESGABELLO, 0, 0, curObj);
		retVal = false;
		break;

	case oBARILOTTO1D:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1D7SPINGEBARILOTTO, 0, 0, curObj);
		retVal = false;
		break;

	case oCASSA1D:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1D10ESAMINABOTTIGLIE, 0, 0, curObj);
		retVal = false;
		break;

	case oSCATOLETTA23:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2310, 0, 0, curObj);
		retVal = false;
		break;

	case oPALMA26:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a262, 0, 0, curObj);
		retVal = false;
		break;

	case oINSEGNA26:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a263, 0, 0, curObj);
		retVal = false;
		break;

	case oTEMPIO28:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2810, 0, 0, curObj);
		retVal = false;
		break;

	case oSERPENTET28:
	case oSERPENTEA28:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2811, 0, 0, curObj);
		retVal = false;
		break;

	case oSERPENTE2B:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B11ESAMINASERPENTE, 0, 0, curObj);
		retVal = false;
		break;

	case oLEOPARDO2B:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B9ESAMINALEOPARDO, 0, 0, curObj);
		retVal = false;
		break;

	case oPELLICANO2B:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B10ESAMINAPELLICANO, 0, 0, curObj);
		retVal = false;
		break;

	case oBACHECA2B:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B13OSSERVAREFARFALLE, 0, 0, curObj);
		retVal = false;
		break;

	case oROBOT2F:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2F6TOCCADINOSAURO, 0, 0, curObj);
		retVal = false;
		break;

	case oCRACK2E:
		if (_vm->_room[kRoom2E].hasExtra())
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2E7GUARDACREPACCIODILA, 0, 0, curObj);
		else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2E6GUARDACREPACCIODIQUA, 0, 0, curObj);
		retVal = false;
		break;

	case oGENERATORE34:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a347ESAMINAGENERATORE, 0, 0, curObj);
		retVal = false;
		break;

	case oTUBOF33:
		if (_vm->isObjectVisible(oVALVOLA34) && _vm->_obj[oVALVOLA34]._anim)
			_vm->_textMgr->characterSay(2000);
		else
			_vm->_textMgr->characterSay(_vm->_obj[curObj]._examine);
		retVal = false;
		break;

	case oTUBET33:
		if (_vm->isObjectVisible(oVALVOLA34) && _vm->_obj[oVALVOLA34]._anim)
			_vm->_textMgr->characterSay(2001);
		else
			_vm->_textMgr->characterSay(_vm->_obj[curObj]._examine);
		retVal = false;
		break;

	case oTUBOA34:
		if (_vm->isObjectVisible(oVALVOLA34) && _vm->_obj[oVALVOLA34]._anim)
			_vm->_textMgr->characterSay(2002);
		else
			_vm->_textMgr->characterSay(_vm->_obj[curObj]._examine);
		retVal = false;
		break;

	case oTUBOF34:
		if (_vm->isObjectVisible(oVALVOLA34) && _vm->_obj[oVALVOLA34]._anim)
			_vm->_textMgr->characterSay(2000);
		else
			_vm->_textMgr->characterSay(_vm->_obj[curObj]._examine);
		retVal = false;
		break;

	case oTUBOFT34:
		if (_vm->isObjectVisible(oVALVOLA34) && _vm->_obj[oVALVOLA34]._anim)
			_vm->_textMgr->characterSay(2001);
		else
			_vm->_textMgr->characterSay(_vm->_obj[curObj]._examine);
		retVal = false;
		break;

	case oCASSE35:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a3522ESAMINACASSE, 0, 0, curObj);
		retVal = false;
		break;

	case oSCAFFALE35:
		if (_vm->_room[kRoom35].hasExtra())
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a3517ESAMINACIANFRUSAGLIE, 0, 0, curObj);
		else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a3517AESAMINACIANFRUSAGLIE, 0, 0, curObj);
		retVal = false;
		break;

	case oGIORNALE35:
		if (_vm->_room[kRoom35].hasExtra()) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a3521LEGGEGIORNALE, 0, 0, curObj);
			retVal = false;
		}
		break;

	case oSCAFFALE36:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[oSCAFFALE36]._anim, 0, 0, curObj);
		retVal = false;
		break;

	case oFESSURA41:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a411, 0, 0, curObj);
		retVal = false;
		break;

	case oCARTELLOV42:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a424, 0, 0, curObj);
		retVal = false;
		break;

	case oCARTELLOF42:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a426, 0, 0, curObj);
		retVal = false;
		break;

	case oCAMPANAT43:
		if (_vm->isObjectVisible(oMARTELLOR43))
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a431R, 0, 0, curObj);
		else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a431, 0, 0, curObj);
		retVal = false;
		break;

	case oTAMBURO43:
		if (_vm->isObjectVisible(oMARTELLOR43))
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a432R, 0, 0, curObj);
		else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a432, 0, 0, curObj);
		retVal = false;
		break;

	case oRAGNATELA45:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a451, 0, 0, curObj);
		retVal = false;
		break;

	case oQUADROS4A:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4A5, 0, 0, curObj);
		retVal = false;
		break;

	case oCARTELLO55:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a5511, 0, 0, curObj);
		retVal = false;
		break;

	case oEXIT12CU:
	case oEXIT13CU:
	case oEXIT2BL:
	case oEXIT36F:
	case oEXIT41D:
	case oEXIT4CT:
	case oEXIT58M:
	case oEXIT59L:
		_vm->changeRoom(_vm->_obj[curObj]._goRoom);
		break;

	case oEXIT58T:
		_count58 = 0;
		for (int i = 0; i < 6; ++i)
			_vm->setObjectVisible(oLED158 + i, false);
		_vm->changeRoom(_vm->_obj[oEXIT58T]._goRoom);
		break;

	default:
		retVal = true;
		break;
	}

	return retVal;
}

bool LogicManager::mouseOperate(uint16 curObj) {
	bool retVal = false;

	if (!curObj)
		warning("doMouseOperate");

	switch (curObj) {
	case oFAX17:
		if (_vm->_obj[oSCALA16]._anim) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
			_vm->_inventoryObj[kItemMinicom]._action = 1416;
			retVal = false;
		}
		break;

	case ocPOLIZIOTTO16:
	case oSCALA16:
		if (!_vm->_obj[oSCALA16].isFlagExtra()) {
			_vm->_obj[oSCALA16].setFlagExtra(true);
			_vm->_dialogMgr->toggleChoice(61, true);
			_vm->_dialogMgr->playDialog(dPOLIZIOTTO16);
			_vm->_obj[oSCALA16]._action = 166;
			_vm->_obj[ocPOLIZIOTTO16]._action = 166;
		} else
			retVal = true;
		break;

	case oPANNELLOC12:
		if (_vm->_obj[oPANNELLOC12].isFlagExtra()) {
			if (_vm->isObjectVisible(oASCENSOREC12))
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a128RIUSABOTTONE, 0, 0, curObj);
			else
				_vm->_textMgr->characterSay(24);
		} else {
			_vm->_obj[oPANNELLOC12].setFlagExtra(true);
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		}
		break;
	case oPANNELLO13:
		if (_vm->isObjectVisible(oASCENSOREA13)) {
			_vm->_textMgr->characterSay(48);
			retVal = false;
		} else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);

		break;

	case oPANNELLO16:
		if (_vm->isObjectVisible(oASCENSOREA16)) {
			_vm->_textMgr->characterSay(48);
			retVal = false;
		} else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		break;

	case oLATTINA13:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a132PRENDELATTINA, 0, 0, curObj);
		_vm->addIcon(kItemBeerCan);
		break;

	case oDOORA13:
		if (_vm->_room[kRoom14].isDone()) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1312METTELETTERARICALCA, kRoom14, 14, _vm->_useWith[WITH]);
			retVal = false;
		} else
			retVal = true;
		break;

	case oCESTINO14:
		if (_vm->_obj[curObj]._anim) {
			if (!_vm->isObjectVisible(oDOORR14))
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
			else
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a143CPRENDECREDITCARD, 0, 0, curObj);
			_vm->setObjectAnim(curObj, 0);
		} else
			retVal = true;
		break;
	case oTASTOB15:
		if (_vm->isObjectVisible(oTAPPARELLAA15)) {
			_vm->_textMgr->characterSay(_vm->_obj[oTASTOB15]._action);
			retVal = false;
		} else {
			if (!_vm->_obj[oNASTRO15].isFlagExtra())
				_vm->setObjectVisible(oNASTRO15, true);
			if (_vm->_obj[curObj]._anim)
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
			retVal = false;
		}
		break;

	case oTASTOA15:
		if (!_vm->isObjectVisible(oTAPPARELLAA15)) {
			_vm->_textMgr->characterSay(_vm->_obj[oTASTOA15]._action);
			retVal = false;
		} else {
			_vm->setObjectVisible(oNASTRO15, false);
			if (_vm->_obj[curObj]._anim)
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
			retVal = false;
		}
		break;

	case oFINGERPADP16:
		retVal = true;
		break;

	case oDOORC18:
		if (_vm->_obj[ocGUARD18].isFlagPerson()) {
			_vm->_dialogMgr->toggleChoice(151, false);
			_vm->_dialogMgr->toggleChoice(152, true);
			_vm->_dialogMgr->playDialog(dGUARDIAN18);
			_vm->_obj[ocGUARD18].setFlagPerson(false);
			_vm->_obj[ocGUARD18]._action = 227;
			_vm->_obj[oDOORC18]._action = 220;
		} else
			retVal = true;
		break;

	case oGRATA1C:
		if (_vm->_obj[oFAX17].isFlagExtra())
			_vm->changeRoom(kRoom21, 0, 10);
		else
			retVal = true;
		break;

	case oBOTOLAC1B:
		if ((_vm->_obj[oBOTOLAC1B]._anim == a1B3APREBOTOLA) && _vm->isObjectVisible(oTOMBINOA1B))
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B3AAPREBOTOLA, 0, 0, curObj);
		else if (_vm->_obj[oBOTOLAC1B]._anim == a1B3APREBOTOLA)
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B3APREBOTOLA, 0, 0, curObj);
		else
			retVal = true;
		break;

	case oARMADIETTORC22:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		_vm->setObjectAnim(oMANIGLIONE22, a227A);
		_vm->setObjectAnim(oMANIGLIONEO22, a229A);
		_vm->setObjectAnim(od22TO29, a2214A);
		_vm->setObjectAnim(od22TO29I, a2215A);
		break;

	case oARMADIETTORA22:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		_vm->setObjectAnim(oMANIGLIONE22, a227);
		_vm->setObjectAnim(oMANIGLIONEO22, a229);
		_vm->setObjectAnim(od22TO29, a2214);
		_vm->setObjectAnim(od22TO29I, a2215);
		break;

	case oCHAIN21:
		if ((_vm->iconPos(kItemMagneticBar) != -1) && (_vm->_dialogMgr->isDialogFinished(436) || _vm->_dialogMgr->isDialogFinished(466))) {
			if (_vm->_room[_vm->_curRoom].hasExtra()) // Go right
				_vm->_dialogMgr->playDialog(dF212);                             // 436
			else                                                 // Go left
				_vm->_dialogMgr->playDialog(dF213);                             // 466
			retVal = false;
		} else if (_vm->_dialogMgr->isDialogFinished(451) || _vm->_dialogMgr->isDialogFinished(481)) {
			if (_vm->_room[_vm->_curRoom].hasExtra()) // Go right
				_vm->_dialogMgr->playDialog(dF212B);                            // 451
			else                                                 // Go left
				_vm->_dialogMgr->playDialog(dF213B);                            // 481
			retVal = false;
		} else
			retVal = true;
		break;

	case oPULSANTEACS2D:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		_vm->setObjectAnim(oPULSANTEBC2D, a2D7SCHIACCIATASTO6V);
		break;

	case oPULSANTEACA2D:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		_vm->setObjectAnim(oPULSANTEBC2D, a2D7SCHIACCIATASTO6R);
		break;

	case oSWITCH29:
		if (_vm->_curRoom == kRoom29L)
			retVal = true;
		else if (_vm->isObjectVisible(oLAMPADINAS29)) {
			_vm->changeRoom(kRoom29L);
			retVal = false;
		} else if (!_vm->_obj[_vm->_curObj].isFlagExtra()) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a291USAINTERRUTTORELUCE, 0, 0, curObj);
			_vm->_obj[_vm->_curObj].setFlagExtra(true);
			retVal = false;
		} else
			retVal = true;
		break;

	case oLEVAS23:
		if (_vm->isObjectVisible(oCAVI23))
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a238, 0, 0, curObj);
		else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a234, 0, 0, curObj);
		break;

	case oLEVAG23:
		if (_vm->isObjectVisible(oCAVIE23))
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a239, 0, 0, curObj);
		else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a235, 0, 0, curObj);
		break;

	case oBOTOLAC25:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		_vm->setObjectAnim(oTRONCHESE25, a254B);
		break;

	case oBOTOLAA25:
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		_vm->setObjectAnim(oTRONCHESE25, a254);
		break;

	case oPASSAGE24:
		if (_vm->_room[kRoom24].hasExtra()) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a244, 0, 14, curObj);
			_vm->_logicMgr->setupAltRoom(kRoom24, false);
			retVal = false;
		} else {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a243, 0, 13, curObj);
			_vm->_logicMgr->setupAltRoom(kRoom24, true);
			retVal = false;
		}
		break;

	case oDOOR26:
		if (_vm->_obj[curObj]._anim)
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 4, curObj);
		break;

	case oRUBINETTOC28:
		if (_vm->_obj[curObj]._anim) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
			_vm->_obj[oBRACIERES28]._examine = 455;
			_vm->_obj[oBRACIERES28].setFlagExtra(true);
		} else
			retVal = true;
		break;

	case oEXIT12CU:
	case oEXIT13CU:
	case oEXIT2BL:
	case oEXIT36F:
	case oEXIT41D:
	case oEXIT4CT:
	case oEXIT58M:
	case oEXIT59L:
		_vm->changeRoom(_vm->_obj[curObj]._goRoom);
		break;

	case oEXIT58T:
		_count58 = 0;
		for (uint8 i = 0; i < 6; ++i)
			_vm->setObjectVisible(oLED158 + i, false);
		_vm->changeRoom(_vm->_obj[oEXIT58T]._goRoom);
		break;

	case oPANELM2G:
		if (!_vm->_obj[oPANELM2G].isFlagExtra()) {
			_vm->_animMgr->_animTab[aBKG2G].toggleAnimArea(1, false);
			_vm->_dialogMgr->playDialog(dF2G1);
			_vm->setObjectVisible(oCOPERCHIO2G, false);
			_vm->setObjectVisible(oSERBATOIOC2G, false);
			_vm->setObjectVisible(oRAGAZZOP2G, false);
			_vm->setObjectVisible(oRAGAZZOS2G, true);
			_vm->setObjectVisible(oSERBATOIOA2G, true);
			_vm->setObjectVisible(oPANNELLOE2G, true);
			_vm->_obj[oPANELM2G].setFlagExtra(true);
			retVal = false;
		} else
			retVal = true;

		break;

	case oWHEELS2C:
		if (!_vm->isObjectVisible(od2CTO2D)) {
			_vm->_animMgr->smkStop(kSmackerBackground);
			_vm->_animMgr->_animTab[aBKG2C].toggleAnimArea(1, false);
			_vm->setObjectVisible(oBASEWHEELS2C, true);
			_vm->setObjectVisible(omWHEELS2C, true);
			_vm->setObjectVisible(oPULSANTE2C, true);
			_vm->setObjectVisible(_wheelPos[0] * 3 + 0 + oWHEEL1A2C, true);
			_vm->setObjectVisible(_wheelPos[1] * 3 + 1 + oWHEEL1A2C, true);
			_vm->setObjectVisible(_wheelPos[2] * 3 + 2 + oWHEEL1A2C, true);
			_vm->setObjectVisible(oCAMPO2C, false);
			_vm->setObjectVisible(oTEMPIO2C, false);
			_vm->setObjectVisible(oLEONE2C, false);
			_vm->setObjectVisible(od2CTO2D, false);
			_vm->setObjectVisible(oSFINGE2C, false);
			_vm->setObjectVisible(oSTATUA2C, false);
			_vm->setObjectVisible(oWHEELS2C, false);
			_vm->setObjectVisible(od2CTO2E, false);
			_vm->setObjectVisible(oCARTELLOS2C, false);
			_vm->setObjectVisible(oCARTELLOA2C, false);
			_vm->setObjectVisible(od2CTO26, false);
			_vm->_flagShowCharacter = false;
			_vm->readExtraObj2C();
		} else
			retVal = true;
		break;

	case oCATWALKA2E:
		if (!_vm->_obj[oCATWALKA2E].isFlagExtra()) {
			_vm->_animMgr->_animTab[aBKG2E].toggleAnimArea(2, true);
			_vm->_dialogMgr->playDialog(dF2E1);
			_vm->setObjectVisible(oDINOSAURO2E, true);
			_vm->_obj[oCATWALKA2E].setFlagExtra(true);
			retVal = false;
		} else if (_vm->_obj[curObj]._anim) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
			retVal = false;
		} else
			retVal = true;
		break;

	case oBORSA22:
	case oPORTALAMPADE2B:
	case oMAPPAMONDO2B:
		if (_vm->_obj[curObj]._anim) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
			_vm->setObjectAnim(curObj, 0);
		} else
			retVal = true;
		break;

	case oTUBOF34:
		if (!_vm->isObjectVisible(oTUBOFT34))
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		else
			retVal = true;
		break;

	case oFILOT31:
		_vm->setObjectVisible(oFILOT31, false);
		_vm->setObjectVisible(oFILOS31, true);
		break;

	case oCOPERCHIOA31:
		if (_vm->isObjectVisible(oFILOTC31)) {
			_vm->_soundMgr->play(wCOVER31);
			_vm->setObjectVisible(oPANNELLOM31, false);
			_vm->setObjectVisible(oPANNELLOMA31, true);
			_vm->setObjectAnim(oPANNELLOM31, 0);
			_vm->_obj[oPANNELLOM31]._examine = 715;
			_vm->_obj[oPANNELLOM31]._action = 716;
			_vm->_obj[oPANNELLOM31].setFlagRoomOut(false);
			_vm->changeRoom(kRoom31, a3118CHIUDEPANNELLO, 3);
		} else
			retVal = true;
		break;

	case oVALVOLAC34:
	case oVALVOLA34:
		if (_vm->_obj[curObj]._anim)
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		else
			retVal = true;
		break;

	case oPROIETTORE35:
		_vm->_room[kRoom35].setExtra(true);
		_vm->read3D("352.3d"); // After the shock

		_vm->setObjectVisible(oRIBELLEA35, false);
		_vm->setObjectVisible(oDOORC35, false);
		_vm->setObjectVisible(omDOORC35, false);
		_vm->setObjectVisible(oPRESA35, false);

		_vm->setObjectVisible(oDOORA35, true);
		_vm->setObjectVisible(omDOORA35, true);
		_vm->setObjectVisible(oRIBELLES35, true);
		_vm->setObjectVisible(oSEDIA35, true);
		_vm->setObjectVisible(oMONITOR35, true);
		_vm->setObjectVisible(omPIANO35, true);

		_vm->setObjectAnim(oFRONTOFFICEC35, a356PROVASPORTELLO);
		_vm->_obj[oASCENSORE35].setFlagRoomOut(true);
		_vm->setObjectAnim(oASCENSORE35, a3514ENTRAASCENSORE);

		_vm->_animMgr->_animTab[aBKG35].toggleAnimArea(1, false);
		_vm->_dialogMgr->playDialog(dF351);
		_vm->_pathFind->setPosition(7);
		break;

	case oCOMPUTER36:
		if (!_vm->_dialogMgr->isDialogFinished(646)) {
			_vm->_dialogMgr->playDialog(dF361);
			_vm->_obj[oCOMPUTER36]._action = 2004;
			retVal = false;
		} else
			retVal = true;
		break;

	case oPULSANTEA35:
	case oPULSANTEB35:
	case oPULSANTEC35:
	case oPULSANTED35:
	case oPULSANTEE35:
	case oPULSANTEF35:
	case oPULSANTEG35:
		_vm->setObjectVisible(curObj, false);
		_vm->setObjectVisible(curObj + 7, true);
		_comb35[_count35++] = curObj;
		_vm->_soundMgr->play(wPAD5);
		if (_count35 == 7) {
			if (((_comb35[0] == oPULSANTEF35) && (_comb35[1] == oPULSANTED35) && (_comb35[2] == oPULSANTEC35) &&
				 (_comb35[3] == oPULSANTEG35) && (_comb35[4] == oPULSANTEB35) && (_comb35[5] == oPULSANTEA35) &&
				 (_comb35[6] == oPULSANTEE35)) ||
				((_comb35[0] == oPULSANTEE35) &&
				 (_comb35[1] == oPULSANTEA35) && (_comb35[2] == oPULSANTEB35) && (_comb35[3] == oPULSANTEG35) &&
				 (_comb35[4] == oPULSANTEC35) && (_comb35[5] == oPULSANTED35) && (_comb35[6] == oPULSANTEF35))) {
				_vm->setObjectVisible(oPULSANTEAA35, false);
				_vm->setObjectVisible(oPULSANTEBA35, false);
				_vm->setObjectVisible(oPULSANTECA35, false);
				_vm->setObjectVisible(oPULSANTEDA35, false);
				_vm->setObjectVisible(oPULSANTEEA35, false);
				_vm->setObjectVisible(oPULSANTEFA35, false);
				_vm->setObjectVisible(oPULSANTEGA35, false);
				_vm->setObjectVisible(oPULSANTIV35, true);
				_vm->setObjectVisible(oLEDS35, false);

				_vm->setObjectAnim(oFRONTOFFICEA35, 0);
				_vm->_obj[oFRONTOFFICEA35]._action = 1844;
				_vm->_obj[oFRONTOFFICEA35].setFlagExtra(true);
				_vm->_obj[oDOORMC36].setFlagRoomOut(true);
				_vm->setObjectAnim(oDOORMC36, a3610APREPORTA);
				_vm->setObjectAnim(oSCAFFALE36, a3615APRENDESCAFFALE);

				_vm->_animMgr->_animTab[aBKG36].toggleAnimArea(2, false);
				_vm->_animMgr->_animTab[aBKG36].toggleAnimArea(3, false);
				_vm->setObjectVisible(oSCANNERLA36, false);
				_vm->setObjectVisible(oSCANNERLS36, true);
				_vm->setObjectVisible(oSCANNERMA36, false);
				_vm->setObjectVisible(oSCANNERMS36, true);

				_vm->_soundMgr->play(wWIN35);
			} else {
				_vm->setObjectVisible(oPULSANTEA35, true);
				_vm->setObjectVisible(oPULSANTEB35, true);
				_vm->setObjectVisible(oPULSANTEC35, true);
				_vm->setObjectVisible(oPULSANTED35, true);
				_vm->setObjectVisible(oPULSANTEE35, true);
				_vm->setObjectVisible(oPULSANTEF35, true);
				_vm->setObjectVisible(oPULSANTEG35, true);
				_vm->setObjectVisible(oPULSANTEAA35, false);
				_vm->setObjectVisible(oPULSANTEBA35, false);
				_vm->setObjectVisible(oPULSANTECA35, false);
				_vm->setObjectVisible(oPULSANTEDA35, false);
				_vm->setObjectVisible(oPULSANTEEA35, false);
				_vm->setObjectVisible(oPULSANTEFA35, false);
				_vm->setObjectVisible(oPULSANTEGA35, false);
			}
			_count35 = 0;
			_comb35[0] = 0;
			_comb35[1] = 0;
			_comb35[2] = 0;
			_comb35[3] = 0;
			_comb35[4] = 0;
			_comb35[5] = 0;
			_comb35[6] = 0;
		}
		_vm->_curObj += 7;
		break;

	case oCARD35:
		_vm->setObjectVisible(oPULSANTEA35, true);
		_vm->setObjectVisible(oPULSANTEB35, true);
		_vm->setObjectVisible(oPULSANTEC35, true);
		_vm->setObjectVisible(oPULSANTED35, true);
		_vm->setObjectVisible(oPULSANTEE35, true);
		_vm->setObjectVisible(oPULSANTEF35, true);
		_vm->setObjectVisible(oPULSANTEG35, true);
		_vm->setObjectVisible(oPULSANTEAA35, false);
		_vm->setObjectVisible(oPULSANTEBA35, false);
		_vm->setObjectVisible(oPULSANTECA35, false);
		_vm->setObjectVisible(oPULSANTEDA35, false);
		_vm->setObjectVisible(oPULSANTEEA35, false);
		_vm->setObjectVisible(oPULSANTEFA35, false);
		_vm->setObjectVisible(oPULSANTEGA35, false);
		_vm->addIcon(kItemSecurityCard);

		_vm->changeRoom(kRoom35, a359RITIRACARD, 6);
		break;

	case oSCAFFALE36:
		retVal = true;
		break;

	case oFOROC49:
	case oFORO849:
	case oFORO949:
	case oFORO1049:
	case oFORO1149:
	case oFORO1249:
	case oFORO149:
	case oFORO249:
	case oFORO349:
	case oFORO449:
	case oFORO549:
	case oFORO649:
	case oFORO749:
		for (int i = oASTAC49; i <= oASTA749; ++i)
			_vm->setObjectVisible(i, false);
		_vm->setObjectVisible(oASTAC49 + curObj - oFOROC49, true);
		_comb49[3] = _comb49[2];
		_comb49[2] = _comb49[1];
		_comb49[1] = _comb49[0];
		_comb49[0] = curObj;
		_vm->_soundMgr->play(wASTA49);
		if ((_comb49[3] == oFORO749) && (_comb49[2] == oFORO849) && (_comb49[1] == oFORO449) && (_comb49[0] == oFORO549)) {
			_vm->_graphicsMgr->paintScreen(false);
			_vm->waitDelay(60);
			_vm->setObjectVisible(oOMBRAS49, true);
			_vm->setObjectVisible(oSCOMPARTO49, true);
			_vm->setObjectVisible(oAGENDA49, true);
			_vm->setObjectVisible(oSUNDIAL49, false);
			_vm->_obj[oAGENDA49]._examine = 1099;
			_vm->_obj[oAGENDA49]._action = 1100;
			_vm->_flagShowCharacter = true;
			_vm->_curObj = oAGENDA49;
			_vm->playScript(s49SUNDIAL);
		}
		retVal = false;
		break;

	case oASTAC49:
	case oASTA849:
	case oASTA949:
	case oASTA1049:
	case oASTA1149:
	case oASTA1249:
	case oASTA149:
	case oASTA249:
	case oASTA349:
	case oASTA449:
	case oASTA549:
	case oASTA649:
	case oASTA749:
		for (int i = oASTAC49; i <= oASTA749; ++i)
			_vm->setObjectVisible(i, false);
		_vm->setObjectVisible(oASTAC49, true);
		_vm->changeRoom(kRoom49, a496, 1);
		retVal = false;
		break;

	case oNUMERO14C:
	case oNUMERO24C:
	case oNUMERO34C:
	case oNUMERO44C:
	case oNUMERO54C:
	case oNUMERO64C:
	case oNUMERO74C:
	case oNUMERO84C:
	case oNUMERO94C:
	case oNUMERO04C: {
		int a;
		for (a = 0; a < 6; ++a) {
			if (_comb4CT[a] == 0) {
				_vm->setObjectVisible(a + oAST14C, true);
				_comb4CT[a] = curObj - oNUMERO14C + 1;
				break;
			}
		}
		_vm->_soundMgr->play(wPAD1 + curObj - oNUMERO14C);
		if (a < 5)
			break;
		_vm->_graphicsMgr->paintScreen(false);
		_vm->waitDelay(60);
		if ((_comb4CT[0] == 5) && (_comb4CT[1] == 6) && (_comb4CT[2] == 2) &&
			(_comb4CT[3] == 3) && (_comb4CT[4] == 9) && (_comb4CT[5] == 6)) {
			for (a = 0; a < 6; ++a) {
				_comb4CT[a] = 0;
				_vm->setObjectVisible(oAST14C + a, false);
			}
			_vm->changeRoom(kRoom51, 0, 1);
			_vm->_flagShowCharacter = true;
		} else {
			for (a = 0; a < 6; ++a) {
				_comb4CT[a] = 0;
				_vm->setObjectVisible(oAST14C + a, false);
			}
			_vm->changeRoom(kRoom4C, 0, 4);
			_vm->_flagShowCharacter = true;
		}
		retVal = false;
		break;
		}
	case oDOORC4A:
		if (!_vm->_dialogMgr->isDialogFinished(245) && !_vm->_dialogMgr->isDialogFinished(766)) {
			_vm->_dialogMgr->toggleChoice(245, true);
			_vm->_dialogMgr->playDialog(dC4A1);
			_vm->_pathFind->setPosition(14);
			_vm->_obj[oDOORC4A]._action = 1117;
			_vm->setObjectAnim(oDOORC4A, 0);
			retVal = false;
		} else
			retVal = true;
		break;

	case oPULSANTE4A:
		if (_vm->_obj[curObj]._anim)
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		else if (!_vm->_dialogMgr->isDialogFinished(244)) {
			_vm->_dialogMgr->toggleChoice(244, true);
			_vm->_dialogMgr->playDialog(dC4A1);
			_vm->_obj[oPULSANTE4A]._examine = 1108;
			_vm->_obj[oPULSANTE4A]._action = 1109;
			retVal = false;
		} else
			retVal = true;
		break;

	case oWINDOW56:
		if (_vm->isObjectVisible(oPANNELLOC56))
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a5614, 0, 0, curObj);
		else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a5614P, 0, 0, curObj);
		break;

	case oPULSANTECD:
		for (int i = oPULSANTE1AD; i <= oPULSANTE33AD; ++i) {
			if ((_vm->_obj[i]._goRoom == _vm->_obj[oEXIT41D]._goRoom) ||
				((_vm->_obj[i]._goRoom == kRoom45) && (_vm->_obj[oEXIT41D]._goRoom == kRoom45S))) {
				_vm->_textMgr->characterSay(903);
				break;
			}

			if (_vm->_obj[i]._goRoom == 0) {
				if (_vm->_obj[oEXIT41D]._goRoom == kRoom45S)
					_vm->_obj[i]._goRoom = kRoom45;
				else
					_vm->_obj[i]._goRoom = _vm->_obj[oEXIT41D]._goRoom;
				_vm->setObjectVisible(i, true);
				_vm->setObjectVisible(i - 40, false);
				break;
			}
		}
		retVal = false;
		break;

	case oDOORC54:
		if (!_vm->_dialogMgr->isDialogFinished(826)) {
			_vm->_dialogMgr->playDialog(dF541);
			retVal = false;
		} else
			retVal = true;
		break;

	case oLAVATRICEL54:
		if (_vm->_obj[curObj]._anim && _vm->_obj[oSECCHIOS54].isFlagExtra() && _vm->isObjectVisible(oGRATAC54) && !_vm->_dialogMgr->isDialogFinished(841))
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		else
			retVal = true;
		break;

	case oSECCHIOS54:
		if (_vm->isObjectVisible(oGRATAC54))
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a544G, 0, 0, curObj);
		else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		retVal = false;
		break;

	case oKEYBOARD56:
		if (_vm->_dialogMgr->isDialogFinished(262)) {
			if (_vm->isObjectVisible(od56TO59))
				_vm->_textMgr->characterSay(_vm->_obj[curObj]._action);
			else
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a567, 0, 0, curObj);
			retVal = false;
		} else {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a564, 0, 0, curObj);
			retVal = false;
		}
		break;

	case oLIBRIEG2B:
		if (_vm->_room[kRoom2C].isDone()) { // If room 2C has been visited before, take the book
			retVal = false;
			if (_vm->_obj[curObj]._anim)
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B4TAKEBOOK, 0, 0, curObj);
			else
				retVal = true;
		} else { // If room 2C hasn't been visited before, he tells it's useless
			_vm->_textMgr->characterSay(2014);
			retVal = false;
		}
		break;

	case oKEYBOARD58:
		_vm->changeRoom(kRoom58T);
		break;

	case oLAVAGNA59:
	case oSIMBOLI59:
		_vm->changeRoom(kRoom59L);
		break;

	case oWINDOWA5A:
		if (_vm->_dialogMgr->isDialogFinished(871) && !_vm->_dialogMgr->isDialogFinished(286)) {
			_vm->_dialogMgr->playDialog(dC5A1);
			retVal = false;
		} else
			retVal = true;
		break;

	case oWINDOWB5A:
		if (!_vm->_dialogMgr->isDialogFinished(256)) {
			_vm->_textMgr->characterSay(1999);
			retVal = false;
		} else
			retVal = true;
		break;

	case oTASTO158:
	case oTASTO258:
	case oTASTO358:
	case oTASTO458:
	case oTASTO558:
	case oTASTO658:
	case oTASTO758:
	case oTASTO858:
	case oTASTO958:
	case oTASTO058:
		retVal = false;
		_comb58[5] = _comb58[4];
		_comb58[4] = _comb58[3];
		_comb58[3] = _comb58[2];
		_comb58[2] = _comb58[1];
		_comb58[1] = _comb58[0];
		_comb58[0] = curObj;

		_vm->_soundMgr->play(wPAD1 + curObj - oTASTO158);
		_vm->setObjectVisible(oLED158 + _count58, true);
		++_count58;
		if (_count58 < 6)
			break;

		_vm->_graphicsMgr->paintScreen(false);
		_vm->waitDelay(60);
		_count58 = 0;
		for (int i = 0; i < 6; ++i)
			_vm->setObjectVisible(oLED158 + i, false);

		if ((_comb58[0] == oTASTO058) && (_comb58[1] == oTASTO258) && (_comb58[2] == oTASTO358) &&
			(_comb58[3] == oTASTO858) && (_comb58[4] == oTASTO558) && (_comb58[5] == oTASTO958)) {
			_vm->_soundMgr->stopAllExceptMusic();
			_vm->_dialogMgr->playDialog(dF582);
		} else
			_vm->changeRoom(kRoom58, 0, 2);

		for (int i = 0; i < 6; ++i)
			_comb58[i] = 0;
		break;

	default:
		if (_vm->_obj[curObj]._anim)
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, _vm->_obj[curObj]._anim, 0, 0, curObj);
		else
			retVal = true;
		break;
	}

	return retVal;
}

bool LogicManager::mouseTake(uint16 curObj) {
	bool retVal;

	switch (curObj) {
	case oTINFOIL11:
		retVal = false;
		break;
	case oNASTRO15:
		_vm->_obj[oNASTRO15].setFlagExtra(true);
		retVal = false;
		break;
	case oMONETA13:
		if (!_vm->isObjectVisible(oLATTINA13))
			_vm->setObjectAnim(curObj, a133CPRENDEMONETA);
		retVal = true;
		break;
	case oFOGLIETTO14:
		_vm->_obj[oFOGLIETTO14].setFlagExtra(true);
		_vm->_obj[oMAPPA16]._examine = 152;
		retVal = false;
		break;
	case oPOSTERC22:
		_vm->setObjectAnim(oARMADIETTOCC22, a221);
		_vm->setObjectAnim(oARMADIETTOCA22, a222);
		retVal = true;
		break;
	case oKEY22:
		_vm->_obj[oARMADIETTORA22]._examine = 2013;
		retVal = true;
		break;
	default:
		retVal = true;
		break;
	}

	return retVal;
}

bool LogicManager::mouseTalk(uint16 curObj) {
	bool retVal = true;

	switch (curObj) {
	case oTICKETOFFICE16:
		if (_vm->_obj[oFINGERPADP16].isFlagRoomOut() && _vm->_dialogMgr->isDialogFinished(50)) {
			_vm->_textMgr->characterSay(147);
			retVal = false;
			break;
		}

		if (!_vm->_dialogMgr->isChoiceVisible(49) && !_vm->_dialogMgr->isChoiceVisible(50)) {
			if (_vm->_obj[oMAPPA16].isFlagExtra()) {
				_vm->_dialogMgr->toggleChoice(46, true);
				_vm->_dialogMgr->toggleChoice(48, true);
				_vm->_obj[oTICKETOFFICE16].setFlagExtra(true);
			} else {
				if (_vm->_dialogMgr->isDialogFinished(46)) {
					_vm->_textMgr->characterSay(_vm->_obj[oTICKETOFFICE16]._action);
					retVal = false;
					break;
				}

				_vm->_dialogMgr->toggleChoice(46, true);
				_vm->_dialogMgr->toggleChoice(47, true);
			}
		}
		break;

	case ocGUARD18:
		_vm->_obj[ocGUARD18].setFlagPerson(false);
		_vm->_obj[ocGUARD18]._action = 227;
		_vm->_obj[oDOORC18]._action = 220;
		break;

	case ocNEGOZIANTE1A:
		if (!_vm->_dialogMgr->handleShopKeeperDialog(curObj))
			return false;

		if (_vm->_obj[ocNEGOZIANTE1A]._action) {
			_vm->_textMgr->characterSay(_vm->_obj[ocNEGOZIANTE1A]._action);
			retVal = false;
		}
		break;

	case ocEVA19:
		_vm->_inventoryObj[kItemMinicom]._action = 1415;
		break;

	default:
		break;
	}

	return retVal;
}

bool LogicManager::mouseClick(uint16 curObj) {
	bool retVal;
	if (_vm->_curRoom == kRoom1D && !_vm->_room[kRoom1D].hasExtra() && (curObj != oSCALA1D)) {
		_vm->_curObj = oDONNA1D;
		_vm->_pathFind->goToPosition(_vm->_obj[oDONNA1D]._position);
		retVal = true;
	} else if (_vm->_curRoom == kRoom2B && _vm->_room[kRoom2B].hasExtra() && (curObj != oCARTELLO2B) && (curObj != od2BTO28)) {
		_vm->_curObj = oDOOR2B;
		_vm->_pathFind->goToPosition(_vm->_obj[oCARTELLO2B]._position);
		retVal = true;
	} else {
		switch (curObj) {
		case oDOORA13:
			if (_vm->_mouseRightBtn && _vm->_room[kRoom14].isDone()) {
				if (_vm->_pathFind->_characterGoToPosition != 4)
					_vm->_pathFind->goToPosition(4);
				_vm->_mouseRightBtn = false;
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oDIVANO14:
			if (_vm->_mouseLeftBtn) {
				if (_vm->_pathFind->_characterGoToPosition != 2)
					_vm->_pathFind->goToPosition(2);
				_vm->_mouseLeftBtn = false;
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oSCAFFALE1D:
			if (_vm->_mouseRightBtn) {
				if (_vm->_pathFind->_characterGoToPosition != 9)
					_vm->_pathFind->goToPosition(9);
				_vm->_mouseRightBtn = false;
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oDIVANOR4A:
			if (_vm->_mouseRightBtn) {
				if (_vm->_pathFind->_characterGoToPosition != 1)
					_vm->_pathFind->goToPosition(1);
				_vm->_mouseRightBtn = false;
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oDOORC21:
			if ((_vm->_useWith[USED] == kItemPistol) && _vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 1)
					_vm->_pathFind->goToPosition(1);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oTUBO21:
			if ((_vm->_useWith[USED] == oRAMPINO21) && _vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 4)
					_vm->_pathFind->goToPosition(4);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oCAMPO23:
			if ((_vm->_useWith[USED] == oCAVIE23) && _vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 2)
					_vm->_pathFind->goToPosition(2);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oTASTO2F:
			if ((_vm->_useWith[USED] == kItemElevatorRemoteControl) && _vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 9)
					_vm->_pathFind->goToPosition(9);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oSAMA33:
		case oSERRATURA33:
			if ((_vm->_useWith[USED] == oTUBET33) && _vm->isObjectVisible(oVALVOLA34)
			&& _vm->isObjectVisible(oSAMA33) && _vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 4)
					_vm->_pathFind->goToPosition(4);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oWINDOW33:
			if (_vm->_mouseLeftBtn) {
				if (_vm->_pathFind->_characterGoToPosition != 7)
					_vm->_pathFind->goToPosition(7);
				_vm->_mouseLeftBtn = false;
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oPRESA35:
			if (!_vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 2)
					_vm->_pathFind->goToPosition(2);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oTRIPLA35:
			if (!_vm->_flagUseWithStarted && _vm->_mouseLeftBtn) {
				if (_vm->_pathFind->_characterGoToPosition != 2)
					_vm->_pathFind->goToPosition(2);
				_vm->_mouseLeftBtn = false;
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oPORTALC36:
			if ((_vm->_useWith[USED] == kItemStethoscope) && _vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 8)
					_vm->_pathFind->goToPosition(8);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oSERBATOIOA2G:
			if ((_vm->_useWith[USED] == kItemFlare) && _vm->_inventoryObj[kItemGasCylinder].isFlagExtra() && _vm->_inventoryObj[kItemCanWithFuel].isFlagExtra() && _vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 6)
					_vm->_pathFind->goToPosition(6);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oRAGNO41:
			if ((_vm->_useWith[USED] == kItemHammer) && _vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 1)
					_vm->_pathFind->goToPosition(1);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oTAMBURO43:
			if ((_vm->_useWith[USED] == kItemClub) && _vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 5)
					_vm->_pathFind->goToPosition(5);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oLUCCHETTO53:
			if ((_vm->_useWith[USED] == kItemLaserGun) && _vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 2)
					_vm->_pathFind->goToPosition(2);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		case oDOOR58C55:
			if ((_vm->_useWith[USED] == kItemGuardKeys) && _vm->_flagUseWithStarted) {
				if (_vm->_pathFind->_characterGoToPosition != 2)
					_vm->_pathFind->goToPosition(2);
			} else if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
				_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);

			retVal = true;
			break;

		default:
			if (_vm->_obj[curObj]._position == -1) {
				_vm->_pathFind->_characterGoToPosition = -1;
				retVal = false;
			} else {
				if (_vm->_pathFind->_characterGoToPosition != _vm->_obj[curObj]._position)
					_vm->_pathFind->goToPosition(_vm->_obj[curObj]._position);
				retVal = true;
			}
			break;
		}

		if (_vm->_room[_vm->_curRoom].hasExtra()) {
			if ((curObj == oTUBO21) || (curObj == oBILLBOARD21) || (curObj == oESSE21)
			|| (curObj == oRAMPINO21) || (curObj == oCATENA21) || (curObj == od21TO22)
			|| (curObj == oDOORC21) || (curObj == oDOORA21) || (curObj == oCUNICLE21) || (curObj == od24TO23) || (curObj == od2ETO2C) || (curObj == od2GVTO26)) {
				_vm->_pathFind->_characterGoToPosition = -1;
				retVal = true;
			}
		} else if ((curObj == od21TO23) || (curObj == od24TO26) || (curObj == oENTRANCE2E)
		|| (curObj == oCARTELLO2B) || (curObj == oFRONTOFFICEC35) || (curObj == oFRONTOFFICEA35)
		|| (curObj == oASCENSORE35) || (curObj == oGIORNALE35)) {
			_vm->_pathFind->_characterGoToPosition = -1;
			retVal = true;
		}
	}
	return retVal;
}

bool LogicManager::operateInventory() {
	bool printSentence = true;

	switch (_vm->_curInventory) {
	case kItemMinicom:
		if (_vm->_dialogMgr->isDialogFinished(166) && (_vm->_curRoom == kRoom17 || _vm->_curRoom == kRoom1B || _vm->_curRoom == kRoom1C || _vm->_curRoom == kRoom1D)) {
			if (_vm->isObjectVisible(oNUMERO17)) {
				if (!_vm->_dialogMgr->isDialogFinished(196)) {
					if (_vm->_curRoom == kRoom17) {
						if (_vm->_dialogMgr->isDialogFinished(198)) {
							_vm->_inventoryObj[kItemMinicom]._action = 1787;
							printSentence = true;
						} else {
							_vm->_dialogMgr->toggleChoice(197, false);
							_vm->_dialogMgr->toggleChoice(198, true);
							_vm->_dialogMgr->playDialog(dSAM17);
							printSentence = false;
							_vm->setObjectVisible(oFAX17, true);
						}
					} else {
						if (_vm->_dialogMgr->isDialogFinished(199)) {
							_vm->_inventoryObj[kItemMinicom]._action = 1787;
							printSentence = true;
						} else {
							_vm->_dialogMgr->toggleChoice(197, false);
							_vm->_dialogMgr->toggleChoice(199, true);
							_vm->_dialogMgr->playDialog(dSAM17);
							printSentence = false;
							_vm->setObjectVisible(oFAX17, true);
						}
					}
				} else {
					if (_vm->_dialogMgr->isDialogFinished(198) || _vm->_dialogMgr->isDialogFinished(199)) {
						_vm->_inventoryObj[kItemMinicom]._action = 1787;
						printSentence = true;
					} else if (_vm->_curRoom == kRoom17) {
						if (_vm->_dialogMgr->isDialogFinished(201)) {
							_vm->_inventoryObj[kItemMinicom]._action = 1787;
							printSentence = true;
						} else {
							_vm->_dialogMgr->toggleChoice(201, true);
							_vm->_dialogMgr->playDialog(dSAM17);
							printSentence = false;
							_vm->setObjectVisible(oFAX17, true);
						}
					} else {
						if (_vm->_dialogMgr->isDialogFinished(200)) {
							_vm->_inventoryObj[kItemMinicom]._action = 1787;
							printSentence = true;
						} else {
							_vm->_dialogMgr->toggleChoice(200, true);
							_vm->_dialogMgr->playDialog(dSAM17);
							printSentence = false;
							_vm->setObjectVisible(oFAX17, true);
						}
					}
				}
			} else if (_vm->_dialogMgr->isDialogFinished(197)) {
				_vm->_inventoryObj[kItemMinicom]._action = 1786;
				printSentence = true;
			} else {
				_vm->_dialogMgr->playDialog(dSAM17);
				printSentence = false;
			}
		}
		break;

	case kItemSubwayMap:
		if (_vm->_curRoom == kRoom23A) {
			_vm->_textMgr->characterSay(361);
			printSentence = false;
		} else
			printSentence = true;
		break;

	case kItemEgyptologyBook:
		_vm->_obj[oEXIT2BL]._goRoom = _vm->_curRoom;
		_vm->changeRoom(kRoom2BL);
		_vm->_actor->actorStop();
		_vm->_pathFind->nextStep();
		printSentence = false;
		break;

	case kItemSecuritySystemSequence:
		_vm->_obj[oEXIT36F]._goRoom = _vm->_curRoom;
		_vm->changeRoom(kRoom36F);
		_vm->_actor->actorStop();
		_vm->_pathFind->nextStep();
		printSentence = false;
		break;

	case kItemPositioner:
		for (int i = oROOM41; i <= oROOM45B; ++i)
			_vm->setObjectVisible(i, false);
		_vm->_obj[oEXIT41D]._goRoom = _vm->_curRoom;
		_vm->changeRoom(kRoom41D);
		_vm->_inventoryObj[kItemPositioner].setFlagExtra(false);
		_vm->_actor->actorStop();
		_vm->_pathFind->nextStep();
		printSentence = false;
		break;

	case kItemGovernorsCode:
		_vm->_obj[oEXIT58M]._goRoom = _vm->_curRoom;
		_vm->changeRoom(kRoom58M);
		_vm->_actor->actorStop();
		_vm->_pathFind->nextStep();
		printSentence = false;
		break;

	case kItemPuppetRemoteControl:
		if ((_vm->_actor->_px < 5057.6) && _vm->isObjectVisible(oPUPAZZO44) && _vm->_curRoom == kRoom43) {
			_vm->changeRoom(kRoom46, 0, 7);
			printSentence = false;
		}
		break;

	case kItemPrisonMap:
		if ((_vm->_curRoom >= kRoom51) && (_vm->_curRoom <= kRoom5A))
			_vm->_inventoryObj[kItemPrisonMap]._action = 1725 + (_vm->_curRoom - kRoom51);
		printSentence = true;
		break;
	default:
		break;
	}

	return printSentence;
}

void LogicManager::doMouseGame() {
	// For the wheel in 2C
	if ((_vm->_curObj >= oWHEEL1A2C) && (_vm->_curObj <= oWHEEL12C2C))
		_vm->_textMgr->showObjName((oWHEEL1A2C % 3) + oWHEELA2C, true);
	// For the displacer
	else if (_vm->_curRoom == kRoom41D) {
		const uint16 displacerRoom = oROOM41 + _vm->_obj[_vm->_curObj]._goRoom - kRoom41;
		if ((_vm->_curObj >= oPULSANTE1AD) && (_vm->_curObj <= oPULSANTE33AD)) {
			if (!_vm->isObjectVisible(displacerRoom)) {
				for (int i = oROOM41; i <= oROOM4X; ++i) {
					if (_vm->isObjectVisible(i))
						_vm->setObjectVisible(i, false);
				}
				_vm->setObjectVisible(oROOM45B, false);

				if (displacerRoom == oROOM45 && _vm->_obj[od44TO45]._goRoom == kRoom45S)
					_vm->setObjectVisible(oROOM45B, true);
				else
					_vm->setObjectVisible(displacerRoom, true);
			}
		} else {
			for (int i = oROOM41; i <= oROOM4X; ++i) {
				if (_vm->isObjectVisible(i))
					_vm->setObjectVisible(i, false);
			}
			_vm->setObjectVisible(oROOM45B, false);

		}
		_vm->_textMgr->showObjName(_vm->_curObj, true);
	} else
		// not a wheel nor the displacer
		_vm->_textMgr->showObjName(_vm->_curObj, true);

	if (_vm->_inventoryStatus == INV_INACTION)
		_vm->closeInventory();
}

// Returns true when it's in a room without a character, such as the map
bool LogicManager::doMouseInventory() {
	return !_vm->_flagShowCharacter && _vm->_curRoom != kRoom31P && _vm->_curRoom != kRoom35P;
}

void LogicManager::handleClickSphinxPuzzle() {
	if (_vm->checkMask(_vm->_mousePos)) {
		if ((_vm->_curObj >= oWHEEL1A2C) && (_vm->_curObj <= oWHEEL12C2C))
			_wheel = (_vm->_curObj - oWHEEL1A2C) % 3;
		else if (_vm->_curObj == oPULSANTE2C) {
			if (_vm->_curMessage->_event == ME_MLEFT) {
				_vm->_scheduler->mouseExamine(_vm->_curObj);
				return;
			}
			_vm->_animMgr->_animTab[aBKG2C].toggleAnimArea(1, true);
			_vm->setObjectVisible(oBASEWHEELS2C, false);
			_vm->setObjectVisible(omWHEELS2C, false);
			_vm->setObjectVisible(oPULSANTE2C, false);
			_vm->setObjectVisible(_wheelPos[0] * 3 + 0 + oWHEEL1A2C, false);
			_vm->setObjectVisible(_wheelPos[1] * 3 + 1 + oWHEEL1A2C, false);
			_vm->setObjectVisible(_wheelPos[2] * 3 + 2 + oWHEEL1A2C, false);
			_vm->setObjectVisible(oCAMPO2C, true);
			_vm->setObjectVisible(oTEMPIO2C, true);
			_vm->setObjectVisible(oLEONE2C, true);
			_vm->setObjectVisible(oSFINGE2C, true);
			_vm->setObjectVisible(oSTATUA2C, true);
			_vm->setObjectVisible(od2CTO2E, true);
			_vm->setObjectVisible(oCARTELLOA2C, true);
			_vm->setObjectVisible(od2CTO26, true);
			_vm->setObjectVisible(oWHEELS2C, true);
			_vm->_flagShowCharacter = true;
			_vm->_animMgr->startSmkAnim(_vm->_room[_vm->_curRoom]._bkgAnim);

			// right combination
			if ((_wheelPos[0] == 7) && (_wheelPos[1] == 5) && (_wheelPos[2] == 11)) {
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2C6PREMEPULSANTEAPERTURA, 0, 0, _vm->_curObj);
				_vm->_obj[oSFINGE2C].setFlagPerson(false);
			} else
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2C6PREMEPULSANTE, 0, 0, _vm->_curObj);

			return;
		} else
			return;

		if (_vm->_curMessage->_event == ME_MLEFT)
			_wheelPos[_wheel] = (_wheelPos[_wheel] > 10) ? 0 : _wheelPos[_wheel] + 1;
		if (_vm->_curMessage->_event == ME_MRIGHT)
			_wheelPos[_wheel] = (_wheelPos[_wheel] < 1) ? 11 : _wheelPos[_wheel] - 1;

		_vm->_soundMgr->play(wWHEELS2C);
		_vm->setObjectVisible(_vm->_curObj, false);
		_vm->setObjectVisible(_wheelPos[_wheel] * 3 + _wheel + oWHEEL1A2C, true);
	}
}

void LogicManager::handleClickPositioner() {
	if ((_vm->_obj[_vm->_curObj]._goRoom == kRoom45) && (_vm->_obj[od44TO45]._goRoom == kRoom45S) &&
		(_vm->_obj[oEXIT41D]._goRoom == kRoom45S) && (_vm->_curMessage->_event == ME_MRIGHT))
		_vm->_scheduler->mouseOperate(_vm->_curObj);
	else if ((_vm->_obj[_vm->_curObj]._goRoom == kRoom45) && (_vm->_obj[od44TO45]._goRoom == kRoom45S) &&
			 (_vm->_obj[oEXIT41D]._goRoom != kRoom45S) && (_vm->_curMessage->_event == ME_MRIGHT)) {
		_vm->_obj[oEXIT41D]._goRoom = kRoom45S;
		_vm->_inventoryObj[kItemPositioner].setFlagExtra(true);
		_vm->changeRoom(kRoom45S);
	} else if (_vm->_obj[oEXIT41D]._goRoom != _vm->_obj[_vm->_curObj]._goRoom && (_vm->_curMessage->_event == ME_MRIGHT)) {
		_vm->_obj[oEXIT41D]._goRoom = _vm->_obj[_vm->_curObj]._goRoom;
		_vm->_inventoryObj[kItemPositioner].setFlagExtra(true);
		_vm->changeRoom(_vm->_obj[oEXIT41D]._goRoom);
	} else if ((_vm->_curMessage->_event == ME_MLEFT) && _vm->_curObj)
		_vm->_scheduler->mouseExamine(_vm->_curObj);
	else if ((_vm->_curMessage->_event == ME_MRIGHT) && _vm->_curObj)
		_vm->_scheduler->mouseOperate(_vm->_curObj);
}

void LogicManager::handleClickSnakeEscape() {
	if (_vm->isObjectVisible(oSNAKEU52) && _vm->isGameArea(_vm->_mousePos) && !_vm->_flagUseWithStarted && _vm->_curObj != oSNAKEU52) {
		_vm->startCharacterAction(a526, 0, 1, 0);
		_vm->setObjectAnim(oSCAVO51, a516);
		_vm->_snake52.set(_vm->_curMessage);
	}
}

// Handles rooms without a character, like maps or books
void LogicManager::handleClickCloseup() {
	if (_vm->isInventoryArea(_vm->_mousePos) && (_vm->_curRoom == kRoom31P || _vm->_curRoom == kRoom35P)) {
		if (_vm->isIconArea(_vm->_mousePos) && _vm->whatIcon(_vm->_mousePos) && (_vm->_inventoryStatus == INV_INACTION)) {
			_vm->_useWith[WITH] = 0;
			_vm->_curObj = 0;
			_vm->_lightIcon = 0xFF;
			_vm->setInventoryStart(_vm->_iconBase, INVENTORY_SHOW);
			if (_vm->_curMessage->_event == ME_MRIGHT || _vm->_flagUseWithStarted)
				_vm->useItem();
			else
				_vm->examineItem();
		}
		return;
	}

	if ((_vm->_curMessage->_event == ME_MLEFT) && _vm->_curObj)
		_vm->_scheduler->mouseExamine(_vm->_curObj);
	else if ((_vm->_curMessage->_event == ME_MRIGHT) && _vm->_curObj)
		_vm->_scheduler->mouseOperate(_vm->_curObj);
}

void LogicManager::handleClickGameArea() {
	if (_vm->_flagScriptActive)
		_vm->_curObj = _vm->_curMessage->_u32Param;

	int pmousex = _vm->_curMessage->_u16Param1;
	int pmousey = _vm->_curMessage->_u16Param2;
	if (!_vm->_logicMgr->mouseClick(_vm->_curObj)) {
		if (_vm->checkMask(_vm->_mousePos)) {
			if ((_vm->_obj[_vm->_curObj]._area.right - _vm->_obj[_vm->_curObj]._area.left) < MAXX / 7) {
				pmousex = (_vm->_obj[_vm->_curObj]._area.left + _vm->_obj[_vm->_curObj]._area.right) / 2;
				pmousey = ((_vm->_obj[_vm->_curObj]._area.top + _vm->_obj[_vm->_curObj]._area.bottom) / 2) + TOP;
			}
		}
		_vm->_pathFind->whereIs(pmousex, pmousey);
		_vm->_pathFind->findPath();
	}
	_vm->_scheduler->initCharacterQueue();

	if (_vm->checkMask(_vm->_mousePos) && !_vm->_flagDialogActive) {
		if (_vm->_curRoom == kRoom1D && !_vm->_room[kRoom1D].hasExtra() && (_vm->_curObj != oSCALA1D))
			_vm->_curObj = oDONNA1D;
		else if (_vm->_curRoom == kRoom2B && _vm->_room[kRoom2B].hasExtra() && (_vm->_curObj != oCARTELLO2B) && (_vm->_curObj != od2BTO28)) {
			_vm->_textMgr->clearLastText();
			_vm->_curObj = oDOOR2B;
			_vm->startCharacterAction(a2B1PROVAAPRIREPORTA, 0, 0, 0);
			_vm->clearUseWith();
			return;
		} else if (_vm->_curRoom == kRoom35 && !_vm->_room[kRoom35].hasExtra() && ((_vm->_curObj == oFRONTOFFICEC35) || (_vm->_curObj == oFRONTOFFICEA35) || (_vm->_curObj == oASCENSORE35) || (_vm->_curObj == oMONITOR35) || (_vm->_curObj == oSEDIA35) || (_vm->_curObj == oRIBELLEA35) || (_vm->_curObj == oCOMPUTER35) || (_vm->_curObj == oGIORNALE35))) {
			_vm->_curObj = oLASTLEV5;
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERGOTOEXAMINE, MP_DEFAULT, _vm->_curMessage->_u16Param1, _vm->_curMessage->_u16Param2, 0, _vm->_curObj);
			_vm->clearUseWith();
			return;
		} else if ((_vm->_curMessage->_event == ME_MLEFT) &&
				   ((!_vm->_room[_vm->_curRoom].hasExtra() && ((_vm->_curObj == oENTRANCE2E) || (_vm->_curObj == od24TO26) || (_vm->_curObj == od21TO23 && !_vm->_obj[_vm->_curObj].isFlagExamine()))) ||
					(_vm->_room[_vm->_curRoom].hasExtra() && ((_vm->_curObj == od2ETO2C) || (_vm->_curObj == od24TO23) || (_vm->_curObj == od21TO22 && !_vm->_obj[_vm->_curObj].isFlagExamine()) || (_vm->_curObj == od2GVTO26))))) {
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERGOTO, MP_DEFAULT, _vm->_curMessage->_u16Param1, _vm->_curMessage->_u16Param2, 0, 0);
			return;
		}

		if (_vm->_curMessage->_event == ME_MRIGHT) {
			if (!_vm->_obj[_vm->_curObj].isFlagExamine() && (_vm->_curObj != 0)) {
				if (_vm->_flagUseWithStarted) {
					_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERGOTO, MP_DEFAULT, _vm->_curMessage->_u16Param1, _vm->_curMessage->_u16Param2, 0, 0);
					return;
				}
				if (_vm->_obj[_vm->_curObj].isFlagRoomIn())
					_vm->changeRoom(_vm->_obj[_vm->_curObj]._goRoom, _vm->_obj[_vm->_curObj]._anim, _vm->_obj[_vm->_curObj]._ninv);
				else if (_vm->_obj[_vm->_curObj].isFlagRoomOut())
					_vm->changeRoom(_vm->_obj[_vm->_curObj]._goRoom, 0, _vm->_obj[_vm->_curObj]._ninv);
				_vm->_actor->actorStop();
				_vm->_pathFind->nextStep();
				_vm->_obj[_vm->_curObj].setFlagDone(true);
			} else if (_vm->_obj[_vm->_curObj].isFlagUseWith()) {
				_vm->_pathFind->_characterGoToPosition = -1;
				_vm->_actor->actorStop();
				_vm->_pathFind->nextStep();
				_vm->_scheduler->mouseOperate(_vm->_curObj);
			} else
				_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERGOTOACTION, MP_DEFAULT, _vm->_curMessage->_u16Param1, _vm->_curMessage->_u16Param2, 0, _vm->_curObj);
		} else
			_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERGOTOEXAMINE, MP_DEFAULT, _vm->_curMessage->_u16Param1, _vm->_curMessage->_u16Param2, 0, _vm->_curObj);
	} else
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERGOTO, MP_DEFAULT, _vm->_curMessage->_u16Param1, _vm->_curMessage->_u16Param2, 0, 0);
}

void LogicManager::handleClickInventoryArea() {
	if (_vm->_animMgr->isActionActive() || _vm->_flagDialogActive || _vm->_curRoom == kRoomControlPanel)
		return;

	if (_vm->isIconArea(_vm->_mousePos) && _vm->whatIcon(_vm->_mousePos) && (_vm->_inventoryStatus == INV_INACTION)) {
		_vm->_scheduler->initCharacterQueue();
		_vm->_actor->actorStop();
		_vm->_pathFind->nextStep();
		_vm->_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERGOTOACTION, MP_DEFAULT, _vm->_curMessage->_u16Param1, _vm->_curMessage->_u16Param2, 0, 0);
		_vm->_useWith[WITH] = 0;
		_vm->_curObj = 0;
		_vm->_lightIcon = 0xFF;
		_vm->setInventoryStart(_vm->_iconBase, INVENTORY_SHOW);
		if (_vm->_curMessage->_event == ME_MRIGHT || _vm->_flagUseWithStarted)
			_vm->useItem();
		else
			_vm->examineItem();
	}
}

void LogicManager::doMouseLeftRight() {
	if (_vm->_curObj >= oPULSANTE1AD && _vm->_curObj <= oPULSANTE33AD) {
		handleClickPositioner();
	} else if (_vm->isObjectVisible(oBASEWHEELS2C) && _vm->_curRoom == kRoom2C) {
		handleClickSphinxPuzzle();
	} else if (_vm->_curRoom == kRoomControlPanel) {
		handleClickControlPanel(_vm->_curObj);
	} else if (!_vm->_flagShowCharacter) {
		handleClickCloseup();
	} else if (_vm->isGameArea(_vm->_mousePos) && !_vm->_animMgr->isActionActive()) {
		if (_vm->_curRoom == kRoom52)
			handleClickSnakeEscape();

		handleClickGameArea();
	} else if (_vm->isInventoryArea(_vm->_mousePos)) {
		handleClickInventoryArea();
	}
}

void LogicManager::initControlPanel() {
	const bool speechON = !ConfMan.getBool("speech_mute");
	const bool textON = ConfMan.getBool("subtitles");
	const int speechVol = ConfMan.getInt("speech_volume");
	const int musicVol = ConfMan.getInt("music_volume");
	const int sfxVol = ConfMan.getInt("sfx_volume");
	const uint16 speechObjId = o00SPEECH1D + (speechVol / 51) * 2;
	const uint16 musicObjId = o00MUSIC1D + (musicVol / 51) * 2;
	const uint16 sfxObjId = o00SOUND1D + (sfxVol / 51) * 2;

	if (speechON)
		_vm->setObjectVisible(o00SPEECHON, true);
	else
		_vm->setObjectVisible(o00SPEECHOFF, true);

	if (textON)
		_vm->setObjectVisible(o00TEXTON, true);
	else
		_vm->setObjectVisible(o00TEXTOFF, true);

	_vm->setObjectVisible(speechObjId, true);
	_vm->setObjectVisible(musicObjId, true);
	_vm->setObjectVisible(sfxObjId, true);

	if (speechVol < 256)
		_vm->setObjectVisible(speechObjId + 1, true);
	if (musicVol < 256)
		_vm->setObjectVisible(musicObjId + 1, true);
	if (sfxVol < 256)
		_vm->setObjectVisible(sfxObjId + 1, true);
}

void LogicManager::doSystemChangeRoom(uint16 room) {
	if (_vm->_curRoom == kRoom41D && _vm->_oldRoom != room)
		_vm->_graphicsMgr->dissolve();

	_vm->_oldRoom = _vm->_curRoom;
	_vm->_curRoom = room;
	_vm->_scheduler->resetQueues();

	_vm->clearUseWith();
	_vm->closeInventoryImmediately();
	_vm->_mouseLeftBtn = _vm->_mouseRightBtn = false;

	_vm->_flagShowCharacter = true;
	_vm->_flagCharacterSpeak = false;
	_vm->_flagSomeoneSpeaks = false;
	_vm->_actor->actorStop();
	_vm->_pathFind->nextStep();

	// Handle exit velocity in dual rooms level 2
	if (_vm->_room[_vm->_oldRoom].hasExtra()) {
		switch (_vm->_curObj) {
		case od2ETO2C:
			_vm->_logicMgr->setupAltRoom(kRoom2E, false);
			break;
		case od24TO23:
			_vm->_logicMgr->setupAltRoom(kRoom24, false);
			break;
		case od21TO22:
			_vm->_logicMgr->setupAltRoom(kRoom21, false);
			break;
		case od2GVTO26:
			_vm->_logicMgr->setupAltRoom(kRoom2GV, false);
			break;
		default:
			break;
		}
	} else {
		switch (_vm->_curObj) {
		case oENTRANCE2E:
			_vm->_logicMgr->setupAltRoom(kRoom2E, true);
			break;
		case od24TO26:
			_vm->_logicMgr->setupAltRoom(kRoom24, true);
			break;
		case od21TO23:
			_vm->_logicMgr->setupAltRoom(kRoom21, true);
			break;
		default:
			break;
		}
	}

	if (_vm->_curRoom == kRoom12 && _vm->_oldRoom == kRoom11)
		_vm->_animMgr->_animTab[aBKG11].toggleAnimArea(1, false);
	else if (_vm->_oldRoom == kRoom2BL || _vm->_oldRoom == kRoom36F)
		_vm->_oldRoom = _vm->_curRoom;
	else if (_vm->_curRoom == kRoomControlPanel)
		initControlPanel();

	_vm->readLoc();
	_vm->_graphicsMgr->showCursor();

	if (_vm->_curRoom == kRoom21) {
		switch (_vm->_oldRoom) {
		case kRoom22:
			_vm->_logicMgr->setupAltRoom(kRoom21, false);
			break;
		case kRoom23A:
		case kRoom23B:
			_vm->_logicMgr->setupAltRoom(kRoom21, true);
			break;
		}
	} else if (_vm->_curRoom == kRoom24) {
		switch (_vm->_oldRoom) {
		case kRoom23A:
		case kRoom23B:
			_vm->_logicMgr->setupAltRoom(kRoom24, false);
			break;
		case kRoom26:
			_vm->_logicMgr->setupAltRoom(kRoom24, true);
			break;
		}
	} else if (_vm->_curRoom == kRoom2A) {
		switch (_vm->_oldRoom) {
		case kRoom25:
			_vm->_logicMgr->setupAltRoom(kRoom2A, true);
			break;
		case kRoom2B:
		case kRoom29:
		case kRoom29L:
			_vm->_logicMgr->setupAltRoom(kRoom2A, false);
			break;
		}
	} else if (_vm->_curRoom == kRoom2B) {
		switch (_vm->_oldRoom) {
		case kRoom28:
			_vm->_logicMgr->setupAltRoom(kRoom2B, true);
			break;
		case kRoom2A:
			_vm->_logicMgr->setupAltRoom(kRoom2B, false);
			break;
		}
	} else if (_vm->_room[_vm->_curRoom].hasExtra()) {
		// for save/load
		switch (_vm->_curRoom) {
		case kRoom15: _vm->read3D("152.3d"); break;
		case kRoom17: _vm->read3D("172.3d"); break;
		case kRoom1D: _vm->read3D("1d2.3d"); break;
		case kRoom21: _vm->read3D("212.3d"); break;
		case kRoom24: _vm->read3D("242.3d"); break;
		case kRoom28: _vm->read3D("282.3d"); break;
		case kRoom2A: _vm->read3D("2A2.3d"); break;
		case kRoom2B: _vm->read3D("2B2.3d"); break;
		case kRoom2E: _vm->read3D("2E2.3d"); break;
		case kRoom2GV: _vm->read3D("2GV2.3d"); break;
		case kRoom35: _vm->read3D("352.3d"); break;
		case kRoom37: _vm->read3D("372.3d"); break;
		case kRoom4P: _vm->read3D("4P2.3d"); break;
		default:
			break;
		}
	}
}

void LogicManager::handleClickControlPanel(uint16 curObj) {
	_vm->checkMask(_vm->_mousePos);

	switch (curObj) {
	case o00QUIT:
		if (_vm->quitPrompt())
			_vm->quitGame();
		break;

	case o00EXIT:
		if (_vm->_oldRoom == kRoomControlPanel)
			break;
		_vm->changeRoom(_vm->_obj[o00EXIT]._goRoom);
		break;

	case o00SAVE:
		if (_vm->_oldRoom == kRoomControlPanel)
			break;
		_vm->_curRoom = _vm->_obj[o00EXIT]._goRoom;
		_vm->dataSave();
		_vm->showInventoryName(NO_OBJECTS, false);
		_vm->showIconName();
		_vm->changeRoom(_vm->_obj[o00EXIT]._goRoom);
		break;

	case o00LOAD:
		if (!_vm->dataLoad()) {
			_vm->showInventoryName(NO_OBJECTS, false);
			_vm->showIconName();
		}
		break;

	case o00SPEECHON:
		if (ConfMan.getBool("subtitles")) {
			_vm->setObjectVisible(o00SPEECHON, false);
			_vm->setObjectVisible(o00SPEECHOFF, true);
			ConfMan.setBool("speech_mute", true);
			_vm->_curObj = o00SPEECHOFF;
			_vm->_textMgr->showObjName(_vm->_curObj, true);
		}
		break;

	case o00SPEECHOFF:
		_vm->setObjectVisible(o00SPEECHOFF, false);
		_vm->setObjectVisible(o00SPEECHON, true);
		ConfMan.setBool("speech_mute", false);
		_vm->_curObj = o00SPEECHON;
		_vm->_textMgr->showObjName(_vm->_curObj, true);
		break;

	case o00TEXTON:
		if (!ConfMan.getBool("speech_mute")) {
			_vm->setObjectVisible(o00TEXTON, false);
			_vm->setObjectVisible(o00TEXTOFF, true);
			ConfMan.setBool("subtitles", false);
			_vm->_curObj = o00TEXTOFF;
			_vm->_textMgr->showObjName(_vm->_curObj, true);
		}
		break;

	case o00TEXTOFF:
		_vm->setObjectVisible(o00TEXTOFF, false);
		_vm->setObjectVisible(o00TEXTON, true);
		ConfMan.setBool("subtitles", true);
		_vm->_curObj = o00TEXTON;
		_vm->_textMgr->showObjName(_vm->_curObj, true);
		break;

	case o00SPEECH1D:
	case o00SPEECH2D:
	case o00SPEECH3D:
	case o00SPEECH4D:
	case o00SPEECH5D:
	case o00SPEECH6D:
	case o00MUSIC1D:
	case o00MUSIC2D:
	case o00MUSIC3D:
	case o00MUSIC4D:
	case o00MUSIC5D:
	case o00MUSIC6D:
	case o00SOUND1D:
	case o00SOUND2D:
	case o00SOUND3D:
	case o00SOUND4D:
	case o00SOUND5D:
	case o00SOUND6D:
		_vm->setObjectVisible(curObj, false);
		if ((curObj != o00SPEECH6D) && (curObj != o00MUSIC6D) && (curObj != o00SOUND6D))
			_vm->setObjectVisible(curObj + 1, false);
		_vm->setObjectVisible(curObj - 1, true);
		_vm->setObjectVisible(curObj - 2, true);
		if (curObj < o00MUSIC1D)
			ConfMan.setInt("speech_volume", ((curObj - 2 - o00SPEECH1D) / 2) * 51);
		else if (curObj > o00MUSIC6D)
			ConfMan.setInt("sfx_volume", ((curObj - 2 - o00SOUND1D) / 2) * 51);
		else
			ConfMan.setInt("music_volume", ((curObj - 2 - o00MUSIC1D) / 2) * 51);
		break;

	case o00SPEECH1U:
	case o00SPEECH2U:
	case o00SPEECH3U:
	case o00SPEECH4U:
	case o00SPEECH5U:
	case o00MUSIC1U:
	case o00MUSIC2U:
	case o00MUSIC3U:
	case o00MUSIC4U:
	case o00MUSIC5U:
	case o00SOUND1U:
	case o00SOUND2U:
	case o00SOUND3U:
	case o00SOUND4U:
	case o00SOUND5U:
		_vm->setObjectVisible(curObj, false);
		_vm->setObjectVisible(curObj - 1, false);
		_vm->setObjectVisible(curObj + 1, true);
		if ((curObj != o00SPEECH5U) && (curObj != o00MUSIC5U) && (curObj != o00SOUND5U))
			_vm->setObjectVisible(curObj + 2, true);
		if (curObj < o00MUSIC1D)
			ConfMan.setInt("speech_volume", ((curObj + 1 - o00SPEECH1D) / 2) * 51);
		else if (curObj > o00MUSIC6D)
			ConfMan.setInt("sfx_volume", ((curObj + 1 - o00SOUND1D) / 2) * 51);
		else
			ConfMan.setInt("music_volume", ((curObj + 1 - o00MUSIC1D) / 2) * 51);
		break;
	default:
		break;
	}

	_vm->syncSoundSettings();
	ConfMan.flushToDisk();
}

bool LogicManager::isCloseupOrControlRoom() const {
	const uint16 curRoom = _vm->_curRoom;
	return curRoom == kRoom2BL ||
		   curRoom == kRoom36F ||
		   curRoom == kRoom41D ||
		   curRoom == kRoom49M ||
		   curRoom == kRoom4CT ||
		   curRoom == kRoom58T ||
		   curRoom == kRoom58M ||
		   curRoom == kRoom59L ||
		   curRoom == kRoomControlPanel ||
		   curRoom == kRoom12CU ||
		   curRoom == kRoom13CU;
}

} // End of namespace Trecision
