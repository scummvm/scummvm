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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_UTILS_H
#define LAB_UTILS_H

namespace Lab {

class Utils {
private:
	LabEngine *_vm;
	uint16 _dataBytesPerRow;

	void unDiffByteByte(byte *dest, byte *diff);
	void unDiffByteWord(uint16 *dest, uint16 *diff);
	void VUnDiffByteByte(byte *dest, byte *diff, uint16 bytesPerRow);
	void VUnDiffByteWord(uint16 *dest, uint16 *diff, uint16 bytesPerRow);
	void VUnDiffByteLong(uint32 *dest, uint32 *diff, uint16 bytesPerRow);

public:
	Utils(LabEngine *vm);

	Common::RandomSource _rnd;

	uint16 scaleX(uint16 x);
	uint16 scaleY(uint16 y);
	int16 vgaScaleX(int16 x);
	int16 vgaScaleY(int16 y);
	uint16 svgaCord(uint16 cord);
	uint16 mapScaleX(uint16 x);
	uint16 mapScaleY(uint16 y);
	Common::Point vgaUnscale(Common::Point pos);
	void unDiff(byte *newBuf, byte *oldBuf, byte *diffData, uint16 bytesPerRow, bool isV);
	void runLengthDecode(byte *dest, byte *source);
	void VRunLengthDecode(byte *dest, byte *source, uint16 bytesPerRow);
	void setBytesPerRow(int num);
	uint16 getRandom(uint16 max);
};


} // End of namespace Lab

#endif // LAB_UTILS_H
