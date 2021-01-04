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

#ifndef AGS_LIB_ALFONT_H
#define AGS_LIB_ALFONT_H

#include "common/hashmap.h"
#include "common/memstream.h"
#include "graphics/font.h"
#include "ags/lib/allegro/gfx.h"

namespace AGS3 {

struct ALFONT_FONT {
	Common::MemoryReadStream _ttfData;
	int _size;
	Common::HashMap<int, Graphics::Font *> _fonts;

	ALFONT_FONT() : _size(-1), _ttfData(nullptr, 0) {}
	ALFONT_FONT(const byte *data, size_t size) : _size(-1), _ttfData(data, size, DisposeAfterUse::YES) {}

	~ALFONT_FONT() {
		for (Common::HashMap<int, Graphics::Font *>::iterator it = _fonts.begin();
			it != _fonts.end(); ++it)
			delete (*it)._value;
	}

	Graphics::Font *getFont();
};

inline void alfont_init() {}
inline void alfont_exit() {}
inline void alfont_text_mode(int val) {}
extern ALFONT_FONT *alfont_load_font_from_mem(const byte *data, int data_len);
extern void alfont_destroy_font(ALFONT_FONT *font);

extern size_t alfont_text_length(ALFONT_FONT *font, const char *text);
extern size_t alfont_text_height(ALFONT_FONT *font);
extern void alfont_textout(BITMAP *bmp, ALFONT_FONT *font, const char *text, int x, int y, uint32 color);
extern void alfont_textout_aa(BITMAP *bmp, ALFONT_FONT *font, const char *text, int x, int y, uint32 color);
extern const char *alfont_get_name(ALFONT_FONT *font);
extern void alfont_set_font_size(ALFONT_FONT *font, int size);


} // namespace AGS3

#endif
