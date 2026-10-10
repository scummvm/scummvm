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

#ifndef GLK_PREVIEW_H
#define GLK_PREVIEW_H

#include "glk/options.h"
#include "graphics/font.h"

namespace Glk {

/** Independently owned, bounded font resources for a preview of preferences. */
class GlkPreviewFonts {
	const Graphics::Font *_fonts[5];
	FontInfo _metrics[5];
public:
	GlkPreviewFonts();
	~GlkPreviewFonts();
	void clear();
	bool load(GlkOptionsState &settings, int scale);
	const Graphics::Font *font(uint index) const { return _fonts[index]; }
	int lineHeight(uint index) const { return MAX(1, _metrics[index]._leading); }
	static bool fitsSurface(int width, int height, const Graphics::PixelFormat &format);
private:
	GlkPreviewFonts(const GlkPreviewFonts &);
	GlkPreviewFonts &operator=(const GlkPreviewFonts &);
};

/** Measurements shared by capability checking and the actual dialog. */
struct GlkPreviewLayout {
	int width, height, contentWidth, contentHeight;
	int lineHeight, popupHeight, buttonHeight, popupWidth, closeWidth;
	int padding, spacing;
	Common::U32String measure(GlkOptionsState &settings);
};

Common::U32String glkPreviewUnavailableReason(GlkOptionsState &settings);
void showGlkPreview(GlkOptionsState &settings);

} // End of namespace Glk

#endif
