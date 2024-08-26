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

#include "immortal/immortal.h"

/* Decompression:
 * This decompression algorithm follows the source assembly very closely,
 * which is itself a modification to LZW (a derivative of LZ78).
 * In: Source data as File, size of data
 * Out: Pointer to uncompressed data as SeekableReadStream
 */
namespace Immortal {

enum codeMask {
	kMaskMSBS = 0xF000,							// Code link is Most significant bits
	kMaskLSBS = 0xFF00,							// K link is Least significant bits
	kMaskCode = 0x0FFF							// Code is 12 bit
};

Common::SeekableReadStream *ImmortalEngine::unCompress(Common::File *source, int lSource) {
	/* Note: this function does not seek() in the file, which means
	 * that if there is a header on the data, the expectation is that
	 * seek() was already used to move past the header before this function.
	 */

	/* Other notes:
	 * Tk is k in (w,k)
	 * Link is spread out between code and tk, where code has the most significant 4 bits, and tk has the least significant 8
	 * Codes contains the keys (plus link codes) for the substring values of the dictionary and can be up to 12 bits (4096 total entries) in size
	 * Tk contains byte values from the compressed data (plus link codes)
	 * Stack contains the currently being recreated string before it gets sent to the output
	 */

	// In the source, the data allocated here is a pointer passed to the function, but it's only used by this anyway
	uint16 *pCodes = (uint16 *)malloc(k8K);		// The Codes stack has 8 * 1024 bytes allocated
	uint16 *pTk    = (uint16 *)malloc(k8K);		// The Tk has 8 * 1024 bytes allocated
	uint16 pStack[k8K];							// In the source, the stack has the rest of the 20K. That's way more than it needs though, so we're just giving it 8k for now

	uint16 oldCode   = 0;
	uint16 finChar   = 0;
	uint16 topStack  = 0;
	uint16 evenOdd   = 0;
	uint16 myCode    = 0;
	uint16 inCode    = 0;
	uint16 findEmpty = 0;
	uint16 index     = 0;

	// If the source data has no length, we certainly do not want to decompress it
	if (lSource == 0) {
		return nullptr;
	}

	/* This will be the dynamically re-allocated writeable memory stream.
	 * We do not want it to be deleted from scope, as this location is where
	 * the readstream being returned will point to.
	 */
	Common::MemoryWriteStreamDynamic dest(DisposeAfterUse::NO);

	/* In the source we save a backup of the starting pointer to the destination, which is increased
	 * as more data is added to it, so that the final length can be dest - destBkp. However in
	 * our case, the MemoryReadStream already has a size associated with it.
	 */

	// Clear the dictionary
	setUpDictionary(pCodes, pTk, findEmpty);
	evenOdd = 0;
	topStack = 0;

	// Get the initial input (always 0?)
	inputCode(finChar, lSource, source, evenOdd);
	oldCode = finChar;
	myCode = oldCode;

	// (byte) is basically the same as the SEP #$20 : STA : REP #$20
	dest.writeByte((byte)myCode);

	// Loops until it gets no more input codes (ie. length of source is 0)
	while (inputCode(inCode, lSource, source, evenOdd) == 0) {
		myCode = inCode;

		// The source uses the Y register for this
		// We can rearrange this a little to avoid using an extra variable, but for now we're pretending index is the register
		index = inCode;

		/* Check if the code is defined (has links for the linked list).
		 * We do this by grabbing the link portion from the code,
		 * then adding the Tk, and grabbing just the link portion.
		 * This way, if either of the link codes exists, we know it's defined,
		 * otherwise you just get zeros.
		 * This special case is for a string which is the same as the last string,
		 * but with the first char duplicated and added to the end (how common can that possibly be??)
		 */
		if ((((pCodes[index] & kMaskMSBS) | pTk[index]) & kMaskLSBS) == 0) {
			// Push the last char of this string, which is the same as the first of the previous one
			pStack[topStack] = finChar;
			topStack++;
			myCode = oldCode;
		}

		// The code is defined, but it could be either a single char or a multi char
		// If the index into the dictionary is above 100, it's a multi character substring
		while ((myCode) >= 0x100) {
			index = myCode;
			myCode = pCodes[index] & kMaskCode;
			pStack[topStack] = pTk[index] & kMaskLow;
			topStack++;
		}

		// Otherwise, it's a single char
		finChar = myCode;

		// which we write to the output
		dest.writeByte((byte)myCode);

		// Dump the stack
		index = topStack;
		index--;
		while (index < 0x8000) {
			dest.writeByte((byte)pStack[index]);
			index--;
		}
		topStack = 0;

		// Hash the old code with the current char, if it isn't in the dictionary, append it
		member(oldCode, finChar, pCodes, pTk, findEmpty, index);

		// Set up the current code as the old code for the next code
		oldCode = inCode;
	}

	/* Return a readstream with a pointer to the data in the write stream.
	 * This one we do want to dispose after using, because it will be in the scope of the engine itself
	 */
	return new Common::MemoryReadStream(dest.getData(), dest.size(), DisposeAfterUse::YES);

}

/* Clear the tables and mark the first 256 bytes of the char table as used */
void ImmortalEngine::setUpDictionary(uint16 *pCodes, uint16 *pTk, uint16 &findEmpty) {
	// Clear the tables completely (4095 entries, same as the mask for codes)
	for (int i = kMaskCode; i >= 0; i -= 1) {
		pCodes[i] = 0;
		pTk[i] = 0;
	}

	// Mark the first 0x100 as used for uncompress
	for (int i = 0xFF; i >= 0; i -= 1) {
		pTk[i] = 0x100;
	}

	// findEmpty is a pointer for finding empty slots, so it starts at the end of the data (data is 2 bytes wide, so it's 4k instead of 8k)
	findEmpty = k4K;
}

/* Get a code from the input stream. 1 = no more codes, 0 = got code
 * On even iterations, we grab the first word.
 * On odd iterations, we grab the word starting from the second byte of the previous word
 */
int ImmortalEngine::inputCode(uint16 &outCode, int &lSource, Common::File *source, uint16 &evenOdd) {
	// If length is 0, we're done getting codes
	if (lSource == 0) {
		// No more codes
		return 1;
	}

	// Even
	if (evenOdd == 0) {
		lSource -= 2;				// Even number of bytes, decrease by 2
		evenOdd++;					// Next alignment will be odd

		/* The codes are stored in 12 bits, so 3 bytes = 2 codes
		 * nnnn nnnn [nnnn cccc cccc cccc] & 0x0FFF
		 * nnnn nnnn [0000 cccc cccc cccc]
		 */
		outCode = source->readUint16LE() & kMaskCode;
		source->seek(-1, SEEK_CUR);

	// Odd
	} else {
		lSource--;
		evenOdd--;
		/* This grabs the next code which is made up of the previous code's second byte
		 * plus the current code's byte + the next 2 byte value
		 * [nnnn nnnn nnnn cccc] cccc cccc >> 3
		 * [000n nnnn nnnn nnnc] cccc cccc & 0xFFFE		<- this is done so the Y register has code * 2
		 * [000n nnnn nnnn nnn0] cccc cccc >> 1 		<- in our case, we could have just done code >> 4
		 * [0000 nnnn nnnn nnnn]
		 */
		outCode = ((source->readUint16LE() >> 3) & 0xFFFE) >> 1;
	}

	// We have a good code, no error
	return 0;
}

int ImmortalEngine::member(uint16 &codeW, uint16 &k, uint16 *pCodes, uint16 *pTk, uint16 &findEmpty, uint16 &index) {
	// Step one is to make a hash value out of W (oldCode) and k (finChar)
	index = ((((((k << 3) ^ k) << 1) ^ k) ^ codeW));

	// The hash value has to be larger than 200 because that's where the single chars are
	if (index < 0x100) {
		index += 0x100;
	}

	if ((((pCodes[index] & kMaskMSBS) | pTk[index]) & kMaskLSBS) == 0) {
		// There was no link, so we insert the key, mark the table as used, with no link
		pCodes[index] = codeW;
		pTk[index] = k | 0x100;

		// Code not found, return error
		return 1;
	}

	// There is a link, so it's not empty
	// This is a bad loop, because there is no safe way out if the data isn't right, but it's the source logic
	// If there is anything corrupted in the data, the game will get stuck forever
	while (true) {
		uint16 tmp = 0;

		// If the code matches
		if ((pCodes[index] & kMaskCode) == codeW) {
			// And k also matches
			if ((pTk[index] & kMaskLow) == k) {
				// Then entry is found, return no error
				return 0;
			}
		}

		// Entry was used, but it is not holding the desired key
		// Follow link to next entry, if there is no next entry, append to the list
		if ((pCodes[index] & kMaskMSBS) == 0) {
			// Find an empty entry and link it to the last entry in the chain, then put the data in the new entry
			uint16 prev = index;
			if (findEmpty >= 0x100) {
				// Table is not full, keep looking
				do {
					findEmpty--;
					// This is slightly more redundant than the source, but I trust the compiler to add a branch here
					if (findEmpty < 0x100) {
						setUpDictionary(pCodes, pTk, findEmpty);
						return 1;
					}
				// We decrease the index and check the entry until we find an empty entry or the end of the table
				} while ((((pCodes[findEmpty] & kMaskMSBS) | pTk[findEmpty]) & kMaskLSBS) != 0);

				// The link is zero, therefor we have found an empty entry

				pCodes[findEmpty] = codeW;
				pTk[findEmpty] = k | 0x100;			// Marked as used, but still no link because this is the end of the list

				// Now we attach a link to this entry from the previous one in the list
				uint16 link = findEmpty;

				// Get the link of this entry
				/* 0000 llll llll llll xba
				 * llll llll 0000 llll & kMaskLSBS
				 * llll llll 0000 0000
				 */
				tmp = xba(link) & kMaskLSBS;

				// On the previous entry, take out the K and add the new link onto it
				/* xxxx xxxx xxxx xxxx & kMaskLow
				 * 0000 0000 xxxx xxxx | tmp
				 * llll llll xxxx xxxx
				 */
				pTk[prev] = (pTk[prev] & kMaskLow) | tmp;

				// And now the code gets it's half of the link written in
				/* 0000 llll llll llll << 4
				 * llll llll llll llll & kMaskMSBS
				 * llll 0000 0000 0000 | pCodes[Prev]
				 * llll xxxx xxxx xxxx
				 */
				pCodes[prev] = ((link << 4) & kMaskMSBS) | pCodes[prev];

				// Done
				return 1;

			} else {
				// Table is full, reset dictionary
				setUpDictionary(pCodes, pTk, findEmpty);
				return 1;
			}

		} else {
			// Put the link code together by combining the MSBS of the code and the LSBS of k
			/* code = l000 >> 4
			 *      = 0l00
			 * k = ll00 xba
			 *   = 00ll
			 * k | code = 0lll
			 */
			tmp = (pCodes[index] & kMaskMSBS) >> 4;
			index = ((xba(pTk[index]) & kMaskLow) | tmp);// << 1;
		}
	}
}

} // namespace Immortal
