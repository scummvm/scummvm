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

#ifndef CODEC37_H
#define CODEC37_H

#include "decoder.h"

class Codec37Decoder : public Decoder {
private:
	int32 _deltaSize;
	byte *_deltaBufs[2];
	byte *_deltaBuf;
	int16 *_offsetTable;
	int32 _curtable;
	uint16 _prevSeqNb;
	int32 _tableLastPitch;
	int32 _tableLastIndex;

public:
	bool initSize(const Point &, const Rect &);
	Codec37Decoder();
	void clean();
	virtual ~Codec37Decoder();
protected:
	void maketable(int32, int32);
	void bompDecode(byte *dst, byte *src, int32 len);
	void proc3WithFDFE(byte *, byte *, int32, int32, int32, int32, int16 *);
	void proc3WithoutFDFE(byte *, byte *, int32, int32, int32, int32, int16 *);
	void proc4WithFDFE(byte *, byte *, int32, int32, int32, int32, int16 *);
	void proc4WithoutFDFE(byte *, byte *, int32, int32, int32, int32, int16 *);
public:
	bool decode(byte *dst, Chunk &);
};

#endif
