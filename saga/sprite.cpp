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
 
    Sprite management module

 Notes: 
*/

#include "reinherit.h"

#include "yslib.h"

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "game_mod.h"
#include "gfx_mod.h"
#include "scene_mod.h"
#include "rscfile_mod.h"

#include "text_mod.h"
#include "font_mod.h"

/*
 * Begin module component
\*--------------------------------------------------------------------------*/
#include "sprite_mod.h"
#include "sprite.h"

namespace Saga {

static R_SPRITE_MODULE SpriteModule = { 0, 0, 0, 0 };

int SPRITE_Init(void)
{
	int result;

	if (SpriteModule.init) {
		return R_FAILURE;
	}

	R_printf(R_STDOUT, "Initializing sprite subsystem...\n");

	/* Load sprite module resource context
	 * \*------------------------------------------------------------- */
	result = GAME_GetFileContext(&SpriteModule.sprite_ctxt,
	    R_GAME_RESOURCEFILE, 0);
	if (result != R_SUCCESS) {

		return R_FAILURE;
	}

	SpriteModule.decode_buf_len = R_DECODE_BUF_LEN;

	SpriteModule.decode_buf = (uchar *)malloc(R_DECODE_BUF_LEN);
	if (SpriteModule.decode_buf == NULL) {
		return R_MEM;
	}

	SpriteModule.init = 1;

	return R_SUCCESS;
}

int SPRITE_Shutdown(void)
{
	if (!SpriteModule.init) {
		return R_FAILURE;
	}

	R_printf(R_STDOUT, "Shutting down sprite subsystem...\n");

	free(SpriteModule.decode_buf);

	return R_SUCCESS;
}

int SPRITE_LoadList(int resource_num, R_SPRITELIST ** sprite_list_p)
{
	R_SPRITELIST *new_slist;

	uchar *spritelist_data;
	size_t spritelist_len;

	const uchar *read_p;

	uint sprite_count;
	uint i;

	new_slist = (R_SPRITELIST *)malloc(sizeof *new_slist);
	if (new_slist == NULL) {

		return R_MEM;
	}

	if (RSC_LoadResource(SpriteModule.sprite_ctxt,
		resource_num,
		&spritelist_data, &spritelist_len) != R_SUCCESS) {

		return R_FAILURE;
	}

	read_p = spritelist_data;

	sprite_count = ys_read_u16_le(read_p, &read_p);

	new_slist->sprite_count = sprite_count;

	new_slist->offset_list = (R_SPRITELIST_OFFSET *)malloc(sprite_count *
	    sizeof *new_slist->offset_list);
	if (new_slist->offset_list == NULL) {
		free(new_slist);

		return R_MEM;
	}

	for (i = 0; i < sprite_count; i++) {
		new_slist->offset_list[i].data_idx = 0;
		new_slist->offset_list[i].offset =
		    ys_read_u16_le(read_p, &read_p);
	}

	new_slist->slist_rn = resource_num;
	new_slist->sprite_data[0] = spritelist_data;
	new_slist->append_count = 0;

	*sprite_list_p = new_slist;

	return R_SUCCESS;
}

int SPRITE_AppendList(int resource_num, R_SPRITELIST * spritelist)
{
	uchar *spritelist_data;
	size_t spritelist_len;

	const uchar *read_p;

	void *test_p;

	uint old_sprite_count;
	uint new_sprite_count;
	uint sprite_count;

	int i;

	if (spritelist->append_count >= (R_APPENDMAX - 1)) {

		return R_FAILURE;
	}

	if (RSC_LoadResource(SpriteModule.sprite_ctxt,
		resource_num,
		&spritelist_data, &spritelist_len) != R_SUCCESS) {

		return R_FAILURE;
	}

	read_p = spritelist_data;

	sprite_count = ys_read_u16_le(read_p, &read_p);

	old_sprite_count = spritelist->sprite_count;
	new_sprite_count = spritelist->sprite_count + sprite_count;

	test_p = realloc(spritelist->offset_list,
	    new_sprite_count * sizeof *spritelist->offset_list);
	if (test_p == NULL) {
		return R_MEM;
	}

	spritelist->offset_list = (R_SPRITELIST_OFFSET *)test_p;

	spritelist->sprite_count = new_sprite_count;
	spritelist->append_count++;

	for (i = old_sprite_count; i < spritelist->sprite_count; i++) {
		spritelist->offset_list[i].data_idx = spritelist->append_count;
		spritelist->offset_list[i].offset =
		    ys_read_u16_le(read_p, &read_p);
	}

	spritelist->sprite_data[spritelist->append_count] = spritelist_data;

	return R_SUCCESS;
}

int SPRITE_GetListLen(R_SPRITELIST * spritelist)
{

	return spritelist->sprite_count;
}

int SPRITE_Free(R_SPRITELIST * spritelist)
{
	int i;

	for (i = 0; i <= spritelist->append_count; i++) {

		RSC_FreeResource(spritelist->sprite_data[i]);
	}

	free(spritelist->offset_list);
	free(spritelist);

	return R_SUCCESS;
}

int
SPRITE_Draw(R_SURFACE * ds,
    R_SPRITELIST * sprite_list, int sprite_num, int spr_x, int spr_y)
{

	int offset;
	int offset_idx;

	uchar *sprite_p;

	const uchar *sprite_data_p;
	const uchar *read_p;

	int i, j;

	uchar *buf_row_p;
	uchar *src_row_p;

	int s_width;
	int s_height;

	int clip_width;
	int clip_height;

	int x_align;
	int y_align;

	if (!SpriteModule.init) {
		return R_FAILURE;
	}

	offset = sprite_list->offset_list[sprite_num].offset;
	offset_idx = sprite_list->offset_list[sprite_num].data_idx;

	sprite_p = sprite_list->sprite_data[offset_idx];
	sprite_p += offset;

	read_p = (uchar *) sprite_p;

	x_align = ys_read_s8(read_p, &read_p);
	y_align = ys_read_s8(read_p, &read_p);

	s_width = ys_read_u8(read_p, &read_p);
	s_height = ys_read_u8(read_p, &read_p);

	sprite_data_p = read_p;

	spr_x += x_align;
	spr_y += y_align;

	if (spr_x < 0) {
		return 0;
	}

	if (spr_y < 0) {
		return 0;
	}

	DecodeRLESprite(sprite_data_p,
	    64000, SpriteModule.decode_buf, s_width * s_height);

	buf_row_p = ds->buf + ds->buf_pitch * spr_y;
	src_row_p = SpriteModule.decode_buf;

	/* Clip to right side of surface */
	clip_width = s_width;
	if (s_width > (ds->buf_w - spr_x)) {
		clip_width = (ds->buf_w - spr_x);
	}

	/* Clip to bottom side of surface */
	clip_height = s_height;
	if (s_height > (ds->buf_h - spr_y)) {
		clip_height = (ds->buf_h - spr_y);
	}

	for (i = 0; i < clip_height; i++) {

		for (j = 0; j < clip_width; j++) {
			if (*(src_row_p + j) != 0) {
				*(buf_row_p + j + spr_x) = *(src_row_p + j);
			}
		}

		buf_row_p += ds->buf_pitch;
		src_row_p += s_width;
	}

	return R_SUCCESS;
}

int
SPRITE_DrawOccluded(R_SURFACE * ds,
    R_SPRITELIST * sprite_list, int sprite_num, int spr_x, int spr_y)
{
	int offset;
	int offset_idx;

	uchar *sprite_p;
	const uchar *sprite_data_p;
	const uchar *read_p;

	int i;

	int x, y;
	uchar *dst_row_p;
	uchar *src_row_p;
	uchar *src_p;
	uchar *dst_p;
	uchar *mask_p;

	int s_width;
	int s_height;
	int x_align;
	int y_align;

	int z_lut[R_SPRITE_ZMAX];
	int e_slope;

	/* Clipinfo variables */
	R_POINT spr_pt;
	R_RECT spr_src_rect;
	R_RECT spr_dst_rect;

	R_CLIPINFO ci;

	/* BG mask variables */
	int mask_w;
	int mask_h;
	uchar *mask_buf;
	size_t mask_buf_len;

	uchar *mask_row_p;
	int mask_z;

	/* Z info variables */
	SCENE_ZINFO zinfo;
	int actor_z;

	if (!SpriteModule.init) {
		return R_FAILURE;
	}

	if (!SCENE_IsBGMaskPresent()) {
		return SPRITE_Draw(ds, sprite_list, sprite_num, spr_x, spr_y);
	}

	if (sprite_num >= sprite_list->sprite_count) {

		R_printf(R_STDOUT,
		    "Invalid sprite number (%d) for sprite list %d.\n",
		    sprite_num, sprite_list->slist_rn);

		return R_FAILURE;
	}

	/* Get sprite data from list 
	 * \*------------------------------------------------------------- */
	offset = sprite_list->offset_list[sprite_num].offset;
	offset_idx = sprite_list->offset_list[sprite_num].data_idx;

	sprite_p = sprite_list->sprite_data[offset_idx];
	sprite_p += offset;

	read_p = sprite_p;

	/* Read sprite dimensions -- should probably cache this stuff in 
	 * sprite list */
	x_align = ys_read_s8(read_p, &read_p);
	y_align = ys_read_s8(read_p, &read_p);

	s_width = ys_read_u8(read_p, &read_p);
	s_height = ys_read_u8(read_p, &read_p);

	sprite_data_p = read_p;

	/* Create actor Z occlusion LUT
	 * \*---------------------------------------------------------------------- */
	SCENE_GetZInfo(&zinfo);

	e_slope = zinfo.end_slope;

	for (i = 0; i < R_SPRITE_ZMAX; i++) {

		z_lut[i] =
		    (int)(e_slope + ((137.0 - e_slope) / 14.0) * (15.0 - i));
	}

	actor_z = spr_y;

	SCENE_GetBGMaskInfo(&mask_w, &mask_h, &mask_buf, &mask_buf_len);

	spr_src_rect.left = 0;
	spr_src_rect.top = 0;
	spr_src_rect.right = s_width - 1;
	spr_src_rect.bottom = s_height - 1;

	spr_dst_rect.left = 0;
	spr_dst_rect.top = 0;
	spr_dst_rect.right = ds->clip_rect.right;
	spr_dst_rect.bottom = MIN(ds->clip_rect.bottom, (int16)(mask_h - 1));

	spr_pt.x = spr_x + x_align;
	spr_pt.y = spr_y + y_align;

	spr_x += x_align;
	spr_y += y_align;

	ci.dst_rect = &spr_dst_rect;
	ci.src_rect = &spr_src_rect;
	ci.dst_pt = &spr_pt;

	GFX_GetClipInfo(&ci);

	if (ci.nodraw) {
		return R_SUCCESS;
	}

	DecodeRLESprite(sprite_data_p,
	    64000, SpriteModule.decode_buf, s_width * s_height);

	/* Finally, draw the occluded sprite
	 * \*---------------------------------------------------------------------- */
	src_row_p = SpriteModule.decode_buf + ci.src_draw_x +
	    (ci.src_draw_y * s_width);

	dst_row_p = ds->buf + ci.dst_draw_x + (ci.dst_draw_y * ds->buf_pitch);
	mask_row_p = mask_buf + ci.dst_draw_x + (ci.dst_draw_y * mask_w);

	for (y = 0; y < ci.draw_h; y++) {

		src_p = src_row_p;
		dst_p = dst_row_p;
		mask_p = mask_row_p;

		for (x = 0; x < ci.draw_w; x++) {

			if (*src_p != 0) {

				mask_z = *mask_p & R_SPRITE_ZMASK;

				if (actor_z > z_lut[mask_z]) {
					*dst_p = *src_p;
				}
			}
			src_p++;
			dst_p++;
			mask_p++;
		}
		dst_row_p += ds->buf_pitch;
		mask_row_p += mask_w;
		src_row_p += s_width;
	}
/*
    {
        char buf[1024] = { 0 };
        
        sprintf( buf, "dw: %d, dh: %d.", ci.draw_w, ci.draw_h );

        TEXT_Draw( 2,
                   ds,
                   buf,
                   spr_x - x_align, spr_y - y_align,
                   255, 0,
                   FONT_OUTLINE );
    }
*/
	return R_SUCCESS;
}

int
DecodeRLESprite(const uchar * inbuf,
    size_t inbuf_len, uchar * outbuf, size_t outbuf_len)
{

	int bg_runcount;
	int fg_runcount;

	const uchar *inbuf_ptr;
	uchar *outbuf_ptr;

	const uchar *inbuf_end;
	uchar *outbuf_end;

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

			*outbuf_ptr = (uchar) 0;
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

	return R_SUCCESS;
}

} // End of namespace Saga
