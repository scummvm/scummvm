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

#include "common/array.h"
#include "mads/console.h"
#include "mads/core/env.h"
#include "mads/core/imath.h"
#include "mads/core/kernel.h"
#include "mads/core/matte.h"
#include "mads/core/mem.h"
#include "mads/core/text.h"
#include "mads/mads.h"

namespace MADS {

Console::Console() : GUI::Debugger() {
	registerCmd("depth", WRAP_METHOD(Console, cmdDepth));
	registerCmd("teleport", WRAP_METHOD(Console, cmdTeleport));
	registerCmd("walkable", WRAP_METHOD(Console, cmdWalkable));
	registerCmd("quotes", WRAP_METHOD(Console, cmdQuotes));
	registerCmd("playsound", WRAP_METHOD(Console, cmdPlaySound));
	registerCmd("soundcommand", WRAP_METHOD(Console, cmdSoundCommand));
	registerCmd("soundsection", WRAP_METHOD(Console, cmdSoundSection));
	registerCmd("soundstop", WRAP_METHOD(Console, cmdSoundStop));
	registerCmd("text", WRAP_METHOD(Console, cmdText));
}

int strToInt(const char *s) {
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

bool Console::cmdDepth(int argc, const char **argv) {
	const byte *srcP = scr_depth.data;
	byte *destP = scr_orig.data;

	for (int i = 0; i < scr_orig.x * scr_orig.y; i += 2, ++srcP) {
		*destP++ = *srcP >> 4;
		*destP++ = *srcP & 0xf;
	}

	matte_refresh_work();
	return false;
}

bool Console::cmdTeleport(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Current room is: %d\n", new_room);
		debugPrintf("Usage: %s <room number> [<previous room>]\n", argv[0]);
		return true;
	} else {
		if (argc >= 3)
			room_id = strToInt(argv[2]);
		new_room = strToInt(argv[1]);

		return false;
	}
}

bool Console::cmdQuotes(int argc, const char **argv) {
	bool showAll = (argc >= 2) && !strcmp(argv[1], "all");
	int quoteId = -1;
	if (showAll) {
		if (argc >= 3)
			quoteId = strToInt(argv[2]);
	} else if (argc >= 2) {
		quoteId = strToInt(argv[1]);
	}

	bool found = false;

	if (showAll) {
		// Stream the whole quotes.dat file directly rather than going through
		// quote_load(): its scratch buffer is sized for a handful of in-scene
		// quotes, not the whole game's quote table, and it needs the caller to
		// already know which Ids to ask for. A quote's Id is simply its
		// 1-based position in the file, so nothing needs to be kept around
		// afterwards - the stream is closed once we're done reading it.
		Common::SeekableReadStream *handle = env_open("*quotes.dat", "rb");
		if (!handle) {
			debugPrintf("Could not open quotes.dat\n");
			return true;
		}

		for (int id = 1; ; ++id) {
			Common::String quoteStr = handle->readString();
			if (handle->eos())
				break;

			if (quoteId == -1) {
				debugPrintf("%d: %s\n", id, quoteStr.c_str());
			} else if (id == quoteId) {
				debugPrintf("%d: %s\n", id, quoteStr.c_str());
				found = true;
				break;
			}
		}

		delete handle;
	} else {
		if (!kernel.quotes) {
			debugPrintf("No quotes are currently loaded.\n");
			return true;
		}

		// A quote list is a run of null-terminated strings, each immediately
		// followed by a 2-byte quote Id, with the run ending on an empty string.
		// See quote_string() in core/quote.cpp for the canonical traversal.
		char *marker, *search;
		for (marker = kernel.quotes; *marker; marker = search + 2) {
			for (search = marker; *search; search++)
				;
			search++;
			int id = *((uint16 *)search);

			if (quoteId == -1) {
				debugPrintf("%d: %s\n", id, marker);
			} else if (id == quoteId) {
				debugPrintf("%d: %s\n", id, marker);
				found = true;
				break;
			}
		}
	}

	if (quoteId != -1 && !found)
		debugPrintf("Quote %d not found.\n", quoteId);

	return true;
}

bool Console::cmdSoundCommand(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Usage: %s <command> [parameter]\n", argv[0]);
		return true;
	}
	if (!g_engine->_soundManager->isLoaded()) {
		debugPrintf("No section sound driver is loaded. Use soundsection first.\n");
		return true;
	}

	const int commandId = strToInt(argv[1]);
	const int parameter = argc == 3 ? strToInt(argv[2]) : 0;
	const int result = g_engine->_soundManager->command(commandId, parameter);
	debugPrintf("Sound command %d(%d) returned %d.\n",
		commandId, parameter, result);
	return true;
}

