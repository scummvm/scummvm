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
#include "common/endian.h"
#include "common/substream.h"
#include "math/utils.h"

#include "freescape/games/3dck/8bit.h"
#include "freescape/language/detokeniser.h"

namespace Freescape {

static void requireBytes(Common::SeekableReadStream &file, uint32 size) {
	if (file.err() || file.pos() > file.size() || size > file.size() - file.pos())
		error("Truncated 8-bit 3D Construction Kit data");
}

Kit8Engine::Kit8Engine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	_screenW = isSpectrum() ? 256 : 320;
	_screenH = isSpectrum() ? 192 : 200;
	_fullscreenViewArea = Common::Rect(_screenW, _screenH);
	_playerHeightNumber = _playerHeightMaxNumber = 0;
	_playerWidth = _playerDepth = 16;
	_soundIndexShoot = -1;
	_soundIndexCollide = 5;
	_soundIndexFall = 7;
}

void Kit8Engine::loadAssets() {
	Common::File dataFile;
	if (!dataFile.open(_gameDescription->filesDescriptions[0].fileName))
		error("Unable to open 8-bit 3D Construction Kit data");
	requireBytes(dataFile, 160);
	uint32 signature = dataFile.readUint32BE();
	uint32 dataOffset = 0;
	uint32 dataEnd = dataFile.size();
	if (isC64()) {
		dataFile.seek(0);
		if (dataFile.readUint16LE() != 0x0400)
			error("Invalid 3D Construction Kit C64 runner address");
		// The compiled runner embeds its world at $4a00.
		dataOffset = 2 + 0x4a00 - 0x0400;
		if (dataOffset + 160 > dataEnd)
			error("Truncated 3D Construction Kit C64 runner");
		dataFile.seek(dataOffset + 4);
		uint16 size = dataFile.readUint16LE();
		if (size < 160 || dataOffset + size > dataEnd)
			error("Invalid 3D Construction Kit C64 world size");
		dataEnd = dataOffset + size;
	} else if (isCPC() && signature != MKTAG('K', 'I', 'T', 'A') && signature != MKTAG('K', 'I', 'T', 'C')) {
		byte header[128];
		dataFile.seek(0);
		dataFile.read(header, sizeof(header));
		uint16 checksum = 0;
		for (uint i = 0; i < 67; i++)
			checksum += header[i];
		if (checksum != READ_LE_UINT16(header + 67) || READ_LE_UINT24(header + 64) != dataFile.size() - sizeof(header))
			error("Invalid 3D Construction Kit AMSDOS header");
		dataOffset = sizeof(header);
	}
	Common::SeekableSubReadStream file(&dataFile, dataOffset, dataEnd);
	requireBytes(file, 160);
	signature = file.readUint32BE();
	bool validSignature = isSpectrum() ? signature == MKTAG('K', 'I', 'T', 'S') :
		signature == MKTAG('K', 'I', 'T', 'C') || (isCPC() && signature == MKTAG('K', 'I', 'T', 'A'));
	if (!validSignature || file.readUint16LE() != file.size())
		error("Unsupported 8-bit 3D Construction Kit data format");
	uint16 procedures = file.readUint16LE();
	uint16 conditions = file.readUint16LE();
	uint16 messages = file.readUint16LE();
	byte areaCount = file.readByte();
	_climbHeight = file.readByte();
	_fallHeight = file.readByte();
	_walkSpeed = file.readByte();
	byte turnSpeed = file.readByte();
	_startArea = file.readByte();
	_startEntrance = file.readByte();
	file.skip(1);
	_activationRange = file.readByte();
	int x = 8 * file.readByte();
	int y = 8 * file.readByte();
	int width = 8 * file.readByte();
	int height = 8 * file.readByte();
	if (!width || !height || x + width > _screenW || y + height > _screenH || !_walkSpeed || !turnSpeed)
		error("Invalid 8-bit 3D Construction Kit display or movement settings");
	_viewArea = Common::Rect(x, y, x + width, y + height);
	// Projection scales: 125 * 64 / (extent - 1), with a depth scale of 18.
	int xScale = 8000 / (width - 1);
	int yScale = 8000 / (height - 1);
	if (isC64()) {
		// C64 normalizes the projection scales to 64.
		xScale = 64;
		yScale = 64 * width / height;
		if (yScale >= 64) {
			xScale = 4096 / yScale;
			yScale = 64;
		}
	}
	if (xScale > 127 || yScale > 127)
		error("Unsupported 8-bit 3D Construction Kit viewport size");
	_fieldOfView = 2 * Math::rad2deg(atan(18.0f / xScale));
	_viewAspectRatio = float(yScale) / xScale;
	_angleRotations.clear();
	_angleRotations.push_back(5 * turnSpeed);
	_angleRotationIndex = 0;
	_playerSteps.clear();
	_playerSteps.push_back(_walkSpeed);
	_playerStepIndex = 0;
	file.seek(0x30);
	file.read(_instruments, sizeof(_instruments));
	file.seek(0xa0);
	requireBytes(file, 2 * areaCount);
	Common::Array<uint16> offsets;
	for (uint i = 0; i < areaCount; i++)
		offsets.push_back(file.readUint16LE());
	Common::sort(offsets.begin(), offsets.end());
	if (offsets.empty() || offsets.back() > file.size() - 12 || messages < file.pos() ||
			messages >= procedures || procedures >= conditions || conditions >= offsets.front())
		error("Invalid 8-bit 3D Construction Kit table offsets");

	Common::SeekableSubReadStream messageData(&file, messages, procedures);
	requireBytes(messageData, 1);
	byte messageCount = messageData.readByte();
	for (uint i = 0; i < messageCount; i++) {
		requireBytes(messageData, 2);
		byte id = messageData.readByte();
		byte length = messageData.readByte();
		requireBytes(messageData, length);
		if (_kitMessages.contains(id))
			error("Duplicate 8-bit 3D Construction Kit message %u", id);
		Common::String text;
		while (length--)
			text += char(messageData.readByte());
		// C64 message lengths include the editor's terminator.
		if (isC64() && !text.empty() && byte(text.lastChar()) == 0xff)
			text.deleteLastChar();
		_kitMessages[id] = text;
	}
	Common::SeekableSubReadStream procedureData(&file, procedures, conditions);
	_procedures = loadConditions(procedureData);
	Common::SeekableSubReadStream conditionData(&file, conditions, offsets.front());
	_globalConditions = loadConditions(conditionData);
	for (uint i = 0; i < offsets.size(); i++) {
		uint32 end = i + 1 < offsets.size() ? offsets[i + 1] : file.size();
		if (offsets[i] >= end || end > uint32(file.size()))
			error("Invalid 8-bit 3D Construction Kit area offset");
		Common::SeekableSubReadStream areaData(&file, offsets[i], end);
		Area *area = loadArea(areaData);
		if (_areaMap.contains(area->getAreaID()))
			error("Duplicate 8-bit 3D Construction Kit area %u", area->getAreaID());
		_areaMap[area->getAreaID()] = area;
	}
	for (auto &entry : _areaMap) {
		if (entry._key == 255)
			continue;
		for (byte id : _areaData[entry._key].globals) {
			// The CPC runner ignores references to absent globals.
			if (!_areaMap.contains(255) || !_areaMap[255]->objectWithID(id))
				continue;
			if (entry._value->objectWithID(id))
				error("Duplicate 8-bit 3D Construction Kit global object %u", id);
			entry._value->addObjectFromArea(id, _areaMap[255]);
		}
	}
	if (!_areaMap.contains(_startArea) || !_areaMap[_startArea]->entranceWithID(_startEntrance))
		error("Invalid 8-bit 3D Construction Kit starting entrance");
	loadPresentation();
	loadSounds();
}

