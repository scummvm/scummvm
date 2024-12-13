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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#include "common/memstream.h"
#include "common/file.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/objects/connections.h"
#include "freescape/objects/global.h"
#include "freescape/objects/group.h"
#include "freescape/objects/sensor.h"

namespace Freescape {

uint16 FreescapeEngine::readPtr(Common::SeekableReadStream *file) {
	uint16 value;
	if (isAmiga() || isAtariST()) {
		uint16 lo = file->readUint16BE();
		assert(lo < 256);
		uint16 hi = file->readUint16BE();
		assert(hi < 256);
		value = 256 * hi + lo;
		if (value >= 0xFFFF / 2)
			error("Failed to read pointer with value 0x%x", value);
		value = 2 * value;
	} else
		value = file->readUint16LE();
	return value;
}

uint16 FreescapeEngine::readField(Common::SeekableReadStream *file, int bits) {
	uint16 value;
	assert(bits == 8 || bits == 16);
	if (isAmiga() || isAtariST()) {
		if (bits == 16) {
			value = file->readUint16BE();
		} else {
			assert(bits == 8);
			value = file->readUint16BE();
			if (value >= 256) {
				warning("failed to read byte with value 0x%x", value);
				value = value & 0xff;
			}
		}
	} else {
		if (bits == 8)
			value = file->readByte();
		else
			value = file->readUint16LE();
	}

	return value;
}

Common::Array<uint16> FreescapeEngine::readArray(Common::SeekableReadStream *file, int size) {
	Common::Array<uint16> array;

	for (int i = 0; i < size; i++) {
		if (isAmiga() || isAtariST()) {
			uint16 value = file->readUint16BE();
			array.push_back(value);
		} else
			array.push_back(readField(file, 8));
	}
	return array;
}

Group *FreescapeEngine::load8bitGroup(Common::SeekableReadStream *file, byte rawFlagsAndType) {
	if (isDark() || isEclipse())
		return load8bitGroupV1(file, rawFlagsAndType);
	else
		return load8bitGroupV2(file, rawFlagsAndType);
}

Group *FreescapeEngine::load8bitGroupV1(Common::SeekableReadStream *file, byte rawFlagsAndType) {
	debugC(1, kFreescapeDebugParser, "Object of type 'group'");
	Common::Array<AnimationOpcode *> animation;
	Common::Array<uint16> groupObjects = readArray(file, 3);
	Math::Vector3d offset1;
	Math::Vector3d offset2;

	for (int i = 0; i < 3; i++) {
		uint16 value = 0;
		if (isAmiga() || isAtariST())
			value = readField(file, 16);
		else
			value = readField(file, 8);

		if (value > 127)
			value = value - 255;

		debugC(1, kFreescapeDebugParser, "Group offset[1][%d] = %d", i, value);
		offset1.setValue(i, value);
	}

	// object ID
	uint16 objectID = readField(file, 8);
	// size of object on disk; we've accounted for 8 bytes
	// already so we can subtract that to get the remaining
	// length beyond here
	uint8 byteSizeOfObject = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Raw object %d ; type group ; size %d", objectID, byteSizeOfObject);
	if (byteSizeOfObject < 9) {
		error("Not enough bytes %d to read object %d with type group", byteSizeOfObject, objectID);
	}

	assert(byteSizeOfObject >= 9);
	byteSizeOfObject = byteSizeOfObject - 9;
	for (int i = 0; i < 3; i++) {
		uint16 value = 0;
		if (isAmiga() || isAtariST())
			value = readField(file, 16);
		else
			value = readField(file, 8);

		if (value > 127)
			value = value - 255;

		debugC(1, kFreescapeDebugParser, "Group offset[2][%d] = %d", i, value);
		offset2.setValue(i, value);
	}

	byteSizeOfObject = byteSizeOfObject - 3;
	for (int i = 0; i < 3; i++)
		debugC(1, kFreescapeDebugParser, "Group object[%d] = %d", i, groupObjects[i]);

	Common::Array<uint16> groupOperations;
	Common::Array<Math::Vector3d> groupPositions;
	while (byteSizeOfObject > 0) {
		uint16 value = 0;
		if (isAmiga() || isAtariST())
			value = readField(file, 16);
		else
			value = readField(file, 8);

		int opcode = value >> 8;
		debugC(1, kFreescapeDebugParser, "Reading opcode: %x", opcode);
		AnimationOpcode* operation = new AnimationOpcode(opcode);
		byteSizeOfObject--;
		if (opcode == 0xff) {
			assert(value == 0xffff);
			debugC(1, kFreescapeDebugParser, "Group operation rewind");
		} else if (opcode == 0x01) {
			debugC(1, kFreescapeDebugParser, "Group operation script execution");
			// get the length
			uint32 lengthOfCondition = value & 0xff;
			assert(lengthOfCondition > 0);
			debugC(1, kFreescapeDebugParser, "Length of condition: %d at %lx", lengthOfCondition, long(file->pos()));
			// get the condition
			Common::Array<uint16> conditionArray = readArray(file, lengthOfCondition);
			operation->conditionSource = detokenise8bitCondition(conditionArray, operation->condition, isAmiga() || isAtariST());
			debugC(1, kFreescapeDebugParser, "%s", operation->conditionSource.c_str());
			byteSizeOfObject = byteSizeOfObject - lengthOfCondition;
		} else {
			if (byteSizeOfObject >= 1) {
				operation->position.x() = value & 0xff;
				operation->position.y() = file->readByte();
				operation->position.z() = file->readByte();
				debugC(1, kFreescapeDebugParser, "Group operation %d move to: %f %f %f", opcode, operation->position.x(), operation->position.y(), operation->position.z());
				byteSizeOfObject = byteSizeOfObject - 1;
			} else {
				debugC(1, kFreescapeDebugParser, "Incomplete group operation %d", opcode);
				byteSizeOfObject = 0;
				delete operation;
				continue;
			}
		}
		animation.push_back(operation);
	}

	return new Group(
		objectID,
		rawFlagsAndType,
		groupObjects,
		offset1,
		offset2,
		animation);
}


Group *FreescapeEngine::load8bitGroupV2(Common::SeekableReadStream *file, byte rawFlagsAndType) {
	debugC(1, kFreescapeDebugParser, "Object of type 'group'");
	Common::Array<AnimationOpcode *> animation;
	Common::Array<uint16> groupObjects = readArray(file, 3);
	Math::Vector3d offset1;
	Math::Vector3d offset2;

	for (int i = 0; i < 3; i++) {
		int16 value = 0;
		if (isAmiga() || isAtariST())
			value = readField(file, 16);
		else
			value = readField(file, 8);

		if (value > 127)
			value = value - 255;

		debugC(1, kFreescapeDebugParser, "Group offset[1][%d] = %d", i, value);
		offset1.setValue(i, value);
	}

	// object ID
	uint16 objectID = readField(file, 8);
	// size of object on disk; we've accounted for 8 bytes
	// already so we can subtract that to get the remaining
	// length beyond here
	uint8 byteSizeOfObject = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Raw object %d ; type group ; size %d", objectID, byteSizeOfObject);
	if (byteSizeOfObject < 9) {
		error("Not enough bytes %d to read object %d with type group", byteSizeOfObject, objectID);
	}

	assert(byteSizeOfObject >= 9);
	byteSizeOfObject = byteSizeOfObject - 9;

	for (int i = 0; i < 3; i++) {
		int16 value = 0;
		if (isAmiga() || isAtariST())
			value = readField(file, 16);
		else
			value = readField(file, 8);

		if (value > 127)
			value = value - 255;

		debugC(1, kFreescapeDebugParser, "Group offset[2][%d] = %d", i, value);
		offset2.setValue(i, value);
	}

	byteSizeOfObject = byteSizeOfObject - 3;
	for (int i = 0; i < 3; i++)
		debugC(1, kFreescapeDebugParser, "Group object[%d] = %d", i, groupObjects[i]);

	Common::Array<uint16> groupOperations;
	Common::Array<Math::Vector3d> groupPositions;
	while (byteSizeOfObject > 0) {
		uint16 opcode = 0;
		if (isAmiga() || isAtariST())
			opcode = readField(file, 16);
		else
			opcode = readField(file, 8);

		AnimationOpcode* operation = new AnimationOpcode(opcode);

		byteSizeOfObject--;
		if (opcode == 0x80) {
			debugC(1, kFreescapeDebugParser, "Group operation rewind");
		} else if (opcode == 0x01) {
			debugC(1, kFreescapeDebugParser, "Group operation script execution");
			// get the length
			uint32 lengthOfCondition = readField(file, 8);
			assert(lengthOfCondition > 0);
			byteSizeOfObject--;
			debugC(1, kFreescapeDebugParser, "Length of condition: %d at %lx", lengthOfCondition, long(file->pos()));
			// get the condition
			Common::Array<uint16> conditionArray = readArray(file, lengthOfCondition);
			operation->conditionSource = detokenise8bitCondition(conditionArray, operation->condition, isAmiga() || isAtariST());
			debugC(1, kFreescapeDebugParser, "%s", operation->conditionSource.c_str());
			byteSizeOfObject = byteSizeOfObject - lengthOfCondition;
		} else {
			if (byteSizeOfObject >= 3) {
				operation->position.x() = readField(file, 8);
				operation->position.y() = readField(file, 8);
				operation->position.z() = readField(file, 8);
				debugC(1, kFreescapeDebugParser, "Group operation %d move to: %f %f %f", opcode, operation->position.x(), operation->position.y(), operation->position.z());
				byteSizeOfObject = byteSizeOfObject - 3;
			} else {
				byteSizeOfObject = 0;
				delete operation;
				continue;
			}
		}
		animation.push_back(operation);
	}

	return new Group(
		objectID,
		rawFlagsAndType,
		groupObjects,
		offset1,
		offset2,
		animation);
}

Object *FreescapeEngine::load8bitObject(Common::SeekableReadStream *file) {

	byte rawFlagsAndType = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Raw object data flags and type: %d", rawFlagsAndType);
	ObjectType objectType = (ObjectType)(rawFlagsAndType & 0x1F);

	if (objectType == ObjectType::kGroupType)
		return load8bitGroup(file, rawFlagsAndType);

	Math::Vector3d position, v;

	position.x() = readField(file, 8);
	position.y() = readField(file, 8);
	position.z() = readField(file, 8);

	v.x() = readField(file, 8);
	v.y() = readField(file, 8);
	v.z() = readField(file, 8);

	// object ID
	uint16 objectID = readField(file, 8);

	if (objectID == 224 && (rawFlagsAndType & 0x1F) == 29) // If objectType is out of range, fix it
		objectType = (ObjectType)7;

	// size of object on disk; we've accounted for 8 bytes
	// already so we can subtract that to get the remaining
	// length beyond here
	uint8 byteSizeOfObject = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Raw object %d ; type %d ; size %d", objectID, (int)objectType, byteSizeOfObject);
	if (byteSizeOfObject < 9) {
		error("Not enough bytes %d to read object %d with type %d", byteSizeOfObject, objectID, objectType);
	}

	if (objectType > ObjectType::kGroupType && isDemo() && isCastle()) {
		// Castle DOS demo has an invalid object, which should not be parsed.
		debugC(1, kFreescapeDebugParser, "WARNING: invalid object %d!", objectID);
		readArray(file, byteSizeOfObject - 9);
		return nullptr;
	}
	assert(objectType <= ObjectType::kGroupType);
	assert(byteSizeOfObject >= 9);
	byteSizeOfObject = byteSizeOfObject - 9;
	if (objectID == 255 && objectType == ObjectType::kEntranceType) {
		debugC(1, kFreescapeDebugParser, "Found the room structure (objectID: 255 with size %d)", byteSizeOfObject + 6);
		Common::Array<uint8> structureArray;
		structureArray.push_back(uint8(position.x()));
		structureArray.push_back(uint8(position.y()));
		structureArray.push_back(uint8(position.z()));

		structureArray.push_back(uint8(v.x()));
		structureArray.push_back(uint8(v.y()));
		structureArray.push_back(uint8(v.z()));

		byteSizeOfObject++;
		while(--byteSizeOfObject > 0)
			structureArray.push_back(readField(file, 8));
		return new GlobalStructure(structureArray);
	} else if (objectID == 254 && objectType == ObjectType::kEntranceType) {
		debugC(1, kFreescapeDebugParser, "Found the area connections (objectID: 254 with size %d)", byteSizeOfObject + 6);
		Common::Array<uint8> connectionsArray;
		connectionsArray.push_back(uint8(position.x()));
		connectionsArray.push_back(uint8(position.y()));
		connectionsArray.push_back(uint8(position.z()));

		connectionsArray.push_back(uint8(v.x()));
		connectionsArray.push_back(uint8(v.y()));
		connectionsArray.push_back(uint8(v.z()));

		byteSizeOfObject++;
		while(--byteSizeOfObject > 0)
			connectionsArray.push_back(readField(file, 8));
		return new AreaConnections(connectionsArray);
	}

	debugC(1, kFreescapeDebugParser, "Object %d ; type %d ; size %d", objectID, (int)objectType, byteSizeOfObject);
	debugC(1, kFreescapeDebugParser, "pos: %f %f %f", position.x(), position.y(), position.z());
	switch (objectType) {
	default: {
		debugC(1, kFreescapeDebugParser, "size: %f %f %f", v.x(), v.y(), v.z());
		// read the appropriate number of colours
		int numberOfColours = GeometricObject::numberOfColoursForObjectOfType(objectType);
		Common::Array<uint8> *colours = new Common::Array<uint8>;
		Common::Array<uint8> *ecolours = nullptr;

		if (!isDriller() && (isAmiga() || isAtariST()))
			ecolours = new Common::Array<uint8>;

		debugC(1, kFreescapeDebugParser, "Number of colors: %d", numberOfColours / 2);
		uint8 entry;
		for (uint8 colour = 0; colour < numberOfColours / 2; colour++) {
			uint8 data = 0;
			uint8 extraData = 0;
			if (!isDriller() && (isAmiga() || isAtariST())) {
				uint16 field = file->readUint16BE();
				data = field & 0xff;
				extraData = field >> 8;
			} else
				data = readField(file, 8);

			entry = data & 0xf;

			colours->push_back(entry);
			debugC(1, kFreescapeDebugParser, "color[%d] = %x", 2 * colour, entry);
			if (!isDriller() && (isAmiga() || isAtariST())) {
				ecolours->push_back(extraData & 0xf);
				debugC(1, kFreescapeDebugParser, "ecolor[%d] = %x", 2 * colour, extraData & 0xf);
			}

			entry = data >> 4;
			colours->push_back(entry);
			debugC(1, kFreescapeDebugParser, "color[%d] = %x", 2 * colour + 1, entry);
			if (!isDriller() && (isAmiga() || isAtariST())) {
				ecolours->push_back(extraData >> 4);
				debugC(1, kFreescapeDebugParser, "ecolor[%d] = %x", 2 * colour + 1, extraData >> 4);
			}

			byteSizeOfObject--;
		}

		// read extra vertices if required...
		int numberOfOrdinates = GeometricObject::numberOfOrdinatesForType(objectType);
		debugC(1, kFreescapeDebugParser, "number of ordinates %d", numberOfOrdinates);
		Common::Array<float> *ordinates = nullptr;

		if (numberOfOrdinates) {
			ordinates = new Common::Array<float>;
			uint16 ord = 0;
			if (byteSizeOfObject < numberOfOrdinates) {
				error("Not enough bytes to read all the ordinates");
				// file->seek(byteSizeOfObject, SEEK_CUR);
				// return nullptr;
			}
			for (int ordinate = 0; ordinate < numberOfOrdinates; ordinate++) {
				ord = readField(file, 8);
				debugC(1, kFreescapeDebugParser, "ord: %x", ord);
				ordinates->push_back(32 * ord);
				byteSizeOfObject--;
			}
		}

		// grab the object condition, if there is one
		FCLInstructionVector instructions;
		Common::String conditionSource;
		if (byteSizeOfObject) {
			Common::Array<uint16> conditionArray = readArray(file, byteSizeOfObject);
			conditionSource = detokenise8bitCondition(conditionArray, instructions, isAmiga() || isAtariST());
			// instructions = getInstructions(conditionSource);
			debugC(1, kFreescapeDebugParser, "%s", conditionSource.c_str());
		}
		debugC(1, kFreescapeDebugParser, "End of object at %lx", long(file->pos()));

		// create an object
		return new GeometricObject(
			objectType,
			objectID,
			rawFlagsAndType, // flags
			32 * position,
			32 * v, // size
			colours,
			ecolours,
			ordinates,
			instructions,
			conditionSource);
	} break;
	case kEntranceType: {
		debugC(1, kFreescapeDebugParser, "rotation: %f %f %f", v.x(), v.y(), v.z());
		FCLInstructionVector instructions;
		Common::String conditionSource;

		if (byteSizeOfObject > 0) {
			if (!isCastle()) {
				debugC(1, kFreescapeDebugParser, "Warning: extra %d bytes in entrance", byteSizeOfObject);
				while (byteSizeOfObject--)
					debugC(1, kFreescapeDebugParser, "b: %x", readField(file, 8));
			} else {
				Common::Array<uint16> conditionArray = readArray(file, byteSizeOfObject);
				conditionSource = detokenise8bitCondition(conditionArray, instructions, isAmiga() || isAtariST());
				debugC(1, kFreescapeDebugParser, "Entrance condition:");
				debugC(1, kFreescapeDebugParser, "%s", conditionSource.c_str());
			}
			byteSizeOfObject = 0;
		}
		assert(byteSizeOfObject == 0);
		debugC(1, kFreescapeDebugParser, "End of object at %lx", long(file->pos()));

		if (isCastle()) {

			if (position.x() == 255)
				position.x() = -8096;
			else
				position.x() = 32 * position.x();

			if (position.y() == 255)
				position.y() = -8096;
			else
				position.y() = 32 * position.y();

			if (position.z() == 255)
				position.z() = -8096;
			else
				position.z() = 32 * position.z();

			if (v.x() == 255 && v.y() == 255 && v.z() == 255) {
				v.x() = -8096;
				v.y() = -8096;
				v.z() = -8096;
			} else {
				v.x() = 5 * v.x();
				v.y() = 5 * v.y();
				v.z() = 5 * v.z();
			}

		} else {
			v = 5 * v;
			position = 32 * position;
		}

		// create an entrance
		return new Entrance(
			objectID,
			position,
			v, // rotation
			instructions,
			conditionSource);
	} break;

	case kSensorType: {
		debugC(1, kFreescapeDebugParser, "rotation: %f %f %f", v.x(), v.y(), v.z());
		FCLInstructionVector instructions;
		Common::String conditionSource;

		if (isCastle()) { // TODO
			assert(byteSizeOfObject == 0);
			return new Sensor(
				objectID,
				32 * position,
				5 * v,
				0,
				0,
				0,
				0,
				rawFlagsAndType,
				instructions,
				conditionSource);
		}

		assert(byteSizeOfObject >= 5);
		byte color = readField(file, 8) & 0xf;
		assert(color > 0);
		byte firingInterval = readField(file, 8);
		uint16 firingRange = readPtr(file) / 2;
		if (isDark())
			firingRange = firingRange / 2;
		byte sensorAxis = readField(file, 8);
		byteSizeOfObject = byteSizeOfObject - 5;
		debugC(1, kFreescapeDebugParser, "Sensor detected with firing interval %d, firing range %d and axis %d", firingInterval, firingRange, sensorAxis);
		// grab the object condition, if there is one
		if (byteSizeOfObject) {
			Common::Array<uint16> conditionArray = readArray(file, byteSizeOfObject);
			conditionSource = detokenise8bitCondition(conditionArray, instructions, isAmiga() || isAtariST());
			debugC(1, kFreescapeDebugParser, "%s", conditionSource.c_str());
		}
		debugC(1, kFreescapeDebugParser, "End of object at %lx", long(file->pos()));
		// create an entrance
		return new Sensor(
			objectID,
			32 * position,
			5 * v, // rotation?
			color,
			firingInterval,
			firingRange,
			sensorAxis,
			rawFlagsAndType,
			instructions,
			conditionSource);
	} break;

	case kGroupType:
		error("Unreachable");
		break;
	}
	// Unreachable
}

static const char *eclipseRoomName[] = {
	"* SAHARA",
	"HORAKHTY",
	"NEPHTHYS",
	"KHEPRESH",
	" RAMESES",
	"PHARAOHS",
	" SHABAKA",
	"ILLUSION",
	"????????"};

Area *FreescapeEngine::load8bitArea(Common::SeekableReadStream *file, uint16 ncolors) {

	Common::String name;
	uint32 base = file->pos();
	debugC(1, kFreescapeDebugParser, "Area base: %x", base);
	uint8 areaFlags = readField(file, 8);
	uint8 numberOfObjects = readField(file, 8);
	uint8 areaNumber = readField(file, 8);

	uint16 cPtr = readPtr(file);
	debugC(1, kFreescapeDebugParser, "Condition pointer: %x", cPtr);
	uint8 scale = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Scale: %d", scale);

	uint8 skyColor = areaFlags & 15;
	uint8 groundColor = areaFlags >> 4;

	if (groundColor == 0)
		groundColor = 255;

	uint8 usualBackgroundColor = 0;
	uint8 underFireBackgroundColor = 0;
	uint8 paperColor = 0;
	uint8 inkColor = 0;

	if (!(isCastle() && (isSpectrum() || isCPC()))) {
		usualBackgroundColor = readField(file, 8);
		underFireBackgroundColor = readField(file, 8);
		paperColor = readField(file, 8);
		inkColor = readField(file, 8);
	} else {
		uint8 attribute = readField(file, 8);
		debugC(1, kFreescapeDebugParser, "Attribute: %x", attribute);
		if (isSpectrum()) {
			paperColor = attribute >> 4;
			inkColor = attribute & 0xf;
		} else if (isCPC()) {
			paperColor = attribute;
			inkColor = 0xb;
		}
		skyColor = 0;
	}

	debugC(1, kFreescapeDebugParser, "Colors usual background: %d", usualBackgroundColor);
	debugC(1, kFreescapeDebugParser, "Colors under fire background: %d", underFireBackgroundColor);
	debugC(1, kFreescapeDebugParser, "Color Paper: %d", paperColor);
	debugC(1, kFreescapeDebugParser, "Color Ink: %d", inkColor);
	debugC(1, kFreescapeDebugParser, "Additional colors: %d %d", skyColor, groundColor);

	debugC(1, kFreescapeDebugParser, "Area %d", areaNumber);
	debugC(1, kFreescapeDebugParser, "Flags: %d Objects: %d", areaFlags, numberOfObjects);
	// debug("Condition Ptr: %x", cPtr);
	debugC(1, kFreescapeDebugParser, "Pos before first object: %lx", long(file->pos()));

	// Driller specific
	uint8 gasPocketX = 0;
	uint8 gasPocketY = 0;
	uint8 gasPocketRadius = 0;
	// Castle specific
	uint8 extraColor[4] = {};
	if (isEclipse()) {
		byte idx = readField(file, 8);
		name = idx < 8 ? eclipseRoomName[idx] : eclipseRoomName[8];
		name = name + "-" + char(readField(file, 8)) + " ";

		int i = 0;
		while (i < 3) {
			name = name + char(readField(file, 8));
			i++;
		}

		if (isAmiga() || isAtariST()) {
			groundColor = skyColor;
			skyColor = 0;
		}
	} else if (isDriller() || isDark()) {
		if (isDriller()) {
			gasPocketX = readField(file, 8);
			gasPocketY = readField(file, 8);
			gasPocketRadius = readField(file, 8);
		} else {
			name = name + char(readField(file, 8));
			name = name + char(readField(file, 8));
			name = name + char(readField(file, 8));
		}
		debugC(1, kFreescapeDebugParser, "Gas pocket at (%d, %d) with radius %d", gasPocketX, gasPocketY, gasPocketRadius);
		int i = 0;
		while (i < 12) {
			name = name + char(readField(file, 8));
			i++;
		}
	} else if (isCastle()) {
		byte idx = readField(file, 8);
		if (isAmiga())
			name = _messagesList[idx + 51];
		else if (isSpectrum() || isCPC())
			name = areaNumber == 255 ? "GLOBAL" : _messagesList[idx + 16];
		else
			name = _messagesList[idx + 41];

		if (isDOS()) {
			extraColor[0] = readField(file, 8);
			extraColor[1] = readField(file, 8);
			extraColor[2] = readField(file, 8);
			extraColor[3] = readField(file, 8);
			debugC(1, kFreescapeDebugParser, "Extra colors: %x %x %x %x", extraColor[0], extraColor[1], extraColor[2], extraColor[3]);
		}

		if (isAmiga()) {
			extraColor[0] = readField(file, 8);
			extraColor[1] = readField(file, 8);
			extraColor[2] = readField(file, 8);
			extraColor[3] = readField(file, 8);
			// TODO
			groundColor = skyColor;
			skyColor = 0;
			debugC(1, kFreescapeDebugParser, "Extra colors: %x %x %x %x", extraColor[0], extraColor[1], extraColor[2], extraColor[3]);
		}
	}
	debugC(1, kFreescapeDebugParser, "Area name: %s", name.c_str());

	ObjectMap *objectsByID = new ObjectMap;
	ObjectMap *entrancesByID = new ObjectMap;
	for (uint8 object = 0; object < numberOfObjects && areaNumber != 192; object++) {
		debugC(1, kFreescapeDebugParser, "Reading object: %d", object);
		Object *newObject = load8bitObject(file);

		if (newObject) {
			newObject->scale(scale);
			if (newObject->getType() == kEntranceType) {
				if (entrancesByID->contains(newObject->getObjectID() & 0x7fff))
					error("WARNING: replacing object id %d (%d)", newObject->getObjectID(), newObject->getObjectID() & 0x7fff);

				(*entrancesByID)[newObject->getObjectID() & 0x7fff] = newObject;
			} else {
				if (objectsByID->contains(newObject->getObjectID()))
					error("WARNING: replacing object id %d", newObject->getObjectID());
				(*objectsByID)[newObject->getObjectID()] = newObject;
			}
		} else if (!(isDemo() && isCastle()))
			error("Failed to read an object!");
	}

	// Link all groups
	if (areaNumber != 255) { // Do not link objects in the room structure
		for (ObjectMap::iterator it = objectsByID->begin(); it != objectsByID->end(); ++it) {
			if (it->_value->getType() == ObjectType::kGroupType) {
				Group *group = (Group *)it->_value;
				for (ObjectMap::iterator itt = objectsByID->begin(); itt != objectsByID->end(); ++itt)
					group->linkObject(itt->_value);
			}
		}
	}

	int64 endLastObject = file->pos();
	debugC(1, kFreescapeDebugParser, "Last position %" PRIx64, endLastObject);
	debugC(1, kFreescapeDebugParser, "endLastObject is supposed to be %x", base + cPtr);
	if ((isDark() || isEclipse()) && (isAmiga() || isAtariST()))
		assert(endLastObject <= static_cast<int64>(base + cPtr) + 4);
	else
		assert(endLastObject == static_cast<int64>(base + cPtr) || areaNumber == 192);
	file->seek(base + cPtr);
	uint8 numConditions = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "%d area conditions at %x of area %d", numConditions, base + cPtr, areaNumber);

