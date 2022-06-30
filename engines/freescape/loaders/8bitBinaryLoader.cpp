#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"

#include "freescape/area.h"
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/language/instruction.h"
#include "freescape/language/parser.h"
#include "freescape/loaders/8bitBinaryLoader.h"
#include "freescape/objects/geometricobject.h"
#include "freescape/objects/entrance.h"
#include "freescape/objects/object.h"

namespace Freescape {

Object *FreescapeEngine::load8bitObject(Common::SeekableReadStream *file) {

	byte rawType = file->readByte();
	debug("Raw object type: %d", rawType);
	Object::Type objectType = (Object::Type)(rawType & 0x1F);

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
		for (uint8 colour = 0; colour < numberOfColours/2; colour++) {
			uint8 c = file->readByte();
			colours->push_back(c & 0xf);
			debug("color[%d] = %x", 2*colour, c & 0xf);
			colours->push_back(c >> 4);
			debug("color[%d] = %x", 2*colour+1, c >> 4);
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
				ordinates->push_back(ord);
				byteSizeOfObject--;
			}
		}

		// grab the object condition, if there is one
		FCLInstructionVector instructions;
		if (byteSizeOfObject) {
			byte *conditionData = (byte*)malloc(byteSizeOfObject);
			file->read(conditionData, byteSizeOfObject);
			Common::Array<uint8> conditionArray(conditionData, byteSizeOfObject);
			Common::String *conditionSource = detokenise8bitCondition(conditionArray);
			//instructions = getInstructions(conditionSource);
			//debug("%s", conditionSource->c_str());
		}
		debug("End of object at %lx", file->pos());

		// create an object
		return new GeometricObject(
			objectType,
			objectID,
			0, // flags
			32 * position,
			32 * v, // size
			colours,
			ordinates,
			instructions);
	} break;
	case Object::Entrance: {
		debug("rotation: %f %f %f", v.x(), v.y(), v.z());
		if (byteSizeOfObject > 0) {
			error("Extra bytes in entrance");
			//file->seek(byteSizeOfObject, SEEK_CUR);
			//return nullptr;
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

float specColors[16][3] = {
    {0, 0, 0},
	{0, 0, 0.75},
	{0.75, 0, 0},
	{0.75, 0, 0.75},
	{0, 0.75, 0},
	{0, 0.75, 0.75},
	{0.75, 0.75, 0},
	{0.75, 0.75, 0.75},
    {0, 0, 0},
	{0, 0, 1},
	{1, 0, 0},
	{1, 0, 1},
	{0, 1, 0},
	{0, 1, 1},
	{1, 1, 0},
	{1, 1, 1}
};

Graphics::PixelBuffer *getPaletteGradient(float *c1, float *c2, uint16 ncolors) {
	Common::Array <uint8> *raw_palette = new Common::Array <uint8>();
	uint16 y0, y1, y2;
	for(int c = 0; c < ncolors; c++)
	{
		float ic = (float)c / float(ncolors-1);
		ic = sqrt(ic);
		y0  = 255*(ic*c2[0] + (1-ic)*c1[0]);
		y1  = 255*(ic*c2[1] + (1-ic)*c1[1]);
		y2  = 255*(ic*c2[2] + (1-ic)*c1[2]);
		//debug("%x %x %x", y0, y1, y2);
		raw_palette->push_back(y0);
		raw_palette->push_back(y1);
		raw_palette->push_back(y2);
	}
	assert(ncolors == 16);
	assert(raw_palette->size() == ncolors * 3);
	Graphics::PixelFormat pixelFormat = Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
	Graphics::PixelBuffer *palette = new Graphics::PixelBuffer(pixelFormat, ncolors, DisposeAfterUse::NO); //TODO
	*palette = raw_palette->data();
	return palette;
}

Graphics::PixelBuffer *getPaletteGradient(uint8 c1, uint8 c2, uint16 ncolors) {
	Common::Array <uint8> *raw_palette = new Common::Array <uint8>();
	uint16 y0, y1, y2;

	for(int c = 0; c < ncolors; c++)
	{
		if (true /*c1 == 4 && c2 == 3*/) {
			switch (c) {
				case 0:
				case 1:
				y0 = 0x00; y1 = 0x00; y2 = 0x00;
				break;
				case 2:
				y0 = 0x00; y1 = 0xaa; y2 = 0xaa;
				break;
				case 3:
				y0 = 0xaa; y1 = 0x00; y2 = 0xaa;
				break;
				case 4:
				y0 = 0xff; y1 = 0xff; y2 = 0xff;
				break;
				case 5:
				y0 = 0x55; y1 = 0x55; y2 = 0x55;
				break;
				case 6:
				y0 = 0x00; y1 = 0x00; y2 = 0xaa;
				break;
				case 7:
				y0 = 0xaa; y1 = 0x55; y2 = 0x00;
				break;
				case 9:
				y0 = 0xaa; y1 = 0x00; y2 = 0x00;
				break;
				case 0xa:
				y0 = 0xff; y1 = 0x55; y2 = 0xff;
				break;
				default:
				y0 = 0x12; y1 = 0xf3; y2 = 0x56;
				break;
			}
		}
		raw_palette->push_back(y0);
		raw_palette->push_back(y1);
		raw_palette->push_back(y2);
	}
	//assert(ncolors == 16);
	assert(raw_palette->size() == ncolors * 3);
	Graphics::PixelFormat pixelFormat = Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
	Graphics::PixelBuffer *palette = new Graphics::PixelBuffer(pixelFormat, ncolors, DisposeAfterUse::NO); //TODO
	*palette = raw_palette->data();
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
	uint8 ci3 = 0;
	uint8 ci4 = 0;
	if (_targetName != "castlemaster") {
		ci1 = file->readByte() & 15;
		ci2 = file->readByte() & 15;
		ci3 = file->readByte() & 15;
		ci4 = file->readByte() & 15;
		debug("Colors: %d %d %d %d", ci1, ci2, ci3, ci4);
	} else {
		ci3 = file->readByte();
		ci4 = file->readByte();
		debug("Colors: %d %d", ci3, ci4);
	}
	Graphics::PixelBuffer *palette = getPaletteGradient(1, 1, ncolors);

	debug("Area %d", areaNumber);
	debug("Skipped: %d Objects: %d", skippedValue, numberOfObjects);
	//debug("Condition Ptr: %x", cPtr);
	debug("Pos before first object: %lx", file->pos());

	if (_targetName != "castlemaster")
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
				(*entrancesByID)[newObject->getObjectID() & 0x7fff] = newObject;
			} else {
				if (objectsByID->contains(newObject->getObjectID()))
					debug("WARNING: replacing object id %d", newObject->getObjectID());
				(*objectsByID)[newObject->getObjectID()] = newObject;
			}
		}
	}
	long int endLastObject = file->pos();
	debug("Last position %lx", endLastObject);
	assert(endLastObject <= base + cPtr);
	file->seek(base + cPtr);
	uint8 numConditions = file->readByte();
	debug("%d area conditions", numConditions);
	while (numConditions--) {
		// get the length
		uint32 lengthOfCondition = file->readByte();
		debug("length of condition: %d", lengthOfCondition);
		// get the condition
		byte *conditionData = (byte*)malloc(lengthOfCondition);
		file->read(conditionData, lengthOfCondition);
		Common::Array<uint8> conditionArray(conditionData, lengthOfCondition);
		debug("%s", detokenise8bitCondition(conditionArray)->c_str());
	}

	return (new Area(areaNumber, objectsByID, entrancesByID, scale, 255, 255, palette));
}

