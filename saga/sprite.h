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
 
	Sprite management module private header file

 Notes: 
*/

#ifndef SAGA_SPRITE_H__
#define SAGA_SPRITE_H__

namespace Saga {

#define R_APPENDMAX 4

#define R_SPRITE_ZMAX  16
#define R_SPRITE_ZMASK 0x0F

#define R_DECODE_BUF_LEN 64000

typedef struct R_SPRITELIST_ENTRY_tag {

	int x_align;
	int y_align;
	int width;
	int height;

} R_SPRITELIST_ENTRY;

typedef struct R_SPRITELIST_OFFSET_tag {

	uint data_idx;
	size_t offset;

} R_SPRITELIST_OFFSET;

struct R_SPRITELIST_tag {

	int append_count;
	int sprite_count;

	R_SPRITELIST_OFFSET *offset_list;

	int slist_rn;
	uchar *sprite_data[R_APPENDMAX];

};

typedef struct R_SPRITE_MODULE_tag {

	int init;

	R_RSCFILE_CONTEXT *sprite_ctxt;

	uchar *decode_buf;
	size_t decode_buf_len;

} R_SPRITE_MODULE;

int
DecodeRLESprite(const uchar * inbuf,
    size_t inbuf_len, uchar * outbuf, size_t outbuf_len);

} // End of namespace Saga

#endif				/* SAGA_SPRITE_H__ */
