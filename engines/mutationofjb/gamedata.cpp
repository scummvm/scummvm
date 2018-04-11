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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mutationofjb/gamedata.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/translation.h"

namespace MutationOfJB {

static bool readString(Common::ReadStream &stream, char *str) {
	char buf[MAX_ENTITY_NAME_LENGTH];
	memset(str, 0, MAX_ENTITY_NAME_LENGTH + 1);

	uint8 len = stream.readByte();
	stream.read(buf, MAX_ENTITY_NAME_LENGTH);

	len = MIN(len, (uint8) MAX_ENTITY_NAME_LENGTH);
	memcpy(str, buf, len);

	return true;
}

bool Door::loadFromStream(Common::ReadStream &stream) {
	readString(stream, _name);

	_destSceneId = stream.readByte();
	_destX = stream.readUint16LE();
	_destY = stream.readUint16LE();
	_x = stream.readUint16LE();
	_y = stream.readByte();
	_width = stream.readUint16LE();
	_height = stream.readByte();
	_walkToX = stream.readUint16LE();
	_walkToY = stream.readByte();
	_SP = stream.readByte();

	return true;
}

bool Object::loadFromStream(Common::ReadStream &stream) {
	_AC = stream.readByte();
	_FA = stream.readByte();
	_FR = stream.readByte();
	_NA = stream.readByte();
	_FS = stream.readByte();
	_unknown = stream.readByte();
	_CA = stream.readByte();
	_x = stream.readUint16LE();
	_y = stream.readByte();
	_XL = stream.readUint16LE();
	_YL = stream.readByte();
	_WX = stream.readUint16LE();
	_WY = stream.readByte();
	_SP = stream.readByte();

	return true;
}

bool Static::loadFromStream(Common::ReadStream &stream) {
	_active = stream.readByte();
	readString(stream, _name);
	_x = stream.readUint16LE();
	_y = stream.readByte();
	_width = stream.readUint16LE();
	_height = stream.readByte();
	_walkToX = stream.readUint16LE();
	_walkToY = stream.readByte();
	_SP = stream.readByte();

	return true;
}

bool Bitmap::loadFromStream(Common::ReadStream &stream) {
	_frame = stream.readByte();
	_isVisible = stream.readByte();
	_x1 = stream.readUint16LE();
	_y1 = stream.readByte();
	_x2 = stream.readUint16LE();
	_y2 = stream.readByte();

	return true;
}

bool Scene::loadFromStream(Common::ReadStream &stream) {
	int i;

	_startup = stream.readByte();
	_unknown001 = stream.readByte();
	_unknown002 = stream.readByte();
	_unknown003 = stream.readByte();
	_DL = stream.readByte();

	_noDoors = stream.readByte();
	_noDoors = MIN(_noDoors, (uint8) ARRAYSIZE(_doors));
	for (i = 0; i < ARRAYSIZE(_doors); ++i) {
		_doors[i].loadFromStream(stream);
	}

	_noObjects = stream.readByte();
	_noObjects = MIN(_noObjects, (uint8) ARRAYSIZE(_objects));
	for (i = 0; i < ARRAYSIZE(_objects); ++i) {
		_objects[i].loadFromStream(stream);
	}

	_noStatics = stream.readByte();
	_noStatics = MIN(_noStatics, (uint8) ARRAYSIZE(_statics));
	for (i = 0; i < ARRAYSIZE(_statics); ++i) {
		_statics[i].loadFromStream(stream);
	}

	for (i = 0; i < ARRAYSIZE(_bitmaps); ++i) {
		_bitmaps[i].loadFromStream(stream);
	}

	_obstacleY1 = stream.readUint16LE();
	_palRotStart = stream.readByte();
	_palRotEnd = stream.readByte();
	_palRotPeriod = stream.readByte();
	stream.read(_unknown38A, 80);

	return true;
}

Door *Scene::getDoor(uint8 doorId) {
	if (doorId == 0 || doorId > _noDoors) {
		warning(_("Door %d does not exist"), doorId);
		return nullptr;
	}

	return &_doors[doorId - 1];
}

Object *Scene::getObject(uint8 objectId, bool ignoreNo) {
	if (objectId == 0 || objectId > getNoObjects(ignoreNo))  {
		warning(_("Object %d does not exist"), objectId);
		return nullptr;
	}

	return &_objects[objectId - 1];
}

Static *Scene::getStatic(uint8 staticId, bool ignoreNo) {
	if (staticId == 0 || staticId > (!ignoreNo ? MIN(_noStatics, (uint8) ARRAYSIZE(_statics)) : ARRAYSIZE(_statics))) {
		warning(_("Static %d does not exist"), staticId);
		return nullptr;
	}

	return &_statics[staticId - 1];
}

uint8 Scene::getNoDoors(bool ignoreNo) const {
	return (!ignoreNo ? MIN(_noDoors, (uint8) ARRAYSIZE(_doors)) : ARRAYSIZE(_doors));
}

uint8 Scene::getNoObjects(bool ignoreNo) const {
	return (!ignoreNo ? MIN(_noObjects, (uint8) ARRAYSIZE(_objects)) : ARRAYSIZE(_objects));
}

uint8 Scene::getNoStatics(bool ignoreNo) const {
	return (!ignoreNo ? MIN(_noStatics, (uint8) ARRAYSIZE(_statics)) : ARRAYSIZE(_statics));
}

Door *Scene::findDoor(int16 x, int16 y, int *index) {
	for (int i = 0; i < getNoDoors(); ++i) {
		Door &door = _doors[i];
		if ((x >= door._x) && (x < door._x + door._width) && (y >= door._y) && (y < door._y + door._height)) {
			if (index) {
				*index = i + 1;
			}
			return &door;
		}
	}

	return nullptr;
}

Static *Scene::findStatic(int16 x, int16 y, int *index) {
	for (int i = 0; i < getNoStatics(); ++i) {
		Static &stat = _statics[i];
		if ((x >= stat._x) && (x < stat._x + stat._width) && (y >= stat._y) && (y < stat._y + stat._height)) {
			if (index) {
				*index = i + 1;
			}
			return &stat;
		}
	}

	return nullptr;
}

Bitmap *Scene::findBitmap(int16 x, int16 y, int *index) {
	for (int i = 0; i < ARRAYSIZE(_bitmaps); ++i) {
		Bitmap &bitmap = _bitmaps[i];
		if ((x >= bitmap._x1) && (x <= bitmap._x2) && (y >= bitmap._y1) && (y <= bitmap._y2)) {
			if (index) {
				*index = i + 1;
			}
			return &bitmap;
		}
	}

	return nullptr;
}


GameData::GameData()
	: _currentScene(0),
	_lastScene(0),
	_partB(false),
	_inventory()
	{}

Scene *GameData::getScene(uint8 sceneId) {
	if (sceneId == 0 || sceneId > ARRAYSIZE(_scenes)) {
		warning(_("Scene %d does not exist"), sceneId);
		return nullptr;
	}

	return &_scenes[sceneId - 1];
}

Scene *GameData::getCurrentScene() {
	return getScene(_currentScene);
}

Inventory &GameData::getInventory() {
	return _inventory;
}

bool GameData::loadFromStream(Common::ReadStream &stream) {
	for (int i = 0; i < ARRAYSIZE(_scenes); ++i) {
		_scenes[i].loadFromStream(stream);
	}

	return true;
}

}
