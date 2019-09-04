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
#include "bladerunner/game_info.h"
#include "bladerunner/lights.h"
#include "bladerunner/screen_effects.h"
#include "bladerunner/view.h"
#include "bladerunner/zbuffer.h"

#include "audio/decoders/raw.h"

#include "common/array.h"
#include "common/util.h"
#include "common/memstream.h"

namespace BladeRunner {

#define kAESC 0x41455343
#define kCBFZ 0x4342465A
#define kCBPZ 0x4342505A
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

VQADecoder::VQADecoder() {
	_s                   = nullptr;
	_frameInfo           = nullptr;
	_videoTrack          = nullptr;
	_audioTrack          = nullptr;
	_maxVIEWChunkSize    = 0;
	_maxZBUFChunkSize    = 0;
	_maxAESCChunkSize    = 0;
	_header.version      = 0;
	_header.flags        = 0;
	_header.numFrames    = 0;
	_header.width        = 0;
	_header.height       = 0;
	_header.blockW       = 0;
	_header.blockH       = 0;
	_header.frameRate    = 0;
	_header.cbParts      = 0;
	_header.colors       = 0;
	_header.maxBlocks    = 0;
	_header.offsetX      = 0;
	_header.offsetY      = 0;
	_header.maxVPTRSize  = 0;
	_header.freq         = 0;
	_header.channels     = 0;
	_header.bits         = 0;
	_header.unk3         = 0;
	_header.unk4         = 0;
	_header.maxCBFZSize  = 0;
	_header.unk5         = 0;
	_readingFrame        = -1;
	_decodingFrame       = -1;
}

VQADecoder::~VQADecoder() {
	for (uint i = 0; i < _codebooks.size(); ++i) {
		delete[] _codebooks[i].data;
	}
	delete _audioTrack;
	delete _videoTrack;
	delete[] _frameInfo;
}

bool VQADecoder::loadStream(Common::SeekableReadStream *s) {
	// close();
	_s = s;

	IFFChunkHeader chd;
	uint32 type;

	readIFFChunkHeader(s, &chd);
	if (chd.id != kFORM || !chd.size)
		return false;

	type = s->readUint32BE();

	if (type != kWVQA)
		return false;

	do {
		if (!readIFFChunkHeader(_s, &chd))
			return false;

		bool rc = false;
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
			warning("Unhandled chunk '%s'", tag2str(chd.id));
			s->skip(roundup(chd.size));
			rc = true;
		}

		if (!rc) {
			warning("failed to handle chunk %s", tag2str(chd.id));
			return false;
		}
	} while (chd.id != kFINF);

	_videoTrack = new VQAVideoTrack(this);
	_audioTrack = new VQAAudioTrack(this);

	return true;
}

