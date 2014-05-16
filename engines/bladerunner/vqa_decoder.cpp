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

#include "bladerunner/vqa_decoder.h"

#include "bladerunner/decompress_lcw.h"
#include "bladerunner/decompress_lzo.h"

#include "common/array.h"
#include "common/util.h"

namespace BladeRunner {

#define kAESC 0x41455343
#define kCBFZ 0x4342465A
#define kCIND 0x43494E44
#define kCINF 0x43494E46
#define kCINH 0x43494E48
#define kCLIP 0x434C4950
#define kFINF 0x46494E46
#define kFORM 0x464f524d
#define kLIND 0x4C494E44
#define kLINF 0x4C494E46
#define kLINH 0x4C494E48
#define kLITE 0x4C495445
#define kLNID 0x4C4E4944
#define kLNIH 0x4C4E4948
#define kLNIN 0x4C4E494E
#define kLNIO 0x4C4E494F
#define kMFCD 0x4D464344
#define kMFCH 0x4D464348
#define kMFCI 0x4D464349
#define kMFCT 0x4D464354
#define kMSCH 0x4D534348
#define kMSCI 0x4D534349
#define kMSCT 0x4D534354
#define kSN2J 0x534e324a
#define kSND2 0x534e4432
#define kVIEW 0x56494557
#define kVPTR 0x56505452
#define kVQFL 0x5651464C
#define kVQFR 0x56514652
#define kVQHD 0x56514844
#define kWVQA 0x57565141
#define kZBUF 0x5A425546

VQADecoder::VQADecoder(Common::SeekableReadStream *r)
	: r(r),
	  frame(0),
	  zbuf(0),
	  codebook(0),
	  cbfz(0),
	  vptr(0),
	  cur_frame(-1),
	  cur_loop(-1),
	  loop_special(-1),
	  loop_default(-1),
	  has_view(false),
	  audio_frame(0),
	  max_view_chunk_size(0),
	  max_zbuf_chunk_size(0),
	  max_aesc_chunk_size(0)
{
	// debug("Opening VQA: '%s'\n", r->get_name());
}

VQADecoder::~VQADecoder()
{
}

struct iff_chunk_header_s
{
	iff_chunk_header_s()
		: id(0), size(0)
	{}

