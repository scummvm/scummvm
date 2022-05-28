/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "mm/mm1/console.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

Console::Console() : GUI::Debugger() {
	registerCmd("dump_map", WRAP_METHOD(Console, Cmd_DumpMap));
	registerCmd("map_string", WRAP_METHOD(Console, Cmd_MapString));
}

bool Console::Cmd_DumpMap(int argc, const char **argv) {
	Common::File f;
	Common::OutSaveFile *dest;

	if (argc != 2) {
		debugPrintf("%s <roomNum>\n", argv[0]);
	} else {
		int mapId = strToInt(argv[1]);
		Maps::Map *map = g_globals->_maps.getMap(mapId);

		// Dump the map data
		if (f.open("mazedata.dta")) {
			dest = g_system->getSavefileManager()->openForSaving(
				Common::String::format("map%.2d-maze.bin", mapId), false);
			assert(dest);
			byte buffer[512];

			f.seek(mapId * 512);
			f.read(buffer, 512);
			dest->write(buffer, 512);

			dest->finalize();
			delete dest;
			f.close();
		}

		if (f.open(Common::String::format("%s.ovr", map->getName().c_str()))) {
			int magicId = f.readUint16LE();
			int codePtr = f.readUint16LE();
			int codeSize = f.readUint16LE();
			int dataPtr = f.readUint16LE();
			int dataSize = f.readUint16LE();
			int extrasSize = f.readUint16LE();
			int startPtr = f.readUint16LE();

			assert(magicId == 0xf2);
			assert(startPtr >= codePtr &&
				startPtr < (codePtr + codeSize));

			dest = g_system->getSavefileManager()->openForSaving(
				Common::String::format("map%.2d-code.bin", mapId), false);
			byte *code = new byte[codeSize];
			f.read(code, codeSize);

			for (int i = 0; i < (codePtr % 16); ++i)
				dest->writeByte(0);
			dest->write(code, codeSize);
			dest->flush();
			delete dest;
			delete[] code;

			dest = g_system->getSavefileManager()->openForSaving(
				Common::String::format("map%.2d-data.bin", mapId), false);
			byte *data = new byte[dataSize];
			f.read(data, dataSize);
			dest->write(data, dataSize);
			dest->flush();
			delete dest;
			delete[] data;

			f.close();

			debugPrintf("data: ptr=%xh, size=%xh\n", dataPtr, dataSize);
			debugPrintf("code: ptr=%xh, size=%xh start=%xh\n",
				codePtr, codeSize, startPtr);
			debugPrintf("Extras size=%xh\n", extrasSize);
		}

		debugPrintf("Done.\n");
	}

	return true;
}

bool Console::Cmd_MapString(int argc, const char **argv) {
	Common::File f;

	if (argc != 3) {
		debugPrintf("%s <map Id> <offset>\n");
	} else {
		int mapId = strToInt(argv[1]);
		Maps::Map *map = g_globals->_maps.getMap(mapId);
		int offset = strToInt(Common::String::format(
			"%sh", argv[2]).c_str());

		if (!f.open(Common::String::format("%s.ovr", map->getName().c_str())))
			error("Failed to open map");

		f.readUint16LE();
		f.readUint16LE();
		int codeSize = f.readUint16LE();
		int dataPtr = f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();

		f.skip(codeSize);
		f.skip(offset - dataPtr);

		// Read the string
		Common::String s;
		char c;
		
		while ((c = f.readByte()) != '\0') {
			if (c == '\r') {
				s += "\\n";
				debugPrintf("%s\n", s.c_str());
				s = "";
			} else {
				s += c;

				if (s.size() == 40) {
					debugPrintf("%s\n", s.c_str());
					s = "";
				}
			}
		}
		debugPrintf("%s\n", s.c_str());

		f.close();
	}

	return true;
}

} // namespace MM1
} // namespace MM
