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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Based on http://wiki.multimedia.cx/index.php?title=Smacker
// and the FFmpeg Smacker decoder (libavcodec/smacker.c), revision 16143
// http://git.ffmpeg.org/?p=ffmpeg;a=blob;f=libavcodec/smacker.c;hb=b8437a00a2f14d4a437346455d624241d726128e

#include "video/smk_decoder.h"

#include "common/endian.h"
#include "common/util.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

namespace Video {

enum SmkBlockTypes {
	SMK_BLOCK_MONO = 0,
	SMK_BLOCK_FULL = 1,
	SMK_BLOCK_SKIP = 2,
	SMK_BLOCK_FILL = 3
};

/*
 * class BitStream
 * Little-endian bit stream provider.
 */

class BitStream {
public:
	BitStream(byte *buf, uint32 length)
		: _buf(buf), _end(buf+length), _bitCount(8) {
		_curByte = *_buf++;
	}

	bool getBit();
	byte getBits8();

	byte peek8() const;
	void skip(int n);

private:
	byte *_buf;
	byte *_end;
	byte _curByte;
	byte  _bitCount;
};

bool BitStream::getBit() {
	if (_bitCount == 0) {
		assert(_buf < _end);
		_curByte = *_buf++;
		_bitCount = 8;
	}

	bool v = _curByte & 1;

	_curByte >>= 1;
	--_bitCount;

	return v;
}

byte BitStream::getBits8() {
	assert(_buf < _end);

	byte v = (*_buf << _bitCount) | _curByte;
	_curByte = *_buf++ >> (8 - _bitCount);

	return v;
}

byte BitStream::peek8() const {
	if (_buf == _end)
		return _curByte;

	assert(_buf < _end);
	return (*_buf << _bitCount) | _curByte;
}

void BitStream::skip(int n) {
	assert(n <= 8);
	_curByte >>= n;

	if (_bitCount >= n) {
		_bitCount -= n;
	} else {
		assert(_buf < _end);
		_bitCount = _bitCount + 8 - n;
		_curByte = *_buf++ >> (8 - _bitCount);
	}
}

/*
 * class SmallHuffmanTree
 * A Huffman-tree to hold 8-bit values.
 */

class SmallHuffmanTree {
public:
	SmallHuffmanTree(BitStream &bs);

	uint16 getCode(BitStream &bs);
private:
	enum {
		SMK_NODE = 0x8000
	};

	uint16 decodeTree(uint32 prefix, int length);

	uint16 _treeSize;
	uint16 _tree[511];

	uint16 _prefixtree[256];
	byte _prefixlength[256];

	BitStream &_bs;
};

SmallHuffmanTree::SmallHuffmanTree(BitStream &bs)
	: _treeSize(0), _bs(bs) {
	uint32 bit = _bs.getBit();
	assert(bit);

	for (uint16 i = 0; i < 256; ++i)
		_prefixtree[i] = _prefixlength[i] = 0;

	decodeTree(0, 0);

	bit = _bs.getBit();
	assert(!bit);
}

uint16 SmallHuffmanTree::decodeTree(uint32 prefix, int length) {
	if (!_bs.getBit()) { // Leaf
		_tree[_treeSize] = _bs.getBits8();

		if (length <= 8) {
			for (int i = 0; i < 256; i += (1 << length)) {
				_prefixtree[prefix | i] = _treeSize;
				_prefixlength[prefix | i] = length;
			}
		}
		++_treeSize;

		return 1;
	}

	uint16 t = _treeSize++;

	if (length == 8) {
		_prefixtree[prefix] = t;
		_prefixlength[prefix] = 8;
	}

	uint16 r1 = decodeTree(prefix, length + 1);

	_tree[t] = (SMK_NODE | r1);

	uint16 r2 = decodeTree(prefix | (1 << length), length + 1);

	return r1+r2+1;
}

uint16 SmallHuffmanTree::getCode(BitStream &bs) {
	byte peek = bs.peek8();
	uint16 *p = &_tree[_prefixtree[peek]];
	bs.skip(_prefixlength[peek]);

	while (*p & SMK_NODE) {
		if (bs.getBit())
			p += *p & ~SMK_NODE;
		p++;
	}

	return *p;
}

/*
 * class BigHuffmanTree
 * A Huffman-tree to hold 16-bit values.
 */

class BigHuffmanTree {
public:
	BigHuffmanTree(BitStream &bs, int allocSize);
	~BigHuffmanTree();

