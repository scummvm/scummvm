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
 * $URL$
 * $Id$
 *
 */

// Based on http://wiki.multimedia.cx/index.php?title=Smacker
// and the FFmpeg Smacker decoder (libavcodec/smacker.c), revision 16143
// http://svn.ffmpeg.org/ffmpeg/trunk/libavcodec/smacker.c?revision=16143&view=markup

#include "graphics/video/smk_decoder.h"

#include "common/archive.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/stream.h"
#include "common/system.h"

#include "sound/mixer.h"
#include "sound/audiostream.h"

namespace Graphics {

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

	_markers[0] = _bs.getBits8() | (_bs.getBits8() << 8);
	_markers[1] = _bs.getBits8() | (_bs.getBits8() << 8);
	_markers[2] = _bs.getBits8() | (_bs.getBits8() << 8);

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

SmackerDecoder::SmackerDecoder(Audio::Mixer *mixer)
	: _audioStarted(false), _audioStream(0), _mixer(mixer) {
}

SmackerDecoder::~SmackerDecoder() {
	closeFile();
}

int SmackerDecoder::getHeight() {
	if (!_fileStream)
		return 0;
	return (_header.flags ? 2 : 1) * _videoInfo.height;
}

int32 SmackerDecoder::getAudioLag() {
	if (!_fileStream)
		return 0;

	int32 frameDelay = getFrameDelay();
	int32 videoTime = _videoInfo.currentFrame * frameDelay;
	int32 audioTime;

	if (!_audioStream) {
		/* No audio.
		   Calculate the lag by how much time has gone by since the first frame
		   and how much time *should* have passed.
		*/

		audioTime = (g_system->getMillis() - _videoInfo.startTime) * 100;
	} else
		audioTime = (((int32) _mixer->getSoundElapsedTime(_audioHandle)) * 100);

	return videoTime - audioTime;
}

bool SmackerDecoder::loadFile(const char *fileName) {
	int32 frameRate;

	closeFile();

	_fileStream = SearchMan.createReadStreamForMember(fileName);
	if (!_fileStream)
		return false;

	// Seek to the first frame
	_videoInfo.currentFrame = 0;
	_header.signature = _fileStream->readUint32BE();

	// No BINK support available
	if (_header.signature == MKID_BE('BIKi')) {
		delete _fileStream;
		_fileStream = 0;
		return false;
	}

	assert(_header.signature == MKID_BE('SMK2') || _header.signature == MKID_BE('SMK4'));

	_videoInfo.width = _fileStream->readUint32LE();
	_videoInfo.height = _fileStream->readUint32LE();
	_videoInfo.frameCount = _fileStream->readUint32LE();
	frameRate = _fileStream->readSint32LE();

	if (frameRate > 0) {
		_videoInfo.frameRate = 1000 / frameRate;
		_videoInfo.frameDelay = frameRate * 100;
	} else if (frameRate < 0) {
		_videoInfo.frameRate = 100000 / (-frameRate);
		_videoInfo.frameDelay = -frameRate;
	} else {
		_videoInfo.frameRate = 10;
		_videoInfo.frameDelay = 10000;
	}

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

	uint32 audioInfo;
	for (i = 0; i < 7; ++i) {
		// AudioRate - Frequency and format information for each sound track, up to 7 audio tracks.
		// The 32 constituent bits have the following meaning:
		// * bit 31 - data is compressed
		// * bit 30 - indicates that audio data is present for this track
		// * bit 29 - 1 = 16-bit audio; 0 = 8-bit audio
		// * bit 28 - 1 = stereo audio; 0 = mono audio
		// * bits 27-26 - if both set to zero - use v2 sound decompression
		// * bits 25-24 - unused
		// * bits 23-0 - audio sample rate
		audioInfo = _fileStream->readUint32LE();
		_header.audioInfo[i].isCompressed = audioInfo & 0x80000000;
		_header.audioInfo[i].hasAudio = audioInfo & 0x40000000;
		_header.audioInfo[i].is16Bits = audioInfo & 0x20000000;
		_header.audioInfo[i].isStereo = audioInfo & 0x10000000;
		_header.audioInfo[i].hasV2Compression = !(audioInfo & 0x8000000) &&
												!(audioInfo & 0x4000000);
		_header.audioInfo[i].sampleRate = audioInfo & 0xFFFFFF;

		if (_header.audioInfo[i].hasAudio && i == 0) {
			byte flags = 0;

			if (_header.audioInfo[i].is16Bits)
				flags = flags | Audio::Mixer::FLAG_16BITS;

			if (_header.audioInfo[i].isStereo)
				flags = flags | Audio::Mixer::FLAG_STEREO;

			_audioStream = Audio::makeAppendableAudioStream(_header.audioInfo[i].sampleRate, flags);
		}
	}

	_header.dummy = _fileStream->readUint32LE();

	_frameSizes = (uint32 *)malloc(_videoInfo.frameCount * sizeof(uint32));
	for (i = 0; i < _videoInfo.frameCount; ++i)
		_frameSizes[i] = _fileStream->readUint32LE();

	_frameTypes = (byte *)malloc(_videoInfo.frameCount);
	for (i = 0; i < _videoInfo.frameCount; ++i)
		_frameTypes[i] = _fileStream->readByte();

	byte *huffmanTrees = new byte[_header.treesSize];
	_fileStream->read(huffmanTrees, _header.treesSize);

	BitStream bs(huffmanTrees, _header.treesSize);

	_MMapTree = new BigHuffmanTree(bs, _header.mMapSize);
	_MClrTree = new BigHuffmanTree(bs, _header.mClrSize);
	_FullTree = new BigHuffmanTree(bs, _header.fullSize);
	_TypeTree = new BigHuffmanTree(bs, _header.typeSize);

	delete[] huffmanTrees;

	_videoFrameBuffer = (byte *)malloc(2 * _videoInfo.width * _videoInfo.height);
	memset(_videoFrameBuffer, 0, 2 * _videoInfo.width * _videoInfo.height);
	_palette = (byte *)malloc(3 * 256);
	memset(_palette, 0, 3 * 256);

	_videoInfo.firstframeOffset = _fileStream->pos();

	return true;
}

void SmackerDecoder::closeFile() {
	if (!_fileStream)
		return;

	if (_audioStarted && _audioStream) {
		_mixer->stopHandle(_audioHandle);
		_audioStream = 0;
		_audioStarted = false;
	}

	delete _fileStream;
	_fileStream = 0;

	delete _MMapTree;
	delete _MClrTree;
	delete _FullTree;
	delete _TypeTree;

	free(_frameSizes);
	free(_frameTypes);
	free(_videoFrameBuffer);
	free(_palette);
}

bool SmackerDecoder::decodeNextFrame() {
	uint i;
	uint32 chunkSize = 0;
	uint32 dataSizeUnpacked = 0;

	uint32 startPos = _fileStream->pos();

	if (_videoInfo.currentFrame == 0)
		_videoInfo.startTime = g_system->getMillis();

	// Check if we got a frame with palette data, and
	// call back the virtual setPalette function to set
	// the current palette
	if (_frameTypes[_videoInfo.currentFrame] & 1) {
		unpackPalette();
		setPalette(_palette);
	}

	// Load audio tracks
	for (i = 0; i < 7; ++i) {
		if (!(_frameTypes[_videoInfo.currentFrame] & (2 << i)))
			continue;

		chunkSize = _fileStream->readUint32LE();
		chunkSize -= 4;    // subtract the first 4 bytes (chunk size)

		if (_header.audioInfo[i].isCompressed) {
			dataSizeUnpacked = _fileStream->readUint32LE();
			chunkSize -= 4;    // subtract the next 4 bytes (unpacked data size)
		} else {
			dataSizeUnpacked = 0;
		}

		if (_header.audioInfo[i].hasAudio && chunkSize > 0 && i == 0) {
			// If it's track 0, play the audio data
			byte *soundBuffer = new byte[chunkSize];

			_fileStream->read(soundBuffer, chunkSize);

			if (_header.audioInfo[i].isCompressed) {
				// Compressed audio (Huffman DPCM encoded)
				queueCompressedBuffer(soundBuffer, chunkSize, dataSizeUnpacked, i);
				delete[] soundBuffer;
			} else {
				// Uncompressed audio (PCM)
				_audioStream->queueBuffer(soundBuffer, chunkSize);
				// The sound buffer will be deleted by AppendableAudioStream
			}

			if (!_audioStarted) {
				_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_audioHandle, _audioStream, -1, 255);
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

	uint32 frameSize = _frameSizes[_videoInfo.currentFrame] & ~3;

	if (_fileStream->pos() - startPos > frameSize)
		exit(1);

	uint32 frameDataSize = frameSize - (_fileStream->pos() - startPos);

	_frameData = (byte *)malloc(frameDataSize);
	_fileStream->read(_frameData, frameDataSize);

	BitStream bs(_frameData, frameDataSize);

	_MMapTree->reset();
	_MClrTree->reset();
	_FullTree->reset();
	_TypeTree->reset();

	uint bw = _videoInfo.width / 4;
	uint bh = _videoInfo.height / 4;
	uint stride = _videoInfo.width;
	uint block = 0, blocks = bw*bh;

	uint doubleY = _header.flags ? 2 : 1;

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
				out = _videoFrameBuffer + (block / bw) * (stride * 4 * doubleY) + (block % bw) * 4;
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
			if (_header.signature == MKID_BE('SMK2')) {
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
				out = _videoFrameBuffer + (block / bw) * (stride * 4 * doubleY) + (block % bw) * 4;
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
				out = _videoFrameBuffer + (block / bw) * (stride * 4 * doubleY) + (block % bw) * 4;
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

	return ++_videoInfo.currentFrame < _videoInfo.frameCount;
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

	byte *unpackedBuffer = new byte[unpackedSize];
	byte *curPointer = unpackedBuffer;
	uint32 curPos = 0;

	SmallHuffmanTree *audioTrees[4];
	for (int k = 0; k < numBytes; k++)
		audioTrees[k] = new SmallHuffmanTree(audioBS);

	// Base values, stored as big endian

	int32 bases[2];

	if (isStereo)
		bases[1] = (!is16Bits) ?   audioBS.getBits8() :
		               ((int16) (((audioBS.getBits8() << 8) | audioBS.getBits8())));

	bases[0] = (!is16Bits) ?   audioBS.getBits8() :
	               ((int16) (((audioBS.getBits8() << 8) | audioBS.getBits8())));


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
				bases[k] += (int16) (audioTrees[k * 2]->getCode(audioBS) |
				                    (audioTrees[k * 2 + 1]->getCode(audioBS) << 8));

				WRITE_BE_UINT16(curPointer, CLIP<int32>(bases[k], -32768, 32767));
				curPointer += 2;
				curPos += 2;
			}
		}

	}

	for (int k = 0; k < numBytes; k++)
		delete audioTrees[k];

	_audioStream->queueBuffer(unpackedBuffer, unpackedSize);
	// unpackedBuffer will be deleted by AppendableAudioStream
}

void SmackerDecoder::unpackPalette() {
	uint startPos = _fileStream->pos();
	uint32 len = 4 * _fileStream->readByte();

	byte *chunk = (byte *)malloc(len);
	_fileStream->read(&chunk[0], len);
	byte *p = &chunk[0];

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

} // End of namespace Graphics
