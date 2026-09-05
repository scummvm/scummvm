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
#include "mads/core/concat.h"
#include "mads/core/config.h"
#include "mads/core/env.h"
#include "mads/forest/console.h"
#include "mads/forest/global.h"
#include "mads/forest/sound/digi.h"
#include "mads/forest/sound/midi.h"

namespace MADS {
namespace Forest {

Console::Console() : MADS::Console() {
	registerCmd("forestmusic", WRAP_METHOD(Console, cmdForestMusic));
	registerCmd("forestsample", WRAP_METHOD(Console, cmdForestSample));
	registerCmd("forestsounds", WRAP_METHOD(Console, cmdForestSounds));
}

bool Console::cmdForestMusic(int argc, const char **argv) {
	if (argc == 1 || (argc == 2 && !scumm_stricmp(argv[1], "list"))) {
		for (int id = 1; id <= 15; ++id)
			debugPrintf("%2d: %s\n", id, global_midi_name(id));
		return true;
	}

	if (argc != 2) {
		debugPrintf("Usage: %s <1-15|list|stop>\n", argv[0]);
		return true;
	}

	if (!scumm_stricmp(argv[1], "stop")) {
		midi_stop();
		debugPrintf("Stopped Forest music.\n");
		return true;
	}

	const int id = strToInt(argv[1]);
	const char *name = global_midi_name(id);
	if (!name) {
		debugPrintf("Music ID must be between 1 and 15.\n");
		return true;
	}
	if (!config_file.music_flag) {
		debugPrintf("Forest music is muted in the current game settings.\n");
		return true;
	}

	global_midi_play(id);
	debugPrintf("Started Forest music %d (%s).\n", id, name);
	return true;
}

bool Console::cmdForestSample(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Usage: %s <resource|stop> [slot 1-3|all]\n", argv[0]);
		return true;
	}

	if (!scumm_stricmp(argv[1], "stop")) {
		if (argc == 3 && !scumm_stricmp(argv[2], "all")) {
			for (int slot = 1; slot <= MAX_DIGI_CHANNELS; ++slot)
				digi_stop(slot);
			debugPrintf("Stopped all Forest digital channels.\n");
			return true;
		}

		const int slot = argc == 3 ? strToInt(argv[2]) : 1;
		if (slot < 1 || slot > MAX_DIGI_CHANNELS) {
			debugPrintf("Slot must be between 1 and %d.\n", MAX_DIGI_CHANNELS);
			return true;
		}

		digi_stop(slot);
		debugPrintf("Stopped Forest digital channel %d.\n", slot);
		return true;
	}

	const int slot = argc == 3 ? strToInt(argv[2]) : 1;
	if (slot < 1 || slot > MAX_DIGI_CHANNELS) {
		debugPrintf("Slot must be between 1 and %d.\n", MAX_DIGI_CHANNELS);
		return true;
	}

	Common::String name(argv[1]);
	if (name.hasSuffixIgnoreCase(".rac") || name.hasSuffixIgnoreCase(".raw"))
		name.erase(name.size() - 4);
	const Common::String racName = Common::String::format("*%s.rac", name.c_str());
	const Common::String rawName = Common::String::format("*%s.raw", name.c_str());
	if (!env_exist(racName.c_str()) && !env_exist(rawName.c_str())) {
		debugPrintf("Forest sample %s was not found.\n", name.c_str());
		return true;
	}

	digi_play(name.c_str(), slot);
	debugPrintf("Started Forest sample %s on channel %d.\n",
		name.c_str(), slot);
	return true;
}

bool Console::cmdForestSounds(int argc, const char **argv) {
	if (argc > 3) {
		debugPrintf("Usage: %s [speech|effects|all] [name filter]\n", argv[0]);
		return true;
	}

	char type = 0;
	if (argc >= 2) {
		if (!scumm_stricmp(argv[1], "speech"))
			type = 'S';
		else if (!scumm_stricmp(argv[1], "effects"))
			type = 'D';
		else if (scumm_stricmp(argv[1], "all")) {
			debugPrintf("Sound type must be speech, effects, or all.\n");
			return true;
		}
	}

	Common::String filter = argc == 3 ? argv[2] : "";
	filter.toUppercase();

	Common::File index;
	if (!index.open("SPEECH.IDX") && !index.open("SPEECH.HAG")) {
		debugPrintf("Could not open the Forest speech archive index.\n");
		return true;
	}

	char header[CONCAT_ID_LENGTH];
	if (index.read(header, sizeof(header)) != sizeof(header)) {
		debugPrintf("The Forest speech archive index is truncated.\n");
		return true;
	}

	for (int i = 0; i < CONCAT_ID_CHECK; ++i) {
		if (header[i] != CONCAT_ID_STRING[i]) {
			debugPrintf("The Forest speech archive index is invalid.\n");
			return true;
		}
	}

	const uint count = index.readUint16LE();
	if (count > CONCAT_MAX_FILES ||
			index.size() < CONCAT_ID_LENGTH + 2 + count * Concat::record_size()) {
		debugPrintf("The Forest speech archive directory is invalid.\n");
		return true;
	}

	uint matches = 0;
	for (uint i = 0; i < count; ++i) {
		Concat entry;
		entry.load(&index);

		Common::String name(entry.name);
		name.toUppercase();
		if (type && name[0] != type)
			continue;
		if (!filter.empty() && !name.contains(filter))
			continue;
		if (name.hasSuffixIgnoreCase(".rac") || name.hasSuffixIgnoreCase(".raw"))
			name.erase(name.size() - 4);

		if (matches && matches % 6 == 0)
			debugPrintf("\n");
		debugPrintf("%-12s", name.c_str());
		++matches;
	}

	if (matches)
		debugPrintf("\n%u matching resources.\n", matches);
	else
		debugPrintf("No matching resources.\n");
	return true;
}

} // namespace Forest
} // namespace MADS
