/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/rscfile_mod.h"

#include "saga/font.h"
#include "saga/stream.h"

namespace Saga {

Font::Font(SagaEngine *vm) : _vm(vm), _initialized(false) {
	int i;

	// Load font module resource context 
	_fontContext = _vm->getFileContext(GAME_RESOURCEFILE, 0);
	if (_fontContext == NULL) {
		error("Font::Font(): Couldn't get resource context.");
	}

	assert(_vm->getFontsCount() > 0);

	_nFonts = 0;

	_fonts = (FONT **)malloc(_vm->getFontsCount() * sizeof(*_fonts));
	if (_fonts == NULL) {
		memoryError("Font::Font");
	}

	for (i = 0; i < _vm->getFontsCount(); i++) {
		loadFont(_vm->getFontDescription(i)->fontResourceId);
	}

	_initialized = true;
}

Font::~Font(void) {
//	int i;

	debug(8, "Font::~Font(): Freeing fonts.");
/*
	for (i = 0 ; i < FONT_COUNT ; i++) {
		if (_fonts[i] != NULL) {
			if (_fonts[i]->normal_loaded) {
				free(_fonts[i]->normal->font_free_p);
				free(_fonts[i]->normal);
			}

			if (_fonts[i]->outline_loaded) {
				free(_fonts[i]->outline->font_free_p);
				free(_fonts[i]->outline);
			}
		}

		free(_fonts[i]);
	}
*/
}

int Font::loadFont(uint32 fontResourceId) {
	FONT_HEADER fh;
	FONT *font;
	FONT_STYLE *normal_font;
	byte *fontres_p;
	size_t fontres_len;
	int nbits;
	int c;

	// Load font resource
	if (RSC_LoadResource(_fontContext, fontResourceId, &fontres_p, &fontres_len) != SUCCESS) {
		error("Font::loadFont(): Couldn't load font resource.");
	}

	if (fontres_len < FONT_DESCSIZE) {
		warning("Font::loadFont(): Invalid font length (%d < %d)", fontres_len, FONT_DESCSIZE);
		return FAILURE;
	}

	MemoryReadStreamEndian readS(fontres_p, fontres_len, IS_BIG_ENDIAN);

	// Create new font structure
	font = (FONT *)malloc(sizeof(*font));
	if (font == NULL) {
		memoryError("Font::loadFont");
	}

	// Read font header
	fh.c_height = readS.readUint16();
	fh.c_width = readS.readUint16();
	fh.row_length = readS.readUint16();

	debug(1, "Font::loadFont(): Reading fontResourceId %d...", fontResourceId);

	debug(2, "Character width: %d", fh.c_width);
	debug(2, "Character height: %d", fh.c_height);
	debug(2, "Row padding: %d", fh.row_length);

	// Create normal font style
	normal_font = (FONT_STYLE *)malloc(sizeof(*normal_font));
	if (normal_font == NULL) {
		memoryError("Font::loadFont");
	}

	normal_font->font_free_p = fontres_p;
	normal_font->hdr.c_height = fh.c_height;
	normal_font->hdr.c_width = fh.c_width;
	normal_font->hdr.row_length = fh.row_length;

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		normal_font->fce[c].index = readS.readUint16();
	}

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		nbits = normal_font->fce[c].width = readS.readByte();
		normal_font->fce[c].byte_width = getByteLen(nbits);
	}

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		normal_font->fce[c].flag = readS.readByte();
	}

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		normal_font->fce[c].tracking = readS.readByte();
	}

	if (readS.pos() != FONT_DESCSIZE) {
		warning("Invalid font resource size.");
		return FAILURE;
	}

	normal_font->font_p = fontres_p + FONT_DESCSIZE;

	font->normal = normal_font;
	font->normal_loaded = 1;

	// Create outline font style
	font->outline = createOutline(normal_font);
	font->outline_loaded = 1;

	// Set font data 
	_fonts[_nFonts++] = font;

	return SUCCESS;
}

int Font::getHeight(int font_id) {
	FONT *font;

	if (!_initialized) {
		return FAILURE;
	}

	if ((font_id < 0) || (font_id >= _nFonts) || (_fonts[font_id] == NULL)) {
		error("Font::getHeight(): Invalid font id.");
	}

	font = _fonts[font_id];

	return font->normal->hdr.c_height;
}

