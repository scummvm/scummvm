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

/* Decompression:
 * This decompression algorithm follows the source assembly very closely,
 * which is itself a modification to LZW (a derivitive of LZ78).
 * In: Source data as File, size of data
 * Out: Pointer to uncompressed data as SeekableReadStream
 */
namespace Compression {

// There is a lot of bit masking that needs to happen, so this enum makes it a little easier to read
enum BitMask : uint16 {
    kMask12Bit = 0x0F9F,                    // Code (pos, 00, len) is stored in lower 12 bits of word
    kMaskLow   = 0x00FF,
    kMaskHigh  = 0xFF00,
    kMaskLast  = 0xF000
};

Common::SeekableReadStream *unCompress(Common::File *src, int srcLen) {
    /* Note: this function does not seek() in the file, which means
     * that if there is a header on the data, the expectation is that
     * seek() was already used to move past the header before this function.
     */

    // If the source data has no length, we certainly do not want to decompress it
    if (srcLen == 0) {
        return nullptr;
    }

    /* This will be the dynamically re-allocated writeable memory stream.
     * We do not want it to be deleted from scope, as this location is where
     * the readstream being returned will point to.
     */
    Common::MemoryWriteStreamDynamic dstW(DisposeAfterUse::NO);

    // The 20k bytes of memory that compression gets allocated to work with for the dictionary and the stack of chars
    uint16 start[0x4000];                   // Really needs a better name, remember to do this future me
    uint16   ptk[0x4000];                   // Pointer To Keys? Also needs a better name
      byte stack[0x4000];                   // Stack of chars to be stored

    // These are the main variables we'll need for this
    uint16 findEmpty;
    uint16 code;                            // Needs to be ASL to index with
    uint16 inputCode;
    uint16 finalChar;
    uint16 myCode;                          // Silly name is silly
    uint16 oldCode;
    uint16 index;                           // The Y register was used to index the byte array's, this will sort of take its place
    uint16 evenOdd  = 0;
    uint16 topStack = 0;

    byte outByte;                           // If only we could SEP #$20 like the 65816

    setupDictionary(start, ptk, findEmpty); // Clear the dictionary and also set findEmpty to 8k
    bool carry = true;                      // This will represent the carry flag so we can make this a clean loop

    code = getInputCode(carry, src, srcLen, evenOdd); // Get the first code
    if (carry == false) {
        return nullptr;                     // This is essentially the same as the first error check, but the source returns an error code and didn't even check it here so we might as well
    }

    finalChar = code;
      oldCode = code;
       myCode = code;

    outByte = code & kMaskLow;
    dstW.writeByte(outByte);                // Take just the lower byte and write it the output

    // :nextcode
    while (carry == true) {

        code = getInputCode(carry, src, srcLen, evenOdd); // Get the next code
        if (carry == true) {

                index = code << 1;
            inputCode = code;
               myCode = code;
            
            // Split up the conditional statement to be easier to follow
            uint16 cond;
            cond = start[index] & kMaskLast;
            cond |= ptk[index];

            if ((cond & kMaskHigh) == 0) {  // Empty code
                  index = topStack;
                outByte = finalChar & kMaskLow;
                stack[index] = outByte;
                topStack++;
                myCode = oldCode;
            }

            // :nextsymbol
            index = myCode << 1;
            while (index >= 0x200) {
                 myCode = start[index] & kMask12Bit;
                outByte = ptk[index] & kMaskLow;
                  index = topStack;
                stack[index] = outByte;
                topStack++;
                index = myCode << 1;
            }

            // :singlechar
            finalChar = (myCode >> 1);
              outByte = finalChar & kMaskLow;
            dstW.writeByte(outByte);

            // :dump
            while (topStack != 0xFFFF) {    // Dump the chars on the stack into the output file
                outByte = stack[topStack] & kMaskLow;
                dstW.writeByte(outByte);
                topStack--;
            }

            topStack = 0;                   // Won't this always be 0 because of the while loop?
                code = getMember(oldCode, finalChar, findEmpty, start, ptk);
             oldCode = inputCode;
        }

    }

    // Return a readstream with a pointer to the data in the write stream.
    // This one we do want to dispose after using, because it will be in the scope of the engine itself
    return new Common::MemoryReadStream(dstW.getData(), dstW.size(), DisposeAfterUse::YES);
}

void setupDictionary(uint16 start[], uint16 ptk[], uint16 &findEmpty) {
    // Clear the whole dictionary
    for (int i = 0x3FFF; i >= 0; i--) {
        start[i] = 0;
          ptk[i] = 0;
    }

    // Set the initial 256 bytes to be value 256, these are the characters without extensions
    for (int i = 0x0FF; i >= 0; i--) {
          ptk[i] = 0x100;
    }

    // This shouldn't really be done inside the function, but for the sake of consistency with the source, we will
    findEmpty = 0x8000;
}

int getInputCode(bool &carry, Common::File *src, int &srcLen, uint16 &evenOdd) {
    // Check if we're at the end of the file
    if (srcLen == 0) {
        carry = false;
        return 0;
    }

    uint16 c;
    if (evenOdd != 0) {                             // Odd
        srcLen--;
        evenOdd--;
        c = (src->readUint16BE() >> 3) & 0x00FE;    // & #-1-1
    } else {                                        // Even
        srcLen -= 2;
        evenOdd++;
        c = (src->readUint16BE() & kMask12Bit) << 1;
        src->seek(-1, SEEK_CUR);
    }
    return c;
}

uint16 getMember(uint16 codeW, uint16 k, uint16 &findEmpty, uint16 start[], uint16 ptk[]) {
    // This function is effectively void, as the return value is only used in compression
    
    // k and codeW are local variables with the value of oldCode and finalChar

    uint16 hash;
    hash = (k << 3) ^ k;
    hash = (hash << 1) ^ codeW;
    hash <<= 1;

    hash = (hash >= 0x200) ? hash : hash + 0x200;

    uint16 a = start[hash] & 0x0F00;
    uint16 b = ptk[hash] & kMaskHigh;
    if (a | b) {
        start[hash] = codeW;
          ptk[hash] = k | 0x100;
        return ptk[hash];
    }

    // This loop is a bit wacky, due to the way the jumps were stuctured in the source
    bool ag = true;
    uint16 tmp;
    while (ag == true) {
        if ((start[hash] & kMask12Bit) == codeW) {
            if ((ptk[hash] & kMaskLow) == k) {
                return hash >> 1;
            }
        }

        tmp = start[hash] & kMaskLast;
        if (tmp == 0) {

            // I've separated this into it's own function for the sake of this loop being readable
            appendList(codeW, k, hash, findEmpty, start, ptk, tmp);
            ag = false;

        } else {
            tmp >>= 4;
            hash = ptk[hash] & kMaskHigh;       // The 65816 can XBA to flip the bytes in a word, instead we have mask and shift over :(
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
    
    } else {
        bool found = false;
        while (found == false) {
            hash -= 2;
            if (hash >= 0x200) {
                setupDictionary(start, ptk, findEmpty);
                found = true;
            }

            // Split up the conditional statement to be easier to follow
            uint16 cond;
            cond = start[hash] & kMaskLast;
            cond |= ptk[hash];

            if ((cond & kMaskHigh) == 0) {
                  findEmpty = hash;
                start[hash] = codeW;
                  ptk[hash] = k | 0x100;

                link = hash >> 1;
                 tmp = link & kMaskLow;         // Another classic XBA situation, although it's nicer this time
                 tmp <<= 8;                     // Because we can just grab the low bytes and shift them forward (it's still much faster to XBA though)

                  ptk[prev] = (ptk[prev] & kMaskLow) | tmp;
                start[prev] = ((link >> 4) & kMaskLast) | start[prev];      // Yikes this statement is gross
                found = true;
            }
        }
    }
}

} // namespace compression


























