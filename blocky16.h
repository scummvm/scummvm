// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef BLOCKY16_H
#define BLOCKY16_H

#include "bits.h"

class Blocky16 {
private:

	int32 _deltaSize;
	byte *_deltaBufs[2];
	byte *_deltaBuf;
	byte *_curBuf;
	int32 _prevSeqNb;
	int _lastTableWidth;
	const byte *_d_src, *_paramPtr, *_param6_7Ptr;
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
	void decode2(byte *dst, const byte *src, int width, int height, const byte *param_ptr, const byte *param6_7_ptr);

public:
	Blocky16();
	~Blocky16();
	void init(int width, int height);
	void deinit();
	void decode(byte *dst, const byte *src);
};

#endif
