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

	return Binary{nullptr, nullptr};
}

} // namespace Freescape