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

#ifndef M4_GRAPHICS_GR_FONT_H
#define M4_GRAPHICS_GR_FONT_H

#include "common/str.h"
#include "m4/m4_types.h"

namespace M4 {

struct Font {
	byte max_y_size;
	byte max_x_size;
	uint32 dataSize;

	byte *width;
	int16 *offset;
	byte *pixData;
};

void  gr_font_system_shutdown();
void  gr_font_dealloc(Font *killMe);
Font *gr_font_create_system_font();
void  gr_font_set_color(uint8 foreground);
Font *gr_font_get();
void  gr_font_set(Font *font);
int32 gr_font_get_height();
int32 gr_font_write(Buffer *target, char *out_string, int32 x, int32 y,
	int32 w, int32 auto_spacing = 1);
int32 gr_font_write(Buffer *target, const char *out_string, int32 x, int32 y,
	int32 w, int32 auto_spacing);
int32 gr_font_string_width(char *out_string, int32 auto_spacing = 1);
int32 gr_font_string_width(const Common::String &str, int32 auto_spacing = 1);

Font *gr_font_load(const char *fontName);

void font_set_colors(uint8 alt1, uint8 alt2, uint8 foreground);

} // namespace M4

#endif
