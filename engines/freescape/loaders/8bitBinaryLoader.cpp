#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"
#include "image/bmp.h"

#include "freescape/freescape.h"
#include "freescape/area.h"

#include "freescape/language/8bitDetokeniser.h"
#include "freescape/language/instruction.h"

#include "freescape/objects/object.h"
#include "freescape/objects/geometricobject.h"
#include "freescape/objects/entrance.h"
#include "freescape/objects/sensor.h"

namespace Freescape {

Object *FreescapeEngine::load8bitObject(Common::SeekableReadStream *file) {

	byte rawFlagsAndType = file->readByte();
	debugC(1, kFreescapeDebugParser, "Raw object data flags and type: %d", rawFlagsAndType);
	Object::Type objectType = (Object::Type)(rawFlagsAndType & 0x1F);

	Math::Vector3d position, v;

	position.x() = file->readByte();
	position.y() = file->readByte();
	position.z() = file->readByte();

	v.x() = file->readByte();
	v.y() = file->readByte();
	v.z() = file->readByte();

	// object ID
	uint16 objectID = file->readByte();
	// size of object on disk; we've accounted for 8 bytes
	// already so we can subtract that to get the remaining
	// length beyond here
	uint8 byteSizeOfObject = file->readByte();
	debugC(1, kFreescapeDebugParser, "Raw object %d ; type %d ; size %d", objectID, (int)objectType, byteSizeOfObject);
	if (byteSizeOfObject < 9) {
		error("Not enough bytes %d to read object %d with type %d", byteSizeOfObject, objectID, objectType);
	}

	assert(byteSizeOfObject >= 9);
	byteSizeOfObject = byteSizeOfObject - 9;
	if (objectID == 255) {
		debug("Found the room structure (objectID: 255 with size %d)", byteSizeOfObject + 6);
		byte *structureData = (byte*)malloc(byteSizeOfObject + 6);
		structureData[0] = int(position.x());
		structureData[1] = int(position.y());
		structureData[2] = int(position.z());

		structureData[3] = int(v.x());
		structureData[4] = int(v.y());
		structureData[5] = int(v.z());

		if (byteSizeOfObject > 0)
			file->read(structureData+6, byteSizeOfObject);
		Common::Array<uint8> structureArray(structureData, byteSizeOfObject + 6);
		return new RoomStructure(structureArray);
	}

	debugC(1, kFreescapeDebugParser, "Object %d ; type %d ; size %d", objectID, (int)objectType, byteSizeOfObject);
    debugC(1, kFreescapeDebugParser, "pos: %f %f %f", position.x(), position.y(), position.z());
	switch (objectType) {
	default: {
		debugC(1, kFreescapeDebugParser, "size: %f %f %f", v.x(), v.y(), v.z());
		// read the appropriate number of colours
		int numberOfColours = GeometricObject::numberOfColoursForObjectOfType(objectType);
		Common::Array<uint8> *colours = new Common::Array<uint8>;
		debugC(1, kFreescapeDebugParser, "Number of colors: %d", numberOfColours/2);
		uint8 entry;
		for (uint8 colour = 0; colour < numberOfColours/2; colour++) {
			uint8 data = file->readByte();
			entry = data & 0xf;
			if (_renderMode == "cga")
				entry = entry % 4; // TODO: use dithering

			colours->push_back(entry);
			debugC(1, kFreescapeDebugParser, "color[%d] = %x", 2*colour, entry);

			entry = data >> 4;
			if (_renderMode == "cga")
				entry = entry % 4; // TODO: use dithering

			colours->push_back(entry);
			debugC(1, kFreescapeDebugParser, "color[%d] = %x", 2*colour+1, entry);
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
				//file->seek(byteSizeOfObject, SEEK_CUR);
				//return nullptr;
			}
			for (int ordinate = 0; ordinate < numberOfOrdinates; ordinate++) {
				ord = file->readByte();
				debugC(1, kFreescapeDebugParser, "ord: %x", ord);
				ordinates->push_back(32 * ord);
				byteSizeOfObject--;
			}
		}

		// grab the object condition, if there is one
		FCLInstructionVector instructions;
		Common::String *conditionSource = nullptr;
		if (byteSizeOfObject) {
			byte *conditionData = (byte*)malloc(byteSizeOfObject);
			file->read(conditionData, byteSizeOfObject);
			Common::Array<uint8> conditionArray(conditionData, byteSizeOfObject);
			conditionSource = detokenise8bitCondition(conditionArray, instructions);
			//instructions = getInstructions(conditionSource);
			debugC(1, kFreescapeDebugParser, "%s", conditionSource->c_str());
		}
		debugC(1, kFreescapeDebugParser, "End of object at %lx", file->pos());

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
	case Object::Entrance: {
		debugC(1, kFreescapeDebugParser, "rotation: %f %f %f", v.x(), v.y(), v.z());
		if (byteSizeOfObject > 0) {
			// TODO: there is something here
			debugC(1, kFreescapeDebugParser, "Warning: extra %d bytes in entrance", byteSizeOfObject);
			while (byteSizeOfObject--) {
				debugC(1, kFreescapeDebugParser, "b: %x", file->readByte());
			}
			byteSizeOfObject = 0;
		}
		assert(byteSizeOfObject == 0);
		debugC(1, kFreescapeDebugParser, "End of object at %lx", file->pos());
		// create an entrance
		return new Entrance(
			objectID,
			32 * position,
			5 * v); // rotation
	} break;

	case Object::Sensor: {
		debugC(1, kFreescapeDebugParser, "rotation: %f %f %f", v.x(), v.y(), v.z());
		if (byteSizeOfObject > 0) {
			// TODO: there is something here
			debugC(1, kFreescapeDebugParser, "Warning: extra %d bytes in sensor", byteSizeOfObject);
			file->seek(byteSizeOfObject, SEEK_CUR);
			byteSizeOfObject = 0;
		}
		assert(byteSizeOfObject == 0);
		debugC(1, kFreescapeDebugParser, "End of object at %lx", file->pos());
		// create an entrance
		return new Sensor(
			objectID,
			32 * position,
			5 * v); // rotation
	} break;

	case Object::Group:
		error("Object of type 'group'");
		break;
	}

	file->seek(byteSizeOfObject, SEEK_CUR);
	return nullptr;
}

Area *FreescapeEngine::load8bitArea(Common::SeekableReadStream *file, uint16 ncolors) {

	uint32 base = file->pos();
	debugC(1, kFreescapeDebugParser, "Area base: %x", base);
	uint8 areaFlags = file->readByte();
	uint8 numberOfObjects = file->readByte();
	uint8 areaNumber = file->readByte();

	uint16 cPtr = file->readUint16LE();
	debugC(1, kFreescapeDebugParser, "Condition pointer: %x", cPtr);
	uint8 scale = file->readByte();
	debugC(1, kFreescapeDebugParser, "Scale: %d", scale);

	uint8 ci1 = 0;
	uint8 ci2 = 0;
	uint8 skyColor = 255;
	uint8 groundColor = 255;
	if (_targetName != "castlemaster") {
		groundColor = file->readByte() & 15;
		skyColor = file->readByte() & 15;
		ci1 = file->readByte();
		ci2 = file->readByte();
		debugC(1, kFreescapeDebugParser, "Colors: %d %d %d %d", ci1, ci2, skyColor, groundColor);
	} else {
		groundColor = file->readByte() & 15;
		skyColor = file->readByte() & 15;
		debugC(1, kFreescapeDebugParser, "Colors: %d %d", skyColor, groundColor);
	}

	if (_renderMode == "cga") {
		skyColor = skyColor % 4;
		groundColor = groundColor % 4;
	}

	Graphics::PixelBuffer *palette = getPalette(areaNumber, ci1, ci2, skyColor, groundColor, ncolors);

	debugC(1, kFreescapeDebugParser, "Area %d", areaNumber);
	debugC(1, kFreescapeDebugParser, "Flags: %d Objects: %d", areaFlags, numberOfObjects);
	//debug("Condition Ptr: %x", cPtr);
	debugC(1, kFreescapeDebugParser, "Pos before first object: %lx", file->pos());
	if (_targetName == "totaleclipse") {
		debugC(1, kFreescapeDebugParser, "b: %x", file->readByte());
		debugC(1, kFreescapeDebugParser, "b: %x", file->readByte());
		debugC(1, kFreescapeDebugParser, "b: %x", file->readByte());
		debugC(1, kFreescapeDebugParser, "b: %x", file->readByte());
		debugC(1, kFreescapeDebugParser, "b: %x", file->readByte());
	} else if (_targetName != "castlemaster")
		file->seek(15, SEEK_CUR);

	ObjectMap *objectsByID = new ObjectMap;
	ObjectMap *entrancesByID = new ObjectMap;
	for (uint8 object = 0; object < numberOfObjects; object++) {
		debugC(1, kFreescapeDebugParser, "Reading object: %d", object);
		Object *newObject = load8bitObject(file);

		if (newObject) {
			if (newObject->getType() == Object::Entrance) {
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
	assert(endLastObject == base + cPtr);
	file->seek(base + cPtr);
	uint8 numConditions = file->readByte();
	debugC(1, kFreescapeDebugParser, "%d area conditions at %x of area %d", numConditions, base + cPtr, areaNumber);

	Area *area = new Area(areaNumber, areaFlags, objectsByID, entrancesByID, scale, skyColor, groundColor, palette);

	while (numConditions--) {
		FCLInstructionVector instructions;
		// get the length
		uint32 lengthOfCondition = file->readByte();
		debugC(1, kFreescapeDebugParser, "length of condition: %d", lengthOfCondition);
		// get the condition
		byte *conditionData = (byte*)malloc(lengthOfCondition);
		file->read(conditionData, lengthOfCondition);
		Common::Array<uint8> conditionArray(conditionData, lengthOfCondition);
		Common::String *conditionSource = detokenise8bitCondition(conditionArray, instructions);
		area->conditions.push_back(instructions);
		area->conditionSources.push_back(conditionSource);
		debugC(1, kFreescapeDebugParser, "%s", conditionSource->c_str());
	}

	if (_areaMap.contains(255))
		area->addStructure(_areaMap[255]);
	else if (_targetName.hasPrefix("castle") || _targetName.hasPrefix("totaleclipse"))
		area->addStructure(nullptr);

	debugC(1, kFreescapeDebugParser, "End of area at %lx", file->pos());
	return area;
}

void FreescapeEngine::load8bitBinary(Common::SeekableReadStream *file, int offset, int ncolors) {
	Image::BitmapDecoder decoder;
	Common::File borderFile;

	if ((_renderMode == "ega" && borderFile.open("ega.bmp")) ||
	    (_renderMode == "cga" && borderFile.open("cga.bmp"))) {
		decoder.loadStream(borderFile);
		_border = new Graphics::Surface();
		_border->copyFrom(*decoder.getSurface());
	}

	file->seek(offset);
	uint8 numberOfAreas = file->readByte();
	uint16 dbSize = file->readUint16LE();
	debugC(1, kFreescapeDebugParser, "Number of areas: %d", numberOfAreas);
	debugC(1, kFreescapeDebugParser, "Database ends at %x", dbSize);

	uint8 startArea = file->readByte();
	debugC(1, kFreescapeDebugParser, "Start area: %d", startArea);
	uint8 startEntrance = file->readByte();
	debugC(1, kFreescapeDebugParser, "Entrace area: %d", startEntrance);

	file->seek(offset + 0x46); // 0x46

	uint16 globalSomething;
	globalSomething = file->readUint16LE();
	debugC(1, kFreescapeDebugParser, "Pointer to something: %x\n", globalSomething);

	uint16 globalByteCodeTable;
	globalByteCodeTable = file->readUint16LE();
	debugC(1, kFreescapeDebugParser, "GBCT: %d\n", globalByteCodeTable);

	file->seek(offset + globalByteCodeTable);
	uint8 numConditions = file->readByte();
	debugC(1, kFreescapeDebugParser, "%d global conditions", numConditions);
	while (numConditions--) {
		FCLInstructionVector instructions;
		// get the length
		uint32 lengthOfCondition = file->readByte();
		debugC(1, kFreescapeDebugParser, "length of condition: %d at %lx", lengthOfCondition, file->pos());
		// get the condition
		byte *conditionData = (byte*)malloc(lengthOfCondition);
		file->read(conditionData, lengthOfCondition);
		Common::Array<uint8> conditionArray(conditionData, lengthOfCondition);
		//debug("Global condition %d", numConditions + 1);
		Common::String *conditionSource = detokenise8bitCondition(conditionArray, instructions);
		_conditions.push_back(instructions);
		_conditionSources.push_back(conditionSource);
		debugC(1, kFreescapeDebugParser, "%s", conditionSource->c_str());
	}

	if (_targetName.hasPrefix("driller")) {
		file->seek(0x3b42);
		for (int i = 0; i < 8; i++)
			load8bitObject(file);
	}

	if (_targetName != "castlemaster")
		file->seek(offset + 0xc8);
	else
		file->seek(offset + 0x4f);

	debugC(1, kFreescapeDebugParser, "areas index at: %lx", file->pos());
	uint16 *fileOffsetForArea = new uint16[numberOfAreas];
	for (uint16 area = 0; area < numberOfAreas; area++) {
		fileOffsetForArea[area] = file->readUint16LE();
		debugC(1, kFreescapeDebugParser, "offset: %x", fileOffsetForArea[area]);
	}

	// grab the areas
	Area *newArea = nullptr;
	for (uint16 area = 0; area < numberOfAreas; area++) {
		debugC(1, kFreescapeDebugParser, "Area offset %d", fileOffsetForArea[area]);

		file->seek(offset + fileOffsetForArea[area]);
		newArea = load8bitArea(file, ncolors);

		if (newArea) {
			if (!_areaMap.contains(newArea->getAreaID()))
				_areaMap[newArea->getAreaID()] = newArea;
			else
				debugC(1, kFreescapeDebugParser, "WARNING: area ID repeated: %d", newArea->getAreaID());
		} else
			error("Invalid area?");
	}
	if (_targetName.hasPrefix("totaleclipse")) {
		_playerHeight = 48;
		_playerWidth = 8;
		_playerDepth = 8;
	} else {
		_playerHeight = 64;
		_playerWidth = 12;
		_playerDepth = 32;
	}

	if (!_areaMap.contains(startArea))
		_startArea = newArea->getAreaID();
	else
		_startArea = startArea;
	_startEntrance = startEntrance;
	_colorNumber = ncolors;
	_binaryBits = 8;
}

} // namespace Freescape