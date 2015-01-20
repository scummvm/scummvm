/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef CODEC48_H
#define CODEC48_H

#include "common/scummsys.h"

namespace Grim {

class Codec48Decoder {
public:
	Codec48Decoder();
	~Codec48Decoder();
	void init(int width, int height);
	void deinit();
	bool decode(byte *dst, const byte *src);

private:
	void makeTable(int pitch, int index);

	void bompDecodeLine(byte *dst, const byte *src, int len);

	void decode3(byte *dst, const byte *src, int bufOffset);
	void scaleBlock(byte *dst, const byte *src);
	void copyBlock(byte *dst, int deltaBufOffset, int offset);

	int _curBuf;
	byte *_deltaBuf[2];
	int _blockX, _blockY;
	int _pitch;
	int16 *_offsetTable;
	int _tableLastPitch, _tableLastIndex;
	int16 _prevSeqNb;
	int32 _frameSize;
	int _width, _height;
	byte *_interTable;
};

} // end of namespace Grim

#endif
