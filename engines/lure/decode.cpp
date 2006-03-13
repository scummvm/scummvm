/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "lure/decode.h"
#include "lure/memory.h"
#include "lure/luredefs.h"

namespace Lure {

/*--------------------------------------------------------------------------*/
/* PictureDecoder class                                                     */
/*                                                                          */
/* Provides the functionality for decoding screens                          */
/*--------------------------------------------------------------------------*/

void PictureDecoder::writeByte(MemoryBlock *dest, byte v) {
	if (outputOffset == dest->size()) 
		error("Decoded data exceeded allocated output buffer size");
	dest->data()[outputOffset++] = v;
}

void PictureDecoder::writeBytes(MemoryBlock *dest, byte v, uint16 numBytes) {
	if (outputOffset + numBytes > dest->size()) 
		error("Decoded data exceeded allocated output buffer size");
	dest->setBytes(v, outputOffset, numBytes);
	outputOffset += numBytes;
}

byte PictureDecoder::DSSI(bool incr) {
	byte result = dataIn[dataPos];
	if (incr) ++dataPos;
	return result;
}

byte PictureDecoder::ESBX(bool incr) {
	byte result = dataIn[dataPos2];
	if (incr) ++dataPos2;
	return result;
}

void PictureDecoder::decrCtr() {
	--CL;
	if (CL == 0) {
		CH = ESBX();
		CL = 8;
	}
}

bool PictureDecoder::shlCarry() {
	bool result = (CH & 0x80) != 0;
	CH <<= 1;
	return result;
}

void PictureDecoder::swap(uint16 &v1, uint16 &v2) {
	uint16 vTemp;
	vTemp = v1; 
	v1 = v2;
	v2 = vTemp;
}

// decode_data
// Takes care of decoding compressed Lure of the Temptress data

MemoryBlock *PictureDecoder::decode(MemoryBlock *src, uint32 maxOutputSize) {
	MemoryBlock *dest = Memory::allocate(maxOutputSize);

	// Set up initial states
	dataIn = src->data();
	outputOffset = 0;
	dataPos = READ_LE_UINT32(dataIn + 0x400);
	dataPos2 = 0x404;

	CH = ESBX();
	CL = 9;

Loc754:
	AL = DSSI();
	writeByte(dest, AL);
	BP = ((uint16) AL) << 2;

Loc755:
	decrCtr();
	if (shlCarry()) goto Loc761;
	decrCtr();
	if (shlCarry()) goto Loc759;
	AL = dataIn[BP];

Loc758:
	writeByte(dest, AL);
	BP = ((uint16) AL) << 2;
	goto Loc755;

Loc759:
	AL = (byte) (BP >> 2);
	AH = DSSI();
	if (AH == 0) goto Loc768;

	writeBytes(dest, AL, AH);
	goto Loc755;

Loc761:
	decrCtr();
	if (shlCarry()) goto Loc765;
	decrCtr();

	if (shlCarry()) goto Loc764;
	AL = dataIn[BP+1];
	goto Loc758;

Loc764:
	AL = dataIn[BP+2];
	goto Loc758;

Loc765:
	decrCtr();
	if (shlCarry()) goto Loc767;
	AL = dataIn[BP+3];
	goto Loc758;

Loc767:
	goto Loc754;

Loc768:
	AL = DSSI();
	if (AL != 0) goto Loc755;

	// Resize the output to be the number of outputed bytes and return it
	if (outputOffset < dest->size()) dest->reallocate(outputOffset);
	return dest;
}

/*--------------------------------------------------------------------------*/
/* AnimationDecoder class                                                   */
/*                                                                          */
/* Provides the functionality for decoding animations                       */
/*--------------------------------------------------------------------------*/

// The code below is responsible for decompressing the pixel data
// for an animation. I'm not currently sure of the of the exact details
// of the compression format - for now I've simply copied the code
// from the executable

void AnimationDecoder::rcl(uint16 &value, bool &carry) {
	bool result = (value & 0x8000) != 0;
	value = (value << 1) + (carry ? 1 : 0);
	carry = result;
}

#define GET_BYTE currData = (currData & 0xff00) | *pSrc++
#define BX_VAL(x) *((byte *) (dest->data() + tableOffset + x))
#define SET_HI_BYTE(x,v) x = (x & 0xff) | ((v) << 8);
#define SET_LO_BYTE(x,v) x = (x & 0xff00) | (v);

void AnimationDecoder::decode_data_2(byte *&pSrc, uint16 &currData, uint16 &bitCtr, 
									 uint16 &dx, bool &carry) {
	SET_HI_BYTE(dx, currData >> 8);
	
	for (int v = 0; v < 8; ++v) {
		rcl(currData, carry);
		if (--bitCtr == 0) {
			GET_BYTE;
			bitCtr = 8;
		}
	}
}

uint32 AnimationDecoder::decode_data(MemoryBlock *src, MemoryBlock *dest, uint32 srcPos) {
	byte *pSrc = src->data() + srcPos;
	byte *pDest = dest->data();
	uint16 v;
	bool carry = false;
	uint16 currData, bitCtr, dx;
	byte tableOffset;
	uint16 tempReg1, tempReg2;

	// Handle splitting up 16 bytes into individual nibbles
	for (int numBytes = 0; numBytes < 16; ++numBytes, ++pDest) {
		// Split up next byte to pDest and pDest+0x10
		currData = *pSrc++;
		*(pDest + 0x10) = currData & 0xf;
		*pDest = (currData >> 4) & 0xf;

		// Split up next byte to pDest+0x20 and pDest+0x30
		currData = *pSrc++;
		*(pDest + 0x30) = currData & 0xf;
		*(pDest + 0x20) = (currData >> 4) & 0xf;
	}	

	pDest = (byte *) (dest->data() + 0x40);
	currData = READ_BE_UINT16(pSrc);
	pSrc += sizeof(uint16);

	bitCtr = 4;
	*pDest = (currData >> 8) & 0xf0;
	tableOffset = currData >> 12;
	currData <<= 4;
	dx = 1;

	for (;;) {
		carry = false;
		rcl(currData, carry);
		if (--bitCtr == 0) {
			GET_BYTE;
			bitCtr = 8;
		}
		if (carry) goto loc_1441;
		tableOffset = BX_VAL(0);

loc_1439:
		dx ^= 1;
		if ((dx & 1) != 0) {
			SET_HI_BYTE(dx, tableOffset << 4);
			*pDest = dx >> 8;
		} else {
			*pDest++ |= tableOffset;
		}
		continue;

loc_1441:
		rcl(currData, carry);
		if (--bitCtr == 0) {
			GET_BYTE;
			bitCtr = 8;
		}
		if (!carry) {
			rcl(currData, carry);
			if (--bitCtr == 0) {
				GET_BYTE;
				bitCtr = 8;
			}

			if (!carry) {
				tableOffset = BX_VAL(0x10);
			} else {
				tableOffset = BX_VAL(0x20);
			}
			goto loc_1439;
		}

		rcl(currData, carry);
		if (--bitCtr == 0) {
			GET_BYTE;
			bitCtr = 8;
		}
		if (!carry) {
			tableOffset = BX_VAL(0x30);
			goto loc_1439;
		}

		SET_HI_BYTE(dx, currData >> 12);
		carry = false;
		for (int ctr = 0; ctr < 4; ++ctr) {
			rcl(currData, carry);
			if (--bitCtr == 0) {
				GET_BYTE;
				bitCtr = 8;
			}
		}

		byte dxHigh = dx >> 8;
		if (dxHigh == BX_VAL(0)) {
			tempReg1 = bitCtr;
			tempReg2 = dx;
			decode_data_2(pSrc, currData, bitCtr, dx, carry);
		
			SET_LO_BYTE(dx, dx >> 8);
			decode_data_2(pSrc, currData, bitCtr, dx, carry);
			SET_HI_BYTE(bitCtr, dx & 0xff);
			SET_LO_BYTE(bitCtr, dx >> 8);
			dx = tempReg2;

			if (bitCtr == 0) 
				// Exit out of infinite loop
				break;

		} else if (dxHigh == BX_VAL(0x10)) {
			tempReg1 = bitCtr;
			decode_data_2(pSrc, currData, bitCtr, dx, carry);
			bitCtr = dx >> 8;

		} else if (dxHigh == BX_VAL(0x20)) {
			SET_HI_BYTE(dx, currData >> 10);

			for (v = 0; v < 6; ++v) {
				rcl(currData, carry);
				if (--bitCtr == 0) {
					GET_BYTE;
					bitCtr = 8;
				}
			}

			tempReg1 = bitCtr;
			bitCtr = dx >> 8;
		
		} else if (dxHigh == BX_VAL(0x30)) {
			SET_HI_BYTE(dx, currData >> 11);

			for (v = 0; v < 5; ++v) {
				rcl(currData, carry);
				if (--bitCtr == 0) {
					GET_BYTE;
					bitCtr = 8;
				}
			}

			tempReg1 = bitCtr;
			bitCtr = dx >> 8;

		} else {
			tableOffset = dx >> 8;
			goto loc_1439;
		}

		if ((dx & 1) == 1) {
			*pDest++ |= tableOffset;
			--bitCtr;
			dx &= 0xfffe;
		}

		SET_HI_BYTE(dx, tableOffset << 4);
		tableOffset |= dx >> 8;

		v = bitCtr >> 1;
		while (v-- > 0) *pDest++ = tableOffset;

		bitCtr &= 1;
		if (bitCtr != 0) {
			*pDest = tableOffset & 0xf0;
			dx |= 1; //dx.l
		}

		bitCtr = tempReg1;
		tableOffset &= 0x0f;
	}

	// Return number of bytes written
	return pDest - dest->data();
}

} // end of namespace Lure