	Area *area = new Area(areaNumber, areaFlags, objectsByID, entrancesByID);
	area->_name = name;
	area->_scale = scale;
	area->_skyColor = skyColor;
	area->_groundColor = groundColor;

	area->_inkColor = inkColor;
	area->_paperColor = paperColor;
	area->_usualBackgroundColor = usualBackgroundColor;
	area->_underFireBackgroundColor = underFireBackgroundColor;

	area->_extraColor[0] = extraColor[0];
	area->_extraColor[1] = extraColor[1];
	area->_extraColor[2] = extraColor[2];
	area->_extraColor[3] = extraColor[3];

	// Driller specific
	area->_gasPocketPosition = Common::Point(32 * gasPocketX, 32 * gasPocketY);
	area->_gasPocketRadius = 32 * gasPocketRadius;

	while (numConditions--) {
		FCLInstructionVector instructions;
		// get the length
		uint32 lengthOfCondition = readField(file, 8);
		debugC(1, kFreescapeDebugParser, "length of condition: %d", lengthOfCondition);
		// get the condition
		if (lengthOfCondition > 0) {
			Common::Array<uint16> conditionArray = readArray(file, lengthOfCondition);
			Common::String conditionSource = detokenise8bitCondition(conditionArray, instructions, isAmiga() || isAtariST());
			area->_conditions.push_back(instructions);
			area->_conditionSources.push_back(conditionSource);
			debugC(1, kFreescapeDebugParser, "%s", conditionSource.c_str());
		}
	}

