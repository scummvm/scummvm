/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <stdafx.h>
#include "codec37.h"
#include "chunk.h"
#include "blitter.h"

#include "common/engine.h" // for debug, warning, error

#include <assert.h>
#include <string.h>

bool Codec37Decoder::initSize(const Point & p, const Rect & r) {
	if(r.width() != getRect().width() && r.height() != getRect().height()) {
		if(
			(r.width() != 320 || r.height() != 200) && 
			(r.width() != 384 || r.height() != 242) && 
			(r.width() != 640 || r.height() != 480)
			)
			return false;
		Decoder::initSize(p, r);
		clean();
		int32 frame_size = getRect().width() * getRect().height();
		_deltaSize = frame_size * 2 + DELTA_ADD * 4;
		_deltaBuf = new byte[_deltaSize];
		if(_deltaBuf == 0) error("unable to allocate decoder buffer");
		_deltaBufs[0] = _deltaBuf + DELTA_ADD;
		_deltaBufs[1] = _deltaBuf + frame_size + DELTA_ADD * 3;
		_offsetTable = new int16[255];
		if(_offsetTable == 0) error("unable to allocate decoder offset table");
		_tableLastPitch = -1;
		_tableLastIndex = -1;
		return true; 
	}
	return false;
}

Codec37Decoder::Codec37Decoder() {
	_deltaSize = 0;
	_deltaBuf = 0;
	_deltaBufs[0] = 0;
	_deltaBufs[1] = 0;
	_curtable = 0;
	_offsetTable = 0;
	_tableLastPitch = -1;
	_tableLastIndex = -1;
	_prevSeqNb = 32768; // Some invalid number
}

void Codec37Decoder::clean() {
	if(_offsetTable) {
		delete []_offsetTable;
		_offsetTable = 0;
		_tableLastPitch = -1;
		_tableLastIndex = -1;
	}
	if(_deltaBuf) {
		delete []_deltaBuf;
		_deltaSize = 0;
		_deltaBuf = 0;
		_deltaBufs[0] = 0;
		_deltaBufs[1] = 0;
	}
}

Codec37Decoder::~Codec37Decoder() {
	clean();
}

