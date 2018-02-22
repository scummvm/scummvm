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

#include "mutationofjb/game.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/translation.h"

namespace MutationOfJB {

static bool readString(Common::ReadStream &stream, char *str)
{
	char buf[MAX_STR_LENGTH];
	memset(str, 0, MAX_STR_LENGTH + 1);

	uint8 len = stream.readByte();
	stream.read(buf, MAX_STR_LENGTH);

	len = MIN(len, MAX_STR_LENGTH);
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
	_unknown = stream.readByte();
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

	_obstacleY1 = stream.readByte();
	_unknown386 = stream.readByte();
	_palRotStart = stream.readByte();
	_palRotEnd = stream.readByte();
	_palRotPeriod = stream.readByte();
	stream.read(_unknown38A, 80);

	return true;
}

Object *Scene::getObject(uint8 objectId) {
	if (objectId == 0 || objectId > _noObjects) {
		warning(_("Object %d does not exist"), objectId);
		return nullptr;
	}

	return &_objects[objectId - 1];
}

GameData::GameData() : _currentScene(0) {}

Scene *GameData::getScene(uint8 sceneId)
{
	if (sceneId == 0 || sceneId > ARRAYSIZE(_scenes)) {
		warning(_("Scene %d does not exist"), sceneId);
		return nullptr;
	}

	return &_scenes[sceneId - 1];
}

bool GameData::loadFromStream(Common::ReadStream &stream) {
	for (int i = 0; i < ARRAYSIZE(_scenes); ++i) {
		_scenes[i].loadFromStream(stream);
	}

	return true;
}

}
