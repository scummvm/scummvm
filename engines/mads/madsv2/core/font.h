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

#ifndef MADS_CORE_FONT_H
#define MADS_CORE_FONT_H

#include "mads/madsv2/core/buffer.h"

namespace MADS {
namespace MADSV2 {

#define FONT_SIZE       128
#define FONT_MAX_WIDTH  255
#define FONT_MAX_HEIGHT 200

struct FontBuf {
	byte max_y_size;
	byte max_x_size;

	byte width[FONT_SIZE];
	byte *data[FONT_SIZE];
};

typedef struct FontBuf Font;
typedef Font *FontPtr;

extern byte font_colors[4];

extern FontPtr font_inter;              /* Font handle for interface text  */
extern FontPtr font_main;               /* Font handle for main sentence   */
extern FontPtr font_conv;               /* Font handle for conversations   */
extern FontPtr font_menu;               /* Font handle for menu stuff      */
extern FontPtr font_misc;               /* Font handle for symbols & icons */


extern FontPtr font_load(const char *name);
extern int font_write(FontPtr font, Buffer *target, const char *out_string,
	int x, int y, int auto_spacing);
extern void font_set_colors(int background, int high_intense, int med_intense, int low_intense);
extern int font_string_width(FontPtr font, const char *out_string, int auto_spacing);

} // namespace MADSV2
} // namespace MADS

#endif
