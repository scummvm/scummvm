/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/lib/alfont/alfont.h"
#include "ags/ags.h"
#include "ags/globals.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/display.h"
#include "common/file.h"
#include "graphics/fonts/ttf.h"

namespace AGS3 {

Graphics::Font *ALFONT_FONT::getFont() {
#ifdef USE_FREETYPE2
	if (!_fonts.contains(_size)) {
		// Instantiate the raw TTF data into a font of the given size
		Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome;
		if (ShouldAntiAliasText())
			renderMode = Graphics::kTTFRenderModeLight;
		_fonts[_size] = Graphics::loadTTFFont(_ttfData, _size, Graphics::kTTFSizeModeCharacter, 0, renderMode);
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

void alfont_textout(BITMAP *bmp, ALFONT_FONT *font, const char *text, int x, int y, uint32 color) {
	// Note: the original does not use antialiasing when drawing on 8 bit bmp
	// if (bitmap_color_depth(bmp) > 8) do not use AA in getFont()...
	// The original alfont changes the y based on the font height and ascent.
	y += (font->_size - font->getFont()->getFontAscent());
	Graphics::ManagedSurface &surf = **bmp;
	font->getFont()->drawString(&surf, text, x, y, bmp->w - x, color);
}

void alfont_set_font_size(ALFONT_FONT *font, int size) {
	font->_size = size;
}

const char *alfont_get_name(ALFONT_FONT *font) {
	// TODO: Return ttf font name
	return "Unsupported";
}

} // namespace AGS3
