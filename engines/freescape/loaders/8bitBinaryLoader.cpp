#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"

#include "freescape/area.h"
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/language/instruction.h"
#include "freescape/language/parser.h"
#include "freescape/loaders/8bitBinaryLoader.h"
#include "freescape/loaders/loader.h"
#include "freescape/objects/geometricobject.h"
#include "freescape/objects/entrance.h"
#include "freescape/objects/object.h"

namespace Freescape {

static Object *load8bitObject(StreamLoader &stream) {
	
	Object::Type objectType = (Object::Type)(stream.get8() & 0x1F);
	Vector3d position, v;

	position.x() = stream.get8() * 32;
	position.y() = stream.get8() * 32;
	position.z() = stream.get8() * 32;

	v.x() = stream.get8();
	v.y() = stream.get8();
	v.z() = stream.get8();

	// object ID
	uint16 objectID = stream.get8();
	// size of object on disk; we've accounted for 8 bytes
	// already so we can subtract that to get the remaining
	// length beyond here
	uint8 byteSizeOfObject = stream.get8();
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
			uint8 c = stream.get8();
			colours->push_back(c >> 4);
			debug("color[%d] = %x", 2*colour, c >> 4);
			colours->push_back(c & 0xf);
			debug("color[%d] = %x", 2*colour+1, c & 0xf);
			byteSizeOfObject--;
		}

		// read extra vertices if required...
		int numberOfOrdinates = GeometricObject::numberOfOrdinatesForType(objectType);
		debug("number of ordinates %d", numberOfOrdinates);
		Common::Array<uint16> *ordinates = nullptr;

		if (numberOfOrdinates) {
			ordinates = new Common::Array<uint16>;
			uint16 ord = 0;

			for (int ordinate = 0; ordinate < numberOfOrdinates; ordinate++) {
				ord = stream.get8();
				debug("ord: %x", ord);
				ordinates->push_back(ord);
				byteSizeOfObject--;
			}
		}

		// grab the object condition, if there is one
		FCLInstructionVector instructions;
		if (byteSizeOfObject) {
			Common::Array<uint8> *conditionData = stream.nextBytes(byteSizeOfObject);

			Common::String *conditionSource = detokenise8bitCondition(*conditionData);
			//instructions = getInstructions(conditionSource);
			debug("%s", conditionSource->c_str());
		}

		// create an object
		return new GeometricObject(
			objectType,
			objectID,
			position,
			v, // size
			colours,
			ordinates,
			instructions);
	} break;

	case Object::Entrance: {
		debug("rotation: %f %f %f", v.x(), v.y(), v.z());
		assert(byteSizeOfObject == 0);
		// create an entrance
		return new Entrance(
			objectID,
			position,
			v); // rotation
	} break;

	case Object::Sensor:
	case Object::Group:
		break;
	}

	stream.skipBytes(byteSizeOfObject);
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

Common::Array <uint8>*getPaletteGradient(float *c1, float *c2, uint16 ncolors) {
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
	return raw_palette;
}


Area *load8bitArea(StreamLoader &stream, uint16 ncolors) {
	uint32 base = stream.getFileOffset();
	uint8 skippedValue = stream.get8();
	uint8 numberOfObjects = stream.get8();
	uint8 areaNumber = stream.get8();

	uint16 cPtr = stream.rget16();
	uint8 scale = stream.get8(); 
	uint8 ci1 = stream.get8()&15;
	uint8 ci2 = stream.get8()&15; 
	uint8 ci3 = stream.get8()&15; 
	uint8 ci4 = stream.get8()&15; 

	debug("Colors: %d %d %d %d", ci1, ci2, ci3, ci4);

	float *f1, *f2;
	f1 = specColors[ci3];
	f2 = specColors[ci4];

	Common::Array <uint8> *raw_palette = getPaletteGradient(f1, f2, ncolors);

	debug("Area %d", areaNumber);
	debug("Objects: %d", numberOfObjects);
	debug("Condition Ptr: %x", cPtr);

	stream.skipBytes(15);
	ObjectMap *objectsByID = new ObjectMap;
	ObjectMap *entrancesByID = new ObjectMap;

	for (uint8 object = 0; object < numberOfObjects; object++) {
		Object *newObject = load8bitObject(stream);

		if (newObject) {
			if (newObject->getType() == Object::Entrance) {
				(*entrancesByID)[newObject->getObjectID() & 0x7fff] = newObject;
			} else {
				(*objectsByID)[newObject->getObjectID()] = newObject;
			}
		}
	}
	stream.setFileOffset(base+cPtr);
	uint8 numConditions = stream.get8();
	debug("%d area conditions", numConditions);
	while (numConditions--) {
		// get the length
		uint32 lengthOfCondition = stream.get8();
		debug("length of condition: %d", lengthOfCondition);
		// get the condition
		Common::Array<uint8> *conditionData = stream.nextBytes(lengthOfCondition);

		debug("%s", detokenise8bitCondition(*conditionData)->c_str());
	}

	return (new Area(areaNumber, objectsByID, entrancesByID, 0, 1, raw_palette));
}

