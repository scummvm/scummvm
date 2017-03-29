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

#include "bladerunner/vqa_decoder.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/decompress_lcw.h"
#include "bladerunner/decompress_lzo.h"
#include "bladerunner/lights.h"
#include "bladerunner/view.h"
#include "bladerunner/zbuffer.h"

#include "audio/decoders/raw.h"

#include "common/array.h"
#include "common/util.h"
#include "common/memstream.h"

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

int32 remain(Common::SeekableReadStream *s) {
	int32 pos = s->pos();
	if (pos == -1) return -1;

	int32 size = s->size();
	if (size == -1) return -1;

	return size - pos;
}

struct IFFChunkHeader {
	IFFChunkHeader()
		: id(0), size(0)
	{}

	uint32 id;
	uint32 size;
};

static bool readIFFChunkHeader(Common::SeekableReadStream *s, IFFChunkHeader *ts) {
	if (remain(s) < 8)
		return false;

	ts->id   = s->readUint32BE();
	ts->size = s->readUint32BE();

	return true;
}

static inline uint32 roundup(uint32 v) {
	return (v + 1) & ~1u;
}

const char *strTag(uint32 tag) {
	static char s[5];

	sprintf(s, "%c%c%c%c",
		(tag >> 24) & 0xff,
		(tag >> 16) & 0xff,
		(tag >>  8) & 0xff,
		(tag >>  0) & 0xff);

	return s;
}

VQADecoder::VQADecoder() : _s(nullptr),
	  _frameInfo(nullptr),
	  _videoTrack(nullptr),
	  _audioTrack(nullptr),
	  _maxVIEWChunkSize(0),
	  _maxZBUFChunkSize(0),
	  _maxAESCChunkSize(0) {
}

VQADecoder::~VQADecoder() {
	delete _audioTrack;
	delete _videoTrack;
	delete[] _frameInfo;
}

bool VQADecoder::loadStream(Common::SeekableReadStream *s) {
	// close();
	_s = s;

	IFFChunkHeader chd;
	uint32 type;
	bool rc;

	readIFFChunkHeader(s, &chd);
	if (chd.id != kFORM || !chd.size)
		return false;

	type = s->readUint32BE();

	if (type != kWVQA)
		return false;

	do {
		if (!readIFFChunkHeader(_s, &chd))
			return false;

		rc = false;
		switch (chd.id) {
		case kCINF: rc = readCINF(s, chd.size); break;
		case kCLIP: rc = readCLIP(s, chd.size); break;
		case kFINF: rc = readFINF(s, chd.size); break;
		case kLINF: rc = readLINF(s, chd.size); break;
		case kLNIN: rc = readLNIN(s, chd.size); break;
		case kMFCI: rc = readMFCI(s, chd.size); break;
		case kMSCI: rc = readMSCI(s, chd.size); break;
		case kVQHD: rc = readVQHD(s, chd.size); break;
		default:
			warning("Unhandled chunk '%s'", strTag(chd.id));
			s->skip(roundup(chd.size));
			rc = true;
		}

		if (!rc) {
			warning("failed to handle chunk %s", strTag(chd.id));
			return false;
		}
	} while (chd.id != kFINF);

	_videoTrack = new VQAVideoTrack(this);
	_audioTrack = new VQAAudioTrack(this);

#if 0
	for (int i = 0; i != _loopInfo.loopCount; ++i) {
		debug("LOOP %2d: %4d %4d %s", i,
			_loopInfo.loops[i].begin,
			_loopInfo.loops[i].end,
			_loopInfo.loops[i].name.c_str());
	}
#endif

	return true;
}

const Graphics::Surface *VQADecoder::decodeVideoFrame() {
	return _videoTrack->decodeVideoFrame();
}

void VQADecoder::decodeZBuffer(ZBuffer *zbuffer) {
	_videoTrack->decodeZBuffer(zbuffer);
}

Audio::SeekableAudioStream *VQADecoder::decodeAudioFrame() {
	return _audioTrack->decodeAudioFrame();
}

void VQADecoder::decodeView(View *view) {
	_videoTrack->decodeView(view);
}

void VQADecoder::decodeLights(Lights *lights) {
	_videoTrack->decodeLights(lights);
}

