/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#include <string.h>
#include "common/scummsys.h"

#define ROL(x, n) (((x) << (n)) | ((x) >> (16 - (n))))
#define ROR(x, n) (((x) << (16 - (n))) | ((x) >> (n)))
#define XCHG(a, b) (a ^=b, b ^= a, a ^= b)

//conditional flags
#define CHECKSUMS	   1
#define PROTECTED	   0

//return codes
#define NOT_PACKED	  0
#define PACKED_CRC	  -1
#define UNPACKED_CRC	-2

//other defines
#define TABLE_SIZE	  (16 * 8)
#define MIN_LENGTH	  2
#define HEADER_LEN	  18

uint16 raw_table[TABLE_SIZE / 2];
uint16 pos_table[TABLE_SIZE / 2];
uint16 len_table[TABLE_SIZE / 2];

#ifdef CHECKSUMS
uint16 crc_table[0x100];
#endif

uint16 bit_buffl = 0;
uint16 bit_buffh = 0;
uint8 bit_count = 0;

uint8 *srcPtr, *dstPtr; //these need to be global because input_bits() uses them

void init_crc()
{
	uint16 cnt = 0;
	uint16 tmp1 = 0;
	uint16 tmp2 = 0;

	for (tmp2 = 0; tmp2 < 0x100; tmp2++) {
		tmp1 = tmp2; 
		for (cnt = 8; cnt > 0; cnt--) {
			if (tmp1 % 2) {
				tmp1 /= 2;
				tmp1 ^= 0x0a001;
			} else
				tmp1 /= 2;
		}
		crc_table[tmp2] = tmp1;
	}
}

//calculate 16 bit crc of a block of memory
uint16 crc_block(uint8 *block, uint32 size)
{
	uint16 crc = 0;
	uint8 *crcTable8 = (uint8 *)crc_table; //make a uint8* to crc_table
	uint8 tmp;
	uint32 i;

	for (i = 0; i < size; i++) {
		tmp = *block++;
		crc ^= tmp; 
		tmp = (uint8)((crc >> 8) & 0x00FF); 
		crc &= 0x00FF;  
		crc = crc << 1; 
		crc = *(uint16 *)&crcTable8[crc];
		crc ^= tmp;
	}

	return crc;
}

uint16 input_bits(uint8 amount)
{
	uint16 newBitBuffh = bit_buffh;
	uint16 newBitBuffl = bit_buffl;
	int16 newBitCount = bit_count;
	uint16 remBits, returnVal;

	returnVal = ((1 << amount) - 1) & newBitBuffl;	
	newBitCount -= amount;

	if (newBitCount < 0) {
		newBitCount += amount;
		XCHG(newBitCount, amount);
		remBits = ROR((uint16)(((1 << amount) - 1) & newBitBuffh), amount);
		newBitBuffh >>= amount;
		newBitBuffl >>= amount;
		newBitBuffl |= remBits;	
		srcPtr += 2;
		newBitBuffh = READ_LE_UINT16(srcPtr);
		XCHG(newBitCount, amount);
		amount -= newBitCount;
		newBitCount = 16 - amount;
	}
	remBits = ROR((uint16)(((1 << amount) - 1) & newBitBuffh), amount);
	bit_buffh = newBitBuffh >> amount;
	bit_buffl = (newBitBuffl >> amount) | remBits;
	bit_count = (uint8)newBitCount;

	return returnVal;
}

void make_huftable(uint16 *table) 
{
	uint16 bitLength, i, j;
	uint16 numCodes = input_bits(5);

	if (!numCodes)
		return;

	uint8 huffLength[16];
	for (i = 0; i < numCodes; i++)
		huffLength[i] = (uint8)(input_bits(4) & 0x00FF);

	uint16 huffCode = 0;

	for (bitLength = 1; bitLength < 17; bitLength++) {
		for (i = 0; i < numCodes; i++) {
			if (huffLength[i] == bitLength) {
				*table++ = (1 << bitLength) - 1;

				uint16 b = huffCode >> (16 - bitLength);
				uint16 a = 0;

				for (j = 0; j < bitLength; j++)
					a |= ((b >> j) & 1) << (bitLength - j - 1);
				*table++ = a;

				*(table + 0x1e) = (huffLength[i] << 8)|(i & 0x00FF);
				huffCode += 1 << (16 - bitLength);
			}
		}
	}
}

