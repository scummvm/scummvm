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

VQADecoder::VQADecoder(Common::SeekableReadStream *s)
	: _s(s),
	  _frame(0),
	  _zbuf(0),
	  _codebook(0),
	  _cbfz(0),
	  _vptr(0),
	  _curFrame(-1),
	  _curLoop(-1),
	  _loopSpecial(-1),
	  _loopDefault(-1),
	  _hasView(false),
	  _audioFrame(0),
	  _maxVIEWChunkSize(0),
	  _maxZBUFChunkSize(0),
	  _maxAESCChunkSize(0)
{
}

VQADecoder::~VQADecoder()
{
}

struct IFFChunkHeader
{
	IFFChunkHeader()
		: id(0), size(0)
	{}

	uint32 id;
	uint32 size;
};

static inline uint32 roundup(uint32 v)
{
	return (v + 1) & ~1u;
}

const char *strTag(uint32 tag);

int32 remain(Common::SeekableReadStream *s) {
	int32 pos = s->pos();
	if (pos == -1) return -1;

	int32 size = s->size();
	if (size == -1) return -1;

	return size - pos;
}

static
bool readIFFChunkHeader(Common::SeekableReadStream *s, IFFChunkHeader *ts)
{
	if (remain(s) < 8)
		return false;

	ts->id   = s->readUint32BE();
	ts->size = s->readUint32BE();

	// if (ts->size != roundup(ts->size))
	// 	debug("%s: %d\n", strTag(ts->id), ts->size);

	return true;
}

const char *strTag(uint32 tag)
{
	static char s[5];

	sprintf(s, "%c%c%c%c",
		(tag >> 24) & 0xff,
		(tag >> 16) & 0xff,
		(tag >>  8) & 0xff,
		(tag >>  0) & 0xff);

	return s;
}

bool VQADecoder::readHeader()
{
	IFFChunkHeader chd;
	uint32 type;
	bool rc;

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kFORM || !chd.size)
		return false;

	type = _s->readUint32BE();

	if (type != kWVQA)
		return false;

	do {
		if (!readIFFChunkHeader(_s, &chd))
			return false;

		debug("\t%s : %x\n", strTag(chd.id), chd.size);

		rc = false;
		switch (chd.id)
		{
			case kCINF: rc = readCINF(chd.size); break;
			case kCLIP: rc = readCLIP(chd.size); break;
			case kFINF: rc = readFINF(chd.size); break;
			case kLINF: rc = readLINF(chd.size); break;
			case kLNIN: rc = readLNIN(chd.size); break;
			case kMFCI: rc = readMFCI(chd.size); break;
			case kMSCI: rc = readMSCI(chd.size); break;
			case kVQHD: rc = readVQHD(chd.size); break;
			default:
				debug("Unhandled chunk '%s'\n", strTag(chd.id));
				_s->skip(roundup(chd.size));
				rc = true;
		}

		if (!rc)
		{
			debug("failed to handle chunk %s\n", strTag(chd.id));
			return false;
		}

	} while (chd.id != kFINF);

	for (int i = 0; i != _loopInfo.loopCount; ++i) {
		debug("LOOP %2d: %4d %4d %s\n", i,
			_loopInfo.loops[i].begin,
			_loopInfo.loops[i].end,
			_loopInfo.loops[i].name.c_str());
	}

	return true;
}