void Codec37Decoder::maketable(int32 pitch, int32 index) {
	static const int8 maketable_bytes[] = {
		0, 0, 1, 0, 2, 0, 3, 0, 5, 0, 8, 0, 13, 0, 21, 0,
		-1, 0, -2, 0, -3, 0, -5, 0, -8, 0, -13, 0, -17, 0, -21, 0,
		0, 1, 1, 1, 2, 1, 3, 1, 5, 1, 8, 1, 13, 1, 21, 1,
		-1, 1, -2, 1, -3, 1, -5, 1, -8, 1, -13, 1, -17, 1, -21, 1,
		0, 2, 1, 2, 2, 2, 3, 2, 5, 2, 8, 2, 13, 2, 21, 2,
		-1, 2, -2, 2, -3, 2, -5, 2, -8, 2, -13, 2, -17, 2, -21, 2,
		0, 3, 1, 3, 2, 3, 3, 3, 5, 3, 8, 3, 13, 3, 21, 3,
		-1, 3, -2, 3, -3, 3, -5, 3, -8, 3, -13, 3, -17, 3, -21, 3,
		0, 5, 1, 5, 2, 5, 3, 5, 5, 5, 8, 5, 13, 5, 21, 5,
		-1, 5, -2, 5, -3, 5, -5, 5, -8, 5, -13, 5, -17, 5, -21, 5,
		0, 8, 1, 8, 2, 8, 3, 8, 5, 8, 8, 8, 13, 8, 21, 8,
		-1, 8, -2, 8, -3, 8, -5, 8, -8, 8, -13, 8, -17, 8, -21, 8,
		0, 13, 1, 13, 2, 13, 3, 13, 5, 13, 8, 13, 13, 13, 21, 13,
		-1, 13, -2, 13, -3, 13, -5, 13, -8, 13, -13, 13, -17, 13, -21, 13,
		0, 21, 1, 21, 2, 21, 3, 21, 5, 21, 8, 21, 13, 21, 21, 21,
		-1, 21, -2, 21, -3, 21, -5, 21, -8, 21, -13, 21, -17, 21, -21, 21,
		0, -1, 1, -1, 2, -1, 3, -1, 5, -1, 8, -1, 13, -1, 21, -1,
		-1, -1, -2, -1, -3, -1, -5, -1, -8, -1, -13, -1, -17, -1, -21, -1,
		0, -2, 1, -2, 2, -2, 3, -2, 5, -2, 8, -2, 13, -2, 21, -2,
		-1, -2, -2, -2, -3, -2, -5, -2, -8, -2, -13, -2, -17, -2, -21, -2,
		0, -3, 1, -3, 2, -3, 3, -3, 5, -3, 8, -3, 13, -3, 21, -3,
		-1, -3, -2, -3, -3, -3, -5, -3, -8, -3, -13, -3, -17, -3, -21, -3,
		0, -5, 1, -5, 2, -5, 3, -5, 5, -5, 8, -5, 13, -5, 21, -5,
		-1, -5, -2, -5, -3, -5, -5, -5, -8, -5, -13, -5, -17, -5, -21, -5,
		0, -8, 1, -8, 2, -8, 3, -8, 5, -8, 8, -8, 13, -8, 21, -8,
		-1, -8, -2, -8, -3, -8, -5, -8, -8, -8, -13, -8, -17, -8, -21, -8,
		0, -13, 1, -13, 2, -13, 3, -13, 5, -13, 8, -13, 13, -13, 21, -13,
		-1, -13, -2, -13, -3, -13, -5, -13, -8, -13, -13, -13, -17, -13, -21, -13,
		0, -17, 1, -17, 2, -17, 3, -17, 5, -17, 8, -17, 13, -17, 21, -17,
		-1, -17, -2, -17, -3, -17, -5, -17, -8, -17, -13, -17, -17, -17, -21, -17,
		0, -21, 1, -21, 2, -21, 3, -21, 5, -21, 8, -21, 13, -21, 21, -21,
		-1, -21, -2, -21, -3, -21, -5, -21, -8, -21, -13, -21, -17, -21, 0, 0,
		-8, -29, 8, -29, -18, -25, 17, -25, 0, -23, -6, -22, 6, -22, -13, -19,
		12, -19, 0, -18, 25, -18, -25, -17, -5, -17, 5, -17, -10, -15, 10, -15,
		0, -14, -4, -13, 4, -13, 19, -13, -19, -12, -8, -11, -2, -11, 0, -11,
		2, -11, 8, -11, -15, -10, -4, -10, 4, -10, 15, -10, -6, -9, -1, -9,
		1, -9, 6, -9, -29, -8, -11, -8, -8, -8, -3, -8, 3, -8, 8, -8,
		11, -8, 29, -8, -5, -7, -2, -7, 0, -7, 2, -7, 5, -7, -22, -6,
		-9, -6, -6, -6, -3, -6, -1, -6, 1, -6, 3, -6, 6, -6, 9, -6,
		22, -6, -17, -5, -7, -5, -4, -5, -2, -5, 0, -5, 2, -5, 4, -5,
		7, -5, 17, -5, -13, -4, -10, -4, -5, -4, -3, -4, -1, -4, 0, -4,
		1, -4, 3, -4, 5, -4, 10, -4, 13, -4, -8, -3, -6, -3, -4, -3,
		-3, -3, -2, -3, -1, -3, 0, -3, 1, -3, 2, -3, 4, -3, 6, -3,
		8, -3, -11, -2, -7, -2, -5, -2, -3, -2, -2, -2, -1, -2, 0, -2,
		1, -2, 2, -2, 3, -2, 5, -2, 7, -2, 11, -2, -9, -1, -6, -1,
		-4, -1, -3, -1, -2, -1, -1, -1, 0, -1, 1, -1, 2, -1, 3, -1,
		4, -1, 6, -1, 9, -1, -31, 0, -23, 0, -18, 0, -14, 0, -11, 0,
		-7, 0, -5, 0, -4, 0, -3, 0, -2, 0, -1, 0, 0, -31, 1, 0,
		2, 0, 3, 0, 4, 0, 5, 0, 7, 0, 11, 0, 14, 0, 18, 0,
		23, 0, 31, 0, -9, 1, -6, 1, -4, 1, -3, 1, -2, 1, -1, 1,
		0, 1, 1, 1, 2, 1, 3, 1, 4, 1, 6, 1, 9, 1, -11, 2,
		-7, 2, -5, 2, -3, 2, -2, 2, -1, 2, 0, 2, 1, 2, 2, 2,
		3, 2, 5, 2, 7, 2, 11, 2, -8, 3, -6, 3, -4, 3, -2, 3,
		-1, 3, 0, 3, 1, 3, 2, 3, 3, 3, 4, 3, 6, 3, 8, 3,
		-13, 4, -10, 4, -5, 4, -3, 4, -1, 4, 0, 4, 1, 4, 3, 4,
		5, 4, 10, 4, 13, 4, -17, 5, -7, 5, -4, 5, -2, 5, 0, 5,
		2, 5, 4, 5, 7, 5, 17, 5, -22, 6, -9, 6, -6, 6, -3, 6,
		-1, 6, 1, 6, 3, 6, 6, 6, 9, 6, 22, 6, -5, 7, -2, 7,
		0, 7, 2, 7, 5, 7, -29, 8, -11, 8, -8, 8, -3, 8, 3, 8,
		8, 8, 11, 8, 29, 8, -6, 9, -1, 9, 1, 9, 6, 9, -15, 10,
		-4, 10, 4, 10, 15, 10, -8, 11, -2, 11, 0, 11, 2, 11, 8, 11,
		19, 12, -19, 13, -4, 13, 4, 13, 0, 14, -10, 15, 10, 15, -5, 17,
		5, 17, 25, 17, -25, 18, 0, 18, -12, 19, 13, 19, -6, 22, 6, 22,
		0, 23, -17, 25, 18, 25, -8, 29, 8, 29, 0, 31, 0, 0, -6, -22,
		6, -22, -13, -19, 12, -19, 0, -18, -5, -17, 5, -17, -10, -15, 10, -15,
		0, -14, -4, -13, 4, -13, 19, -13, -19, -12, -8, -11, -2, -11, 0, -11,
		2, -11, 8, -11, -15, -10, -4, -10, 4, -10, 15, -10, -6, -9, -1, -9,
		1, -9, 6, -9, -11, -8, -8, -8, -3, -8, 0, -8, 3, -8, 8, -8,
		11, -8, -5, -7, -2, -7, 0, -7, 2, -7, 5, -7, -22, -6, -9, -6,
		-6, -6, -3, -6, -1, -6, 1, -6, 3, -6, 6, -6, 9, -6, 22, -6,
		-17, -5, -7, -5, -4, -5, -2, -5, -1, -5, 0, -5, 1, -5, 2, -5,
		4, -5, 7, -5, 17, -5, -13, -4, -10, -4, -5, -4, -3, -4, -2, -4,
		-1, -4, 0, -4, 1, -4, 2, -4, 3, -4, 5, -4, 10, -4, 13, -4,
		-8, -3, -6, -3, -4, -3, -3, -3, -2, -3, -1, -3, 0, -3, 1, -3,
		2, -3, 3, -3, 4, -3, 6, -3, 8, -3, -11, -2, -7, -2, -5, -2,
		-4, -2, -3, -2, -2, -2, -1, -2, 0, -2, 1, -2, 2, -2, 3, -2,
		4, -2, 5, -2, 7, -2, 11, -2, -9, -1, -6, -1, -5, -1, -4, -1,
		-3, -1, -2, -1, -1, -1, 0, -1, 1, -1, 2, -1, 3, -1, 4, -1,
		5, -1, 6, -1, 9, -1, -23, 0, -18, 0, -14, 0, -11, 0, -7, 0,
		-5, 0, -4, 0, -3, 0, -2, 0, -1, 0, 0, -23, 1, 0, 2, 0,
		3, 0, 4, 0, 5, 0, 7, 0, 11, 0, 14, 0, 18, 0, 23, 0,
		-9, 1, -6, 1, -5, 1, -4, 1, -3, 1, -2, 1, -1, 1, 0, 1,
		1, 1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1, 9, 1, -11, 2,
		-7, 2, -5, 2, -4, 2, -3, 2, -2, 2, -1, 2, 0, 2, 1, 2,
		2, 2, 3, 2, 4, 2, 5, 2, 7, 2, 11, 2, -8, 3, -6, 3,
		-4, 3, -3, 3, -2, 3, -1, 3, 0, 3, 1, 3, 2, 3, 3, 3,
		4, 3, 6, 3, 8, 3, -13, 4, -10, 4, -5, 4, -3, 4, -2, 4,
		-1, 4, 0, 4, 1, 4, 2, 4, 3, 4, 5, 4, 10, 4, 13, 4,
		-17, 5, -7, 5, -4, 5, -2, 5, -1, 5, 0, 5, 1, 5, 2, 5,
		4, 5, 7, 5, 17, 5, -22, 6, -9, 6, -6, 6, -3, 6, -1, 6,
		1, 6, 3, 6, 6, 6, 9, 6, 22, 6, -5, 7, -2, 7, 0, 7,
		2, 7, 5, 7, -11, 8, -8, 8, -3, 8, 0, 8, 3, 8, 8, 8,
		11, 8, -6, 9, -1, 9, 1, 9, 6, 9, -15, 10, -4, 10, 4, 10,
		15, 10, -8, 11, -2, 11, 0, 11, 2, 11, 8, 11, 19, 12, -19, 13,
		-4, 13, 4, 13, 0, 14, -10, 15, 10, 15, -5, 17, 5, 17, 0, 18,
		-12, 19, 13, 19, -6, 22, 6, 22, 0, 23,
	};

	if (_tableLastPitch == pitch && _tableLastIndex == index)
		return;
#ifdef DEBUG_CODEC37
	debug(7, "codec37::maketable(%d, %d) called", pitch, index);
#endif
	_tableLastPitch = pitch;
	_tableLastIndex = index;
	index *= 255;
	assert(index + 254 < (int32)(sizeof(maketable_bytes) / 2));

	for (int32 i = 0; i < 255; i++) {
		int32 j = (i + index) << 1; // * 2
		_offsetTable[i] = maketable_bytes[j + 1] * pitch + maketable_bytes[j];
	}
}

