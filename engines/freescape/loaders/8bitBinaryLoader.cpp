#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"

#include "freescape/area.h"
#include "freescape/loaders/loader.h"
//#include "freescape/language/8bitDetokeniser.h"
#include "freescape/language/instruction.h"
#include "freescape/language/parser.h"
#include "freescape/loaders/8bitBinaryLoader.h"
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
	streamLoader.get8(); // meaning unknown

	debug("Number of areas: %d", numberOfAreas);
	uint8 startArea = streamLoader.get8();
    debug("Start area: %d", startArea);



    return Binary{nullptr, nullptr};
}

}