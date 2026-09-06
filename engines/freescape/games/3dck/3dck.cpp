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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/substream.h"
#include "math/utils.h"

#include "freescape/games/3dck/3dck.h"

namespace Freescape {

static void requireBytes(Common::SeekableReadStream &file, uint32 size) {
	if (file.err() || file.pos() > file.size() || size > file.size() - file.pos())
		error("Truncated 3D Construction Kit data");
}

static uint16 readBlockSize(Common::SeekableReadStream &file) {
	requireBytes(file, 2);
	uint16 size = file.readUint16LE();
	requireBytes(file, size);
	return size;
}

static Math::Vector3d readVector(Common::SeekableReadStream &file) {
	uint16 x = file.readUint16BE();
	uint16 y = file.readUint16BE();
	uint16 z = file.readUint16BE();
	return Math::Vector3d(x, y, z);
}

KitEngine::KitEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd), _initialPlayerHeight(0) {
	_renderMode = Common::kRenderVGA;
	_screenW = 320;
	_screenH = 200;
	_fullscreenViewArea = Common::Rect(_screenW, _screenH);
	_playerHeightNumber = 0;
	_playerHeightMaxNumber = 0;
	_playerWidth = 12;
	_playerDepth = 32;
	_soundIndexShoot = -1;
}

void KitEngine::loadAssets() {
	Common::File file;
	if (!file.open(_gameDescription->filesDescriptions[0].fileName))
		error("Unable to open 3D Construction Kit RUN file");

	char description[80];
	if (file.read(description, sizeof(description)) != sizeof(description) ||
			memcmp(description + 75, "VGA\x1a", 5) != 0)
		error("Unsupported 3D Construction Kit RUN format");

	// RUN blocks use little-endian lengths; the world data is big-endian.
	uint16 worldSize = readBlockSize(file);
	uint32 worldEnd = file.pos() + worldSize;
	Common::SeekableSubReadStream world(&file, file.pos(), worldEnd);
	loadWorld(world);
	file.seek(worldEnd);

	uint16 menuSize = readBlockSize(file);
	file.skip(menuSize);
	uint16 borderSize = readBlockSize(file);
	if (borderSize != _screenW * _screenH)
		error("Invalid 3D Construction Kit border size");
	_border = new Graphics::ManagedSurface();
	_border->create(_screenW, _screenH, Graphics::PixelFormat::createFormatCLUT8());
	for (int y = 0; y < _screenH; y++) {
		if (file.read(_border->getBasePtr(0, y), _screenW) != uint(_screenW))
			error("Truncated 3D Construction Kit border");
	}

	if (file.read(_palette, sizeof(_palette)) != sizeof(_palette))
		error("Truncated 3D Construction Kit palette");
	for (uint i = 0; i < sizeof(_palette); i++) {
		byte component = _palette[i] & 0x3f;
		_palette[i] = (component << 2) | (component >> 4);
	}
	_border->setPalette(_palette, 0, 256);
	_gfx->_palette = _palette;
	_gfx->_keyColor = 0;
}