bool VQADecoder::readVQHD(uint32 size)
{
	if (size != 42)
		return false;

	_header.version     = _s->readUint16LE();
	_header.flags       = _s->readUint16LE();
	_header.numFrames   = _s->readUint16LE();
	_header.width       = _s->readUint16LE();
	_header.height      = _s->readUint16LE();
	_header.blockW      = _s->readByte();
	_header.blockH      = _s->readByte();
	_header.frameRate   = _s->readByte();
	_header.cbParts     = _s->readByte();
	_header.colors      = _s->readUint16LE();
	_header.maxBlocks   = _s->readUint16LE();
	_header.offset_x    = _s->readUint16LE();
	_header.offset_y    = _s->readUint16LE();
	_header.maxVPTRSize = _s->readUint16LE();
	_header.freq        = _s->readUint16LE();
	_header.channels    = _s->readByte();
	_header.bits        = _s->readByte();
	_header.unk3        = _s->readUint32LE();
	_header.unk4        = _s->readUint16LE();
	_header.maxCBFZSize = _s->readUint32LE();
	_header.unk5        = _s->readUint32LE();

	if (_header.offset_x || _header.offset_y)
	{
		debug("_header.offset_x, _header.offset_y: %d %d\n", _header.offset_x, _header.offset_y);
	}

	// if (_header.unk3 || _header.unk4 != 4 || _header.unk5 || _header.flags != 0x0014)
	{
		debug("_header.version      %d\n", _header.version);
		debug("_header.flags        %04x\n", _header.flags);
		debug("_header.numFrames    %d\n", _header.numFrames);
		debug("_header.width        %d\n", _header.width);
		debug("_header.height       %d\n", _header.height);
		debug("_header.blockW       %d\n", _header.blockW);
		debug("_header.blockH       %d\n", _header.blockH);
		debug("_header.frameRate    %d\n", _header.frameRate);
		debug("_header.cbParts      %d\n", _header.cbParts);
		debug("_header.colors       %d\n", _header.colors);
		debug("_header.maxBlocks    %d\n", _header.maxBlocks);
		debug("_header.offsetX      %d\n", _header.offset_x);
		debug("_header.offsetY      %d\n", _header.offset_y);
		debug("_header.maxVPTRSize  %d\n", _header.maxVPTRSize);
		debug("_header.freq         %d\n", _header.freq);
		debug("_header.channels     %d\n", _header.channels);
		debug("_header.bits         %d\n", _header.bits);
		debug("_header.unk3         %d\n", _header.unk3);
		debug("_header.unk4         %d\n", _header.unk4);
		debug("_header.maxCBFZSize  %d\n", _header.maxCBFZSize);
		debug("_header.unk5         %d\n", _header.unk5);
	}

	// exit(-1);

	return true;
}

bool VQADecoder::readMSCI(uint32 size)
{
	IFFChunkHeader chd;
	readIFFChunkHeader(_s, &chd);

	if (chd.id != kMSCH)
		return false;

	uint32 count, unk0;
	count = _s->readUint32LE();
	unk0  = _s->readUint32LE();
	assert(unk0 == 0);

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kMSCT || chd.size != count * 0x10)
		return false;

	for (uint32 i = 0; i < count; ++i)
	{
		uint32 tag, size;
		tag  = _s->readUint32BE();
		size = _s->readUint32LE();

		switch (tag)
		{
		case kVIEW:
			_maxVIEWChunkSize = size;
			debug("max VIEW size: %08x\n", _maxVIEWChunkSize);
			break;
		case kZBUF:
			_maxZBUFChunkSize = size;
			_zbufChunk = new uint8[roundup(_maxZBUFChunkSize)];
			debug("max ZBUF size: %08x\n", _maxZBUFChunkSize);
			break;
		case kAESC:
			_maxAESCChunkSize = size;
			debug("max AESC size: %08x\n", _maxAESCChunkSize);
			break;
		default:
			debug("Unknown tag in MSCT: %s\n", strTag(tag));
		}

		uint32 zero;
		zero = _s->readUint32LE(); assert(zero == 0);
		zero = _s->readUint32LE(); assert(zero == 0);
	}

	return true;
}

bool VQADecoder::readLINF(uint32 size)
{
	IFFChunkHeader chd;
	readIFFChunkHeader(_s, &chd);

	if (chd.id != kLINH || chd.size != 6)
		return false;

	_loopInfo.loopCount = _s->readUint16LE();
	_loopInfo.flags = _s->readUint32LE();

	if ((_loopInfo.flags & 3) == 0)
		return false;

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kLIND || chd.size != 4u * _loopInfo.loopCount)
		return false;

	_loopInfo.loops = new Loop[_loopInfo.loopCount];
	for (int i = 0; i != _loopInfo.loopCount; ++i)
	{
		_loopInfo.loops[i].begin = _s->readUint16LE();
		_loopInfo.loops[i].end   = _s->readUint16LE();

		// debug("Loop %d: %04x %04x\n", i, _loopInfo.loops[i].begin, _loopInfo.loops[i].end);
	}

	return true;
}

bool VQADecoder::readCINF(uint32 size)
{
	IFFChunkHeader chd;

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kCINH || chd.size != 8u)
		return false;

	_clipInfo.clipCount = _s->readUint16LE();
	_s->skip(6);

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kCIND || chd.size != 6u * _clipInfo.clipCount)
		return false;

	for (int i = 0; i != _clipInfo.clipCount; ++i)
	{
		uint16 a;
		uint32 b;
		a = _s->readUint16LE();
		b = _s->readUint32LE();
		debug("%4d %08x\n", a, b);
	}

	return true;
}