Common::Array<Kit8Engine::ConditionData> Kit8Engine::loadConditions(Common::SeekableReadStream &file) {
	requireBytes(file, 1);
	byte count = file.readByte();
	Common::Array<ConditionData> conditions;
	for (uint i = 0; i < count; i++) {
		requireBytes(file, 2);
		ConditionData condition;
		condition.id = file.readByte();
		byte length = file.readByte();
		requireBytes(file, length);
		for (const auto &existing : conditions) {
			if (existing.id == condition.id)
				error("Duplicate 8-bit 3D Construction Kit condition %u", condition.id);
		}
		Common::Array<byte> code;
		code.resize(length);
		file.read(code.data(), length);
		Common::String source = detokeniseKit8Condition(code, condition.condition);
		debugC(1, kFreescapeDebugParser, "Condition %u:\n%s", condition.id, source.c_str());
		conditions.push_back(condition);
	}
	return conditions;
}

Area *Kit8Engine::loadArea(Common::SeekableReadStream &file) {
	requireBytes(file, 12);
	byte id = file.readByte();
	byte count = file.readByte();
	byte flags = file.readByte();
	uint16 conditions = file.readUint16LE();
	file.skip(2);
	AreaData &data = _areaData[id];
	for (uint i = 0; i < 4; i++) {
		data.palette[i] = file.readByte();
		if (data.palette[i] > (isSpectrum() ? (i == 2 ? 1 : 7) : isC64() ? 15 : 26))
			error("Invalid 8-bit 3D Construction Kit palette");
	}
	byte scale = file.readByte();
	if (!scale || conditions < file.pos() || conditions > file.size())
		error("Invalid 8-bit 3D Construction Kit area header");
	ObjectMap *objects = new ObjectMap;
	ObjectMap *entrances = new ObjectMap;
	for (uint i = 0; i < count; i++) {
		requireBytes(file, 9);
		uint32 start = file.pos();
		byte header[9];
		file.read(header, sizeof(header));
		byte type = header[0] & 0x0f;
		byte objectID = header[7];
		byte size = header[8];
		if (id == 255 && !objectID && !size) {
			// Ciudadela Fantasma ends its globals with an unused, incomplete record.
			warning("Ignoring incomplete 8-bit 3D Construction Kit global object");
			file.seek(conditions);
			break;
		}
		if (size < 9 || start + size > conditions)
			error("Invalid 8-bit 3D Construction Kit object size");
		if (objectID == 255) {
			for (uint j = 9; j < size; j++)
				data.globals.push_back(file.readByte());
			continue;
		}
		if (objects->contains(objectID) || entrances->contains(objectID))
			error("Duplicate 8-bit 3D Construction Kit object %u", objectID);
		Object *object = nullptr;
		if (type == kEntranceType) {
			if (size != 12)
				error("Invalid 8-bit 3D Construction Kit entrance size");
			Math::Vector3d origin, rotation;
			for (int axis = 0; axis < 3; axis++) {
				byte fine = file.readByte();
				origin.setValue(axis, header[axis + 1] == 255 ? -1 : 32 * header[axis + 1] + fine / 2.0f);
				rotation.setValue(axis, header[axis + 4] == 255 ? -1 : 5 * header[axis + 4]);
			}
			object = new Entrance(objectID, origin, rotation, FCLInstructionVector(), "");
		} else if (type == kSensorType) {
			if (size != 10)
				error("Invalid 8-bit 3D Construction Kit sensor size");
			byte color = file.readByte();
			object = new Sensor(objectID, Math::Vector3d(32 * header[1], 32 * header[2], 32 * header[3]),
				Math::Vector3d(), color & 15, header[5], 32 * header[4], 0, header[0], FCLInstructionVector(), "");
		} else
			object = loadGeometricObject(file, header);
		file.seek(start + size);
		object->_loadIndex = (id == 255 ? 0 : 0x4000) + i;
		if (id != 255)
			object->scale(scale);
		if (type == kEntranceType)
			(*entrances)[objectID] = object;
		else
			(*objects)[objectID] = object;
	}
	if (file.pos() != conditions)
		error("Invalid 8-bit 3D Construction Kit object table length");
	if (id != 255)
		data.conditions = loadConditions(file);
	Area *area = new Area(id, flags, objects, entrances, false);
	area->_scale = scale;
	area->_name = Common::String::format("Area %u", id);
	area->_groundColor = 255;
	return area;
}

