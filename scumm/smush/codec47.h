/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef SMUSH_CODEC_47_H
#define SMUSH_CODEC_47_H

#include "common/scummsys.h"

namespace Scumm {

class Codec47Decoder {
private:

	int32 _deltaSize;
	byte *_deltaBufs[2];
	byte *_deltaBuf;
	byte *_curBuf;
	int32 _prevSeqNb;
	int _lastTableWidth;
	const byte *_d_src, *_paramPtr;
	int _d_pitch;
	int32 _offset1, _offset2;
	byte *_tableBig;
	byte *_tableSmall;
	int16 _table[256];
	int32 _frameSize;
	int _width, _height;

	void makeTablesInterpolation(int param);
	void makeTables47(int width);
	void level1(byte *d_dst);
	void level2(byte *d_dst);
	void level3(byte *d_dst);
	void decode2(byte *dst, const byte *src, int width, int height, const byte *param_ptr);

public:
	Codec47Decoder();
	~Codec47Decoder();
	void init(int width, int height);
	void deinit();
	bool decode(byte *dst, const byte *src);
};

} // End of namespace Scumm

#endif
