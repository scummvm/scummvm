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

#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "immortal/compression.h"

/* In: Source data as File, size of data, pointer that will be replaced with pointer to the data
 * Out: size of uncompressed data
 */
namespace Compression {

int unCompress(Common::File *src, int srcLen, byte *dst) {
	uint16 k12BitWord = 0x0F9F;   		// Code is stored in lower 12 bits of word, probably doesn't need to be const

	Common::MemoryWriteStreamDynamic dstW(DisposeAfterUse::YES);

	//debug("lets get started");

	// If the source data has no length, we certainly do not want to decompress it
	if (srcLen == 0) {
		return -1;						// Classic C error code
	}

	//debug("not null at least");
	// This is the 20k bytes the compression gets allocated to work with for the dictionary and the stack of chars
    uint16 start[0x4000];       		// Rename! <- I did? Probably still needs a better name though
    uint16   ptk[0x4000];         		// Pointer to keys?
    byte   stack[0x4000];       		// Stack of chars to be stored

    // These are the main variables we'll need for this. They were basically scratch ram in the original
    uint16 findEmpty;
    uint16 code;                		// Needs to be ASL to index with
    uint16 inputCode;
    uint16 finalChar;
    uint16 myCode;              		// Silly name is silly
    uint16 oldCode;
    uint16 index;             	  		// The Y register was used to index the byte array's, this will sort of take its place
	uint16 evenOdd = 0;
	uint16 topStack = 0;

	byte outByte;

	setupDictionary(start, ptk, findEmpty);	// Clear the dictionary and also set findEmpty to 8k for some reason
	bool carry = true;					// This will represent the carry flag so we can make this a clean loop

	code = getInputCode(carry, src, srcLen, evenOdd); // Get the first code
	if (carry == false) {
		return -1;						// This is essentially the same as the first error check, but the source returns an error code and didn't even check it here
	}

	finalChar = code;
	  oldCode = code;
	   myCode = code;

	outByte = code & 0x00FF;
	dstW.writeByte(outByte);			// Take just the lower byte and write it the output
	
	//debug("first data write: %02X", outByte);

	// :nextcode
	while (carry == true) {

		code = getInputCode(carry, src, srcLen, evenOdd); // Get the next code
		if (carry == true) {

			index = code << 1; 			// Could just write this code*2 probably
			inputCode = code;
			myCode = code;

			//debug("%04X, %04X, %04X", index, inputCode, myCode);

			uint16 a = start[index] & 0x0F00;
			uint16 b = ptk[index] & 0xFF00;
			if ((a & b) == 0) {			// Empty code
				index = topStack;
				outByte = finalChar & 0x00FF;
				stack[index] = outByte;
				topStack++;
				myCode = oldCode;
			}

			//debug("%04X, %04X, %04X, %04X, %02X", index, inputCode, myCode, topStack, outByte);

			//debug("nextsymbol");
			// :nextsymbol
			index = myCode << 1;
			while (index >= 0x200) {
				myCode = start[index] & k12BitWord;
				outByte = ptk[index] & 0x00FF;
				index = topStack;
				stack[index] = outByte;
				topStack++;
				//debug("i: %02X, tB: %02X, mC: %02X, b: %02X", index, topStack, myCode, outByte);
				index = myCode << 1;
			}

			//debug("singlechar");
			// :singlechar
			finalChar = (myCode >> 1);
			outByte = finalChar & 0x00FF;
			
			//debug("second data write: %02X", outByte);
			dstW.writeByte(outByte);

			//debug("topstack %d", topStack);

			// :dump
			while (topStack != 0xFFFF) {	// Dump the chars on the stack into the output file
				outByte = stack[topStack] & 0x00FF;
				//debug("dumping stack %02X", temp);
				dstW.writeByte(outByte);
				topStack--;
			}
			topStack = 0;				// Won't this always be 0 because of the while loop?
			code = getMember(oldCode, finalChar, findEmpty, start, ptk);
			oldCode = inputCode;
		}
	}
	dst = dstW.getData();
	return dstW.size();
}

void setupDictionary(uint16 start[], uint16 ptk[], uint16 &findEmpty) {
	for (int i = 0x3FFF; i >= 0; i--) {
		start[i] = 0;
		ptk[i] = 0;
	}
	for (int i=0x0FF; i >=0; i--) {
		ptk[i] = 0x100;
	}
	findEmpty = 0x8000;
}

int getInputCode(bool &carry, Common::File *src, int &srcLen, uint16 &evenOdd) {
	uint16 k12BitWord = 0x0F9F;

	if (srcLen == 0) {
		carry = false;
		return 0;
	}

	uint16 c;
	if (evenOdd != 0) {						// Odd
		srcLen--;
		evenOdd--;
		c = (src->readUint16LE() >> 3) & 0x00FE;	// & #-1-1 ????
	} else {								// Even
		srcLen -= 2;
		evenOdd++;
		c = (src->readUint16LE() & k12BitWord) << 1;
		src->seek(-1, SEEK_CUR);
	}
	return c;
}

// This function is effectively void, as the return value is only used in compression
uint16 getMember(uint16 codeW, uint16 k, uint16 &findEmpty, uint16 start[], uint16 ptk[]) {
	// k and codeW are local variables with the value of oldCode and finalChar
	uint16 k12BitWord = 0x0F9F;

	uint16 hash;
	hash = (k << 3) ^ k;
	hash = (hash << 1) ^ codeW;
	hash <<= 1;

	hash = (hash >= 0x200) ? hash : hash + 0x200;

	uint16 a = start[hash] & 0x0F00;
	uint16 b = ptk[hash] & 0xFF00;
	if (a | b) {
		start[hash] = codeW;
		ptk[hash] = k | 0x100;
		return k | 0x100;
	}

	bool ag = true;
	while (ag == true) {
		if ((start[hash] & k12BitWord) == codeW) {
			if ((ptk[hash] & 0x00FF) == k) {
				return hash >> 1;
			}
		}

		uint16 tmp = start[hash] & 0xF000;
		if (tmp == 0) {
			appendList(codeW, k, hash, findEmpty, start, ptk, tmp);
			ag = false;
		} else {
			tmp >>= 4;
			hash &= 0xFF00;					// The 65816 can XBA to flip the bytes in a word, instead we have mask and shift over
			hash >>= 8;
			hash = (hash | tmp) << 1;
		}
	}
	return hash;
}

void appendList(uint16 codeW, uint16 k, uint16 &hash, uint16 &findEmpty, uint16 start[], uint16 ptk[], uint16 &tmp) {
	uint16 prev;
	uint16 link;

	prev = hash;
	if (hash >= 0x200) {
		setupDictionary(start, ptk, findEmpty);
		return;
	}

	bool found = false;
	while (found == false) {
		hash -= 2;
		if (hash >= 0x200) {
			setupDictionary(start, ptk, findEmpty);
			found = true;
		}
		uint16 cond;
		cond = start[hash] & 0xF000;
		cond |= ptk[hash];
		if ( (cond & 0xFF00) == 0) {
			findEmpty = hash;
			start[hash] = codeW;
			ptk[hash] = k | 0x100;
			link = hash >> 1;
			tmp = link & 0x00FF;			// Another classic XBA situation, although this time it's no less efficient here
			tmp <<= 8;
			ptk[prev] = (ptk[prev] & 0x00FF) | tmp;
			start[prev] = ((link >> 4) & 0xF000) | start[prev];		// Yikes this statement is gross
			found = true;
		}
	}
}

} // namespace compression


























