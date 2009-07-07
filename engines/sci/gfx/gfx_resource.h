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

/** @file gfx_resource.h
 * SCI Resource library.
 */

#ifndef SCI_GFX_GFX_RESOURCE_H
#define SCI_GFX_GFX_RESOURCE_H

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_driver.h"

#include "common/rect.h"

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
#define DRAWPIC1_FLAG_MIRRORED 4

#define GFXR_AUX_MAP_SIZE (320*200)


#define GFX_SCI0_IMAGE_COLORS_NR 16
#define GFX_SCI0_PIC_COLORS_NR 256

#define GFX_SCI1_AMIGA_COLORS_NR 32

extern int sci0_palette;

/** The 16 EGA base colors */
extern Palette* gfx_sci0_image_pal[];
extern gfx_pixmap_color_t gfx_sci0_image_colors[][16];

/**
 * The 256 interpolated colors (initialized when gfxr_init_pic() is called
 * for the first time, or when gfxr_init_static_palette() is called)
 */
extern Palette* gfx_sci0_pic_colors;

struct gfxr_pic0_params_t {
	gfx_line_mode_t line_mode; /* one of GFX_LINE_MODE_* */
	gfx_brush_mode_t brush_mode;
};

/** A SCI resource pic */
struct gfxr_pic_t {
	int ID; /**< pic number (NOT resource ID, just number) */
	gfx_mode_t *mode;
	gfx_pixmap_t *visual_map; /**< Visual part of pic */
	gfx_pixmap_t *priority_map; /**< Priority map for pic */
	gfx_pixmap_t *control_map; /**< Control map for pic */

	/**
	 * Auxiliary map.
	 * Bit 0: Vis
	 * Bit 1: Pri
	 * Bit 2: Ctrl
	 * Bit 3-5: 'filled' (all three bits are set to 1)
	 */
	byte aux_map[GFXR_AUX_MAP_SIZE];

	// rect_t bounds;	// unused

	void *undithered_buffer; /**< copies visual_map->index_data before dithering */
	int undithered_buffer_size;

	int *priorityTable;
};

/** A animation loop */
struct gfxr_loop_t {
	int cels_nr; /**< Number of 'cels' or frames in the animation */
	gfx_pixmap_t **cels; /**< Pointer to the pixmaps for the cels */
};

/** A graphics view */
struct gfxr_view_t {
	int ID;

	int flags;
	Palette *palette;

	int loops_nr;
	gfxr_loop_t *loops;

	int translation[GFX_SCI0_IMAGE_COLORS_NR];
};

/**
 * Initializes the static 256 color palette.
 */
void gfxr_init_static_palette();

/** @name Resource picture management functions */
/** @{ */

/**
 * Initializes a gfxr_pic_t for a specific mode.
 *
 * This function allocates memory for use by resource drawer functions.
 *
 * @param[in] mode	The specific graphics mode
 * @param[in] ID	The ID to assign to the resulting pixmaps
 * @param[in] sci1	true if a SCI1 pic, false otherwise
 * @return			The allocated pic resource, or NULL on error.
 */
gfxr_pic_t *gfxr_init_pic(gfx_mode_t *mode, int ID, bool sci1);

/**
 * Uninitializes a pic resource.
 *
 * @param[in] pic	The pic to free
 */
void gfxr_free_pic(gfxr_pic_t *pic);

/**
 * Frees all memory associated with a view.
 *
 * @param[in] view	The view to free
 */
void gfxr_free_view(gfxr_view_t *view);
/** @} */
/** @name SCI0 resource picture operations */
/** @{ */

/**
 * Clears all pic buffers of one pic/
 *
 * This function should be called before gfxr_draw_pic0, unless cumulative
 * drawing is intended
 *
 * @param[in] pic			The picture to clear
 * @param[in] titlebar_size	How much space to reserve for the title bar
 */
void gfxr_clear_pic0(gfxr_pic_t *pic, int titlebar_size);

/**
 * Draws a pic resource (all formats prior to SCI1.1).
 *
 * The result is stored in gfxr_visual_map, gfxr_priority_map, and
 * gfxr_control_map. The palette entry of gfxr_visual_map is never used.
 * Note that the picture will not be drawn dithered; use gfxr_dither_pic0
 * for that.
 *
 * @param[in] pic				The pic to draw to
 * @param[in] fill_normally		If 1, the pic is drawn normally; if 0, all
 * 								fill operations will fill with black
 * @param[in] default_palette	The default palette to use for drawing
 * @param[in] size				Resource size
 * @param[in] resource			Pointer to the resource data
 * @param[in] style				The drawing style
 * @param[in] resid				The resource ID
 * @param[in] sci1				true if SCI1, false otherwise
 * @param[in] static_pal		The static palette
 * @param[in] portBounds		The bounds of the port being drawn to
 */