bool VQADecoder::readFINF(uint32 size)
{
	if (size != 4u * _header.numFrames)
		return false;

	_frameInfo = new uint32[_header.numFrames];

	for (uint32 i = 0; i != _header.numFrames; ++i)
		_frameInfo[i] = _s->readUint32LE();

	if (false) {
		uint32 last = 0;
		for (uint32 i = 0; i != _header.numFrames; ++i)
		{
			uint32 diff = _frameInfo[i] - last;
			debug("_frameInfo[%4d] = 0x%08x   - %08x\n", i, _frameInfo[i], diff);
			last = _frameInfo[i];
		}
	}

	return true;
}

bool VQADecoder::readLNIN(uint32 size)
{
	IFFChunkHeader chd;

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kLNIH || chd.size != 10)
		return false;

	uint16 loopNamesCount, loopUnk1, loopUnk2, loopUnk3, loopUnk4;

	loopNamesCount = _s->readUint16LE();
	loopUnk1       = _s->readUint16LE();
	loopUnk2       = _s->readUint16LE();
	loopUnk3       = _s->readUint16LE();
	loopUnk4       = _s->readUint16LE();

	if (loopNamesCount != _loopInfo.loopCount)
		return false;

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kLNIO || chd.size != 4u * loopNamesCount)
		return false;

	uint32 *loopNameOffsets = (uint32*)alloca(loopNamesCount * sizeof(uint32));
	for (int i = 0; i != loopNamesCount; ++i) {
		loopNameOffsets[i] = _s->readUint32LE();
	}

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kLNID)
		return false;

	char *names = (char*)alloca(roundup(chd.size));
	_s->read(names, roundup(chd.size));

	for (int i = 0; i != loopNamesCount; ++i) {
		char   *begin = names + loopNameOffsets[i];
		size_t  len   = ((i == loopNamesCount) ? chd.size : loopNameOffsets[i+1]) - loopNameOffsets[i];

		_loopInfo.loops[i].name = Common::String(begin, len);
	}

	return true;
}

bool VQADecoder::readCLIP(uint32 size)
{
	_s->skip(roundup(size));
	return true;
}

bool VQADecoder::readMFCI(uint32 size)
{
	_s->skip(roundup(size));
	return true;
}

int VQADecoder::readFrame()
{
	// debug("VQADecoder::readFrame(): %d, %d, %d, %d\n", _loopDefault, _loopSpecial, _curLoop, _curFrame);

	if (_loopInfo.loopCount)
	{
		if (_loopSpecial >= 0)
		{
			_curLoop = _loopSpecial;
			_loopSpecial = -1;

			_curFrame = _loopInfo.loops[_curLoop].begin;
			seekToFrame(_curFrame);
		}
		else if (_curLoop == -1 && _loopDefault >= 0)
		{
			_curLoop = _loopDefault;
			_curFrame = _loopInfo.loops[_curLoop].begin;
			seekToFrame(_curFrame);
		}
		else if (_curLoop >= -1 && _curFrame == _loopInfo.loops[_curLoop].end)
		{
			if (_loopDefault == -1)
				return -1;

			_curLoop = _loopDefault;
			_curFrame = _loopInfo.loops[_curLoop].begin;
			seekToFrame(_curFrame);
		}
		else
			++_curFrame;
	}
	else
		++_curFrame;

	if (_curFrame >= _header.numFrames)
		return -1;

	IFFChunkHeader chd;

	_hasView = false;

	if (remain(_s) < 8) {
		debug("remain: %d\n", remain(_s));
		return -1;
	}

	do {
		if (!readIFFChunkHeader(_s, &chd)) {
			debug("Error reading chunk header\n");
			return -1;
		}

		// debug("%s ", strTag(chd.id));

		bool rc = false;
		switch (chd.id)
		{
			case kAESC: rc = readAESC(chd.size); break;
			case kLITE: rc = readLITE(chd.size); break;
			case kSN2J: rc = readSN2J(chd.size); break;
			case kSND2: rc = readSND2(chd.size); break;
			case kVIEW: rc = readVIEW(chd.size); break;
			case kVQFL: rc = readVQFL(chd.size); break;
			case kVQFR: rc = readVQFR(chd.size); break;
			case kZBUF: rc = readZBUF(chd.size); break;
			default:
				_s->skip(roundup(chd.size));
				rc = true;
		}

		if (!rc)
		{
			debug("Error handling chunk %s\n", strTag(chd.id));
			return -1;
		}
	} while (chd.id != kVQFR);

	return _curFrame;
}