void VQADecoder::readPacket(int skipFlags) {
	IFFChunkHeader chd;

	if (remain(_s) < 8) {
		warning("remain: %d", remain(_s));
		assert(remain(_s) < 8);
	}

	do {
		if (!readIFFChunkHeader(_s, &chd)) {
			warning("Error reading chunk header");
			return;
		}

		bool rc = false;
		// Video track
		switch (chd.id) {
		case kAESC: rc = skipFlags & 1 ? _s->skip(roundup(chd.size)) : _videoTrack->readAESC(_s, chd.size); break;
		case kLITE: rc = skipFlags & 1 ? _s->skip(roundup(chd.size)) : _videoTrack->readLITE(_s, chd.size); break;
		case kVIEW: rc = skipFlags & 1 ? _s->skip(roundup(chd.size)) : _videoTrack->readVIEW(_s, chd.size); break;
		case kVQFL: rc = skipFlags & 1 ? _s->skip(roundup(chd.size)) : _videoTrack->readVQFL(_s, chd.size); break;
		case kVQFR: rc = skipFlags & 1 ? _s->skip(roundup(chd.size)) : _videoTrack->readVQFR(_s, chd.size); break;
		case kZBUF: rc = skipFlags & 1 ? _s->skip(roundup(chd.size)) : _videoTrack->readZBUF(_s, chd.size); break;
		// Sound track
		case kSN2J: rc = skipFlags & 2 ? _s->skip(roundup(chd.size)) : _audioTrack->readSN2J(_s, chd.size); break;
		case kSND2: rc = skipFlags & 2 ? _s->skip(roundup(chd.size)) : _audioTrack->readSND2(_s, chd.size); break;
		default:
			rc = false;
			_s->skip(roundup(chd.size));
		}

		if (!rc) {
			warning("Error handling chunk %s", strTag(chd.id));
			return;
		}
	} while (chd.id != kVQFR);
}

void VQADecoder::readFrame(int frame, int skipFlags) {
	if (frame < 0 || frame >= numFrames()) {
		error("frame %d out of bounds, frame count is %d", frame, numFrames());
	}

	uint32 frameOffset = 2 * (_frameInfo[frame] & 0x0FFFFFFF);
	_s->seek(frameOffset);
	readPacket(skipFlags);
}

bool VQADecoder::readVQHD(Common::SeekableReadStream *s, uint32 size) {
	if (size != 42)
		return false;

	_header.version     = s->readUint16LE();
	_header.flags       = s->readUint16LE();
	_header.numFrames   = s->readUint16LE();
	_header.width       = s->readUint16LE();
	_header.height      = s->readUint16LE();
	_header.blockW      = s->readByte();
	_header.blockH      = s->readByte();
	_header.frameRate   = s->readByte();
	_header.cbParts     = s->readByte();
	_header.colors      = s->readUint16LE();
	_header.maxBlocks   = s->readUint16LE();
	_header.offsetX     = s->readUint16LE();
	_header.offsetY     = s->readUint16LE();
	_header.maxVPTRSize = s->readUint16LE();
	_header.freq        = s->readUint16LE();
	_header.channels    = s->readByte();
	_header.bits        = s->readByte();
	_header.unk3        = s->readUint32LE();
	_header.unk4        = s->readUint16LE();
	_header.maxCBFZSize = s->readUint32LE();
	_header.unk5        = s->readUint32LE();

	if (_header.offsetX || _header.offsetY) {
		debug("_header.offsetX, _header.offsetY: %d %d", _header.offsetX, _header.offsetY);
	}

	// if (_header.unk3 || _header.unk4 != 4 || _header.unk5 || _header.flags != 0x0014)
	if (false) {
		debug("_header.version      %d", _header.version);
		debug("_header.flags        %04x", _header.flags);
		debug("_header.numFrames    %d", _header.numFrames);
		debug("_header.width        %d", _header.width);
		debug("_header.height       %d", _header.height);
		debug("_header.blockW       %d", _header.blockW);
		debug("_header.blockH       %d", _header.blockH);
		debug("_header.frameRate    %d", _header.frameRate);
		debug("_header.cbParts      %d", _header.cbParts);
		debug("_header.colors       %d", _header.colors);
		debug("_header.maxBlocks    %d", _header.maxBlocks);
		debug("_header.offsetX      %d", _header.offsetX);
		debug("_header.offsetY      %d", _header.offsetY);
		debug("_header.maxVPTRSize  %d", _header.maxVPTRSize);
		debug("_header.freq         %d", _header.freq);
		debug("_header.channels     %d", _header.channels);
		debug("_header.bits         %d", _header.bits);
		debug("_header.unk3         %d", _header.unk3);
		debug("_header.unk4         %d", _header.unk4);
		debug("_header.maxCBFZSize  %d", _header.maxCBFZSize);
		debug("_header.unk5         %d", _header.unk5);
		debug("\n");
	}

	assert(_header.version == 2);
	if (_header.channels != 0) {
		assert(_header.freq == 22050);
		assert(_header.channels == 1);
		assert(_header.bits == 16);
	}
	assert(_header.colors == 0);

	return true;
}