	debugC(1, kFreescapeDebugParser, "End of area at %lx", long(file->pos()));
	return area;
}

void FreescapeEngine::load8bitBinary(Common::SeekableReadStream *file, int offset, int ncolors) {
	file->seek(offset);
	uint8 numberOfAreas = readField(file, 8);
	if (isAmiga() && isCastle() && isDemo())
		numberOfAreas = 87;
	debugC(1, kFreescapeDebugParser, "Number of areas: %d", numberOfAreas);

	uint32 dbSize = readField(file, 16);
	debugC(1, kFreescapeDebugParser, "Database ends at %x", dbSize);
	if (isAmiga() || isAtariST())
		debugC(1, kFreescapeDebugParser, "Extra field: %x", readField(file, 16));

	uint8 startArea = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Start area: %d", startArea);
	uint8 startEntrance = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Entrace area: %d", startEntrance);

	uint8 initialEnergy1 = 0;
	uint8 initialShield1 = 0;
	uint8 initialEnergy2 = 0;
	uint8 initialShield2 = 0;

	if (isCastle() && (isSpectrum() || isCPC())) {
		initialShield1 = readField(file, 8);
	} else {
		readField(file, 8); // Unknown

		initialEnergy1 = readField(file, 8);
		initialShield1 = readField(file, 8);
		initialEnergy2 = readField(file, 8);
		initialShield2 = readField(file, 8);
	}

	debugC(1, kFreescapeDebugParser, "Initial levels of energy: %d and shield: %d", initialEnergy1, initialShield1);
	debugC(1, kFreescapeDebugParser, "Initial levels of energy: %d and shield: %d", initialEnergy2, initialShield2);

	if (isCastle() && (isSpectrum() || isCPC()))
		file->seek(offset + 0x6);
	else if (isAmiga() || isAtariST())
		file->seek(offset + 0x14);
	else
		file->seek(offset + 0xa);

	debugC(1, kFreescapeDebugParser, "Color map:");
	uint8 data;
	for (int i = 0; i < 15; i++) {
		byte *entry = (byte *)malloc(4 * sizeof(byte));
		data = readField(file, 8);
		*entry = data;
		entry++;
		debugC(1, kFreescapeDebugParser, "%x", data);
		data = readField(file, 8);
		*entry = data;
		entry++;
		debugC(1, kFreescapeDebugParser, "%x", data);
		data = readField(file, 8);
		*entry = data;
		entry++;
		debugC(1, kFreescapeDebugParser, "%x", data);
		data = readField(file, 8);
		*entry = data;
		debugC(1, kFreescapeDebugParser, "%x", data);
		debugC(1, kFreescapeDebugParser, "---");
		_colorMap.push_back(entry - 3);
	}

	if (isCastle() && (isSpectrum() || isCPC()))
		file->seek(offset + 0x42);
	else if (isAmiga() || isAtariST())
		file->seek(offset + 0x8c);
	else
		file->seek(offset + 0x46);

	uint16 demoDataTable;
	demoDataTable = readPtr(file);
	debugC(1, kFreescapeDebugParser, "Pointer to demo data: %x\n", demoDataTable);

	uint16 globalByteCodeTable;
	globalByteCodeTable = readPtr(file);
	debugC(1, kFreescapeDebugParser, "GBCT: %x\n", globalByteCodeTable);

	if (isDOS())
		loadDemoData(file, offset + demoDataTable, 128); // TODO: check this size

	file->seek(offset + globalByteCodeTable);
	debugC(1, kFreescapeDebugParser, "Position: %lx\n", long(file->pos()));

	uint8 numConditions = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "%d global conditions", numConditions);
	while (numConditions--) { // TODO: read global conditions in Amiga
		FCLInstructionVector instructions;
		// get the length
		uint32 lengthOfCondition = readField(file, 8);
		debugC(1, kFreescapeDebugParser, "length of condition: %d at %lx", lengthOfCondition, long(file->pos()));
		// get the condition
		if (lengthOfCondition > 0) {
			Common::Array<uint16> conditionArray = readArray(file, lengthOfCondition);
			Common::String conditionSource = detokenise8bitCondition(conditionArray, instructions, isAmiga() || isAtariST());
			_conditions.push_back(instructions);
			_conditionSources.push_back(conditionSource);
			debugC(1, kFreescapeDebugParser, "%s", conditionSource.c_str());
		}
	}

	if (isDriller() || isDark()) {
		debugC(1, kFreescapeDebugParser, "Time to finish the game:");
		if (isAmiga() || isAtariST())
			file->seek(offset + 0x168);
		else
			file->seek(offset + 0xb4);
		Common::String n;

		if (isDriller()) {
			n += char(readField(file, 8));
			n += char(readField(file, 8));
			debugC(1, kFreescapeDebugParser, "'%s' hours", n.c_str());
			_initialCountdown =_initialCountdown + 3600 * atoi(n.c_str());
			n.clear();
			n += char(readField(file, 8));
			assert(n == ":");
			n.clear();
		}
		n += char(readField(file, 8));
		n += char(readField(file, 8));
		debugC(1, kFreescapeDebugParser, "'%s' minutes", n.c_str());
		_initialCountdown = _initialCountdown + 60 * atoi(n.c_str());
		n.clear();
		n += char(readField(file, 8));
		assert(n == ":");
		n.clear();
		n += char(readField(file, 8));
		n += char(readField(file, 8));
		debugC(1, kFreescapeDebugParser, "'%s' seconds", n.c_str());
		_initialCountdown = _initialCountdown + atoi(n.c_str());
		if (_useExtendedTimer)
			_initialCountdown = 359999; // 99:59:59
	} else if (isCastle())
		_initialCountdown = 1000000000;
	else if (isEclipse())
		_initialCountdown = 7200; // 02:00:00

	if (isCastle() && (isSpectrum() || isCPC()))
		file->seek(offset + 0x4f);
	else if (isAmiga() || isAtariST())
		file->seek(offset + 0x190);
	else
		file->seek(offset + 0xc8);

	debugC(1, kFreescapeDebugParser, "areas index at: %lx", long(file->pos()));
	uint16 *fileOffsetForArea = new uint16[numberOfAreas];
	for (uint16 area = 0; area < numberOfAreas; area++) {
		fileOffsetForArea[area] = readPtr(file);
		debugC(1, kFreescapeDebugParser, "offset: %x", fileOffsetForArea[area]);
	}

	// grab the areas
	Area *newArea = nullptr;
	for (uint16 area = 0; area < numberOfAreas; area++) {
		debugC(1, kFreescapeDebugParser, "Starting to parse area index %d at offset %x", area, fileOffsetForArea[area]);

		file->seek(offset + fileOffsetForArea[area]);
		newArea = load8bitArea(file, ncolors);

		if (newArea) {
			if (!_areaMap.contains(newArea->getAreaID()))
				_areaMap[newArea->getAreaID()] = newArea;
			else
				debugC(1, kFreescapeDebugParser, "WARNING: area ID repeated: %d", newArea->getAreaID());
		} else {
			error("Invalid area %d?", area);
		}
	}

	delete[] fileOffsetForArea;

	if (!_areaMap.contains(startArea))
		_startArea = newArea->getAreaID();
	else
		_startArea = startArea;
	_startEntrance = startEntrance;
	_colorNumber = ncolors;
	_binaryBits = 8;
}

