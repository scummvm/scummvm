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

#ifndef SCI_GFX_FONT_H
#define SCI_GFX_FONT_H

#include "common/scummsys.h"


namespace Sci {
/** @name Font operations and stuctures */
/** @{ */

struct TextFragment {
	const char *offset;
	int length;

	TextFragment() : offset(0), length(0) {}
	TextFragment(const char *o) : offset(o), length(0) {}
};

/**
 * Bitmap font information.
 */
struct gfx_bitmap_font_t {
	int ID; 			/**< Unique resource ID */
	int chars_nr;		/**< Numer of available characters */
	int *widths;		/**< chars_nr character widths, in pixels */
	int row_size;		/**
						 * Byte size of each pixel row. For unscaled fonts,
						 * this is always 1, 2, or 4. Otherwise, it's a
						 * multiple of 4.
						 */
	int line_height;	/**
						 * Height of each text line (usually identical to
						 * height)
						 */
	int height;			/**< Height for all characters, in pixel rows */
	int char_size;		/**
						  * Amount of memory occupied by one character
						  * in data
						  */
	byte *data;			/**
						  * Font data, consisting of 'chars_nr' entries
						  * of 'height' rows of 'row_size' bytes. For each
						  * character ch, its first byte (the topmost row)
						  * is located at (data + (charsize * ch)), and its
						  * pixel width is widths[ch], provided that
						  * (ch < chars_nr).
						  */
};

/**
 * Font handling flags.
 *
 * SCI0, SCI01 and SCI1 all use the same font format.
 */
enum fontFlags {
	kFontCountWhitespace = 1 << 0,    //!< In SQ3, whitespace is included in text size
	kFontNoNewlines      = 1 << 1,    //!< Don't treat newline characters
	kFontIgnoreLF        = 1 << 2     //!< Interpret CR LF sequences as a single newline, rather than two
};

/**
 * Generates a bitmap font data structure from a resource.
 *
 * @param[in] id		Resource ID of the resulting font
 * @param[in] resource	Pointer to the resource data
 * @param[in] size		Size of the resource block
 * @return				The resulting font structure, or NULL on error
 */
gfx_bitmap_font_t *gfxr_read_font(int id, byte *resource, int size);

/**
 * Frees a previously allocated font structure.
 *
 * @param font	The font to free
 */
void gfxr_free_font(gfx_bitmap_font_t *font);

/**
 * Calculates the size that would be occupied by drawing a specified
 * text.
 *
 * This function assumes 320x200 mode.
 *
 * @param[in]  font			The font to calculate with
 * @param[in]  max_width	Maximum pixel width allowed for the output
 * @param[in]  text			The text to calculate for
 * @param[in]  flags		Any text formatting flags
 * @param[out] fragments	A newly allocated array of text_fragments,
 * 							containing the start and size of each string
 * 							segment.
 * @param[out] width		The resulting width
 * @param[out] height		The resulting height
 * @param[out] line_height	Pixel height of a single line of text
 * @param[out] last_offset	Pixel offset after the last drawn line
 * @return					true if successful, false otherwise
 */
bool gfxr_font_calculate_size(Common::Array<TextFragment> &fragments,
    gfx_bitmap_font_t *font, int max_width, const char *text,
	int *width, int *height, int *line_height, int *last_offset, int flags);

/**
 * Draws text in a specific font to a pixmap.
 *
 * The results are written to the pixmap's index buffer. Contents of the
 * foreground and background fields are copied into a newly allocated font
 * structure, so that the pixmap may be translated directly. If any of the
 * colors is null, it will be assumed to be transparent.
 * In color index mode, the specified colors have to be preallocated.
 *
 * @param[in] font			The font to use for drawing
 * @param[in] text			The start of the text to draw
 * @param[in] characters	The number of characters to draw
 * @param[in] fg0			The first foreground color
 * @param[in] fg1			The second foreground color
 * @param[in] bg			The background color
 * @return					The result pixmap, or NULL on error
 */
gfx_pixmap_t *gfxr_draw_font(gfx_bitmap_font_t *font, const char *text,
		int characters, PaletteEntry *fg0, PaletteEntry *fg1,
		PaletteEntry *bg);
/** @} */

} // End of namespace Sci

#endif // SCI_GFX_FONT_H
