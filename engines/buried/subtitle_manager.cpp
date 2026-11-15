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

#include "buried/subtitle_manager.h"

#include "common/array.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Buried {

namespace {

// File name of the packed binary subtitle data archive.
constexpr const char *kSubtitlesDataFileName = "buried_subtitles.dat";

// Latest version of the subtitles .dat file.
constexpr uint16 kSubtitleDataVersion = 1;

// Fixed width in bytes of the ASCII media ID stored in each Table of Contents entry.
constexpr int kMediaIdLength = 16;

// A single Table of Contents (TOC) directory entry in the subtitles file,
// mapping a media ID to its track payload's byte offset in the file and card count.
struct TocEntry {
	Common::String mediaId;
	uint32 offset;
	uint16 cardCount;
};

// A single timed subtitle card belonging to a track.
struct SubtitleCard {
	uint32 startMs;
	uint32 endMs;
	SubtitleManager::Subtitle subtitle;
};

// A subtitle track for an audio or video file, containing entries sorted by start time.
struct SubtitleTrack {
	Common::Array<SubtitleCard> entries;
};

using SubtitleTrackMap = Common::HashMap<Common::String, SubtitleTrack, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>;

// Extracts a media ID stem from a file path by taking the base filename and stripping its extension.
// Example: "DATA/DA_V2_01.RAW" -> "DA_V2_01"
Common::String extractMediaIdFromPath(const Common::Path &filePath) {
	// Strip directory components while keeping the extension (e.g. "DATA/DA_V2_01.RAW" -> "DA_V2_01.RAW").
	Common::String mediaId = filePath.baseName();
	// Strip the file extension (e.g. "DA_V2_01.RAW" -> "DA_V2_01").
	size_t dotPos = mediaId.findLastOf('.');
	if (dotPos != Common::String::npos) {
		mediaId = mediaId.substr(0, dotPos);
	}
	return mediaId;
}

// Reads numTracks Table of Contents entries from the subtitles file, or returns an empty array on error.
Common::Array<TocEntry> readToc(Common::File &file, uint16 numTracks) {
	Common::Array<TocEntry> toc;
	toc.reserve(numTracks);

	for (int i = 0; i < numTracks; i++) {
		// Each TOC entry stores a 16-byte fixed-width ASCII media ID (null-padded only when shorter
		// than 16 characters), so allocate 17 bytes to guarantee space for a trailing '\0'.
		char mediaIdBuf[kMediaIdLength + 1];
		if (file.read(mediaIdBuf, kMediaIdLength) != kMediaIdLength) {
			warning("[SubtitleManager] Truncated TOC in %s", kSubtitlesDataFileName);
			return Common::Array<TocEntry>();
		}
		mediaIdBuf[kMediaIdLength] = '\0';

		TocEntry entry;
		entry.mediaId = mediaIdBuf;
		entry.offset = file.readUint32BE();
		entry.cardCount = file.readUint16BE();
		if (file.eos() || file.err()) {
			warning("[SubtitleManager] Truncated TOC entry in %s", kSubtitlesDataFileName);
			return Common::Array<TocEntry>();
		}
		toc.push_back(entry);
	}

	return toc;
}

// Reads all subtitle tracks indexed by toc from the subtitles file, or returns an empty map on error.
SubtitleTrackMap readTracks(Common::File &file, const Common::Array<TocEntry> &toc) {
	SubtitleTrackMap loadedTracks;

	for (const auto &tocEntry : toc) {
		if (!file.seek(tocEntry.offset, SEEK_SET)) {
			warning("[SubtitleManager] Failed to seek to track '%s' in %s", tocEntry.mediaId.c_str(), kSubtitlesDataFileName);
			return SubtitleTrackMap();
		}

		SubtitleTrack track;
		track.entries.reserve(tocEntry.cardCount);

		for (int cardIndex = 0; cardIndex < tocEntry.cardCount; cardIndex++) {
			SubtitleCard card;
			card.startMs = file.readUint32BE();
			card.endMs = file.readUint32BE();

			uint16 speakerNameLength = file.readUint16BE();
			if (speakerNameLength > 0) {
				card.subtitle.speaker = Common::U32String(file.readString(0, speakerNameLength), Common::kUtf8);
			}

			uint16 textLength = file.readUint16BE();
			if (textLength > 0) {
				card.subtitle.text = Common::U32String(file.readString(0, textLength), Common::kUtf8);
			}

			if (file.eos() || file.err()) {
				warning("[SubtitleManager] Truncated payload for track '%s' in %s", tocEntry.mediaId.c_str(), kSubtitlesDataFileName);
				return SubtitleTrackMap();
			}

			track.entries.push_back(card);
		}

		loadedTracks[tocEntry.mediaId] = Common::move(track);
	}

	return loadedTracks;
}

// Loads and returns the subtitle tracks from the binary subtitle package, or an empty map on failure.
//
// File Format Specification (Big-Endian):
// 1. Magic Signature (4 bytes): 'BURS' (0x42555253)
// 2. Format Version (uint16): 1
// 3. Track Count (uint16): N tracks
// 4. Table of Contents (TOC) (N x 22 bytes):
//    - Media ID (16 bytes): Fixed ASCII string (null-padded)
//    - Payload Offset (uint32): Byte offset to track data
//    - Card Count (uint16): M subtitle cards in track
// 5. Track Payloads:
//    - Start Time (uint32): Start time in milliseconds
//    - End Time (uint32): End time in milliseconds
//    - Speaker Length (uint16): Speaker string length
//    - Speaker String: UTF-8 speaker name
//    - Text Length (uint16): Dialogue string length
//    - Text String: UTF-8 dialogue text
SubtitleTrackMap loadSubtitlesData() {
	Common::File file;
	if (!file.open(Common::Path(kSubtitlesDataFileName))) {
		if (SubtitleManager::areSubtitlesEnabled()) {
			warning("[SubtitleManager] Could not open %s", kSubtitlesDataFileName);
		} else {
			debug(1, "[SubtitleManager] Could not open %s", kSubtitlesDataFileName);
		}
		return SubtitleTrackMap();
	}

	// Look for "Buried Subtitle" signature to skip trying to parse an errant or malformed file.
	uint32 magic = file.readUint32BE();
	if (file.eos() || file.err() || magic != MKTAG('B', 'U', 'R', 'S')) {
		warning("[SubtitleManager] Invalid magic in %s: 0x%08X", kSubtitlesDataFileName, magic);
		return SubtitleTrackMap();
	}

	// Verify the binary format version matches the layout this reader expects.
	uint16 version = file.readUint16BE();
	if (file.eos() || file.err() || version != kSubtitleDataVersion) {
		warning("[SubtitleManager] Unsupported subtitle version in %s: %d", kSubtitlesDataFileName, version);
		return SubtitleTrackMap();
	}

	// Read the total number of subtitle tracks and ensure the file header was not truncated.
	uint16 numTracks = file.readUint16BE();
	if (file.eos() || file.err()) {
		warning("[SubtitleManager] Truncated header in %s", kSubtitlesDataFileName);
		return SubtitleTrackMap();
	}

	if (numTracks == 0) {
		return SubtitleTrackMap();
	}

	// Table of Contents (TOC) indexing each track's payload offset and card count in the subtitles file.
	Common::Array<TocEntry> toc = readToc(file, numTracks);
	if (toc.empty()) {
		return SubtitleTrackMap();
	}

	return readTracks(file, toc);
}

} // End of anonymous namespace

