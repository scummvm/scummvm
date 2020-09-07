#include "agds/patch.h"
#include "agds/resourceManager.h"
#include "common/stream.h"
#include "common/debug.h"
#include "common/error.h"

namespace AGDS {

void Patch::load(Common::SeekableReadStream *stream) {
	bool regionNameValid = stream->readByte();
	if (regionNameValid) {
		screenRegionName = readString(stream);
	} else {
		stream->skip(32);
	}
	prevScreenName = readString(stream);
	unk41 = stream->readUint32LE();
	characterX = stream->readUint32LE();
	characterY = stream->readUint32LE();
	characterDirection = stream->readUint32LE();
	unk51 = stream->readUint32LE();
	debug("unknown entries: %u (character at %u,%u with dir: %u) %u", unk41, characterX, characterY, characterDirection, unk51);
	uint object_count = stream->readUint32LE();
	debug("objects in this patch: %u", object_count);
	if (stream->read(palette, sizeof(palette)) != sizeof(palette)) {
		error("short read, can't read palette");
	}

	defaultMousePointerName = readString(stream);
	debug("default pointer name: %s", defaultMousePointerName.c_str());
	objects.clear();
	for(uint i = 0; i < object_count; ++i) {
		int flag = stream->readSint16LE();
		Common::String name = readString(stream);
		debug("object %s, status: %d", name.c_str(), flag);
	}
}

}