void KitEngine::loadWorld(Common::SeekableReadStream &file) {
	requireBytes(file, 500);
	uint32 signature = file.readUint32BE();
	if (signature != MKTAG('C', 'P', 0, 0) && signature != MKTAG('C', 'P', '0', '1'))
		error("Unsupported 3D Construction Kit world format");
	uint16 areaCount = file.readUint16BE();
	uint32 globalConditions = 2 * file.readUint16BE();
	uint16 centerX = file.readUint16BE();
	uint16 centerY = file.readUint16BE();
	uint16 halfWidth = file.readUint16BE();
	uint16 halfHeight = file.readUint16BE();
	if (!halfWidth || !halfHeight || centerX < halfWidth || centerY < halfHeight ||
			centerX + halfWidth > _screenW || centerY + halfHeight >= _screenH)
		error("Invalid 3D Construction Kit viewport");
	int top = _screenH - 1 - centerY - halfHeight;
	_viewArea = Common::Rect(centerX - halfWidth, top, centerX + halfWidth, top + 2 * halfHeight);

	// RUNVGA derives its projection scales from the viewport dimensions.
	int xScale = 74 * (_viewArea.height() - 1) / 256;
	int yScale = 55 * (_viewArea.width() - 1) / 256;
	if (!xScale || !yScale)
		error("3D Construction Kit viewport is too small");
	_fieldOfView = 2.0f * Math::rad2deg(atan(24.0f / xScale));
	_viewAspectRatio = float(yScale) / xScale;

	file.skip(10);
	_maxFallingDistance = file.readUint16BE();
	_stepUpDistance = file.readUint16BE();
	_startArea = file.readUint16BE();
	_startEntrance = file.readUint16BE();
	_initialPlayerHeight = file.readUint16BE();
	uint16 step = file.readUint16BE();
	uint16 angle = file.readUint16BE();
	uint16 vehicle = file.readUint16BE();
	if (!_initialPlayerHeight || !step || !angle || angle > 90 || vehicle)
		error("Unsupported 3D Construction Kit player settings");
	_playerSteps.clear();
	_playerSteps.push_back(step);
	_playerStepIndex = 0;
	_angleRotations.clear();
	_angleRotations.push_back(angle);
	_angleRotationIndex = 0;

	file.seek(500);
	requireBytes(file, uint32(areaCount) * 4);
	Common::Array<uint32> areaOffsets;
	for (uint i = 0; i < areaCount; i++) {
		uint32 offset = file.readUint32BE();
		if (offset > uint32(file.size()) / 2)
			error("Invalid 3D Construction Kit area offset");
		areaOffsets.push_back(2 * offset);
	}
	if (globalConditions < uint32(file.pos()) || globalConditions > uint32(file.size()))
		error("Invalid 3D Construction Kit global condition offset");
	file.seek(globalConditions);
	requireBytes(file, 2);
	if (file.readUint16BE())
		error("3D Construction Kit conditions are not implemented yet");

	for (uint i = 0; i < areaOffsets.size(); i++) {
		if (areaOffsets[i] < globalConditions + 2)
			error("Invalid 3D Construction Kit area offset");
		file.seek(areaOffsets[i]);
		Area *area = loadArea(file);
		uint16 id = area->getAreaID();
		if (_areaMap.contains(id))
			error("Duplicate 3D Construction Kit area %u", id);
		_areaMap[id] = area;
	}
	if (!_areaMap.contains(_startArea) || !_areaMap[_startArea]->entranceWithID(_startEntrance))
		error("Invalid 3D Construction Kit starting area or entrance");
}

Area *KitEngine::loadArea(Common::SeekableReadStream &file) {
	uint32 start = file.pos();
	requireBytes(file, 30);
	uint16 flags = file.readUint16BE();
	uint16 objectCount = file.readUint16BE();
	uint16 id = file.readUint16BE();
	file.skip(2);
	uint32 conditions = start + 2 * file.readUint16BE();
	uint16 scale = file.readUint16BE();
	uint16 sky = file.readUint16BE();
	uint16 ground = file.readUint16BE();
	file.skip(14);
	if (!scale || scale > 255 || conditions < uint32(file.pos()) || conditions > uint32(file.size()))
		error("Invalid 3D Construction Kit area header");
	if (id == 255 && objectCount)
		error("3D Construction Kit global objects are not implemented yet");

	ObjectMap *objects = new ObjectMap();
	ObjectMap *entrances = new ObjectMap();
	Common::SeekableSubReadStream objectData(&file, file.pos(), conditions);
	for (uint i = 0; i < objectCount; i++) {
		Object *obj = loadObject(objectData);
		obj->scale(scale);
		obj->_loadIndex = i;
		ObjectMap *map = obj->getType() == kEntranceType ? entrances : objects;
		if (map->contains(obj->getObjectID()))
			error("Duplicate 3D Construction Kit object %u", obj->getObjectID());
		(*map)[obj->getObjectID()] = obj;
	}
	if (objectData.pos() != objectData.size())
		error("Invalid 3D Construction Kit object count");
	file.seek(conditions);
	requireBytes(file, 2);
	if (file.readUint16BE())
		error("3D Construction Kit conditions are not implemented yet");

	Area *area = new Area(id, flags, objects, entrances, false);
	area->_scale = scale;
	area->_name = Common::String::format("AREA %u", id);
	area->_skyColor = ((sky & 0xf) << 4) | ((sky >> 8) & 0xf);
	area->_groundColor = ((ground & 0xf) << 4) | ((ground >> 8) & 0xf);
	area->_usualBackgroundColor = 0;
	area->_underFireBackgroundColor = 0;
	return area;
}

