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
#include "dragons/cursor.h"
#include "dragons/actor.h"
#include "dragons/actorresource.h"
#include "dragons/dragons.h"
#include "dragons/dragonimg.h"
#include "dragons/dragonini.h"
#include "dragons/dragonobd.h"
#include "dragons/inventory.h"
#include "dragons/scene.h"
#include "dragons/scriptopcodes.h"
#include "dragons/screen.h"

namespace Dragons {

Cursor::Cursor(DragonsEngine *vm): _vm(vm), _actor(0), _x(0), _y(0) {
	_sequenceID = 0;
	_data_800728b0_cursor_seqID = 0;
	_iniUnderCursor = 0;
	_performActionTargetINI = 0;
	_objectInHandSequenceID = 0;
	_cursorActivationSeqOffset = 0;
	_iniItemInHand = 0;
	_handPointerSequenceID = _vm->getCursorHandPointerSequenceID();
}

void Cursor::init(ActorManager *actorManager, DragonINIResource *dragonINIResource) {
	_sequenceID = 0;
	_actor = actorManager->loadActor(0, 0); //Load cursor
	_actor->_x_pos = _x = 160;
	_actor->_y_pos = _y = 100;
	_actor->_priorityLayer = 6;
	_actor->_flags = 0;
	_actor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	_actor->updateSequence(_sequenceID);
	_actor->_flags |= (ACTOR_FLAG_40 | Dragons::ACTOR_FLAG_80 | Dragons::ACTOR_FLAG_100 |
					   ACTOR_FLAG_200);

	dragonINIResource->getFlickerRecord()->actor = _actor; //TODO is this correct?
	dragonINIResource->getFlickerRecord()->flags |= INI_FLAG_1;
	_iniUnderCursor = 0;
	_iniItemInHand = 0;
	_objectInHandSequenceID = 0;
	_cursorActivationSeqOffset = 0;
	_data_800728b0_cursor_seqID = 0;
	_performActionTargetINI = 0;
}


void Cursor::update() {
	if (!_vm->isFlagSet(ENGINE_FLAG_8) || _vm->isFlagSet(Dragons::ENGINE_FLAG_100)) {
		return;
	}
	// TODO update cursor from inputs here.

	// 0x800280b8
	if (_sequenceID == 0 && _vm->_inventory->isOpen()) {
		_sequenceID = 1;
	}

	_actor->_x_pos = _x;
	_actor->_y_pos = _y;

	// 0x80028104
	if (_iniUnderCursor != 0
			&& ((_iniUnderCursor & 0x8000 && _vm->_inventory->isOpen())
			||(!(_iniUnderCursor & 0x8000) && _vm->getINI(_iniUnderCursor - 1)->flags & 0x80))) {
		if (_actor->_sequenceID != _handPointerSequenceID) {
			_actor->updateSequence(_handPointerSequenceID);
		}
		return;
	}
	int32 inventorySequenceID = _vm->_inventory->getSequenceId();
	if ((_iniUnderCursor == 0x8001) && (inventorySequenceID == 1)) {
		if (_actor->_sequenceID != _handPointerSequenceID) {
			_actor->updateSequence(_handPointerSequenceID);
		}
		return;
	}

	if (_iniUnderCursor == 0x8002 && inventorySequenceID == 4) {//goto LAB_80028204;
		if (_actor->_sequenceID != _handPointerSequenceID) {
			_actor->updateSequence(_handPointerSequenceID);
		}
		return;
	}

	if (_iniUnderCursor != 0x8002 || (inventorySequenceID != 1 && inventorySequenceID != 3)) {
		if ((_iniUnderCursor != 0x8001) || ((inventorySequenceID != 0 && (inventorySequenceID != 3)))) {
			if (_sequenceID == 5) {
				uint16 uVar1 = (uint) _objectInHandSequenceID;
				if (_cursorActivationSeqOffset != 0) {
					uVar1 = uVar1 + 1;
				}
				if (uVar1 == (uint) _actor->_sequenceID) {
					return;
				}
				_actor->updateSequence((uint) _objectInHandSequenceID + (uint) (_cursorActivationSeqOffset != 0));
			} else {
				if (_sequenceID + (uint) _cursorActivationSeqOffset != (uint) _actor->_sequenceID) {
					_actor->updateSequence(_sequenceID + (uint) _cursorActivationSeqOffset);
				}
			}
			return;
		}
	}

	if (_iniItemInHand == 0) {
		if (_actor->_sequenceID != _handPointerSequenceID) {
			_actor->updateSequence(_handPointerSequenceID);
		}
		return;
	} else {
		if ((uint)_actor->_sequenceID != (uint)_objectInHandSequenceID + 1) {
			_actor->updateSequence((uint)_objectInHandSequenceID + 1);
		}
	}

	// 0x_800281c0
}

void Cursor::updateVisibility() {
	if (_vm->isFlagSet(ENGINE_FLAG_8) && !_vm->isUnkFlagSet(Dragons::ENGINE_UNK1_FLAG_10)) {
		_actor->_priorityLayer = 9;
	} else {
		_actor->_priorityLayer = 0;
	}
}

void Cursor::updatePosition(int16 x, int16 y) {
	_x = x;
	_y = y;
}

int16 Cursor::updateINIUnderCursor() {
	if (_vm->isFlagSet(ENGINE_FLAG_10)) {
		int16 xOffset = 0;
		if (_vm->_inventory->getSequenceId() == 0 || _vm->_inventory->getSequenceId() == 2) {
			if (_vm->_inventory->getPositionIndex() == 1 || _vm->_inventory->getPositionIndex() == 3) {
				xOffset = 0x32;
			}
		}
		Common::Point inventoryPosition = _vm->_inventory->getPosition();
		if (_x >= inventoryPosition.x + 0xa + xOffset
				&& _x < inventoryPosition.x + 0x35 + xOffset
				&& _y >= inventoryPosition.y + 0xa
				&& _y < inventoryPosition.y + 0x25) {
			_iniUnderCursor = 0x8001;
			return _iniUnderCursor;
		}

		if (_x >= inventoryPosition.x + 0x36
				&& _x < inventoryPosition.x + 0x5f
				&& _y >= inventoryPosition.y + 0xa
				&& _y < inventoryPosition.y + 0x25
				&& _vm->_inventory->getPositionIndex() != 0
				&& _vm->_inventory->getPositionIndex() != 2) {
			_iniUnderCursor = 0x8002;
			return _iniUnderCursor;
		}
	}

	// TODO 0x80028940
	if (_vm->_inventory->getState() == InventoryOpen) {
		_iniUnderCursor = _vm->_inventory->getIniAtPosition(_x, _y);
		return _iniUnderCursor;
	}

	return updateIniFromScene();
}

int16 Cursor::updateIniFromScene() {
	int16 cursorX = _x + _vm->_scene->_camera.x;
	int16 cursorY = _y + _vm->_scene->_camera.y;
	int16 cursorTileX = cursorX / 32;
	int16 cursorTileY = cursorY / 8;
	int16 data_80072890_orig = _performActionTargetINI;
	int16 data_800728b0_cursor_seqID_orig = _data_800728b0_cursor_seqID;

	for (int i = 0; i <_vm->_dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = _vm->_dragonINIResource->getRecord(i);
		if (ini->sceneId != _vm->_scene->getSceneId()) {
			// 0x80028be4
		} else if (!_vm->_dragonINIResource->isFlicker(ini) && !(ini->flags & 0x40)) {
			int16 cursorOverIni = 0;
			// 0x80028a10
			if (ini->flags & 1) {
				// 0x80028b18
				if (ini->actor->isFlagSet(ACTOR_FLAG_40) && ini->actor->isFlagSet(ACTOR_FLAG_8)) {
					int16 iniActorXPosition = ini->actor->_x_pos - ini->actor->_frame->xOffset;
					int16 iniActorYPosition = ini->actor->_y_pos - ini->actor->_frame->yOffset;
					if (cursorX >= iniActorXPosition && cursorX < iniActorXPosition + ini->actor->_frame->width
							&& cursorY >= iniActorYPosition && cursorY < iniActorYPosition + ini->actor->_frame->height) {
						cursorOverIni = i + 1;
					}
				}
			} else {
				// 0x80028a24
				if (ini->imgId != -1) {
					Img *img = _vm->_dragonImg->getImg((uint32)ini->imgId);
					if (img->field_e - 1 >= 1) { // TODO this is >= 2 in the original.
						if (cursorTileX >= img->x && cursorTileX < img->x + img->w && cursorTileY >= img->y && cursorTileY < img->y + img->h) {
							cursorOverIni = i + 1;
						}
					} else {
						// 0x80028ac4
						if (cursorX >= img->x && cursorX < img->x + img->w && cursorY >= img->y && cursorY < img->y + img->h) {
							cursorOverIni = i + 1;
						}
					}
				}
			}
			if (cursorOverIni != 0) {
				// 0x80028bf0
				// _iniUnderCursor = cursorOverIni;
				_performActionTargetINI = _iniUnderCursor;
				_data_800728b0_cursor_seqID = _sequenceID;

				if (ini->flags & 0x800) {
					_performActionTargetINI = cursorOverIni;
					uint32 newSeqId = 1;
					for (int idx = 0; idx < 5; idx++) {
						_data_800728b0_cursor_seqID = idx;
						byte *obd = _vm->_dragonOBD->getFromOpt(cursorOverIni - 1); //_dragonRMS->getAfterSceneLoadedScript(sceneId);
						ScriptOpCall scriptOpCall(obd + 8, READ_LE_UINT32(obd));

//						uVar17 = uVar15;
//						local_58 = dragon_Obd_Offset + *(int *)(uVar16 * 8 + dragon_Opt_Offset + -8) + 8;
//						data_800728b0 = idx;
//						local_54 = read_int32();
//						local_54 = local_54 + local_58;
//						uVar6 = ;
						if (executeScript(scriptOpCall, 0)) {
							newSeqId = idx;
							break;
						}
					}

					_sequenceID = newSeqId;
					_iniUnderCursor = cursorOverIni;
					_performActionTargetINI = _iniUnderCursor;
					_data_800728b0_cursor_seqID = _sequenceID;
					return _iniUnderCursor;
				}
				if (_sequenceID != 0) {
					_iniUnderCursor = cursorOverIni;
					_performActionTargetINI = data_80072890_orig;
					_data_800728b0_cursor_seqID = data_800728b0_cursor_seqID_orig;
					return _iniUnderCursor;
				}
				byte *obd = _vm->_dragonOBD->getFromOpt(cursorOverIni - 1); //_dragonRMS->getAfterSceneLoadedScript(sceneId);
				ScriptOpCall scriptOpCall(obd + 8, READ_LE_UINT32(obd));

//				local_48 = dragon_Obd_Offset + *(int *)(uVar16 * 8 + dragon_Opt_Offset + -8) + 8;
//				local_44 = read_int32();
//				local_44 = local_44 + local_48;
				if (executeScript(scriptOpCall, 0)) {
					_iniUnderCursor = cursorOverIni;
					_performActionTargetINI = data_80072890_orig;
					_data_800728b0_cursor_seqID = data_800728b0_cursor_seqID_orig;
					return _iniUnderCursor;
				}
			}
		}
	}
	_iniUnderCursor = 0;
	_performActionTargetINI = data_80072890_orig;
	_data_800728b0_cursor_seqID = data_800728b0_cursor_seqID_orig;
	return 0;
}

int16 Cursor::executeScript(ScriptOpCall &scriptOpCall, uint16 unkFlag) {
	int16 temp = _vm->_scriptOpcodes->_scriptTargetINI;
	byte *codeStart = scriptOpCall._code;

	scriptOpCall._field8 = 1;
	scriptOpCall._result = 0;
	_vm->_scriptOpcodes->_numDialogStackFramesToPop = 0;
	_vm->_scriptOpcodes->executeScriptLoop(scriptOpCall);

	if (!(scriptOpCall._result & 1) && _data_800728b0_cursor_seqID == 5 && unkFlag != 0) {
		_vm->_scriptOpcodes->_scriptTargetINI = -1;
		scriptOpCall._code = codeStart;
		scriptOpCall._field8 = 1;
		scriptOpCall._result = 0;
		_vm->_scriptOpcodes->_numDialogStackFramesToPop = 0;
		_vm->_scriptOpcodes->executeScriptLoop(scriptOpCall);
		_vm->_scriptOpcodes->_scriptTargetINI = temp;
		if (scriptOpCall._result & 1) {
			scriptOpCall._result |= 2;
		}
	}
	return scriptOpCall._result & 3;
}

void Cursor::selectPreviousCursor() {
	int16 newSequenceID = _sequenceID - 1;
	InventoryState inventoryType = _vm->_inventory->getState();
	if (newSequenceID == 0 && (inventoryType == InventoryOpen || inventoryType == InventionBookOpen)) {
		newSequenceID = _sequenceID - 2;
	}
	_sequenceID = newSequenceID;
	if (_sequenceID == 3 && inventoryType == InventoryOpen) {
		_sequenceID = 1;
	}
	if (_sequenceID == 2) {
		_sequenceID = 1;
	}
	if (_sequenceID == -1) {
		_sequenceID = _iniItemInHand == 0 ? 4 : 5;
	}
}

void Cursor::updateSequenceID(int16 sequenceID) {
	_sequenceID = sequenceID;
	_actor->updateSequence(_sequenceID);
}

void Cursor::setActorFlag400() {
	_actor->setFlag(ACTOR_FLAG_400);
}

void Cursor::clearActorFlag400() {
	_actor->clearFlag(ACTOR_FLAG_400);
}

byte *Cursor::getPalette() {
	return _actor->_actorResource->getPalette();
}

void Cursor::updateActorPosition(int16 x, int16 y) {
	updatePosition(x, y);
	_actor->_x_pos = _x;
	_actor->_y_pos = _y;
}

} // End of namespace Dragons
