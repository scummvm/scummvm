/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef CODEC_47_H
#define CODEC_47_H

#include "config.h"

#include "decoder.h"

class Codec47Decoder : public Decoder {
private:
	int32 _deltaSize;
	byte *_deltaBufs[2];
	byte *_deltaBuf;
	byte *_curBuf;
	int32 _prevSeqNb;
	int32 _lastTableWidth;
	byte *_d_src, *_paramPtr;
	int32 _d_pitch;
	int32 _offset1, _offset2;
	byte _tableBig[99328];
	byte _tableSmall[32768];
	int16 _table[256];

	void makeTables47(int32 width);
	void makeTables37(int32 param);
	void bompDecode(byte *dst, byte *src, int32 len);
	void level1(byte *d_dst);
	void level2(byte *d_dst);
	void level3(byte *d_dst);
	void decode2(byte *dst, byte *src, int32 width, int32 height, byte *param_ptr);

public:
	Codec47Decoder();
	virtual ~Codec47Decoder();
	bool initSize(const Point &, const Rect &);
	void clean();
	bool decode(byte *dst, Chunk &);
};

#endif
