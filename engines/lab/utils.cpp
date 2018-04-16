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

#include "common/file.h"

#include "lab/lab.h"
#include "lab/utils.h"

namespace Lab {
Utils::Utils(LabEngine *vm) : _vm(vm), _rnd("lab") {
	_dataBytesPerRow = 0;
}

uint16 Utils::scaleX(uint16 x) {
	if (_vm->_isHiRes)
		return (uint16)((x * 16) / 9);
	else
		return (uint16)((x * 8) / 9);
}

uint16 Utils::scaleY(uint16 y) {
	if (_vm->_isHiRes)
		return (y + (y / 14));
	else
		return ((y * 10) / 24);
}

Common::Rect Utils::rectScale(int16 x1, int16 y1, int16 x2, int16 y2) {
	return Common::Rect(scaleX(x1), scaleY(y1), scaleX(x2), scaleY(y2));
}

uint16 Utils::mapScaleX(uint16 x) {
	if (_vm->_isHiRes)
		return (x - 45);
	else
		return ((x - 45) >> 1);
}

uint16 Utils::mapScaleY(uint16 y) {
	if (_vm->_isHiRes)
		return y;
	else
		return ((y - 35) >> 1) - (y >> 6);
}

Common::Rect Utils::mapRectScale(int16 x1, int16 y1, int16 x2, int16 y2) {
	return Common::Rect(mapScaleX(x1), mapScaleY(y1), mapScaleX(x2), mapScaleY(y2));
}

int16 Utils::vgaScaleX(int16 x) {
	if (_vm->_isHiRes)
		return (x * 2);
	else
		return x;
}

int16 Utils::vgaScaleY(int16 y) {
	if (_vm->_isHiRes)
		return ((y * 12) / 5);
	else
		return y;
}

Common::Rect Utils::vgaRectScale(int16 x1, int16 y1, int16 x2, int16 y2) {
	return Common::Rect(vgaScaleX(x1), vgaScaleY(y1), vgaScaleX(x2), vgaScaleY(y2));
}

uint16 Utils::svgaCord(uint16 cord) {
	if (_vm->_isHiRes)
		return cord;
	else
		return 0;
}

Common::Point Utils::vgaUnscale(Common::Point pos) {
	Common::Point result;
	if (_vm->_isHiRes) {
		result.x = pos.x / 2;
		result.y = (pos.y * 5) / 12;
	} else
		result = pos;

	return result;
}

template<typename T>
void Utils::unDiff(T *dest, Common::File *sourceFile) {
	byte bytesPerWord = sizeof(T);

	while (1) {
		uint16 skip = sourceFile->readByte();
		uint16 copy = sourceFile->readByte();

		if (skip == 255) {
			if (copy == 0) {
				skip = sourceFile->readUint16LE();
				copy = sourceFile->readUint16LE();
			} else if (copy == 255)
				return;
		}

		dest += skip;

		if (bytesPerWord == 1) {
			sourceFile->read(dest, copy);
			dest += copy;
		} else {
			while (copy) {
				*dest = sourceFile->readUint16LE();
				dest++;
				copy--;
			}
		}
	}
}

template<typename T>
void Utils::verticalUnDiff(T *dest, Common::File *sourceFile, uint16 bytesPerRow) {
	uint16 counter = 0;
	byte bytesPerWord = sizeof(T);
	uint16 wordsPerRow = bytesPerRow / bytesPerWord;

	while (counter < wordsPerRow) {
		T *curPtr = dest + counter;

		for (;;) {
			uint16 skip = sourceFile->readByte();
			uint16 copy = sourceFile->readByte();

			if (skip == 255) {
				counter += copy;
				break;
			} else {
				curPtr += (skip * wordsPerRow);

				while (copy) {
					if (bytesPerWord == 1)
						*curPtr = sourceFile->readByte();
					else if (bytesPerWord == 2)
						*curPtr = sourceFile->readUint16LE();
					else if (bytesPerWord == 4)
						*curPtr = sourceFile->readUint32LE();
					else
						error("verticalUnDiff: Invalid bytesPerWord (%d)", bytesPerWord);
					curPtr += wordsPerRow;
					copy--;
				}
			}
		}
	}
}

void Utils::runLengthDecode(byte *dest, Common::File *sourceFile) {
	int8 num;
	int16 count;

	while (1) {
		num = sourceFile->readSByte();

		if (num == 127) {
			return;
		} else if (num > '\0') {
			sourceFile->read(dest, num);
			dest   += num;
		} else {
			count = (int16)(-num);
			num = sourceFile->readSByte();

			while (count) {
				*dest = num;
				dest++;
				count--;
			}
		}
	}
}

void Utils::verticalRunLengthDecode(byte *dest, Common::File *sourceFile, uint16 bytesPerRow) {
	int16 count;
	byte *top = dest;

	for (int i = 0; i < _dataBytesPerRow; i++) {
		dest = top;
		dest += i;

		int8 num = sourceFile->readSByte();

		while (num != 127) {
			if (num > '\0') {
				while (num) {
					*dest = sourceFile->readByte();
					dest += bytesPerRow;
					num--;
				}
			} else {
				count = (int16)(-num);
				num = sourceFile->readSByte();

				while (count) {
					*dest = num;
					dest += bytesPerRow;
					count--;
				}
			}

			num = sourceFile->readSByte();
		}
	}
}

void Utils::unDiff(byte *newBuf, byte *oldBuf, Common::File *sourceFile, uint16 bytesPerRow, bool isVertical) {
	sourceFile->skip(1);
	byte bufType = sourceFile->readByte();

	if (isVertical) {
		if (bufType == 0)
			verticalUnDiff<byte>(newBuf, sourceFile, bytesPerRow);
		else if (bufType == 1)
			verticalUnDiff<uint16>((uint16 *)newBuf, sourceFile, bytesPerRow);
		else if (bufType == 3)
			verticalUnDiff<uint32>((uint32 *)newBuf, sourceFile, bytesPerRow);
		else
			error("Unexpected variable compression scheme %d", bufType);
	} else {
		if (bufType == 0)
			unDiff<byte>(newBuf, sourceFile);
		else if (bufType == 1)
			unDiff<uint16>((uint16 *)newBuf, sourceFile);
		else
			error("Unexpected compression scheme %d", bufType);
	}
}

void Utils::setBytesPerRow(int num) {
	_dataBytesPerRow = num;
}

uint16 Utils::getRandom(uint16 max) {
	if (max > 1)
		return _rnd.getRandomNumber(max - 1);
	else
		return 0;
}

} // End of namespace Lab
