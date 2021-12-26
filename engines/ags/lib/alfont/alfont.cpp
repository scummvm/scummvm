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
#include "graphics/fonts/ttf.h"
#include "graphics/fonts/winfont.h"
#include "ags/lib/alfont/alfont.h"
#include "ags/ags.h"
#include "ags/globals.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/display.h"

namespace AGS3 {

Graphics::Font *ALFONT_FONT::getFont() {
#ifdef USE_FREETYPE2
	if (!_fonts.contains(_size)) {
		// Instantiate the raw TTF data into a font of the given size
		Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome;
		if (ShouldAntiAliasText())
			renderMode = Graphics::kTTFRenderModeLight;
		Graphics::Font *font = Graphics::loadTTFFont(_ttfData, _size, Graphics::kTTFSizeModeCharacter, 0, renderMode);
		if (!font) {
			// Try WinFont as TTFFont may fail loading those
			Graphics::WinFont *winfont = new Graphics::WinFont();
			if (winfont->loadFromFON(_ttfData))
				font = winfont;
			else
				delete winfont;
		}
		_fonts[_size] = font;
		assert(_fonts[_size]);
	}

	return _fonts[_size];
#else
	error("Game needs FreeType library, which was not included in this build");
#endif
}

/*------------------------------------------------------------------*/

ALFONT_FONT *alfont_load_font_from_mem(const byte *data, int data_len) {
	return new ALFONT_FONT(data, data_len);
}

void alfont_destroy_font(ALFONT_FONT *font) {
	delete font;
}

size_t alfont_text_length(ALFONT_FONT *font, const char *text) {
	return font->getFont()->getStringWidth(text);
}

size_t alfont_text_height(ALFONT_FONT *font) {
	return font->_size;
}

void alfont_textout(BITMAP *bmp, ALFONT_FONT *font, ALFONT_FONT *refFont, const char *text, int x, int y, uint32 color) {
	// Note: the original does not use antialiasing when drawing on 8 bit bmp
	// if (bitmap_color_depth(bmp) > 8) do not use AA in getFont()...
	// The original alfont changes the y based on the font height and ascent.
	y += (font->_size - font->getFont()->getFontAscent());
	Graphics::ManagedSurface &surf = **bmp;
	Graphics::Font *fnt = font->getFont();
	uint32 col = (color == surf.getTransparentColor()) ? color - 1 : color;

	if (!refFont) {
		// Standard string draw
		fnt->drawString(&surf, text, x, y, bmp->w - x, col);

	} else {
		// Drawing an outline prior to standard font drawing on top.
		// We use the font's character widths to ensure the two match up
		refFont->_size = font->_size;
		Graphics::Font *srcFnt = refFont->getFont();

		for (int w = bmp->w - x; *text && w > 0; ++text) {
			fnt->drawChar(&surf, *text, x, y, col);

			int charWidth = srcFnt->getCharWidth(*text);
			x += charWidth;
			w -= charWidth;
		}
	}
}

void alfont_set_font_size(ALFONT_FONT *font, int size) {
	font->_size = size;
}

int alfont_get_font_height(ALFONT_FONT *font) {
	return font->_size;
}

int alfont_get_font_real_height(ALFONT_FONT *font) {
	return font->_size;
}

const char *alfont_get_name(ALFONT_FONT *font) {
	// TODO: Return ttf font name
	return "Unsupported";
}

} // namespace AGS3