// struct BinaryTable {
// 	const char *filename;
// 	int ncolors;
// 	int offset;
// };

// static const BinaryTable binaryTable[] = {
// 	{ "DRILLE.EXE",  16,  0x9b40},
// 	{ "DRILLC.EXE",  4,  0x7bb0},
// 	{ "TOTE.EXE",    16,  0xcdb7},
//  { "TOTC.EXE",    16,  ??????},
// 	{ nullptr,       0,  0  }
// };

void FreescapeEngine::load8bitBinary(Common::SeekableReadStream *file, int offset, int ncolors) {
	//const uint32 fileSize = file->size();
	file->seek(0x210);
	uint16 frameSize = file->readUint16BE();
	//Common::Array<uint8> *raw_border = streamLoader.nextBytes(frameSize);
	debug("Found border image of size %x", frameSize);

	file->seek(offset);
	uint8 numberOfAreas = file->readByte();
	if (numberOfAreas < 2) // TODO: just for testing
		numberOfAreas = 20;
	uint16 dbSize = file->readUint16LE();
	debug("Database ends at %x", dbSize);

	debug("Number of areas: %d", numberOfAreas);

	uint8 startArea = file->readByte();
	debug("Start area: %d", startArea);
	uint8 startEntrance = file->readByte();
	debug("Entrace area: %d", startEntrance);

	//streamLoader.skipBytes(66);
	file->seek(66, SEEK_CUR);

	uint16 globalSomething;
	globalSomething = file->readUint16BE();
	debug("Pointer to something: %x\n", globalSomething);

	uint16 globalByteCodeTable;
	globalByteCodeTable = file->readUint16BE();
	debug("GBCT: %d\n", globalByteCodeTable);

	file->seek(offset + globalByteCodeTable);
	uint8 numConditions = file->readByte();
	debug("%d global conditions", numConditions);
	while (numConditions--) {
		// get the length
		uint32 lengthOfCondition = file->readByte();
		debug("length of condition: %d", lengthOfCondition);
		// get the condition
		byte *conditionData = (byte*)malloc(lengthOfCondition);
		file->read(conditionData, lengthOfCondition);
		Common::Array<uint8> conditionArray(conditionData, lengthOfCondition);
		//debug("Global condition %d", numCondition + 1);
		//debug("%s", detokenise8bitCondition(conditionArray)->c_str());
	}

	file->seek(offset + 200);
	debug("areas index at: %lx", file->pos());
	uint16 *fileOffsetForArea = new uint16[numberOfAreas];
	for (uint16 area = 0; area < numberOfAreas; area++) {
		fileOffsetForArea[area] = file->readUint16LE();
		debug("offset: %x", fileOffsetForArea[area]);
	}
	//fileOffsetForArea[0] = 0x9e75 - offset - 8 - 12; // Table?
	//fileOffsetForArea[0] = 0x9571 - offset;
	//fileOffsetForArea[0] = 0xaba5 - offset - 8 - 16; // ???

	//fileOffsetForArea[0] = 0x959b - offset - 16; // Church? (22 elements)
	//fileOffsetForArea[0] = 0x94b7 - offset; // For testing
	//fileOffsetForArea[0] = 0x97cb - offset; // For testing

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
		if (_targetName == "castlemaster")
			break;
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