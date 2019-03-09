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
#include "dragons.h"
#include "dragonimg.h"
#include "dragonini.h"
#include "inventory.h"
#include "scene.h"

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
	data_8007283c = 0;
	data_either_5_or_0 = 0;
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
				if (data_either_5_or_0 != 0) {
					uVar1 = uVar1 + 1;
				}
				if (uVar1 == (uint) _actor->_sequenceID) {
					return;
				}
				_actor->updateSequence((uint) data_8007283c + (uint) (data_either_5_or_0 != 0));
			} else {
				if (_sequenceID + (uint) data_either_5_or_0 != (uint) _actor->_sequenceID) {
					_actor->updateSequence(_sequenceID + (uint) data_either_5_or_0);
				}
			}
			return;
		}
	}

	if (_vm->data_8006f3a8 == 0) {
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

	_iniUnderCursor = 0; //TODO remove this once we complete function.

	// TODO 0x80028940
	int16 inventoryType = _vm->_inventory->getType();
	if (inventoryType == 1) {
		// Might be open inventory bag.
		//TODO 0x80028da8
	} else if (inventoryType > 2) {
		_iniUnderCursor = 0;
		return 0;
	}

	return updateIniFromScene();
}

int16 Cursor::updateIniFromScene() {
	int16 cursorTileX = _actor->x_pos / 32;
	int16 cursorTileY = _actor->y_pos / 8;

	for(int i=0;i <_vm->_dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = _vm->_dragonINIResource->getRecord(i);
		if (ini->sceneId != _vm->_scene->getSceneId()) {
			// 0x80028be4
		} else if (!_vm->_dragonINIResource->isFlicker(ini) && !(ini->field_1a_flags_maybe & 0x40)) {
			int16 cursorOverIni = 0;
			// 0x80028a10
			if (ini->field_1a_flags_maybe & 1) {
				// 0x80028b18
			} else {
				// 0x80028a24
				if (ini->field_2 != -1) {
					IMG *img = _vm->_dragonIMG->getIMG((uint32)ini->field_2);
					if (img->field_e - 1 >= 2) {
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
				_iniUnderCursor = cursorOverIni;
			}
		}
	}

			return 0;
}

} // End of namespace Dragons