GeometricObject *Kit8Engine::loadGeometricObject(Common::SeekableReadStream &file, const byte header[9]) {
	ObjectType type = ObjectType(header[0] & 0x0f);
	int colorCount = GeometricObject::numberOfColoursForObjectOfType(type);
	int ordinateCount = GeometricObject::numberOfOrdinatesForType(type);
	if (type > kHexagonType || header[8] != 9 + colorCount / 2 + ordinateCount)
		error("Unsupported 8-bit 3D Construction Kit object type %u", type);
	requireBytes(file, colorCount / 2 + ordinateCount);
	Math::Vector3d origin(32 * header[1], 32 * header[2], 32 * header[3]);
	Math::Vector3d size(32 * header[4], 32 * header[5], 32 * header[6]);
	Common::Array<byte> *colors = new Common::Array<byte>;
	for (int i = 0; i < colorCount / 2; i++) {
		byte color = file.readByte();
		colors->push_back(color & 15);
		colors->push_back(color >> 4);
	}
	if (type == kCubeType) {
		// CPC Kit stores the positive X face first.
		SWAP((*colors)[0], (*colors)[1]);
	} else if (GeometricObject::isPyramid(type)) {
		// Kit stores opposite sides together; the renderer walks around the base.
		const byte sides[] = {(*colors)[2], (*colors)[0], (*colors)[3], (*colors)[1]};
		for (uint i = 0; i < ARRAYSIZE(sides); i++)
			(*colors)[i] = sides[i];
	}
	Common::Array<float> *ordinates = nullptr;
	if (ordinateCount) {
		static const byte pyramidAxes[3][2] = {{1, 2}, {0, 2}, {0, 1}};
		ordinates = new Common::Array<float>;
		for (int i = 0; i < ordinateCount; i++) {
			int axis = i % 3;
			if (GeometricObject::isPyramid(type))
				axis = pyramidAxes[(type - kEastPyramidType) / 2][i % 2];
			// Kit points use 1/64 of the corresponding bounding-box dimension.
			float ordinate = size.getValue(axis) * file.readByte() / 64.0f;
			if (GeometricObject::isPolygon(type))
				ordinate += origin.getValue(axis);
			ordinates->push_back(ordinate);
		}
	}
	return new GeometricObject(type, header[7], header[0], origin, size,
		colors, nullptr, ordinates, FCLInstructionVector(), "");
}

