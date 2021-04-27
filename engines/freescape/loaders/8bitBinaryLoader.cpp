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
#include "freescape/objects/object.h"

namespace Freescape {

Area *load8bitArea(StreamLoader &stream) {
	uint8 skippedValue = stream.get8(); // 0
	uint8 numberOfObjects = stream.get8(); // 1
	uint8 areaNumber = stream.get8(); // 2

	debug("Area %d", areaNumber);
	debug("Objects: %d", numberOfObjects);

	uint16 cPtr = stream.rget16(); // 4
	uint8 scale = stream.get8(); // 5
	stream.get16(); // 6-7
	stream.get16(); // 8-9
	// Set16PaletteGradient(SpecCols[IPtr[8]&15], SpecCols[IPtr[9]&15]);

	stream.setFileOffset(cPtr);
	uint8 numConditions = stream.get8();
	debug("%d area conditions", numConditions);
	while (numConditions--) {
		// get the length
		uint32 lengthOfCondition = stream.get8();
		debug("length of condition: %d", lengthOfCondition);
		// get the condition
		Common::Array<uint8> *conditionData = stream.nextBytes(lengthOfCondition);

		//debug("Global condition %d", numCondition + 1);
		debug("%s", detokenise8bitCondition(*conditionData)->c_str());
	}


	/*Uint8 *ObjPtr = &IPtr[(Offset == OFFSET_TOTALECLIPSE) ? 15 : 25];
			while(NumObjects--)
			{*/

	/*
	// I've yet to decipher this fully
	stream.get16();
	stream.get16();
	stream.get16();
	

	uint8 skyColor = stream.get8();
	skyColor = (stream.get8() << 4) | skyColor;

	debug("Sky color %x", skyColor);
	uint8 groundColor = stream.get8();
	groundColor = (stream.get8() << 4) | groundColor;
	debug("Ground color %x", groundColor);
	stream.skipBytes(14);

	//for (int paletteEntry = 0; paletteEntry < 22; paletteEntry++) {
	//	uint8 paletteColour = stream.get8() << 2;
	//	debug("Palette colour (?) %x", paletteColour);
	//}

	// we'll need to collate all objects and entrances; it's likely a
	// plain C array would do but maps are safer and the total application
	// cost is going to be negligible regardless
	ObjectMap *objectsByID = new ObjectMap;
	ObjectMap *entrancesByID = new ObjectMap;

	// get the objects or whatever; entrances use a unique numbering
	// system and have the high bit of their IDs set in the original file
	for (uint16 object = 0; object < numberOfObjects; object++) {
		Object *newObject = loadObject(stream);

		if (newObject) {
			if (newObject->getType() == Object::Entrance) {
				(*entrancesByID)[newObject->getObjectID() & 0x7fff] = newObject;
			} else {
				(*objectsByID)[newObject->getObjectID()] = newObject;
			}
		}
	}

	return (new Area(areaNumber, objectsByID, entrancesByID, skyColor, groundColor));
	*/
	return nullptr;
}


Binary load8bitBinary(Common::String filename, uint offset) {
	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		delete file;
		error("NULL");
	}

	const uint32 fileSize = file->size();
	byte *buf = (byte *)malloc(fileSize);
	file->read(buf, fileSize);

	Common::Array<uint8> binary;

	uint32 i = offset;
	while (i < fileSize) {
		binary.push_back(buf[i]);
		i++;
	}

	StreamLoader streamLoader(binary);
	uint8 numberOfAreas = streamLoader.get8();
	streamLoader.get16(); // meaning unknown

	debug("Number of areas: %d", numberOfAreas);
	uint8 startArea = streamLoader.get8();
	debug("Start area: %d", startArea);
	uint8 entranceArea = streamLoader.get8();
	debug("Entrace area: %d", entranceArea);
	streamLoader.skipBytes(68);
	uint16 globalByteCodeTable;
	globalByteCodeTable = streamLoader.get16();
	debug("GBCT: %d\n", globalByteCodeTable);

	streamLoader.setFileOffset(globalByteCodeTable);
	//uint8 *ConditionPointer = &Base[GlobalByteCodeTable];
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

	streamLoader.setFileOffset(200);
	uint16 *fileOffsetForArea = new uint16[numberOfAreas];
	for (uint16 area = 0; area < numberOfAreas; area++) {
		fileOffsetForArea[area] = streamLoader.rget16();
	}

	// grab the areas
	AreaMap *areaMap = new AreaMap;
	for (uint16 area = 0; area < numberOfAreas; area++) {
		debug("Area offset %d", fileOffsetForArea[area]);

		streamLoader.setFileOffset(fileOffsetForArea[area]);
		Area *newArea = load8bitArea(streamLoader);

		if (newArea) {
			(*areaMap)[newArea->getAreaID()] = newArea;
		}
	}

	return Binary{0, nullptr, nullptr, nullptr};
}

} // namespace Freescape