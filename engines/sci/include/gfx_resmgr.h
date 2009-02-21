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

#ifndef _GFX_RESMGR_H_
#define _GFX_RESMGR_H_

// FIXME/TODO: The name "(Graphics) resource manager", and the associated
// filenames, are misleading. This should be renamed to "Graphics manager"
// or something like that.

#include "sci/include/gfx_resource.h"
#include "sci/include/sbtree.h"

namespace Sci {

typedef enum {
	GFX_RESOURCE_TYPE_VIEW = 0,
	GFX_RESOURCE_TYPE_PIC,
	GFX_RESOURCE_TYPE_FONT,
	GFX_RESOURCE_TYPE_CURSOR,
	GFX_RESOURCE_TYPE_PALETTE,
	/* FIXME: Add PAL resource */

	GFX_RESOURCE_TYPES_NR /* Number of resource types that are to be supported */
} gfx_resource_type_t;

#define GFX_RESOURCE_TYPE_0 GFX_RESOURCE_TYPE_VIEW

#define GFXR_RES_ID(type, index) ((type) << 16 | (index))
#define GFXR_RES_TYPE(id) (id >> 16)
#define GFXR_RES_NR(id) (id & 0xffff)


typedef struct gfx_resource_struct {
	int ID; /* Resource ID */
	int lock_sequence_nr; /* See description of lock_counter in gfx_resstate_t */
	int mode; /* A mode type hash */

	union {
		gfx_pixmap_t *pointer;
		gfxr_view_t *view;
		gfx_bitmap_font_t *font;
		gfxr_pic_t *pic;
	} scaled_data;

	union {
		gfx_pixmap_t *pointer;
		gfxr_view_t *view;
		gfx_bitmap_font_t *font;
		gfxr_pic_t *pic;
	} unscaled_data;

} gfx_resource_t;


struct _gfx_options;

typedef struct {
	int version; /* Interpreter version */
	struct _gfx_options *options;
	gfx_driver_t *driver;
	gfx_pixmap_color_t *static_palette;
	int static_palette_entries;
	int lock_counter; /* Global lock counter; increased for each new resource allocated.
			  ** The newly allocated resource will then be assigned the new value
			  ** of the lock_counter, as will any resources referenced afterwards.
			  */
	int tag_lock_counter; /* lock counter value at tag time */

	sbtree_t *resource_trees[GFX_RESOURCE_TYPES_NR];
	void *misc_payload;
} gfx_resstate_t;



gfx_resstate_t *
gfxr_new_resource_manager(int version, struct _gfx_options *options,
                          gfx_driver_t *driver, void *misc_payload);
/* Allocates and initializes a new resource manager
** Parameters: (int) version: Interpreter version
**             (gfx_options_t *): Pointer to all relevant drawing options
**             (gfx_driver_t *): The graphics driver (needed for capability flags and the mode
**                               structure)
**             (void *) misc_payload: Additional information for the interpreter's
**                      resource loaders
** Returns   : (gfx_resstate_t *): A newly allocated resource manager
** The options are considered to be read-only, as they belong to the overlying state object.
*/

void
gfxr_free_resource_manager(gfx_driver_t *driver, gfx_resstate_t *state);
/* Frees a previously allocated resource manager, and all allocated resources.
** Parameters: (gfx_driver_t *) driver: The graphics driver; used to free pixmaps that
**                                      are installed in a driver-specific registry
**             (gfx_resstate_t *) state: The state manager to free
** Return    : (void)
*/

void
gfxr_free_all_resources(gfx_driver_t *driver, gfx_resstate_t *state);
/* Frees all resources currently allocated
** Parameter: (gfx_driver_t *) driver: The driver to free with
**            (gfx_resstate_t *) state: The state to do this on
** Returns  : (void)
** This function is intended to be used primarily for debugging.
*/

void
gfxr_tag_resources(gfx_resstate_t *state);
/* 'Tags' all resources for deletion
** Paramters: (gfx_resstate_t *) state: The resource state to modify
** Returns  : (void)
** Tagged resources are untagged if they are referenced.
*/

void
gfxr_free_tagged_resources(gfx_driver_t *driver, gfx_resstate_t *state);
/* Frees all tagged resources.
** Parameters: (gfx_driver_t *) driver: The graphics driver the pixmaps are potentially
**                                      registered in
**             (gfx_resstate_t *) state: The state to alter
** Returns   : (void)
** Resources are tagged by calling gfx_tag_resources(), and untagged by calling the
** approprate dereferenciation function.
** Note that this function currently only affects view resources, as pic resources are
** treated differently, while font and cursor resources are relatively rare.
*/


gfxr_pic_t *
gfxr_get_pic(gfx_resstate_t *state, int nr, int maps, int flags,
             int default_palette, int scaled);
/* Retreives a displayable (translated) pic resource
** Parameters: (gfx_resstate_t *) state: The resource state
**             (int) nr: Number of the pic resource
**             (int) maps: The maps to translate (ORred GFX_MASK_*)
**             (int) flags: Interpreter-dependant pic flags
**             (int) default_palette: The default palette to use for drawing (if applicable)
**             (int) scaled: Whether to return the scaled maps, or the unscaled
**                           ones (which may be identical) for some special operations.
** Returns   : (gfx_pic_t *) The appropriate pic resource with all maps as index (but not
**                           neccessarily translated) data.
*/

gfxr_pic_t *
gfxr_add_to_pic(gfx_resstate_t *state, int old_nr, int new_nr, int maps, int flags,
                int old_default_palette, int default_palette, int scaled);
/* Retreives a displayable (translated) pic resource written ontop of an existing pic
** Parameters: (gfx_resstate_t *) state: The resource state
**             (int) old_nr: Number of the pic resource to write on
**             (int) new_nr: Number of the pic resource that is to be added
**             (int) maps: The maps to translate (ORred GFX_MASK_*)
**             (int) flags: Interpreter-dependant pic flags
**             (int) default_palette: The default palette to use for drawing (if applicable)
**             (int) scaled: Whether to return the scaled maps, or the unscaled
**                           ones (which may be identical) for some special operations.
** Returns   : (gfx_pic_t *) The appropriate pic resource with all maps as index (but not
**                           neccessarily translated) data.
** This function invalidates the cached pic pointed to by old_nr in the cache. While subsequent
** gfxr_add_to_pic() writes will still modify the 'invalidated' pic, gfxr_get_pic() operations will
** cause it to be removed from the cache and to be replaced by a clean version.
*/

gfxr_view_t *
gfxr_get_view(gfx_resstate_t *state, int nr, int *loop, int *cel, int palette);
/* Retreives a translated view cel
** Parameters: (gfx_resstate_t *) state: The resource state
**             (int) nr: The view number
**             (int *) loop: Pointer to a variable containing the loop number
**             (int *) cel: Pointer to a variable containing the cel number
**	       (int) palette: The palette to use
** Returns   : (gfx_view_t *) The relevant view, or NULL if nr was invalid
** loop and cel are given as pointers in order to allow the underlying variables to be
** modified if they are invalid (this is relevant for SCI version 0, where invalid
** loop and cel numbers have to be interpreted as 'maximum' or 'minimum' by the interpreter)
*/

gfx_bitmap_font_t *
gfxr_get_font(gfx_resstate_t *state, int nr, int scaled);
/* Retreives a font
** Parameters: (gfx_resstate_t *) state: The relevant resource state
**             (int) nr: The font number
**             (int) scaled: Whether the font should be font-scaled
** Returns   : (gfx_font_t *) The appropriate font, or NULL on error
*/

gfx_pixmap_t *
gfxr_get_cursor(gfx_resstate_t *state, int nr);
/* Retreives an SCI0/SCI01 mouse cursor
** Parameters: (gfx_resstate_t *) state: The resource state
**             (int) nr: The cursour number
** Returns   : (gfx_font_t *) The approprate cursor as a pixmap, or NULL on error
*/

gfx_pixmap_color_t *
gfxr_get_palette(gfx_resstate_t *state, int nr);
/* Retreives a palette
** Parameters: (gfx_resstate_t *) state: The resource state
**             (int) nr: The cursour number
** Returns   : (gfx_font_t *) The approprate cursor as a pixmap, or NULL on error
*/


/* =========================== */
/* Interpreter-dependant stuff */
/* =========================== */


int
gfxr_interpreter_options_hash(gfx_resource_type_t type, int version,
                              struct _gfx_options *options, void *internal, int palette);
/* Calculates a unique hash value for the specified options/type setup
** Parameters: (gfx_resource_type_t) type: The type the hash is to be generated for
**             (int) version: The interpreter type and version
**             (gfx_options_t *) options: The options to hashify
**             (void *) internal: Internal information provided by the interpreter
**	       (int) palette: The palette to use (FIXME: should this be here?)
** Returns   : (int) A hash over the values of the options entries, covering entries iff
**                   they are relevant for the specified type
** Covering more entries than relevant may slow down the system when options are changed,
** while covering less may result in invalid cached data being used.
** Only positive values may be returned, as negative values are used internally by the generic
** resource manager code.
** Also, only the lower 20 bits are available to the interpreter.
** (Yes, this isn't really a "hash" in the traditional sense...)
*/

int *
gfxr_interpreter_get_resources(gfx_resstate_t *state, gfx_resource_type_t type,
                               int version, int *entries_nr, void *internal);
/* Retreives all resources of a specified type that are available from the interpreter
** Parameters: (gfx_resstate_t *) state: The relevant resource state
**             (gfx_respirce_type_t) type: The resource type to query
**             (int) version: The interpreter type and version
**             (int *) entries_nr: The variable the number of entries will eventually be stored in
**             (void *) internal: Internal information provided by the interpreter
** Returns   : (int *) An array of resource numbers
** Unsupported/non-existing resources should return NULL here; this is equivalent to supported
** resources of which zero are available.
** The returned structure (if non-zero) must be freed by the querying code (the resource manager).
*/

gfxr_pic_t *
gfxr_interpreter_init_pic(int version, gfx_mode_t *mode, int ID, void *internal);
/* Initializes a pic
** Parameters: (int) version: Interpreter version to use
**             (gfx_mode_t *) mode: The graphics mode the pic will be using
**             (int) ID: The ID to assign to the gfxr_pic_t structure
**             (void *) internal: Internal information provided by the interpreter
** Returns   : (gfxr_pic_t *) A newly allocated pic
** This function is typically called befode gfxr_interpreter_clear_pic().
** Must remember to initialize 'internal' to NULL or a malloc()'d area.
*/

void
gfxr_interpreter_clear_pic(int version, gfxr_pic_t *pic, void *internal);
/* Clears a previously allocated pic
** Parameters: (int) version: Interpreter version
**             (gfxr_pic_t *) pic: The pic to clear
**             (void *) internal: Internal information provided by the interpreter
** Returns  :  (void)
** This function is called in preparation for the pic to be drawn with gfxr_interpreter_calculate_pic.
*/

int
gfxr_interpreter_calculate_pic(gfx_resstate_t *state, gfxr_pic_t *scaled_pic, gfxr_pic_t *unscaled_pic,
                               int flags, int default_palette, int nr, void *internal);
/* Instructs the interpreter-specific code to calculate a picture
** Parameters: (gfx_resstate_t *) state: The resource state, containing options and version information
**             (gfxr_pic_t *) scaled_pic: The pic structure that is to be written to
**             (gfxr_pic_t *) unscaled_pic: The pic structure the unscaled pic is to be written to,
**                                          or NULL if it isn't needed.
**             (int) flags: Pic drawing flags (interpreter dependant)
**             (int) default_palette: The default palette to use for pic drawing (interpreter dependant)
**             (int) nr: pic resource number
**             (void *) internal: Internal information provided by the interpreter
** Returns   : (int) GFX_ERROR if the resource could not be found, GFX_OK otherwise
*/

gfxr_view_t *
gfxr_interpreter_get_view(gfx_resstate_t *state, int nr, void *internal, int palette);
/* Instructs the interpreter-specific code to calculate a view
** Parameters: (gfx_resstate_t *) state: The resource manager state
**             (int) nr: The view resource number
**             (void *) internal: Internal information provided by the interpreter
** Returns   : (gfx_view_t *) The appropriate view, or NULL on error
*/

gfx_bitmap_font_t *
gfxr_interpreter_get_font(gfx_resstate_t *state, int nr, void *internal);
/* Instructs the interpreter-specific code to calculate a font
** Parameters: (gfx_resstate_t *) state: The resource manager state
**             (int) nr: The font resource number
**             (void *) internal: Internal information provided by the interpreter
** Returns   : (gfx_font_t *) The newly calculated font, or NULL on error
*/

gfx_pixmap_t *
gfxr_interpreter_get_cursor(gfx_resstate_t *state, int nr, void *internal);
/* Instructs the interpreter-specific code to calculate a cursor
** Paramaters: (gfx_resstate_t *) state: The resource manager state
**             (int nr): The cursor resource number
**             (void *) internal: Internal information provided by the interpreter
** Returns   : (gfx_pixmap_t *) The cursor pixmap, or NULL on error
*/

gfx_pixmap_color_t *
gfxr_interpreter_get_static_palette(gfx_resstate_t *state, int version, int *colors_nr, void *internal);
/* Retreives the static palette from the interpreter-specific code
** Parameters: (int) version: Interpreter version to use
**             (int *) colors_nr: Number of colors to use
**             (void *) internal: Internal information provided by the interpreter
** Returns   : (gfx_pixmap_color_t *) *colors_nr static color entries
**             if a static palette must be used, NULL otherwise
*/

gfx_pixmap_color_t *
gfxr_interpreter_get_palette(gfx_resstate_t *state, int version, int *colors_nr, void *internal, int nr);
/* Retreives the static palette from the interpreter-specific code
** Parameters: (int) version: Interpreter version to use
**             (int *) colors_nr: Number of colors to use
**             (void *) internal: Internal information provided by the interpreter
** Returns   : (gfx_pixmap_color_t *) *colors_nr static color entries
**             if a static palette must be used, NULL otherwise
*/

int
gfxr_interpreter_needs_multicolored_pointers(int version, void *internal);
/* Determines whether support for pointers with more than two colors is required
** Parameters: (int) version: Interpreter version to test for
**             (void *) internal: Internal information provided by the interpreter
** Returns   : (int) 0 if no support for multi-colored pointers is required, non-0
**                   otherwise
*/

} // End of namespace Sci

#endif /* !_GFX_RSMGR_H_ */