void FreescapeEngine::loadFonts(Common::SeekableReadStream *file, int offset) {
	Common::Array<Graphics::ManagedSurface *> chars;

	if (isAmiga() || isAtariST())
		chars = getCharsAmigaAtari(file, offset, 85);
	else
		chars = getChars(file, offset, 85);

	_font = Font(chars);
	if (_renderMode == Common::kRenderHercG) {
		_font.setCharWidth(16);
	}
	_fontLoaded = true;
}

void FreescapeEngine::loadMessagesFixedSize(Common::SeekableReadStream *file, int offset, int size, int number) {
	file->seek(offset);
	byte *buffer = (byte *)malloc(size + 1);
	buffer[size] = 0;
	debugC(1, kFreescapeDebugParser, "String table:");

	for (int i = 0; i < number; i++) {
		file->read(buffer, size);
		Common::String message = (const char *)buffer;
		_messagesList.push_back(message);
		debugC(1, kFreescapeDebugParser, "'%s'", _messagesList[_messagesList.size() - 1].c_str());
	}
	free(buffer);
}

void FreescapeEngine::loadDemoData(Common::SeekableReadStream *file, int offset, int size) {
	file->seek(offset);
	/*if (isAmiga()) {
		_demoData.push_back(0x50);
		_demoData.push_back(0x64);
		_demoData.push_back(0x30);
		_demoData.push_back(0x00);
		_demoData.push_back(0x64);
		_demoData.push_back(0x64);
		_demoData.push_back(0x5F);
		_demoData.push_back(0x00);
	}*/

	debugC(1, kFreescapeDebugParser, "Reading demo data");
	for (int i = 0; i < size; i++) {
		byte b = file->readByte();
		_demoData.push_back(b);
		debugC(1, kFreescapeDebugParser, "%x", b);
	}
}