bool VQADecoder::readSN2J(uint32 size)
{
	if (size != 6)
		return false;

	uint16 step_index;
	uint32 predictor;

	step_index = _s->readUint16LE();
	predictor  = _s->readUint32LE();

	// ima_adpcm_ws_decoder.set_parameters(step_index >> 5, predictor);

	return true;
}

bool VQADecoder::readSND2(uint32 size)
{
	if (size != 735)
	{
		debug("audio frame size: %d\n", size);
		return false;
	}

	if (!_audioFrame)
		_audioFrame = new int16[2 * size];
	memset(_audioFrame, 0, 4 * size);

	uint8 *inFrame = new uint8[roundup(size)];
	_s->read(inFrame, roundup(size));

	// ima_adpcm_ws_decoder.decode(inFrame, size, _audioFrame);

	delete[] inFrame;

	return true;
}

bool VQADecoder::readVQFR(uint32 size)
{
	IFFChunkHeader chd;

	while (size >= 8)
	{
		if (!readIFFChunkHeader(_s, &chd))
			return false;
		size -= roundup(chd.size) + 8;

		// debug("(%s) ", strTag(chd.id)); fflush(0);

		bool rc = false;
		switch (chd.id)
		{
			case kCBFZ: rc = readCBFZ(chd.size); break;
			case kVPTR: rc = readVPTR(chd.size); break;
			default:
				_s->skip(roundup(chd.size));
		}

		if (!rc)
		{
			debug("VQFR: error handling chunk %s\n", strTag(chd.id));
			return false;
		}
	}

	return true;
}

bool VQADecoder::readVQFL(uint32 size)
{
	IFFChunkHeader chd;

	while (size >= 8)
	{
		if (!readIFFChunkHeader(_s, &chd))
			return false;
		size -= roundup(chd.size) + 8;

		bool rc = false;
		switch (chd.id)
		{
			case kCBFZ: rc = readCBFZ(chd.size); break;
			default:
				_s->skip(roundup(chd.size));
		}

		if (!rc)
		{
			debug("VQFL: error handling chunk %s\n", strTag(chd.id));
			return false;
		}
	}

	return true;
}

bool VQADecoder::readCBFZ(uint32 size)
{
	if (size > _header.maxCBFZSize)
	{
		debug("%d > %d\n", size, _header.maxCBFZSize);
		return false;
	}

	if (!_codebook)
	{
		_codebookSize = 2 * _header.maxBlocks * _header.blockW * _header.blockH;
		_codebook = new uint8[_codebookSize];
	}
	if (!_cbfz)
		_cbfz = new uint8[roundup(_header.maxCBFZSize)];

	_s->read(_cbfz, roundup(size));

	decompress_lcw(_cbfz, size, _codebook, _codebookSize);

	return true;
}

static
int decodeZBUF_partial(uint8 *src, uint16 *curZBUF, uint32 srcLen)
{
	uint32 dstSize = 640 * 480; // This is taken from global variables?
	uint32 dstRemain = dstSize;

	uint16 *curzp = curZBUF;
	uint16 *inp = (uint16*)src;

	while (dstRemain && (inp - (uint16*)src) < (ptrdiff_t)srcLen)
	{
		uint32 count = FROM_LE_16(*inp++);

		if (count & 0x8000)
		{
			count = MIN(count & 0x7fff, dstRemain);
			dstRemain -= count;

			while (count--)
			{
				uint16 value = FROM_LE_16(*inp++);
				if (value)
					*curzp = value;
				++curzp;
			}
		}
		else
		{
			count = MIN(count, dstRemain);
			dstRemain -= count;
			uint16 value = FROM_LE_16(*inp++);

			if (!value)
				curzp += count;
			else
			{
				while (count--)
					*curzp++ = value;
			}
		}
	}
	return dstSize - dstRemain;
}

bool VQADecoder::readZBUF(uint32 size)
{
	if (size > _maxZBUFChunkSize) {
		debug("VQA ERROR: ZBUF chunk size: %08x > %08x\n", size, _maxZBUFChunkSize);
		_s->skip(roundup(size));
		return false;
	}

	uint32 width, height, complete, unk0;
	width    = _s->readUint32LE();
	height   = _s->readUint32LE();
	complete = _s->readUint32LE();
	unk0     = _s->readUint32LE();

	uint32 remain = size - 16;

	if (width != _header.width || height != _header.height)
	{
		debug("%d, %d, %d, %d\n", width, height, complete, unk0);
		_s->skip(roundup(remain));
		return false;
	}

	if (!_zbuf)
	{
		if (!complete) {
			_s->skip(roundup(remain));
			return false;
		}
		_zbuf = new uint16[width * height];
	}

	_s->read(_zbufChunk, roundup(remain));

	if (complete) {
		size_t zbufOutSize;
		decompress_lzo1x(_zbufChunk, remain, (uint8*)_zbuf, &zbufOutSize);
	} else {
		decodeZBUF_partial(_zbufChunk, _zbuf, remain);
	}

	return true;
}

