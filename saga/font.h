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
/*

 Description:   
 
    Font management and font drawing header file

 Notes: 
*/

#ifndef SAGA_FONT_H__
#define SAGA_FONT_H__

namespace Saga {

#define R_FONT_DBGLVL R_DEBUG_NONE

#define R_FONT_SHOWUNDEFINED 1	/* Define to draw undefined characters 
				 * as ?'s */

/* The first defined character (!) is the only one that may 
   have a valid offset of '0' */
#define R_FONT_FIRSTCHAR 33

#define R_FONT_CH_SPACE 32
#define R_FONT_CH_QMARK 63

/* Minimum font header size without font data 
   (6 + 512 + 256 + 256 + 256 ) */
#define R_FONT_DESCSIZE 1286

#define R_FONT_CHARCOUNT 256
#define R_FONT_CHARMASK 0xFFU

#define SAGA_FONT_HEADER_LEN 6

struct R_FONT_HEADER {

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

struct R_FONT_STYLE {

	R_FONT_HEADER hdr;
	FONT_CHAR_ENTRY fce[256];

	uchar *font_free_p;
	uchar *font_p;

};

struct R_FONT {

	ulong font_rn;
	int font_id;

	int normal_loaded;
	R_FONT_STYLE *normal;
	int outline_loaded;
	R_FONT_STYLE *outline;

	uchar *res_data;
	size_t res_len;

};

struct R_FONT_MODULE {

	int init;

	R_RSCFILE_CONTEXT *font_ctxt;

	int n_fonts;
	R_FONT **fonts;

	int err_n;
	const char *err_str;

};

int FONT_Load(ulong font_rn, int font_id);

static R_FONT_STYLE *FONT_CreateOutline(R_FONT_STYLE * src_font);

int
FONT_Out(R_FONT_STYLE * font,
    R_SURFACE * ds,
    const char *draw_str,
    size_t draw_str_ct, int text_x, int text_y, int color);

static int GetByteLen(int num_bits);

extern int CharMap[];

} // End of namespace Saga

#endif				/* R_FONT_H__ */
/* end "r_font.h" */
