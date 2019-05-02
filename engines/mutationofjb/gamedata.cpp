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

#include "common/serializer.h"
#include "common/stream.h"
#include "common/util.h"

namespace MutationOfJB {

static bool readEntityNameString(Common::ReadStream &stream, char *str) {
	char buf[MAX_ENTITY_NAME_LENGTH];
	memset(str, 0, MAX_ENTITY_NAME_LENGTH + 1);

	uint8 len = stream.readByte();
	stream.read(buf, MAX_ENTITY_NAME_LENGTH);

	len = MIN(len, static_cast<uint8>(MAX_ENTITY_NAME_LENGTH));
	memcpy(str, buf, len);

	return true;
}

static void syncEntityNameString(char *cstr, Common::Serializer &sz) {
	if (sz.isLoading()) {
		Common::String str;
		sz.syncString(str);
		strncpy(cstr, str.c_str(), MAX_ENTITY_NAME_LENGTH);
		cstr[MAX_ENTITY_NAME_LENGTH] = 0;
	} else {
		Common::String str(cstr);
		sz.syncString(str);
	}
}

bool Door::isActive() {
	return *_name != '\0';
}

bool Door::loadInitialState(Common::ReadStream &stream) {
	readEntityNameString(stream, _name);

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

void Door::saveLoadWithSerializer(Common::Serializer &sz) {
	syncEntityNameString(_name, sz);
	sz.syncAsByte(_destSceneId);
	sz.syncAsUint16LE(_destX);
	sz.syncAsUint16LE(_destY);
	sz.syncAsUint16LE(_x);
	sz.syncAsByte(_y);
	sz.syncAsUint16LE(_width);
	sz.syncAsByte(_height);
	sz.syncAsUint16LE(_walkToX);
	sz.syncAsByte(_walkToY);
	sz.syncAsByte(_SP);
}

bool Door::allowsImplicitSceneChange() const {
	const size_t length = strlen(_name);
	if (length == 0)
		return false;

	return _name[length - 1] != '+';
}

bool Object::loadInitialState(Common::ReadStream &stream) {
	_active = stream.readByte();
	_firstFrame = stream.readByte();
	_randomFrame = stream.readByte();
	_numFrames = stream.readByte();
	_roomFrameLSB = stream.readByte();
	_jumpChance = stream.readByte();
	_currentFrame = stream.readByte();
	_x = stream.readUint16LE();
	_y = stream.readByte();
	_width = stream.readUint16LE();
	_height = stream.readByte();
	_WX = stream.readUint16LE();
	_roomFrameMSB = stream.readByte();
	_SP = stream.readByte();

	return true;
}

void Object::saveLoadWithSerializer(Common::Serializer &sz) {
	sz.syncAsByte(_active);
	sz.syncAsByte(_firstFrame);
	sz.syncAsByte(_randomFrame);
	sz.syncAsByte(_numFrames);
	sz.syncAsByte(_roomFrameLSB);
	sz.syncAsByte(_jumpChance);
	sz.syncAsByte(_currentFrame);
	sz.syncAsUint16LE(_x);
	sz.syncAsByte(_y);
	sz.syncAsUint16LE(_width);
	sz.syncAsByte(_height);
	sz.syncAsUint16LE(_WX);
	sz.syncAsByte(_roomFrameMSB);
	sz.syncAsByte(_SP);
}

bool Static::loadInitialState(Common::ReadStream &stream) {
	_active = stream.readByte();
	readEntityNameString(stream, _name);
	_x = stream.readUint16LE();
	_y = stream.readByte();
	_width = stream.readUint16LE();
	_height = stream.readByte();
	_walkToX = stream.readUint16LE();
	_walkToY = stream.readByte();
	_walkToFrame = stream.readByte();

	return true;
}

void Static::saveLoadWithSerializer(Common::Serializer &sz) {
	sz.syncAsByte(_active);
	syncEntityNameString(_name, sz);
	sz.syncAsUint16LE(_x);
	sz.syncAsByte(_y);
	sz.syncAsUint16LE(_width);
	sz.syncAsByte(_height);
	sz.syncAsUint16LE(_walkToX);
	sz.syncAsByte(_walkToY);
	sz.syncAsByte(_walkToFrame);
}

bool Static::isCombinable() const {
	const size_t length = strlen(_name);
	if (length == 0)
		return false;

	return _name[length - 1] == '[';
}

bool Static::allowsImplicitPickup() const {
	return _name[0] == '~';
}

bool Bitmap::loadInitialState(Common::ReadStream &stream) {
	_roomFrame = stream.readByte();
	_isVisible = stream.readByte();
	_x1 = stream.readUint16LE();
	_y1 = stream.readByte();
	_x2 = stream.readUint16LE();
	_y2 = stream.readByte();

	return true;
}

void Bitmap::saveLoadWithSerializer(Common::Serializer &sz) {
	sz.syncAsByte(_roomFrame);
	sz.syncAsByte(_isVisible);
	sz.syncAsUint16LE(_x1);
	sz.syncAsByte(_y1);
	sz.syncAsUint16LE(_x2);
	sz.syncAsByte(_y2);
}

bool Scene::loadInitialState(Common::ReadStream &stream) {
	int i;

	_startup = stream.readByte();
	_unknown001 = stream.readByte();
	_unknown002 = stream.readByte();
	_unknown003 = stream.readByte();
	_delay = stream.readByte();

	_noDoors = stream.readByte();
	_noDoors = MIN(_noDoors, static_cast<uint8>(ARRAYSIZE(_doors)));
	for (i = 0; i < ARRAYSIZE(_doors); ++i) {
		_doors[i].loadInitialState(stream);
	}

	_noObjects = stream.readByte();
	_noObjects = MIN(_noObjects, static_cast<uint8>(ARRAYSIZE(_objects)));
	for (i = 0; i < ARRAYSIZE(_objects); ++i) {
		_objects[i].loadInitialState(stream);
	}

	_noStatics = stream.readByte();
	_noStatics = MIN(_noStatics, static_cast<uint8>(ARRAYSIZE(_statics)));
	for (i = 0; i < ARRAYSIZE(_statics); ++i) {
		_statics[i].loadInitialState(stream);
	}

	for (i = 0; i < ARRAYSIZE(_bitmaps); ++i) {
		_bitmaps[i].loadInitialState(stream);
	}

	_obstacleY1 = stream.readUint16LE();
	_palRotFirst = stream.readByte();
	_palRotLast = stream.readByte();
	_palRotDelay = stream.readByte();
	_exhaustedConvItemNext = stream.readByte();

	for (i = 0; i < ARRAYSIZE(_exhaustedConvItems); ++i) {
		_exhaustedConvItems[i]._encodedData = stream.readByte();
	}

	return true;
}

void Scene::saveLoadWithSerializer(Common::Serializer &sz) {
	sz.syncAsByte(_startup);
	sz.syncAsByte(_unknown001);
	sz.syncAsByte(_unknown002);
	sz.syncAsByte(_unknown003);
	sz.syncAsByte(_delay);

	sz.syncAsByte(_noDoors);
	for (int i = 0; i < ARRAYSIZE(_doors); ++i) {
		_doors[i].saveLoadWithSerializer(sz);
	}

	sz.syncAsByte(_noObjects);
	for (int i = 0; i < ARRAYSIZE(_objects); ++i) {
		_objects[i].saveLoadWithSerializer(sz);
	}

	sz.syncAsByte(_noStatics);
	for (int i = 0; i < ARRAYSIZE(_statics); ++i) {
		_statics[i].saveLoadWithSerializer(sz);
	}

	for (int i = 0; i < ARRAYSIZE(_bitmaps); ++i) {
		_bitmaps[i].saveLoadWithSerializer(sz);
	}

	sz.syncAsUint16LE(_obstacleY1);
	sz.syncAsByte(_palRotFirst);
	sz.syncAsByte(_palRotLast);
	sz.syncAsByte(_palRotDelay);
	sz.syncAsByte(_exhaustedConvItemNext);

	for (int i = 0; i < ARRAYSIZE(_exhaustedConvItems); ++i) {
		sz.syncAsByte(_exhaustedConvItems[i]._encodedData);
	}
}

Door *Scene::getDoor(uint8 doorId) {
	if (doorId == 0 || doorId > _noDoors) {
		warning("Door %d does not exist", doorId);
		return nullptr;
	}

	return &_doors[doorId - 1];
}

Object *Scene::getObject(uint8 objectId, bool ignoreNo) {
	if (objectId == 0 || objectId > getNoObjects(ignoreNo))  {
		warning("Object %d does not exist", objectId);
		return nullptr;
	}

	return &_objects[objectId - 1];
}

Static *Scene::getStatic(uint8 staticId, bool ignoreNo) {
	if (staticId == 0 || staticId > (!ignoreNo ? MIN(_noStatics, static_cast<uint8>(ARRAYSIZE(_statics))) : ARRAYSIZE(_statics))) {
		warning("Static %d does not exist", staticId);
		return nullptr;
	}

	return &_statics[staticId - 1];
}

Bitmap *Scene::getBitmap(uint8 bitmapId) {
	if (bitmapId == 0 || bitmapId > ARRAYSIZE(_bitmaps)) {
		warning("Bitmap %d does not exist", bitmapId);
		return nullptr;
	}

	return &_bitmaps[bitmapId - 1];
}

uint8 Scene::getNoDoors(bool ignoreNo) const {
	return (!ignoreNo ? MIN(_noDoors, static_cast<uint8>(ARRAYSIZE(_doors))) : ARRAYSIZE(_doors));
}

uint8 Scene::getNoObjects(bool ignoreNo) const {
	return (!ignoreNo ? MIN(_noObjects, static_cast<uint8>(ARRAYSIZE(_objects))) : ARRAYSIZE(_objects));
}

uint8 Scene::getNoStatics(bool ignoreNo) const {
	return (!ignoreNo ? MIN(_noStatics, static_cast<uint8>(ARRAYSIZE(_statics))) : ARRAYSIZE(_statics));
}

uint8 Scene::getNoBitmaps() const {
	return ARRAYSIZE(_bitmaps);
}

Door *Scene::findDoor(int16 x, int16 y, bool activeOnly, int *index) {
	for (int i = 0; i < getNoDoors(); ++i) {
		Door &door = _doors[i];
		if ((!activeOnly || door.isActive()) && (x >= door._x) && (x < door._x + door._width) && (y >= door._y) && (y < door._y + door._height)) {
			if (index) {
				*index = i + 1;
			}
			return &door;
		}
	}

	return nullptr;
}

Static *Scene::findStatic(int16 x, int16 y, bool activeOnly, int *index) {
	for (int i = 0; i < getNoStatics(); ++i) {
		Static &stat = _statics[i];
		if ((!activeOnly || stat._active) && (x >= stat._x) && (x < stat._x + stat._width) && (y >= stat._y) && (y < stat._y + stat._height)) {
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

void Scene::addExhaustedConvItem(uint8 context, uint8 convItemIndex, uint8 convGroupIndex) {
	_exhaustedConvItems[_exhaustedConvItemNext - 1] = ExhaustedConvItem(context, convItemIndex, convGroupIndex);
	_exhaustedConvItemNext++;
}

bool Scene::isConvItemExhausted(uint8 context, uint8 convItemIndex, uint8 convGroupIndex) const {
	for (uint8 i = 0; i < _exhaustedConvItemNext - 1; ++i) {
		const ExhaustedConvItem &convItem = _exhaustedConvItems[i];
		if (convItem.getContext() == context && convItem.getConvItemIndex() == convItemIndex && convItem.getConvGroupIndex() == convGroupIndex) {
			return true;
		}
	}

	return false;
}


GameData::GameData()
	: _currentScene(0),
	  _lastScene(0),
	  _partB(false),
	  _inventory(),
	  _currentAPK("piggy.apk"),
	  _color(WHITE) {}

Scene *GameData::getScene(uint8 sceneId) {
	if (sceneId == 0 || sceneId > ARRAYSIZE(_scenes)) {
		warning("Scene %d does not exist", sceneId);
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

bool GameData::loadInitialState(Common::ReadStream &stream) {
	for (int i = 0; i < ARRAYSIZE(_scenes); ++i) {
		_scenes[i].loadInitialState(stream);
	}

	return true;
}

void GameData::saveLoadWithSerializer(Common::Serializer &sz) {
	for (int i = 0; i < ARRAYSIZE(_scenes); ++i) {
		_scenes[i].saveLoadWithSerializer(sz);
	}

	sz.syncAsByte(_currentScene);
	sz.syncAsByte(_partB);
	_inventory.saveLoadWithSerializer(sz);
	sz.syncString(_currentAPK);
}

}
