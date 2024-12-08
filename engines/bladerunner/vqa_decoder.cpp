/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#define kCPL0 0x43504C30
#define kVPTZ 0x5650545A

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
	_vqpPalsArr          = nullptr;
	_numOfVQPPalettes    = 0;
	_oldV2VQA                 = false;
	_allowHorizontalScanlines = false; // only for old VQA(with optional VQP) version 2 videos
	_allowVerticalScanlines   = false; // only for old VQA(with optional VQP) version 2 videos
	_scaleVideoTo2xRequested  = true;  // only for old VQA(with optional VQP) version 2 videos
	_centerVideoRequested     = true;  // only for old VQA(with optional VQP) version 2 videos
}

VQADecoder::~VQADecoder() {
	close();
}

void VQADecoder::close() {
	for (uint i = _codebooks.size(); i != 0; --i) {
		delete[] _codebooks[i - 1].data;
	}
	_codebooks.clear();

	delete _audioTrack;
	_audioTrack = nullptr;

	delete _videoTrack;
	_videoTrack = nullptr;

	delete[] _frameInfo;
	_frameInfo = nullptr;

	_loopInfo.close();

	deleteVQPTable();
}

bool VQADecoder::loadStream(Common::SeekableReadStream *s) {
	close();

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

void VQADecoder::overrideOffsetXY(uint16 offX, uint16 offY) {
	_header.offsetX = offX;
	_header.offsetY = offY;
	if (_videoTrack != nullptr) {
		_videoTrack->overrideOffsetXY(offX, offY);
	}
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
	if (_header.flags == 1) {
		_oldV2VQA = true;
		_header.maxCBFZSize = _header.cbParts * _header.maxBlocks;
	}
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
	assert(_header.colors == 0 || (_oldV2VQA && _header.colors == 256));

	return true;
}

bool VQADecoder::VQAVideoTrack::readVQFR(Common::SeekableReadStream *s, uint32 size, uint readFlags) {
	IFFChunkHeader chd;
	signed int sizeLeft = size; // we have to use signed int to avoid underflow

	// NOTE This check should be here before the start of a actually reading a new VQFR segment.
	//      When using codebooks that get updated during the video playback, for the case where
	//      a codebook is constructed from parts and there's no starting frame codebook info,
	//      the new frame should be rendered with the existing codebook within the same VQFR chunk,
	//      if one was active. The newly constructed codebook should come in effect with the next VQFR chunk.
	if (_vqaDecoder->_oldV2VQA) {
		if (_cbParts != 0 && _countOfCBPsToCBF == _cbParts) {
			CodebookInfo& codebookInfo = _vqaDecoder->codebookInfoForFrame(_vqaDecoder->_readingFrame);
			const uint32 codebookPartialSize = _maxBlocks;
			uint32 bytesDecomprsd = decompress_lcw(_cbfzNext,
			                                       _accumulatedCBPZsizeToCBF,
			                                       _codebookInfoNext->data,
			                                       _cbParts * codebookPartialSize);
			assert(bytesDecomprsd <= _cbParts * codebookPartialSize);

			if (!codebookInfo.data) {
				codebookInfo.data = new uint8[roundup(_cbParts * codebookPartialSize)];
			}
			uint8* intermediateSwapPtr = codebookInfo.data;
			codebookInfo.data = _codebookInfoNext->data;
			codebookInfo.size = bytesDecomprsd;
			_codebookInfoNext->data = intermediateSwapPtr;

			_countOfCBPsToCBF = 0;
			_accumulatedCBPZsizeToCBF = 0;
		}
	}

	while (sizeLeft >= 8) {
		if (!readIFFChunkHeader(s, &chd))
			return false;
		sizeLeft -= roundup(chd.size) + 8;

		bool rc = false;
		switch (chd.id) {
		case kCBFZ: rc = ((readFlags & kVQAReadCodebook          ) == 0) ? s->skip(roundup(chd.size)) : readCBFZ(s, chd.size); break;
		case kCBPZ: rc = ((readFlags & kVQAReadCodebook          ) == 0) ? s->skip(roundup(chd.size)) : readCBPZ(s, chd.size); break;
		case kVPTR: rc = ((readFlags & kVQAReadVectorPointerTable) == 0) ? s->skip(roundup(chd.size)) : readVPTR(s, chd.size); break;
		case kVPTZ: rc = ((readFlags & kVQAReadVectorPointerTable) == 0) ? s->skip(roundup(chd.size)) : readVPTZ(s, chd.size); break;
		case kCPL0: rc = readCPL0(s, chd.size); break;
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

	for (uint32 i = count; i != 0; --i) {
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
	for (uint16 i = _loopInfo.loopCount; i != 0; --i) {
		_loopInfo.loops[_loopInfo.loopCount - i].begin = s->readUint16LE();
		_loopInfo.loops[_loopInfo.loopCount - i].end   = s->readUint16LE();

		// debug("Loop %d: %04x %04x", _loopInfo.loopCount - i, _loopInfo.loops[_loopInfo.loopCount - i].begin, _loopInfo.loops[_loopInfo.loopCount - i].end);
	}

	return true;
}

VQADecoder::CodebookInfo &VQADecoder::codebookInfoForFrame(int frame) {
	assert(frame < numFrames());
	assert(!_codebooks.empty() || _oldV2VQA);

	if (_codebooks.empty() && _oldV2VQA) {
		_codebooks.resize(1);
		_codebooks[0].frame = 0;
		_codebooks[0].size = 0;
		_codebooks[0].data = nullptr;
	}

	CodebookInfo *ci = nullptr;
	uint count = _codebooks.size();

	// find the last stored CodebookInfo where the current frame (param) belongs based on the codebook's CodebookInfo frame field.
	for (uint i = count; i != 0; --i) {
		if (frame >= _codebooks[i - 1].frame) {
			return _codebooks[i - 1];
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

	for (uint16 i = codebookCount; i != 0; --i) {
		_codebooks[codebookCount - i].frame = s->readUint16LE();
		_codebooks[codebookCount - i].size  = s->readUint32LE();
		_codebooks[codebookCount - i].data  = nullptr;

		// debug("Codebook %2u: %4d %8d", codebookCount - i, _codebooks[codebookCount - i].frame, _codebooks[codebookCount - i].size);

		assert(_codebooks[codebookCount - i].frame < numFrames());
	}

	return true;
}

bool VQADecoder::readFINF(Common::SeekableReadStream *s, uint32 size) {
	if (size != 4u * _header.numFrames)
		return false;

	_frameInfo = new uint32[_header.numFrames];

	for (uint16 i =  _header.numFrames; i != 0; --i)
		_frameInfo[ _header.numFrames - i] = s->readUint32LE();

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
	for (uint16 i = loopNamesCount; i != 0; --i) {
		loopNameOffsets[loopNamesCount - i] = s->readUint32LE();
	}

	readIFFChunkHeader(_s, &chd);
	if (chd.id != kLNID) {
		free(loopNameOffsets);
		return false;
	}

	char *names = (char *)malloc(roundup(chd.size));
	s->read(names, roundup(chd.size));

	for (uint16 i = loopNamesCount; i != 0; --i) {
		char   *begin = names + loopNameOffsets[loopNamesCount - i];
		uint32  len   = ((i == 1) ? chd.size : loopNameOffsets[loopNamesCount - i + 1]) - loopNameOffsets[loopNamesCount - i];

		_loopInfo.loops[loopNamesCount - i].name = Common::String(begin, len);

		// debug("%2u: %s", loopNamesCount - i, _loopInfo.loops[loopNamesCount - i].name.c_str());
	}

	free(loopNameOffsets);
	free(names);
	return true;
}

bool VQADecoder::getLoopBeginAndEndFrame(int loopId, int *begin, int *end) {
	assert(begin && end);

	if (loopId < 0 || loopId >= _loopInfo.loopCount)
		return false;

	*begin = _loopInfo.loops[loopId].begin;
	*end   = _loopInfo.loops[loopId].end;

	return true;
}

// Note that some video loops (scene loops) share frames (but will have different start frame (or end frame?))
// Thus this method may not return the "correct" loop here. It will just return the first loop that contains the frame.
int VQADecoder::getLoopIdFromFrame(int frame) {
	if (frame >= 0) {
		for (int loopId = 0; loopId < _loopInfo.loopCount; ++loopId) {
			if (frame >= _loopInfo.loops[loopId].begin && frame <= _loopInfo.loops[loopId].end) {
				return loopId;
			}
		}
	}
	return -1;
}

// Aux method for allocating new memory space for VQP palette table
// for the old version 2 VQP/VQA videos (eg. sizzle reels)
void VQADecoder::allocatePaletteVQPTable(const uint32 numOfPalettes) {
	deleteVQPTable();
	_vqpPalsArr = new VQPPalette[numOfPalettes];
	_numOfVQPPalettes = numOfPalettes;
}

// Aux method for populating the VQP 2d table per palette Id
// with an index (colorByte) to the palette's r,g,b entry
// for the old version 2 VQP/VQA videos (eg. sizzle reels)
void VQADecoder::updatePaletteVQPTable(uint32 palId, uint16 j, uint16 k, uint8 colorByte) {
	// the call assumes that there's space allocated for the _vqpPalsArr table
	if (palId < _numOfVQPPalettes) {
		_vqpPalsArr[palId].interpol2D[j][k] = colorByte;
		_vqpPalsArr[palId].interpol2D[k][j] = colorByte;
	}
}

// Aux method for deleting old memory space for VQP palette table
// for the old version 2 VQP/VQA videos (eg. sizzle reels)
void VQADecoder::deleteVQPTable() {
	if (_vqpPalsArr) {
		delete[] _vqpPalsArr;
		_vqpPalsArr = nullptr;
	}
	_numOfVQPPalettes = 0;
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
	_cbParts   = header->cbParts;
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

	_currentPaletteId         = -1; // init to -1 so that first increase will make it 0 (first valid index for palette)
	_cpalPointerSize          = 0;
	_cpalPointerSizeNext      = 0;
	_cpalPointer              = nullptr;
	_cpalPointerNext          = nullptr;
	_vptz                     = nullptr;

	_cbfzNext                 = nullptr; // stores all _cbParts compressed codebook parts and then decompress
	_codebookInfoNext         = nullptr; // stores the decompressed codebook parts and it's swapped with the active codebook
	_countOfCBPsToCBF         = 0;
	_accumulatedCBPZsizeToCBF = 0;
}

VQADecoder::VQAVideoTrack::~VQAVideoTrack() {
	delete[] _cbfz;
	delete[] _zbufChunk;
	delete[] _vpointer;

	delete[] _viewData;
	delete[] _screenEffectsData;
	delete[] _lightsData;

	delete[] _cpalPointer;
	delete[] _cpalPointerNext;
	delete[] _vptz;

	delete[] _cbfzNext;
	if (_codebookInfoNext != nullptr && _codebookInfoNext->data != nullptr) {
		delete[] _codebookInfoNext->data;
	}
	delete _codebookInfoNext;
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

void VQADecoder::VQAVideoTrack::overrideOffsetXY(uint16 offX, uint16 offY) {
	_offsetX = offX;
	_offsetY = offY;
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

// Read full codebook
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
	if (_vqaDecoder->_oldV2VQA) {
		codebookSize = _maxBlocks * _cbParts;
	}

	// This is released in VQADecoder::~VQADecoder()
	codebookInfo.data = new uint8[roundup(codebookSize)];

	if (!_cbfz) {
		_cbfz = new uint8[roundup(_maxCBFZSize)];
	}

	s->read(_cbfz, roundup(size));

	uint32 bytesDecomprsd = decompress_lcw(_cbfz, size, codebookInfo.data, codebookSize);
	codebookInfo.size = bytesDecomprsd;
	return true;
}

// Read partial codebook
bool VQADecoder::VQAVideoTrack::readCBPZ(Common::SeekableReadStream* s, uint32 size) {
	if (size > _maxCBFZSize) {
		warning("readCBPZ: chunk too large: %d > %d", size, _maxCBFZSize);
		return false;
	}

	// NOTE The Lands of Lore 2 reel VQA/VQP only has CBPZ chunks (no CBF* chunks)
	//      so we construct the first full codebook from the compressed parts.
	//      This means that we don't get a full codebook first, as in the case of
	//      the Blade Runner VQA/VQP reel.
	if (_cbParts == 0) {
		s->skip(roundup(size));
		return true;
	}

	if (!_cbfzNext) {
		_cbfzNext = new uint8[roundup(_maxCBFZSize)];
		_codebookInfoNext = new CodebookInfo();
		_codebookInfoNext->frame = 0;
		_codebookInfoNext->data = new uint8[roundup(_cbParts * _maxBlocks)];
		_codebookInfoNext->size = roundup(_cbParts * _maxBlocks);
		_countOfCBPsToCBF = 0;
		_accumulatedCBPZsizeToCBF = 0;
	}

	s->read(_cbfzNext + _accumulatedCBPZsizeToCBF, roundup(size));

	_accumulatedCBPZsizeToCBF += size;
	assert(_accumulatedCBPZsizeToCBF <= roundup(_maxCBFZSize));
	++_countOfCBPsToCBF;
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

bool VQADecoder::VQAVideoTrack::readCPL0(Common::SeekableReadStream *s, uint32 size) {
	// The size of a full Palette chunk is 0x300 = 3 x 256 = 768 bytes,
	// ie. r,g,b (in this order, 1 byte each)
	// When reading a new CPL0 section, that is not identical to the immediately previous one,
	// the active palette (_cpalPointer) is updated and the _currentPaletteId is increased
	// in order to index the corresponding palette segment in the VQP table from
	// the accompanying VQP file, if such exists.
	// For the purposes of the filtering process we use two buffers for the palette chunks,
	// ie. _cpalPointer and _cpalPointerNext.
	// CPL0 sections number (after filtering out the subsequent identical ones)
	// should match the number of palettes reported in VQP file, if such exists.
	if (size != kSizeInBytesOfCPL0Chunk)
		return false;

	// NOTE The Lands of Lore 2 reel VQA has many palette chunks
	//      but most are identical to their immediately preceding one.
	//      Its accompanying VQP file only specifies 52 palettes,
	//      which is the number we end up with,
	//      if we filter the consecutive duplicates.
	//
	// NOTE The roundup() is not really needed for kSizeInBytesOfCPL0Chunk
	//      since 768 is an even number. But we keep it for consistency,
	//      as it doesn't hurt.
	if (!_cpalPointer) {
		_cpalPointer = new uint8[roundup(kSizeInBytesOfCPL0Chunk)];
		memset(_cpalPointer, 0, roundup(kSizeInBytesOfCPL0Chunk));
	}

	if (!_cpalPointerNext) {
		_cpalPointerNext = new uint8[roundup(kSizeInBytesOfCPL0Chunk)];
		memset(_cpalPointerNext, 0, roundup(kSizeInBytesOfCPL0Chunk));
	}

	// Add a new palette ONLY if the previous is not identical.
	// This accommodates the VQA reel for Lands of Lore 2.
	if (_currentPaletteId == -1) {
		_cpalPointerSize = size;
		s->read(_cpalPointer, roundup(size));
		++_currentPaletteId;
	} else {
		_cpalPointerSizeNext = size;
		s->read(_cpalPointerNext, roundup(size));
		if (memcmp(_cpalPointer, _cpalPointerNext, roundup(kSizeInBytesOfCPL0Chunk)) != 0) {
			// Swap the current active palette buffer
			// with the one we've just read-in.
			uint8* tmpPalPointer = _cpalPointer;
			_cpalPointer = _cpalPointerNext;
			_cpalPointerNext = tmpPalPointer;
			_cpalPointerSize = _cpalPointerSizeNext;
			++_currentPaletteId;
		}
	}

	if (_vqaDecoder->_scaleVideoTo2xRequested || (_vqaDecoder->_allowHorizontalScanlines && _vqaDecoder->_allowVerticalScanlines)) {
		if (_vqaDecoder->_vqpPalsArr != nullptr && _vqaDecoder->_numOfVQPPalettes > 0) {
			if ((uint32)_currentPaletteId >= _vqaDecoder->_numOfVQPPalettes) {
				debug("cpl0 **ERROR** ID %d Vs palettes max valid id %d", (uint32)_currentPaletteId, _vqaDecoder->_numOfVQPPalettes - 1);
				// This is an unexpected case.
				// We can try cycling from the beginning of palette ids:
				// _currentPaletteId = 0;
				// Or we can stay in the last valid value for _currentPaletteId:
				_currentPaletteId = _vqaDecoder->_numOfVQPPalettes - 1;
				// We could also return false here:
				// return false;
			}
		}
	}
	return true;
}

bool VQADecoder::VQAVideoTrack::readVPTZ(Common::SeekableReadStream* s, uint32 size) {
	if (size > _maxVPTRSize)
		return false;

	if (!_vptz) {
		_vptz = new uint8[roundup(_maxVPTRSize)];
	}

	s->read(_vptz, roundup(size));

	if (!_vpointer) {
		_vpointer = new uint8[roundup(_maxBlocks * _blockW * _blockH)];
	}

	uint32 bytesDecomprsd = decompress_lcw(_vptz, size, _vpointer, _maxBlocks * _blockW * _blockH);
	assert(bytesDecomprsd <= roundup(_maxBlocks * _blockW * _blockH));
	_vpointerSize = bytesDecomprsd;

	_hasNewFrame = true;

	return true;
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

	uint16 blocks_per_line = _width / _blockW;

	uint32 intermDiv = 0;
	uint32 dst_x = 0;
	uint32 dst_y = 0;
	uint16 vqaColor = 0;
	uint8 a, r, g, b;

	for (uint i = count; i != 0; --i) {
		// aux variable to avoid duplicate division and a modulo operation
		intermDiv = (dstBlock + count - i) / blocks_per_line; // start of current blocks line
		dst_x = ((dstBlock + count - i) - intermDiv * blocks_per_line) * _blockW + _offsetX;
		dst_y = intermDiv * _blockH + _offsetY;

		const uint8 *src_p = block_src;

		for (uint y = _blockH; y != 0; --y) {
			for (uint x = _blockW; x != 0; --x) {
				vqaColor = READ_LE_UINT16(src_p);
				src_p += 2;

				getGameDataColor(vqaColor, a, r, g, b);

				if (!(alpha && a)) {
					// CLIP() is too slow and it is not needed.
					// void* dstPtr = surface->getBasePtr(CLIP(dst_x + x, (uint32)0, (uint32)(surface->w - 1)), CLIP(dst_y + y, (uint32)0, (uint32)(surface->h - 1)));
					void* dstPtr = surface->getBasePtr(dst_x + _blockW - x, dst_y + _blockH - y);
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

	// count    is the number of blocks to be covered (written or skipped)
	// dstBlock is each time the starting dstBlock (to be written);
	//          it is increased by count blocks, which might be 1 for some commands.
	// srcBlock is each time the starting srcBlock to be coppied, and used to calculate
	//          the proper index for the block data from the codebook data table.
	uint16 count = 0, srcBlock = 0, dstBlock = 0;

	if (!_vqaDecoder->_oldV2VQA) {
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

				for (uint16 i = count; i != 0; --i) {
					srcBlock = *src++;
					VPTRWriteBlock(surface, dstBlock, srcBlock, 1);
					++dstBlock;
				}
				break;

			case 3:
				// fall through
			case 4:
				count = 1;
				srcBlock = command & 0x1fff;

				VPTRWriteBlock(surface, dstBlock, srcBlock, count, prefix == 4);
				++dstBlock;
				break;

			case 5:
				// fall through
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
	} else {
		uint8 srcPartA = 0;
		uint8 srcPartB = 0;
		uint16 blocks_per_line   = _width  / _blockW;
		uint16 blocks_per_column = _height / _blockH;
		uint16 currLineBlock = 0;
		uint16 currColumnBlock = 0;
		uint32 dst_x = 0;
		uint32 dst_y = 0;
		void  *dstPtr = nullptr;

		assert(_vpointerSize == 2u * (blocks_per_column * blocks_per_line));
		// Create a pointer to the second half of the frame data:
		const uint8 *srcB = src + (blocks_per_column * blocks_per_line);

		bool scale2xPossible = (_vqaDecoder->_scaleVideoTo2xRequested
		                        && (   (_vqaDecoder->_vqpPalsArr != nullptr && _vqaDecoder->_numOfVQPPalettes > 0)
		                            || (_vqaDecoder->_allowVerticalScanlines && _vqaDecoder->_allowHorizontalScanlines) )
			                    && ((2 * getWidth()) <=  surface->w && (2 * getHeight()) <= surface->h) );
		// These offsets are different from the header _offsetX and _offsetY
		// They are explicitly evaluated in order to center (if requested) the video image
		uint16 xOffs = 0;
		uint16 yOffs = 0;
		if (_vqaDecoder->_centerVideoRequested
		    && !(surface->w == getWidth() && surface->h == getHeight())
		    && !(scale2xPossible && (surface->w == (2 * getWidth()) && surface->h == (2 * getHeight()))) ) {
			xOffs = (surface->w - (scale2xPossible? 2 : 1) * getWidth())  / 2; // leftmost x start
			yOffs = (surface->h - (scale2xPossible? 2 : 1) * getHeight()) / 2; // topmost  y start
		}

		uint8 *topBlockRowColorIndexForPalette = nullptr;
		uint8 *currBlockRowColorIndexForPalette = nullptr;
		if (scale2xPossible && !(_vqaDecoder->_allowVerticalScanlines && _vqaDecoder->_allowHorizontalScanlines)) {
			topBlockRowColorIndexForPalette  = new uint8[2 * _blockH * 2 * _width];
			currBlockRowColorIndexForPalette = new uint8[2 * _blockH * 2 * _width];
		}

		uint8 r, g, b;

		while (end - src > (blocks_per_column * blocks_per_line)) {
			srcPartA = *src++;	// next byte from first half of data
			srcPartB = *srcB++;	// next byte from second half of data

			if (srcPartB < 0x0F) {
				assert(_blockW * _blockH * (((uint32)srcPartB << 8) | srcPartA) < codebookInfo.size);

				// Copy a block from codebook data to the surface
				// The for() loop traverses the whole block of (_blockH) rows of (_blockW) pixels
				for (int i = 0; i < _blockH; ++i) {
					for (int k = 0; k < _blockW; ++k) {
						const uint8 cbdbyte = _codebook[_blockW * _blockH * (((uint32)srcPartB << 8) | srcPartA) + i * _blockW + k];

						if (scale2xPossible && !(_vqaDecoder->_allowVerticalScanlines && _vqaDecoder->_allowHorizontalScanlines)) {
							currBlockRowColorIndexForPalette[(2 * i * 2 * _width) + ((currColumnBlock * 2 * _blockW) + 2 * k)] = cbdbyte;
						}

						// R, G, B values are in 6bits which are converted to 8bits
						// NOTE Some R, G, B values are > 63 and are typically in [224, 255], ie. between 0xE0 and 0xFF.
						//      This makes bits 7 and 8 always set (1) in those cases
						//      TODO What is the significance of these extra bits?
						r = (0x3F & _cpalPointer[cbdbyte * 3]);
						g = (0x3F & _cpalPointer[cbdbyte * 3 + 1]);
						b = (0x3F & _cpalPointer[cbdbyte * 3 + 2]);

						r = (r << 2) | (r >> 4); // 6 to 8 bits
						g = (g << 2) | (g >> 4); // 6 to 8 bits
						b = (b << 2) | (b >> 4); // 6 to 8 bits

						dst_x = xOffs + ((currColumnBlock * _blockW) + k) * (scale2xPossible ? 2 : 1);
						dst_y = yOffs + ((currLineBlock * _blockH) + i) * (scale2xPossible ? 2 : 1);
						dstPtr = surface->getBasePtr(dst_x, dst_y);
						drawPixel(*surface, dstPtr, surface->format.RGBToColor(r, g, b));
					}
				}
			} else {
				// Fill the next block with colorByte
				// The for() loop traverses the whole block of (_blockH) rows of (_blockW) pixels
				for (int i = 0; i < _blockH; ++i) {
					for (int k = 0; k < _blockW; ++k) {
						const uint8 colorByte = srcPartA;

						if (scale2xPossible && !(_vqaDecoder->_allowVerticalScanlines && _vqaDecoder->_allowHorizontalScanlines)) {
							currBlockRowColorIndexForPalette[(2 * i * 2 * _width) + ((currColumnBlock * 2 * _blockW) + 2 * k)] = colorByte;
						}

						r = (0x3F & _cpalPointer[colorByte * 3]);
						g = (0x3F & _cpalPointer[colorByte * 3 + 1]);
						b = (0x3F & _cpalPointer[colorByte * 3 + 2]);
						r = (r << 2) | (r >> 4); // 6 to 8 bits
						g = (g << 2) | (g >> 4); // 6 to 8 bits
						b = (b << 2) | (b >> 4); // 6 to 8 bits

						dst_x = xOffs + ((currColumnBlock * _blockW) + k) * (scale2xPossible ? 2 : 1);
						dst_y = yOffs + ((currLineBlock * _blockH) + i) * (scale2xPossible ? 2 : 1);
						dstPtr = surface->getBasePtr(dst_x, dst_y);
						drawPixel(*surface, dstPtr, surface->format.RGBToColor(r, g, b));
					}
				}
			}
			if (currColumnBlock == blocks_per_line - 1) {
				// Here the initial pass for a row of blocks was completed
				if (scale2xPossible && !(_vqaDecoder->_allowVerticalScanlines && _vqaDecoder->_allowHorizontalScanlines)) {
					//    When VQP data is available, we can fill in any gaps in the 2x scaled video image:
					// 1. First go through currBlockRowColorIndexForPalette and fill in the gaps in the even rows of blocks
					//    Since we don't have info on a right pixel color for the last entry, just copy the left pixel color.
					// 2. Then go through the now filled currBlockRowColorIndexForPalette and the topBlockRowColorIndexForPalette,
					//    (if the latter exists --so not for the first line of blocks) and fill in the last pixels' row of the
					//    topBlockRowColorIndexForPalette, by using the data on the color of the previous to last pixels' row in
					//    topBlockRowColorIndexForPalette and the color of the first pixels' row in currBlockRowColorIndexForPalette.
					// 3. Swap topBlockRowColorIndexForPalette and currBlockRowColorIndexForPalette (to preserve it as previous row of blocks)
					// 4. For the last row of pixels, since we don't have info on a below pixel color, just copy the previous row of pixels.
					uint8 midColorIdx = 0;
					if (!_vqaDecoder->_allowVerticalScanlines) {
						uint8 leftColorIdx = 0;
						uint8 rightColorIdx = 0;
						for (int i = 0; i < 2 * _blockH - 1; i += 2) {
							for (int j = 1; j < 2 * _width; j += 2) {
								leftColorIdx = currBlockRowColorIndexForPalette[(i * 2 * _width) + j - 1];
								if (j == 2 * _width - 1) {
									rightColorIdx = leftColorIdx;
								} else {
									rightColorIdx = currBlockRowColorIndexForPalette[(i * 2 * _width) + j + 1];
								}
								midColorIdx = _vqaDecoder->_vqpPalsArr[_currentPaletteId].interpol2D[leftColorIdx][rightColorIdx];
								currBlockRowColorIndexForPalette[(i * 2 * _width) + j] = midColorIdx;
								r = (0x3F & _cpalPointer[midColorIdx * 3]);
								g = (0x3F & _cpalPointer[midColorIdx * 3 + 1]);
								b = (0x3F & _cpalPointer[midColorIdx * 3 + 2]);
								r = (r << 2) | (r >> 4); // 6 to 8 bits
								g = (g << 2) | (g >> 4); // 6 to 8 bits
								b = (b << 2) | (b >> 4); // 6 to 8 bits

								dst_x = xOffs + j;
								dst_y = yOffs + (currLineBlock * 2 * _blockH) + i;
								dstPtr = surface->getBasePtr(dst_x, dst_y);
								drawPixel(*surface, dstPtr, surface->format.RGBToColor(r, g, b));
							}
						}
					}
					if (!_vqaDecoder->_allowHorizontalScanlines) {
						// With the even rows of the line blocks completely filled, go to the odd rows
						// which are empty and completely fill them in, too.
						// NOTE if _allowVerticalScanlines is true, then the even rows are not completely filled.
						//      In that case we need to skip the odd columns here
						uint8 topColorIdx = 0;
						uint8 botColorIdx = 0;
						uint8 jIncr = 1;
						if (_vqaDecoder->_allowVerticalScanlines) {
							jIncr = 2;
						}
						for (int i = 1; i < 2 * _blockH - 1; i += 2) {
							for (int j = 0; j < 2 * _width; j += jIncr) {
								topColorIdx = currBlockRowColorIndexForPalette[((i - 1) * 2 * _width) + j];
								botColorIdx = currBlockRowColorIndexForPalette[((i + 1) * 2 * _width) + j];
								midColorIdx = _vqaDecoder->_vqpPalsArr[_currentPaletteId].interpol2D[topColorIdx][botColorIdx];
								currBlockRowColorIndexForPalette[(i * 2 * _width) + j] = midColorIdx;
								r = (0x3F & _cpalPointer[midColorIdx * 3]);
								g = (0x3F & _cpalPointer[midColorIdx * 3 + 1]);
								b = (0x3F & _cpalPointer[midColorIdx * 3 + 2]);
								r = (r << 2) | (r >> 4); // 6 to 8 bits
								g = (g << 2) | (g >> 4); // 6 to 8 bits
								b = (b << 2) | (b >> 4); // 6 to 8 bits

								dst_x = xOffs + j;
								dst_y = yOffs + (currLineBlock * 2 * _blockH) + i;
								dstPtr = surface->getBasePtr(dst_x, dst_y);
								drawPixel(*surface, dstPtr, surface->format.RGBToColor(r, g, b));
							}
						}
						// If we are at a row of blocks with index currLineBlock > 0, then use info
						// from the previous to last row of topBlockRowColorIndexForPalette
						// and the first row of currBlockRowColorIndexForPalette to fill the last row
						// of topBlockRowColorIndexForPalette and draw those pixels too.
						if (currLineBlock > 0) {
							for (int j = 0; j < 2 * _width; j += jIncr) {
								topColorIdx = topBlockRowColorIndexForPalette[((2 * _blockH - 2) * 2 * _width) + j];
								botColorIdx = currBlockRowColorIndexForPalette[j];
								midColorIdx = _vqaDecoder->_vqpPalsArr[_currentPaletteId].interpol2D[topColorIdx][botColorIdx];
								topBlockRowColorIndexForPalette[((2 * _blockH - 1) * 2 * _width) + j] = midColorIdx;
								r = (0x3F & _cpalPointer[midColorIdx * 3]);
								g = (0x3F & _cpalPointer[midColorIdx * 3 + 1]);
								b = (0x3F & _cpalPointer[midColorIdx * 3 + 2]);
								r = (r << 2) | (r >> 4); // 6 to 8 bits
								g = (g << 2) | (g >> 4); // 6 to 8 bits
								b = (b << 2) | (b >> 4); // 6 to 8 bits

								dst_x = xOffs + j;
								dst_y = yOffs + (currLineBlock * 2 * _blockH) - 1;
								dstPtr = surface->getBasePtr(dst_x, dst_y);
								drawPixel(*surface, dstPtr, surface->format.RGBToColor(r, g, b));
								if (currLineBlock == blocks_per_column - 1) {
									// For last row of blocks, fill in the last row pixels by assuming that
									// "bottom" pixel color is the same as previous to last row pixel's color
									topColorIdx = currBlockRowColorIndexForPalette[((2 * _blockH - 2) * 2 * _width) + j];
									botColorIdx = topColorIdx;
									midColorIdx = _vqaDecoder->_vqpPalsArr[_currentPaletteId].interpol2D[topColorIdx][botColorIdx];
									currBlockRowColorIndexForPalette[((2 * _blockH - 1) * 2 * _width) + j] = midColorIdx;
									r = (0x3F & _cpalPointer[midColorIdx * 3]);
									g = (0x3F & _cpalPointer[midColorIdx * 3 + 1]);
									b = (0x3F & _cpalPointer[midColorIdx * 3 + 2]);
									r = (r << 2) | (r >> 4); // 6 to 8 bits
									g = (g << 2) | (g >> 4); // 6 to 8 bits
									b = (b << 2) | (b >> 4); // 6 to 8 bits

									dst_x = xOffs + j;
									dst_y = yOffs + (blocks_per_column * 2 * _blockH) - 1;
									dstPtr = surface->getBasePtr(dst_x, dst_y);
									drawPixel(*surface, dstPtr, surface->format.RGBToColor(r, g, b));
								}
							}
						}
					}
					// Do the swap
					uint8* tmpSwapPointer = topBlockRowColorIndexForPalette;
					topBlockRowColorIndexForPalette = currBlockRowColorIndexForPalette;
					currBlockRowColorIndexForPalette = tmpSwapPointer;
				}
				++currLineBlock;     // advance by 1 line block
				currColumnBlock = 0; // and go to the first column block of the line
			} else {
				++currColumnBlock;   // advance by 1 column block
			}
		}

		if (scale2xPossible && !(_vqaDecoder->_allowVerticalScanlines && _vqaDecoder->_allowHorizontalScanlines)) {
			delete[] topBlockRowColorIndexForPalette;
			delete[] currBlockRowColorIndexForPalette;
		}
	}

	return true;
}

VQADecoder::VQAAudioTrack::VQAAudioTrack(VQADecoder *vqaDecoder) {
	if (vqaDecoder != nullptr) {
		_frequency = vqaDecoder->_header.freq;
	} else {
		warning("VQADecoder::VQAAudioTrack::VQAAudioTrack: null pointer for vqaDecoder parameter");
		// TODO use some typical value?
		_frequency = 0;
	}
	memset(_compressedAudioFrame, 0, kSizeInBytesOfCompressedAudioFrameMax * sizeof(uint8));
	_bigCompressedAudioFrame = false;
}

VQADecoder::VQAAudioTrack::~VQAAudioTrack() {
}

Audio::SeekableAudioStream *VQADecoder::VQAAudioTrack::decodeAudioFrame() {
	int16* audioFrame = nullptr;
	if (_bigCompressedAudioFrame) {
		audioFrame = (int16*)malloc(kSizeInShortsAllocatedToAudioFrameMax);
	} else {
		audioFrame = (int16*)malloc(kSizeInShortsAllocatedToAudioFrame);
	}
	if (audioFrame != nullptr) {

		uint flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;

		if (_bigCompressedAudioFrame) {
			memset(audioFrame, 0, kSizeInShortsAllocatedToAudioFrameMax);
			_adpcmDecoder.decode(_compressedAudioFrame, kSizeInBytesOfCompressedAudioFrameMax, audioFrame, true);
			_bigCompressedAudioFrame = false;
			return Audio::makeRawStream((byte*)audioFrame, kSizeInShortsAllocatedToAudioFrameMax, _frequency, flags, DisposeAfterUse::YES);
		} else {
			memset(audioFrame, 0, kSizeInShortsAllocatedToAudioFrame);
			_adpcmDecoder.decode(_compressedAudioFrame, kSizeInBytesOfCompressedAudioFrame, audioFrame, true);
			return Audio::makeRawStream((byte*)audioFrame, kSizeInShortsAllocatedToAudioFrame, _frequency, flags, DisposeAfterUse::YES);
		}
	} else {
		warning("VQADecoder::VQAAudioTrack::decodeAudioFrame: Insufficient memory to allocate for audio frame");
		return nullptr;
	}
}

bool VQADecoder::VQAAudioTrack::readSND2(Common::SeekableReadStream *s, uint32 size) {
	if (size != kSizeInBytesOfCompressedAudioFrame
	    && size != kSizeInBytesOfCompressedAudioFrameMax) {
		warning("audio frame size: %d", size);
		return false;
	}

	if (size == kSizeInBytesOfCompressedAudioFrameMax) {
		_bigCompressedAudioFrame = true;
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
