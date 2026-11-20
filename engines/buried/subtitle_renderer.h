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

#ifndef BURIED_SUBTITLE_RENDERER_H
#define BURIED_SUBTITLE_RENDERER_H

#include "buried/subtitle_manager.h"

#include "common/path.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/scummsys.h"

namespace Graphics {
class Font;
struct Surface;
}

namespace Buried {

class BuriedEngine;
struct ColorRGB;
class Window;

// Font size preset values (pixel height)
enum FontSize {
	kFontSizeSmall = 12,
	kFontSizeMedium = 14,
	kFontSizeLarge = 18,
	kFontSizeXLarge = 22
};

static constexpr int kDefaultSubtitleFontSize = kFontSizeMedium;

// Custom key for the user's font size preference from the Game Options menu
static constexpr auto kSubtitleFontSizeKey = "subtitle_font_size";

class SubtitleRenderer {
public:
	SubtitleRenderer(BuriedEngine *vm, const SubtitleManager *subtitles);
	~SubtitleRenderer();

	// Reloads the regular and bold subtitle fonts if the configured font size has changed,
	// marking targetWindow and the subtitle box dirty when a change occurs.
	void updateFont(Window *targetWindow);

	// Marks the target window and subtitle box region dirty so subtitles are redrawn or cleared on the next frame.
	void markDirty(Window *targetWindow);

	// Renders subtitles for the specified video window at the specified playback time onto the destination surface.
	bool renderForVideo(Graphics::Surface *destSurface, const Window *videoWindow, const Common::Path &filePath, uint32 currentMs, const Common::Rect &mediaRect);

private:
	// Bundles the regular/bold font pair with the pixel size they were created for so they are updated atomically.
	struct FontSet {
		int size = 0;
		Common::ScopedPtr<Graphics::Font> regular;
		Common::ScopedPtr<Graphics::Font> bold;

		bool matchesSize(int requestedSize) const {
			return regular && bold && size == requestedSize;
		}
	};

	// Precomputed geometry and wrapped lines for rendering a single subtitle card.
	struct SubtitleLayout {
		Common::Rect boxBounds;
		Common::Rect clippedBounds;
		Common::U32String speakerPrefix;
		int speakerWidth = 0;
		Common::Array<Common::U32String> lines;
	};

	// Loads or reloads the configured font pair if needed, returning true when the font size changed.
	bool loadFont();

	// Renders subtitles for the specified media file at the specified playback time onto the destination surface at the
	// specified position.
	bool renderForMedia(Graphics::Surface *destSurface, const Common::Rect &boxRect, const Common::Path &filePath, uint32 currentMs);

	// Computes wrapped dialogue lines, speaker prefix metrics, and positioned/clipped box bounds for a subtitle entry.
	SubtitleLayout buildSubtitleLayout(const SubtitleManager::SubtitleEntry &entry, const Common::Rect &targetBoxRect, const Common::Rect &surfaceBounds) const;

	// Draws the chamfered subtitle box borders and CRT scanline-blended background scrim.
	void drawSubtitleBox(Graphics::Surface *destSurface, const Common::Rect &boxBounds, const Common::Rect &clippedBounds) const;

	// Draws the speaker prefix and wrapped dialogue lines inside the subtitle box.
	void drawSubtitleText(Graphics::Surface *destSurface, const SubtitleLayout &layout) const;

	// Returns true if the given video window plays inside the Jumpsuit visor (direct child of SceneViewWindow).
	bool isJumpsuitVideoWindow(const Window *videoWindow) const;

	// Returns a rectangle to show subtitles beneath media showing within the specified rectangle. The box will match the width of the
	// rectangle and be tall enough to render two lines of subtitle text.
	Common::Rect calculateBoxBoundsForVideo(const Window *videoWindow, const Common::Rect &mediaRect) const;

	// Returns the subtitle font height in pixels.
	int getFontHeight() const;

	// Returns the height of the subtitle box, in pixels, for the default or given number of text lines.
	int getBoxHeight() const;
	int getBoxHeight(int numLines) const;

	// Returns the rectangle for the default subtitle position. The default position is suitable for subtitles shown while the main HUD
	// (jumpsuit helmet interior) is shown.
	Common::Rect getDefaultBoxBounds() const;

	uint32 getColor(const ColorRGB &color) const;

	BuriedEngine *_vm;
	const SubtitleManager *_subtitles;
	FontSet _fonts;
	Common::Rect _lastRenderedBounds;
};

} // End of namespace Buried

#endif
