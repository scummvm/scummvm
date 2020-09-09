#include "agds/patch.h"
#include "agds/resourceManager.h"
#include "common/stream.h"
#include "common/debug.h"
#include "common/error.h"

namespace AGDS {

void Patch::load(Common::SeekableReadStream *stream) {
	byte extended = stream->readByte();
	if (extended != 1 && extended != 0) {
		Common::String prototype = (char)extended + readString(stream, 31);
		Common::String unk = readString(stream);
		debug("patch for object: %s %s", prototype.c_str(), unk.c_str());
		return;
	}

	screenRegionName = readString(stream);
	prevScreenName = readString(stream);
	if (extended == 0)
		return;

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

	defaultMouseCursor = readString(stream);
	debug("default pointer name: %s", defaultMouseCursor.c_str());
	objects.clear();
	for(uint i = 0; i < object_count; ++i) {
		int flag = stream->readSint16LE();
		Common::String name = readString(stream);
		objects.push_back(Object(name, flag));
	}
}

}
