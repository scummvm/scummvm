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

// Sprite management module
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/scene.h"
#include "saga/rscfile_mod.h"

#include "saga/text.h"
#include "saga/font.h"

#include "saga/sprite.h"
#include "saga/stream.h"

namespace Saga {

Sprite::Sprite(SagaEngine *vm) : _vm(vm), _initialized(false) {
	debug(0, "Initializing sprite subsystem...");

	// Load sprite module resource context
	_spriteContext = _vm->getFileContext(GAME_RESOURCEFILE, 0);
	if (_spriteContext == NULL) {
		return;
	}

	_decodeBufLen = DECODE_BUF_LEN;

	_decodeBuf = (byte *)malloc(DECODE_BUF_LEN);
	if (_decodeBuf == NULL) {
		return;
	}

	_initialized = true;
}

Sprite::~Sprite(void) {
	if (!_initialized) {
		return;
	}

	debug(0, "Shutting down sprite subsystem...");

	free(_decodeBuf);
}

int Sprite::loadList(int resource_num, SPRITELIST **sprite_list_p) {
	SPRITELIST *new_slist;
	byte *spritelist_data;
	size_t spritelist_len;
	uint16 sprite_count;
	uint16 i;

	if (RSC_LoadResource(_spriteContext, resource_num, &spritelist_data, &spritelist_len) != SUCCESS) {
		return FAILURE;
	}

	if (spritelist_len == 0)
		return FAILURE;

	MemoryReadStreamEndian readS(spritelist_data, spritelist_len, IS_BIG_ENDIAN);

	sprite_count = readS.readUint16();

	new_slist = (SPRITELIST *)malloc(sizeof *new_slist);
	if (new_slist == NULL) {
		return MEM;
	}

	new_slist->sprite_count = sprite_count;

	new_slist->offset_list = (SPRITELIST_OFFSET *)malloc(sprite_count * sizeof *new_slist->offset_list);
	if (new_slist->offset_list == NULL) {
		free(new_slist);
		return MEM;
	}

	for (i = 0; i < sprite_count; i++) {
		new_slist->offset_list[i].data_idx = 0;
		new_slist->offset_list[i].offset = readS.readUint16();
	}

	new_slist->slist_rn = resource_num;
	new_slist->sprite_data[0] = spritelist_data;
	new_slist->append_count = 0;

	*sprite_list_p = new_slist;

	return SUCCESS;
}

int Sprite::appendList(int resource_num, SPRITELIST *spritelist) {
	byte *spritelist_data;
	size_t spritelist_len;
	void *test_p;
	uint16 old_sprite_count;
	uint16 new_sprite_count;
	uint16 sprite_count;
	int i;

	if (spritelist->append_count >= (APPENDMAX - 1)) {
		return FAILURE;
	}

	if (RSC_LoadResource(_spriteContext, resource_num, &spritelist_data, &spritelist_len) != SUCCESS) {
		return FAILURE;
	}

	MemoryReadStreamEndian readS(spritelist_data, spritelist_len, IS_BIG_ENDIAN);

	sprite_count = readS.readUint16();

	old_sprite_count = spritelist->sprite_count;
	new_sprite_count = spritelist->sprite_count + sprite_count;

	test_p = realloc(spritelist->offset_list, new_sprite_count * sizeof *spritelist->offset_list);
	if (test_p == NULL) {
		return MEM;
	}

	spritelist->offset_list = (SPRITELIST_OFFSET *)test_p;

	spritelist->sprite_count = new_sprite_count;
	spritelist->append_count++;

	for (i = old_sprite_count; i < spritelist->sprite_count; i++) {
		spritelist->offset_list[i].data_idx = spritelist->append_count;
		spritelist->offset_list[i].offset = readS.readUint16();
	}

	spritelist->sprite_data[spritelist->append_count] = spritelist_data;

	return SUCCESS;
}

int Sprite::getListLen(SPRITELIST *spritelist) {
	return spritelist->sprite_count;
}

int Sprite::freeSprite(SPRITELIST *spritelist) {
	int i;

	for (i = 0; i <= spritelist->append_count; i++) {

		RSC_FreeResource(spritelist->sprite_data[i]);
	}

	free(spritelist->offset_list);
	free(spritelist);

	return SUCCESS;
}

int Sprite::draw(SURFACE *ds, SPRITELIST *sprite_list, int sprite_num, const Point &screenCoord, int scale) {
	int offset;
	int offset_idx;
	byte *sprite_p;
	const byte *sprite_data_p;
	int i, j;
	byte *buf_row_p;
	byte *src_row_p;
	int s_width, so_width;
	int s_height, so_height;
	int clip_width;
	int clip_height;
	int x_align;
	int y_align;
	Point spr_pt;

	if (!_initialized) {
		return FAILURE;
	}

	offset = sprite_list->offset_list[sprite_num].offset;
	offset_idx = sprite_list->offset_list[sprite_num].data_idx;

	sprite_p = sprite_list->sprite_data[offset_idx];
	sprite_p += offset;

	assert(sprite_p);

	MemoryReadStreamEndian readS(sprite_p, 8, IS_BIG_ENDIAN);
	if (!(_vm->_features & GF_MAC_RESOURCES)) {
		x_align = readS.readSByte();
		y_align = readS.readSByte();

		so_width = s_width = readS.readByte();
		so_height = s_height = readS.readByte();
	} else {
		x_align = readS.readSint16();
		y_align = readS.readSint16();

		so_width = s_width = readS.readUint16();
		so_height = s_height = readS.readUint16();
	}
	debug(0, "%d x %d", s_width, s_height);
	spr_pt.x = screenCoord.x + x_align;
	spr_pt.y = screenCoord.y + y_align;

	if (scale < 256)
		scaleSpriteCoords(scale, &s_width, &s_height, &x_align, &y_align);

	sprite_data_p = sprite_p + readS.pos();

	decodeRLESprite(sprite_data_p, 64000, _decodeBuf, so_width * so_height);

	if (scale < 256)
		scaleSprite(_decodeBuf, so_width, so_height, scale);

	if (spr_pt.x < 0) {
		return 0;
	}

	if (spr_pt.y < 0) {
		return 0;
	}

	buf_row_p = (byte *)ds->pixels + ds->pitch * spr_pt.y;
	src_row_p = _decodeBuf;

	// Clip to right side of surface
	clip_width = s_width;
	if (s_width > (ds->w - spr_pt.x)) {
		clip_width = (ds->w - spr_pt.x);
	}

	// Clip to bottom side of surface
	clip_height = s_height;
	if (s_height > (ds->h - spr_pt.y)) {
		clip_height = (ds->h - spr_pt.y);
	}

	for (i = 0; i < clip_height; i++) {
		for (j = 0; j < clip_width; j++) {
			if (*(src_row_p + j) != 0) {
				*(buf_row_p + j + spr_pt.x) = *(src_row_p + j);
			}
		}
		buf_row_p += ds->pitch;
		src_row_p += s_width;
	}

	return SUCCESS;
}

int Sprite::drawOccluded(SURFACE *ds, SPRITELIST *sprite_list, int sprite_num, const Point &screenCoord, int scale, int depth) {
	int offset;
	int offset_idx;
	byte *sprite_p;
	const byte *sprite_data_p;
	int x, y;
	byte *dst_row_p;
	byte *src_row_p;
	byte *src_p;
	byte *dst_p;
	byte *mask_p;
	int s_width, so_width;
	int s_height, so_height;
	int x_align;
	int y_align;

	// Clipinfo variables
	Point spr_pt;
	Rect spr_src_rect;
	Rect spr_dst_rect;
	CLIPINFO ci;

	// BG mask variables
	int mask_w;
	int mask_h;
	byte *mask_buf;
	size_t mask_buf_len;
	byte *mask_row_p;
	int mask_z;


	if (!_initialized) {
		return FAILURE;
	}

	if (!_vm->_scene->isBGMaskPresent()) {
		return draw(ds, sprite_list, sprite_num, screenCoord, scale);
	}

	if (sprite_num >= sprite_list->sprite_count) {
		warning("Invalid sprite number (%d) for sprite list %d", sprite_num, sprite_list->slist_rn);
		return FAILURE;
	}

	// Get sprite data from list 
	offset = sprite_list->offset_list[sprite_num].offset;
	offset_idx = sprite_list->offset_list[sprite_num].data_idx;

	sprite_p = sprite_list->sprite_data[offset_idx];
	sprite_p += offset;

	MemoryReadStreamEndian readS(sprite_p, 5, IS_BIG_ENDIAN);

	// Read sprite dimensions -- should probably cache this stuff in 
	// sprite list
	x_align = readS.readSByte();
	y_align = readS.readSByte();

	so_width = s_width = readS.readByte();
	so_height = s_height = readS.readByte();

	sprite_data_p = sprite_p + readS.pos();

	_vm->_scene->getBGMaskInfo(&mask_w, &mask_h, &mask_buf, &mask_buf_len);

	if (scale < 256)
		scaleSpriteCoords(scale, &s_width, &s_height, &x_align, &y_align);

	spr_src_rect.left = 0;
	spr_src_rect.top = 0;
	spr_src_rect.right = s_width;
	spr_src_rect.bottom = s_height;

	spr_dst_rect.left = 0;
	spr_dst_rect.top = 0;
	spr_dst_rect.right = ds->clip_rect.right;
	spr_dst_rect.bottom = MIN(ds->clip_rect.bottom, (int16)mask_h);

	spr_pt.x = screenCoord.x + x_align;
	spr_pt.y = screenCoord.y + y_align;

	ci.dst_rect = &spr_dst_rect;
	ci.src_rect = &spr_src_rect;
	ci.dst_pt = &spr_pt;

	getClipInfo(&ci);

	if (ci.nodraw) {
		return SUCCESS;
	}

	decodeRLESprite(sprite_data_p, 64000, _decodeBuf, so_width * so_height);

	if (scale < 256)
		scaleSprite(_decodeBuf, so_width, so_height, scale);

	// Finally, draw the occluded sprite
	src_row_p = _decodeBuf + ci.src_draw_x + (ci.src_draw_y * s_width);

	dst_row_p = (byte *)ds->pixels + ci.dst_draw_x + (ci.dst_draw_y * ds->pitch);
	mask_row_p = mask_buf + ci.dst_draw_x + (ci.dst_draw_y * mask_w);

	for (y = 0; y < ci.draw_h; y++) {
		src_p = src_row_p;
		dst_p = dst_row_p;
		mask_p = mask_row_p;
		for (x = 0; x < ci.draw_w; x++) {
			if (*src_p != 0) {
				mask_z = *mask_p & SPRITE_ZMASK;
				if (mask_z > depth) {
					*dst_p = *src_p;
				}
			}
			src_p++;
			dst_p++;
			mask_p++;
		}
		dst_row_p += ds->pitch;
		mask_row_p += mask_w;
		src_row_p += s_width;
	}
/*
	{
		char buf[1024] = { 0 };
		sprintf(buf, "dw: %d, dh: %d.", ci.draw_w, ci.draw_h);

		_vm->textDraw(2, ds, buf, spr_x - x_align, spr_y - y_align, 255, 0, FONT_OUTLINE);
	}
*/
	return SUCCESS;
}

int Sprite::decodeRLESprite(const byte *inbuf, size_t inbuf_len, byte *outbuf, size_t outbuf_len) {
	int bg_runcount;
	int fg_runcount;
	const byte *inbuf_ptr;
	byte *outbuf_ptr;
	const byte *inbuf_end;
	byte *outbuf_end;
	int c;

	inbuf_ptr = inbuf;
	outbuf_ptr = outbuf;

	inbuf_end = inbuf + (inbuf_len);
	inbuf_end--;

	outbuf_end = outbuf + outbuf_len;
	outbuf_end--;

	memset(outbuf, 0, outbuf_len);

	while ((inbuf_ptr < inbuf_end) && (outbuf_ptr < outbuf_end)) {
		bg_runcount = *inbuf_ptr;
		if (inbuf_ptr < inbuf_end)
			inbuf_ptr++;
		else
			return 0;
		fg_runcount = *inbuf_ptr;
		if (inbuf_ptr < inbuf_end)
			inbuf_ptr++;
		else
			return 0;

		for (c = 0; c < bg_runcount; c++) {
			*outbuf_ptr = (byte) 0;
			if (outbuf_ptr < outbuf_end)
				outbuf_ptr++;
			else
				return 0;
		}

		for (c = 0; c < fg_runcount; c++) {
			*outbuf_ptr = *inbuf_ptr;
			if (inbuf_ptr < inbuf_end)
				inbuf_ptr++;
			else
				return 0;
			if (outbuf_ptr < outbuf_end)
				outbuf_ptr++;
			else
				return 0;
		}
	}

	return SUCCESS;
}

void Sprite::scaleSprite(byte *buf, int width, int height, int scale) {
	byte skip = 256 - scale; // skip factor

	byte vskip = 0x80, hskip;
	byte *src, *dst;

	src = dst = buf;

	for (int i = 0; i < height; i++) {
		vskip += skip;

		if(vskip < skip) { // We had an overflow
			src += width;
		} else {
			hskip = 0x80;

			for (int j = 0; j < width; j++) {
				*dst++ = *src++;
				
				hskip += skip;
				if (hskip < skip) // overflow
					dst--;
			}
		}
	}
}

void Sprite::scaleSpriteCoords(int scale, int *width, int *height, int *x_align, int *y_align) {
	*x_align = (*x_align * scale) >> 8;
	*y_align = (*y_align * scale) >> 8;
	*height = (*height * scale + 0x80) >> 8;
	*width = (*width * scale + 0x80) >> 8;
}


} // End of namespace Saga
