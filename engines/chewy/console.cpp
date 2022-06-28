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
#include "chewy/console.h"
#include "chewy/globals.h"
#include "chewy/chewy.h"
#include "chewy/sound.h"
#include "chewy/video/video_player.h"

namespace Chewy {

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
	registerCmd("room", WRAP_METHOD(Console, Cmd_Room));
	registerCmd("item", WRAP_METHOD(Console, Cmd_Item));
	registerCmd("play_sound", WRAP_METHOD(Console, Cmd_PlaySound));
	registerCmd("play_speech", WRAP_METHOD(Console, Cmd_PlaySpeech));
	registerCmd("play_music", WRAP_METHOD(Console, Cmd_PlayMusic));
	registerCmd("play_video", WRAP_METHOD(Console, Cmd_PlayVideo));
	registerCmd("walk", WRAP_METHOD(Console, Cmd_WalkAreas));
	registerCmd("text", WRAP_METHOD(Console, Cmd_Text));
}

Console::~Console() {
}

bool Console::Cmd_Room(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("%s <roomNum>\n", argv[0]);
		debugPrintf("Current room is %d\n", _G(gameState)._personRoomNr[P_CHEWY]);
		return true;
	} else {
		int roomNum = strToInt(argv[1]);
		exit_room(-1);
		_G(gameState)._personRoomNr[P_CHEWY] = roomNum;
		_G(room)->loadRoom(&_G(room_blk), roomNum, &_G(gameState));
		_G(fx_blend) = BLEND1;
		enter_room(-1);

		return false;
	}
}

bool Console::Cmd_Item(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("%s <itemNum>\n", argv[0]);
	} else {
		int itemNum = strToInt(argv[1]);
		invent_2_slot(itemNum);
		debugPrintf("Done.\n");
	}

	return true;
}

bool Console::Cmd_PlaySound(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: play_sound <number>\n");
		return true;
	}

	int resNum = atoi(argv[1]);
	g_engine->_sound->playSound(resNum);

	return true;
}

bool Console::Cmd_PlaySpeech(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: play_speech <number>\n");
		return true;
	}

	int resNum = atoi(argv[1]);
	g_engine->_sound->playSpeech(resNum, false);

	return true;
}

bool Console::Cmd_PlayMusic(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: play_music <number>\n");
		return true;
	}

	int resNum = atoi(argv[1]);
	g_engine->_sound->stopMusic();
	g_engine->_sound->playMusic(resNum);

	return true;
}

bool Console::Cmd_PlayVideo(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: play_video <number>\n");
		return true;
	}

	int resNum = atoi(argv[1]);
	g_engine->_video->playVideo(resNum);

	return false;
}

bool Console::Cmd_WalkAreas(int argc, const char **argv) {
	g_engine->_showWalkAreas = (argc == 2) && !strcmp(argv[1], "on");
	return false;
}

bool Console::Cmd_Text(int argc, const char **argv) {
	if (argc < 4) {
		debugPrintf("Usage: text <chunk> <entry> <type>\n");
		return true;
	}

	int chunk = atoi(argv[1]);
	int entry = atoi(argv[2]);
	int type = atoi(argv[3]);
	Common::StringArray text = _G(atds)->getTextArray(chunk, entry, type);
	for (uint i = 0; i < text.size(); i++) {
		debugPrintf("%d: %s\n", i, text[i].c_str());
	}
	return true;
}

} // namespace Chewy
