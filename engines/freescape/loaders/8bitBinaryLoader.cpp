#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"
#include "image/bmp.h"

#include "freescape/freescape.h"
#include "freescape/area.h"
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/language/instruction.h"
#include "freescape/objects/geometricobject.h"
#include "freescape/objects/entrance.h"
#include "freescape/objects/object.h"

namespace Freescape {

Object *FreescapeEngine::load8bitObject(Common::SeekableReadStream *file) {

	byte rawFlagsAndType = file->readByte();
	debug("Raw object data flags and type: %d", rawFlagsAndType);
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
	debug("Raw object %d ; type %d ; size %d", objectID, (int)objectType, byteSizeOfObject);
	if (byteSizeOfObject < 9) {
		error("Not enough bytes %d to read object %d with type %d", byteSizeOfObject, objectID, objectType);
		//file->seek(byteSizeOfObject, SEEK_CUR);
		//return nullptr;
	}

	assert(byteSizeOfObject >= 9);
	byteSizeOfObject = byteSizeOfObject - 9;
	debug("Object %d ; type %d ; size %d", objectID, (int)objectType, byteSizeOfObject);
    debug("pos: %f %f %f", position.x(), position.y(), position.z());
	switch (objectType) {
	default: {
		debug("size: %f %f %f", v.x(), v.y(), v.z());
		// read the appropriate number of colours
		int numberOfColours = GeometricObject::numberOfColoursForObjectOfType(objectType);
		Common::Array<uint8> *colours = new Common::Array<uint8>;
		debug("Number of colors: %d", numberOfColours/2);
		uint8 entry;
		for (uint8 colour = 0; colour < numberOfColours/2; colour++) {
			uint8 data = file->readByte();
			entry = data & 0xf;
			if (_renderMode == "cga")
				entry = entry % 4; // TODO: use dithering

			colours->push_back(entry);
			debug("color[%d] = %x", 2*colour, entry);

			entry = data >> 4;
			if (_renderMode == "cga")
				entry = entry % 4; // TODO: use dithering

			colours->push_back(entry);
			debug("color[%d] = %x", 2*colour+1, entry);
			byteSizeOfObject--;
		}

		// read extra vertices if required...
		int numberOfOrdinates = GeometricObject::numberOfOrdinatesForType(objectType);
		debug("number of ordinates %d", numberOfOrdinates);
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
				debug("ord: %x", ord);
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
			debug("%s", conditionSource->c_str());
		}
		debug("End of object at %lx", file->pos());

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
		debug("rotation: %f %f %f", v.x(), v.y(), v.z());
		if (byteSizeOfObject > 0) {
			// TODO: there is something here
			debug("Warning: extra %d bytes in entrance", byteSizeOfObject);
			file->seek(byteSizeOfObject, SEEK_CUR);
			byteSizeOfObject = 0;
		}
		assert(byteSizeOfObject == 0);
		debug("End of object at %lx", file->pos());
		// create an entrance
		return new Entrance(
			objectID,
			32 * position,
			5 * v); // rotation
	} break;

	case Object::Sensor:
	case Object::Group:
		break;
	}

	file->seek(byteSizeOfObject, SEEK_CUR);
	return nullptr;
}

byte drillerEGA[16][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0xaa, 0xaa},
	{0xaa, 0x00, 0xaa},
	{0xff, 0xff, 0xff},
	{0x55, 0x55, 0x55},
	{0x00, 0x00, 0xaa},
	{0xaa, 0x55, 0x00},
	{0x12, 0xf3, 0x56},
	{0xaa, 0x00, 0x00},
	{0xff, 0x55, 0xff},
	{0x12, 0xf3, 0x56},
	{0x12, 0xf3, 0x56},
	{0x12, 0xf3, 0x56},
	{0x12, 0xf3, 0x56},
	{0x12, 0xf3, 0x56}
};

byte drillerCGA[4][3] = {
	{0x00, 0x00, 0x00},
	{0xff, 0xff, 0xff},
	{0xa8, 0x00, 0xa8},
	{0x00, 0xa8, 0xa8},
};

byte castleCGA[4][3] = {
	{0x83, 0x85, 0x83},
	{0x00, 0x00, 0x00},
	{0x00, 0x85, 0x00},
	{0xff, 0xfb, 0xff},
};

