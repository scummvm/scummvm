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

#ifndef SCI_GFX_GFX_RESMGR_H
#define SCI_GFX_GFX_RESMGR_H

// FIXME/TODO: The name "(Graphics) resource manager", and the associated
// filenames, are misleading. This should be renamed to "Graphics manager"
// or something like that.

#include "sci/gfx/gfx_resource.h"
#include "sci/resource.h"

#include "common/hashmap.h"
#include "common/rect.h"

namespace Sci {

struct gfx_bitmap_font_t;
class ResourceManager;

enum gfx_resource_type_t {
	GFX_RESOURCE_TYPE_VIEW = 0,
	GFX_RESOURCE_TYPE_PIC,
	GFX_RESOURCE_TYPE_FONT,
	GFX_RESOURCE_TYPE_CURSOR,
	GFX_RESOURCE_TYPE_PALETTE,
	/* FIXME: Add PAL resource */

	GFX_RESOURCE_TYPES_NR /* Number of resource types that are to be supported */
};

#define GFX_RESOURCE_TYPE_0 GFX_RESOURCE_TYPE_VIEW

#define GFXR_RES_ID(type, index) ((type) << 16 | (index))
#define GFXR_RES_TYPE(id) (id >> 16)
#define GFXR_RES_NR(id) (id & 0xffff)


struct gfx_resource_t {
	int ID; /* Resource ID */
	int lock_sequence_nr; /* See description of lock_counter in GfxResManager */
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

};


struct gfx_options_t;

typedef Common::HashMap<int, gfx_resource_t *> IntResMap;


class GfxResManager {
public:
	GfxResManager(int version, bool isVGA, gfx_options_t *options, GfxDriver *driver, ResourceManager *resManager);

	~GfxResManager();

	/* Calculates a unique hash value for the specified options/type setup
	** Parameters: (gfx_resource_type_t) type: The type the hash is to be generated for
	** Returns   : (int) A hash over the values of the options entries, covering entries iff
	**                   they are relevant for the specified type
	** Covering more entries than relevant may slow down the system when options are changed,
	** while covering less may result in invalid cached data being used.
	** Only positive values may be returned, as negative values are used internally by the generic
	** resource manager code.
	** Also, only the lower 20 bits are available to the interpreter.
	** (Yes, this isn't really a "hash" in the traditional sense...)
	*/
	int getOptionsHash(gfx_resource_type_t type);


	/* 'Tags' all resources for deletion
	** Paramters: (void)
	** Returns  : (void)
	** Tagged resources are untagged if they are referenced.
	*/
	void tagResources() { _tagLockCounter++; }


	/* Retrieves an SCI0/SCI01 mouse cursor
	** Parameters: (int) num: The cursor number
	** Returns   : (gfx_font_t *) The approprate cursor as a pixmap, or NULL on error
	*/
	gfx_pixmap_t *getCursor(int num);


	/* Retrieves the static palette from the interpreter-specific code
	** Parameters: (int *) colors_nr: Number of colors to use
	**             (int) nr: The palette to read
	** Returns   : (Palette *) static palette
	**             if a static palette must be used, NULL otherwise
	*/
	Palette *getPalette(int *colors_nr, int num = 999);


	/* Retrieves a font
	** Parameters: (int) nr: The font number
	**             (int) scaled: Whether the font should be font-scaled
	** Returns   : (gfx_font_t *) The appropriate font, or NULL on error
	*/
	gfx_bitmap_font_t *getFont(int num, bool scaled = false);


	/* Retrieves a translated view cel
	** Parameters:
	**             (int) nr: The view number
	**             (int *) loop: Pointer to a variable containing the loop number
	**             (int *) cel: Pointer to a variable containing the cel number
	**	           (int) palette: The palette to use
	** Returns   : (gfx_view_t *) The relevant view, or NULL if nr was invalid
	** loop and cel are given as pointers in order to allow the underlying variables to be
	** modified if they are invalid (this is relevant for SCI version 0, where invalid
	** loop and cel numbers have to be interpreted as 'maximum' or 'minimum' by the interpreter)
	*/
	gfxr_view_t *getView(int nr, int *loop, int *cel, int palette);


	/* Retrieves a displayable (translated) pic resource
	** Parameters: (int) nr: Number of the pic resource
	**             (int) maps: The maps to translate (ORred GFX_MASK_*)
	**             (int) flags: Interpreter-dependant pic flags
	**             (int) default_palette: The default palette to use for drawing (if applicable)
	**             (bool) scaled: Whether to return the scaled maps, or the unscaled
	**                           ones (which may be identical) for some special operations.
	** Returns   : (gfxr_pic_t *) The appropriate pic resource with all maps as index (but not
	**                           neccessarily translated) data.
	*/
	gfxr_pic_t *getPic(int num, int maps, int flags, int default_palette, bool scaled = false);


