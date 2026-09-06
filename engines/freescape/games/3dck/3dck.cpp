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

#include "common/algorithm.h"
#include "common/substream.h"
#include "math/utils.h"

#include "freescape/games/3dck/3dck.h"
#include "freescape/language/detokeniser.h"

namespace Freescape {

enum {
	kKitAnimatorType = 16,
	kKitInitiallyInvisible = 0x04,
	kKitMovable = 0x80
};

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
	int16 x = file.readSint16BE();
	int16 y = file.readSint16BE();
	int16 z = file.readSint16BE();
	return Math::Vector3d(x, y, z);
}

static Common::Array<uint16> readWords(Common::SeekableReadStream &file, uint32 count) {
	requireBytes(file, 2 * count);
	Common::Array<uint16> words;
	for (uint32 i = 0; i < count; i++)
		words.push_back(file.readUint16BE());
	return words;
}

static Common::Array<byte> readCode(Common::SeekableReadStream &file, uint32 size) {
	requireBytes(file, size);
	Common::Array<byte> code;
	code.resize(size);
	if (size && file.read(code.data(), size) != size)
		error("Truncated 3D Construction Kit condition");
	return code;
}

static void readColors(Common::SeekableReadStream &file, byte &first, byte &second) {
	requireBytes(file, 2);
	// Each word interleaves the nibbles of two VGA palette indices.
	uint16 pair = file.readUint16BE();
	first = ((pair & 0xf) << 4) | ((pair >> 8) & 0xf);
	second = (pair & 0xf0) | ((pair >> 12) & 0xf);
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
	_scriptSurface.create(_screenW, _screenH, _gfx->_texturePixelFormat);
	_scriptSurface.fillRect(_fullscreenViewArea, 0);
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

	file.skip(6);
	_timerInterval = file.readUint16BE();
	_activationRange = file.readUint16BE();
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

	file.skip(2);
	uint32 indicatorOffset = 2 * file.readUint16BE();
	uint16 indicatorCount = file.readUint16BE();
	_initialCondition = file.readUint16BE();
	if (indicatorOffset > uint32(file.size()) || (!indicatorOffset && indicatorCount))
		error("Invalid 3D Construction Kit indicator offset");
	uint32 areasEnd = indicatorOffset ? indicatorOffset : file.size();
	if (indicatorCount) {
		file.seek(indicatorOffset);
		// The indicator table follows the last area's conditions.
		_indicatorData = readWords(file, 17 * indicatorCount);
	}

	file.seek(150);
	_controlData = readWords(file, 35 * 5);
	requireBytes(file, uint32(areaCount) * 4);
	Common::Array<uint32> areaOffsets;
	for (uint i = 0; i < areaCount; i++) {
		uint32 offset = file.readUint32BE();
		if (offset >= areasEnd / 2)
			error("Invalid 3D Construction Kit area offset");
		areaOffsets.push_back(2 * offset);
	}
	Common::sort(areaOffsets.begin(), areaOffsets.end());
	if (areaOffsets.empty() || globalConditions < uint32(file.pos()) ||
			globalConditions + 2 > areaOffsets.front())
		error("Invalid 3D Construction Kit global condition offset");
	Common::SeekableSubReadStream conditionData(&file, globalConditions, areaOffsets.front());
	_globalConditions = loadConditions(conditionData);
	if (_initialCondition > _globalConditions.size())
		error("Invalid 3D Construction Kit initial condition");
	if (conditionData.pos() != conditionData.size())
		error("Invalid 3D Construction Kit global condition size");

	for (uint i = 0; i < areaOffsets.size(); i++) {
		uint32 end = i + 1 < areaOffsets.size() ? areaOffsets[i + 1] : areasEnd;
		if (areaOffsets[i] >= end)
			error("Invalid 3D Construction Kit area offset");
		Common::SeekableSubReadStream areaData(&file, areaOffsets[i], end);
		Area *area = loadArea(areaData);
		uint16 id = area->getAreaID();
		_areaMap[id] = area;
	}
	if (!_areaMap.contains(_startArea) || !_areaMap[_startArea]->entranceWithID(_startEntrance))
		error("Invalid 3D Construction Kit starting area or entrance");
}

