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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* SCI1 palette resource defrobnicator */

#include "sci/include/sci_memory.h"
#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"

namespace Sci {

#define MAX_COLORS 256
#define PALETTE_START 260
#define COLOR_OK 0x01

#define SCI_PAL_FORMAT_VARIABLE_FLAGS 0
#define SCI_PAL_FORMAT_CONSTANT_FLAGS 1

gfx_pixmap_color_t *gfxr_read_pal11(int id, int *colors_nr, byte *resource, int size) {
	int start_color = resource[25];
	int format = resource[32];
	int entry_size = 0;
	gfx_pixmap_color_t *retval;
	byte *pal_data = resource + 37;
	int _colors_nr = *colors_nr = getUInt16(resource + 29);
	int i;

	switch (format) {
	case SCI_PAL_FORMAT_VARIABLE_FLAGS :
		entry_size = 4;
		break;
	case SCI_PAL_FORMAT_CONSTANT_FLAGS :
		entry_size = 3;
		break;
	}

	retval = (gfx_pixmap_color_t *)sci_malloc(sizeof(gfx_pixmap_color_t) * (_colors_nr + start_color));
	memset(retval, 0, sizeof(gfx_pixmap_color_t) * (_colors_nr + start_color));

	for (i = 0; i < start_color; i ++) {
		retval[i].global_index = GFX_COLOR_INDEX_UNMAPPED;
		retval[i].r = 0;
		retval[i].g = 0;
		retval[i].b = 0;
	}
	for (i = start_color; i < start_color + _colors_nr; i ++) {
		switch (format) {
		case SCI_PAL_FORMAT_CONSTANT_FLAGS:
			retval[i].global_index = GFX_COLOR_INDEX_UNMAPPED;
			retval[i].r = pal_data[0];
			retval[i].g = pal_data[1];
			retval[i].b = pal_data[2];
			break;
		case SCI_PAL_FORMAT_VARIABLE_FLAGS:
			retval[i].global_index = GFX_COLOR_INDEX_UNMAPPED;
			retval[i].r = pal_data[1];
			retval[i].g = pal_data[2];
			retval[i].b = pal_data[3];
			break;
		}
		pal_data += entry_size;
	}

	return retval;
}

gfx_pixmap_color_t *gfxr_read_pal1(int id, int *colors_nr, byte *resource, int size) {
	int counter = 0;
	int pos;
	unsigned int colors[MAX_COLORS] = {0};
	gfx_pixmap_color_t *retval;

	if (size < PALETTE_START + 4) {
		GFXERROR("Palette resource too small in %04x\n", id);
		return NULL;
	}

	pos = PALETTE_START;

	while (pos < size/* && resource[pos] == COLOR_OK && counter < MAX_COLORS*/) {
		int color = resource[pos] | (resource[pos + 1] << 8) | (resource[pos + 2] << 16) | (resource[pos + 3] << 24);

		pos += 4;

		colors[counter++] = color;
	}

	if (counter < MAX_COLORS && resource[pos] != COLOR_OK) {
		GFXERROR("Palette %04x uses unknown palette color prefix 0x%02x at offset 0x%04x\n", id, resource[pos], pos);
		return NULL;
	}

	if (counter < MAX_COLORS) {
		GFXERROR("Palette %04x ends prematurely\n", id);
		return NULL;
	}

	retval = (gfx_pixmap_color_t*)sci_malloc(sizeof(gfx_pixmap_color_t) * counter);

	*colors_nr = counter;
	for (pos = 0; pos < counter; pos++) {
		unsigned int color = colors[pos];

		retval[pos].global_index = GFX_COLOR_INDEX_UNMAPPED;
		retval[pos].r = (color >> 8) & 0xff;
		retval[pos].g = (color >> 16) & 0xff;
		retval[pos].b = (color >> 24) & 0xff;
	}

	return retval;
}

gfx_pixmap_color_t *gfxr_read_pal1_amiga(int *colors_nr, FILE *f) {
	int i;
	gfx_pixmap_color_t *retval;

	retval = (gfx_pixmap_color_t *)sci_malloc(sizeof(gfx_pixmap_color_t) * 32);

	for (i = 0; i < 32; i++) {
		int b1, b2;

		b1 = fgetc(f);
		b2 = fgetc(f);

		if (b1 == EOF || b2 == EOF) {
			GFXERROR("Palette file ends prematurely\n");
			return NULL;
		}

		retval[i].global_index = GFX_COLOR_INDEX_UNMAPPED;
		retval[i].r = (b1 & 0xf) * 0x11;
		retval[i].g = ((b2 & 0xf0) >> 4) * 0x11;
		retval[i].b = (b2 & 0xf) * 0x11;
	}

	*colors_nr = 32;

	return retval;
}

} // End of namespace Sci
