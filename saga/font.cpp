/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Font management and font drawing module

#include "reinherit.h"

#include "yslib.h"

#include "rscfile_mod.h"
#include "game_mod.h"

#include "font_mod.h"
#include "font.h"

namespace Saga {

static R_FONT_MODULE FontModule;

int FONT_Init() {
	R_GAME_FONTDESC *gamefonts;
	int i;

	if (FontModule.init) {
		FontModule.err_str = "Font module already initialized.";
		return R_FAILURE;
	}

	// Load font module resource context 
	if (GAME_GetFileContext(&FontModule.font_ctxt,
		R_GAME_RESOURCEFILE, 0) != R_SUCCESS) {
		FontModule.err_str = "Couldn't get resource context.";
		return R_FAILURE;
	}

	// Allocate font table
	GAME_GetFontInfo(&gamefonts, &FontModule.n_fonts);

	assert(FontModule.n_fonts > 0);

	FontModule.fonts = (R_FONT **)malloc(FontModule.n_fonts * sizeof *FontModule.fonts);
	if (FontModule.fonts == NULL) {
		FontModule.err_str = "Memory allocation failure.";
		return R_MEM;
	}

	for (i = 0; i < FontModule.n_fonts; i++) {
		FONT_Load(gamefonts[i].font_rn, gamefonts[i].font_id);
	}

	FontModule.init = 1;

	return R_SUCCESS;
}

int FONT_Shutdown() {
//	int i;

	R_printf(R_STDOUT, "FONT_Shutdown(): Freeing fonts.\n");
/*
	for ( i = 0 ; i < R_FONT_COUNT ; i ++ ) {
		if ( FontModule.fonts[i] != NULL ) {
			if ( FontModule.fonts[i]->normal_loaded ) {
				free( FontModule.fonts[i]->normal->font_free_p );
				free( FontModule.fonts[i]->normal );
			}

			if ( FontModule.fonts[i]->outline_loaded ) {
				free( FontModule.fonts[i]->outline->font_free_p );
				free( FontModule.fonts[i]->outline );
			}
		}

		free( FontModule.fonts[i] );
	}
*/
	return R_SUCCESS;
}

int FONT_Load(uint32 font_rn, int font_id) {
	R_FONT_HEADER fh;
	R_FONT *font;
	R_FONT_STYLE *normal_font;
	byte *fontres_p;
	size_t fontres_len;
	size_t remain;
	const byte *read_p;
	int nbits;
	int c;

	if ((font_id < 0) || (font_id >= FontModule.n_fonts)) {
		return R_FAILURE;
	}

	// Load font resource
	if (RSC_LoadResource(FontModule.font_ctxt, font_rn, &fontres_p, &fontres_len) != R_SUCCESS) {
		FontModule.err_str = "Couldn't load font resource.";
		return R_FAILURE;
	}

	if (fontres_len < R_FONT_DESCSIZE) {
		FontModule.err_str = "Invalid font length.";
	}

	read_p = fontres_p;
	remain = fontres_len;

	// Create new font structure
	font = (R_FONT *)malloc(sizeof *font);
	if (font == NULL) {
		FontModule.err_str = "Memory allocation error.";
		return R_MEM;
	}

	// Read font header
	fh.c_height = ys_read_u16_le(read_p, &read_p);
	fh.c_width = ys_read_u16_le(read_p, &read_p);
	fh.row_length = ys_read_u16_le(read_p, &read_p);

#if R_FONT_DBGLVL >= R_DEBUG_INFO
	R_printf(R_STDOUT, "FONT_Load(): Reading font resource...\n");
#endif

#if R_FONT_DBGLVL >= R_DEBUG_VERBOSE
	R_printf(R_STDOUT, "Character width:\t%d\n", fh.c_width);
	R_printf(R_STDOUT, "Character height:\t%d\n", fh.c_height);
	R_printf(R_STDOUT, "Row padding:\t%d\n", fh.row_length);
#endif

	// Create normal font style
	normal_font = (R_FONT_STYLE *)malloc(sizeof *normal_font);
	if (normal_font == NULL) {
		FontModule.err_str = "Memory allocation error.";
		free(font);
		return R_MEM;
	}

	normal_font->font_free_p = fontres_p;
	normal_font->hdr.c_height = fh.c_height;
	normal_font->hdr.c_width = fh.c_width;
	normal_font->hdr.row_length = fh.row_length;

	for (c = 0; c < R_FONT_CHARCOUNT; c++) {
		normal_font->fce[c].index = ys_read_u16_le(read_p, &read_p);
	}

	for (c = 0; c < R_FONT_CHARCOUNT; c++) {
		nbits = normal_font->fce[c].width = ys_read_u8(read_p, &read_p);
		normal_font->fce[c].byte_width = GetByteLen(nbits);
	}

	for (c = 0; c < R_FONT_CHARCOUNT; c++) {
		normal_font->fce[c].flag = ys_read_u8(read_p, &read_p);
	}

	for (c = 0; c < R_FONT_CHARCOUNT; c++) {
		normal_font->fce[c].tracking = ys_read_u8(read_p, &read_p);
	}

	if ((read_p - fontres_p) != R_FONT_DESCSIZE) {
		R_printf(R_STDERR, "Invalid font resource size.\n");
		return R_FAILURE;
	}

	normal_font->font_p = fontres_p + R_FONT_DESCSIZE;

	font->normal = normal_font;
	font->normal_loaded = 1;

	// Create outline font style
	font->outline = FONT_CreateOutline(normal_font);
	font->outline_loaded = 1;

	// Set font data 
	FontModule.fonts[font_id] = font;

	return R_SUCCESS;
}

int FONT_GetHeight(int font_id) {
	R_FONT *font;

	if (!FontModule.init) {
		return R_FAILURE;
	}

	if ((font_id < 0) || (font_id >= FontModule.n_fonts) || (FontModule.fonts[font_id] == NULL)) {
		FontModule.err_str = "Invalid font id.";
		return R_FAILURE;
	}

	font = FontModule.fonts[font_id];

	return font->normal->hdr.c_height;
}

static R_FONT_STYLE *FONT_CreateOutline(R_FONT_STYLE *src_font) {
	R_FONT_STYLE *new_font;
	unsigned char *new_font_data;
	size_t new_font_data_len;
	int s_width = src_font->hdr.c_width;
	int s_height = src_font->hdr.c_height;
	int new_row_len = 0;
	int row;
	int i;
	int index;
	size_t index_offset = 0;
	int new_byte_width;
	int old_byte_width;
	int current_byte;
	unsigned char *base_ptr;
	unsigned char *src_ptr;
	unsigned char *dest_ptr1;
	unsigned char *dest_ptr2;
	unsigned char *dest_ptr3;
	unsigned char c_rep;

	// Create new font style structure
	new_font = (R_FONT_STYLE *)malloc(sizeof *new_font);

	if (new_font == NULL) {
		FontModule.err_str = "Memory allocation error.";
		return NULL;
	}

	memset(new_font, 0, sizeof *new_font);

	// Populate new font style character data 
	for (i = 0; i < R_FONT_CHARCOUNT; i++) {
		new_byte_width = 0;
		old_byte_width = 0;
		index = src_font->fce[i].index;
		if ((index > 0) || (i == R_FONT_FIRSTCHAR)) {
			index += index_offset;
		}

		new_font->fce[i].index = index;
		new_font->fce[i].tracking = src_font->fce[i].tracking;
		new_font->fce[i].flag = src_font->fce[i].flag;

		if (src_font->fce[i].width != 0) {
			new_byte_width = GetByteLen(src_font->fce[i].width + 2);
			old_byte_width = GetByteLen(src_font->fce[i].width);

			if (new_byte_width > old_byte_width) {
				index_offset++;
			}
		}

		new_font->fce[i].width = src_font->fce[i].width + 2;
		new_font->fce[i].byte_width = new_byte_width;
		new_row_len += new_byte_width;
	}

#if R_FONT_DBGLVL >= R_DEBUG_VERBOSE
	R_printf(R_STDOUT, "New row length: %d\n", new_row_len);
#endif

	new_font->hdr.c_width = s_width + 2;
	new_font->hdr.c_height = s_height + 2;
	new_font->hdr.row_length = new_row_len;

	// Allocate new font representation storage 
	new_font_data_len = new_row_len * (s_height + 2);
	new_font_data = (unsigned char *)malloc(new_font_data_len);

	if (new_font_data == NULL) {
		FontModule.err_str = "Memory allocation error.";
		return NULL;
	}

	memset(new_font_data, 0, new_font_data_len);

	new_font->font_free_p = new_font_data;
	new_font->font_p = new_font_data;

	// Generate outline font representation
	for (i = 0; i < R_FONT_CHARCOUNT; i++) {
		for (row = 0; row < s_height; row++) {
			for (current_byte = 0; current_byte < new_font->fce[i].byte_width; current_byte++) {
				base_ptr = new_font->font_p + new_font->fce[i].index + current_byte;
				dest_ptr1 = base_ptr + new_font->hdr.row_length * row;
				dest_ptr2 = base_ptr + new_font->hdr.row_length * (row + 1);
				dest_ptr3 = base_ptr + new_font->hdr.row_length * (row + 2);
				if (current_byte > 0) {
					// Get last two columns from previous byte
					src_ptr = src_font->font_p + src_font->hdr.row_length * row + src_font->fce[i].index +
								(current_byte - 1);
					c_rep = *src_ptr;
					*dest_ptr1 |= ((c_rep << 6) | (c_rep << 7));
					*dest_ptr2 |= ((c_rep << 6) | (c_rep << 7));
					*dest_ptr3 |= ((c_rep << 6) | (c_rep << 7));
				}

				if (current_byte < src_font->fce[i].byte_width) {
					src_ptr = src_font->font_p + src_font->hdr.row_length * row + src_font->fce[i].index +
								current_byte;
					c_rep = *src_ptr;
					*dest_ptr1 |= c_rep | (c_rep >> 1) | (c_rep >> 2);
					*dest_ptr2 |= c_rep | (c_rep >> 1) | (c_rep >> 2);
					*dest_ptr3 |= c_rep | (c_rep >> 1) | (c_rep >> 2);
				}
			}
		}

		// "Hollow out" character to prevent overdraw
		for (row = 0; row < s_height; row++) {
			for (current_byte = 0; current_byte < new_font->fce[i].byte_width; current_byte++) {
				dest_ptr2 = new_font->font_p +  new_font->hdr.row_length * (row + 1) + new_font->fce[i].index + current_byte;
				if (current_byte > 0) {
					// Get last two columns from previous byte
					src_ptr = src_font->font_p + src_font->hdr.row_length * row + src_font->fce[i].index +
								(current_byte - 1);
					*dest_ptr2 &= ((*src_ptr << 7) ^ 0xFFU);
				}

				if (current_byte < src_font->fce[i].byte_width) {
					src_ptr = src_font->font_p + src_font->hdr.row_length * row + src_font->fce[i].index +
								current_byte;
					*dest_ptr2 &= ((*src_ptr >> 1) ^ 0xFFU);
				}
			}
		}
	}

	return new_font;
}

static int GetByteLen(int num_bits) {
	int byte_len;
	byte_len = num_bits / 8;

	if (num_bits % 8) {
		byte_len++;
	}

	return byte_len;
}

// Returns the horizontal length in pixels of the graphical representation
// of at most 'test_str_ct' characters of the string 'test_str', taking
// into account any formatting options specified by 'flags'.
// If 'test_str_ct' is 0, all characters of 'test_str' are counted.
int FONT_GetStringWidth(int font_id, const char *test_str, size_t test_str_ct, int flags) {
	R_FONT *font;
	size_t ct;
	int width = 0;
	int ch;
	const byte *txt_p;

	if (!FontModule.init) {
		return R_FAILURE;
	}

	if ((font_id < 0) || (font_id >= FontModule.n_fonts) || (FontModule.fonts[font_id] == NULL)) {
		FontModule.err_str = "Invalid font id.";
		return R_FAILURE;
	}

	font = FontModule.fonts[font_id];
	assert(font != NULL);

	txt_p = (const byte *) test_str;

	for (ct = test_str_ct; *txt_p && (!test_str_ct || ct > 0); txt_p++, ct--) {
		ch = *txt_p & 0xFFU;
		// Translate character
		ch = CharMap[ch];
		assert(ch < R_FONT_CHARCOUNT);
		width += font->normal->fce[ch].tracking;
	}

	if ((flags & FONT_BOLD) || (flags & FONT_OUTLINE)) {
		width += 1;
	}

	return width;
}

int FONT_Draw(int font_id, R_SURFACE *ds, const char *draw_str, size_t draw_str_ct,
			int text_x, int text_y, int color, int effect_color, int flags) {
	R_FONT *font;

	if (!FontModule.init) {
		FontModule.err_str = "Font Module not initialized.";

		return R_FAILURE;
	}

	if ((font_id < 0) || (font_id >= FontModule.n_fonts) || (FontModule.fonts[font_id] == NULL)) {
		FontModule.err_str = "Invalid font id.";
		return R_FAILURE;
	}

	font = FontModule.fonts[font_id];

	if (flags & FONT_OUTLINE) { 
		FONT_Out(font->outline, ds, draw_str, draw_str_ct, text_x - 1, text_y - 1, effect_color);
		FONT_Out(font->normal, ds, draw_str, draw_str_ct, text_x, text_y, color);
	} else if (flags & FONT_SHADOW) {
		FONT_Out(font->normal, ds, draw_str, draw_str_ct, text_x - 1, text_y + 1, effect_color);
		FONT_Out(font->normal, ds, draw_str, draw_str_ct, text_x, text_y, color);
	} else { // FONT_NORMAL
		FONT_Out(font->normal, ds, draw_str, draw_str_ct, text_x, text_y, color);
	}

	return R_SUCCESS;
}

int FONT_Out(R_FONT_STYLE * draw_font, R_SURFACE * ds, const char *draw_str, size_t draw_str_ct,
				int text_x, int text_y, int color) {
	const byte *draw_str_p;
	byte *c_data_ptr;
	int c_code;
	int char_row;

	byte *output_ptr;
	byte *output_ptr_min;
	byte *output_ptr_max;

	int row;
	int row_limit;

	int c_byte_len;
	int c_byte;
	int c_bit;
	int ct;

	if ((text_x > ds->buf_w) || (text_y > ds->buf_h)) {
		// Output string can't be visible
		return R_SUCCESS;
	}

	draw_str_p = (const byte *) draw_str;
	ct = draw_str_ct;

	// Draw string one character at a time, maximum of 'draw_str'_ct 
	// characters, or no limit if 'draw_str_ct' is 0
	for (; *draw_str_p && (!draw_str_ct || ct); draw_str_p++, ct--) {
		c_code = *draw_str_p & 0xFFU;

		// Translate character
		c_code = CharMap[c_code];
		assert(c_code < R_FONT_CHARCOUNT);

		// Check if character is defined
		if ((draw_font->fce[c_code].index == 0) && (c_code != R_FONT_FIRSTCHAR)) {
#if R_FONT_SHOWUNDEFINED
			if (c_code == R_FONT_CH_SPACE) {
				text_x += draw_font->fce[c_code].tracking;
				continue;
			}
			c_code = R_FONT_CH_QMARK;
#else
			// Character code is not defined, but advance tracking
			// ( Not defined if offset is 0, except for 33 ('!') which
			//   is defined )
			text_x += draw_font->fce[c_code].tracking;
			continue;
#endif
		}

		// Get length of character in bytes
		c_byte_len = ((draw_font->fce[c_code].width - 1) / 8) + 1;
		row_limit = (ds->buf_h < (text_y + draw_font->hdr.c_height)) ? ds->buf_h : text_y + draw_font->hdr.c_height;
		char_row = 0;

		for (row = text_y; row < row_limit; row++, char_row++) {
			// Clip negative rows */
			if (row < 0) {
				continue;
			}

			output_ptr = ds->buf + (ds->buf_pitch * row) + text_x;
			output_ptr_min = ds->buf + (ds->buf_pitch * row) + (text_x > 0 ? text_x : 0);
			output_ptr_max = output_ptr + (ds->buf_pitch - text_x);

			// If character starts off the screen, jump to next character
			if (output_ptr < output_ptr_min) {
				break;
			}

			c_data_ptr = draw_font->font_p + char_row * draw_font->hdr.row_length + draw_font->fce[c_code].index;

			for (c_byte = 0; c_byte < c_byte_len; c_byte++, c_data_ptr++) {
				// Check each bit, draw pixel if bit is set
				for (c_bit = 7; c_bit >= 0 && (output_ptr < output_ptr_max); c_bit--) {
					if ((*c_data_ptr >> c_bit) & 0x01) {
						*output_ptr = (byte) color;
					}
					output_ptr++;
				} // end per-bit processing
			} // end per-byte processing
		} // end per-row processing

		// Advance tracking position
		text_x += draw_font->fce[c_code].tracking;
	} // end per-character processing

	return R_SUCCESS;
}

} // End of namespace Saga
