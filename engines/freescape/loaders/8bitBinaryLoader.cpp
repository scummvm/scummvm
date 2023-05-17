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

#include "image/bmp.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/objects/connections.h"
#include "freescape/objects/global.h"
#include "freescape/objects/group.h"
#include "freescape/objects/sensor.h"

namespace Freescape {

uint16 FreescapeEngine::readField(Common::SeekableReadStream *file, int bits) {
	uint16 value;
	assert(bits == 8 || bits == 16);
	if (isAmiga() || isAtariST()) {
		if (bits == 16) {
			uint16 lo = file->readUint16BE();
			assert(lo < 256);
			uint16 hi = file->readUint16BE();
			assert(hi < 256);
			value = 256 * hi + lo;
			value = 2 * value; // Unclear why, but this reads a pointer
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

Common::Array<uint8> FreescapeEngine::readArray(Common::SeekableReadStream *file, int size) {
	byte *data = (byte *)malloc(size);
	for (int i = 0; i < size; i++) {
		data[i] = readField(file, 8);
	}
	Common::Array<uint8> array(data, size);
	free(data);
	return array;
}

Object *FreescapeEngine::load8bitObject(Common::SeekableReadStream *file) {

	byte rawFlagsAndType = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Raw object data flags and type: %d", rawFlagsAndType);
	ObjectType objectType = (ObjectType)(rawFlagsAndType & 0x1F);

	Math::Vector3d position, v;

	position.x() = readField(file, 8);
	position.y() = readField(file, 8);
	position.z() = readField(file, 8);

	v.x() = readField(file, 8);
	v.y() = readField(file, 8);
	v.z() = readField(file, 8);

	// object ID
	uint16 objectID = readField(file, 8);
	// size of object on disk; we've accounted for 8 bytes
	// already so we can subtract that to get the remaining
	// length beyond here
	uint8 byteSizeOfObject = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Raw object %d ; type %d ; size %d", objectID, (int)objectType, byteSizeOfObject);
	if (byteSizeOfObject < 9) {
		error("Not enough bytes %d to read object %d with type %d", byteSizeOfObject, objectID, objectType);
	}

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
			structureArray.push_back(file->readByte());
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
			connectionsArray.push_back(file->readByte());
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
		debugC(1, kFreescapeDebugParser, "Number of colors: %d", numberOfColours / 2);
		uint8 entry;
		for (uint8 colour = 0; colour < numberOfColours / 2; colour++) {
			uint8 data = readField(file, 8);
			entry = data & 0xf;
			//if (_renderMode == Common::kRenderCGA)
			//	entry = entry % 4; // TODO: use dithering

			colours->push_back(entry);
			debugC(1, kFreescapeDebugParser, "color[%d] = %x", 2 * colour, entry);

			entry = data >> 4;
			//if (_renderMode == Common::kRenderCGA)
			//	entry = entry % 4; // TODO: use dithering

			colours->push_back(entry);
			debugC(1, kFreescapeDebugParser, "color[%d] = %x", 2 * colour + 1, entry);
			byteSizeOfObject--;
		}

		// read extra vertices if required...
		int numberOfOrdinates = GeometricObject::numberOfOrdinatesForType(objectType);
		debugC(1, kFreescapeDebugParser, "number of ordinates %d", numberOfOrdinates);
		Common::Array<uint16> *ordinates = nullptr;

		if (numberOfOrdinates) {
			ordinates = new Common::Array<uint16>;
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
			Common::Array<uint8> conditionArray = readArray(file, byteSizeOfObject);
			conditionSource = detokenise8bitCondition(conditionArray, instructions, isCastle());
			// instructions = getInstructions(conditionSource);
			debugC(1, kFreescapeDebugParser, "%s", conditionSource.c_str());
		}
		debugC(1, kFreescapeDebugParser, "End of object at %lx", long(file->pos()));

		if (!GeometricObject::isPolygon(objectType))
			position = 32 * position;

		// create an object
		return new GeometricObject(
			objectType,
			objectID,
			rawFlagsAndType, // flags
			position,
			32 * v, // size
			colours,
			ordinates,
			instructions,
			conditionSource);
	} break;
	case kEntranceType: {
		debugC(1, kFreescapeDebugParser, "rotation: %f %f %f", v.x(), v.y(), v.z());
		if (byteSizeOfObject > 0) {
			// TODO: there is something here
			debugC(1, kFreescapeDebugParser, "Warning: extra %d bytes in entrance", byteSizeOfObject);
			while (byteSizeOfObject--) {
				debugC(1, kFreescapeDebugParser, "b: %x", readField(file, 8));
			}
			byteSizeOfObject = 0;
		}
		assert(byteSizeOfObject == 0);
		debugC(1, kFreescapeDebugParser, "End of object at %lx", long(file->pos()));
		// create an entrance
		return new Entrance(
			objectID,
			32 * position,
			5 * v); // rotation
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
				0,
				instructions,
				conditionSource);
		}

		assert(byteSizeOfObject >= 5);
		byte color = readField(file, 8) & 0xf;
		assert(color > 0);
		byte firingInterval = readField(file, 8);
		uint16 firingRange = readField(file, 16);
		byte sensorAxis = readField(file, 8);
		byteSizeOfObject = byteSizeOfObject - 5;
		// grab the object condition, if there is one
		if (byteSizeOfObject) {
			Common::Array<uint8> conditionArray = readArray(file, byteSizeOfObject);
			conditionSource = detokenise8bitCondition(conditionArray, instructions, isCastle());
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
		debugC(1, kFreescapeDebugParser, "Object of type 'group'");
		Common::Array<uint8> groupDataArray;
		groupDataArray.push_back(uint8(position.x()));
		groupDataArray.push_back(uint8(position.y()));
		groupDataArray.push_back(uint8(position.z()));

		groupDataArray.push_back(uint8(v.x()));
		groupDataArray.push_back(uint8(v.y()));
		groupDataArray.push_back(uint8(v.z()));

		byteSizeOfObject++;
		while(--byteSizeOfObject > 0)
			groupDataArray.push_back(file->readByte());

		return new Group(
			objectID,
			rawFlagsAndType,
			groupDataArray);
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

void FreescapeEngine::renderPixels8bitBinImage(Graphics::ManagedSurface *surface, int &i, int &j, uint8 pixels, int color) {
	if (i >= 320) {
		//debug("cannot continue, stopping here at row %d!", j);
		return;
	}

	int acc = 1 << 7;
	while (acc > 0) {
		assert(i < 320);
		if (acc & pixels) {
			int previousColor = surface->getPixel(i, j);
			surface->setPixel(i, j, previousColor + color);
			assert(previousColor + color < 16);
		}
		i++;
		acc = acc >> 1;
	}

}

Graphics::ManagedSurface *FreescapeEngine::load8bitBinImage(Common::SeekableReadStream *file, int offset) {
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(_screenW, _screenH, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, 320, 200), 0);

	file->seek(offset);
	int imageSize = file->readUint16BE();

	int i = 0;
	int j = 0;
	int hPixelsWritten = 0;
	int color = 1;
	int command = 0;
	while (file->pos() <= offset + imageSize) {
		//debug("pos: %lx", file->pos());
		command = file->readByte();

		color = 1 + hPixelsWritten / 320;
		//debug("command: %x with j: %d", command, j);
		if (j >= 200)
			return surface;

		if (command <= 0x7f) {
			//debug("starting singles at i: %d j: %d", i, j);
			int start = i;
			while (command-- >= 0) {
				int pixels = file->readByte();
				//debug("single pixels command: %d with pixels: %x", command, pixels);
				renderPixels8bitBinImage(surface, i, j, pixels, color);
			}
			hPixelsWritten = hPixelsWritten + i - start;
		} else if (command <= 0xff && command >= 0xf0) {
			int size = (136 - 8*(command - 0xf0)) / 2;
			int start = i;
			int pixels = file->readByte();
			//debug("starting 0xfX: at i: %d j: %d with pixels: %x", i, j, pixels);
			while (size > 0) {
				renderPixels8bitBinImage(surface, i, j, pixels, color);
				size = size - 4;
			}
			hPixelsWritten = hPixelsWritten + i - start;
			assert(i <= 320);
		} else if (command <= 0xef && command >= 0xe0) {
			int size = (264 - 8*(command - 0xe0)) / 2;
			int start = i;
			int pixels = file->readByte();
			//debug("starting 0xeX: at i: %d j: %d with pixels: %x", i, j, pixels);
			while (size > 0) {
				renderPixels8bitBinImage(surface, i, j, pixels, color);
				size = size - 4;
			}
			hPixelsWritten = hPixelsWritten + i - start;
		} else if (command <= 0xdf && command >= 0xd0) {
			int size = (272 + 8*(0xdf - command)) / 2;
			int start = i;
			int pixels = file->readByte();
			while (size > 0) {
				renderPixels8bitBinImage(surface, i, j, pixels, color);
				size = size - 4;
			}
			hPixelsWritten = hPixelsWritten + i - start;
		} else {
			error("unknown command: %x", command);
		}

		if (i >= 320) {
			i = 0;
			if (hPixelsWritten >= (_renderMode == Common::kRenderCGA ? 640 : 1280)) {
				j++;
				hPixelsWritten = 0;
			}
		}


	}
	return surface;
}

Area *FreescapeEngine::load8bitArea(Common::SeekableReadStream *file, uint16 ncolors) {

	Common::String name;
	uint32 base = file->pos();
	debugC(1, kFreescapeDebugParser, "Area base: %x", base);
	uint8 areaFlags = readField(file, 8);
	uint8 numberOfObjects = readField(file, 8);
	uint8 areaNumber = readField(file, 8);

	uint16 cPtr = readField(file, 16);
	debugC(1, kFreescapeDebugParser, "Condition pointer: %x", cPtr);
	uint8 scale = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Scale: %d", scale);

	uint8 skyColor = areaFlags & 15;
	uint8 groundColor = areaFlags >> 4;

	if (groundColor == 0)
		groundColor = 255;

	uint8 usualBackgroundColor = readField(file, 8);
	uint8 underFireBackgroundColor = readField(file, 8);
	uint8 paperColor = readField(file, 8);
	uint8 inkColor = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Colors usual background: %d", usualBackgroundColor);
	debugC(1, kFreescapeDebugParser, "Colors under fire background: %d", underFireBackgroundColor);
	debugC(1, kFreescapeDebugParser, "Color Paper: %d", paperColor);
	debugC(1, kFreescapeDebugParser, "Color Ink: %d", inkColor);

	debugC(1, kFreescapeDebugParser, "Additional colors: %d %d", skyColor, groundColor);
	// CPC
	// groundColor = file->readByte() & 15;
	// skyColor = file->readByte() & 15;
	// debugC(1, kFreescapeDebugParser, "Colors: %d %d", skyColor, groundColor);

	// Graphics::PixelBuffer *palette = getPalette(areaNumber, ci1, ci2, skyColor, groundColor, ncolors);

	debugC(1, kFreescapeDebugParser, "Area %d", areaNumber);
	debugC(1, kFreescapeDebugParser, "Flags: %d Objects: %d", areaFlags, numberOfObjects);
	// debug("Condition Ptr: %x", cPtr);
	debugC(1, kFreescapeDebugParser, "Pos before first object: %lx", long(file->pos()));

	// Driller specific
	uint8 gasPocketX = 0;
	uint8 gasPocketY = 0;
	uint8 gasPocketRadius = 0;
	// Castle specific
	uint8 extraColor[4];
	if (isEclipse()) {
		byte idx = file->readByte();
		name = idx < 8 ? eclipseRoomName[idx] : eclipseRoomName[8];
		name = name + "-" + char(file->readByte()) + " ";

		int i = 0;
		while (i < 3) {
			name = name + char(file->readByte());
			i++;
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
		name = _messagesList[idx + 41];
		extraColor[0] = readField(file, 8);
		extraColor[1] = readField(file, 8);
		extraColor[2] = readField(file, 8);
		extraColor[3] = readField(file, 8);
	}
	debugC(1, kFreescapeDebugParser, "Area name: %s", name.c_str());

	ObjectMap *objectsByID = new ObjectMap;
	ObjectMap *entrancesByID = new ObjectMap;
	for (uint8 object = 0; object < numberOfObjects && areaNumber != 192; object++) {
		debugC(1, kFreescapeDebugParser, "Reading object: %d", object);
		Object *newObject = load8bitObject(file);

		if (newObject->getType() == ObjectType::kGroupType) {
			Group *group = (Group *)newObject;
			for (ObjectMap::iterator it = objectsByID->begin(); it != objectsByID->end(); ++it)
				group->assemble(it->_value);
		}

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
		} else
			error("Failed to read an object!");
	}
	long int endLastObject = file->pos();
	debugC(1, kFreescapeDebugParser, "Last position %lx", endLastObject);
	assert(endLastObject == base + cPtr || areaNumber == 192);
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
			Common::Array<uint8> conditionArray = readArray(file, lengthOfCondition);
			Common::String conditionSource = detokenise8bitCondition(conditionArray, instructions, isCastle());
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
	debugC(1, kFreescapeDebugParser, "Number of areas: %d", numberOfAreas);

	if (isDOS() && isCastle()) // Castle Master for DOS has an invalid number of areas
		numberOfAreas = isDemo() ? 31 : 104;

	uint32 dbSize = readField(file, 16);
	debugC(1, kFreescapeDebugParser, "Database ends at %x", dbSize);

	uint8 startArea = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Start area: %d", startArea);
	uint8 startEntrance = readField(file, 8);
	debugC(1, kFreescapeDebugParser, "Entrace area: %d", startEntrance);
	readField(file, 8); // Unknown

	uint8 initialEnergy1 = readField(file, 8);
	uint8 initialShield1 = readField(file, 8);
	uint8 initialEnergy2 = readField(file, 8);
	uint8 initialShield2 = readField(file, 8);

	debugC(1, kFreescapeDebugParser, "Initial levels of energy: %d and shield: %d", initialEnergy1, initialShield1);
	debugC(1, kFreescapeDebugParser, "Initial levels of energy: %d and shield: %d", initialEnergy2, initialShield2);

	if (isAmiga() || isAtariST())
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

	if (isAmiga() || isAtariST())
		file->seek(offset + 0x8c);
	else
		file->seek(offset + 0x46);

	uint16 demoDataTable;
	demoDataTable = readField(file, 16);
	debugC(1, kFreescapeDebugParser, "Pointer to demo data: %x\n", demoDataTable);

	uint16 globalByteCodeTable;
	globalByteCodeTable = readField(file, 16);
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
			Common::Array<uint8> conditionArray = readArray(file, lengthOfCondition);
			Common::String conditionSource = detokenise8bitCondition(conditionArray, instructions, isCastle());
			_conditions.push_back(instructions);
			_conditionSources.push_back(conditionSource);
			debugC(1, kFreescapeDebugParser, "%s", conditionSource.c_str());
		}
	}

	if (isDriller()) {
		if (isAmiga() || isAtariST())
			file->seek(offset + 0x168);
		else
			file->seek(offset + 0xb4);
		Common::String n;
		n += char(readField(file, 8));
		n += char(readField(file, 8));
		_initialCountdown =_initialCountdown + 3600 * atoi(n.c_str());
		n.clear();
		n += char(readField(file, 8));
		assert(n == ":");
		n.clear();
		n += char(readField(file, 8));
		n += char(readField(file, 8));
		_initialCountdown = _initialCountdown + 60 * atoi(n.c_str());
		n.clear();
		n += char(readField(file, 8));
		assert(n == ":");
		n.clear();
		n += char(readField(file, 8));
		n += char(readField(file, 8));
		_initialCountdown = _initialCountdown + atoi(n.c_str());

		if (_useExtendedTimer)
			_initialCountdown = 359999; // 99:59:59
	} else if (isDark())
		_initialCountdown = 2 * 3600; // 02:00:00
	else if (isCastle())
		_initialCountdown = 1000000000;

	if (isAmiga() || isAtariST())
		file->seek(offset + 0x190);
	else
		file->seek(offset + 0xc8);
	// file->seek(offset + 0x4f); //CPC

	debugC(1, kFreescapeDebugParser, "areas index at: %lx", long(file->pos()));
	uint16 *fileOffsetForArea = new uint16[numberOfAreas];
	for (uint16 area = 0; area < numberOfAreas; area++) {
		fileOffsetForArea[area] = readField(file, 16);
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

void FreescapeEngine::loadBundledImages() {
	/*Image::BitmapDecoder decoder;
	Common::String targetName = Common::String(_gameDescription->gameId);
	if (isDOS() && isDemo())
		Common::replace(targetName, "-demo", "");

	Common::String borderFilename = targetName + "_" + Common::getRenderModeCode(_renderMode) + ".bmp";
	if (_dataBundle->hasFile(borderFilename)) {
		Common::SeekableReadStream *borderFile = _dataBundle->createReadStreamForMember(borderFilename);
		decoder.loadStream(*borderFile);
		_border = new Graphics::Surface();
		_border->copyFrom(*decoder.getSurface());
		decoder.destroy();
	} else
		error("Missing border file '%s' in data bundle", borderFilename.c_str());

	Common::String titleFilename = targetName + "_" + Common::getRenderModeDescription(_renderMode) + "_title.bmp";
	if (_dataBundle->hasFile(titleFilename)) {
		Common::SeekableReadStream *titleFile = _dataBundle->createReadStreamForMember(titleFilename);
		decoder.loadStream(*titleFile);
		_title = new Graphics::Surface();
		_title->copyFrom(*decoder.getSurface());
		decoder.destroy();
	}*/
}

void FreescapeEngine::loadFonts(byte *font, int charNumber) {
	if (isDOS() || isSpectrum() || isCPC() || isC64()) {
		_font.set_size(64 * charNumber);
		_font.set_bits(font);
	} else if (isAmiga() || isAtariST()) {
		error("Not implemented yet");
	}
	_fontLoaded = true;
}


void FreescapeEngine::loadFonts(Common::SeekableReadStream *file, int offset) {
	file->seek(offset);
	int charNumber = 60;
	byte *font = nullptr;
	if (isDOS() || isSpectrum() || isCPC() || isC64()) {
		font = (byte *)malloc(6 * charNumber);
		file->read(font, 6 * charNumber);

		_font.set_size(48 * charNumber);
		_font.set_bits((byte *)font);
	} else if (isAmiga() || isAtariST()) {
		int fontSize = 4654; // Driller
		font = (byte *)malloc(fontSize);
		file->read(font, fontSize);

		_font.set_size(fontSize * 8);
		_font.set_bits((byte *)font);
	} else {
		_fontLoaded = false;
	}
	_fontLoaded = true;
	free(font);
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
			if (c <= 1)
				break;
			message = message + c;
		}

		_messagesList.push_back(message);
		debugC(1, kFreescapeDebugParser, "%s", _messagesList[i].c_str());
	}
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


} // namespace Freescape
