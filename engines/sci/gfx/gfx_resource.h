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

/* SCI Resource library */

#ifndef SCI_GFX_GFX_RESOURCE_H
#define SCI_GFX_GFX_RESOURCE_H

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_driver.h"

namespace Common {
	class File;
}

namespace Sci {

/*** Styles for pic0 drawing ***/
/* Dithering modes */
#define GFXR_DITHER_MODE_D16 0  /* Sierra SCI style */
#define GFXR_DITHER_MODE_F256 1 /* Flat color interpolation */
#define GFXR_DITHER_MODE_D256 2 /* 256 color dithering */
/* Dithering patterns */
#define GFXR_DITHER_PATTERN_SCALED 0 /* Dither per pixel on the 320x200 grid */
#define GFXR_DITHER_PATTERN_1 1      /* Dither per pixel on the target */

#define SCI_TITLEBAR_SIZE 10

#define DRAWPIC01_FLAG_FILL_NORMALLY 1
#define DRAWPIC01_FLAG_OVERLAID_PIC 2

#define GFXR_AUX_MAP_SIZE (320*200)


#define GFX_SCI0_IMAGE_COLORS_NR 16
#define GFX_SCI0_PIC_COLORS_NR 256

#define GFX_SCI1_AMIGA_COLORS_NR 32

extern int sci0_palette;

/* (gfx_pic_0.c) The 16 EGA base colors */
extern gfx_pixmap_color_t gfx_sci0_image_colors[][16];

/* (gfx_pic_0.c) The 256 interpolated colors (initialized when
** gfxr_init_pic() is called for the first time, or when gfxr_init_static_palette() is called)
*/
extern gfx_pixmap_color_t gfx_sci0_pic_colors[];


struct gfxr_pic0_params_t {
	gfx_line_mode_t line_mode; /* one of GFX_LINE_MODE_* */
	gfx_brush_mode_t brush_mode;
	rect_t pic_port_bounds;
};

struct gfxr_pic_t {
	int ID; /* pic number (NOT resource ID, just number) */
	gfx_mode_t *mode;
	gfx_pixmap_t *visual_map;
	gfx_pixmap_t *priority_map;
	gfx_pixmap_t *control_map;

	byte aux_map[GFXR_AUX_MAP_SIZE];

	/* Auxiliary map details:
	** Bit 0: Vis
	** Bit 1: Pri
	** Bit 2: Ctrl
	** Bit 3-5: 'filled' (all three bits are set to 1)
	*/

	rect_t bounds;

	void *undithered_buffer; /* copies visual_map->index_data before dithering */
	int undithered_buffer_size;

	void *internal; /* Interpreter information, or NULL. Will be freed
			** automatically when the pic is freed!  */

};


struct gfxr_loop_t {
	int cels_nr;
	gfx_pixmap_t **cels;
};


struct gfxr_view_t {
	int ID;

	int flags;
	int colors_nr;
	gfx_pixmap_color_t *colors;

	int loops_nr;
	gfxr_loop_t *loops;