void Kit8Engine::initGameState() {
	FreescapeEngine::initGameState();
	resetScripts();
	_currentArea = nullptr;
	_movementMode = 1;
	_playerHeight = 0;
}

void Kit8Engine::gotoArea(uint16 areaID, int entranceID) {
	if (!_areaMap.contains(areaID) || areaID == 255)
		error("Unknown 8-bit 3D Construction Kit area %u", areaID);
	float oldScale = _currentArea ? _currentArea->getScale() : 1;
	_currentArea = _areaMap[areaID];
	float scale = _currentArea->getScale();
	Math::Vector3d position = _position * (oldScale / scale);
	_playerHeight = 0;
	setMovementMode(_movementMode);
	_position = position;
	Entrance *entrance = static_cast<Entrance *>(_currentArea->entranceWithID(entranceID));
	if (entrance) {
		Math::Vector3d origin = entrance->getOrigin(), rotation = entrance->getRotation();
		for (int axis = 0; axis < 3; axis++) {
			if (origin.getValue(axis) >= 0)
				_position.setValue(axis, origin.getValue(axis) + (axis == 1 ? _playerHeight : 0));
		}
		if (rotation.x() >= 0)
			_pitch = rotation.x() > 180 ? rotation.x() - 360 : rotation.x();
		if (rotation.y() >= 0)
			_yaw = 90 - rotation.y();
		if (rotation.z() >= 0)
			_roll = rotation.z();
	}
	AreaData &data = _areaData[areaID];
	memcpy(_palette, data.palette, sizeof(_palette));
	for (byte id : data.globals) {
		Object *object = _currentArea->objectWithID(id);
		if (!object)
			continue;
		object->restore();
		object->makeVisible();
	}
	applyPalette();
	_sensors = _currentArea->getSensors();
	_gfx->_scale = scale;
	_gotoExecuted = true;
	_lastPosition = _position;
	float pitch = _pitch;
	resetInput();
	_pitch = pitch;
	updateCamera();
	g_system->lockMouse(true);
	readSystemVariables();
}

