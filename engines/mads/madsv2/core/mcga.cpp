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

#include "common/system.h"
#include "graphics/paletteman.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {

#define mcga_retrace_magic      14      /* Dave McKibbin's magic heuristic # */

word mcga_shakes = false;
int  mcga_retrace_computed = false;
word mcga_retrace_ticks = 0;
int  mcga_retrace_max_colors = 256;
int  mcga_retrace_max_bytes = 768;

word mcga_palette_update = false;
int  mcga_palette_fast = true;


void mcga_put_pixel(word x, word y, byte c) {
	byte *dest = (byte *)g_engine->getScreen()->getBasePtr(x, y);
	*dest = c;
}

byte mcga_get_pixel(word x, word y) {
	return *(byte *)g_engine->getScreen()->getBasePtr(x, y);
}

byte *mcga_open_window(word x, word y, word xsize, word ysize) {
	byte *out;
	byte *live;
	byte *work;
	word *run;
	word yy, size;

	size = xsize * ysize + 8;
	out = (byte *)mem_get((long)size);
	if (out == NULL)
		return(NULL);

	run = (word *)out;
	work = out + 8;

	*run++ = x;
	*run++ = y;
	*run++ = xsize;
	*run++ = ysize;

	live = (byte *)g_engine->getScreen()->getBasePtr(x, y);

	for (yy = 1; yy <= ysize; yy++) {
		memcpy(work, live, xsize);
		work += xsize;
		live += 320;
	}

	return out;
}

void mcga_close_window(byte *inp) {
	word *run;
	byte *live;
	byte *work;
	word x, y, xsize, ysize, yy;

	run = (word *)inp;
	work = inp + 8;
	x = *run++;
	y = *run++;
	xsize = *run++;
	ysize = *run++;

	live = (byte *)g_engine->getScreen()->getBasePtr(x, y);

	for (yy = 1; yy <= ysize; yy++) {
		memcpy(live, work, xsize);
		work += xsize;
		live += 320;
	}

	mem_free(inp);
}

void mcga_getpal(Palette *pal) {
	byte tmp[Graphics::PALETTE_COUNT * 3];
	g_system->getPaletteManager()->grabPalette(tmp, 0, Graphics::PALETTE_COUNT);
	for (int i = 0; i < Graphics::PALETTE_COUNT; i++) {
		(*pal)[i].r = tmp[i * 3 + 0] * 63 / 255;
		(*pal)[i].g = tmp[i * 3 + 1] * 63 / 255;
		(*pal)[i].b = tmp[i * 3 + 2] * 63 / 255;
	}
}

void mcga_setpal_range(Palette *pal, int first_color, int num_colors) {
	byte tmp[Graphics::PALETTE_COUNT * 3];
	for (int i = 0; i < num_colors; i++) {
		tmp[i * 3 + 0] = (*pal)[first_color + i].r * 255 / 63;
		tmp[i * 3 + 1] = (*pal)[first_color + i].g * 255 / 63;
		tmp[i * 3 + 2] = (*pal)[first_color + i].b * 255 / 63;
	}
	g_system->getPaletteManager()->setPalette(tmp, first_color, num_colors);
}

void mcga_setpal(Palette *pal) {
	mcga_setpal_range(pal, 0, Graphics::PALETTE_COUNT);
}

void mcga_cls(byte inp) {
	g_engine->getScreen()->clear(inp);
}

void mcga_retrace() {
	g_engine->getScreen()->update();
}

void mcga_compute_retrace_parameters(void) {
	mcga_setpal(&master_palette);

	// On original hardware this function also measured how many palette
	// entries could be written inside one vertical retrace interval.
	// Under a modern graphics API there is no such constraint.
	mcga_palette_fast = true;
	mcga_retrace_max_colors = Graphics::PALETTE_COUNT;
	mcga_retrace_max_bytes = Graphics::PALETTE_SIZE;
	mcga_retrace_computed = true;
}

void mcga_shake() {
	warning("TODO: mcga_shake");
}

void mcga_reset() {
	Palette temp_palette;
	pal_interface(temp_palette);

	memcpy(&temp_palette[20], &temp_palette[6], sizeof(RGBcolor));
	memcpy(&temp_palette[56], &temp_palette[8], sizeof(RGBcolor) << 3);

	mcga_setpal(&temp_palette);
}

} // namespace MADSV2
} // namespace MADS