uint16 input_value(uint16 *table)
{
	uint16 valOne, valTwo, value = bit_buffl;

	do {
		valTwo = (*table++) & value;
		valOne = *table++;

	} while (valOne != valTwo);

	value = *(table + 0x1e);
	input_bits((uint8)((value>>8) & 0x00FF));
	value &= 0x00FF;

	if (value >= 2) {
		value--;
		valOne = input_bits((uint8)value & 0x00FF);
		valOne |= (1 << value);
		value = valOne;
	}

	return value;
}

int32 UnpackM1(void *input, void *output, uint16 key)
{
	uint8 *inputHigh, *outputLow, *outputHigh;
	uint8 *inputptr = (uint8 *)input;

	uint32 unpack_len = 0;
	uint32 pack_len = 0;
	uint16 counts = 0;

#ifdef CHECKSUMS
	uint16 crc_u = 0;
	uint16 crc_p = 0;
#endif
	
	if (CHECKSUMS)
		init_crc();

	//Check for "RNC "
	if (READ_BE_UINT32(inputptr) != 0x524e4301)
		return NOT_PACKED;

	inputptr += 4;

	// read unpacked/packed file length
	unpack_len = READ_BE_UINT32(inputptr); inputptr += 4;
	pack_len = READ_BE_UINT32(inputptr); inputptr += 4;

	uint8 blocks = *(inputptr + 5);

	if (CHECKSUMS) {
		//read CRC's
		crc_u = READ_BE_UINT16(inputptr); inputptr += 2;
		crc_p = READ_BE_UINT16(inputptr); inputptr += 2;
		inputptr = (inputptr + HEADER_LEN - 16);

		if (crc_block(inputptr, pack_len) != crc_p)
			return PACKED_CRC;

		inputptr = (((uint8 *)input) + HEADER_LEN); 
		srcPtr = inputptr;
	}

	// inputLow = *input
	inputHigh = ((uint8 *)input) + pack_len + HEADER_LEN;;
	outputLow = (uint8 *)output;
	outputHigh = *(((uint8 *)input) + 16) + unpack_len + outputLow;

	if (! ((inputHigh <= outputLow) || (outputHigh <= inputHigh)) ) {
		srcPtr = inputHigh;
		dstPtr = outputHigh;
		memcpy((dstPtr-pack_len), (srcPtr-pack_len), pack_len);
		srcPtr = (dstPtr-pack_len);
	}

	//unpack3:
	dstPtr = (uint8 *)output;
	bit_count = 0;

	bit_buffl = READ_LE_UINT16(srcPtr);
	input_bits(2);
	
	do {
		make_huftable(raw_table);
		make_huftable(pos_table);
		make_huftable(len_table);

		counts = input_bits(16);

		for (;;) {
			uint32 input_bits = input_value(raw_table);

			if (input_bits) {
				memcpy(dstPtr, srcPtr, input_bits); //memcpy is allowed here
				dstPtr += input_bits;
				srcPtr += input_bits;
				uint16 b = READ_LE_UINT16(srcPtr);
				uint16 a = ROL(b, bit_count);
				uint16 d = ((1 << bit_count) - 1);
				bit_buffl &= d;
				d &= a;

				a = READ_LE_UINT16((srcPtr + 2));
				b = (b << bit_count);
				a = (a << bit_count);
				a |= d;
				bit_buffl |= b;
				bit_buffh = a;
			}

			if (--counts) {
				uint32 input_offset = input_value(pos_table) + 1;
				uint32 input_length = input_value(len_table) + MIN_LENGTH;

				inputHigh = srcPtr;
				srcPtr = (dstPtr-input_offset);

				//Don't use memcpy here! because input and output overlap	
				while (input_length--)
					*dstPtr++ = *srcPtr++;

				srcPtr = inputHigh;
			} else
				break;

		}
	} while (--blocks);

	if (CHECKSUMS) {
		if (crc_block((uint8 *)output, unpack_len) != crc_u)
			return UNPACKED_CRC;
	}

	// all is done..return the amount of unpacked bytes
	return unpack_len;
}