FONT_STYLE *Font::createOutline(FONT_STYLE *src_font) {
	FONT_STYLE *new_font;
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
	new_font = (FONT_STYLE *)malloc(sizeof(*new_font));

	if (new_font == NULL) {
		memoryError("Font::createOutline");
	}

	memset(new_font, 0, sizeof(*new_font));

	// Populate new font style character data 
	for (i = 0; i < FONT_CHARCOUNT; i++) {
		new_byte_width = 0;
		old_byte_width = 0;
		index = src_font->fce[i].index;
		if ((index > 0) || (i == FONT_FIRSTCHAR)) {
			index += index_offset;
		}

		new_font->fce[i].index = index;
		new_font->fce[i].tracking = src_font->fce[i].tracking;
		new_font->fce[i].flag = src_font->fce[i].flag;

		if (src_font->fce[i].width != 0) {
			new_byte_width = getByteLen(src_font->fce[i].width + 2);
			old_byte_width = getByteLen(src_font->fce[i].width);

			if (new_byte_width > old_byte_width) {
				index_offset++;
			}
		}

		new_font->fce[i].width = src_font->fce[i].width + 2;
		new_font->fce[i].byte_width = new_byte_width;
		new_row_len += new_byte_width;
	}

	debug(2, "New row length: %d", new_row_len);

	new_font->hdr.c_width = s_width + 2;
	new_font->hdr.c_height = s_height + 2;
	new_font->hdr.row_length = new_row_len;

	// Allocate new font representation storage 
	new_font_data_len = new_row_len * (s_height + 2);
	new_font_data = (unsigned char *)malloc(new_font_data_len);

	if (new_font_data == NULL) {
		memoryError("Font::createOutline");
	}

	memset(new_font_data, 0, new_font_data_len);

	new_font->font_free_p = new_font_data;
	new_font->font_p = new_font_data;

	// Generate outline font representation
	for (i = 0; i < FONT_CHARCOUNT; i++) {
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

int Font::getByteLen(int num_bits) {
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
int Font::getStringWidth(int font_id, const char *test_str, size_t test_str_ct, int flags) {
	FONT *font;
	size_t ct;
	int width = 0;
	int ch;
	const byte *txt_p;

	if (!_initialized) {
		return FAILURE;
	}

	if ((font_id < 0) || (font_id >= _nFonts) || (_fonts[font_id] == NULL)) {
		error("Font::getStringWidth(): Invalid font id.");
	}

	font = _fonts[font_id];
	assert(font != NULL);

	txt_p = (const byte *) test_str;

	for (ct = test_str_ct; *txt_p && (!test_str_ct || ct > 0); txt_p++, ct--) {
		ch = *txt_p & 0xFFU;
		// Translate character
		ch = _charMap[ch];
		assert(ch < FONT_CHARCOUNT);
		width += font->normal->fce[ch].tracking;
	}

	if ((flags & FONT_BOLD) || (flags & FONT_OUTLINE)) {
		width += 1;
	}

	return width;
}

int Font::draw(int font_id, SURFACE *ds, const char *draw_str, size_t draw_str_ct,
			int text_x, int text_y, int color, int effect_color, int flags) {
	FONT *font;

	if (!_initialized) {
		error("Font::draw(): Font Module not initialized.");
	}

	if ((font_id < 0) || (font_id >= _nFonts) || (_fonts[font_id] == NULL)) {
		error("Font::draw(): Invalid font id.");
	}

	font = _fonts[font_id];

	if (flags & FONT_OUTLINE) { 
		outFont(font->outline, ds, draw_str, draw_str_ct, text_x - 1, text_y - 1, effect_color, flags);
		outFont(font->normal, ds, draw_str, draw_str_ct, text_x, text_y, color, flags);
	} else if (flags & FONT_SHADOW) {
		outFont(font->normal, ds, draw_str, draw_str_ct, text_x - 1, text_y + 1, effect_color, flags);
		outFont(font->normal, ds, draw_str, draw_str_ct, text_x, text_y, color, flags);
	} else { // FONT_NORMAL
		outFont(font->normal, ds, draw_str, draw_str_ct, text_x, text_y, color, flags);
	}

	return SUCCESS;
}

int Font::outFont(FONT_STYLE * draw_font, SURFACE * ds, const char *draw_str, size_t draw_str_ct,
				  int text_x, int text_y, int color, int flags) {
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

	if ((text_x > ds->w) || (text_y > ds->h)) {
		// Output string can't be visible
		return SUCCESS;
	}

	draw_str_p = (const byte *) draw_str;
	ct = draw_str_ct;

	// Draw string one character at a time, maximum of 'draw_str'_ct 
	// characters, or no limit if 'draw_str_ct' is 0
	for (; *draw_str_p && (!draw_str_ct || ct); draw_str_p++, ct--) {
		c_code = *draw_str_p & 0xFFU;

		// Translate character
		if (!(flags & FONT_DONTMAP))
			c_code = _charMap[c_code];
		assert(c_code < FONT_CHARCOUNT);

		// Check if character is defined
		if ((draw_font->fce[c_code].index == 0) && (c_code != FONT_FIRSTCHAR)) {
#if FONT_SHOWUNDEFINED
			if (c_code == FONT_CH_SPACE) {
				text_x += draw_font->fce[c_code].tracking;
				continue;
			}
			c_code = FONT_CH_QMARK;
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
		row_limit = (ds->h < (text_y + draw_font->hdr.c_height)) ? ds->h : text_y + draw_font->hdr.c_height;
		char_row = 0;

		for (row = text_y; row < row_limit; row++, char_row++) {
			// Clip negative rows */
			if (row < 0) {
				continue;
			}

			output_ptr = (byte *)ds->pixels + (ds->pitch * row) + text_x;
			output_ptr_min = (byte *)ds->pixels + (ds->pitch * row) + (text_x > 0 ? text_x : 0);
			output_ptr_max = output_ptr + (ds->pitch - text_x);

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

	return SUCCESS;
}

} // End of namespace Saga