bool VQADecoder::VQAVideoTrack::readVQFR(Common::SeekableReadStream *s, uint32 size) {
	IFFChunkHeader chd;

	while (size >= 8) {
		if (!readIFFChunkHeader(s, &chd))
			return false;
		size -= roundup(chd.size) + 8;

		bool rc = false;
		switch (chd.id) {
		case kCBFZ: rc = readCBFZ(s, chd.size); break;
		case kVPTR: rc = readVPTR(s, chd.size); break;
		default:
			s->skip(roundup(chd.size));
		}

		if (!rc) {
			debug("VQFR: error handling chunk %s", strTag(chd.id));
			return false;
		}
	}

	return true;
}

bool VQADecoder::readMSCI(Common::SeekableReadStream *s, uint32 size) {
	IFFChunkHeader chd;
	readIFFChunkHeader(_s, &chd);

	if (chd.id != kMSCH)
		return false;

	uint32 count, unk0;
	count = s->readUint32LE();
	unk0  = s->readUint32LE();
	assert(unk0 == 0);

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kMSCT || chd.size != count * 0x10)
		return false;

	for (uint32 i = 0; i < count; ++i) {
		uint32 tag, max_size;
		tag  = s->readUint32BE();
		max_size = s->readUint32LE();

		switch (tag) {
		case kVIEW:
			_maxVIEWChunkSize = max_size;
			break;
		case kZBUF:
			_maxZBUFChunkSize = max_size;
			break;
		case kAESC:
			_maxAESCChunkSize = max_size;
			break;
		default:
			warning("Unknown tag in MSCT: %s", strTag(tag));
		}

		uint32 zero;
		zero = s->readUint32LE(); assert(zero == 0);
		zero = s->readUint32LE(); assert(zero == 0);
	}

	return true;
}

bool VQADecoder::readLINF(Common::SeekableReadStream *s, uint32 size) {
	IFFChunkHeader chd;
	readIFFChunkHeader(_s, &chd);

	if (chd.id != kLINH || chd.size != 6)
		return false;

	_loopInfo.loopCount = s->readUint16LE();
	_loopInfo.flags = s->readUint32LE();

	if ((_loopInfo.flags & 3) == 0)
		return false;

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kLIND || chd.size != 4u * _loopInfo.loopCount)
		return false;

	_loopInfo.loops = new Loop[_loopInfo.loopCount];
	for (int i = 0; i != _loopInfo.loopCount; ++i) {
		_loopInfo.loops[i].begin = s->readUint16LE();
		_loopInfo.loops[i].end   = s->readUint16LE();

		// debug("Loop %d: %04x %04x", i, _loopInfo.loops[i].begin, _loopInfo.loops[i].end);
	}

	return true;
}

bool VQADecoder::readCINF(Common::SeekableReadStream *s, uint32 size) {
	IFFChunkHeader chd;

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kCINH || chd.size != 8u)
		return false;

	_clipInfo.clipCount = s->readUint16LE();
	s->skip(6);

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kCIND || chd.size != 6u * _clipInfo.clipCount)
		return false;

	for (int i = 0; i != _clipInfo.clipCount; ++i) {
		uint16 a = s->readUint16LE();
		uint32 b = s->readUint32LE();
		debug("VQADecoder::readCINF() i: %d a: 0x%04x b: 0x%08x", i, a, b);
	}

	return true;
}

bool VQADecoder::readFINF(Common::SeekableReadStream *s, uint32 size) {
	if (size != 4u * _header.numFrames)
		return false;

	_frameInfo = new uint32[_header.numFrames];

	for (uint32 i = 0; i != _header.numFrames; ++i)
		_frameInfo[i] = s->readUint32LE();

	if (false) {
		uint32 last = 0;
		for (uint32 i = 0; i != _header.numFrames; ++i) {
			uint32 diff = _frameInfo[i] - last;
			debug("_frameInfo[%4d] = 0x%08x   - %08x", i, _frameInfo[i], diff);
			last = _frameInfo[i];
		}
	}

	return true;
}