	uint32 id;
	uint32 size;
};

static inline uint32 roundup(uint32 v)
{
	return (v + 1) & ~1u;
}

const char *str_tag(uint32 tag);

int32 stream_remain(Common::SeekableReadStream *s) {
	int32 pos = s->pos();
	if (pos == -1) return -1;

	int32 size = s->size();
	if (size == -1) return -1;

	return size - pos;
}

static
bool read_iff_chunk_header(Common::SeekableReadStream *r, iff_chunk_header_s *ts)
{
	if (stream_remain(r) < 8)
		return false;

	ts->id   = r->readUint32BE();
	ts->size = r->readUint32BE();

	// if (ts->size != roundup(ts->size))
	// 	debug("%s: %d\n", str_tag(ts->id), ts->size);

	return true;
}

const char *str_tag(uint32 tag)
{
	static char s[5];

	sprintf(s, "%c%c%c%c",
		(tag >> 24) & 0xff,
		(tag >> 16) & 0xff,
		(tag >>  8) & 0xff,
		(tag >>  0) & 0xff);

	return s;
}

bool VQADecoder::read_header()
{
	iff_chunk_header_s chd;
	uint32 type;
	bool rc;

	read_iff_chunk_header(r, &chd);
	if (chd.id != kFORM || !chd.size)
		return false;

	type = r->readUint32BE();

	if (type != kWVQA)
		return false;

	do {
		if (!read_iff_chunk_header(r, &chd))
			return false;

		debug("\t%s : %x\n", str_tag(chd.id), chd.size);

		rc = false;
		switch (chd.id)
		{
			case kCINF: rc = read_cinf(chd.size); break;
			case kCLIP: rc = read_clip(chd.size); break;
			case kFINF: rc = read_finf(chd.size); break;
			case kLINF: rc = read_linf(chd.size); break;
			case kLNIN: rc = read_lnin(chd.size); break;
			case kMFCI: rc = read_mfci(chd.size); break;
			case kMSCI: rc = read_msci(chd.size); break;
			case kVQHD: rc = read_vqhd(chd.size); break;
			default:
				debug("Unhandled chunk '%s'\n", str_tag(chd.id));
				r->skip(roundup(chd.size));
				rc = true;
		}

		if (!rc)
		{
			debug("failed to handle chunk %s\n", str_tag(chd.id));
			return false;
		}

	} while (chd.id != kFINF);

	for (int i = 0; i != loop_info.loop_count; ++i) {
		debug("LOOP %2d: %4d %4d %s\n", i,
			loop_info.loops[i].begin,
			loop_info.loops[i].end,
			loop_info.loops[i].name.c_str());
	}

	return true;
}

bool VQADecoder::read_vqhd(uint32 size)
{
	if (size != 42)
		return false;

	header.version     = r->readUint16LE();
	header.flags       = r->readUint16LE();
	header.numFrames   = r->readUint16LE();
	header.width       = r->readUint16LE();
	header.height      = r->readUint16LE();
	header.blockW      = r->readByte();
	header.blockH      = r->readByte();
	header.frameRate   = r->readByte();
	header.cbParts     = r->readByte();
	header.colors      = r->readUint16LE();
	header.maxBlocks   = r->readUint16LE();
	header.offset_x    = r->readUint16LE();
	header.offset_y    = r->readUint16LE();
	header.maxVPTRSize = r->readUint16LE();
	header.freq        = r->readUint16LE();
	header.channels    = r->readByte();
	header.bits        = r->readByte();
	header.unk3        = r->readUint32LE();
	header.unk4        = r->readUint16LE();
	header.maxCBFZSize = r->readUint32LE();
	header.unk5        = r->readUint32LE();

	if (header.offset_x || header.offset_y)
	{
		debug("header.offset_x, header.offset_y: %d %d\n", header.offset_x, header.offset_y);
	}

	// if (header.unk3 || header.unk4 != 4 || header.unk5 || header.flags != 0x0014)
	{
		debug("header.version      %d\n", header.version);
		debug("header.flags        %04x\n", header.flags);
		debug("header.numFrames    %d\n", header.numFrames);
		debug("header.width        %d\n", header.width);
		debug("header.height       %d\n", header.height);
		debug("header.blockW       %d\n", header.blockW);
		debug("header.blockH       %d\n", header.blockH);
		debug("header.frameRate    %d\n", header.frameRate);
		debug("header.cbParts      %d\n", header.cbParts);
		debug("header.colors       %d\n", header.colors);
		debug("header.maxBlocks    %d\n", header.maxBlocks);
		debug("header.offsetX      %d\n", header.offset_x);
		debug("header.offsetY      %d\n", header.offset_y);
		debug("header.maxVPTRSize  %d\n", header.maxVPTRSize);
		debug("header.freq         %d\n", header.freq);
		debug("header.channels     %d\n", header.channels);
		debug("header.bits         %d\n", header.bits);
		debug("header.unk3         %d\n", header.unk3);
		debug("header.unk4         %d\n", header.unk4);
		debug("header.maxCBFZSize  %d\n", header.maxCBFZSize);
		debug("header.unk5         %d\n", header.unk5);
	}

	// exit(-1);

	return true;
}

bool VQADecoder::read_msci(uint32 size)
{
	iff_chunk_header_s chd;
	read_iff_chunk_header(r, &chd);

	if (chd.id != kMSCH)
		return false;

	uint32 count, unk0;
	count = r->readUint32LE();
	unk0  = r->readUint32LE();
	assert(unk0 == 0);

	read_iff_chunk_header(r, &chd);
	if (chd.id != kMSCT || chd.size != count * 0x10)
		return false;

	for (uint32 i = 0; i < count; ++i)
	{
		uint32 tag, size;
		tag = r->readUint32BE();
		size = r->readUint32LE();

		switch (tag)
		{
		case kVIEW:
			max_view_chunk_size = size;
			debug("max VIEW size: %08x\n", max_view_chunk_size);
			break;
		case kZBUF:
			max_zbuf_chunk_size = size;
			zbuf_chunk = new uint8[roundup(max_zbuf_chunk_size)];
			debug("max ZBUF size: %08x\n", max_zbuf_chunk_size);
			break;
		case kAESC:
			max_aesc_chunk_size = size;
			debug("max AESC size: %08x\n", max_aesc_chunk_size);
			break;
		default:
			debug("Unknown tag in MSCT: %s\n", str_tag(tag));
		}

		uint32 zero;
		zero = r->readUint32LE(); assert(zero == 0);
		zero = r->readUint32LE(); assert(zero == 0);
	}

	return true;
}

bool VQADecoder::read_linf(uint32 size)
{
	iff_chunk_header_s chd;
	read_iff_chunk_header(r, &chd);

	if (chd.id != kLINH || chd.size != 6)
		return false;

	loop_info.loop_count = r->readUint16LE();
	loop_info.flags = r->readUint32LE();

	if ((loop_info.flags & 3) == 0)
		return false;

	read_iff_chunk_header(r, &chd);
	if (chd.id != kLIND || chd.size != 4u * loop_info.loop_count)
		return false;

	loop_info.loops = new Loop[loop_info.loop_count];
	for (int i = 0; i != loop_info.loop_count; ++i)
	{
		loop_info.loops[i].begin = r->readUint16LE();
		loop_info.loops[i].end = r->readUint16LE();

		// debug("Loop %d: %04x %04x\n", i, loop_info.loops[i].begin, loop_info.loops[i].end);
	}

	return true;
}

bool VQADecoder::read_cinf(uint32 size)
{
	iff_chunk_header_s chd;

	read_iff_chunk_header(r, &chd);
	if (chd.id != kCINH || chd.size != 8u)
		return false;

	clip_info.clip_count = r->readUint16LE();
	r->skip(6);

	read_iff_chunk_header(r, &chd);
	if (chd.id != kCIND || chd.size != 6u * clip_info.clip_count)
		return false;

	for (int i = 0; i != clip_info.clip_count; ++i)
	{
		uint16 a;
		uint32 b;
		a = r->readUint16LE();
		b = r->readUint32LE();
		debug("%4d %08x\n", a, b);
	}

	return true;
}

bool VQADecoder::read_finf(uint32 size)
{
	if (size != 4u * header.numFrames)
		return false;

	frame_info = new uint32[header.numFrames];

	for (uint32 i = 0; i != header.numFrames; ++i)
		frame_info[i] = r->readUint32LE();

	if (false) {
		uint32 last = 0;
		for (uint32 i = 0; i != header.numFrames; ++i)
		{
			uint32 diff = frame_info[i] - last;
			debug("frame_info[%4d] = 0x%08x   - %08x\n", i, frame_info[i], diff);
			last = frame_info[i];
		}
	}

	return true;
}

bool VQADecoder::read_lnin(uint32 size)
{
	iff_chunk_header_s chd;

	read_iff_chunk_header(r, &chd);
	if (chd.id != kLNIH || chd.size != 10)
		return false;

	uint16 loop_names_count, loop_unk_1, loop_unk_2, loop_unk_3, loop_unk_4;

	loop_names_count = r->readUint16LE();
	loop_unk_1       = r->readUint16LE();
	loop_unk_2       = r->readUint16LE();
	loop_unk_3       = r->readUint16LE();
	loop_unk_4       = r->readUint16LE();

	if (loop_names_count != loop_info.loop_count)
		return false;

	read_iff_chunk_header(r, &chd);
	if (chd.id != kLNIO || chd.size != 4u * loop_names_count)
		return false;

	uint32 *loop_name_offsets = (uint32*)alloca(loop_names_count * sizeof(uint32));
	for (int i = 0; i != loop_names_count; ++i) {
		loop_name_offsets[i] = r->readUint32LE();
	}

	read_iff_chunk_header(r, &chd);
	if (chd.id != kLNID)
		return false;

	char *names = (char*)alloca(roundup(chd.size));
	r->read(names, roundup(chd.size));

	for (int i = 0; i != loop_names_count; ++i) {
		char   *begin = names + loop_name_offsets[i];
		size_t  len   = ((i == loop_names_count) ? chd.size : loop_name_offsets[i+1]) - loop_name_offsets[i];

		loop_info.loops[i].name = Common::String(begin, len);
	}

	return true;
}

bool VQADecoder::read_clip(uint32 size)
{
	r->skip(roundup(size));
	return true;
}

bool VQADecoder::read_mfci(uint32 size)
{
	r->skip(roundup(size));
	return true;
}

int VQADecoder::read_frame()
{
	// debug("VQADecoder::read_frame(): %d, %d, %d, %d\n", loop_default, loop_special, cur_loop, cur_frame);

	if (loop_info.loop_count)
	{
		if (loop_special >= 0)
		{
			cur_loop = loop_special;
			loop_special = -1;

			cur_frame = loop_info.loops[cur_loop].begin;
			seek_to_frame(cur_frame);
		}
		else if (cur_loop == -1 && loop_default >= 0)
		{
			cur_loop = loop_default;
			cur_frame = loop_info.loops[cur_loop].begin;
			seek_to_frame(cur_frame);
		}
		else if (cur_loop >= -1 && cur_frame == loop_info.loops[cur_loop].end)
		{
			if (loop_default == -1)
				return -1;

			cur_loop = loop_default;
			cur_frame = loop_info.loops[cur_loop].begin;
			seek_to_frame(cur_frame);
		}
		else
			++cur_frame;
	}
	else
		++cur_frame;

	if (cur_frame >= header.numFrames)
		return -1;

	iff_chunk_header_s chd;

	has_view = false;

	if (stream_remain(r) < 8) {
		debug("remain: %d\n", stream_remain(r));
		return -1;
	}

	do {
		if (!read_iff_chunk_header(r, &chd)) {
			debug("Error reading chunk header\n");
			return -1;
		}

		// debug("%s ", str_tag(chd.id));

		bool rc = false;
		switch (chd.id)
		{
			case kAESC: rc = read_aesc(chd.size); break;
			case kLITE: rc = read_lite(chd.size); break;
			case kSN2J: rc = read_sn2j(chd.size); break;
			case kSND2: rc = read_snd2(chd.size); break;
			case kVIEW: rc = read_view(chd.size); break;
			case kVQFL: rc = read_vqfl(chd.size); break;
			case kVQFR: rc = read_vqfr(chd.size); break;
			case kZBUF: rc = read_zbuf(chd.size); break;
			default:
				r->skip(roundup(chd.size));
				rc = true;
		}

		if (!rc)
		{
			debug("Error handling chunk %s\n", str_tag(chd.id));
			return -1;
		}
	} while (chd.id != kVQFR);

	return cur_frame;
}


bool VQADecoder::read_sn2j(uint32 size)
{
	if (size != 6)
		return false;

	uint16 step_index;
	uint32 predictor;

	step_index = r->readUint16LE();
	predictor  = r->readUint32LE();

	// ima_adpcm_ws_decoder.set_parameters(step_index >> 5, predictor);

	return true;
}

bool VQADecoder::read_snd2(uint32 size)
{
	if (size != 735)
	{
		debug("audio frame size: %d\n", size);
		return false;
	}

	if (!audio_frame)
		audio_frame = new int16[2 * size];
	memset(audio_frame, 0, 4 * size);

	uint8 *in_frame = new uint8[roundup(size)];
	r->read(in_frame, roundup(size));

	// ima_adpcm_ws_decoder.decode(in_frame, size, audio_frame);

	delete[] in_frame;

	return true;
}

bool VQADecoder::read_vqfr(uint32 size)
{
	iff_chunk_header_s chd;

	while (size >= 8)
	{
		if (!read_iff_chunk_header(r, &chd))
			return false;
		size -= roundup(chd.size) + 8;

		// debug("(%s) ", str_tag(chd.id)); fflush(0);

		bool rc = false;
		switch (chd.id)
		{
			case kCBFZ: rc = read_cbfz(chd.size); break;
			case kVPTR: rc = read_vptr(chd.size); break;
			default:
				r->skip(roundup(chd.size));
		}

		if (!rc)
		{
			debug("VQFR: error handling chunk %s\n", str_tag(chd.id));
			return false;
		}
	}

	return true;
}

bool VQADecoder::read_vqfl(uint32 size)
{
	iff_chunk_header_s chd;

	while (size >= 8)
	{
		if (!read_iff_chunk_header(r, &chd))
			return false;
		size -= roundup(chd.size) + 8;

		bool rc = false;
		switch (chd.id)
		{
			case kCBFZ: rc = read_cbfz(chd.size); break;
			default:
				r->skip(roundup(chd.size));
		}

		if (!rc)
		{
			debug("VQFL: error handling chunk %s\n", str_tag(chd.id));
			return false;
		}
	}

	return true;
}

bool VQADecoder::read_cbfz(uint32 size)
{
	if (size > header.maxCBFZSize)
	{
		debug("%d > %d\n", size, header.maxCBFZSize);
		return false;
	}

	if (!codebook)
	{
		codebookSize = 2 * header.maxBlocks * header.blockW * header.blockH;
		codebook = new uint8[codebookSize];
	}
	if (!cbfz)
		cbfz = new uint8[roundup(header.maxCBFZSize)];

	r->read(cbfz, roundup(size));

	decompress_lcw(cbfz, size, codebook, codebookSize);

	return true;
}

static
int decodeZBUF_partial(uint8 *src, uint16 *cur_zbuf, uint32 src_len)
{
	uint32 dst_size = 640 * 480; // This is taken from global variables?
	uint32 dst_remain = dst_size;

	uint16 *curz_p = cur_zbuf;
	uint16 *in_p = (uint16*)src;

	while (dst_remain && (in_p - (uint16*)src) < (ptrdiff_t)src_len)
	{
		uint32 count = FROM_LE_16(*in_p++);

		if (count & 0x8000)
		{
			count = MIN(count & 0x7fff, dst_remain);
			dst_remain -= count;

			while (count--)
			{
				uint16 value = FROM_LE_16(*in_p++);
				if (value)
					*curz_p = value;
				++curz_p;
			}
		}
		else
		{
			count = MIN(count, dst_remain);
			dst_remain -= count;
			uint16 value = FROM_LE_16(*in_p++);

			if (!value)
				curz_p += count;
			else
			{
				while (count--)
					*curz_p++ = value;
			}
		}
	}
	return dst_size - dst_remain;
}

bool VQADecoder::read_zbuf(uint32 size)
{
	if (size > max_zbuf_chunk_size) {
		debug("VQA ERROR: ZBUF chunk size: %08x > %08x\n", size, max_zbuf_chunk_size);
		r->skip(roundup(size));
		return false;
	}

	uint32 width, height, complete, unk0;
	width    = r->readUint32LE();
	height   = r->readUint32LE();
	complete = r->readUint32LE();
	unk0     = r->readUint32LE();

	uint32 remain = size - 16;

	if (width != header.width || height != header.height)
	{
		debug("%d, %d, %d, %d\n", width, height, complete, unk0);
		r->skip(roundup(remain));
		return false;
	}

	if (!zbuf)
	{
		if (!complete) {
			r->skip(roundup(remain));
			return false;
		}
		zbuf = new uint16[width * height];
	}

	r->read(zbuf_chunk, roundup(remain));

	if (complete) {
		size_t zbuf_out_size;
		decompress_lzo1x(zbuf_chunk, remain, (uint8*)zbuf, &zbuf_out_size);
	} else {
		decodeZBUF_partial(zbuf_chunk, zbuf, remain);
	}

	return true;
}

bool VQADecoder::get_zbuf(uint16 *a_zbuf)
{
	if (!zbuf)
		return false;

	memcpy(a_zbuf, zbuf, 2 * header.width * header.height);
	return true;
}

bool VQADecoder::read_view(uint32 size)
{
	if (size != 56)
		return false;

	r->skip(size);
	// has_view = true;

	return true;
}

bool VQADecoder::read_aesc(uint32 size)
{
	r->skip(roundup(size));
	return true;
}

bool VQADecoder::read_lite(uint32 size)
{
	r->skip(roundup(size));
	return true;
}

bool VQADecoder::read_vptr(uint32 size)
{
	if (size > header.maxVPTRSize)
		return false;

	if (!vptr)
		vptr = new uint8[roundup(header.maxVPTRSize)];

	vptrSize = size;
	r->read(vptr, roundup(size));

	return true;
}

void VQADecoder::vptr_write_block(uint16 *frame, unsigned int dst_block, unsigned int src_block, int count, bool alpha) const
{
	uint16 frame_width  = header.width;
	uint32 frame_stride = 640;
	uint16 block_width  = header.blockW;
	uint16 block_height = header.blockH;

	const uint8 *const block_src =
		&codebook[2 * src_block * block_width * block_height];

	int blocks_per_line = frame_width / block_width;

	do
	{
		uint32 frame_x = dst_block % blocks_per_line * block_width  + header.offset_x / 2;
		uint32 frame_y = dst_block / blocks_per_line * block_height + header.offset_y;

		uint32 dst_offset = frame_x + frame_y * frame_stride;

		const uint8 *__restrict src = block_src;
		uint16      *__restrict dst = frame + dst_offset;

		unsigned int block_y;
		for (block_y = 0; block_y != block_height; ++block_y)
		{
			unsigned int block_x;
			for (block_x = 0; block_x != block_width; ++block_x)
			{
				uint16 rgb555 = src[0] | (src[1] << 8);
				src += 2;

				if (!(alpha && (rgb555 & 0x8000)))
					*dst = rgb555;
				++dst;
			}
			dst += frame_stride - block_width;
		}

		++dst_block;
	}
	while (--count);
}

void VQADecoder::set_loop_special(int loop, bool wait)
{
	loop_special = loop;
	if (!wait)
		cur_loop = -1;
}

void VQADecoder::set_loop_default(int loop)
{
	loop_default = loop;
}

bool VQADecoder::seek_to_frame(int frame)
{
	if (frame < 0 || frame >= header.numFrames)
		return false;

	r->seek(2 * (frame_info[frame] & 0x0fffffff), SEEK_SET);
	return true;
}

bool VQADecoder::decode_frame(uint16 *a_frame)
{
	if (!codebook || !vptr)
		return false;

	if (!frame)
		frame = new uint16[header.width * header.height];

	uint8 *src = vptr;
	uint8 *end = vptr + vptrSize;

	uint16 count, src_block, dst_block = 0;
	(void)src_block;

	while (end - src >= 2)
	{
		uint16 command = src[0] | (src[1] << 8);
		uint8  prefix = command >> 13;
		src += 2;

		switch (prefix)
		{
		case 0:
			count = command & 0x1fff;
			dst_block += count;
			break;
		case 1:
			count = 2 * (((command >> 8) & 0x1f) + 1);
			src_block = command & 0x00ff;

			vptr_write_block(frame, dst_block, src_block, count);
			dst_block += count;
			break;
		case 2:
			count = 2 * (((command >> 8) & 0x1f) + 1);
			src_block = command & 0x00ff;

			vptr_write_block(frame, dst_block, src_block, 1);
			++dst_block;

			for (int i = 0; i < count; ++i)
			{
				src_block = *src++;
				vptr_write_block(frame, dst_block, src_block, 1);
				++dst_block;
			}
			break;
		case 3:
		case 4:
			count = 1;
			src_block = command & 0x1fff;

			vptr_write_block(frame, dst_block, src_block, count, prefix == 4);
			++dst_block;
			break;
		case 5:
		case 6:
			count = *src++;
			src_block = command & 0x1fff;

			vptr_write_block(frame, dst_block, src_block, count, prefix == 6);
			dst_block += count;
			break;
		default:
			debug("Undefined case %d\n", command >> 13);
		}
	}

	memcpy(a_frame, frame, 2 * 640 * 480);

	return true;
}

int16 *VQADecoder::get_audio_frame()
{
	return audio_frame;
}

}; // End of namespace BladeRunner

#endif