	void reset();
	uint32 getCode(BitStream &bs);
private:
	enum {
		SMK_NODE = 0x80000000
	};

	uint32 decodeTree(uint32 prefix, int length);

	uint32  _treeSize;
	uint32 *_tree;
	uint32  _last[3];

	uint32 _prefixtree[256];
	byte _prefixlength[256];

	/* Used during construction */
	BitStream &_bs;
	uint32 _markers[3];
	SmallHuffmanTree *_loBytes;
	SmallHuffmanTree *_hiBytes;
};

BigHuffmanTree::BigHuffmanTree(BitStream &bs, int allocSize)
	: _bs(bs) {
	uint32 bit = _bs.getBit();
	if (!bit) {
		_tree = new uint32[1];
		_tree[0] = 0;
		_last[0] = _last[1] = _last[2] = 0;
		return;
	}

	for (uint32 i = 0; i < 256; ++i)
		_prefixtree[i] = _prefixlength[i] = 0;

	_loBytes = new SmallHuffmanTree(_bs);
	_hiBytes = new SmallHuffmanTree(_bs);

	_markers[0] = _bs.getBits8();
	_markers[0] |= (_bs.getBits8() << 8);
	_markers[1] = _bs.getBits8();
	_markers[1] |= (_bs.getBits8() << 8);
	_markers[2] = _bs.getBits8();
	_markers[2] |= (_bs.getBits8() << 8);

	_last[0] = _last[1] = _last[2] = 0xffffffff;

	_treeSize = 0;
	_tree = new uint32[allocSize / 4];
	decodeTree(0, 0);
	bit = _bs.getBit();
	assert(!bit);

	for (uint32 i = 0; i < 3; ++i) {
		if (_last[i] == 0xffffffff) {
			_last[i] = _treeSize;
			_tree[_treeSize++] = 0;
		}
	}

	delete _loBytes;
	delete _hiBytes;
}

BigHuffmanTree::~BigHuffmanTree()
{
	delete[] _tree;
}

void BigHuffmanTree::reset() {
	_tree[_last[0]] = _tree[_last[1]] = _tree[_last[2]] = 0;
}

uint32 BigHuffmanTree::decodeTree(uint32 prefix, int length) {
	uint32 bit = _bs.getBit();

	if (!bit) { // Leaf
		uint32 lo = _loBytes->getCode(_bs);
		uint32 hi = _hiBytes->getCode(_bs);

		uint32 v = (hi << 8) | lo;

		_tree[_treeSize] = v;

		if (length <= 8) {
			for (int i = 0; i < 256; i += (1 << length)) {
				_prefixtree[prefix | i] = _treeSize;
				_prefixlength[prefix | i] = length;
			}
		}

		for (int i = 0; i < 3; ++i) {
			if (_markers[i] == v) {
				_last[i] = _treeSize;
				_tree[_treeSize] = 0;
			}
		}
		++_treeSize;

		return 1;
	}

	uint32 t = _treeSize++;

	if (length == 8) {
		_prefixtree[prefix] = t;
		_prefixlength[prefix] = 8;
	}

	uint32 r1 = decodeTree(prefix, length + 1);

	_tree[t] = SMK_NODE | r1;

	uint32 r2 = decodeTree(prefix | (1 << length), length + 1);
	return r1+r2+1;
}

uint32 BigHuffmanTree::getCode(BitStream &bs) {
	byte peek = bs.peek8();
	uint32 *p = &_tree[_prefixtree[peek]];
	bs.skip(_prefixlength[peek]);

	while (*p & SMK_NODE) {
		if (bs.getBit())
			p += (*p) & ~SMK_NODE;
		p++;
	}

	uint32 v = *p;
	if (v != _tree[_last[0]]) {
		_tree[_last[2]] = _tree[_last[1]];
		_tree[_last[1]] = _tree[_last[0]];
		_tree[_last[0]] = v;
	}

	return v;
}

SmackerDecoder::SmackerDecoder(Audio::Mixer *mixer, Audio::Mixer::SoundType soundType)
	: _audioStarted(false), _audioStream(0), _mixer(mixer), _soundType(soundType) {
	_surface = 0;
	_fileStream = 0;
	_dirtyPalette = false;
}

SmackerDecoder::~SmackerDecoder() {
	close();
}

uint32 SmackerDecoder::getElapsedTime() const {
	if (_audioStream && _audioStarted)
		return _mixer->getSoundElapsedTime(_audioHandle);

	return FixedRateVideoDecoder::getElapsedTime();
}

bool SmackerDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_fileStream = stream;

