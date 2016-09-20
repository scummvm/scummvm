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

#include "gui/debugger.h"

#include "chewy/chewy.h"
#include "chewy/console.h"
#include "chewy/graphics.h"
#include "chewy/resource.h"

namespace Chewy {

Console::Console(ChewyEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("dump",			WRAP_METHOD(Console, Cmd_Dump));
	registerCmd("draw",			WRAP_METHOD(Console, Cmd_Draw));
}

Console::~Console() {
}

bool Console::Cmd_Dump(int argc, const char **argv) {
	if (argc < 4) {
		debugPrintf("Usage: dump <file> <resource number> <dump file name>\n");
		return true;
	}

	Common::String filename = argv[1];
	int resNum = atoi(argv[2]);
	Common::String dumpFilename = argv[3];

	Resource *res = new Resource(filename);
	Chunk *chunk = res->getChunk(resNum);
	byte *data = res->getChunkData(resNum);
	uint32 size = chunk->size;
	if (chunk->type == kResourceTBF) {
		TBFChunk *tbf = res->getTBFChunk(resNum);
		size = tbf->unpackedSize;
	}

	Common::DumpFile outFile;
	outFile.open(dumpFilename);
	outFile.write(data, size);
	outFile.flush();
	outFile.close();

	delete[] data;
	delete res;

	return true;
}

bool Console::Cmd_Draw(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: draw <file> <resource number>\n");
		return true;
	}

	Common::String filename = argv[1];
	int resNum = atoi(argv[2]);
	
	Graphics *g = new Graphics();
	g->drawImage(filename, resNum);
	delete g;

	return false;
}

} // End of namespace Chewy