void Codec37Decoder::proc1(Blitter & dst, Chunk & src, int32 next_offs, int32 bw, int32 bh, int32 size) {
	byte * decoded = new byte[size];
	int32 w = 0;
	while(!src.eof()) {
		int32 code = src.getByte();
		int32 length = (code >> 1) + 1;
		if (code & 1) {
			byte val = src.getByte();
			while(length--)
				decoded[w++] = val;
		} else {
			while(length--) {
				decoded[w++] = src.getByte();
			}
		}
	}
	assert(w == size);
	w = 0;
	// Now we have our stream ready...
	for(int32 i = 0; i < size; i++) {
		if(decoded[i] == 0xFF) {
			dst.putBlock(decoded + i + 1);
			i += 16;
		} else {
			dst.blockCopy(_offsetTable[decoded[i]] + next_offs);
		}
		if(++w == bw) {
			w = 0;
			dst.advance(0, 3);
		}
	}
	delete []decoded;
}

void Codec37Decoder::proc2(Blitter & dst, Chunk & src, int32 size) { // This is codec1 like...
#ifdef DEBUG_CODEC37_PROC2
	int32 decoded_size = 0;
	int32 coded_size = 0;
#endif
	do {
		int32 code = src.getByte();
		int32 length = (code >> 1) + 1;
		size -= length;
#ifdef DEBUG_CODEC37_PROC2
		decoded_size += length;
		coded_size += 1 + ((code & 1) ? 1 : length);

		debug(7, "proc2() : code == %d : length == %d : decoded_size == %d : coded_size == %d : seek - header == %d : size == %d",
			code, length, decoded_size, coded_size, src.tell() - 31, size + decoded_size);
#endif
		if (code & 1)
			dst.put(src.getChar(), length);
		else
			// while(length--) dst.put(src.get_char());
			dst.blit(src, length);
	} while (size);
}