	// Read in the Smacker header
	_header.signature = _fileStream->readUint32BE();

	if (_header.signature != MKTAG('S', 'M', 'K', '2') && _header.signature != MKTAG('S', 'M', 'K', '4'))
		return false;

	uint32 width = _fileStream->readUint32LE();
	uint32 height = _fileStream->readUint32LE();
	_frameCount = _fileStream->readUint32LE();
	int32 frameRate = _fileStream->readSint32LE();

	// framerate contains 2 digits after the comma, so 1497 is actually 14.97 fps
	if (frameRate > 0)
		_frameRate = Common::Rational(1000, frameRate);
	else if (frameRate < 0)
		_frameRate = Common::Rational(100000, -frameRate);
	else
		_frameRate = 1000;

	// Flags are determined by which bit is set, which can be one of the following:
	// 0 - set to 1 if file contains a ring frame.
	// 1 - set to 1 if file is Y-interlaced
	// 2 - set to 1 if file is Y-doubled
	// If bits 1 or 2 are set, the frame should be scaled to twice its height
    // before it is displayed.
	_header.flags = _fileStream->readUint32LE();

	// TODO: should we do any extra processing for Smacker files with ring frames?

	// TODO: should we do any extra processing for Y-doubled videos? Are they the
	// same as Y-interlaced videos?

	uint32 i;
	for (i = 0; i < 7; ++i)
		_header.audioSize[i] = _fileStream->readUint32LE();

	_header.treesSize = _fileStream->readUint32LE();
	_header.mMapSize = _fileStream->readUint32LE();
	_header.mClrSize = _fileStream->readUint32LE();
	_header.fullSize = _fileStream->readUint32LE();
	_header.typeSize = _fileStream->readUint32LE();

	for (i = 0; i < 7; ++i) {
		// AudioRate - Frequency and format information for each sound track, up to 7 audio tracks.
		// The 32 constituent bits have the following meaning:
		// * bit 31 - indicates Huffman + DPCM compression
		// * bit 30 - indicates that audio data is present for this track
		// * bit 29 - 1 = 16-bit audio; 0 = 8-bit audio
		// * bit 28 - 1 = stereo audio; 0 = mono audio
		// * bit 27 - indicates Bink RDFT compression
		// * bit 26 - indicates Bink DCT compression
		// * bits 25-24 - unused
		// * bits 23-0 - audio sample rate
		uint32 audioInfo = _fileStream->readUint32LE();
		_header.audioInfo[i].hasAudio = audioInfo & 0x40000000;
		_header.audioInfo[i].is16Bits = audioInfo & 0x20000000;
		_header.audioInfo[i].isStereo = audioInfo & 0x10000000;
		_header.audioInfo[i].sampleRate = audioInfo & 0xFFFFFF;

		if (audioInfo & 0x8000000)
			_header.audioInfo[i].compression = kCompressionRDFT;
		else if (audioInfo & 0x4000000)
			_header.audioInfo[i].compression = kCompressionDCT;
		else if (audioInfo & 0x80000000)
			_header.audioInfo[i].compression = kCompressionDPCM;
		else
			_header.audioInfo[i].compression = kCompressionNone;

		if (_header.audioInfo[i].hasAudio) {
			if (_header.audioInfo[i].compression == kCompressionRDFT || _header.audioInfo[i].compression == kCompressionDCT)
				warning("Unhandled Smacker v2 audio compression");

			if (i == 0)
				_audioStream = Audio::makeQueuingAudioStream(_header.audioInfo[0].sampleRate, _header.audioInfo[0].isStereo);
		}
	}

