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

// Background animation management module private header

#ifndef SAGA_ANIMATION_H_
#define SAGA_ANIMATION_H_

namespace Saga {

#define MAX_ANIMATIONS 7
#define DEFAULT_FRAME_TIME 140

#define SAGA_FRAME_HEADER_LEN (IS_MAC_VERSION ? 13 : 12)

#define SAGA_FRAME_START 0xF
#define SAGA_FRAME_END 0x3F
#define SAGA_FRAME_REPOSITION 0x30
#define SAGA_FRAME_ROW_END 0x2F
#define SAGA_FRAME_LONG_COMPRESSED_RUN	0x20
#define SAGA_FRAME_LONG_UNCOMPRESSED_RUN	0x10
#define SAGA_FRAME_COMPRESSED_RUN 0x80
#define SAGA_FRAME_UNCOMPRESSED_RUN 0x40
#define SAGA_FRAME_EMPTY_RUN 0xC0

// All animation resources begin with an ANIMATION_HEADER
// at 0x00, followed by a RLE code stream

struct ANIMATION_HEADER {
	uint16 magic;

	uint16 screen_w;
	uint16 screen_h;

	uint16 unknown06;
	uint16 unknown07;

	uint16 nframes;
	uint16 flags;

	uint16 unknown10;
	uint16 unknown11;

};

struct FRAME_HEADER {
	int x_start;
	int y_start;

	int x_pos;
	int y_pos;

	int width;
	int height;
};

// Animation info array member
struct ANIMATION {

	const byte *resdata;
	size_t resdata_len;

	uint16 n_frames;
	size_t *frame_offsets;
	uint16 current_frame;
	uint16 end_frame;
	uint16 stop_frame;
	const byte *cur_frame_p;
	size_t cur_frame_len;
	int frame_time;

	uint16 play_flag;
	int link_flag;
	uint16 link_id;
	uint16 flags;
};

enum ANIM_FLAGS {
	ANIM_LOOP = 0x01,
	ANIM_PAUSE = 0x02,
	ANIM_ENDSCENE = 0x80	// When animation ends, dispatch scene end event
};

class Anim {
public:
	Anim(SagaEngine *vm);
	~Anim(void);

	int load(const byte *anim_resdata, size_t anim_resdata_len, uint16 *anim_id_p);
	int freeId(uint16 anim_id);
	int play(uint16 anim_id, int vector_time);
	int link(uint16 anim_id1, uint16 anim_id2);
	int setFlag(uint16 anim_id, uint16 flag);
	int clearFlag(uint16 anim_id, uint16 flag);
	int setFrameTime(uint16 anim_id, int time);
	int reset(void);
	void animInfo(void);

private:
	int getNumFrames(const byte *anim_resource, size_t anim_resource_len, uint16 *n_frames);
	int ITE_DecodeFrame(const byte *anim_resource, size_t anim_resource_len, size_t frame_offset, byte *buf, size_t buf_len);
	int IHNM_DecodeFrame(byte *decode_buf, size_t decode_buf_len, const byte *thisf_p,
					size_t thisf_len, const byte **nextf_p, size_t *nextf_len);
	int getFrameOffset(const byte *anim_resource, size_t anim_resource_len, uint16 find_frame, size_t *frame_offset);

	SagaEngine *_vm;
	bool _initialized;

	uint16 _anim_count;
	uint16 _anim_limit;
	ANIMATION *_anim_tbl[MAX_ANIMATIONS];

};

} // End of namespace Saga

#endif				/* ANIMATION_H_ */
/* end "r_animation.h" */