struct SubtitleManager::TrackMap {
	const SubtitleTrackMap tracks;

	TrackMap() : tracks(loadSubtitlesData()) {}
};

SubtitleManager::SubtitleManager() : _trackMap(new TrackMap()) {
}

SubtitleManager::~SubtitleManager() = default;

bool SubtitleManager::areSubtitlesEnabled() {
	return ConfMan.getBool("subtitles");
}

bool SubtitleManager::hasSubtitleTrack(const Common::Path &filePath) const {
	Common::String mediaId = extractMediaIdFromPath(filePath);
	if (mediaId.empty()) {
		return false;
	}
	return _trackMap->tracks.contains(mediaId);
}

const SubtitleManager::Subtitle *SubtitleManager::getSubtitleForTime(const Common::Path &filePath, uint32 currentMs) const {
	Common::String mediaId = extractMediaIdFromPath(filePath);
	if (mediaId.empty() || !_trackMap->tracks.contains(mediaId)) {
		return nullptr;
	}

	// Find the subtitle card whose [startMs, endMs] window contains currentMs.
	// Linear search is sufficient here -- tracks typically have only a few entries and none exceed 20.
	const SubtitleTrack &track = _trackMap->tracks[mediaId];
	for (const auto &card : track.entries) {
		if (currentMs >= card.startMs && currentMs <= card.endMs) {
			return &card.subtitle;
		}
	}

	return nullptr;
}

} // End of namespace Buried