void VQADecoder::decodeVideoFrame(Graphics::Surface *surface, int frame, bool forceDraw) {
	_decodingFrame = frame;
	_videoTrack->decodeVideoFrame(surface, forceDraw);
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

void VQADecoder::decodeScreenEffects(ScreenEffects *screenEffects) {
	_videoTrack->decodeScreenEffects(screenEffects);
}

void VQADecoder::decodeLights(Lights *lights) {
	_videoTrack->decodeLights(lights);
}

void VQADecoder::readPacket(uint readFlags) {
	IFFChunkHeader chd;

	if (remain(_s) < 8) {
		warning("VQADecoder::readPacket(): remain: %d", remain(_s));
		assert(remain(_s) < 8);
	}

	do {
		if (!readIFFChunkHeader(_s, &chd)) {
			error("VQADecoder::readPacket(): Error reading chunk header");
		}

		bool rc = false;
		// Video track
		switch (chd.id) {
		case kAESC: rc = ((readFlags & kVQAReadCustom) == 0) ? _s->skip(roundup(chd.size)) : _videoTrack->readAESC(_s, chd.size); break;
		case kLITE: rc = ((readFlags & kVQAReadCustom) == 0) ? _s->skip(roundup(chd.size)) : _videoTrack->readLITE(_s, chd.size); break;
		case kVIEW: rc = ((readFlags & kVQAReadCustom) == 0) ? _s->skip(roundup(chd.size)) : _videoTrack->readVIEW(_s, chd.size); break;
		case kVQFL: rc = ((readFlags & kVQAReadVideo ) == 0) ? _s->skip(roundup(chd.size)) : _videoTrack->readVQFL(_s, chd.size, readFlags); break;
		case kVQFR: rc = ((readFlags & kVQAReadVideo ) == 0) ? _s->skip(roundup(chd.size)) : _videoTrack->readVQFR(_s, chd.size, readFlags); break;
		case kZBUF: rc = ((readFlags & kVQAReadCustom) == 0) ? _s->skip(roundup(chd.size)) : _videoTrack->readZBUF(_s, chd.size); break;
		// Sound track
		case kSN2J: rc = ((readFlags & kVQAReadAudio) == 0) ? _s->skip(roundup(chd.size)) : _audioTrack->readSN2J(_s, chd.size); break;
		case kSND2: rc = ((readFlags & kVQAReadAudio) == 0) ? _s->skip(roundup(chd.size)) : _audioTrack->readSND2(_s, chd.size); break;
		default:
			rc = false;
			_s->skip(roundup(chd.size));
		}

		if (!rc) {
			warning("VQADecoder::readPacket(): Error handling chunk %s", tag2str(chd.id));
			return;
		}
	} while (chd.id != kVQFR);
}

void VQADecoder::readFrame(int frame, uint readFlags) {
	if (frame < 0 || frame >= numFrames()) {
		error("VQADecoder::readFrame(): frame %d out of bounds, frame count is %d", frame, numFrames());
	}

	uint32 frameOffset = 2 * (_frameInfo[frame] & 0x0FFFFFFF);
	_s->seek(frameOffset);

	_readingFrame = frame;
	readPacket(readFlags);
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

	// if (_header.unk3 || _header.unk4 != 4 || _header.unk5 || _header.flags != 0x0014) {
	// 	debug("_header.version      %d", _header.version);
	// 	debug("_header.flags        %04x", _header.flags);
	// 	debug("_header.numFrames    %d", _header.numFrames);
	// 	debug("_header.width        %d", _header.width);
	// 	debug("_header.height       %d", _header.height);
	// 	debug("_header.blockW       %d", _header.blockW);
	// 	debug("_header.blockH       %d", _header.blockH);
	// 	debug("_header.frameRate    %d", _header.frameRate);
	// 	debug("_header.cbParts      %d", _header.cbParts);
	// 	debug("_header.colors       %d", _header.colors);
	// 	debug("_header.maxBlocks    %d", _header.maxBlocks);
	// 	debug("_header.offsetX      %d", _header.offsetX);
	// 	debug("_header.offsetY      %d", _header.offsetY);
	// 	debug("_header.maxVPTRSize  %d", _header.maxVPTRSize);
	// 	debug("_header.freq         %d", _header.freq);
	// 	debug("_header.channels     %d", _header.channels);
	// 	debug("_header.bits         %d", _header.bits);
	// 	debug("_header.unk3         %d", _header.unk3);
	// 	debug("_header.unk4         %d", _header.unk4);
	// 	debug("_header.maxCBFZSize  %d", _header.maxCBFZSize);
	// 	debug("_header.unk5         %d", _header.unk5);
	// 	debug("\n");
	// }

	assert(_header.version == 2);
	if (_header.channels != 0) {
		assert(_header.freq == 22050);
		assert(_header.channels == 1);
		assert(_header.bits == 16);
	}
	assert(_header.colors == 0);

	return true;
}

bool VQADecoder::VQAVideoTrack::readVQFR(Common::SeekableReadStream *s, uint32 size, uint readFlags) {
	IFFChunkHeader chd;

	signed int sizeLeft = size; // we have to use signed int to avoid underflow

	while (sizeLeft >= 8) {
		if (!readIFFChunkHeader(s, &chd))
			return false;
		sizeLeft -= roundup(chd.size) + 8;

		bool rc = false;
		switch (chd.id) {
		case kCBFZ: rc = ((readFlags & kVQAReadCodebook          ) == 0) ? s->skip(roundup(chd.size)) : readCBFZ(s, chd.size); break;
		case kCBPZ: rc = ((readFlags & kVQAReadCodebook          ) == 0) ? s->skip(roundup(chd.size)) : readCBFZ(s, chd.size); break;
		case kVPTR: rc = ((readFlags & kVQAReadVectorPointerTable) == 0) ? s->skip(roundup(chd.size)) : readVPTR(s, chd.size); break;
		default:
			s->skip(roundup(chd.size));
		}

		if (!rc) {
			error("VQADecoder::VQAVideoTrack::readVQFR(): error handling chunk %s", tag2str(chd.id));
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
			warning("Unknown tag in MSCT: %s", tag2str(tag));
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

VQADecoder::CodebookInfo &VQADecoder::codebookInfoForFrame(int frame) {
	assert(frame < numFrames());
	assert(!_codebooks.empty());

	CodebookInfo *ci = nullptr;
	uint count = _codebooks.size();
	for (uint i = 0; i != count; ++i) {
		if (frame >= _codebooks[count - i - 1].frame) {
			return _codebooks[count - i - 1];
		}
	}

	assert(ci && "No codebook found");
	return _codebooks[0];
}

bool VQADecoder::readCINF(Common::SeekableReadStream *s, uint32 size) {
	IFFChunkHeader chd;

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kCINH || chd.size != 8u)
		return false;

	uint16 codebookCount = s->readUint16LE();
	_codebooks.resize(codebookCount);

	s->skip(6);

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kCIND || chd.size != 6u * codebookCount)
		return false;

	for (int i = 0; i != codebookCount; ++i) {
		_codebooks[i].frame = s->readUint16LE();
		_codebooks[i].size  = s->readUint32LE();
		_codebooks[i].data  = nullptr;

		// debug("Codebook %2d: %4d %8d", i, _codebooks[i].frame, _codebooks[i].size);

		assert(_codebooks[i].frame < numFrames());
	}

	return true;
}

bool VQADecoder::readFINF(Common::SeekableReadStream *s, uint32 size) {
	if (size != 4u * _header.numFrames)
		return false;

	_frameInfo = new uint32[_header.numFrames];

	for (uint32 i = 0; i != _header.numFrames; ++i)
		_frameInfo[i] = s->readUint32LE();

	// if (false) {
	// 	uint32 last = 0;
	// 	for (uint32 i = 0; i != _header.numFrames; ++i) {
	// 		uint32 diff = _frameInfo[i] - last;
	// 		debug("_frameInfo[%4d] = 0x%08x   - %08x", i, _frameInfo[i], diff);
	// 		last = _frameInfo[i];
	// 	}
	// }

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

#if BLADERUNNER_DEBUG_CONSOLE
	debug("VQADecoder::readLNIN() Unknown Values: 0x%04x 0x%04x 0x%04x 0x%04x", loopUnk1, loopUnk2, loopUnk3, loopUnk4);
#else
	(void)loopUnk1;
	(void)loopUnk2;
	(void)loopUnk3;
	(void)loopUnk4;
#endif

	if (loopNamesCount != _loopInfo.loopCount)
		return false;

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kLNIO || chd.size != 4u * loopNamesCount)
		return false;

	uint32 *loopNameOffsets = (uint32 *)malloc(loopNamesCount * sizeof(uint32));
	for (int i = 0; i != loopNamesCount; ++i) {
		loopNameOffsets[i] = s->readUint32LE();
	}

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kLNID) {
		free(loopNameOffsets);
		return false;
	}

	char *names = (char *)malloc(roundup(chd.size));
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
	_vqaDecoder = vqaDecoder;
	_hasNewFrame = false;

	VQADecoder::Header *header = &vqaDecoder->_header;
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

	_codebook = nullptr;
	_cbfz     = nullptr;

	_vpointerSize = 0;
	_vpointer = nullptr;

	_curFrame = -1;

	_zbufChunkSize = 0;
	_zbufChunk     = new uint8[roundup(_maxZBUFChunkSize)];

	_viewDataSize = 0;
	_viewData     = nullptr;

	_screenEffectsDataSize = 0;
	_screenEffectsData     = nullptr;

	_lightsDataSize = 0;
	_lightsData     = nullptr;
}

