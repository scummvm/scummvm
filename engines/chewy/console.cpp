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

#include "chewy/console.h"
#include "gui/debugger.h"
#include "chewy/chewy.h"
#include "chewy/resource.h"

namespace Chewy {

	Console::Console(ChewyEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("dump",			WRAP_METHOD(Console, Cmd_Dump));
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
	TBFChunk *chunk = res->getChunk(resNum);
	byte *data = res->getChunkData(resNum);

	Common::DumpFile outFile;
	outFile.open(dumpFilename);
	outFile.write(data, chunk->unpackedSize);
	outFile.flush();
	outFile.close();

	delete[] data;
	delete res;

	return true;
}

} // End of namespace Chewy
