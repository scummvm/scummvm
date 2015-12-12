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
Utils::Utils(LabEngine *vm) : _vm(vm) {
	_dataBytesPerRow = 0;
}

/**
 * Scales the x co-ordinates to that of the new display.  In the room parser
 * file, co-ordinates are set up on a 360x336 display.
 */
uint16 Utils::scaleX(uint16 x) {
	if (_vm->_isHiRes)
		return (uint16)((x * 16) / 9);
	else
		return (uint16)((x * 8) / 9);
}

/**
 * Scales the y co-ordinates to that of the new display.  In the room parser
 * file, co-ordinates are set up on a 368x336 display.
 */
uint16 Utils::scaleY(uint16 y) {
	if (_vm->_isHiRes)
		return (y + (y / 14));
	else
		return ((y * 10) / 24);
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

/**
 * Scales the VGA coords to SVGA if necessary; otherwise, returns VGA coords.
 */
int16 Utils::vgaScaleX(int16 x) {
	if (_vm->_isHiRes)
		return (x * 2);
	else
		return x;
}

/**
 * Scales the VGA coords to SVGA if necessary; otherwise, returns VGA coords.
 */
int16 Utils::vgaScaleY(int16 y) {
	if (_vm->_isHiRes)
		return ((y * 12) / 5);
	else
		return y;
}

uint16 Utils::svgaCord(uint16 cord) {
	if (_vm->_isHiRes)
		return cord;
	else
		return 0;
}

/**
 * Converts SVGA coords to VGA if necessary, otherwise returns VGA coords.
 */
Common::Point Utils::vgaUnscale(Common::Point pos) {
	Common::Point result;
	if (_vm->_isHiRes) {
		result.x = pos.x / 2;
		result.y = (pos.y * 5) / 12;
	} else {
		result = pos;
	}

	return result;
}

/**
 * Undiffs a piece of memory when header size is a byte, and copy/skip size
 * is also a byte.
 */
void Utils::unDiffByteByte(byte *dest, byte *diff) {
	uint16 skip, copy;

	while (1) {
		skip = *diff;
		diff++;
		copy = *diff;
		diff++;

		if (skip == 255) {
			if (copy == 0) {
				skip = READ_LE_UINT16(diff);
				diff += 2;
				copy = READ_LE_UINT16(diff);
				diff += 2;
			} else if (copy == 255)
				return;
		}

		dest += skip;
		memcpy(dest, diff, copy);
		dest += copy;
		diff += copy;
	}
}

/**
 * Undiffs a piece of memory when header size is a byte, and copy/skip size
 * is a word.
 */
void Utils::unDiffByteWord(uint16 *dest, uint16 *diff) {
	uint16 skip, copy;

	while (1) {
		skip = ((byte *)diff)[0];
		copy = ((byte *)diff)[1];

		diff++;

		if (skip == 255) {
			if (copy == 0) {
				skip = READ_LE_UINT16(diff);
				diff++;
				copy = READ_LE_UINT16(diff);
				diff++;
			} else if (copy == 255)
				return;
		}

		dest += skip;

		while (copy > 3) {
			*dest = READ_LE_UINT16(diff);
			dest++;
			diff++;

			*dest = READ_LE_UINT16(diff);
			dest++;
			diff++;

			*dest = READ_LE_UINT16(diff);
			dest++;
			diff++;

			*dest = READ_LE_UINT16(diff);
			dest++;
			diff++;

			copy -= 4;
		}

		while (copy) {
			*dest = READ_LE_UINT16(diff);
			dest++;
			diff++;
			copy--;
		}
	}
}

/*------------------------- unDiff Vertical Memory --------------------------*/

/**
 * Undiffs a piece of memory when header size is a byte, and copy/skip size
 * is a byte.
 */
void Utils::VUnDiffByteByte(byte *dest, byte *diff, uint16 bytesPerRow) {
	byte *curPtr;
	uint16 skip, copy;
	uint16 counter = 0;


	while (counter < _dataBytesPerRow) {
		curPtr = dest + counter;

		for (;;) {
			skip = *diff;
			diff++;
			copy = *diff;
			diff++;

			if (skip == 255) {
				counter += copy;
				break;
			}

			else {
				curPtr += (skip * bytesPerRow);

				while (copy) {
					copy--;
					*curPtr = *diff;
					curPtr += bytesPerRow;
					diff++;
				}
			}
		}
	}
}

/**
 * Undiffs a piece of memory when header size is a byte, and copy/skip size
 * is a word.
 */
void Utils::VUnDiffByteWord(uint16 *dest, uint16 *diff, uint16 bytesPerRow) {
	uint16 *curPtr;
	uint16 skip, copy;
	uint16 counter = 0;

	uint16 wordsPerRow = bytesPerRow / 2;

	while (counter < (_dataBytesPerRow >> 1)) {
		curPtr = dest + counter;

		for (;;) {
			skip = ((byte *)diff)[0];
			copy = ((byte *)diff)[1];

			diff++;


			if (skip == 255) {
				counter += copy;
				break;
			}

			else {
				curPtr += (skip * wordsPerRow);

				while (copy) {
					*curPtr = *diff; //swapUShort(*diff);
					curPtr += wordsPerRow;
					diff++;
					copy--;
				}
			}
		}
	}
}

/**
 * Undiffs a piece of memory when header size is a byte, and copy/skip size
 * is a long.
 */
void Utils::VUnDiffByteLong(uint32 *dest, uint32 *diff, uint16 bytesPerRow) {
	uint32 *_curPtr;
	uint16 skip, copy;

	uint16 counter = 0;
	byte *diff1 = (byte *)diff;

	uint16 longsperrow = bytesPerRow / 4;

	while (counter < (_dataBytesPerRow >> 2)) {
		_curPtr = dest + counter;

		for (;;) {
			skip = *diff1;
			diff1++;

			copy = *diff1;
			diff1++;


			if (skip == 255) {
				counter += copy;
				break;
			}

			else {
				_curPtr += (skip * longsperrow);

				while (copy) {
					*_curPtr = *(uint32 *)diff1; //swapULong(*diff);
					_curPtr += longsperrow;
					diff1 += 4;
					copy--;
				}
			}
		}
	}
}

/**
 * Runlength decodes a chunk of memory.
 */
void Utils::runLengthDecode(byte *dest, byte *source) {
	int8 num;
	int16 count;

	while (1) {
		num = (int8)*source;
		source++;

		if (num == 127) {
			return;
		} else if (num > '\0') {
			memcpy(dest, source, num);
			source += num;
			dest   += num;
		} else {
			count = (int16)(-num);
			num   = *source;
			source++;

			while (count) {
				*dest = num;
				dest++;
				count--;
			}
		}
	}
}

/**
 * Does a vertical run length decode.
 */
void Utils::VRunLengthDecode(byte *dest, byte *source, uint16 bytesPerRow) {
	int8 num;
	int16 count;
	byte *top = dest;

	for (uint16 i = 0; i < _dataBytesPerRow; i++) {
		dest = top;
		dest += i;

		num = (int8)*source;
		source++;

		while (num != 127) {
			if (num > '\0') {
				while (num) {
					*dest = *source;
					source++;
					dest += bytesPerRow;
					num--;
				}
			} else {
				count = (int16)(-num);
				num   = (int8)*source;
				source++;

				while (count) {
					*dest = num;
					dest += bytesPerRow;
					count--;
				}
			}

			num = *source;
			source++;
		}
	}
}

/**
 * Does the undiffing between the bitmaps.
 */
void Utils::unDiff(byte *newBuf, byte *oldBuf, byte *diffData, uint16 bytesPerRow, bool isV) {
	diffData++;
	byte bufType = *diffData;
	diffData++;

	if (isV) {
		if (bufType == 0)
			VUnDiffByteByte(newBuf, diffData, bytesPerRow);
		else if (bufType == 1)
			VUnDiffByteWord((uint16 *)newBuf, (uint16 *)diffData, bytesPerRow);
		else if (bufType == 3)
			VUnDiffByteLong((uint32 *)newBuf, (uint32 *)diffData, bytesPerRow);
		else
			error("Unexpected variable compression scheme %d", bufType);
	} else {
		if (bufType == 0)
			unDiffByteByte(newBuf, diffData);
		else if (bufType == 1)
			unDiffByteWord((uint16 *)newBuf, (uint16 *)diffData);
		else
			error("Unexpected compression scheme %d", bufType);
	}
}

void Utils::setBytesPerRow(int num) {
	_dataBytesPerRow = num;
}

/**
 * Generates a random number.
 */
uint16 Utils::getRandom(uint16 max) {
	uint32 secs, micros;

	getTime(&secs, &micros);
	return ((micros + secs) % max);
}

void Utils::readBlock(void *Buffer, uint32 Size, byte **File) {
	memcpy(Buffer, *File, (size_t)Size);
	(*File) += Size;
}

/**
 * Waits for for Secs seconds and Micros microseconds to pass.
 */
void Utils::microDelay(uint32 secs, uint32 micros) {
	uint32 waitSecs, waitMicros;
	addCurTime(secs, micros, &waitSecs, &waitMicros);

	while (1) {
		getTime(&secs, &micros);

		if ((secs > waitSecs) || ((secs == waitSecs) && (micros >= waitMicros)))
			return;

		g_system->delayMillis(10);
	}
}

/**
 * Gets the current system time.
 */
void Utils::getTime(uint32 *secs, uint32 *micros) {
	uint32 t = g_system->getMillis();

	*secs = t / 1000;
	*micros = t % 1000;
}

/**
 * Adds seconds and microseconds to current time to get a new time.
 */
void Utils::addCurTime(uint32 sec, uint32 micros, uint32 *timeSec, uint32 *timeMicros) {
	getTime(timeSec, timeMicros);

	(*timeSec) += sec;
	(*timeMicros) += micros;

	if (*timeMicros >= ONESECOND) {
		(*timeSec)++;
		(*timeMicros) -= ONESECOND;
	}
}

/**
 * Finds the difference between time1 and time2.  If time1 is later than
 * time2, returns 0.
 */
void Utils::anyTimeDiff(uint32 sec1, uint32 micros1, uint32 sec2, uint32 micros2, uint32 *diffSecs, uint32 *diffMicros) {
	*diffSecs = 0;
	*diffMicros = 0;

	if (sec1 > sec2)
		return;
	else if ((sec1 == sec2) && (micros1 >= micros2))
		return;

	if (micros1 > micros2) {
		*diffSecs = sec2 - sec1 - 1;
		*diffMicros = (ONESECOND - micros1) + micros2;
	} else {
		*diffSecs = sec2 - sec1;
		*diffMicros = micros2 - micros1;
	}
}

/**
 * Finds the difference between the current time, and a future time. Returns
 * 0 if the future time is actually before the current time.
 */
void Utils::timeDiff(uint32 sec, uint32 micros, uint32 *diffSec, uint32 *diffMicros) {
	uint32 curSec, curMicros;
	getTime(&curSec, &curMicros);
	anyTimeDiff(curSec, curMicros, sec, micros, diffSec, diffMicros);
}

/**
 * Waits for a specified time to occur.
 */
void Utils::waitForTime(uint32 sec, uint32 micros) {
	uint32 curSec, curMicros;
	getTime(&curSec, &curMicros);

	if (curSec > sec)
		return;
	else if ((curSec == sec) && (curMicros >= micros))
		return;

	if (curMicros > micros)
		microDelay(sec - curSec - 1, (ONESECOND - curMicros) + micros - 1);
	else
		microDelay(sec - curSec, micros - curMicros - 1);
}
} // End of namespace Lab