bool VQADecoder::readLNIN(Common::SeekableReadStream *s, uint32 size) {
	IFFChunkHeader chd;

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kLNIH || chd.size != 10)
		return false;

	uint16 loopNamesCount = s->readUint16LE();
	uint16 loopUnk1       = s->readUint16LE();
	uint16 loopUnk2       = s->readUint16LE();
	uint16 loopUnk3       = s->readUint16LE();
	uint16 loopUnk4       = s->readUint16LE();

	debug("VQADecoder::readLNIN() Unknown Values: 0x%04x 0x%04x 0x%04x 0x%04x", loopUnk1, loopUnk2, loopUnk3, loopUnk4);

	if (loopNamesCount != _loopInfo.loopCount)
		return false;

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kLNIO || chd.size != 4u * loopNamesCount)
		return false;

	uint32 *loopNameOffsets = (uint32*)malloc(loopNamesCount * sizeof(uint32));
	for (int i = 0; i != loopNamesCount; ++i) {
		loopNameOffsets[i] = s->readUint32LE();
	}

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kLNID)
		return false;

	char *names = (char*)malloc(roundup(chd.size));
	s->read(names, roundup(chd.size));

	for (int i = 0; i != loopNamesCount; ++i) {
		char   *begin = names + loopNameOffsets[i];
		uint32  len   = ((i == loopNamesCount - 1) ? chd.size : loopNameOffsets[i+1]) - loopNameOffsets[i];

		_loopInfo.loops[i].name = Common::String(begin, len);

		// debug("%2d: %s", i, _loopInfo.loops[i].name.c_str());
	}

	free(loopNameOffsets);
	free(names);
	return true;
}

bool VQADecoder::getLoopBeginAndEndFrame(int loop, int *begin, int *end) {
	assert(begin && end);

	if (loop < 0 || loop >= _loopInfo.loopCount)
		return false;

	*begin = _loopInfo.loops[loop].begin;
	*end   = _loopInfo.loops[loop].end;

	return true;
}

bool VQADecoder::readCLIP(Common::SeekableReadStream *s, uint32 size) {
	s->skip(roundup(size));
	return true;
}

bool VQADecoder::readMFCI(Common::SeekableReadStream *s, uint32 size) {
	s->skip(roundup(size));
	return true;
}

VQADecoder::VQAVideoTrack::VQAVideoTrack(VQADecoder *vqaDecoder) {
	VQADecoder::Header *header = &vqaDecoder->_header;

	_surface = nullptr;
	_hasNewFrame = false;

	_numFrames = header->numFrames;
	_width     = header->width;
	_height    = header->height;
	_blockW    = header->blockW;
	_blockH    = header->blockH;
	_frameRate = header->frameRate;
	_maxBlocks = header->maxBlocks;
	_offsetX   = header->offsetX;
	_offsetY   = header->offsetY;

	_maxVPTRSize = header->maxVPTRSize;
	_maxCBFZSize = header->maxCBFZSize;
	_maxZBUFChunkSize = vqaDecoder->_maxZBUFChunkSize;

	_codebookSize = 0;
	_codebook  = nullptr;
	_cbfz      = nullptr;
	_zbufChunk = nullptr;

	_vpointerSize = 0;
	_vpointer = nullptr;

	_curFrame = -1;

	_zbufChunk = new uint8[roundup(_maxZBUFChunkSize)];

	_surface = new Graphics::Surface();
	_surface->create(_width, _height, createRGB555());

	_viewData = nullptr;
	_lightsData = nullptr;
}

VQADecoder::VQAVideoTrack::~VQAVideoTrack() {
	delete[] _codebook;
	delete[] _cbfz;
	delete[] _zbufChunk;
	delete[] _vpointer;

	if (_surface)
		_surface->free();
	delete _surface;

	if (_viewData)
		delete[] _viewData;
	if (_lightsData)
		delete[] _lightsData;
}

uint16 VQADecoder::VQAVideoTrack::getWidth() const {
	return _width;
}

uint16 VQADecoder::VQAVideoTrack::getHeight() const {
	return _height;
}

Graphics::PixelFormat VQADecoder::VQAVideoTrack::getPixelFormat() const {
	return _surface->format;
}

int VQADecoder::VQAVideoTrack::getCurFrame() const {
	return _curFrame;
}

int VQADecoder::VQAVideoTrack::getFrameCount() const {
	return _numFrames;
}