bool VQADecoder::getZBUF(uint16 *zbuf)
{
	if (!_zbuf)
		return false;

	memcpy(zbuf, _zbuf, 2 * _header.width * _header.height);
	return true;
}

bool VQADecoder::readVIEW(uint32 size)
{
	if (size != 56)
		return false;

	_s->skip(size);
	// _hasView = true;

	return true;
}

bool VQADecoder::readAESC(uint32 size)
{
	_s->skip(roundup(size));
	return true;
}

bool VQADecoder::readLITE(uint32 size)
{
	_s->skip(roundup(size));
	return true;
}

bool VQADecoder::readVPTR(uint32 size)
{
	if (size > _header.maxVPTRSize)
		return false;

	if (!_vptr)
		_vptr = new uint8[roundup(_header.maxVPTRSize)];

	_vptrSize = size;
	_s->read(_vptr, roundup(size));

	return true;
}

void VQADecoder::VPTRWriteBlock(uint16 *frame, unsigned int dstBlock, unsigned int srcBlock, int count, bool alpha) const
{
	uint16 frame_width  = _header.width;
	uint32 frame_stride = 640;
	uint16 block_width  = _header.blockW;
	uint16 block_height = _header.blockH;

	const uint8 *const block_src =
		&_codebook[2 * srcBlock * block_width * block_height];

	int blocks_per_line = frame_width / block_width;

	do
	{
		uint32 frame_x = dstBlock % blocks_per_line * block_width  + _header.offset_x / 2;
		uint32 frame_y = dstBlock / blocks_per_line * block_height + _header.offset_y;

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

		++dstBlock;
	}
	while (--count);
}

void VQADecoder::setLoopSpecial(int loop, bool wait)
{
	_loopSpecial = loop;
	if (!wait)
		_curLoop = -1;
}

void VQADecoder::setLoopDefault(int loop)
{
	_loopDefault = loop;
}

bool VQADecoder::seekToFrame(int frame)
{
	if (frame < 0 || frame >= _header.numFrames)
		return false;

	_s->seek(2 * (_frameInfo[frame] & 0x0fffffff), SEEK_SET);
	return true;
}

bool VQADecoder::decodeFrame(uint16 *frame)
{
	if (!_codebook || !_vptr)
		return false;

	if (!_frame)
		_frame = new uint16[_header.width * _header.height];

	uint8 *src = _vptr;
	uint8 *end = _vptr + _vptrSize;

	uint16 count, srcBlock, dstBlock = 0;
	(void)srcBlock;

	while (end - src >= 2)
	{
		uint16 command = src[0] | (src[1] << 8);
		uint8  prefix = command >> 13;
		src += 2;

		switch (prefix)
		{
		case 0:
			count = command & 0x1fff;
			dstBlock += count;
			break;
		case 1:
			count = 2 * (((command >> 8) & 0x1f) + 1);
			srcBlock = command & 0x00ff;

			VPTRWriteBlock(_frame, dstBlock, srcBlock, count);
			dstBlock += count;
			break;
		case 2:
			count = 2 * (((command >> 8) & 0x1f) + 1);
			srcBlock = command & 0x00ff;

			VPTRWriteBlock(_frame, dstBlock, srcBlock, 1);
			++dstBlock;

			for (int i = 0; i < count; ++i)
			{
				srcBlock = *src++;
				VPTRWriteBlock(_frame, dstBlock, srcBlock, 1);
				++dstBlock;
			}
			break;
		case 3:
		case 4:
			count = 1;
			srcBlock = command & 0x1fff;

			VPTRWriteBlock(_frame, dstBlock, srcBlock, count, prefix == 4);
			++dstBlock;
			break;
		case 5:
		case 6:
			count = *src++;
			srcBlock = command & 0x1fff;

			VPTRWriteBlock(_frame, dstBlock, srcBlock, count, prefix == 6);
			dstBlock += count;
			break;
		default:
			debug("Undefined case %d\n", command >> 13);
		}
	}

	memcpy(frame, _frame, 2 * 640 * 480);

	return true;
}

int16 *VQADecoder::getAudioFrame()
{
	return _audioFrame;
}

}; // End of namespace BladeRunner

#endif
