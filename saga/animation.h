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
 
    Background animation management module private header

 Notes: 
*/

#ifndef SAGA_ANIMATION_H_
#define SAGA_ANIMATION_H_

namespace Saga {

#define R_MAX_ANIMATIONS 7
#define R_DEFAULT_FRAME_TIME 140

#define SAGA_FRAME_HEADER_MAGIC 15
#define SAGA_FRAME_HEADER_LEN 12

/* All animation resources begin with an ANIMATION_HEADER
 * at 0x00, followed by a RLE code stream
\*--------------------------------------------------------------------------*/

struct R_ANIMATION_HEADER {

	uint magic;

	uint screen_w;
	uint screen_h;

	uint unknown06;
	uint unknown07;

	uint nframes;
	uint flags;

	uint unknown10;
	uint unknown11;

};

/* A byte from the code stream of FRAME_HEADER_MAGIC signifies that a
 * FRAME_HEADER structure follows
\*--------------------------------------------------------------------------*/

struct R_FRAME_HEADER {

	int x_start;
	int y_start;

	int x_pos;
	int y_pos;

	int width;
	int height;

};

/* Animation info array member */
struct R_ANIMATION {

	const uchar *resdata;
	size_t resdata_len;

	uint n_frames;
	size_t *frame_offsets;

	uint current_frame;
	uint end_frame;
	uint stop_frame;

	const uchar *cur_frame_p;
	size_t cur_frame_len;

	int frame_time;

	uint play_flag;
	int link_flag;
	uint link_id;

	uint flags;

};

struct R_ANIMINFO {

	int initialized;

	uint anim_count;
	uint anim_limit;

	R_ANIMATION *anim_tbl[R_MAX_ANIMATIONS];

};

int ANIM_GetNumFrames(const uchar * anim_resource, uint * n_frames);

int
ITE_DecodeFrame(const uchar * anim_resource,
    size_t frame_offset, uchar * buf, size_t buf_len);

int
IHNM_DecodeFrame(uchar * decode_buf,
    size_t decode_buf_len,
    const uchar * thisf_p,
    size_t thisf_len, const uchar ** nextf_p, size_t * nextf_len);

int
ANIM_GetFrameOffset(const uchar * anim_resource,
    uint find_frame, size_t * frame_offset);

static void CF_anim_info(int argc, char *argv[]);

} // End of namespace Saga

#endif				/* R_ANIMATION_H_ */
/* end "r_animation.h" */
