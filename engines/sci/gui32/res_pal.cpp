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

#include "sci/sci.h"	// for INCLUDE_OLDGFX
#ifdef INCLUDE_OLDGFX

/* SCI1 palette resource defrobnicator */

#include "common/file.h"
#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"

namespace Sci {

#define MAX_COLORS 256
#define PALETTE_START 260
#define COLOR_OK 0x01

#define SCI_PAL_FORMAT_VARIABLE_FLAGS 0
#define SCI_PAL_FORMAT_CONSTANT_FLAGS 1

Palette *gfxr_read_pal11(int id, byte *resource, int size) {
	int start_color = resource[25];
	int format = resource[32];
	int entry_size = (format == SCI_PAL_FORMAT_VARIABLE_FLAGS) ? 4 : 3;
	byte *pal_data = resource + 37;
	int _colors_nr = READ_LE_UINT16(resource + 29);

	// Happens at the beginning of Pepper
	if (_colors_nr > 256)
		return NULL;

	Palette *retval = new Palette(_colors_nr + start_color);
	int i;

	char buf[100];
	sprintf(buf, "read_pal11 (id %d)", id);
	retval->name = buf;

	for (i = 0; i < start_color; i ++) {
		retval->setColor(i, 0, 0, 0);
	}
	for (i = start_color; i < start_color + _colors_nr; i ++) {
		switch (format) {
		case SCI_PAL_FORMAT_CONSTANT_FLAGS:
			retval->setColor(i, pal_data[0], pal_data[1], pal_data[2]);
			break;
		case SCI_PAL_FORMAT_VARIABLE_FLAGS:
			if (pal_data[0] & 1)
				retval->setColor(i, pal_data[1], pal_data[2], pal_data[3]);
			break;
		}
		pal_data += entry_size;
	}

	return retval;
}

Palette *gfxr_read_pal1(int id, byte *resource, int size) {
	int counter = 0;
	int pos;
	unsigned int colors[MAX_COLORS] = {0};

	if (size < PALETTE_START + 4) {
		error("Palette resource too small in %04x", id);
		return NULL;
	}

	pos = PALETTE_START;

	while (pos < size/* && resource[pos] == COLOR_OK && counter < MAX_COLORS*/) {
		colors[counter++] = resource[pos] | (resource[pos + 1] << 8) | (resource[pos + 2] << 16) | (resource[pos + 3] << 24);
		pos += 4;
	}

	if (counter < MAX_COLORS) {
		error("SCI1 palette %04x ends prematurely", id);
		return NULL;
	}

	Palette *retval = new Palette(counter);
	char buf[100];
	sprintf(buf, "read_pal1 (id %d)", id);
	retval->name = buf;

	for (pos = 0; pos < counter; pos++) {
		unsigned int color = colors[pos];
		if (color & 1)
			retval->setColor(pos, (color >> 8) & 0xff, (color >> 16) & 0xff, (color >> 24) & 0xff);
	}

	return retval;
}

Palette *gfxr_read_pal1_amiga(Common::File &file) {
	int i;
	Palette *retval = new Palette(32);

	for (i = 0; i < 32; i++) {
		int b1, b2;

		b1 = file.readByte();
		b2 = file.readByte();

		if (b1 == EOF || b2 == EOF) {
			error("Amiga palette file ends prematurely");
			return NULL;
		}

		retval->setColor(i, (b1 & 0xf) * 0x11, ((b2 & 0xf0) >> 4) * 0x11, (b2 & 0xf) * 0x11);
	}

	return retval;
}

} // End of namespace Sci

#endif
