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
// and the FFmpeg Smacker decoder (libavcodec/smacker.c), revision 15884
// http://svn.ffmpeg.org/ffmpeg/trunk/libavcodec/smacker.c?revision=15884&view=markup

#include "graphics/smk_player.h"
#include "common/archive.h"
#include "common/system.h"
#include "common/util.h"
#include "common/array.h"
#include "common/endian.h"

namespace Graphics {

enum SmkBlockTypes {
    SMK_BLOCK_MONO = 0,
    SMK_BLOCK_FULL = 1,
    SMK_BLOCK_SKIP = 2,
    SMK_BLOCK_FILL = 3
};

/*
 * class BitStream
 * Keeps a two-byte lookahead, so overallocate buf by 2 bytes if
 * you want to avoid OOB reads.
 */

class BitStream {
public:
	BitStream(byte *buf, uint32 length)
		: _buf(buf), _end(buf+length), _curBit(8) {
		_curBytes  = *_buf++;
		_curBytes |= *_buf++ << 8;
	}

	bool getBit();
	byte getBits8();

	byte peek8() const;
	void skip(int n);

private:
	byte *_buf;
	byte *_end;
	uint16 _curBytes;
	byte  _curBit;
};

bool BitStream::getBit() {
	bool v = _curBytes & 1;

	_curBytes >>= 1;

	if (--_curBit == 0) {
		_curBytes |= *_buf++ << 8;
		_curBit = 8;
	}

	return v;
}

byte BitStream::getBits8() {
	byte v = _curBytes & 0xff;
	_curBytes >>= 8;
	_curBytes |= *_buf++ << _curBit;
	return v;
}

byte BitStream::peek8() const {
	return _curBytes & 0xff;
}

void BitStream::skip(int n) {
	assert(n <= 8);
	_curBytes >>= n;

	if (_curBit > n) {
		_curBit -= n;
	} else {
		_curBit = _curBit + 8 - n;
		_curBytes |= *_buf++ << _curBit;
	}
}

/*
 * class SmallHuffmanTree
 * A Huffman-tree to hold 8-bit values.
 * Unoptimized since it's only used during smk initialization.
 */

class SmallHuffmanTree {
public:
	SmallHuffmanTree(BitStream &bs) : _bs(bs) {
		uint32 bit = _bs.getBit();
		assert(bit);

		decodeTree(0);

		bit = _bs.getBit();
		assert(!bit);
	}

	uint16 getCode(BitStream &bs);
private:
	enum {
		SMK_NODE = 0x8000
	};

	int decodeTree(int length);

