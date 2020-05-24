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

#include "sherlock/debugger.h"
#include "sherlock/sherlock.h"
#include "sherlock/music.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/scalpel/scalpel_debugger.h"
#include "sherlock/tattoo/tattoo_debugger.h"
#include "common/str-array.h"

namespace Sherlock {

Debugger *Debugger::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_RoseTattoo)
		return new Tattoo::TattooDebugger(vm);
	else
		return new Scalpel::ScalpelDebugger(vm);
}

Debugger::Debugger(SherlockEngine *vm) : GUI::Debugger(), _vm(vm) {
	_showAllLocations = LOC_DISABLED;

	registerCmd("continue",	     WRAP_METHOD(Debugger, cmdExit));
	registerCmd("scene",         WRAP_METHOD(Debugger, cmdScene));
	registerCmd("song",          WRAP_METHOD(Debugger, cmdSong));
	registerCmd("songs",         WRAP_METHOD(Debugger, cmdListSongs));
	registerCmd("listfiles",     WRAP_METHOD(Debugger, cmdListFiles));
	registerCmd("dumpfile",      WRAP_METHOD(Debugger, cmdDumpFile));
	registerCmd("locations",     WRAP_METHOD(Debugger, cmdLocations));
	registerCmd("flag",          WRAP_METHOD(Debugger, cmdFlag));
}

void Debugger::postEnter() {
	if (!_3doPlayMovieFile.empty()) {
		static_cast<Scalpel::ScalpelEngine *>(_vm)->play3doMovie(_3doPlayMovieFile, Common::Point(0, 0));

		_3doPlayMovieFile.clear();
	}

	GUI::Debugger::postEnter();
}

int Debugger::strToInt(const char *s) {
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

bool Debugger::cmdScene(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: scene <room>\n");
		return true;
	} else {
		_vm->_scene->_goToScene = strToInt(argv[1]);
		return false;
	}
}

bool Debugger::cmdSong(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: song <name>\n");
		return true;
	}

	Common::StringArray songs;
	_vm->_music->getSongNames(songs);

	for (uint i = 0; i < songs.size(); i++) {
		if (songs[i].equalsIgnoreCase(argv[1])) {
			_vm->_music->loadSong(songs[i]);
			return false;
		}
	}

	debugPrintf("Invalid song. Use the 'songs' command to see which ones are available.\n");
	return true;
}

bool Debugger::cmdListSongs(int argc, const char **argv) {
	Common::StringArray songs;
	_vm->_music->getSongNames(songs);
	debugPrintColumns(songs);
	return true;
}

bool Debugger::cmdListFiles(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: listfiles <resource file>\n");
		return true;
	}
	Common::StringArray files;
	_vm->_res->getResourceNames(Common::String(argv[1]), files);
	debugPrintColumns(files);
	return true;
}

bool Debugger::cmdDumpFile(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: dumpfile <resource name>\n");
		return true;
	}

	Common::SeekableReadStream *s = _vm->_res->load(argv[1]);
	if (!s) {
		debugPrintf("Invalid resource.\n");
		return true;
	}

	byte *buffer = new byte[s->size()];
	s->read(buffer, s->size());

	Common::DumpFile dumpFile;
	dumpFile.open(argv[1]);

	dumpFile.write(buffer, s->size());
	dumpFile.flush();
	dumpFile.close();

	delete[] buffer;

	debugPrintf("Resource %s has been dumped to disk.\n", argv[1]);

	return true;
}

bool Debugger::cmdLocations(int argc, const char **argv) {
	_showAllLocations = LOC_REFRESH;

	debugPrintf("Now showing all map locations\n");
	return false;
}

bool Debugger::cmdFlag(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Format: flag <number> [set | clear | toggle]\n");
	} else {
		int flagNum = strToInt(argv[1]);

		if (argc == 2) {
			debugPrintf("Flag %d is %s\n", flagNum, _vm->_flags[flagNum] ? "Set" : "Clear");
		} else {
			if (!strcmp(argv[2], "set"))
				_vm->_flags[flagNum] = true;
			else if (!strcmp(argv[2], "clear"))
				_vm->_flags[flagNum] = false;
			else if (!strcmp(argv[2], "toggle"))
				_vm->_flags[flagNum] = !_vm->_flags[flagNum];

			debugPrintf("Flag %d is now %s\n", flagNum, _vm->_flags[flagNum] ? "Set" : "Clear");
		}
	}

	return true;
}

} // End of namespace Sherlock
