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

#include "common/endian.h"
#include "lab/lab.h"

namespace Lab {

extern uint16 _dataBytesPerRow;

/*------------------------ unDiff Horizontal Memory -------------------------*/

/*****************************************************************************/
/* Undiffs a piece of memory when header size is a byte, and copy/skip size  */
/* is also a byte.                                                           */
/*****************************************************************************/
static void unDIFFByteByte(byte *dest, byte *diff) {
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



/*****************************************************************************/
/* Undiffs a piece of memory when header size is a byte, and copy/skip size  */
/* is a word.                                                                */
/*****************************************************************************/
static void unDIFFByteWord(uint16 *dest, uint16 *diff) {
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



/*****************************************************************************/
/* UnDiffs a coded DIFF string onto an already initialized piece of memory.  */
/*****************************************************************************/
bool unDIFFMemory(byte *dest, byte *diff, uint16 headerSize, uint16 copySize) {
	if (headerSize == 1) {
		if (copySize == 1)
			unDIFFByteByte(dest, diff);

		else if (copySize == 2)
			unDIFFByteWord((uint16 *)dest, (uint16 *)diff);

		else
			return false;
	} else
		error("unDIFFMemory: HeaderSize is %d", headerSize);

	return true;
}

/*------------------------- unDiff Vertical Memory --------------------------*/

/*****************************************************************************/
/* Undiffs a piece of memory when header size is a byte, and copy/skip size  */
/* is a byte.                                                                */
/*****************************************************************************/
static void VUnDIFFByteByte(byte *Dest, byte *diff, uint16 bytesperrow) {
	byte *CurPtr;
	uint16 skip, copy;
	uint16 counter = 0;


	while (counter < _dataBytesPerRow) {
		CurPtr = Dest + counter;

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
				CurPtr += (skip * bytesperrow);

				while (copy) {
					copy--;
					*CurPtr = *diff;
					CurPtr += bytesperrow;
					diff++;
				}
			}
		}
	}
}




/*****************************************************************************/
/* Undiffs a piece of memory when header size is a byte, and copy/skip size  */
/* is a word.                                                                */
/*****************************************************************************/
static void VUnDIFFByteWord(uint16 *Dest, uint16 *diff, uint16 bytesperrow) {
	uint16 *CurPtr;
	uint16 skip, copy;
	uint16 counter = 0, wordsperrow;


	wordsperrow = bytesperrow / 2;

	while (counter < (_dataBytesPerRow >> 1)) {
		CurPtr = Dest + counter;

		for (;;) {
			skip = ((byte *)diff)[0];
			copy = ((byte *)diff)[1];

			diff++;


			if (skip == 255) {
				counter += copy;
				break;
			}

			else {
				CurPtr += (skip * wordsperrow);

				while (copy) {
					*CurPtr = *diff; //swapUShort(*diff);
					CurPtr += wordsperrow;
					diff++;
					copy--;
				}
			}
		}
	}
}




/*****************************************************************************/
/* Undiffs a piece of memory when header size is a byte, and copy/skip size  */
/* is a long.                                                                */
/*****************************************************************************/
static void VUnDIFFByteLong(uint32 *Dest, uint32 *diff, uint16 bytesperrow) {
	uint32 *CurPtr;
	uint16 skip, copy;
	uint16 counter = 0, longsperrow;
	byte *diff1 = (byte *)diff;


	longsperrow = bytesperrow / 4;

	while (counter < (_dataBytesPerRow >> 2)) {
		CurPtr = Dest + counter;

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
				CurPtr += (skip * longsperrow);

				while (copy) {
					*CurPtr = *(uint32 *)diff1; //swapULong(*diff);
					CurPtr += longsperrow;
					diff1 += 4;
					copy--;
				}
			}
		}
	}
}




/*****************************************************************************/
/* UnDiffs a coded DIFF string onto an already initialized piece of memory.  */
/*****************************************************************************/
bool VUnDIFFMemory(byte *Dest, byte *diff, uint16 HeaderSize, uint16 CopySize, uint16 bytesperrow) {
	if (HeaderSize == 1) {
		if (CopySize == 1)
			VUnDIFFByteByte(Dest, diff, bytesperrow);

		else if (CopySize == 2)
			VUnDIFFByteWord((uint16 *)Dest, (uint16 *)diff, bytesperrow);

		else if (CopySize == 4)
			VUnDIFFByteLong((uint32 *)Dest, (uint32 *)diff, bytesperrow);

		else
			return false;
	} else
		return (false);

	return true;
}


/*****************************************************************************/
/* Runlength decodes a chunk of memory.                                      */
/*****************************************************************************/
void runLengthDecode(byte *Dest, byte *Source) {
	int8 num;
	int16 count;


	while (1) {
		num = (int8)*Source;
		Source++;

		if (num == 127) {
			return;
		} else if (num > '\0') {
			memcpy(Dest, Source, num);
			Source += num;
			Dest   += num;
		} else {
			count = (int16)(-num);
			num   = *Source;
			Source++;

			while (count) {
				*Dest = num;
				Dest++;
				count--;
			}
		}
	}
}




/*****************************************************************************/
/* Does a vertical run length decode.                                        */
/*****************************************************************************/
void VRunLengthDecode(byte *Dest, byte *Source, uint16 bytesperrow) {
	int8 num;
	int16 count;
	byte *Top = Dest;

	for (uint16 i = 0; i < _dataBytesPerRow; i++) {
		Dest = Top;
		Dest += i;

		num = (int8)*Source;
		Source++;

		while (num != 127) {
			if (num > '\0') {
				while (num) {
					*Dest = *Source;
					Source++;
					Dest += bytesperrow;
					num--;
				}
			} else {
				count = (int16)(-num);
				num   = (int8)*Source;
				Source++;

				while (count) {
					*Dest = num;
					Dest += bytesperrow;
					count--;
				}
			}

			num = *Source;
			Source++;
		}
	}
}

} // End of namespace Lab
