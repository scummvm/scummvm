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
 */

#ifndef NANCY_UTIL_H
#define NANCY_UTIL_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/serializer.h"

#include "engines/nancy/commontypes.h"

namespace Nancy {

struct CapturedPicture;

void readRect(Common::SeekableReadStream &stream, Common::Rect &inRect);
void readRect(Common::Serializer &stream, Common::Rect &inRect, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);
void readRectArray(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &inArray, uint num, uint totalNum = 0);
void readRectArray(Common::Serializer &stream, Common::Array<Common::Rect> &inArray, uint num, uint totalNum = 0, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);

void readRect16(Common::SeekableReadStream &stream, Common::Rect &inRect);
void readRect16(Common::Serializer &stream, Common::Rect &inRect, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);
void readRectArray16(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &inArray, uint num, uint totalNum = 0);
void readRectArray16(Common::Serializer &stream, Common::Array<Common::Rect> &inArray, uint num, uint totalNum = 0, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);

void readFilename(Common::SeekableReadStream &stream, Common::String &inString);
void readFilename(Common::Serializer &stream, Common::String &inString, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);
inline void readFilename(Common::SeekableReadStream &stream, Common::Path &inPath) {
	Common::String inString;
	readFilename(stream, inString);
	inPath = Common::Path(inString);
}
inline void readFilename(Common::Serializer &stream, Common::Path &inPath, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion) {
	Common::String inString;
	readFilename(stream, inString, minVersion, maxVersion);
	inPath = Common::Path(inString);
}
void readFilenameArray(Common::SeekableReadStream &stream, Common::Array<Common::String> &inArray, uint num);
void readFilenameArray(Common::Serializer &stream, Common::Array<Common::String> &inArray, uint num, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);
void readFilenameArray(Common::SeekableReadStream &stream, Common::Array<Common::Path> &inArray, uint num);
void readFilenameArray(Common::Serializer &stream, Common::Array<Common::Path> &inArray, uint num, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);

void assembleTextLine(char *rawCaption, Common::String &output, uint size);

// Resolves a subtitle/caption string that may be a key into an engine-data CVTX text
// table (AUTOTEXT by default, CONVO for some puzzles). Returns the table's entry for
// `keyOrText` when the table exists and contains that key, otherwise returns `fallback`.
Common::String resolveSubtitleText(const Common::String &keyOrText, const Common::String &fallback = Common::String(), const char *tableID = "AUTOTEXT");

// Reads a 30-byte, NUL-terminated subtitle string from `stream` and resolves it as an
// AUTOTEXT key, falling back to the literal text when the key is not present in the table.
Common::String readSubtitleText(Common::SeekableReadStream &stream);

// Shows `text` as a single line in the game textbox, replacing its current contents.
// Does nothing when `text` is empty or when the player has subtitles disabled. A
// non-negative `overrideFontID` selects a font other than the textbox default. When
// `forceRedraw` is true, the textbox is redrawn immediately instead of on the next
// render pass.
void showSubtitle(const Common::String &text, bool forceRedraw = false, int overrideFontID = -1);

// Maps a screen-space rect onto the live viewport's (scrolled) background, clipped
// to it. An empty rect means the whole visible viewport.
Common::Rect viewportScreenToBackground(const Common::Rect &screenRegion);

// Grabs a background-space region of the live viewport into `picture`, converted to
// BGRA32 so it can be redisplayed and saved. Used by both cameras.
bool captureViewportPicture(const Common::Rect &backgroundRegion, CapturedPicture &picture);

void readUIButton(Common::SeekableReadStream &stream, UIButtonRecord &dst);
void readUISlider(Common::SeekableReadStream &stream, UISliderRecord &dst);
void readUIPopupHeader(Common::SeekableReadStream &stream, UIPopupHeader &dst);
void readUIButtonSlot(Common::SeekableReadStream &stream, UIButtonSlot &dst);

// Abstract base class used for loading data that would take too much time in a single frame
class DeferredLoader {
public:
	DeferredLoader() {}
	virtual ~DeferredLoader() {}

	// Calls loadInner() one or many times, until its allotted time is done
	bool load(uint32 endTime);

protected:
	// Contains the actual loading logic, split up into tasks that are as small as possible
	virtual bool loadInner() = 0;
};

} // End of namespace Nancy

#endif // NANCY_UTIL_H
