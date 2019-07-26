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
#include "cursor.h"
#include "actor.h"
#include "actorresource.h"
#include "dragons.h"
#include "dragonimg.h"
#include "dragonini.h"
#include "dragonobd.h"
#include "inventory.h"
#include "scene.h"
#include "scriptopcodes.h"

namespace Dragons {

Cursor::Cursor(DragonsEngine *vm): _vm(vm), _actor(0), _x(0), _y(0) {
	_sequenceID = 0;
}

void Cursor::init(ActorManager *actorManager, DragonINIResource *dragonINIResource) {
	_sequenceID = 0;
	_actor = actorManager->loadActor(0, 0); //Load cursor
	_actor->x_pos = _x = 160;
	_actor->y_pos = _y = 100;
	_actor->priorityLayer = 6;
	_actor->flags = 0;
	_actor->field_e = 0x100;
	_actor->updateSequence(_sequenceID);
	_actor->flags |= (Dragons::ACTOR_FLAG_40 | Dragons::ACTOR_FLAG_80 | Dragons::ACTOR_FLAG_100 |
					  Dragons::ACTOR_FLAG_200);

	dragonINIResource->getFlickerRecord()->actor = _actor; //TODO is this correct?
	dragonINIResource->getFlickerRecord()->field_1a_flags_maybe |= Dragons::INI_FLAG_1;
	_iniUnderCursor = 0;
	iniItemInHand = 0;
	data_8007283c = 0;
	_cursorActivationSeqOffset = 0;
	data_800728b0_cursor_seqID = 0;
	data_80072890 = 0;
}


void Cursor::update() {
	if (!_vm->isFlagSet(Dragons::ENGINE_FLAG_8) || _vm->isFlagSet(Dragons::ENGINE_FLAG_100)) {
		return;
	}
	// TODO update cursor from inputs here.

	// 0x800280b8
	if (_sequenceID == 0 && (_vm->_inventory->getType() == 1 || _vm->_inventory->getType() == 2)) {
		_sequenceID = 1;
	}

	_actor->x_pos = _x + _vm->_scene->_camera.x;
	_actor->y_pos = _y + _vm->_scene->_camera.y;

	// 0x80028104
	if (_iniUnderCursor != 0
	&& ((_iniUnderCursor & 0x8000 && (_vm->_inventory->getType() == 1 || _vm->_inventory->getType() == 2))
		||(!(_iniUnderCursor & 0x8000) && _vm->getINI(_iniUnderCursor - 1)->field_1a_flags_maybe & 0x80))
	) {
		if (_actor->_sequenceID != 0x84) {
			_actor->updateSequence(0x84);
		}
		return;
	}
	int32 inventorySequenceID = _vm->_inventory->getSequenceId();
	if ((_iniUnderCursor == 0x8001) && (inventorySequenceID == 1)) {
		if (_actor->_sequenceID != 0x84) {
			_actor->updateSequence(0x84);
		}
		return;
	}

	if (_iniUnderCursor == 0x8002 && inventorySequenceID == 4) {//goto LAB_80028204;
		if (_actor->_sequenceID != 0x84) {
			_actor->updateSequence(0x84);
		}
		return;
	}

	if (_iniUnderCursor != 0x8002 || (inventorySequenceID != 1 && inventorySequenceID != 3)) {
		if ((_iniUnderCursor != 0x8001) || ((inventorySequenceID != 0 && (inventorySequenceID != 3)))) {
			if (_sequenceID == 5) {
				uint16 uVar1 = (uint) data_8007283c;
				if (_cursorActivationSeqOffset != 0) {
					uVar1 = uVar1 + 1;
				}
				if (uVar1 == (uint) _actor->_sequenceID) {
					return;
				}
				_actor->updateSequence((uint) data_8007283c + (uint) (_cursorActivationSeqOffset != 0));
			} else {
				if (_sequenceID + (uint) _cursorActivationSeqOffset != (uint) _actor->_sequenceID) {
					_actor->updateSequence(_sequenceID + (uint) _cursorActivationSeqOffset);
				}
			}
			return;
		}
	}

	if (iniItemInHand == 0) {
		if (_actor->_sequenceID != 0x84) {
			_actor->updateSequence(0x84);
		}
		return;
	}
	else {
		if ((uint)_actor->_sequenceID != (uint)data_8007283c + 1) {
			_actor->updateSequence((uint)data_8007283c + 1);
		}
	}

	// 0x_800281c0
}

void Cursor::updateVisibility() {
	if (_vm->isFlagSet(Dragons::ENGINE_FLAG_8) && !_vm->isUnkFlagSet(Dragons::ENGINE_UNK1_FLAG_10)) {
		_actor->priorityLayer = 9;
	} else {
		_actor->priorityLayer = 0;
	}
}

void Cursor::updatePosition(int16 x, int16 y) {
	_x = x;
	_y = y;
}

int16 Cursor::updateINIUnderCursor() {
	if (_vm->isFlagSet(Dragons::ENGINE_FLAG_10)) {
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
	int16 inventoryType = _vm->_inventory->getType();
	if (inventoryType == 1) {
		_iniUnderCursor = _vm->_inventory->getIniAtPosition(_x, _y);
		return _iniUnderCursor;
	} else {
		if (inventoryType < 2) {
			if (inventoryType != 0) {
				_iniUnderCursor = 0;
				return 0;
			}
		}
		else {
			if (inventoryType != 2) {
				_iniUnderCursor = 0;
				return 0;
			}
		}
	}

	return updateIniFromScene();
}

int16 Cursor::updateIniFromScene() {
	int16 cursorTileX = _actor->x_pos / 32;
	int16 cursorTileY = _actor->y_pos / 8;
	int16 data_80072890_orig = data_80072890;
	int16 data_800728b0_cursor_seqID_orig = data_800728b0_cursor_seqID;
	for(int i=0;i <_vm->_dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = _vm->_dragonINIResource->getRecord(i);
		if (ini->sceneId != _vm->_scene->getSceneId()) {
			// 0x80028be4
		} else if (!_vm->_dragonINIResource->isFlicker(ini) && !(ini->field_1a_flags_maybe & 0x40)) {
			int16 cursorOverIni = 0;
			// 0x80028a10
			if (ini->field_1a_flags_maybe & 1) {
				// 0x80028b18
				if (ini->actor->isFlagSet(ACTOR_FLAG_40) && ini->actor->isFlagSet(ACTOR_FLAG_8)) {
					int16 iniActorXPosition = ini->actor->x_pos - ini->actor->frame->xOffset;
					int16 iniActorYPosition = ini->actor->y_pos - ini->actor->frame->yOffset;
					if (_actor->x_pos >= iniActorXPosition && _actor->x_pos < iniActorXPosition + ini->actor->frame->width
					&& _actor->y_pos >= iniActorYPosition && _actor->y_pos < iniActorYPosition + ini->actor->frame->height) {
						cursorOverIni = i + 1;
					}
				}
			} else {
				// 0x80028a24
				if (ini->field_2 != -1) {
					IMG *img = _vm->_dragonIMG->getIMG((uint32)ini->field_2);
					if (img->field_e - 1 >= 1) { // TODO this is >= 2 in the original.
						if (cursorTileX >= img->x && cursorTileX < img->x + img->w && cursorTileY >= img->y && cursorTileY < img->y + img->h) {
							cursorOverIni = i + 1;
						}
					} else {
						// 0x80028ac4
						if (_actor->x_pos >= img->x && _actor->x_pos < img->x + img->w && _actor->y_pos >= img->y && _actor->y_pos < img->y + img->h) {
							cursorOverIni = i + 1;
						}
					}
				}
			}
			if (cursorOverIni != 0) {
				// 0x80028bf0
				debug(1, "here OK!!!");
				// _iniUnderCursor = cursorOverIni;
				data_80072890 = _iniUnderCursor;
				data_800728b0_cursor_seqID = _sequenceID;
				if (ini->field_1a_flags_maybe & 0x800) {
					data_80072890 = cursorOverIni;
					uint32 newSeqId = 1;
					for(int idx=0; idx < 5; idx++) {
						data_800728b0_cursor_seqID = idx;
						byte *obd = _vm->_dragonOBD->getFromOpt(cursorOverIni - 1); //_dragonRMS->getObdDataFieldC(sceneId);
						ScriptOpCall scriptOpCall;
						scriptOpCall._code = obd + 8;
						scriptOpCall._codeEnd = scriptOpCall._code + READ_LE_UINT32(obd);

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
					data_80072890 = _iniUnderCursor;
					data_800728b0_cursor_seqID = _sequenceID;
					return _iniUnderCursor;
				}
				if (_sequenceID != 0) {
					_iniUnderCursor = cursorOverIni;
					data_80072890 = data_80072890_orig;
					data_800728b0_cursor_seqID = data_800728b0_cursor_seqID_orig;
					return _iniUnderCursor;
				}
				byte *obd = _vm->_dragonOBD->getFromOpt(cursorOverIni - 1); //_dragonRMS->getObdDataFieldC(sceneId);
				ScriptOpCall scriptOpCall;
				scriptOpCall._code = obd + 8;
				scriptOpCall._codeEnd = scriptOpCall._code + READ_LE_UINT32(obd);

//				local_48 = dragon_Obd_Offset + *(int *)(uVar16 * 8 + dragon_Opt_Offset + -8) + 8;
//				local_44 = read_int32();
//				local_44 = local_44 + local_48;
				if(executeScript(scriptOpCall, 0)) {
					_iniUnderCursor = cursorOverIni;
					data_80072890 = data_80072890_orig;
					data_800728b0_cursor_seqID = data_800728b0_cursor_seqID_orig;
					return _iniUnderCursor;
				}
			}
		}
	}
	_iniUnderCursor = 0;
	data_80072890 = data_80072890_orig;
	data_800728b0_cursor_seqID = data_800728b0_cursor_seqID_orig;
	return 0;
}

int16 Cursor::executeScript(ScriptOpCall &scriptOpCall, uint16 unkFlag) {
	int16 temp = _vm->_scriptOpcodes->_data_800728c0;
	scriptOpCall._field8 = 1;
	scriptOpCall._result = 0;
	_vm->_scriptOpcodes->_data_80071f5c = 0;
	_vm->_scriptOpcodes->executeScriptLoop(scriptOpCall);

	if (!(scriptOpCall._result & 1) && data_800728b0_cursor_seqID == 5 && unkFlag != 0) {
		_vm->_scriptOpcodes->_data_800728c0 = -1;
		scriptOpCall._field8 = 1;
		scriptOpCall._result = 0;
		_vm->_scriptOpcodes->_data_80071f5c = 0;
		_vm->_scriptOpcodes->executeScriptLoop(scriptOpCall);
		_vm->_scriptOpcodes->_data_800728c0 = temp;
		if (scriptOpCall._result & 1) {
			scriptOpCall._result |= 2;
		}
	}
	return scriptOpCall._result & 3;
}

void Cursor::selectPreviousCursor() {
	int16 uVar7 = _sequenceID - 1;
	int16 inventoryType = _vm->_inventory->getType();
	if ((uVar7 == 0) && ((inventoryType == 1 || (inventoryType == 2)))) {
		uVar7 = _sequenceID - 2;
	}
	_sequenceID = uVar7;
	if ((_sequenceID == 3) && (inventoryType == 1)) {
		_sequenceID = 1;
	}
	if (_sequenceID == 2) {
		_sequenceID = 0x10001;
	}
	if (_sequenceID == -1) {
		_sequenceID = iniItemInHand == 0 ? 4 : 5;
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

} // End of namespace Dragons