VQADecoder::VQAVideoTrack::~VQAVideoTrack() {
	delete[] _cbfz;
	delete[] _zbufChunk;
	delete[] _vpointer;

	delete[] _viewData;
	delete[] _screenEffectsData;
	delete[] _lightsData;
}

uint16 VQADecoder::VQAVideoTrack::getWidth() const {
	return _width;
}

uint16 VQADecoder::VQAVideoTrack::getHeight() const {
	return _height;
}

int VQADecoder::VQAVideoTrack::getFrameCount() const {
	return _numFrames;
}

Common::Rational VQADecoder::VQAVideoTrack::getFrameRate() const {
	return _frameRate;
}

void VQADecoder::VQAVideoTrack::decodeVideoFrame(Graphics::Surface *surface, bool forceDraw) {
	if (_hasNewFrame || forceDraw) {
		assert(surface);
		decodeFrame(surface);
		_hasNewFrame = false;
	}
}

bool VQADecoder::VQAVideoTrack::readVQFL(Common::SeekableReadStream *s, uint32 size, uint readFlags) {
	IFFChunkHeader chd;

	signed int sizeLeft = size; // we have to use signed int to avoid underflow

	while (sizeLeft >= 8) {
		if (!readIFFChunkHeader(s, &chd))
			return false;
		sizeLeft -= roundup(chd.size) + 8;

		bool rc = false;
		switch (chd.id) {
			case kCBFZ: rc = readCBFZ(s, chd.size); break;
			default:
				s->skip(roundup(chd.size));
		}

		if (!rc) {
			warning("VQFL: error handling chunk %s", tag2str(chd.id));
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

	CodebookInfo &codebookInfo = _vqaDecoder->codebookInfoForFrame(_vqaDecoder->_readingFrame);
	if (codebookInfo.data) {
		s->skip(roundup(size));
		return true;
	}

	uint32 codebookSize = 2 * _maxBlocks * _blockW * _blockH;
	codebookInfo.data = new uint8[codebookSize];

	if (!_cbfz) {
		_cbfz = new uint8[roundup(_maxCBFZSize)];
	}

	s->read(_cbfz, roundup(size));

	decompress_lcw(_cbfz, size, codebookInfo.data, codebookSize);

	return true;
}

bool VQADecoder::VQAVideoTrack::readZBUF(Common::SeekableReadStream *s, uint32 size) {
	if (size > _maxZBUFChunkSize) {
		warning("VQA ERROR: ZBUF chunk size: %08x > %08x", size, _maxZBUFChunkSize);
		s->skip(roundup(size));
		return false;
	}

	_zbufChunkSize = size;
	s->read(_zbufChunk, roundup(size));

	return true;
}

void VQADecoder::VQAVideoTrack::decodeZBuffer(ZBuffer *zbuffer) {
	if (_zbufChunkSize == 0) {
		return;
	}

	zbuffer->decodeData(_zbufChunk, _zbufChunkSize);
}

bool VQADecoder::VQAVideoTrack::readVIEW(Common::SeekableReadStream *s, uint32 size) {
	if (size != 56) {
		return false;
	}

	if (_viewData) {
		delete[] _viewData;
	}

	_viewDataSize = roundup(size);
	_viewData = new uint8[_viewDataSize];
	s->read(_viewData, _viewDataSize);

	return true;
}

void VQADecoder::VQAVideoTrack::decodeView(View *view) {
	if (!view || !_viewData) {
		return;
	}

	Common::MemoryReadStream s(_viewData, _viewDataSize);
	view->readVqa(&s);

	delete[] _viewData;
	_viewData = nullptr;
}

bool VQADecoder::VQAVideoTrack::readAESC(Common::SeekableReadStream *s, uint32 size) {
	if (_screenEffectsData) {
		delete[] _screenEffectsData;
	}

	_screenEffectsDataSize = roundup(size);
	_screenEffectsData = new uint8[_screenEffectsDataSize];
	s->read(_screenEffectsData, _screenEffectsDataSize);

	return true;
}

void VQADecoder::VQAVideoTrack::decodeScreenEffects(ScreenEffects *aesc) {
	if (!aesc || !_screenEffectsData) {
		return;
	}

	Common::MemoryReadStream s(_screenEffectsData, _screenEffectsDataSize);
	aesc->readVqa(&s);

	delete[] _screenEffectsData;
	_screenEffectsData = nullptr;
}

bool VQADecoder::VQAVideoTrack::readLITE(Common::SeekableReadStream *s, uint32 size) {
	if (_lightsData) {
		delete[] _lightsData;
	}

	_lightsDataSize = roundup(size);
	_lightsData = new uint8[_lightsDataSize];
	s->read(_lightsData, _lightsDataSize);

	return true;
}


void VQADecoder::VQAVideoTrack::decodeLights(Lights *lights) {
	if (!lights || !_lightsData) {
		return;
	}

	Common::MemoryReadStream s(_lightsData, _lightsDataSize);
	lights->readVqa(&s);

	delete[] _lightsData;
	_lightsData = nullptr;
}


bool VQADecoder::VQAVideoTrack::readVPTR(Common::SeekableReadStream *s, uint32 size) {
	if (size > _maxVPTRSize)
		return false;

	if (!_vpointer) {
		_vpointer = new uint8[roundup(_maxVPTRSize)];
	}

	_vpointerSize = size;
	s->read(_vpointer, roundup(size));

	_hasNewFrame = true;

	return true;
}

void VQADecoder::VQAVideoTrack::VPTRWriteBlock(Graphics::Surface *surface, unsigned int dstBlock, unsigned int srcBlock, int count, bool alpha) {
	const uint8 *const block_src = &_codebook[2 * srcBlock * _blockW * _blockH];

	int blocks_per_line = _width / _blockW;

	for (int i = 0; i < count; ++i) {
		uint32 dst_x = (dstBlock + i) % blocks_per_line * _blockW + _offsetX;
		uint32 dst_y = (dstBlock + i) / blocks_per_line * _blockH + _offsetY;

		const uint8 *src_p = block_src;

		for (int y = 0; y != _blockH; ++y) {
			for (int x = 0; x != _blockW; ++x) {
				uint16 vqaColor = READ_LE_UINT16(src_p);
				src_p += 2;

				uint8 a, r, g, b;
				getGameDataColor(vqaColor, a, r, g, b);

				if (!(alpha && a)) {
					// clip is too slow and it is not needed
					// void* dstPtr = surface->getBasePtr(CLIP(dst_x + x, (uint32)0, (uint32)(surface->w - 1)), CLIP(dst_y + y, (uint32)0, (uint32)(surface->h - 1)));
					void* dstPtr = surface->getBasePtr(dst_x + x, dst_y + y);
					// Ignore the alpha in the output as it is inversed in the input
					drawPixel(*surface, dstPtr, surface->format.RGBToColor(r, g, b));
				}
			}
		}
	}
}

bool VQADecoder::VQAVideoTrack::decodeFrame(Graphics::Surface *surface) {
	CodebookInfo &codebookInfo = _vqaDecoder->codebookInfoForFrame(_vqaDecoder->_decodingFrame);

	if (!codebookInfo.data) {
		_vqaDecoder->readFrame(codebookInfo.frame, kVQAReadCodebook);
	}

	_codebook = codebookInfo.data;
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

			VPTRWriteBlock(surface, dstBlock, srcBlock, count);
			dstBlock += count;
			break;
		case 2:
			count = 2 * (((command >> 8) & 0x1f) + 1);
			srcBlock = command & 0x00ff;

			VPTRWriteBlock(surface, dstBlock, srcBlock, 1);
			++dstBlock;

			for (int i = 0; i < count; ++i) {
				srcBlock = *src++;
				VPTRWriteBlock(surface, dstBlock, srcBlock, 1);
				++dstBlock;
			}
			break;
		case 3:
		case 4:
			count = 1;
			srcBlock = command & 0x1fff;

			VPTRWriteBlock(surface, dstBlock, srcBlock, count, prefix == 4);
			++dstBlock;
			break;
		case 5:
		case 6:
			count = *src++;
			srcBlock = command & 0x1fff;

			VPTRWriteBlock(surface, dstBlock, srcBlock, count, prefix == 6);
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
	int16 *audioFrame = (int16 *)malloc(4 * 735);
	memset(audioFrame, 0, 4 * 735);

	_adpcmDecoder.decode(_compressedAudioFrame, 735, audioFrame, true);

	uint flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;

	return Audio::makeRawStream((byte *)audioFrame, 4 * 735, _frequency, flags, DisposeAfterUse::YES);
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
