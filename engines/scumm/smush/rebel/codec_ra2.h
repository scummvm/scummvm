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

#ifndef SCUMM_SMUSH_REBEL_CODEC_RA2_H
#define SCUMM_SMUSH_REBEL_CODEC_RA2_H

#include "common/scummsys.h"

namespace Scumm {

void smushDecodeRLEOpaque(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize);
void smushDecodeLineUpdate(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize);
void smushDecodeSkipRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize);
void smushDecodeRA2SkipRemap(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize, const byte *remap, byte addColor);
void smushDecodeRA2Blur(byte *dst, const byte *src, int left, int top, int dstWidth, int dstHeight, int pitch, int dataSize, byte *palette, byte *lookup);
const byte *smushSkipRLELines(const byte *src, int &dataSize, int lines);

} // End of namespace Scumm

#endif