	Common::Array<uint16> _tree;
	BitStream &_bs;
};

int SmallHuffmanTree::decodeTree(int length) {
	if (!_bs.getBit()) { // Leaf
		uint16 v = _bs.getBits8();	// was uint32

		_tree.push_back(v);
		return 1;
	}

	_tree.push_back(0); // placeholder for r1
	int t = _tree.size() - 1;

	int r1 = decodeTree(length + 1);

	_tree[t] = (SMK_NODE | r1);

	int r2 = decodeTree(length + 1);

	return r1+r2+1;
}

uint16 SmallHuffmanTree::getCode(BitStream &bs) {
	uint16 *p = &_tree[0];

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
 * Contains the beginnings of an optimization.
 */

class BigHuffmanTree {
public:
	BigHuffmanTree(BitStream &bs);

	void reset();
	uint32 getCode(BitStream &bs);
private:
	enum {
		SMK_NODE = 0x80000000
	};

	int decodeTree(uint32 prefix, int length);

	Common::Array<uint32> _tree;
	uint32 _last[3];

	int _prefixtree[256];
	int _prefixlength[256];

	/* Used during construction */
	BitStream &_bs;
	uint32 _markers[3];
	SmallHuffmanTree *_loBytes;
	SmallHuffmanTree *_hiBytes;
};

BigHuffmanTree::BigHuffmanTree(BitStream &bs)
	: _bs(bs) {
	uint32 bit = _bs.getBit();
	if (!bit) {
		_tree.push_back(0);
		_last[0] = _last[1] = _last[2] = 0;
		return;
	}

	int i;
	for (i = 0; i < 256; ++i)
		_prefixtree[i] = 0;

	_loBytes = new SmallHuffmanTree(_bs);
	_hiBytes = new SmallHuffmanTree(_bs);

	_markers[0] = _bs.getBits8() | (_bs.getBits8() << 8);
	_markers[1] = _bs.getBits8() | (_bs.getBits8() << 8);
	_markers[2] = _bs.getBits8() | (_bs.getBits8() << 8);

	_last[0] = _last[1] = _last[2] = 0xffffffff;

	decodeTree(0, 0);
	bit = _bs.getBit();
	assert(!bit);

	for (i = 0; i < 3; ++i) {
		if (_last[i] == 0xffffffff) {
			_tree.push_back(0);
			_last[i] = _tree.size() - 1;
		}
	}

	delete _loBytes;
	delete _hiBytes;
}

void BigHuffmanTree::reset() {
	_tree[_last[0]] = _tree[_last[1]] = _tree[_last[2]] = 0;
}

int BigHuffmanTree::decodeTree(uint32 prefix, int length) {
	uint32 bit = _bs.getBit();

	if (!bit) { // Leaf
		uint32 lo = _loBytes->getCode(_bs);
		uint32 hi = _hiBytes->getCode(_bs);

		uint32 v = (hi << 8) | lo;
		_tree.push_back(v);

		int t = _tree.size() - 1;

		if (length <= 8) {
			uint32 i;
			for (i = 0; i < 256; i += (1 << length)) {
				_prefixtree[prefix | i] = t;
				_prefixlength[prefix | i] = length;
			}
		}

		int i;
		for (i = 0; i < 3; ++i) {
			if (_markers[i] == v) {
				_last[i] = t;
				_tree[t] = 0;
			}
		}

		return 1;
	}

	_tree.push_back(0); // placeholder for r1
	int t = _tree.size() - 1;

	if (length == 8) {
		_prefixtree[prefix] = t;
		_prefixlength[prefix] = 8;
	}

	int r1 = decodeTree(prefix, length + 1);

	_tree[t] = SMK_NODE | r1;

	int r2 = decodeTree(prefix | (1 << length), length + 1);
	return r1+r2+1;
}

uint32 BigHuffmanTree::getCode(BitStream &bs) {
	uint32 *p = &_tree[0];

	byte peek = bs.peek8();
	p = &_tree[_prefixtree[peek]];
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

SMKPlayer::SMKPlayer()
	: _currentSMKFrame(0), _fileStream(0) {
}

SMKPlayer::~SMKPlayer() {
	closeFile();
}

int SMKPlayer::getWidth() {
	if (!_fileStream)
		return 0;
	return _header.width;
}

int SMKPlayer::getHeight() {
	if (!_fileStream)
		return 0;
	return (_header.flags ? 2 : 1) * _header.height;
}

int32 SMKPlayer::getCurFrame() {
	if (!_fileStream)
		return -1;
	return _currentSMKFrame;
}

int32 SMKPlayer::getFrameCount() {
	if (!_fileStream)
		return 0;
	return _framesCount;
}

int32 SMKPlayer::getFrameRate() {
	if (!_fileStream)
		return 0;
	return _header.frameRate;
}

bool SMKPlayer::loadFile(const char *fileName) {
	closeFile();

	_fileStream = SearchMan.openFile(fileName);
	if (!_fileStream)
		return false;

	// Seek to the first frame
	_currentSMKFrame = 0;
	_header.signature = _fileStream->readUint32BE();

	// No BINK support available
	if (_header.signature == MKID_BE('BIKi')) {
		delete _fileStream;
		_fileStream = 0;
		return false;
	}

	assert(_header.signature == MKID_BE('SMK2') || _header.signature == MKID_BE('SMK4'));

	_header.width = _fileStream->readUint32LE();
	_header.height = _fileStream->readUint32LE();
	_header.frames = _fileStream->readUint32LE();
	_framesCount = _header.frames;
	_header.frameRate = (int32)_fileStream->readUint32LE();
	_header.flags = _fileStream->readUint32LE();

	unsigned int i;
	for (i = 0; i < 7; ++i)
		_header.audioSize[i] = _fileStream->readUint32LE();

	_header.treesSize = _fileStream->readUint32LE();
	_header.mMapSize = _fileStream->readUint32LE();
	_header.mClrSize = _fileStream->readUint32LE();
	_header.fullSize = _fileStream->readUint32LE();
	_header.typeSize = _fileStream->readUint32LE();

	for (i = 0; i < 7; ++i)
		_header.audioRate[i] = _fileStream->readUint32LE();

	_header.dummy = _fileStream->readUint32LE();

	_frameSizes = (uint32 *)malloc(_header.frames * sizeof(uint32));
	for (i = 0; i < _header.frames; ++i)
		_frameSizes[i] = _fileStream->readUint32LE();

	_frameTypes = (uint32 *)malloc(_header.frames * sizeof(uint32));
	for (i = 0; i < _header.frames; ++i)
		_frameTypes[i] = _fileStream->readByte();

	Common::Array<byte> huffmanTrees;
	huffmanTrees.resize(_header.treesSize + 2);
	_fileStream->read(&huffmanTrees[0], _header.treesSize);

	BitStream bs(&huffmanTrees[0], _header.treesSize + 2);

	_MMapTree = new BigHuffmanTree(bs);
	_MClrTree = new BigHuffmanTree(bs);
	_FullTree = new BigHuffmanTree(bs);
	_TypeTree = new BigHuffmanTree(bs);

	_image = (byte *)malloc(2 * _header.width * _header.height);
	_palette = (byte *)malloc(3 * 256);

	return true;
}

void SMKPlayer::closeFile() {
	if (!_fileStream)
		return;

	delete _fileStream;

	delete _MMapTree;
	delete _MClrTree;
	delete _FullTree;
	delete _TypeTree;

	free(_frameSizes);
	free(_frameTypes);
	free(_image);
	free(_palette);

	_fileStream = 0;
}

void SMKPlayer::copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch) {
	uint h = _header.height;
	uint w = _header.width;

	byte *src = _image;
	dst += y * pitch + x;

	do {
		memcpy(dst, src, w);
		dst += pitch;
		src += _header.width;
	} while (--h);
}

bool SMKPlayer::decodeNextFrame() {
	uint i;

	uint32 startPos = _fileStream->pos();

	_paletteDidChange = false;
	if (_frameTypes[_currentSMKFrame] & 1) {
		unpackPalette();
		_paletteDidChange = true;
		setPalette(_palette);
	}

	// TODO: Audio support
	// Skip audio tracks for now
	for (i = 0; i < 7; ++i) {
		if (!(_frameTypes[_currentSMKFrame] & (2 << i)))
			continue;

		uint32 len = _fileStream->readUint32LE();
		//uint32 unpackedLen = _fileStream->readUint32LE();
		_fileStream->skip(len - 4);
	}

	uint32 frameSize = _frameSizes[_currentSMKFrame] & ~3;

	if (_fileStream->pos() - startPos > frameSize)
		exit(1);

	uint32 frameDataSize = frameSize - (_fileStream->pos() - startPos);

	_frameData = (byte *)malloc(frameDataSize + 2);
	_fileStream->read(_frameData, frameDataSize);

	BitStream bs(_frameData, frameDataSize + 2);

	_MMapTree->reset();
	_MClrTree->reset();
	_FullTree->reset();
	_TypeTree->reset();

	static const uint blockRuns[64] = {
		 1,    2,    3,    4,    5,    6,    7,    8,
		 9,   10,   11,   12,   13,   14,   15,   16,
		17,   18,   19,   20,   21,   22,   23,   24,
		25,   26,   27,   28,   29,   30,   31,   32,
		33,   34,   35,   36,   37,   38,   39,   40,
		41,   42,   43,   44,   45,   46,   47,   48,
		49,   50,   51,   52,   53,   54,   55,   56,
		57,   58,   59,  128,  256,  512, 1024, 2048
	};

	uint bw = _header.width / 4;
	uint bh = _header.height / 4;
	uint stride = _header.width;
	uint block = 0, blocks = bw*bh;

	uint doubleY = _header.flags ? 2 : 1;

	byte *out;
	uint type, run, j, mode;
	uint32 p1, p2, clr, map;
	byte hi, lo;

	while (block < blocks) {
		type = _TypeTree->getCode(bs);

		run = blockRuns[(type >> 2) & 0x3f];
		//run = getBlockRun((type >> 2) & 0x3f);

		switch (type & 3) {
		case SMK_BLOCK_MONO:
			while (run-- && block < blocks) {
				clr = _MClrTree->getCode(bs);
				map = _MMapTree->getCode(bs);
				out = getCurSMKImage() + (block / bw) * (stride * 4 * doubleY) + (block % bw) * 4;
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
				out = getCurSMKImage() + (block / bw) * (stride * 4 * doubleY) + (block % bw) * 4;
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
						for(i = 0; i < 2; i++) {
							p1 = _FullTree->getCode(bs);
							p2 = _FullTree->getCode(bs);
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
				out = getCurSMKImage() + (block / bw) * (stride * 4 * doubleY) + (block % bw) * 4;
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

	return ++_currentSMKFrame < _header.frames;
}

void SMKPlayer::unpackPalette() {
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
		if (b0 & 0x80) {
			sz += (b0 & 0x7f) + 1;
			pal += 3 * ((b0 & 0x7f) + 1);
		} else if (b0 & 0x40) {
			byte c = (b0 & 0x3f) + 1;
			uint s = 3 * *p++;
			sz += c;

			while (c--) {
				*pal++ = oldPalette[s + 0];
				*pal++ = oldPalette[s + 1];
				*pal++ = oldPalette[s + 2];
				s += 3;
			}
		} else {
			sz++;
			byte b = b0 & 0x3f;
			byte g = (*p++) & 0x3f;
			byte r = (*p++) & 0x3f;

			assert(g < 0xc0 && b < 0xc0);

			*pal++ = b * 4;
			*pal++ = g * 4;
			*pal++ = r * 4;
		}
	}

	_fileStream->seek(startPos + len);

	free(chunk);
}

} // End of namespace Graphics
