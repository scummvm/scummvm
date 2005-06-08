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

// Background animation management module private header

#ifndef SAGA_ANIMATION_H_
#define SAGA_ANIMATION_H_

#include "saga/stream.h"

namespace Saga {

#define MAX_ANIMATIONS 7
#define DEFAULT_FRAME_TIME 140

#define SAGA_FRAME_HEADER_LEN (_vm->getFeatures() & GF_MAC_RESOURCES ? 13 : 12)

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

	byte unknown06;
	byte unknown07;

	byte maxframe;
	byte loopframe;

	uint16 start;
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

	uint16 maxframe;
	size_t *frame_offsets;
	int16 current_frame;
	uint16 completed;
	uint16 cycles;
	int16 loopframe;
	const byte *cur_frame_p;
	size_t cur_frame_len;
	int frame_time;

	int state;
	int16 link_id;
	uint16 flags;
};

enum ANIM_FLAGS {
	ANIM_PLAYING = 0x01,
	ANIM_PAUSE = 0x02,
	ANIM_STOPPING = 0x04,
	ANIM_ENDSCENE = 0x80	// When animation ends, dispatch scene end event
};

class Anim {
public:
	Anim(SagaEngine *vm);
	~Anim(void);

	int load(const byte *anim_resdata, size_t anim_resdata_len, uint16 *anim_id_p);
	void freeId(uint16 animId);
	int play(uint16 anim_id, int vector_time, bool playing = true);
	int link(int16 anim_id1, int16 anim_id2);
	int setFlag(uint16 anim_id, uint16 flag);
	int clearFlag(uint16 anim_id, uint16 flag);
	int setFrameTime(uint16 anim_id, int time);
	int reset(void);
	void animInfo(void);
	void setCycles(uint animId, int cycles);
	void stop(uint16 animId);
	void finish(uint16 animId);
	void resume(uint16 animId, int cycles);
	int16 getCurrentFrame(uint16 anim_id);

private:
	int ITE_DecodeFrame(const byte *anim_resource, size_t anim_resource_len, size_t frame_offset, byte *buf, size_t buf_len);
	int IHNM_DecodeFrame(byte *decode_buf, size_t decode_buf_len, const byte *thisf_p,
					size_t thisf_len, const byte **nextf_p, size_t *nextf_len);
	int getFrameOffset(const byte *anim_resource, size_t anim_resource_len, uint16 find_frame, size_t *frame_offset);
	void readAnimHeader(MemoryReadStreamEndian &readS, ANIMATION_HEADER &ah);

	SagaEngine *_vm;
	bool _initialized;

	uint16 _anim_count;
	uint16 _anim_limit;
	ANIMATION *_anim_tbl[MAX_ANIMATIONS];

};

} // End of namespace Saga

#endif				/* ANIMATION_H_ */
/* end "r_animation.h" */