bool Console::cmdPlaySound(int argc, const char **argv) {
	int section;
	int commandId;
	int parameter = 0;
	char trailing;

	if (argc != 2 ||
			(sscanf(argv[1], "%d:%d:%d%c", &section, &commandId,
				&parameter, &trailing) != 3 &&
			sscanf(argv[1], "%d:%d%c", &section, &commandId,
				&trailing) != 2)) {
		debugPrintf("Usage: %s <section:command[:parameter]>\n", argv[0]);
		return true;
	}

	if (section < 1 || section > 9) {
		debugPrintf("Section must be between 1 and 9.\n");
		return true;
	}

	g_engine->_soundManager->init(section);
	if (!g_engine->_soundManager->isLoaded()) {
		debugPrintf("No sound driver is available for section %d.\n", section);
		return true;
	}

	const int result = g_engine->_soundManager->command(commandId, parameter);
	debugPrintf("Loaded section %d and issued sound command %d(%d), returned %d.\n",
		section, commandId, parameter, result);
	return true;
}

bool Console::cmdSoundSection(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <section 1-9>\n", argv[0]);
		return true;
	}

	const int section = strToInt(argv[1]);
	if (section < 1 || section > 9) {
		debugPrintf("Section must be between 1 and 9.\n");
		return true;
	}

	g_engine->_soundManager->init(section);
	debugPrintf(
		g_engine->_soundManager->isLoaded()
			? "Loaded section %d for the configured audio device.\n"
			: "No sound driver is available for section %d.\n",
		section);
	return true;
}

bool Console::cmdSoundStop(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	g_engine->_soundManager->stop();
	debugPrintf("Stopped the current section sound driver.\n");
	return true;
}

static bool textBufferContains(const char *haystack, uint16 haystackLen, const char *needle) {
	size_t needleLen = strlen(needle);
	if (needleLen == 0 || haystackLen < needleLen)
		return false;

	for (uint16 i = 0; i + needleLen <= haystackLen; ++i) {
		if (!scumm_strnicmp(haystack + i, needle, needleLen))
			return true;
	}

	return false;
}

bool Console::cmdText(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <search string>\n", argv[0]);
		return true;
	}

	Common::String search(argv[1]);
	for (int i = 2; i < argc; ++i) {
		search += ' ';
		search += argv[i];
	}

	// Enumerate every Id in the compiled text file's directory table so each
	// one can be individually decompressed via text_load() and searched.
	// See text_load() in core/text.cpp for the canonical reader of this table.
	char temp_buf[80];
	Common::strcpy_s(temp_buf, text_filename);
	Common::strcat_s(temp_buf, TEXT_COMPILED);

	Common::SeekableReadStream *handle = env_open(temp_buf, "rb");
	if (!handle) {
		debugPrintf("Could not open %s\n", temp_buf);
		return true;
	}

	word num_entries = handle->readUint16LE();
	Common::Array<int32> ids;
	for (int count = 0; count < num_entries; ++count) {
		TextDirectory dir;
		dir.load(handle);
		ids.push_back(dir.id);
	}

	delete handle;

	bool found = false;
	for (uint i = 0; i < ids.size(); ++i) {
		TextPtr text = text_load(ids[i]);
		if (!text)
			continue;

		if (textBufferContains(text->text, text->length, search.c_str())) {
			found = true;

			Common::String display(text->text, text->length);
			for (uint j = 0; j < display.size(); ++j) {
				if (display[j] == '\0')
					display[j] = '\n';
			}

			debugPrintf("--- %d ---\n%s\n\n", ids[i], display.c_str());
		}

		mem_free(text);
	}

	if (!found)
		debugPrintf("No text entries contain \"%s\"\n", search.c_str());

	return true;
}

bool Console::cmdWalkable(int argc, const char **argv) {
	const byte *srcP = scr_walk.data - 1;
	byte *destP = scr_orig.data;
	assert((scr_walk.x * 8) == scr_orig.x);

	// Draw the walkable areas
	for (int i = 0; i < scr_orig.x * scr_orig.y; ++i, ++destP) {
		if ((i % 8) == 0)
			++srcP;
		if (!(*srcP & (1 << (7 - (i % 8)))))
			*destP = 10;
	}

	// Draw cross-hairs at the locations of the rails within the room
	for (int i = 0; i < room->num_rails; ++i) {
		const Rail &r = room->rail[i];

		buffer_hline(scr_orig, r.x - 2, r.x + 2, r.y, 0);
		buffer_vline(scr_orig, r.x, r.y - 2, r.y + 2, 0);
	}

	matte_refresh_work();
	return false;
}

} // namespace MADS