void Kit8Engine::setMovementMode(byte mode) {
	if (mode > 4)
		return;
	_position.y() -= _playerHeight;
	_movementMode = mode;
	_flyMode = mode >= 3;
	float scale = _currentArea ? _currentArea->getScale() : 1;
	_playerHeight = _flyMode ? 0 : (mode == 0 ? 64 : 128) - 26 / scale;
	_position.y() += _playerHeight;
	_playerSteps[0] = mode == 0 ? _walkSpeed / 2 : mode == 2 ? 2 * _walkSpeed : _walkSpeed;
	_stepUpDistance = 32 * _climbHeight;
	_maxFallingDistance = 32 * _fallHeight;
	_lastPosition = _position;
}

Math::Vector3d Kit8Engine::clipPosition(const Math::Vector3d &position) const {
	float scale = _currentArea->getScale();
	// Walking bounds apply to the feet, before restoring the eye height.
	float height = _flyMode ? 0 : _playerHeight;
	return Math::Vector3d(CLIP(position.x(), 0.0f, 4063.5f / scale),
		CLIP(position.y() - height, 0.0f, 2015.5f / scale) + height,
		CLIP(position.z(), 0.0f, 4063.5f / scale));
}

void Kit8Engine::checkIfStillInArea() {
	_position = clipPosition(_position);
}

void Kit8Engine::updatePlayerMovement(float deltaTime) {
	if (!isFrameReady() || _scriptFrameActive || _initialScriptPending || (isSpectrum() && isPlayingSound()))
		return;
	Math::Vector3d front = _cameraFront;
	if (_movementMode == 3)
		_cameraFront = directionToVector(0, _yaw, false);
	FreescapeEngine::updatePlayerMovement(kFrameDuration / 1000.0f);
	_cameraFront = front;
}

void Kit8Engine::pauseEngineIntern(bool pause) {
	uint32 now = g_system->getMillis();
	if (pause)
		_pauseStartTime = now;
	else {
		uint32 elapsed = now - _pauseStartTime;
		_lastTime += elapsed;
		_nextFrameTime += elapsed;
		if (_delayUntil)
			_delayUntil += elapsed;
	}
	FreescapeEngine::pauseEngineIntern(pause);
}

void Kit8Engine::checkSensors() {
	// TODO: sensor firing.
	if (!isFrameReady() || _scriptFrameActive || !_currentArea)
		return;
	for (auto *object : _sensors) {
		Sensor *sensor = static_cast<Sensor *>(object);
		Math::Vector3d diff = _position - sensor->getOrigin();
		bool detected = !sensor->isInvisible() && !sensor->isDestroyed() &&
			ABS(diff.x()) + ABS(diff.y()) + ABS(diff.z()) < sensor->_firingRange;
		sensor->shouldShoot(detected);
	}
}

} // namespace Freescape
