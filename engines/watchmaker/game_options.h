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

#ifndef WATCHMAKER_GAME_OPTIONS_H
#define WATCHMAKER_GAME_OPTIONS_H
#include "common/stream.h"
#include "watchmaker/t3d.h"
#include "watchmaker/types.h"
#include "watchmaker/work_dirs.h"

namespace Watchmaker {

class GameOptions {
public:
	uint8 sound_on = 1;
	uint8 sound_volume = 100;
	uint8 music_on = 1;
	uint8 music_volume = 100;
	uint8 speech_on = 1;
	uint8 speech_volume = 100;
	uint8 subtitles_on = 1;
	bool bShowRoomDescriptions = true;
	bool bShowExtraLocalizationStrings = false;
	bool load(WorkDirs &workDirs) {
		warning("TODO: Game options");
#if 0
		Common::String path = workDirs._gameDir + workDirs._savesDir + "options.dat";

		auto stream = openFile(path);

		sound_on = stream->readByte();
		sound_volume = stream->readByte();
		music_on = stream->readByte();
		music_volume = stream->readByte();
		speech_on = stream->readByte();
		speech_volume = stream->readByte();
		subtitles_on = stream->readByte();
		bShowRoomDescriptions = stream->readByte();
		bShowExtraLocalizationStrings = stream->readByte();
		warning("Done loading options");
#endif
		return true;
	}

	bool save(WorkDirs &workDirs) {
		warning("TODO: Game options");
#if 0
		FILE *fp;
		char str[T3D_NAMELEN];

		sprintf(str, "%sOptions.dat", workDirs._savesDir.c_str());
		fp = fopen(str, "wb");

		if (!fp) {
			assert(false);
		}

		fwrite(&sound_on, sizeof(uint8), 1, fp);
		fwrite(&sound_volume, sizeof(uint8), 1, fp);
		fwrite(&music_on, sizeof(uint8), 1, fp);
		fwrite(&music_volume, sizeof(uint8), 1, fp);
		fwrite(&speech_on, sizeof(uint8), 1, fp);
		fwrite(&speech_volume, sizeof(uint8), 1, fp);
		fwrite(&subtitles_on, sizeof(uint8), 1, fp);
		fwrite(&bShowRoomDescriptions, sizeof(uint8), 1, fp);
		fwrite(&bShowExtraLocalizationStrings, sizeof(uint8), 1, fp);

		fclose(fp);
#endif
		return true;
	}
};



} // End of namespace Watchmaker

#endif // WATCHMAKER_GAME_OPTIONS_H