byte eclipseEGA[16][3] = {
	{0xfc, 0xfc, 0x54},
	{0x00, 0x00, 0x00},
	{0x54, 0xfc, 0xfc},
	{0xaa, 0x00, 0xaa},
	{0x54, 0xfc, 0xfc},
	{0x00, 0x00, 0xA8},
	{0x00, 0x00, 0xaa},
	{0xaa, 0x55, 0x00},
	{0x12, 0xf3, 0x56},
	{0xaa, 0x00, 0x00},
	{0xff, 0x55, 0xff},
	{0xa8, 0x00, 0x00},
	{0x12, 0xf3, 0x56},
	{0x54, 0x54, 0x54},
	{0xa8, 0x54, 0x00},
	{0x12, 0xf3, 0x56}
};

Graphics::PixelBuffer *FreescapeEngine::getPalette(uint8 areaNumber, uint8 c1, uint8 c2, uint8 c3, uint8 c4, uint16 ncolors) {
	Graphics::PixelFormat pixelFormat = Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
	Graphics::PixelBuffer *palette = nullptr;
	if (_targetName.hasPrefix("driller")) {
		if (_renderMode == "ega")
			palette = new Graphics::PixelBuffer(pixelFormat, (byte*)&drillerEGA);
		else if (_renderMode == "cga")
			palette = new Graphics::PixelBuffer(pixelFormat, (byte*)&drillerCGA);
	} else if (_targetName.hasPrefix("castlemaster")) {
		if (_renderMode == "ega")
			palette = new Graphics::PixelBuffer(pixelFormat, (byte*)&drillerEGA); // TODO
		else if (_renderMode == "cga")
			palette = new Graphics::PixelBuffer(pixelFormat, (byte*)&castleCGA);
	} else if (_targetName.hasPrefix("totaleclipse")) {
		palette = new Graphics::PixelBuffer(pixelFormat, (byte*)&eclipseEGA);
	} else
		palette = new Graphics::PixelBuffer(pixelFormat, (byte*)&drillerEGA);

	assert(palette);
	return palette;
}