void FreescapeEngine::loadMessagesVariableSize(Common::SeekableReadStream *file, int offset, int number) {
	file->seek(offset);
	debugC(1, kFreescapeDebugParser, "String table:");

	for (int i = 0; i < number; i++) {
		Common::String message = "";
		while (true) {
			byte c = file->readByte();
			if (c > 0xf0)
				c = ' ';
			if (c <= 1)
				break;
			message = message + c;
		}

		_messagesList.push_back(message);
		debugC(1, kFreescapeDebugParser, "'%s'", _messagesList[i].c_str());
	}
	debugC(1, kFreescapeDebugParser, "End of messages at %" PRIx64, file->pos());
}

void FreescapeEngine::loadGlobalObjects(Common::SeekableReadStream *file, int offset, int size) {
	assert(!_areaMap.contains(255));
	ObjectMap *globalObjectsByID = new ObjectMap;
	file->seek(offset);
	for (int i = 0; i < size; i++) {
		Object *gobj = load8bitObject(file);
		assert(gobj);
		assert(!globalObjectsByID->contains(gobj->getObjectID()));
		debugC(1, kFreescapeDebugParser, "Adding global object: %d", gobj->getObjectID());
		(*globalObjectsByID)[gobj->getObjectID()] = gobj;
	}

	_areaMap[255] = new Area(255, 0, globalObjectsByID, nullptr);
}