Common::Rational VQADecoder::VQAVideoTrack::getFrameRate() const {
	return _frameRate;
}

const Graphics::Surface *VQADecoder::VQAVideoTrack::decodeVideoFrame() {
	if (_hasNewFrame) {
		decodeFrame((uint16*)_surface->getPixels());
		_curFrame++;
		_hasNewFrame = false;
	}
	return _surface;
}

bool VQADecoder::VQAVideoTrack::readVQFL(Common::SeekableReadStream *s, uint32 size) {
	IFFChunkHeader chd;

	while (size >= 8) {
		if (!readIFFChunkHeader(s, &chd))
			return false;
		size -= roundup(chd.size) + 8;

		bool rc = false;
		switch (chd.id) {
			case kCBFZ: rc = readCBFZ(s, chd.size); break;
			default:
				s->skip(roundup(chd.size));
		}

		if (!rc) {
			warning("VQFL: error handling chunk %s", strTag(chd.id));
			return false;
		}
	}

	return true;
}

bool VQADecoder::VQAVideoTrack::readCBFZ(Common::SeekableReadStream *s, uint32 size) {
	if (size > _maxCBFZSize) {
		warning("readCBFZ: chunk too large: %d > %d", size, _maxCBFZSize);
		return false;
	}

	if (!_codebook) {
		_codebookSize = 2 * _maxBlocks * _blockW * _blockH;
		_codebook = new uint8[_codebookSize];
	}
	if (!_cbfz)
		_cbfz = new uint8[roundup(_maxCBFZSize)];

	s->read(_cbfz, roundup(size));

	decompress_lcw(_cbfz, size, _codebook, _codebookSize);

	return true;
}

bool VQADecoder::VQAVideoTrack::readZBUF(Common::SeekableReadStream *s, uint32 size) {
	if (size > _maxZBUFChunkSize) {
		debug("VQA ERROR: ZBUF chunk size: %08x > %08x", size, _maxZBUFChunkSize);
		s->skip(roundup(size));
		return false;
	}

	_zbufChunkSize = size;
	s->read(_zbufChunk, roundup(size));

	return true;
}

void VQADecoder::VQAVideoTrack::decodeZBuffer(ZBuffer *zbuffer) {
	if (_maxZBUFChunkSize == 0)
		return;

	zbuffer->decodeData(_zbufChunk, _zbufChunkSize);
}

bool VQADecoder::VQAVideoTrack::readVIEW(Common::SeekableReadStream *s, uint32 size) {
	if (size != 56)
		return false;

	if (_viewData) {
		delete[] _viewData;
		_viewData = nullptr;
	}

	_viewDataSize = size;
	_viewData = new uint8[_viewDataSize];
	s->read(_viewData, _viewDataSize);

	return true;
}

void VQADecoder::VQAVideoTrack::decodeView(View *view) {
	if (!view || !_viewData)
		return;

	Common::MemoryReadStream s(_viewData, _viewDataSize);
	view->read(&s);

	delete[] _viewData;
	_viewData = nullptr;
}

bool VQADecoder::VQAVideoTrack::readAESC(Common::SeekableReadStream *s, uint32 size) {
	debug("VQADecoder::readAESC(%d)", size);

	s->skip(roundup(size));
	return true;
}

bool VQADecoder::VQAVideoTrack::readLITE(Common::SeekableReadStream *s, uint32 size) {
	if (_lightsData) {
		delete[] _lightsData;
		_lightsData = nullptr;
	}

	_lightsDataSize = size;
	_lightsData = new uint8[_lightsDataSize];
	s->read(_lightsData, _lightsDataSize);

	return true;
}


void VQADecoder::VQAVideoTrack::decodeLights(Lights *lights) {
	if (!lights || !_lightsData)
		return;

	Common::MemoryReadStream s(_lightsData, _lightsDataSize);
	lights->readVqa(&s);

	delete[] _lightsData;
	_lightsData = nullptr;
}


bool VQADecoder::VQAVideoTrack::readVPTR(Common::SeekableReadStream *s, uint32 size) {
	if (size > _maxVPTRSize)
		return false;

	if (!_vpointer)
		_vpointer = new uint8[roundup(_maxVPTRSize)];

	_vpointerSize = size;
	s->read(_vpointer, roundup(size));

	_hasNewFrame = true;

	return true;
}