Area *FreescapeEngine::load8bitArea(Common::SeekableReadStream *file, uint16 ncolors) {

	uint32 base = file->pos();
	debug("Area base: %x", base);
	uint8 skippedValue = file->readByte();
	uint8 numberOfObjects = file->readByte();
	uint8 areaNumber = file->readByte();

	uint16 cPtr = file->readUint16LE();
	debug("Condition pointer: %x", cPtr);
	uint8 scale = file->readByte();
	debug("Scale: %d", scale);

	uint8 ci1 = 0;
	uint8 ci2 = 0;
	uint8 skyColor = 255;
	uint8 groundColor = 255;
	if (_targetName != "castlemaster") {
		ci1 = file->readByte() & 15;
		ci2 = file->readByte() & 15;
		skyColor = file->readByte() & 15;
		groundColor = file->readByte() & 15;
		debug("Colors: %d %d %d %d", ci1, ci2, skyColor, groundColor);
	} else {
		skyColor = file->readByte() & 15;
		groundColor = file->readByte() & 15;
		debug("Colors: %d %d", skyColor, groundColor);
	}

	if (_renderMode == "cga") {
		skyColor = skyColor % 4;
		groundColor = groundColor % 4;
	}

	Graphics::PixelBuffer *palette = getPalette(areaNumber, ci1, ci2, skyColor, groundColor, ncolors);

	debug("Area %d", areaNumber);
	debug("Skipped: %d Objects: %d", skippedValue, numberOfObjects);
	//debug("Condition Ptr: %x", cPtr);
	debug("Pos before first object: %lx", file->pos());

	if (_targetName == "totaleclipse") {
		file->seek(5, SEEK_CUR);
	} else if (_targetName != "castlemaster")
		file->seek(15, SEEK_CUR);

	ObjectMap *objectsByID = new ObjectMap;
	ObjectMap *entrancesByID = new ObjectMap;
	for (uint8 object = 0; object < numberOfObjects; object++) {
		debug("Reading object: %d", object);
		Object *newObject = load8bitObject(file);

		if (newObject) {
			//if (newObject->getObjectID() == 255) // TODO: fix me?
			//	break;
			if (newObject->getType() == Object::Entrance) {
				if (entrancesByID->contains(newObject->getObjectID() & 0x7fff))
					error("WARNING: replacing object id %d (%d)", newObject->getObjectID(), newObject->getObjectID() & 0x7fff);

				(*entrancesByID)[newObject->getObjectID() & 0x7fff] = newObject;
			} else {
				if (objectsByID->contains(newObject->getObjectID()))
					error("WARNING: replacing object id %d", newObject->getObjectID());
				(*objectsByID)[newObject->getObjectID()] = newObject;
			}
		}
	}
	long int endLastObject = file->pos();
	debug("Last position %lx", endLastObject);
	assert(endLastObject == base + cPtr);
	file->seek(base + cPtr);
	uint8 numConditions = file->readByte();
	debug("%d area conditions at %x of area %d", numConditions, base + cPtr, areaNumber);
	while (numConditions--) {
		FCLInstructionVector instructions;
		// get the length
		uint32 lengthOfCondition = file->readByte();
		debug("length of condition: %d", lengthOfCondition);
		// get the condition
		byte *conditionData = (byte*)malloc(lengthOfCondition);
		file->read(conditionData, lengthOfCondition);
		Common::Array<uint8> conditionArray(conditionData, lengthOfCondition);
		debug("%s", detokenise8bitCondition(conditionArray, instructions)->c_str());
	}
	debug("End of area at %lx", file->pos());
	return (new Area(areaNumber, objectsByID, entrancesByID, scale, skyColor, groundColor, palette));
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
	debug("Database ends at %x", dbSize);

	debug("Number of areas: %d", numberOfAreas);

	uint8 startArea = file->readByte();
	debug("Start area: %d", startArea);
	uint8 startEntrance = file->readByte();
	debug("Entrace area: %d", startEntrance);

	file->seek(0x42, SEEK_CUR);

	uint16 globalSomething;
	globalSomething = file->readUint16BE();
	debug("Pointer to something: %x\n", globalSomething);

	file->seek(offset + 0x48);

	uint16 globalByteCodeTable;
	globalByteCodeTable = file->readUint16LE();
	debug("GBCT: %d\n", globalByteCodeTable);

	file->seek(offset + globalByteCodeTable);
	uint8 numConditions = file->readByte();
	debug("%d global conditions", numConditions);
	while (numConditions--) {
		FCLInstructionVector instructions;
		// get the length
		uint32 lengthOfCondition = file->readByte();
		debug("length of condition: %d at %lx", lengthOfCondition, file->pos());
		// get the condition
		byte *conditionData = (byte*)malloc(lengthOfCondition);
		file->read(conditionData, lengthOfCondition);
		Common::Array<uint8> conditionArray(conditionData, lengthOfCondition);
		//debug("Global condition %d", numConditions + 1);
		Common::String *conditions = detokenise8bitCondition(conditionArray, instructions);
		debug("%s", conditions->c_str());
	}

	if (_targetName != "castlemaster")
		file->seek(offset + 0xc8);
	else
		file->seek(offset + 0x4f);

	debug("areas index at: %lx", file->pos());
	uint16 *fileOffsetForArea = new uint16[numberOfAreas];
	for (uint16 area = 0; area < numberOfAreas; area++) {
		fileOffsetForArea[area] = file->readUint16LE();
		debug("offset: %x", fileOffsetForArea[area]);
	}

	// grab the areas
	AreaMap *areaMap = new AreaMap;
	Area *newArea = nullptr;
	for (uint16 area = 0; area < numberOfAreas; area++) {
		debug("Area offset %d", fileOffsetForArea[area]);

		file->seek(offset + fileOffsetForArea[area]);
		newArea = load8bitArea(file, ncolors);

		if (newArea) {
			if (!areaMap->contains(newArea->getAreaID()))
				(*areaMap)[newArea->getAreaID()] = newArea;
			else
				debug("WARNING: area ID repeated: %d", newArea->getAreaID());
		} else
			error("Invalid area?");
	}
	_playerHeight = 64;
	_playerWidth = 32;
	_playerDepth = 32;

	_areasByAreaID = areaMap;
	if (!areaMap->contains(startArea))
		_startArea = newArea->getAreaID();
	else
		_startArea = startArea;
	_startEntrance = startEntrance;
	_colorNumber = ncolors;
	_binaryBits = 8;
}

} // namespace Freescape