	int translation[GFX_SCI0_IMAGE_COLORS_NR];
};


/* unscaled color index mode: Used in addition to a scaled mode
** to render the pic resource twice. See gfxr_remove_artifacts_pic0().
*/
extern gfx_mode_t mode_1x1_color_index;

void gfxr_init_static_palette();
/* Initializes the static 256 color palette
** Parameters: (void)
** Returns   : (void)
*/

gfxr_pic_t *gfxr_init_pic(gfx_mode_t *mode, int ID, int sci1);
/* Initializes a gfxr_pic_t for a specific mode
** Parameters: (gfx_mode_t *) mode: The specific graphics mode
**             (int) ID: The ID to assign to the resulting pixmaps
** Returns   : (gfxr_pic_t *) The allocated pic resource, or NULL on error.
** This function allocates memory for use by resource drawer functions.
*/

void gfxr_free_pic(gfx_driver_t *driver, gfxr_pic_t *pic);
/* Uninitializes a pic resource
** Parameters: (gfx_driver_t *) driver: The driver the pic should be removed from
**             (gfxr_pic_t *) pic: The pic to free
** Returns   : (void)
*/

void gfxr_free_view(gfx_driver_t *driver, gfxr_view_t *view);
/* Frees all memory associated with a view
** Paremeters: (gfx_driver_t *) driver: The driver the view should be removed from
**             (gfxr_view_t *) view: The view to free
** Returns   : (void)
*/




/*********************/
/*  SCI0 operations  */
/*********************/


void gfxr_clear_pic0(gfxr_pic_t *pic, int sci_titlebar_size);
/* Clears all pic buffers of one pic
** Parameters: (gfxr_pic_t) pic: The picture to clear
**             (int) sci_titlebar_size: How much space to reserve for the title bar
** Returns   : (void)
** This function should be called before gfxr_draw_pic0, unless cumulative
** drawing is intended
*/


void gfxr_draw_pic01(gfxr_pic_t *pic, int fill_normally, int default_palette,
	int size, byte *resource, gfxr_pic0_params_t *style, int resid, int sci1,
	gfx_pixmap_color_t *static_pal, int static_pal_nr);
/* Draws a pic resource (all formats prior to SCI1.1)
** Parameters: (gfxr_pic_t *) pic: The pic to draw to
**             (int) fill_normally: If 1, the pic is drawn normally; if 0, all
**                                  fill operations will fill with black
**             (int) default_palette: The default palette to use for drawing
**             (int) size: Resource size
**             (byte *) resource: Pointer to the resource data
**             (gfxr_pic0_params_t *) style: The drawing style
**             (int) resid: The resource ID
**             (int) sci1: Nonzero if SCI1
**             (gfx_pixmap_color_t *) static_pal: The static palette
**             (int) static_pal_nr: Number of entries in static palette
** Returns   : (void)
** The result is stored in gfxr_visual_map, gfxr_priority_map, and gfxr_control_map.
** The palette entry of gfxr_visual_map is never used.
** Note that the picture will not be drawn dithered; use gfxr_dither_pic0 for that.
*/

void gfxr_draw_pic11(gfxr_pic_t *pic, int fill_normally, int default_palette,
	int size, byte *resource, gfxr_pic0_params_t *style, int resid,
	gfx_pixmap_color_t *static_pal, int static_pal_nr);
/* Draws a pic resource (SCI1.1)
** Parameters: (gfxr_pic_t *) pic: The pic to draw to
**             (int) fill_normally: If 1, the pic is drawn normally; if 0, all
**                                  fill operations will fill with black
**             (int) default_palette: The default palette to use for drawing
**             (int) size: Resource size
**             (byte *) resource: Pointer to the resource data
**             (gfxr_pic0_params_t *) style: The drawing style
**             (int) resid: The resource ID
**             (gfx_pixmap_color_t *) static_pal: The static palette
**             (int) static_pal_nr: Number of entries in static palette
** Returns   : (void)
** The result is stored in gfxr_visual_map, gfxr_priority_map, and gfxr_control_map.
** The palette entry of gfxr_visual_map is never used.
** Note that the picture will not be drawn dithered; use gfxr_dither_pic0 for that.
*/

void gfxr_remove_artifacts_pic0(gfxr_pic_t *dest, gfxr_pic_t *src);
/* Removes artifacts from a scaled pic
** Parameters: (gfxr_pic_t *) dest: The scaled pic
**             (gfxr_pic_t *) src: An unscaled pic
** Returns   : (void)
** Using information from the (correctly rendered) src pic, this function implements
** some heuristics to remove artifacts from dest. Must be used before dither_pic0 is
** called, because it operates on the index buffer.
*/

void gfxr_dither_pic0(gfxr_pic_t *pic, int mode, int pattern);
/* Dithers a gfxr_visual_map
** Parameters: (gfxr_pic_t *) pic: The pic to dither
**             (int) mode: One of GFXR_DITHER_MODE
**             (int) pattern: One of GFXR_DITHER_PATTERN
** Returns   : (void)
*/

gfxr_view_t *gfxr_draw_view0(int id, byte *resource, int size, int palette);
/* Calculates an SCI0 view
** Parameters: (int) id: Resource ID of the view
**             (byte *) resource: Pointer to the resource to read
**             (int) size: Size of the resource
**	       (int) palette: The palette to use
** Returns   : (gfxr_view_t *) The resulting view
*/

/*********************/
/*  SCI1 operations  */
/*********************/

gfx_pixmap_color_t *gfxr_read_pal1(int id, int *colors_nr, byte *resource, int size);
/* Reads an SCI1 palette
** Parameters: (int) id: Resource ID for the palette (or the view it was found in)
**             (int *) colors_nr: Pointer to the variable the number of colors
**                                will be stored in
**             (byte *) resource: Source data
**             (int) size: Size of the memory block pointed to by resource
** Returns   : (gfx_pixmap_color_t *) *colors_nr color_t entries with the colors
*/

gfx_pixmap_color_t *gfxr_read_pal1_amiga(int *colors_nr, Common::File &file);
/* Reads an SCI1 palette
** Parameters: (int *) colors_nr: Pointer to the variable the number of colors
**                                will be stored in
**             (FILE *) f: Palette file
** Returns   : (gfx_pixmap_color_t *) *colors_nr color_t entries with the colors
*/

gfx_pixmap_color_t *gfxr_read_pal11(int id, int *colors_nr, byte *resource, int size);
/* Reads an SCI1.1 palette
** Parameters: (int) id: Resource ID for the palette (or the view it was found in)
**             (int *) colors_nr: Pointer to the variable the number of colors
**                                will be stored in
**             (byte *) resource: Source data
**             (int) size: Size of the memory block pointed to by resource
** Returns   : (gfx_pixmap_color_t *) *colors_nr color_t entries with the colors
*/

gfxr_view_t *gfxr_draw_view1(int id, byte *resource, int size, gfx_pixmap_color_t *static_pal,
	int static_pal_nr);
/* Calculates an SCI1 view
** Parameters: (int) id: Resource ID of the view
**             (byte *) resource: Pointer to the resource to read
**             (int) size: Size of the resource
**             (gfx_pixmap_color_t *) static_pal: The static palette
**             (int) static_pal_nr: Number of entries in static palette
** Returns   : (gfxr_view_t *) The resulting view
*/

gfx_pixmap_t *gfxr_draw_cel11(int id, int loop, int cel, int mirrored, byte *resource_base, byte *cel_base, int size, gfxr_view_t *view);


gfx_pixmap_t *gfxr_endianness_adjust(gfx_pixmap_t *pixmap, gfx_mode_t *mode);
/* Endianness-adjusts a pixmap, if neccessary
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to adjust
**             (gfx_mode_t *) mode: The mode to adjust it for
** Returns   : (gfx_pixmap_t *) pixmap, or NULL on error
** The pixmap is adjusted iff the mode signals that this is
** required (by means of setting the appropriate flag), and
** the mode has a byte depth of more than 1.
*/

static inline int get_int_16(const byte *offset) {
	return ((int) offset[0] | (((int) offset[1]) << 8));
}

} // End of namespace Sci

#endif // SCI_GFX_GFX_RESOURCE_H
