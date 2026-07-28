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
#include "common/hashmap.h"
#include "common/str.h"
#include "common/rect.h"

namespace Graphics {
class Font;
struct Surface;
}

namespace Buried {

class BuriedEngine;
class Window;

// Subtitle Box Layout Constants
// Controls position, width, and height of the subtitle overlay box.
// The subtitle box is designed to appear below the jumpsuit's viewport, spanning from one end of the window to the other
// and including the viewport bezels.
//
// This design deliberately obstructs the display of the current date while subtitles are showing. This was deemed preferable
// to showing the subtitles on top of the viewport and obstructing the already-small jumpsuit viewport.
//
// The X coordinate of the subtitle box's top-left edge.
static constexpr int kSubtitleBoxX = 46;
// The Y coordinate of the subtitle box's top-left edge.
static constexpr int kSubtitleBoxY = 317;
// The width of the subtitle box.
static constexpr int kSubtitleBoxWidth = 462;
// The opacity of the subtitle box. 0f would be fully transparent. 1f would be fully opaque.
static constexpr float kSubtitleBoxOpacity = 0.75f;
// Horizontal padding of the interior of the subtitle box.
static constexpr int kSubtitlePadX = 8;
// Additional vertical padding of the interior of the subtitle box. This is added additionally to space reserved
// for font ascenders, descenders, and the angled chamfer section at the bottom of the subtitle box.
static constexpr int kSubtitlePadY = 2;
// Maximum number of wrapped dialogue lines displayed in the subtitle box.
// Drives total text height in getBoxHeight() and loop iterations in wrapText() and renderSubtitle().
static constexpr int kMaxSubtitleLines = 2;

// Height in pixels of the angled chamfer bevel at the bottom of the subtitle box.
// The bevel is rendered at a 2:1 horizontal-to-vertical slope ratio.
// This constant controls the vertical span of the cutouts; the maximum horizontal inset at the bottom-most row is
// derived as (kSubtitleChamferHeight * 2) pixels.
static constexpr int kSubtitleChamferHeight = 6;

// Border stroke thickness in pixels for the top, bottom, and side frame edges.
static constexpr int kSubtitleBorderWidth = 1;

// Vertical spacing in pixels between consecutive lines of wrapped text.
static constexpr int kSubtitleInterlineSpacing = 2;

struct ColorRGB {
	// The red component of a color in [0,255].
	byte r;
	// The green component of a color in [0,255].
	byte g;
	// The blue component of a color in [0,255].
	byte b;
};

class SubtitleManager {
public:
	SubtitleManager(BuriedEngine *vm);
	~SubtitleManager();

	// Returns true if subtitles are enabled and false otherwise.
	static bool areSubtitlesEnabled();

	// Marks the subtitle region of the target window dirty without redrawing it, allowing it to be re-rendered as part
	// of the next frame.
	void markSubtitlesDirty(Window *targetWindow = nullptr);

	// Forces the subtitle region of the target window to re-draw immediately. This is necessary for synchronous flows
	// where UI control is "yielded" to play a synchronous effect e.g. some of Arthur's dialog at the Farnstein lab.
	void forceRepaintSubtitles(Window *targetWindow = nullptr);

	static constexpr uint16 kSubtitleDataVersion = 1;

	// Loads subtitles track and timing file and returns whether subtitles loaded successfully.
	bool loadSubtitlesData();

	// Returns true if a subtitle track exists for the specified ID and false otherwise.
	bool hasSubtitleTrack(const Common::String &mediaId) const;

	// Renders subtitles for whatever audio channel is currently playing subtitled media onto the destination surface.
	bool renderSubtitlesForActiveAudio(Graphics::Surface *destSurface);

	// Renders subtitles for the specified video window at the specified playback time onto the destination surface.
	bool renderSubtitleForVideo(Graphics::Surface *destSurface, const Window *videoWindow, const Common::String &mediaId, uint32 currentMs, const Common::Rect &mediaRect);

	// Returns true if any audio channel is currently playing subtitled media and false otherwise.
	bool isSubtitledAudioPlaying() const;

private:
	// Renders subtitles for the specified media file at the specified playback time onto the destination surface at the
	// default position. The default position is suitable for subtitles shown while main HUD (jumpsuit helmet interior) is shown.
	bool renderSubtitleForMedia(Graphics::Surface *destSurface, const Common::String &mediaId, uint32 currentMs);

	// Renders subtitles for the specified media file at the specified playback time onto the destination surface at the
	// specified position.
	bool renderSubtitleForMedia(Graphics::Surface *destSurface, const Common::Rect &boxRect, const Common::String &mediaId, uint32 currentMs);

	// Returns a rectangle to show subtitles beneath media showing within the specified rectangle. The box will match the width of the
	// rectangle and be tall enough to render two lines of subtitle text.
	Common::Rect calculateBoxBoundsForVideo(const Window *videoWindow, const Common::Rect &mediaRect);

	// A single subtitle entry or "card".
	struct SubtitleEntry {
		// The start time of this subtitle in milliseconds since the beginning of the file.
		uint32 startMs;
		// The end time of this subtitle in milliseconds since the beginning of the file.
		uint32 endMs;
		// The character who speaks the subtitled line of dialog.
		Common::String speaker;
		// The text spoken for the subtitled line of dialog.
		Common::String text;
	};

	// A subtitle track for an audio or video file, containing multiple entries or "cards".
	struct SubtitleTrack {
		// The identifier for the audio or video file to which this track is associated.
		Common::String mediaId;
		// The subtitle entries, sorted by their start times.
		Common::Array<SubtitleEntry> entries;
	};

	BuriedEngine *_vm;
	Common::HashMap<Common::String, SubtitleTrack> _loadedTracks;
	Graphics::Font *_font;
	Graphics::Font *_fontBold;
	int _fontSize;

	// Returns the SubtitleEntry for the specified media file and playback time or null if no entry exists.
	const SubtitleEntry *getSubtitleForTime(const Common::String &mediaId, uint32 currentMs);

	// Renders the specified SubtitleEntry onto the destination surface at the specified position.
	void renderSubtitle(Graphics::Surface *destSurface, const Common::Rect &boxRect, const SubtitleEntry &entry);

	// Returns the subtitle font height in pixels.
	int getFontHeight() const;

	// Returns the height of the subtitle box, in pixels.
	int getBoxHeight() const;

	// Returns the rectangle for the default subtitle position. The default position is suitable for subtitles shown while main HUD (jumpsuit
	// helmet interior) is shown.
	Common::Rect getDefaultBoxBounds();

	void updateFont();

	uint32 getColor(const ColorRGB &color) const;
};
} // End of namespace Buried

#endif
