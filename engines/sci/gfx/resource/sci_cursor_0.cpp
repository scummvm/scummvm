/***************************************************************************
 cursor_0.c Copyright (C) 2000 Christoph Reichenbach


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

#include "sci/include/gfx_system.h"
#include "sci/include/gfx_resource.h"
#include "sci/include/gfx_tools.h"


#define CURSOR_RESOURCE_SIZE 68
#define CURSOR_SIZE 16

#define GFX_SCI01_CURSOR_COLORS_NR 3
#define GFX_SCI0_CURSOR_COLORS_NR 2

#define GFX_CURSOR_TRANSPARENT 255

gfx_pixmap_color_t gfx_sci01_cursor_colors[GFX_SCI01_CURSOR_COLORS_NR] = {
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0xff, 0xff, 0xff},
	{GFX_COLOR_INDEX_UNMAPPED, 0xaa, 0xaa, 0xaa}
};


static gfx_pixmap_t *
_gfxr_draw_cursor(int id, byte *resource, int size, int sci01) {
	int colors[4] = {0, 1, GFX_CURSOR_TRANSPARENT, 1};
	int line;
	byte *data;
	gfx_pixmap_t *retval;

	if (sci01)
		colors[3] = 2;

	if (size != CURSOR_RESOURCE_SIZE) {
		GFXERROR("Expected resource size of %d, but found %d\n", CURSOR_RESOURCE_SIZE, size);
		return NULL;
	}

	retval = gfx_pixmap_alloc_index_data(gfx_new_pixmap(CURSOR_SIZE, CURSOR_SIZE, id, 0, 0));
	retval->colors = gfx_sci01_cursor_colors;
	retval->colors_nr = sci01 ? GFX_SCI01_CURSOR_COLORS_NR : GFX_SCI0_CURSOR_COLORS_NR;
	retval->flags |= GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
	retval->color_key = GFX_CURSOR_TRANSPARENT;

	if (sci01) {
		retval->xoffset = get_int_16(resource);
		retval->yoffset = get_int_16(resource + 2);
	} else if (resource[3]) /* center */
		retval->xoffset = retval->yoffset = CURSOR_SIZE / 2;
	else
		retval->xoffset = retval->yoffset = 0;

	resource += 4;

	data = retval->index_data;
	for (line = 0; line < 16; line++) {
		int mask_a = get_int_16(resource + (line << 1));
		int mask_b = get_int_16(resource + 32 + (line << 1));
		int i;

		for (i = 0; i < 16; i++) {
			int color_code = ((mask_a << i) & 0x8000)
			                 | (((mask_b << i) >> 1) & 0x4000);

			*data++ = colors[color_code >> 14];
		}
	}
	return retval;
}


gfx_pixmap_t *
gfxr_draw_cursor0(int id, byte *resource, int size) {
	return _gfxr_draw_cursor(id, resource, size, 0);
}

gfx_pixmap_t *
gfxr_draw_cursor01(int id, byte *resource, int size) {
	return _gfxr_draw_cursor(id, resource, size, 1);
}