void Codec37Decoder::proc3WithFDFE(Blitter & dst, Chunk & src, int32 next_offs, int32 bw, int32 bh) {
	do {
		int32 i = bw;
		do {
			int32 code = src.getByte();
			if (code == 0xFD) {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(1));
#else
				dst.putBlock(expand(src.getByte()));
#endif
			} else if (code == 0xFE) {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(2));
#else
				dst.putBlock(expand(src.getByte()), expand(src.getByte()), expand(src.getByte()), expand(src.getByte()));
#endif
			} else if (code == 0xFF) {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(3));
#else
				dst.putBlock(src);
#endif
			} else {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(4));
#else
				dst.blockCopy(_offsetTable[code] + next_offs); // copy from an offset !
#endif
			}
		} while (--i);
		dst.advance(0, 3); // advance 3 lines
	} while (--bh);
}

void Codec37Decoder::proc3WithoutFDFE(Blitter & dst, Chunk & src, int32 next_offs, int32 bw, int32 bh) {
	do {
		int32 i = bw;
		do {
			int32 code = src.getByte();
			if (code == 0xFF) {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(5));
#else
				dst.putBlock(src);
#endif
			} else {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(6));
#else
				dst.blockCopy(_offsetTable[code] + next_offs); // copy from an offset !
#endif
			}
		} while (--i);
		dst.advance(0, 3); // advance 3 lines
	} while (--bh);
}

