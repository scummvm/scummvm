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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Buried {

// Latest version of the subtitles .dat file.
static constexpr uint16 kSubtitleDataVersion = 1;

// Fixed width in bytes of the ASCII media ID stored in each Table of Contents entry.
static constexpr int kMediaIdLength = 16;

// A single Table of Contents (TOC) directory entry in buried_subtitles.dat,
// mapping a media ID to its track payload's byte offset in the file and card count.
struct TocEntry {
	Common::String mediaId;
	uint32 offset;
	uint16 cardCount;
};

SubtitleManager::SubtitleManager() {
	loadSubtitlesData();
}

bool SubtitleManager::areSubtitlesEnabled() {
	return ConfMan.getBool("subtitles");
}

// Loads binary subtitle package (buried_subtitles.dat) and returns whether the load was successful.
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
bool SubtitleManager::loadSubtitlesData() {
	Common::File file;
	if (!file.open(Common::Path("buried_subtitles.dat"))) {
		if (areSubtitlesEnabled()) {
			warning("[SubtitleManager] Could not open buried_subtitles.dat");
		} else {
			debug(1, "[SubtitleManager] Could not open buried_subtitles.dat");
		}
		return false;
	}

	// Look for "Buried Subtitle" signature to skip trying to parse an errant or malformed file.
	uint32 magic = file.readUint32BE();
	if (file.eos() || file.err() || magic != MKTAG('B', 'U', 'R', 'S')) {
		warning("[SubtitleManager] Invalid magic in buried_subtitles.dat: 0x%08X", magic);
		return false;
	}

	// Verify the binary format version matches the layout this reader expects.
	uint16 version = file.readUint16BE();
	if (file.eos() || file.err() || version != kSubtitleDataVersion) {
		warning("[SubtitleManager] Unsupported subtitle version in buried_subtitles.dat: %d", version);
		return false;
	}

	// Read the total number of subtitle tracks and ensure the file header was not truncated.
	uint16 numTracks = file.readUint16BE();
	if (file.eos() || file.err()) {
		warning("[SubtitleManager] Truncated header in buried_subtitles.dat");
		return false;
	}

	// Table of Contents (TOC) indexing each track's payload offset and card count in buried_subtitles.dat.
	Common::Array<TocEntry> toc;
	toc.reserve(numTracks);

	for (int i = 0; i < numTracks; ++i) {
		// Each TOC entry stores a 16-byte fixed-width ASCII media ID (null-padded only when shorter
		// than 16 characters), so allocate 17 bytes to guarantee space for a trailing '\0'.
		char mediaIdBuf[kMediaIdLength + 1];
		if (file.read(mediaIdBuf, kMediaIdLength) != kMediaIdLength) {
			warning("[SubtitleManager] Truncated TOC in buried_subtitles.dat");
			return false;
		}
		mediaIdBuf[kMediaIdLength] = '\0';

		TocEntry entry;
		entry.mediaId = mediaIdBuf;
		entry.offset = file.readUint32BE();
		entry.cardCount = file.readUint16BE();
		if (file.eos() || file.err()) {
			warning("[SubtitleManager] Truncated TOC entry in buried_subtitles.dat");
			return false;
		}
		toc.push_back(entry);
	}

	// Load track payload data
	for (const auto &tocEntry : toc) {
		if (!file.seek(tocEntry.offset, SEEK_SET)) {
			warning("[SubtitleManager] Failed to seek to track '%s' in buried_subtitles.dat", tocEntry.mediaId.c_str());
			_loadedTracks.clear();
			return false;
		}

		SubtitleTrack track;
		track.entries.reserve(tocEntry.cardCount);

		for (int cardIndex = 0; cardIndex < tocEntry.cardCount; ++cardIndex) {
			SubtitleEntry card;
			card.startMs = file.readUint32BE();
			card.endMs = file.readUint32BE();

			uint16 speakerNameLength = file.readUint16BE();
			if (speakerNameLength > 0) {
				card.speaker = Common::U32String(file.readString(0, speakerNameLength), Common::kUtf8);
			}

			uint16 textLength = file.readUint16BE();
			if (textLength > 0) {
				card.text = Common::U32String(file.readString(0, textLength), Common::kUtf8);
			}

			if (file.eos() || file.err()) {
				warning("[SubtitleManager] Truncated payload for track '%s' in buried_subtitles.dat", tocEntry.mediaId.c_str());
				_loadedTracks.clear();
				return false;
			}

			track.entries.push_back(card);
		}

		_loadedTracks[tocEntry.mediaId] = Common::move(track);
	}

	return true;
}

Common::String SubtitleManager::extractMediaIdFromPath(const Common::Path &filePath) {
	if (filePath.empty()) {
		return "";
	}
	Common::String mediaId = filePath.getLastComponent().toString();
	if (mediaId.contains(".")) {
		mediaId = mediaId.substr(0, mediaId.findLastOf('.'));
	}
	// Track keys in buried_subtitles.dat are uppercase and _loadedTracks is case-sensitive.
	mediaId.toUppercase();
	return mediaId;
}

bool SubtitleManager::hasSubtitleTrack(const Common::Path &filePath) const {
	Common::String mediaId = extractMediaIdFromPath(filePath);
	if (mediaId.empty()) {
		return false;
	}
	return _loadedTracks.contains(mediaId);
}

const SubtitleManager::SubtitleEntry *SubtitleManager::getSubtitleForTime(const Common::Path &filePath, uint32 currentMs) const {
	Common::String mediaId = extractMediaIdFromPath(filePath);
	if (mediaId.empty() || !_loadedTracks.contains(mediaId)) {
		return nullptr;
	}

	// Find the subtitle card whose [startMs, endMs] window contains currentMs.
	// Linear search is sufficient here -- tracks typically have only a few entries and none exceed 20.
	const SubtitleTrack &track = _loadedTracks[mediaId];
	for (const auto &entry : track.entries) {
		if (currentMs >= entry.startMs && currentMs <= entry.endMs) {
			return &entry;
		}
	}

	return nullptr;
}

} // End of namespace Buried
