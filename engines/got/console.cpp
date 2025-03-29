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

#include "got/console.h"
#include "common/savefile.h"
#include "common/system.h"
#include "got/events.h"
#include "got/got.h"
#include "got/vars.h"

namespace Got {

static const char *SAVE_FILENAMES[3] = {
	"savegam1.got", "savegam1.gt2", "savegam1.gt3"};

Console::Console() : GUI::Debugger() {
	registerCmd("view", WRAP_METHOD(Console, cmdView));
	registerCmd("sound", WRAP_METHOD(Console, cmdSound));
	registerCmd("music", WRAP_METHOD(Console, cmdMusic));
	registerCmd("load", WRAP_METHOD(Console, cmdLoad));
	registerCmd("save", WRAP_METHOD(Console, cmdSave));
	registerCmd("magic", WRAP_METHOD(Console, cmdMagic));
	registerCmd("freeze", WRAP_METHOD(Console, cmdFreeze));
	registerCmd("level", WRAP_METHOD(Console, cmdLevel));
	registerCmd("flying", WRAP_METHOD(Console, cmdFlying));
	registerCmd("flag", WRAP_METHOD(Console, cmdFlag));
}

Console::~Console() {
}

bool Console::cmdView(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("view <view name>\n");
		return true;
	}

	g_events->replaceView(argv[1], true);
	return false;
}

bool Console::cmdSound(int argc, const char **argv) {
	if (argc == 2)
		_G(sound).playSound(atoi(argv[1]), true);
	return false;
}

bool Console::cmdMusic(int argc, const char **argv) {
	if (argc == 2)
		_G(sound).musicPlay(atoi(argv[1]), true);
	return false;
}

bool Console::cmdLoad(int argc, const char **argv) {
	Common::String filename = SAVE_FILENAMES[_G(area) - 1];
	if (argc == 2)
		filename = argv[1];

	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);
	if (f == nullptr) {
		debugPrintf("Could not open %s\n", filename.c_str());
		return true;
	}

	Common::Serializer s(f, nullptr);
	g_engine->syncGame(s);
	delete f;
	return false;
}

bool Console::cmdSave(int argc, const char **argv) {
	Common::String filename = SAVE_FILENAMES[_G(area) - 1];
	if (argc == 2)
		filename = argv[1];

	Common::OutSaveFile *f = g_system->getSavefileManager()->openForSaving(filename, false);
	if (f == nullptr) {
		debugPrintf("Could not create %s\n", filename.c_str());

	} else {
		Common::Serializer s(nullptr, f);
		g_engine->syncGame(s);

		delete f;

		debugPrintf("Created %s\n", filename.c_str());
	}

	return true;
}

bool Console::cmdMagic(int argc, const char **argv) {
	_G(thorInfo)._magic = (argc == 2) ? CLIP(atoi(argv[1]), 0, 150) : 150;

	return false;
}

bool Console::cmdFreeze(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("freeze ['health', 'magic', 'gems']\n");
	} else if (!strcmp(argv[1], "health")) {
		_G(cheats)._freezeHealth = !_G(cheats)._freezeHealth;
		debugPrintf("Health is %s\n", _G(cheats)._freezeHealth ? "frozen" : "unfrozen");
	} else if (!strcmp(argv[1], "magic")) {
		_G(cheats)._freezeMagic = !_G(cheats)._freezeMagic;
		debugPrintf("Magic is %s\n", _G(cheats)._freezeMagic ? "frozen" : "unfrozen");
	} else if (!strcmp(argv[1], "jewels")) {
		_G(cheats)._freezeJewels = !_G(cheats)._freezeJewels;
		debugPrintf("Jewels are %s\n", _G(cheats)._freezeJewels ? "frozen" : "unfrozen");
	} else if (!strcmp(argv[1], "all")) {
		_G(cheats)._freezeHealth = _G(cheats)._freezeMagic =
			_G(cheats)._freezeJewels = !_G(cheats)._freezeHealth;
		debugPrintf("All are %s\n", _G(cheats)._freezeHealth ? "frozen" : "unfrozen");
	}

	return true;
}

bool Console::cmdLevel(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Current level = %d\n", _G(currentLevel));
		return true;
	}

	_G(newLevel) = atoi(argv[1]);
	_G(warpFlag) = true;
	return false;
}

bool Console::cmdFlying(int argc, const char **argv) {
	_G(thor)->_flying = !_G(thor)->_flying;
	debugPrintf("Flying is %s\n", _G(thor)->_flying ? "on" : "off");
	return true;
}

bool Console::cmdFlag(int argc, const char **argv) {
	if (argc == 1) {
		for (int start = 0; start < 64; start += 10) {
			Common::String line;
			for (int i = start; i < (start == 60 ? 64 : (start + 10)); ++i) {
				if ((i % 5) == 0)
					line += " ";
				line += Common::String::format("%c",
					_G(setup)._flags[i] ? 'T' : 'F');
			}

			debugPrintf("%s\n", line.c_str());
		}
	} else {
		int flag = atoi(argv[1]);

		if (flag < 1 || flag > 64) {
			debugPrintf("Flags must be from 1 to 64\n");
		} else if (argc == 2) {
			debugPrintf("Flag #%d is %s\n", flag,
				_G(setup)._flags[flag - 1] ? "true" : "false");
		} else {
			bool val = tolower(argv[2][0]) == 't';
			_G(setup)._flags[flag - 1] = val;

			debugPrintf("Flag #%d is %s\n", flag,
				val ? "true" : "false");
		}
	}

	return true;
}

} // namespace Got