	_header.dummy = _fileStream->readUint32LE();

	_frameSizes = new uint32[_frameCount];
	for (i = 0; i < _frameCount; ++i)
		_frameSizes[i] = _fileStream->readUint32LE();

	_frameTypes = new byte[_frameCount];
	for (i = 0; i < _frameCount; ++i)
		_frameTypes[i] = _fileStream->readByte();

	byte *huffmanTrees = new byte[_header.treesSize];
	_fileStream->read(huffmanTrees, _header.treesSize);

	BitStream bs(huffmanTrees, _header.treesSize);

	_MMapTree = new BigHuffmanTree(bs, _header.mMapSize);
	_MClrTree = new BigHuffmanTree(bs, _header.mClrSize);
	_FullTree = new BigHuffmanTree(bs, _header.fullSize);
	_TypeTree = new BigHuffmanTree(bs, _header.typeSize);

	delete[] huffmanTrees;

	_surface = new Graphics::Surface();

	// Height needs to be doubled if we have flags (Y-interlaced or Y-doubled)
	_surface->create(width, height * (_header.flags ? 2 : 1), Graphics::PixelFormat::createFormatCLUT8());

	memset(_palette, 0, 3 * 256);
	return true;
}

void SmackerDecoder::close() {
	if (!_fileStream)
		return;

	if (_audioStream) {
		if (_audioStarted) {
			// The mixer will delete the stream.
			_mixer->stopHandle(_audioHandle);
			_audioStarted = false;
		} else {
			delete _audioStream;
		}
		_audioStream = 0;
	}

	delete _fileStream;
	_fileStream = 0;

	_surface->free();
	delete _surface;
	_surface = 0;

	delete _MMapTree;
	delete _MClrTree;
	delete _FullTree;
	delete _TypeTree;

	delete[] _frameSizes;
	delete[] _frameTypes;

	reset();
}

