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

#include "common/savefile.h"
#include "common/system.h"
#include "got/console.h"
#include "got/events.h"
#include "got/vars.h"
#include "got/got.h"

namespace Got {

static const char *SAVE_FILENAMES[3] = {
	"savegam1.got", "savegam1.gt2", "savegam1.gt3"
};

Console::Console() : GUI::Debugger() {
	registerCmd("view",   WRAP_METHOD(Console, cmdView));
	registerCmd("sound", WRAP_METHOD(Console, cmdSound));
	registerCmd("music", WRAP_METHOD(Console, cmdMusic));
	registerCmd("load", WRAP_METHOD(Console, cmdLoad));
	registerCmd("save", WRAP_METHOD(Console, cmdSave));
	registerCmd("magic", WRAP_METHOD(Console, cmdMagic));
}

Console::~Console() {
}

bool Console::cmdView(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("view <view name>\n");
		return true;
	} else {
		g_events->replaceView(argv[1], true);
		return false;
	}
}

bool Console::cmdSound(int argc, const char **argv) {
	if (argc == 2)
		_G(sound).play_sound(atoi(argv[1]), true);
	return false;
}

bool Console::cmdMusic(int argc, const char **argv) {
	if (argc == 2)
		_G(sound).music_play(atoi(argv[1]), true);
	return false;
}

bool Console::cmdLoad(int argc, const char **argv) {
	Common::String filename = SAVE_FILENAMES[_G(area) - 1];
	if (argc == 2)
		filename = argv[1];

	Common::InSaveFile *f;
	if ((f = g_system->getSavefileManager()->openForLoading(filename)) == nullptr) {
		debugPrintf("Could not open %s\n", filename.c_str());
		return true;
	} else {
		f->skip(32); // Skip the 32 bytes title

		Common::Serializer s(f, nullptr);
		g_engine->syncGame(s);
		delete f;
		return false;
	}
}

bool Console::cmdSave(int argc, const char **argv) {
	Common::String filename = SAVE_FILENAMES[_G(area) - 1];
	if (argc == 2)
		filename = argv[1];

	Common::OutSaveFile *f;
	if ((f = g_system->getSavefileManager()->openForSaving(filename, false)) == nullptr) {
		debugPrintf("Could not create %s\n", filename.c_str());

	} else {
		// Write out dummy person name
		char title[32];
		Common::fill(title, title + 32, 0);
		Common::strcpy_s(title, "ScummVM");

		f->write(title, 32);

		Common::Serializer s(nullptr, f);
		g_engine->syncGame(s);

		delete f;

		debugPrintf("Created %s\n", filename.c_str());
	}

	return true;
}

bool Console::cmdMagic(int argc, const char **argv) {
	_G(thor_info).magic = (argc == 2) ? CLIP(atoi(argv[1]), 0, 150): 150;
	
	return false;
}

} // namespace Got