void Codec37Decoder::proc4WithFDFE(Blitter & dst, Chunk & src, int32 next_offs, int32 bw, int32 bh) {
	do {
		int32 i = bw;
		do {
			int32 code = src.getByte();
			if (code == 0xFD) {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(7));
#else
				dst.putBlock(expand(src.getByte()));
#endif
			} else if (code == 0xFE) {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(8));
#else
				dst.putBlock(expand(src.getByte()), expand(src.getByte()), expand(src.getByte()), expand(src.getByte()));
#endif
			} else if (code == 0xFF) {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(9));
#else
				dst.putBlock(src);
#endif
			} else if (code == 0x00) {
				int32 length = src.getByte() + 1;
				for (int32 l = 0; l < length; l++) {
#ifdef USE_COLOR_CODE_FOR_BLOCK
					dst.putBlock(expand(10));
#else
					dst.blockCopy(next_offs);
#endif
					i--;
					if (i == 0) {
						dst.advance(0, 3); // advance 3 lines
						bh--;
						i = bw;
					}
				}
				if(bh == 0) return;
				i++;
			} else {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(11));
#else
				dst.blockCopy(_offsetTable[code] + next_offs); // copy from an offset !
#endif
			}
		} while (--i);
		dst.advance(0, 3); // advance 3 lines
	} while (--bh);
}

void Codec37Decoder::proc4WithoutFDFE(Blitter & dst, Chunk & src, int32 next_offs, int32 bw, int32 bh) {
	do {
		int32 i = bw;
		do {
			int32 code = src.getByte();
			if (code == 0xFF) {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(9));
#else
				dst.putBlock(src);
#endif
			} else if (code == 0x00) {
				int32 length = src.getByte() + 1;
				for (int32 l = 0; l < length; l++) {
#ifdef USE_COLOR_CODE_FOR_BLOCK
					dst.putBlock(expand(10));
#else
					dst.blockCopy(next_offs);
#endif
					i--;
					if (i == 0) {
						dst.advance(0, 3); // advance 3 lines
						bh--;
						i = bw;
					}
				}
				if(bh == 0) return;
				i++;
			} else {
#ifdef USE_COLOR_CODE_FOR_BLOCK
				dst.putBlock(expand(11));
#else
				dst.blockCopy(_offsetTable[code] + next_offs); // copy from an offset !
#endif
			}
		} while (--i);
		dst.advance(0, 3); // advance 3 lines
	} while (--bh);
}