const Graphics::Surface *SmackerDecoder::decodeNextFrame() {
	uint i;
	uint32 chunkSize = 0;
	uint32 dataSizeUnpacked = 0;

	uint32 startPos = _fileStream->pos();

	_curFrame++;

	// Check if we got a frame with palette data, and
	// call back the virtual setPalette function to set
	// the current palette
	if (_frameTypes[_curFrame] & 1) {
		unpackPalette();
		_dirtyPalette = true;
	}

	// Load audio tracks
	for (i = 0; i < 7; ++i) {
		if (!(_frameTypes[_curFrame] & (2 << i)))
			continue;

		chunkSize = _fileStream->readUint32LE();
		chunkSize -= 4;    // subtract the first 4 bytes (chunk size)

		if (_header.audioInfo[i].compression == kCompressionNone) {
			dataSizeUnpacked = chunkSize;
		} else {
			dataSizeUnpacked = _fileStream->readUint32LE();
			chunkSize -= 4;    // subtract the next 4 bytes (unpacked data size)
		}

		handleAudioTrack(i, chunkSize, dataSizeUnpacked);
	}

	uint32 frameSize = _frameSizes[_curFrame] & ~3;
//	uint32 remainder =  _frameSizes[_curFrame] & 3;

	if (_fileStream->pos() - startPos > frameSize)
		error("Smacker actual frame size exceeds recorded frame size");

	uint32 frameDataSize = frameSize - (_fileStream->pos() - startPos);

	_frameData = (byte *)malloc(frameDataSize);
	_fileStream->read(_frameData, frameDataSize);

	BitStream bs(_frameData, frameDataSize);

	_MMapTree->reset();
	_MClrTree->reset();
	_FullTree->reset();
	_TypeTree->reset();

	// Height needs to be doubled if we have flags (Y-interlaced or Y-doubled)
	uint doubleY = _header.flags ? 2 : 1;

	uint bw = getWidth() / 4;
	uint bh = getHeight() / doubleY / 4;
	uint stride = getWidth();
	uint block = 0, blocks = bw*bh;

	byte *out;
	uint type, run, j, mode;
	uint32 p1, p2, clr, map;
	byte hi, lo;

	while (block < blocks) {
		type = _TypeTree->getCode(bs);
		run = getBlockRun((type >> 2) & 0x3f);

		switch (type & 3) {
		case SMK_BLOCK_MONO:
			while (run-- && block < blocks) {
				clr = _MClrTree->getCode(bs);
				map = _MMapTree->getCode(bs);
				out = (byte *)_surface->pixels + (block / bw) * (stride * 4 * doubleY) + (block % bw) * 4;
				hi = clr >> 8;
				lo = clr & 0xff;
				for (i = 0; i < 4; i++) {
					for (j = 0; j < doubleY; j++) {
						out[0] = (map & 1) ? hi : lo;
						out[1] = (map & 2) ? hi : lo;
						out[2] = (map & 4) ? hi : lo;
						out[3] = (map & 8) ? hi : lo;
						out += stride;
					}
					map >>= 4;
				}
				++block;
			}
			break;
		case SMK_BLOCK_FULL:
			// Smacker v2 has one mode, Smacker v4 has three
			if (_header.signature == MKTAG('S','M','K','2')) {
				mode = 0;
			} else {
				// 00 - mode 0
				// 10 - mode 1
				// 01 - mode 2
				mode = 0;
				if (bs.getBit()) {
					mode = 1;
				} else if (bs.getBit()) {
					mode = 2;
				}
			}

			while (run-- && block < blocks) {
				out = (byte *)_surface->pixels + (block / bw) * (stride * 4 * doubleY) + (block % bw) * 4;
				switch (mode) {
					case 0:
						for (i = 0; i < 4; ++i) {
							p1 = _FullTree->getCode(bs);
							p2 = _FullTree->getCode(bs);
							for (j = 0; j < doubleY; ++j) {
								out[2] = p1 & 0xff;
								out[3] = p1 >> 8;
								out[0] = p2 & 0xff;
								out[1] = p2 >> 8;
								out += stride;
							}
						}
						break;
					case 1:
						p1 = _FullTree->getCode(bs);
						out[0] = out[1] = p1 & 0xFF;
						out[2] = out[3] = p1 >> 8;
						out += stride;
						out[0] = out[1] = p1 & 0xFF;
						out[2] = out[3] = p1 >> 8;
						out += stride;
						p2 = _FullTree->getCode(bs);
						out[0] = out[1] = p2 & 0xFF;
						out[2] = out[3] = p2 >> 8;
						out += stride;
						out[0] = out[1] = p2 & 0xFF;
						out[2] = out[3] = p2 >> 8;
						out += stride;
						break;
					case 2:
						for (i = 0; i < 2; i++) {
							// We first get p2 and then p1
							// Check ffmpeg thread "[PATCH] Smacker video decoder bug fix"
							// http://article.gmane.org/gmane.comp.video.ffmpeg.devel/78768
							p2 = _FullTree->getCode(bs);
							p1 = _FullTree->getCode(bs);
							for (j = 0; j < doubleY; ++j) {
								out[0] = p1 & 0xff;
								out[1] = p1 >> 8;
								out[2] = p2 & 0xff;
								out[3] = p2 >> 8;
								out += stride;
							}
							for (j = 0; j < doubleY; ++j) {
								out[0] = p1 & 0xff;
								out[1] = p1 >> 8;
								out[2] = p2 & 0xff;
								out[3] = p2 >> 8;
								out += stride;
							}
						}
						break;
				}
				++block;
			}
			break;
		case SMK_BLOCK_SKIP:
			while (run-- && block < blocks)
				block++;
			break;
		case SMK_BLOCK_FILL:
			uint32 col;
			mode = type >> 8;
			while (run-- && block < blocks) {
				out = (byte *)_surface->pixels + (block / bw) * (stride * 4 * doubleY) + (block % bw) * 4;
				col = mode * 0x01010101;
				for (i = 0; i < 4 * doubleY; ++i) {
					out[0] = out[1] = out[2] = out[3] = col;
					out += stride;
				}
				++block;
			}
			break;
		}
	}

	_fileStream->seek(startPos + frameSize);

	free(_frameData);

	if (_curFrame == 0)
		_startTime = g_system->getMillis();

	return _surface;
}

