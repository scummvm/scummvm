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

// Font management and font drawing header file

#ifndef SAGA_FONT_H__
#define SAGA_FONT_H__

#include "saga/gfx.h"

namespace Saga {

#define FONT_SHOWUNDEFINED 1	// Define to draw undefined characters * as ?'s

// The first defined character (!) is the only one that may 
// have a valid offset of '0'
#define FONT_FIRSTCHAR 33

#define FONT_CH_SPACE 32
#define FONT_CH_QMARK 63

// Minimum font header size without font data 
// (6 + 512 + 256 + 256 + 256 )
#define FONT_DESCSIZE 1286

#define FONT_CHARCOUNT 256
#define FONT_CHARMASK 0xFFU

#define SAGA_FONT_HEADER_LEN 6

enum FONT_ID {
	SMALL_FONT_ID,
	MEDIUM_FONT_ID,
	BIG_FONT_ID
};

enum FONT_EFFECT_FLAGS {
	FONT_NORMAL   = 0,
	FONT_OUTLINE  = 1 << 0,
	FONT_SHADOW   = 1 << 1,
	FONT_BOLD     = 1 << 2,
	FONT_CENTERED = 1 << 3,
	FONT_DONTMAP  = 1 << 4
};

struct FONT_HEADER {
	int c_height;
	int c_width;
	int row_length;
};

struct FONT_CHAR_ENTRY {
	int index;
	int byte_width;
	int width;
	int flag;
	int tracking;
};

struct FONT_STYLE {
	FONT_HEADER hdr;
	FONT_CHAR_ENTRY fce[256];
	byte *font_free_p;
	byte *font_p;
};

struct FONT {
	uint32 font_rn;
	int font_id;

	int normal_loaded;
	FONT_STYLE *normal;
	int outline_loaded;
	FONT_STYLE *outline;

	byte *res_data;
	size_t res_len;
};

class Font {
 public:
	Font(SagaEngine *vm);
	~Font(void);
	int draw(int font_id, Surface *ds, const char *draw_str, size_t draw_str_len,
				  int text_x, int text_y, int color, int effect_color, int flags);
	int getStringWidth(int font_id, const char *test_str, size_t test_str_ct, int flags);
	int getHeight(int font_id);

 private:

	int loadFont(uint32 fontResourceId);
	FONT_STYLE *createOutline(FONT_STYLE * src_font);
	int outFont(FONT_STYLE *font, Surface *ds, const char *draw_str, size_t draw_str_ct,
				int text_x, int text_y, int color, int flags);
	int getByteLen(int num_bits);

	static const int _charMap[256];
	SagaEngine *_vm;

	bool _initialized;
	RSCFILE_CONTEXT *_fontContext;

	int _nFonts;
	FONT **_fonts;
};

} // End of namespace Saga

#endif
