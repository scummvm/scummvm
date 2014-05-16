/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_VQA_H
#define BLADERUNNER_VQA_H

#include "common/debug.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/types.h"

namespace BladeRunner {
class VQADecoder
{
	struct Header
	{
		uint16 version;     // 0x00
		uint16 flags;       // 0x02
		uint16 numFrames;   // 0x04
		uint16 width;       // 0x06
		uint16 height;      // 0x08
		uint8  blockW;      // 0x0A
		uint8  blockH;      // 0x0B
		uint8  frameRate;   // 0x0C
		uint8  cbParts;     // 0x0D
		uint16 colors;      // 0x0E
		uint16 maxBlocks;   // 0x10
		uint16 offset_x;    // 0x12
		uint16 offset_y;    // 0x14
		uint16 maxVPTRSize; // 0x16
		uint16 freq;        // 0x18
		uint8  channels;    // 0x1A
		uint8  bits;        // 0x1B
		uint32 unk3;        // 0x1C
		uint16 unk4;        // 0x20
		uint32 maxCBFZSize; // 0x22
		uint32 unk5;        // 0x26
		                    // 0x2A
	};

	struct Loop {
		uint16         begin;
		uint16         end;
		Common::String name;

		Loop() :
			begin(0),
			end(0)
		{}
	};

	struct LoopInfo
	{
		uint16  loop_count;
		uint32  flags;
		Loop     *loops;

		LoopInfo()
			: loop_count(0)
		{}
	};

	struct ClipInfo
	{
		uint16 clip_count;
	};

	Common::SeekableReadStream *r;

	Header   header;
	LoopInfo loop_info;
	ClipInfo clip_info;

	uint16 *frame;
	uint16 *zbuf;

	size_t  codebookSize;
	uint8  *codebook;
	uint8  *cbfz;

	size_t  vptrSize;
	uint8  *vptr;

	uint32 *frame_info;

	int     cur_frame;

	int     cur_loop;
	int     loop_special;
	int     loop_default;

	uint32  max_view_chunk_size;
	uint32  max_zbuf_chunk_size;
	uint32  max_aesc_chunk_size;
	uint8  *zbuf_chunk;

	bool   has_view;
	// view_t view;

	// ima_adpcm_ws_decoder_t ima_adpcm_ws_decoder;
	int16 *audio_frame;

	bool read_vqhd(uint32 size);
	bool read_msci(uint32 size);
	bool read_mfci(uint32 size);
	bool read_linf(uint32 size);
	bool read_cinf(uint32 size);
	bool read_finf(uint32 size);
	bool read_lnin(uint32 size);
	bool read_clip(uint32 size);

	bool read_sn2j(uint32 size);
	bool read_snd2(uint32 size);
	bool read_vqfr(uint32 size);
	bool read_vptr(uint32 size);
	bool read_vqfl(uint32 size);
	bool read_cbfz(uint32 size);
	bool read_zbuf(uint32 size);
	bool read_view(uint32 size);
	bool read_aesc(uint32 size);
	bool read_lite(uint32 size);

public:
	VQADecoder(Common::SeekableReadStream *r);
	~VQADecoder();

	bool read_header();
	int  read_frame();

	void vptr_write_block(uint16 *frame, unsigned int dst_block, unsigned int src_block, int count, bool alpha = false) const;

	void set_loop_special(int loop, bool wait);
	void set_loop_default(int loop);

	bool seek_to_frame(int frame);
	bool decode_frame(uint16 *frame);

	int16 *get_audio_frame();

	// bool get_view(view_t *view);
	bool get_zbuf(uint16 *zbuf);
};

}; // End of namespace BladeRunner

#endif