void SmackerDecoder::handleAudioTrack(byte track, uint32 chunkSize, uint32 unpackedSize) {
	if (_header.audioInfo[track].hasAudio && chunkSize > 0 && track == 0) {
		// If it's track 0, play the audio data
		byte *soundBuffer = (byte *)malloc(chunkSize);

		_fileStream->read(soundBuffer, chunkSize);

		if (_header.audioInfo[track].compression == kCompressionRDFT || _header.audioInfo[track].compression == kCompressionDCT) {
			// TODO: Compressed audio (Bink RDFT/DCT encoded)
			free(soundBuffer);
			return;
		} else if (_header.audioInfo[track].compression == kCompressionDPCM) {
			// Compressed audio (Huffman DPCM encoded)
			queueCompressedBuffer(soundBuffer, chunkSize, unpackedSize, track);
			free(soundBuffer);
		} else {
			// Uncompressed audio (PCM)
			byte flags = 0;
			if (_header.audioInfo[track].is16Bits)
				flags = flags | Audio::FLAG_16BITS;
			if (_header.audioInfo[track].isStereo)
				flags = flags | Audio::FLAG_STEREO;

			_audioStream->queueBuffer(soundBuffer, chunkSize, DisposeAfterUse::YES, flags);
			// The sound buffer will be deleted by QueuingAudioStream
		}

		if (!_audioStarted) {
			_mixer->playStream(_soundType, &_audioHandle, _audioStream, -1, 255);
			_audioStarted = true;
		}
	} else {
		// Ignore the rest of the audio tracks, if they exist
		// TODO: Are there any Smacker videos with more than one audio stream?
		// If yes, we should play the rest of the audio streams as well
		if (chunkSize > 0)
			_fileStream->skip(chunkSize);
	}
}