Common::Array<KitEngine::ConditionData> KitEngine::loadConditions(Common::SeekableReadStream &file) {
	requireBytes(file, 2);
	uint16 count = file.readUint16BE();
	Common::Array<ConditionData> conditions;
	for (uint i = 0; i < count; i++) {
		requireBytes(file, 14);
		char name[13] = {};
		file.read(name, 12);
		uint16 words = file.readUint16BE() & 0x7fff;
		ConditionData condition;
		condition.name = name;
		Common::String source = detokeniseKit16Condition(readCode(file, 2 * words), condition.condition);
		debugC(1, kFreescapeDebugParser, "3DCK condition %s:\n%s", name, source.c_str());
		conditions.push_back(condition);
	}
	return conditions;
}

Area *KitEngine::loadArea(Common::SeekableReadStream &file) {
	uint32 start = file.pos();
	requireBytes(file, 30);
	uint16 flags = file.readUint16BE();
	uint16 objectCount = file.readUint16BE();
	uint16 id = file.readUint16BE();
	if (_areaMap.contains(id))
		error("Duplicate 3D Construction Kit area %u", id);
	file.skip(2);
	uint32 conditions = start + 2 * file.readUint16BE();
	uint16 scale = file.readUint16BE();
	uint16 sky = file.readUint16BE();
	uint16 ground = file.readUint16BE();
	file.skip(14);
	if (!scale || scale > 255 || conditions < uint32(file.pos()) || conditions > uint32(file.size()))
		error("Invalid 3D Construction Kit area header");

	AreaData &data = _areaData[id];
	ObjectMap *objects = new ObjectMap();
	ObjectMap *entrances = new ObjectMap();
	Common::SeekableSubReadStream objectData(&file, file.pos(), conditions);
	for (uint i = 0; i < objectCount; i++) {
		ObjectData record;
		Object *obj = loadObject(objectData, record);
		if (data.objects.contains(record.id))
			error("Duplicate 3D Construction Kit object %u in area %u", record.id, id);
		data.objects[record.id] = record;
		data.objectOrder.push_back(record.id);
		if (!obj)
			continue;
		if (id != 255)
			obj->scale(scale);
		obj->_loadIndex = (id == 255 ? 0 : 0x4000) + i;
		ObjectMap *map = obj->getType() == kEntranceType ? entrances : objects;
		if (map->contains(obj->getObjectID()))
			error("Duplicate 3D Construction Kit object %u", obj->getObjectID());
		(*map)[obj->getObjectID()] = obj;
	}
	if (objectData.pos() != objectData.size())
		error("Invalid 3D Construction Kit object count");
	file.seek(conditions);
	data.conditions = loadConditions(file);
	if (file.pos() != file.size())
		error("Invalid 3D Construction Kit area condition size");
	debugC(1, kFreescapeDebugParser, "3DCK area %u: %u objects, %u conditions", id, objectCount, data.conditions.size());

	Area *area = new Area(id, flags, objects, entrances, false);
	area->_scale = scale;
	area->_name = Common::String::format("AREA %u", id);
	area->_skyColor = ((sky & 0xf) << 4) | ((sky >> 8) & 0xf);
	area->_groundColor = ((ground & 0xf) << 4) | ((ground >> 8) & 0xf);
	area->_usualBackgroundColor = 0;
	area->_underFireBackgroundColor = 0;
	// The runner supplies a default floor at Y=0.
	if (id != 255)
		area->addFloor();
	return area;
}