bool Codec37Decoder::decode(Blitter & dst, Chunk & src) {
	int32 width = getRect().width();
	int32 height = getRect().height();
	int32 bw = (width + 3) >> 2, bh = (height + 3) >> 2;
	int32 pitch = bw << 2;
#ifdef DEBUG_CODEC37
	debug(7, "codec37::decode() : width == %d : height == %d : pitch == %d : _prevSeqNb == %d",
		width, height, pitch, _prevSeqNb);
#endif
	int32 code = src.getByte();			// 0 -> 1	(1)
	int32 index = src.getByte();			// 1 -> 2	(1)
	uint16 seq_nb	= src.getWord();	// 2 -> 4	(2)
	uint32 decoded_size = src.getDword();	// 4 -> 8	(4)
#ifdef DEBUG_CODEC37
	uint32 coded_size	= src.getDword();	// 8 -> 12	(4)
#else
	src.seek(4);
#endif
	uint32 mask_flag	= src.getDword();	// 12 -> 16	(4)
#ifdef DEBUG_CODEC37
	debug(7, "codec37::decode() : code == %d : index == %d : seq_nb == %d : decoded_size == %d : coded_size == %d : mask_flag == %d",
		code, index, seq_nb, decoded_size, coded_size, mask_flag);
#endif
	maketable(pitch, index);
	if(code == 3 || code == 4 || code == 1) {
		assert(seq_nb && _prevSeqNb + 1 == seq_nb);
		if (seq_nb & 1 || !(mask_flag & 1)) _curtable ^= 1;
	}
	Blitter blit((byte *)_deltaBufs[_curtable], Point(width, height), Rect(0, 0, width, height));
	switch(code) {
	case 0:
		memset(_deltaBuf, 0, _deltaBufs[_curtable] - _deltaBuf);
		memset(_deltaBufs[_curtable] + decoded_size, 0, _deltaBuf + _deltaSize - _deltaBufs[_curtable] - decoded_size);
		blit.blit(src, decoded_size);
		break;
	case 1:
		proc1(blit, src, _deltaBufs[_curtable ^ 1] - _deltaBufs[_curtable], bw, bh, decoded_size);
		break;
	case 2:
		memset(_deltaBuf, 0, _deltaBufs[_curtable] - _deltaBuf);
		memset(_deltaBufs[_curtable] + decoded_size, 0, _deltaBuf + _deltaSize - _deltaBufs[_curtable] - decoded_size);
		proc2(blit, src, decoded_size);
		break;
	case 3:
		if(mask_flag & 4)
			proc3WithFDFE(blit, src, _deltaBufs[_curtable ^ 1] - _deltaBufs[_curtable], bw, bh);
		else
			proc3WithoutFDFE(blit, src, _deltaBufs[_curtable ^ 1] - _deltaBufs[_curtable], bw, bh);
		break;
	case 4:
		if(mask_flag & 4)
			proc4WithFDFE(blit, src, _deltaBufs[_curtable ^ 1] - _deltaBufs[_curtable], bw, bh);
		else
			proc4WithoutFDFE(blit, src, _deltaBufs[_curtable ^ 1] - _deltaBufs[_curtable], bw, bh);
		break;
	default:
#ifdef DEBUG_CODEC37
		error("codec37::decode() received an invalid code : %d", code);
#endif
		break;
	}
	dst.blit((byte *)_deltaBufs[_curtable], width * height);
	_prevSeqNb = seq_nb;
	return true;
}