void gfxr_draw_pic01(gfxr_pic_t *pic, int fill_normally,
		int default_palette, int size, byte *resource,
		gfxr_pic0_params_t *style, int resid, int sci1,
		Palette *static_pal, Common::Rect portBounds);

/**
 * Draws a pic resource (SCI1.1).
 *
 * The result is stored in gfxr_visual_map, gfxr_priority_map, and
 * gfxr_control_map. The palette entry of gfxr_visual_map is never used.
 * Note that the picture will not be drawn dithered; use gfxr_dither_pic11
 * for that.
 *
 * @param[in] pic				The pic to draw to
 * @param[in] fill_normally		If 1, the pic is drawn normally; if 0, all
 * 								fill operations will fill with black
 * @param[in] default_palette	The default palette to use for drawing
 * @param[in] size				Resource size
 * @param[in] resource			Pointer to the resource data
 * @param[in] style				The drawing style
 * @param[in] resid				The resource ID
 * @param[in] static_pal		The static palette
 * @param[in] portBounds		Bounds of the port being drawn to
 */
void gfxr_draw_pic11(gfxr_pic_t *pic, int fill_normally,
		int default_palette, int size, byte *resource,
		gfxr_pic0_params_t *style, int resid, Palette *static_pal,
		Common::Rect portBounds);

/**
 * Removes artifacts from a scaled pic.
 *
 * Using information from the (correctly rendered) src pic, this function
 * implements some heuristics to remove artifacts from dest. Must be used
 * before dither_pic0 is called, because it operates on the index buffer.
 *
 * @param[in] dest	The scaled pic
 * @param[in] src	An unscaled pic
 */
void gfxr_remove_artifacts_pic0(gfxr_pic_t *dest, gfxr_pic_t *src);

/**
 * Dithers a gfxr_visual_map.
 *
 * @param[in] pic		The pic to dither
 * @param[in] mode		One of GFXR_DITHER_MODE
 * @param[in] pattern	One of GFXR_DITHER_PATTERN
 */
void gfxr_dither_pic0(gfxr_pic_t *pic, int mode, int pattern);

/**
 * Calculates an EGA view.
 *
 * @param[in] id		Resource ID of the view
 * @param[in] resource	Pointer to the resource to read
 * @param[in] size		Size of the resource
 * @param[in] palette	The palette to use
 * @return				The resulting view
 */
gfxr_view_t *getEGAView(int id, byte *resource, int size, int palette);

/**
 * Calculates a SCI cursor.
 *
 * @param[in] id		The cursor's resource ID
 * @param[in] resource	Pointer to the resource data
 * @param[in] size		Resource size
 * @param[in] isSci01	Set to true to load a SCI1 cursor
 * @return				A newly allocated pixmap containing an index color
 * 						representation of the cursor
 */
gfx_pixmap_t *gfxr_draw_cursor(int id, byte *resource, int size, bool isSci01);
/** @} */


/** @name SCI1/1.1 resource picture operations */
/** @{ */

/**
 * Reads an SCI1 palette.
 *
 * @param[in] id		Resource ID for the palette (or the view it was
 * 						found in)
 * @param[in] resource	Source data
 * @param[in] size		Size of the memory block pointed to by resource
 * @return				Palette with the colors
 */
Palette *gfxr_read_pal1(int id, byte *resource, int size);

/**
 * Reads an SCI1 palette.
 *
 * @param[in] file	Palette file
 * @return			Palette with the colors
 */
Palette *gfxr_read_pal1_amiga(Common::File &file);

/**
 * Reads an SCI1.1 palette.
 *
 * @param[in] id		Resource ID for the palette (or the view it was
 * 						found in)
 * @param[in] resource	Source data
 * @param[in] size		Size of the memory block pointed to by resource
 * @return				Palette with the colors
 */
Palette *gfxr_read_pal11(int id, byte *resource, int size);

/**
 * Calculates a VGA view.
 *
 * @param[in] id			Resource ID of the view
 * @param[in] resource		Pointer to the resource to read
 * @param[in] size			Size of the resource
 * @param[in] static_pal	The static palette
 * @param[in] isSci11		true if SCI1.1, false otherwise
 * @return					The resulting view
 */
gfxr_view_t *getVGAView(int id, byte *resource, int size, Palette *static_pal, bool isSci11);

gfx_pixmap_t *gfxr_draw_cel1(int id, int loop, int cel, int mirrored, byte *resource, byte *cel_base, int size, gfxr_view_t *view, bool isAmiga, bool isSci11);
/** @} */

} // End of namespace Sci

#endif // SCI_GFX_GFX_RESOURCE_H
