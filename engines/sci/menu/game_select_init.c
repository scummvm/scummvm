/***************************************************************************
 game_select_init.c Copyright (C) 2004 Hugues Valois


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

#include <sci_memory.h>
#include <gfx_operations.h>
#include "game_select.h"
#include "sciresource.h"

/* NOTE: THIS CODE BASED ON _gfxop_init_common (COPIED AND MODIFIED) */
static int
game_select_gfxop_init_common(gfx_state_t *state, gfx_options_t *options, void *misc_payload)
{
	int i;

	state->options = options;

	if ((state->static_palette = gfxr_interpreter_get_static_palette(state->resstate,
						SCI_VERSION_0,
						&(state->static_palette_entries),
						misc_payload)))
	{
		for (i = 0; i < state->static_palette_entries; i++)
			gfx_alloc_color(state->driver->mode->palette, state->static_palette + i);
	}

/* 	if (!((state->resstate = gfxr_new_resource_manager(state->version, */
/* 							   state->options, */
/* 							   state->driver, */
/* 							   misc_payload)))) { */
/* 		GFXERROR("Failed to initialize resource manager!\n"); */
/* 		return GFX_FATAL; */
/* 	} */


	state->visible_map = GFX_MASK_VISUAL;
	gfxop_set_clip_zone(state, gfx_rect(0, 0, 320, 200));

	state->mouse_pointer = state->mouse_pointer_bg = NULL;
	state->mouse_pointer_visible = 0;
	state->control_map = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320, 200, GFX_RESID_NONE, 0, 0));
	state->control_map->flags |= GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
	state->options = options;
	state->mouse_pointer_in_hw = 0;
	state->disable_dirty = 0;
	state->events = NULL;

	state->pic = state->pic_unscaled = NULL;

	state->pic_nr = -1; /* Set background pic number to an invalid value */

	state->tag_mode = 0;

	state->dirty_rects = NULL;


	return GFX_OK;
}

/* NOTE: THIS CODE BASED ON gfxop_init_default (COPIED AND MODIFIED) */
int
game_select_gfxop_init_default(gfx_state_t *state, gfx_options_t *options, void *misc_info)
{
	if (state->driver->init(state->driver))
		return GFX_FATAL;

	return game_select_gfxop_init_common(state, options, misc_info);
}

/* NOTE: THIS CODE BASED ON gfxop_init (COPIED AND MODIFIED) */
int
game_select_gfxop_init(gfx_state_t *state, int xfact, int yfact, gfx_color_mode_t bpp,
	   gfx_options_t *options, void *misc_info)
{
	int color_depth = bpp? bpp : 1;
	int initialized = 0;

	do {
		if (!state->driver->init_specific(state->driver, xfact, yfact, color_depth))
			initialized = 1;
		else
			color_depth++;
	} while (!initialized && color_depth < 9 && !bpp);

	if (!initialized)
		return GFX_FATAL;

	return game_select_gfxop_init_common(state, options, misc_info);
}

#if 0
/* this can be used to generate code that creates a particular font at runtime */
/* this is meant to be used as a development tool */
void save_font(int id, gfx_bitmap_font_t* font)
{
	FILE* file;
	char filepath[128];
	char filename[128];
	char buffer[1024];
	int i = 0;

	_itoa(id, filename, 10);
	strcpy(filepath, filename);
	strcat(filepath, ".c");

	file = fopen(filepath, "w");

	strcpy(buffer, "#include <sci_memory.h>\n#include <gfx_system.h>\n#include <gfx_resource.h>\n#include <gfx_tools.h>\n\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* font widths */
	strcpy(buffer, "static int font_widths[] = \n{\n");
	fwrite(buffer, strlen(buffer), 1, file);

	for (i = 0; i < font->chars_nr; i++)
	{
		strcpy(buffer, "\t");
		fwrite(buffer, strlen(buffer), 1, file);

		_itoa(font->widths[i], buffer, 10);
		fwrite(buffer, strlen(buffer), 1, file);

		strcpy(buffer, ",\n");
		fwrite(buffer, strlen(buffer), 1, file);
	}

	strcpy(buffer, "};\n\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* font data */
	strcpy(buffer, "static byte font_data[] = \n{\n");
	fwrite(buffer, strlen(buffer), 1, file);

	for (i = 0; i < font->chars_nr * font->height * font->row_size; i++)
	{
		strcpy(buffer, "\t");
		fwrite(buffer, strlen(buffer), 1, file);

		_itoa(font->data[i], buffer, 10);
		fwrite(buffer, strlen(buffer), 1, file);

		strcpy(buffer, ",\n");
		fwrite(buffer, strlen(buffer), 1, file);
	}

	strcpy(buffer, "};\n\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* font structure */
	strcpy(buffer, "static gfx_bitmap_font_t font = \n{\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* ID */
	strcpy(buffer, "\t");
	fwrite(buffer, strlen(buffer), 1, file);

	_itoa(id, buffer, 10);
	fwrite(buffer, strlen(buffer), 1, file);

	strcpy(buffer, ",\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* chars_nr */
	strcpy(buffer, "\t");
	fwrite(buffer, strlen(buffer), 1, file);

	_itoa(font->chars_nr, buffer, 10);
	fwrite(buffer, strlen(buffer), 1, file);

	strcpy(buffer, ",\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* widths */
	strcpy(buffer, "\t");
	fwrite(buffer, strlen(buffer), 1, file);

	strcpy(buffer, "font_widths,\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* row_size */
	strcpy(buffer, "\t");
	fwrite(buffer, strlen(buffer), 1, file);

	_itoa(font->row_size, buffer, 10);
	fwrite(buffer, strlen(buffer), 1, file);

	strcpy(buffer, ",\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* line_height */
	strcpy(buffer, "\t");
	fwrite(buffer, strlen(buffer), 1, file);

	_itoa(font->line_height, buffer, 10);
	fwrite(buffer, strlen(buffer), 1, file);

	strcpy(buffer, ",\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* height */
	strcpy(buffer, "\t");
	fwrite(buffer, strlen(buffer), 1, file);

	_itoa(font->height, buffer, 10);
	fwrite(buffer, strlen(buffer), 1, file);

	strcpy(buffer, ",\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* char_size */
	strcpy(buffer, "\t");
	fwrite(buffer, strlen(buffer), 1, file);

	_itoa(font->char_size, buffer, 10);
	fwrite(buffer, strlen(buffer), 1, file);

	strcpy(buffer, ",\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* data */
	strcpy(buffer, "\t");
	fwrite(buffer, strlen(buffer), 1, file);

	strcpy(buffer, "font_data,\n");
	fwrite(buffer, strlen(buffer), 1, file);

	strcpy(buffer, "};\n\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* function definition */
	strcpy(buffer, "gfx_bitmap_font_t* get_font_");
	fwrite(buffer, strlen(buffer), 1, file);

	strcpy(buffer, filename);
	fwrite(buffer, strlen(buffer), 1, file);

	/* function body start*/
	strcpy(buffer, "()\n{\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* return */
	strcpy(buffer, "\treturn &font;\n");
	fwrite(buffer, strlen(buffer), 1, file);

	/* function body end */
	strcpy(buffer, "}\n");
	fwrite(buffer, strlen(buffer), 1, file);

	fclose(file);
}
#endif

