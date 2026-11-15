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

#ifndef BURIED_SUBTITLE_MANAGER_H
#define BURIED_SUBTITLE_MANAGER_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/path.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/ustr.h"

namespace Buried {

class SubtitleManager {
public:
	// A single timed subtitle entry or "card".
	struct SubtitleEntry {
		// The start time of this subtitle in milliseconds since the beginning of the file.
		uint32 startMs;
		// The end time of this subtitle in milliseconds since the beginning of the file.
		uint32 endMs;
		// The character who speaks the subtitled line of dialog (decoded from UTF-8).
		Common::U32String speaker;
		// The text spoken for the subtitled line of dialog (decoded from UTF-8).
		Common::U32String text;
	};

	SubtitleManager();
	~SubtitleManager() = default;

	// Returns true if subtitles are enabled in configuration and false otherwise.
	static bool areSubtitlesEnabled();

	// Returns true if a subtitle track exists for the specified media file and false otherwise.
	bool hasSubtitleTrack(const Common::Path &filePath) const;

	// Returns the SubtitleEntry for the specified media file and playback time, or nullptr if none matches.
	const SubtitleEntry *getSubtitleForTime(const Common::Path &filePath, uint32 currentMs) const;

private:
	// A subtitle track for an audio or video file, containing multiple entries or "cards".
	struct SubtitleTrack {
		// The subtitle entries, sorted by their start times.
		Common::Array<SubtitleEntry> entries;
	};

	// Loads binary subtitle package (buried_subtitles.dat) and returns whether loading succeeded.
	bool loadSubtitlesData();

	// Extracts a normalized media ID string from a file path by stripping the extension and converting to uppercase.
	// Example: "DATA/DA_V2_01.RAW" -> "DA_V2_01"
	static Common::String extractMediaIdFromPath(const Common::Path &filePath);

	Common::HashMap<Common::String, SubtitleTrack> _loadedTracks;
};

} // End of namespace Buried

#endif
