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
#include "common/file.h"
#include "graphics/fonts/ttf.h"

namespace AGS3 {

Graphics::Font *ALFONT_FONT::getFont() {
	if (!_fonts.contains(_size)) {
		Common::File f;
		if (!f.open(_filename))
			error("Could not read font - %s", _filename.c_str());
		_fonts[_size] = Graphics::loadTTFFont(f, f.size());
		f.close();

		assert(_fonts[_size]);
	}

	return _fonts[_size];
}

/*------------------------------------------------------------------*/

ALFONT_FONT *alfont_loadFont(const Common::String &filename) {
	ALFONT_FONT *result = new ALFONT_FONT();
	result->_filename = filename;

	return result;
}

void alfont_destroy_font(ALFONT_FONT *font) {
	delete font;
}

size_t alfont_text_length(ALFONT_FONT *font, const char *text) {
	return font->getFont()->getStringWidth(text);
}

size_t alfont_text_height(ALFONT_FONT *font) {
	return font->getFont()->getFontHeight();
}

void alfont_textout(BITMAP *bmp, ALFONT_FONT *font, const char *text, int x, int y, uint32 color) {
	Graphics::ManagedSurface &surf = **bmp;
	font->getFont()->drawString(&surf, Common::U32String(text), x, y, bmp->w - x, color);
}

void alfont_textout_aa(BITMAP *bmp, ALFONT_FONT *font, const char *text, int x, int y, uint32 color) {
	alfont_textout(bmp, font, text, x, y, color);
}

void alfont_set_font_size(ALFONT_FONT *font, int size) {
	font->_size = size;
}

const char *alfont_get_name(ALFONT_FONT *font) {
	// TODO: Return ttf font name
	return "Unsupported";
}

} // namespace AGS3
