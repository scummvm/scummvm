/***************************************************************************
 pal_1.c Copyright (C) 2000 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/
/* SCI1 palette resource defrobnicator */

#include "sci/include/sci_memory.h"
#include "sci/include/gfx_system.h"
#include "sci/include/gfx_resource.h"

#define MAX_COLORS 256
#define PALETTE_START 260
#define COLOR_OK 0x01

#define SCI_PAL_FORMAT_VARIABLE_FLAGS 0
#define SCI_PAL_FORMAT_CONSTANT_FLAGS 1

gfx_pixmap_color_t *
gfxr_read_pal11(int id, int *colors_nr, byte *resource, int size)
{
	int start_color = resource[25];
	int format = resource[32];
	int entry_size;
	gfx_pixmap_color_t *retval;
	byte *pal_data = resource + 37;
	int _colors_nr = *colors_nr = getUInt16(resource + 29);
	int i;

	switch (format)
	{
	case SCI_PAL_FORMAT_VARIABLE_FLAGS :
		entry_size = 4;
		break;
	case SCI_PAL_FORMAT_CONSTANT_FLAGS :
		entry_size = 3;
		break;
	}

	retval = (gfx_pixmap_color_t *)
		sci_malloc(sizeof(gfx_pixmap_color_t) * (_colors_nr + start_color));
	memset(retval, 0, sizeof(gfx_pixmap_color_t) * (_colors_nr + start_color));

	for (i = 0; i < start_color; i ++)
	{
		retval[i].global_index = GFX_COLOR_INDEX_UNMAPPED;
		retval[i].r = 0;
		retval[i].g = 0;
		retval[i].b = 0;
	}
	for (i = start_color; i < start_color + _colors_nr; i ++)
	{
		switch (format)
		{
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

gfx_pixmap_color_t *
gfxr_read_pal1(int id, int *colors_nr, byte *resource, int size)
{
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
		int color = resource[pos]
			| (resource[pos + 1] << 8)
			| (resource[pos + 2] << 16)
			| (resource[pos + 3] << 24);

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
#ifdef SATISFY_PURIFY
	memset(retval, 0, sizeof(gfx_pixmap_color_t) * counter);
#endif

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

gfx_pixmap_color_t *
gfxr_read_pal1_amiga(int *colors_nr, FILE *f)
{
	int i;
	gfx_pixmap_color_t *retval;

	retval = (gfx_pixmap_color_t*)sci_malloc(sizeof(gfx_pixmap_color_t) * 32);

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