void VQADecoder::VQAVideoTrack::VPTRWriteBlock(uint16 *frame, unsigned int dstBlock, unsigned int srcBlock, int count, bool alpha) {
	uint16 frame_width  = _width;
	uint32 frame_stride = 640;
	uint16 block_width  = _blockW;
	uint16 block_height = _blockH;

	const uint8 *const block_src =
		&_codebook[2 * srcBlock * block_width * block_height];

	int blocks_per_line = frame_width / block_width;

	do {
		uint32 frame_x = dstBlock % blocks_per_line * block_width  + _offsetX / 2;
		uint32 frame_y = dstBlock / blocks_per_line * block_height + _offsetY;

		uint32 dst_offset = frame_x + frame_y * frame_stride;

		const uint8 *__restrict src = block_src;
		uint16      *__restrict dst = frame + dst_offset;

		unsigned int block_y;
		for (block_y = 0; block_y != block_height; ++block_y) {
			unsigned int block_x;
			for (block_x = 0; block_x != block_width; ++block_x) {
				uint16 rgb555 = src[0] | (src[1] << 8);
				src += 2;

				if (!(alpha && (rgb555 & 0x8000)))
					*dst = rgb555;
				++dst;
			}
			dst += frame_stride - block_width;
		}

		++dstBlock;
	} while (--count);
}

bool VQADecoder::VQAVideoTrack::decodeFrame(uint16 *frame) {
	if (!_codebook || !_vpointer)
		return false;

	uint8 *src = _vpointer;
	uint8 *end = _vpointer + _vpointerSize;

	uint16 count, srcBlock, dstBlock = 0;
	(void)srcBlock;

	while (end - src >= 2) {
		uint16 command = src[0] | (src[1] << 8);
		uint8  prefix = command >> 13;
		src += 2;

		switch (prefix) {
		case 0:
			count = command & 0x1fff;
			dstBlock += count;
			break;
		case 1:
			count = 2 * (((command >> 8) & 0x1f) + 1);
			srcBlock = command & 0x00ff;

			VPTRWriteBlock(frame, dstBlock, srcBlock, count);
			dstBlock += count;
			break;
		case 2:
			count = 2 * (((command >> 8) & 0x1f) + 1);
			srcBlock = command & 0x00ff;

			VPTRWriteBlock(frame, dstBlock, srcBlock, 1);
			++dstBlock;

			for (int i = 0; i < count; ++i) {
				srcBlock = *src++;
				VPTRWriteBlock(frame, dstBlock, srcBlock, 1);
				++dstBlock;
			}
			break;
		case 3:
		case 4:
			count = 1;
			srcBlock = command & 0x1fff;

			VPTRWriteBlock(frame, dstBlock, srcBlock, count, prefix == 4);
			++dstBlock;
			break;
		case 5:
		case 6:
			count = *src++;
			srcBlock = command & 0x1fff;

			VPTRWriteBlock(frame, dstBlock, srcBlock, count, prefix == 6);
			dstBlock += count;
			break;
		default:
			warning("VQAVideoTrack::decodeFrame: Undefined case %d", command >> 13);
		}
	}

	return true;
}

VQADecoder::VQAAudioTrack::VQAAudioTrack(VQADecoder *vqaDecoder) {
	_frequency = vqaDecoder->_header.freq;
}

VQADecoder::VQAAudioTrack::~VQAAudioTrack() {
}

Audio::SeekableAudioStream *VQADecoder::VQAAudioTrack::decodeAudioFrame() {
	int16 *audioFrame = (int16*)malloc(4 * 735);
	memset(audioFrame, 0, 4 * 735);

	_adpcmDecoder.decode(_compressedAudioFrame, 735, audioFrame);

	uint flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;

	return Audio::makeRawStream((byte*)audioFrame, 4 * 735, _frequency, flags, DisposeAfterUse::YES);
}

bool VQADecoder::VQAAudioTrack::readSND2(Common::SeekableReadStream *s, uint32 size) {
	if (size != 735) {
		warning("audio frame size: %d", size);
		return false;
	}

	s->read(_compressedAudioFrame, roundup(size));

	return true;
}

bool VQADecoder::VQAAudioTrack::readSN2J(Common::SeekableReadStream *s, uint32 size) {
	if (size != 6)
		return false;

	uint16 stepIndex = s->readUint16LE();
	uint32 predictor = s->readUint32LE();

	_adpcmDecoder.setParameters(stepIndex >> 5, predictor);

	return true;
}

} // End of namespace BladeRunner