void SmackerDecoder::queueCompressedBuffer(byte *buffer, uint32 bufferSize,
		uint32 unpackedSize, int streamNum) {

	BitStream audioBS(buffer, bufferSize);
	bool dataPresent = audioBS.getBit();

	if (!dataPresent)
		return;

	bool isStereo = audioBS.getBit();
	assert(isStereo == _header.audioInfo[streamNum].isStereo);
	bool is16Bits = audioBS.getBit();
	assert(is16Bits == _header.audioInfo[streamNum].is16Bits);

	int numBytes = 1 * (isStereo ? 2 : 1) * (is16Bits ? 2 : 1);

	byte *unpackedBuffer = (byte *)malloc(unpackedSize);
	byte *curPointer = unpackedBuffer;
	uint32 curPos = 0;

	SmallHuffmanTree *audioTrees[4];
	for (int k = 0; k < numBytes; k++)
		audioTrees[k] = new SmallHuffmanTree(audioBS);

	// Base values, stored as big endian

	int32 bases[2];

	if (isStereo) {
		if (is16Bits) {
			byte hi = audioBS.getBits8();
			byte lo = audioBS.getBits8();
			bases[1] = (int16) ((hi << 8) | lo);
		} else {
			bases[1] = audioBS.getBits8();
		}
	}

	if (is16Bits) {
		byte hi = audioBS.getBits8();
		byte lo = audioBS.getBits8();
		bases[0] = (int16) ((hi << 8) | lo);
	} else {
		bases[0] = audioBS.getBits8();
	}

	// The bases are the first samples, too
	for (int i = 0; i < (isStereo ? 2 : 1); i++, curPointer += (is16Bits ? 2 : 1), curPos += (is16Bits ? 2 : 1)) {
		if (is16Bits)
			WRITE_BE_UINT16(curPointer, bases[i]);
		else
			*curPointer = (bases[i] & 0xFF) ^ 0x80;
	}

	// Next follow the deltas, which are added to the corresponding base values and
	// are stored as little endian
	// We store the unpacked bytes in big endian format

	while (curPos < unpackedSize) {
		// If the sample is stereo, the data is stored for the left and right channel, respectively
		// (the exact opposite to the base values)
		if (!is16Bits) {
			for (int k = 0; k < (isStereo ? 2 : 1); k++) {
				bases[k] += (int8) ((int16) audioTrees[k]->getCode(audioBS));
				*curPointer++ = CLIP<int>(bases[k], 0, 255) ^ 0x80;
				curPos++;
			}
		} else {
			for (int k = 0; k < (isStereo ? 2 : 1); k++) {
				byte lo = audioTrees[k * 2]->getCode(audioBS);
				byte hi = audioTrees[k * 2 + 1]->getCode(audioBS);
				bases[k] += (int16) (lo | (hi << 8));

				WRITE_BE_UINT16(curPointer, bases[k]);
				curPointer += 2;
				curPos += 2;
			}
		}

	}

	for (int k = 0; k < numBytes; k++)
		delete audioTrees[k];

	byte flags = 0;
	if (_header.audioInfo[0].is16Bits)
		flags = flags | Audio::FLAG_16BITS;
	if (_header.audioInfo[0].isStereo)
		flags = flags | Audio::FLAG_STEREO;
	_audioStream->queueBuffer(unpackedBuffer, unpackedSize, DisposeAfterUse::YES, flags);
	// unpackedBuffer will be deleted by QueuingAudioStream
}

void SmackerDecoder::unpackPalette() {
	uint startPos = _fileStream->pos();
	uint32 len = 4 * _fileStream->readByte();

	byte *chunk = (byte *)malloc(len);
	_fileStream->read(chunk, len);
	byte *p = chunk;

	byte oldPalette[3*256];
	memcpy(oldPalette, _palette, 3 * 256);

	byte *pal = _palette;

	int sz = 0;
	byte b0;
	while (sz < 256) {
		b0 = *p++;
		if (b0 & 0x80) {               // if top bit is 1 (0x80 = 10000000)
			sz += (b0 & 0x7f) + 1;     // get lower 7 bits + 1 (0x7f = 01111111)
			pal += 3 * ((b0 & 0x7f) + 1);
		} else if (b0 & 0x40) {        // if top 2 bits are 01 (0x40 = 01000000)
			byte c = (b0 & 0x3f) + 1;  // get lower 6 bits + 1 (0x3f = 00111111)
			uint s = 3 * *p++;
			sz += c;

			while (c--) {
				*pal++ = oldPalette[s + 0];
				*pal++ = oldPalette[s + 1];
				*pal++ = oldPalette[s + 2];
				s += 3;
			}
		} else {                       // top 2 bits are 00
			sz++;
			// get the lower 6 bits for each component (0x3f = 00111111)
			byte b = b0 & 0x3f;
			byte g = (*p++) & 0x3f;
			byte r = (*p++) & 0x3f;

			assert(g < 0xc0 && b < 0xc0);

			// upscale to full 8-bit color values by multiplying by 4
			*pal++ = b * 4;
			*pal++ = g * 4;
			*pal++ = r * 4;
		}
	}

	_fileStream->seek(startPos + len);
	free(chunk);
}

} // End of namespace Video