// struct BinaryTable {
// 	const char *filename;
// 	int ncolors;
// 	int offset;
// };

// static const BinaryTable binaryTable[] = {
// 	{ "DRILLE.EXE",  8,  0x9b40},
// 	{ "DRILLC.EXE",  4,  0x7bb0},
// 	{ "TOTE.EXE",    8,  0xcdb7},
//     //{ "TOTC.EXE",  8,  ??????},
// 	{ nullptr,       0,  0  }
// };

void FreescapeEngine::load8bitBinary(Common::SeekableReadStream *file, int offset, int ncolors) {
	const uint32 fileSize = file->size();
	byte *buf = (byte *)malloc(fileSize);
	file->read(buf, fileSize);

	Common::Array<uint8> binary;

	uint32 i = 0;
	while (i < fileSize) {
		binary.push_back(buf[i]);		
		i++;
	}

	StreamLoader streamLoader(binary);
	streamLoader.skipBytes(0x210);
	uint16 frameSize = streamLoader.get16();
	Common::Array<uint8> *raw_border = streamLoader.nextBytes(frameSize);
	debug("Found border image of size %x", frameSize);

	streamLoader.setFileOffset(offset);
	uint8 numberOfAreas = streamLoader.get8();
	uint16 dbSize = streamLoader.rget16();
	debug("Database ends at %x", dbSize);

	debug("Number of areas: %d", numberOfAreas);
	uint8 startArea = streamLoader.get8();
	debug("Start area: %d", startArea);
	uint8 entranceArea = streamLoader.get8();
	debug("Entrace area: %d", entranceArea);

	streamLoader.skipBytes(66);

	uint16 globalSomething;
	globalSomething = streamLoader.get16();
	debug("Pointer to something: %x\n", globalSomething);

	uint16 globalByteCodeTable;
	globalByteCodeTable = streamLoader.get16();
	debug("GBCT: %d\n", globalByteCodeTable);

	streamLoader.setFileOffset(offset + globalByteCodeTable);
	uint8 numConditions = streamLoader.get8();
	debug("%d global conditions", numConditions);
	while (numConditions--) {
		// get the length
		uint32 lengthOfCondition = streamLoader.get8();
		debug("length of condition: %d", lengthOfCondition);
		// get the condition
		Common::Array<uint8> *conditionData = streamLoader.nextBytes(lengthOfCondition);

		//debug("Global condition %d", numCondition + 1);
		debug("%s", detokenise8bitCondition(*conditionData)->c_str());
	}

	streamLoader.setFileOffset(offset + 200);
	uint16 *fileOffsetForArea = new uint16[numberOfAreas];
	for (uint16 area = 0; area < numberOfAreas; area++) {
		fileOffsetForArea[area] = streamLoader.rget16();
	}

	// grab the areas
	AreaMap *areaMap = new AreaMap;
	for (uint16 area = 0; area < numberOfAreas; area++) {
		debug("Area offset %d", fileOffsetForArea[area]);

		streamLoader.setFileOffset(offset + fileOffsetForArea[area]);
		Area *newArea = load8bitArea(streamLoader, ncolors);

		if (newArea) {
			(*areaMap)[newArea->getAreaID()] = newArea;
		}
	}
	_areasByAreaID = areaMap;
	_startArea = startArea;
	_colorNumber = ncolors;
	_binaryBits = 8;
}

} // namespace Freescape