Object *KitEngine::loadObject(Common::SeekableReadStream &file, ObjectData &data) {
	requireBytes(file, 20);
	data.flags = file.readByte();
	data.type = file.readByte() & 0x7f;
	data.state = file.readUint16BE();
	data.origin = readVector(file);
	data.size = readVector(file);
	data.initialOrigin = data.origin;
	data.id = file.readUint16BE();
	uint16 words = file.readUint16BE();
	if (words < 10)
		error("Invalid 3D Construction Kit object size");
	requireBytes(file, 2 * (words - 10));
	uint32 end = file.pos() + 2 * (words - 10);
	Common::SeekableSubReadStream payload(&file, file.pos(), end);
	if (data.type > kKitAnimatorType)
		error("Unsupported 3D Construction Kit object %u (type %u)", data.id, data.type);

	bool geometric = data.type >= kCubeType && data.type <= kHexagonType && data.type != kSensorType;
	Common::Array<uint8> *colors = nullptr;
	Common::Array<float> *ordinates = nullptr;
	if (geometric) {
		ObjectType type = ObjectType(data.type);
		int colorCount = GeometricObject::numberOfColoursForObjectOfType(type);
		colors = new Common::Array<uint8>();
		for (int i = 0; i < colorCount; i += 2) {
			byte first, second;
			readColors(payload, first, second);
			colors->push_back(first);
			colors->push_back(second);
		}
		int ordinateCount = GeometricObject::numberOfOrdinatesForType(type);
		if (ordinateCount) {
			requireBytes(payload, 2 * ordinateCount);
			ordinates = new Common::Array<float>();
			for (int i = 0; i < ordinateCount; i++)
				ordinates->push_back(payload.readSint16BE());
		}
	} else if (data.type == kSensorType) {
		requireBytes(payload, 10);
		readColors(payload, data.sensor.colors[0], data.sensor.colors[1]);
		data.sensor.interval = payload.readUint16BE();
		data.sensor.range = payload.readUint16BE();
		data.sensor.unknown = payload.readUint16BE();
		data.sensor.directions = payload.readUint16BE();
	} else if (data.type == kKitAnimatorType) {
		data.extra = readWords(payload, 3);
	}

	if (data.flags & kKitMovable) {
		requireBytes(payload, 6);
		data.initialOrigin = readVector(payload);
	}
	if (data.type == kGroupType || (data.type == kEntranceType && data.id == 255)) {
		data.members = readWords(payload, data.state);
		if (payload.pos() != payload.size())
			error("Invalid 3D Construction Kit object list %u", data.id);
	} else if (data.type == kEntranceType) {
		// Entrances can retain editor data after their header.
		data.extra = readWords(payload, (payload.size() - payload.pos()) / 2);
	} else {
		Common::String source = detokeniseKit16Condition(readCode(payload, payload.size() - payload.pos()), data.condition);
		debugC(1, kFreescapeDebugParser, "3DCK object %u condition:\n%s", data.id, source.c_str());
	}
	file.seek(end);
	debugC(1, kFreescapeDebugParser, "3DCK object %u: type %u, flags %02x, %u instructions",
		data.id, data.type, data.flags, data.condition.size());

	if (data.type == kEntranceType && data.id != 255)
		return new Entrance(data.id & 0x7fff, data.initialOrigin, data.size, FCLInstructionVector(), "");
	if (!geometric)
		return nullptr;

	ObjectType type = ObjectType(data.type);
	if (GeometricObject::isPolygon(type)) {
		// Polygon vertices are relative; pyramid ordinates are already offsets.
		for (uint i = 0; i < ordinates->size(); i++)
			(*ordinates)[i] += data.initialOrigin.getValue(i % 3);
	}
	return new GeometricObject(type, data.id, (data.flags & kKitInitiallyInvisible) ? 0x80 : 0,
		data.initialOrigin, data.size, colors, nullptr, ordinates, FCLInstructionVector());
}

void KitEngine::initGameState() {
	FreescapeEngine::initGameState();
	_playerHeight = _initialPlayerHeight;
	resetScripts();
	_currentArea = nullptr;
}

void KitEngine::gotoArea(uint16 areaID, int entranceID) {
	if (!_areaMap.contains(areaID))
		error("Unknown 3D Construction Kit area %u", areaID);
	if (_currentArea)
		_kitVariables[9] = _currentArea->getAreaID();
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
	_delayedShootObject = nullptr;
	_timerTicks = 0;
	_scriptSurface.fillRect(_viewArea, 0);
	resetInput();
	g_system->lockMouse(true);
	readSystemVariables();
}

void KitEngine::checkIfStillInArea() {
	float limit = 8192.0f / _currentArea->getScale();
	_position.x() = CLIP(_position.x(), 0.0f, limit);
	_position.z() = CLIP(_position.z(), 0.0f, limit);
}

bool KitEngine::checkIfGameEnded() {
	return false;
}

} // namespace Freescape
