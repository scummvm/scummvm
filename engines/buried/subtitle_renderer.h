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
class SubtitleManager;
class Window;

class SubtitleRenderer {
public:
	// Font size preset values (pixel height) used by the Engine Options dialog.
	enum FontSize {
		kFontSizeSmall = 12,
		kFontSizeMedium = 14,
		kFontSizeLarge = 18,
		kFontSizeXLarge = 22
	};

	static constexpr int kDefaultFontSize = kFontSizeMedium;

	// Configuration key for the user's subtitle font size preference in the Engine Options dialog.
	static constexpr const char *kFontSizeKey = "subtitle_font_size";

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

		// Checks that both font faces have been loaded (non-null) and match requestedSize.
		bool matchesSize(int requestedSize) const {
			return regular && bold && size == requestedSize;
		}
	};

	FontSet createFontSet(int fontSize) const;
	bool renderForMedia(Graphics::Surface *destSurface, const Common::Path &filePath, uint32 currentMs, const Common::Rect *aboveRect = nullptr);

	BuriedEngine *_vm;
	const SubtitleManager *_subtitles;
	FontSet _fonts;
	Common::Rect _lastRenderedBounds;
};

} // End of namespace Buried

#endif