Object *KitEngine::loadObject(Common::SeekableReadStream &file) {
	requireBytes(file, 20);
	byte flags = file.readByte();
	byte type = file.readByte();
	file.skip(2);
	Math::Vector3d origin = readVector(file);
	Math::Vector3d size = readVector(file);
	uint16 id = file.readUint16BE();
	uint16 words = file.readUint16BE();
	if (words < 10)
		error("Invalid 3D Construction Kit object size");
	requireBytes(file, 2 * (words - 10));

	if (type == kEntranceType && words == 10)
		return new Entrance(id & 0x7fff, origin, size, FCLInstructionVector(), "");
	if (type != kCubeType || words != 13 || (flags & 0x80))
		error("Unsupported 3D Construction Kit object %u (type %u)", id, type);

	Common::Array<uint8> *colors = new Common::Array<uint8>();
	for (uint i = 0; i < 3; i++) {
		// Each word interleaves the nibbles of two VGA palette indices.
		uint16 pair = file.readUint16BE();
		colors->push_back(((pair & 0xf) << 4) | ((pair >> 8) & 0xf));
		colors->push_back((pair & 0xf0) | ((pair >> 12) & 0xf));
	}
	return new GeometricObject(kCubeType, id, (flags & 4) ? 0x80 : 0,
		origin, size, colors, nullptr, nullptr, FCLInstructionVector());
}

void KitEngine::initGameState() {
	FreescapeEngine::initGameState();
	_playerHeight = _initialPlayerHeight;
}

void KitEngine::gotoArea(uint16 areaID, int entranceID) {
	if (!_areaMap.contains(areaID))
		error("Unknown 3D Construction Kit area %u", areaID);
	_currentArea = _areaMap[areaID];
	Entrance *entrance = static_cast<Entrance *>(_currentArea->entranceWithID(entranceID));
	if (!entrance)
		error("Unknown 3D Construction Kit entrance %d", entranceID);
	_position = entrance->getOrigin();
	_position.y() += _playerHeight;
	Math::Vector3d rotation = entrance->getRotation();
	_pitch = rotation.x();
	_yaw = 90.0f - rotation.y();
	_roll = rotation.z();
	_lastPosition = _position;
	_gfx->_scale = _currentArea->getScale();
	_gotoExecuted = true;
	resetInput();
}

void KitEngine::checkIfStillInArea() {
	float limit = 8192.0f / _currentArea->getScale();
	_position.x() = CLIP(_position.x(), 0.0f, limit);
	_position.z() = CLIP(_position.z(), 0.0f, limit);
}

bool KitEngine::checkIfGameEnded() {
	if (_hasFallen || _playerWasCrushed)
		_gameStateControl = kFreescapeGameStateRestart;
	return false;
}

void KitEngine::drawUI() {
	_gfx->setViewport(_fullscreenViewArea);
	_gfx->renderCrossair(_crossairPosition);
}

} // namespace Freescape
