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

#ifndef BLADERUNNER_VQA_DECODER_H
#define BLADERUNNER_VQA_DECODER_H

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
		uint16  loopCount;
		uint32  flags;
		Loop   *loops;

		LoopInfo()
			: loopCount(0)
		{}
	};

	struct ClipInfo
	{
		uint16 clipCount;
	};

	Common::SeekableReadStream *_s;

	Header   _header;
	LoopInfo _loopInfo;
	ClipInfo _clipInfo;

	uint16  *_frame;
	uint16  *_zbuf;

	size_t   _codebookSize;
	uint8   *_codebook;
	uint8   *_cbfz;

	size_t   _vptrSize;
	uint8   *_vptr;

	uint32  *_frameInfo;

	uint32   _maxVIEWChunkSize;
	uint32   _maxZBUFChunkSize;
	uint32   _maxAESCChunkSize;
	uint8   *_zbufChunk;

	bool   _hasView;
	// view_t view;

	// ima_adpcm_ws_decoder_t ima_adpcm_ws_decoder;
	int16 *_audioFrame;

	bool readVQHD(uint32 size);
	bool readMSCI(uint32 size);
	bool readMFCI(uint32 size);
	bool readLINF(uint32 size);
	bool readCINF(uint32 size);
	bool readFINF(uint32 size);
	bool readLNIN(uint32 size);
	bool readCLIP(uint32 size);

	bool readSN2J(uint32 size);
	bool readSND2(uint32 size);
	bool readVQFR(uint32 size);
	bool readVPTR(uint32 size);
	bool readVQFL(uint32 size);
	bool readCBFZ(uint32 size);
	bool readZBUF(uint32 size);
	bool readVIEW(uint32 size);
	bool readAESC(uint32 size);
	bool readLITE(uint32 size);

public:
	VQADecoder();
	~VQADecoder();

	bool open(Common::SeekableReadStream *s);
	bool readFrame();

	int getFrameTime() { return 1000 / _header.frameRate; }

	void VPTRWriteBlock(uint16 *frame, unsigned int dstBlock, unsigned int srcBlock, int count, bool alpha = false) const;

	bool seekToFrame(int frame);
	bool decodeFrame(uint16 *frame);

	int16 *getAudioFrame();

	// bool get_view(view_t *view);
	bool getZBUF(uint16 *zbuf);

	friend class VQAPlayer;
};

}; // End of namespace BladeRunner

#endif