void FreescapeEngine::parseAmigaAtariHeader(Common::SeekableReadStream *stream) {
	stream->seek(0x22);
	int size = stream->readUint16BE();
	debugC(1, kFreescapeDebugParser, "Header table size %d", size);
	for (int i = 0; i < size; i++) {
		debugC(1, kFreescapeDebugParser, "Location: %x ", stream->readUint32BE());
		Common::String filename;
		while (char c = stream->readByte())
			filename += c;

		for (int j = filename.size() + 1; j < 16; j++)
			stream->readByte();

		debugC(1, kFreescapeDebugParser, "Filename: %s", filename.c_str());
	}
}

Common::SeekableReadStream *FreescapeEngine::decryptFileAmigaAtari(const Common::Path &packed, const Common::Path &unpacker, uint32 unpackArrayOffset) {
	Common::File file;
	file.open(packed);
	if (!file.isOpen())
		error("Failed to open %s", packed.toString().c_str());

	int size = file.size();
	size -= size % 4;
	byte *encryptedBuffer = (byte *)malloc(size);
	file.read(encryptedBuffer, size);
	file.close();

	uint32 d7 = 0;
	uint32 d6 = 0;
	byte *a6 = encryptedBuffer;
	byte *a5 = encryptedBuffer + size - 1;

	while (a6 <= a5) {
		uint64 d0 = (a6[0] << 24) | (a6[1] << 16) | (a6[2] << 8) | a6[3];
		d0 = d0 + d6;
		d0 = uint32(d0);
		d0 = ((d0 << 3) & 0xFFFFFFFF) | ((d0 >> 29) & 0xFFFFFFFF);
		d0 ^= 0x71049763;
		d0 -= d7;
		d0 = ((d0 << 16) & 0xFFFF0000) | ((d0 >> 16) & 0xFFFF);

		a6[0] = byte((d0 >> 24) & 0xFF);
		//debug("%c", a6[0]);
		a6[1] = byte((d0 >> 16) & 0xFF);
		//debug("%c", a6[1]);
		a6[2] = byte((d0 >> 8) & 0xFF);
		//debug("%c", a6[2]);
		a6[3] = byte(d0 & 0xFF);
		//debug("%c", a6[3]);

		d6 += 5;
		d6 = ((d6 >> 3) & 0xFFFFFFFF) | ((d6 << 29) & 0xFFFFFFFF);
		d6 ^= 0x04000000;
		d7 += 4;
		a6 += 4;
	}

	file.open(unpacker);
	if (!file.isOpen())
		error("Failed to open %s", unpacker.toString().c_str());

	int originalSize = size;
	size = file.size();
	byte *unpackArray = (byte *)malloc(size);
	file.read(unpackArray, size);
	file.close();

	byte *unpackArrayPtr = unpackArray + unpackArrayOffset;
	uint32 i = 2 * 1024;
	do {
		uint8 ptr0 = unpackArrayPtr[2 * i];
		//debug("%x -> %x", unpackArrayOffset + 2 * i, ptr0);
		uint8 ptr1 = unpackArrayPtr[2 * i + 1];
		//debug("%x -> %x", unpackArrayOffset + 2 * i + 1, ptr1);
		uint8 val0 = unpackArrayPtr[2 * (i - 1)];
		uint8 val1 = unpackArrayPtr[2 * (i - 1) + 1];

		encryptedBuffer[2 * (ptr1 + 256 * ptr0)] = val0;
		encryptedBuffer[2 * (ptr1 + 256 * ptr0) + 1] = val1;

		i = i - 2;
	} while (i > 0);

	return (new Common::MemoryReadStream(encryptedBuffer, originalSize));
}


} // namespace Freescape