	/* Retrieves a displayable (translated) pic resource written ontop of an existing pic
	** Parameters: (int) old_nr: Number of the pic resource to write on
	**             (int) new_nr: Number of the pic resource that is to be added
	**             (int) flags: Interpreter-dependant pic flags
	**             (int) default_palette: The default palette to use for drawing (if applicable)
	**             (int) scaled: Whether to return the scaled maps, or the unscaled
	**                           ones (which may be identical) for some special operations.
	** Returns   : (gfxr_pic_t *) The appropriate pic resource with all maps as index (but not
	**                           neccessarily translated) data.
	** This function invalidates the cached pic pointed to by old_nr in the cache. While subsequent
	** addToPic() writes will still modify the 'invalidated' pic, gfxr_get_pic() operations will
	** cause it to be removed from the cache and to be replaced by a clean version.
	*/
	gfxr_pic_t *addToPic(int old_nr, int new_nr, int flags, int old_default_palette, int default_palette);

	/* Calculate a picture
	** Parameters: (gfxr_pic_t *) scaled_pic: The pic structure that is to be written to
	**             (gfxr_pic_t *) unscaled_pic: The pic structure the unscaled pic is to be written to,
	**                                          or NULL if it isn't needed.
	**             (int) flags: Pic drawing flags (interpreter dependant)
	**             (int) default_palette: The default palette to use for pic drawing (interpreter dependant)
	**             (int) nr: pic resource number
	** Returns   : (int) GFX_ERROR if the resource could not be found, GFX_OK otherwise
	*/
	int calculatePic(gfxr_pic_t *scaled_pic, gfxr_pic_t *unscaled_pic, int flags, int default_palette, int nr);

	/* Determines whether support for pointers with more than two colors is required
	** Returns   : (bool) false if no support for multi-colored pointers is required, true
	**                   otherwise
	*/
	bool multicoloredPointers() { return _version > SCI_VERSION_1; }


	/* Frees all resources currently allocated
	** Parameter: (void)
	** Returns  : (void)
	** This function is intended to be used primarily for debugging.
	*/
	void freeAllResources();


	/* Frees all tagged resources.
	** Parameters: (void)
	** Returns   : (void)
	** Resources are tagged by calling gfx_tag_resources(), and untagged by calling the
	** approprate dereferenciation function.
	** Note that this function currently only affects view resources, as pic resources are
	** treated differently, while font and cursor resources are relatively rare.
	*/
	void freeTaggedResources();


	/* Frees a previously allocated resource manager, and all allocated resources.
	** Parameters: (void)
	** Return    : (void)
	*/
	void freeResManager();

	const PaletteEntry &getColor(int color) { return _staticPalette->getColor(color); }

	// Set static palette and merge it into the global palette
	void setStaticPalette(Palette *newPalette);

	/*
	** Sets the picture port bounds
	*/
	void changePortBounds(int x1, int y1, int x2, int y2) {
		_portBounds = Common::Rect(x1, y1, x2, y2);
	}

#if 0
	void setPaletteIntensity(int16 from, int16 to, int16 intensity) {
		Palette *pal = _staticPalette->getref();

		for (uint16 i = 0; i < _driver->getMode()->palette->size(); i++) {
			byte r = pal->getColor(i).r * intensity / 100;
			byte g = pal->getColor(i).g * intensity / 100;
			byte b = pal->getColor(i).b * intensity / 100;
			pal->makeSystemColor(i, PaletteEntry(r, g, b));
		}
		pal->mergeInto(_driver->getMode()->palette);
		_driver->install_palette(_driver, pal);
		pal->unmerge();
		pal->free();
	}
#endif

	int getColorCount() { return _staticPalette ? _staticPalette->size() : 0; }

private:
	int _version;
	bool _isVGA;
	gfx_options_t *_options;
	GfxDriver *_driver;
	Palette *_staticPalette;
	int _lockCounter; /* Global lock counter; increased for each new resource allocated.
			  ** The newly allocated resource will then be assigned the new value
			  ** of the lock_counter, as will any resources referenced afterwards.
			  */
	int _tagLockCounter; /* lock counter value at tag time */
	Common::Rect _portBounds;

	IntResMap _resourceMaps[GFX_RESOURCE_TYPES_NR];
	ResourceManager *_resManager;
};

} // End of namespace Sci

#endif // SCI_GFX_GFX_RSMGR_H
