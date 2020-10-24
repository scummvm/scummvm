/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/memstream.h"
#include "dgds/console.h"
#include "dgds/decompress.h"
#include "dgds/dgds.h"
#include "dgds/includes.h"
#include "dgds/parser.h"
#include "dgds/resource.h"
#include "gui/debugger.h"

namespace Dgds {

Console::Console(DgdsEngine *vm) : _vm(vm) {
	registerCmd("fileinfo", WRAP_METHOD(Console, cmdFileInfo));
	registerCmd("filesearch", WRAP_METHOD(Console, cmdFileSearch));
	registerCmd("filedump", WRAP_METHOD(Console, cmdFileDump));
}

bool Console::cmdFileInfo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <file>\n", argv[0]);
		return true;
	}

	Resource res = _vm->_resource->getResourceInfo(argv[1]);
	debugPrintf("Resource volume: %d, position: %d, size: %d, checksum: %d\n", res.volume, res.pos, res.size, res.checksum);

	return true;
}

bool Console::cmdFileSearch(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <file to search>\n", argv[0]);
		return true;
	}

	ResourceList resources = _vm->_resource->_resources;
	for (ResourceList::const_iterator i = resources.begin(), end = resources.end(); i != end; ++i) {
		if (i->_key.contains(argv[1])) {
			Resource res = i->_value;
			debugPrintf("Resource: %s, volume: %d, position: %d, size: %d, checksum: %d\n", i->_key.c_str(), res.volume, res.pos, res.size, res.checksum);
		}
	}

	return true;
}

bool Console::cmdFileDump(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <file> <ignore patches> <unpack>\n", argv[0]);
		return true;
	}

	Common::String fileName = argv[1];
	bool ignorePatches = (argc > 2) && (!scumm_stricmp(argv[2], "true") || !strcmp(argv[2], "1"));
	bool unpack = (argc > 3) && (!scumm_stricmp(argv[3], "true") || !strcmp(argv[3], "1"));
	Common::SeekableReadStream *res = _vm->_resource->getResource(fileName, ignorePatches);
	if (res == nullptr) {
		debugPrintf("Resource not found\n");
		return true;
	}
	int32 size = res->size();
	byte *data;

	if (!unpack) {
		data = new byte[size];
		res->read(data, size);
	} else {
		data = new byte[2000000]; // about 2MB, but maximum container size is around 1.5MB
		byte *ptr = data;
		const char *dot;
		DGDS_EX ex = 0;
		size = 0;

		if ((dot = strrchr(fileName.c_str(), '.'))) {
			ex = MKTAG24(dot[1], dot[2], dot[3]);
		}

		Decompressor *decompressor = new Decompressor();
		DgdsParser ctx(*res, fileName.c_str());
		DgdsChunk chunk;
		while (chunk.readHeader(ctx)) {
			Common::SeekableReadStream *stream = chunk.isPacked(ex) ? chunk.decodeStream(ctx, decompressor) : chunk.readStream(ctx);

			memcpy(ptr, chunk._idStr, 4);
			ptr += 4;

			stream->read(ptr, stream->size());
			ptr += stream->size();

			size += 4 + stream->size();
		}

		delete decompressor;
	}

	delete res;
	
	Common::DumpFile out;
	out.open(fileName);
	out.write(data, size);
	out.flush();
	out.close();
	delete[] data;

    return true;
}

} // End of namespace Dgds
