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

#ifndef BAGEL_API_SMACKER_H
#define BAGEL_API_SMACKER_H

#include "common/scummsys.h"

namespace Bagel {

// Random values for needed constants just to let it compile
#define SMACKTRACKS 1
#define SMACKAUTOEXTRA 1
#define SMACKPRELOADALL 1

struct SmackBuf {};
struct Smack {
	int Frames = 0;
	int Width = 0;
	int Height = 0;
	int FrameNum = 0;
	byte Palette[256 * 3];
};

extern Smack *SmackOpen(const char *filename, int param1, int param2);

extern void SmackToBuffer(Smack *Smack, uint32 Unknown1, uint32 Unknown2,
	uint32 Stride, uint32 FrameHeightInPixels, char *OutBuffer,
	uint32 Flags);

extern bool SmackDoFrame(Smack *smack);
extern void SmackNextFrame(Smack *smack);
inline bool SmackWait(Smack *smack) { return false; }
inline void SmackClose(Smack *smack) {}
inline void SmackBufferClose(SmackBuf *buf) {}
inline void SmackGoto(Smack *smack, int param1) {}

} // namespace Bagel

#endif
