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

// Sprite management module private header file

#ifndef SAGA_SPRITE_H__
#define SAGA_SPRITE_H__

namespace Saga {

#define R_APPENDMAX 4

#define R_SPRITE_ZMAX  16
#define R_SPRITE_ZMASK 0x0F

#define R_DECODE_BUF_LEN 64000

struct R_SPRITELIST_ENTRY {
	int x_align;
	int y_align;
	int width;
	int height;

};

struct R_SPRITELIST_OFFSET {
	uint16 data_idx;
	size_t offset;
};

struct R_SPRITELIST_tag {
	int append_count;
	int sprite_count;
	R_SPRITELIST_OFFSET *offset_list;
	int slist_rn;
	byte *sprite_data[R_APPENDMAX];
};

typedef struct R_SPRITELIST_tag R_SPRITELIST;

class Sprite {
 public:
	Sprite(SagaEngine *vm);
	~Sprite(void);
	int loadList(int resource_num, R_SPRITELIST **sprite_list_p);
	int appendList(int resource_num, R_SPRITELIST *spritelist);
	int getListLen(R_SPRITELIST *spritelist);
	int freeSprite(R_SPRITELIST *spritelist);
	int draw(R_SURFACE *ds, R_SPRITELIST *sprite_list, int sprite_num, int spr_x, int spr_y);
	int drawOccluded(R_SURFACE *ds, R_SPRITELIST *sprite_list, int sprite_num, int spr_x, int spr_y);

 private:
	int decodeRLESprite(const byte *inbuf, size_t inbuf_len, byte *outbuf, size_t outbuf_len);

	SagaEngine *_vm;
	bool _initialized;
	R_RSCFILE_CONTEXT *_spriteContext;
	byte *_decodeBuf;
	size_t _decodeBufLen;
};

} // End of namespace Saga

#endif
