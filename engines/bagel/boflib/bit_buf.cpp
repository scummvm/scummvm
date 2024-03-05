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

#include "bagel/boflib/bit_buf.h"
#include "bagel/bagel.h"

namespace Bagel {

/*
 * table of bit masks 0-15 bits
 */
USHORT bitMask[] = {
	0x0000,
	0x0001,
	0x0003,
	0x0007,
	0x000F,
	0x001F,
	0x003F,
	0x007F,
	0x00FF,
	0x01FF,
	0x03FF,
	0x07FF,
	0x0FFF,
	0x1FFF,
	0x3FFF,
	0x7FFF,
	0xFFFF,
};

VOID BitReadInit(UBYTE *pInBuf, INT nBufSize) {
	zg.bitVar.fileSize = nBufSize;

	zg.bitVar.bufPtr = pInBuf;

#if BOF_WINMAC || BOF_MAC
	zg.bitVar.last = SWAPWORD(*(USHORT *)pInBuf);
#else
	zg.bitVar.last = *(USHORT *)pInBuf;
#endif

	zg.bitVar.bitCnt = 0;
}

/**
 *
 * name      BitRead - Read 'num' bits into 'buf'
 *
 * synopsis  errCode = BitRead(bitPtr, bitCnt)
 *           USHORT *bitPtr;             buffer that will contain the read in bits
 *           SHORT bitCnt;               how many bits to read
 *
 * purpose   To Allow the reading of a certain number of bits
 *
 *
 * returns   errCode = Error return code
 *
 **/
ERROR_CODE BitRead(USHORT *bitPtr, SHORT bitCnt) {
	SHORT numBits;
	UBYTE bits;

	for (;;) {
		/*
		 * do up to USHRT_BITS bits at a time
		 */
		if ((numBits = bitCnt) > USHRT_BITS)
			numBits = USHRT_BITS;

		bits = zg.bitVar.bitCnt;
		if ((zg.bitVar.bitCnt += (UBYTE)numBits) <= USHRT_BITS) {
			*bitPtr = zg.bitVar.last & bitMask[numBits];
			zg.bitVar.last >>= numBits;

		} else {
			zg.bitVar.bitCnt -= USHRT_BITS;
			zg.bitVar.bufPtr += sizeof(USHORT);

#if BOF_WINMAC || BOF_MAC
			*bitPtr = ((SWAPWORD(*(USHORT *)zg.bitVar.bufPtr) << (UBYTE)(USHRT_BITS - bits)) | zg.bitVar.last) & bitMask[numBits];
#else
			*bitPtr = ((*(USHORT *)zg.bitVar.bufPtr << (UBYTE)(USHRT_BITS - bits)) | zg.bitVar.last) & bitMask[numBits];
#endif

			/*
			 * next word in buffer
			 */
#if BOF_WINMAC || BOF_MAC
			zg.bitVar.last = SWAPWORD(*(USHORT *)zg.bitVar.bufPtr) >> zg.bitVar.bitCnt;
#else
			zg.bitVar.last = *(USHORT *)zg.bitVar.bufPtr >> zg.bitVar.bitCnt;
#endif
		}

		/*
		 * check for more source bits
		 */
		if (!(bitCnt -= numBits))
			break;

		/* next word in source */
		bitPtr++;
	}

	return (ERR_NONE);
}

/**
 *
 * name      BitReadBytes - to read bytes from input file after a byte aligning
 *
 * synopsis  errCode = BitReadBytes(bufer, size)
 *           UBYTE *buffer               buffer to put input bytes
 *           SHORT size                  number of bytes to read
 *
 * purpose   To read 'size' bytes into 'buffer' from the input file
 *
 *
 * returns   errCode - Error Return Code
 *
 **/
ERROR_CODE BitReadBytes(UBYTE *buffer, SHORT size, UBYTE *pInBuf, INT /*nBufSize*/) {
	/*
	 * byte align buffer pointer
	 */
	if (zg.bitVar.bitCnt)
		zg.bitVar.bufPtr++;
	if (zg.bitVar.bitCnt > BITS_IN_CHAR)
		zg.bitVar.bufPtr++;

	zg.bitVar.bitCnt = 0;

	for (;;) {

		/*
		 * if done
		 */
		if (!size) {

			/*
			 * setup for bit operations again
			 */
#if BOF_WINMAC || BOF_MAC
			zg.bitVar.last = SWAPWORD(*(USHORT *)zg.bitVar.bufPtr);
#else
			zg.bitVar.last = *(USHORT *)zg.bitVar.bufPtr;
#endif
			if ((zg.bitVar.bufPtr - pInBuf) & 1) {
				zg.bitVar.last = *zg.bitVar.bufPtr--;
				zg.bitVar.bitCnt = BITS_IN_CHAR;
			}
			break;
		}

		/* copy byte */
		*buffer++ = *zg.bitVar.bufPtr++;
		size--;
	}

	return (ERR_NONE);
}

/**
 *
 * name	    BitReadQuick - Read 'codeSize' bits into 'bitPtr'
 *
 * synopsis  errCode = BitReadQuick(bitPtr, codeSize)
 *	    USHORT *bitPtr;
 *	    UBYTE codeSize;
 *
 * purpose   To Allow the reading of a certain number of bits
 *
 *
 * returns   errCode = Error return code
 *
 **/
ERROR_CODE BitReadQuick(USHORT *bitPtr, UBYTE codeSize) {
	UBYTE bits;

	bits = zg.bitVar.bitCnt;
	if ((zg.bitVar.bitCnt += codeSize) <= USHRT_BITS) {
		*bitPtr = zg.bitVar.last & bitMask[codeSize];
		zg.bitVar.last >>= codeSize;

	} else {
		zg.bitVar.bitCnt -= USHRT_BITS;
		zg.bitVar.bufPtr += sizeof(USHORT);

#if BOF_WINMAC || BOF_MAC
		*bitPtr = ((SWAPWORD(*(USHORT *)zg.bitVar.bufPtr) << (UBYTE)(USHRT_BITS - bits)) | zg.bitVar.last) & bitMask[codeSize];
#else
		*bitPtr = ((*(USHORT *)zg.bitVar.bufPtr << (UBYTE)(USHRT_BITS - bits)) | zg.bitVar.last) & bitMask[codeSize];
#endif

#if BOF_WINMAC || BOF_MAC
		zg.bitVar.last = SWAPWORD(*(USHORT *)zg.bitVar.bufPtr) >> zg.bitVar.bitCnt;
#else
		zg.bitVar.last = *(USHORT *)zg.bitVar.bufPtr >> zg.bitVar.bitCnt;
#endif
	}

	return (ERR_NONE);
}

ERROR_CODE BitReadQuick1(USHORT *bitPtr) {
	if (++zg.bitVar.bitCnt <= USHRT_BITS) {
		*bitPtr = zg.bitVar.last & 1;
		zg.bitVar.last >>= 1;

	} else {
		zg.bitVar.bitCnt -= USHRT_BITS;
		zg.bitVar.bufPtr += sizeof(USHORT);

#if BOF_WINMAC || BOF_MAC
		*bitPtr = SWAPWORD(*(USHORT *)zg.bitVar.bufPtr) & 1;
#else
		*bitPtr = *(USHORT *)zg.bitVar.bufPtr & 1;
#endif

#if BOF_WINMAC || BOF_MAC
		zg.bitVar.last = SWAPWORD(*(USHORT *)zg.bitVar.bufPtr) >> 1;
#else
		zg.bitVar.last = *(USHORT *)zg.bitVar.bufPtr >> 1;
#endif
	}

	return (ERR_NONE);
}

VOID BufReadInit(UBYTE *pInBuf, INT nBufSize) {
	zg.bufVar.bufEnd = (zg.bufVar.bufPtr = pInBuf) + nBufSize - sizeof(USHORT);
}

/**
 *
 * name      BufRead - Perform buffered input
 *
 * synopsis  errCode = BufRead(buffer, size, rSize)
 *           UBYTE *buffer;
 *           SHORT size;
 *           SHORT *rSize;
 *
 * purpose   To buffer disk reads using a file handle
 *
 *
 * returns   errCode = Error return code
 *
 **/
ERROR_CODE BufRead(UBYTE *buffer, SHORT size, SHORT *rSize) {
	SHORT saveSize;

	/* save read amount */
	saveSize = size;

	do {
		*buffer++ = *zg.bufVar.bufPtr++;
	} while (--size);

	/*
	 * amount read
	 */
	if (rSize != NULL)
		*rSize = saveSize - size;

	return (ERR_NONE);
}

/**
 *
 * name	    BufReadQuick - Perform buffered input
 *
 * synopsis  errCode = BufReadQuick(data)
 *	    UBYTE *data;
 *
 * purpose   To buffer disk reads using a file handle
 *
 *
 * returns   errCode = Error return code
 *
 **/
ERROR_CODE BufReadQuick(UBYTE *data) {
	*data = *zg.bufVar.bufPtr++;

	return (ERR_NONE);
}

ERROR_CODE BufReadStrQuick(UBYTE *data, SHORT len, SHORT *rLen) {
	ERROR_CODE errCode;

	/* assume no errors */
	errCode = ERR_NONE;

	if (zg.bufVar.bufPtr >= zg.bufVar.bufEnd - len)
		errCode = BufRead(data, len, rLen);
	else {
		memcpy(data, zg.bufVar.bufPtr, len);
		zg.bufVar.bufPtr += len;
		if (rLen != NULL)
			*rLen = len;
	}

	return (errCode);
}

/************************ Bit Writing routines ***************************/
/************************ Bit Writing routines ***************************/
/************************ Bit Writing routines ***************************/

/**
 *
 * name	    BitWriteQuick - Write 'codeSize' bits from 'bitPtr'
 *
 * synopsis  errCode = BitWriteQuick(bitPtr, codeSize)
 *	    USHORT *bitPtr;
 *	    UBYTE codeSize;
 *
 * purpose   To Allow the Writing of a certain number of bits to a file
 *
 *
 * returns   errCode = Error return code
 *
 **/
ERROR_CODE BitWriteQuick(USHORT *bitPtr, UBYTE codeSize) {
	ERROR_CODE errCode;
	UBYTE bits;

	/* assume no errors */
	errCode = ERR_NONE;

	*(USHORT *)zg.bitVar.bufPtr |= *bitPtr << (bits = zg.bitVar.bitCnt);

	if ((zg.bitVar.bitCnt += codeSize) >= USHRT_BITS) {

		zg.bitVar.bitCnt -= USHRT_BITS;
		zg.bitVar.bufPtr += sizeof(USHORT);

		*(USHORT *)zg.bitVar.bufPtr |= (*bitPtr >> (UBYTE)(USHRT_BITS - bits));

		/*
		 * next word in buffer
		 */
		if (zg.bitVar.bufPtr >= zg.bitVar.bufEnd)
			errCode = BitAltFlush();
	}

	return (errCode);
}

/**
 *
 * name      BitWriteBytes - to write bytes to out file after a byte aligning
 *
 * synopsis  errCode = BitWriteBytes(bufer, size)
 *           UBYTE *buffer               buffer of bytes to output
 *           SHORT size                  number of bytes to write
 *
 * purpose   To write 'size' bytes from 'buffer' to a the output file
 *
 *
 * returns   errCode - Error Return Code
 *
 **/
ERROR_CODE BitWriteBytes(UBYTE *buffer, SHORT size) {
	ERROR_CODE errCode;

	/* assume no errors */
	errCode = ERR_NONE;

	/*
	 * byte align buffer pointer by moving past extra bits
	 */
	if (zg.bitVar.bitCnt)
		zg.bitVar.bufPtr++;
	if (zg.bitVar.bitCnt > BITS_IN_CHAR)
		zg.bitVar.bufPtr++;

	/* we are now byte aligned */
	zg.bitVar.bitCnt = 0;

	for (;;) {
		/*
		 * Flush buffer if necessary
		 */
		if (zg.bitVar.bufPtr >= zg.bitVar.bufEnd && ((errCode = BitAltFlush()) != ERR_NONE))
			break;

		/*
		 * if done
		 */
		if (!size) {
			/*
			 * setup for bit operations again
			 */
			if ((zg.bitVar.bufPtr - zg.u.s.outBuffer) & 1) {
				zg.bitVar.bufPtr--;
				zg.bitVar.bitCnt = BITS_IN_CHAR;
			}
			break;
		}

		/* copy byte */
		*zg.bitVar.bufPtr++ = *buffer++;
		size--;
	}

	return (errCode);
}

/**
 *
 * name      BitWriteInit - Initializes BitWrite Variables
 *
 * synopsis  errCode = BitWriteInit()
 *
 * purpose   To prepare to use BitWrite
 *
 *
 **/
VOID BitWriteInit() {
	zg.bitVar.bufEnd = (zg.bitVar.bufPtr = zg.u.s.outBuffer) + (MAXFILEBUF - sizeof(USHORT));
	zg.bitVar.fileSize = 0;
	zg.bitVar.bitCnt = 0;

	memset(zg.bitVar.bufPtr, 0, MAXFILEBUF);
}

/**
 *
 * name      BitWriteSize - returns size in bytes of bit written
 *
 * synopsis  size = BitWriteSize()
 *
 * purpose   Compute size in bytes of total bits written
 *
 *
 **/
LONG BitWriteSize() {
	LONG fileSize;

	fileSize = (zg.bitVar.bufPtr - zg.u.s.outBuffer) + zg.bitVar.fileSize;
	if (zg.bitVar.bitCnt)
		fileSize++;
	if (zg.bitVar.bitCnt > BITS_IN_CHAR)
		fileSize++;

	return (fileSize);
}

/**
 *
 * name      BitWrite - Write 'bitCnt' bits from 'bitPtr'
 *
 * synopsis  errCode = BitWrite(bitPtr, bitCnt)
 *           USHORT *bitPtr;             buffer that contains the bits to write
 *           SHORT bitCnt;               number of bits to be written
 *
 * purpose   To Write a certain number of bits to a file
 *
 *
 * returns   errCode = Error return code
 *
 **/
ERROR_CODE BitWrite(USHORT *bitPtr, SHORT bitCnt) {
	ERROR_CODE errCode;
	SHORT numBits;
	UBYTE bits;

	/* assume no errors */
	errCode = ERR_NONE;

	for (;;) {
		/*
		 * do up to USHRT_BITS bits at a time
		 */
		if ((numBits = bitCnt) > USHRT_BITS)
			numBits = USHRT_BITS;

		*(USHORT *)zg.bitVar.bufPtr |= *bitPtr << (bits = zg.bitVar.bitCnt);

		if ((zg.bitVar.bitCnt += (UBYTE)numBits) >= USHRT_BITS) {

			zg.bitVar.bitCnt -= USHRT_BITS;
			zg.bitVar.bufPtr += sizeof(USHORT);

			*(USHORT *)zg.bitVar.bufPtr |= (*bitPtr >> (UBYTE)(USHRT_BITS - bits));

			/*
			 * next word in buffer
			 */
			if (zg.bitVar.bufPtr >= zg.bitVar.bufEnd && ((errCode = BitAltFlush()) != ERR_NONE))
				break;
		}

		/*
		 * check for more source bits
		 */
		if (!(bitCnt -= numBits))
			break;

		/* next word in source */
		bitPtr++;
	}

	return (errCode);
}

/**
 *
 * name      BitAltFlush - Flush any remaining bits
 *
 * synopsis  errCode = BitAltFlush()
 *
 * purpose   To write out last bits/bytes to outFile
 *
 *
 * returns   errCode = Error return code
 *
 **/
ERROR_CODE BitAltFlush() {
	SHORT used;
	ERROR_CODE errCode;

	errCode = ERR_NONE;

	/* compute used */
	used = zg.bitVar.bufPtr - zg.u.s.outBuffer;

	/*
	 * if no error writing
	 */
	if (zg.zipFile->Write(zg.u.s.outBuffer, used) == ERR_NONE) {

		/* shift-down last word */
		*(USHORT *)zg.u.s.outBuffer = *(USHORT *)zg.bitVar.bufPtr;

		/* zero remaining */
		memset((zg.bitVar.bufPtr = zg.u.s.outBuffer) + sizeof(USHORT), 0, (MAXFILEBUF - sizeof(USHORT)));
		zg.bitVar.fileSize += used;
	} else {
		errCode = ERR_FWRITE;
	}

	return (errCode);
}

/**
 *
 * name      BitWriteFlush - Flush any remaining bits
 *
 * synopsis  errCode = BitWriteFlush()
 *
 * purpose   To write out last bits/bytes to outFile
 *
 *
 * returns   errCode = Error return code
 *
 **/

ERROR_CODE BitWriteFlush(LONG *rFileSize) {
	SHORT used;
	ERROR_CODE errCode;

	errCode = ERR_NONE;

	/*
	 * determine size
	 */
	used = zg.bitVar.bufPtr - zg.u.s.outBuffer;
	if (zg.bitVar.bitCnt)
		used++;
	if (zg.bitVar.bitCnt > BITS_IN_CHAR)
		used++;

	/*
	 * if no errors writing to zip file
	 */
	if (zg.zipFile->Write(zg.u.s.outBuffer, used) == ERR_NONE) {

		/*
		 * update file size
		 */
		zg.bitVar.fileSize += used;
		if (rFileSize != NULL)
			*rFileSize = zg.bitVar.fileSize;
	} else {
		errCode = ERR_FWRITE;
	}

	return (errCode);
}

#if 0
/**
*
* name      BitWriteInit - Initializes BitWrite Variables
*
* synopsis  errCode = BitWriteInit()
*
* purpose   To prepare to use BitWrite
*
*
**/
VOID BitWriteInit(UBYTE *pOutBuf, INT nBufSize)
{
    zg.bitVar.bufEnd = (zg.bitVar.bufPtr = pOutBuf) + (nBufSize - sizeof(USHORT));
    zg.bitVar.fileSize = 0;
    zg.bitVar.bitCnt = 0;

    memset(zg.bitVar.bufPtr, 0, nBufSize);
}


/**
*
* name      BitWriteSize - returns size in bytes of bit written
*
* synopsis  size = BitWriteSize()
*
* purpose   Compute size in bytes of total bits written
*
*
**/
LONG BitWriteSize(UBYTE *pOutBuf, INT nBufSize)
{
    LONG fileSize;

    fileSize = (zg.bitVar.bufPtr - pOutBuf) + zg.bitVar.fileSize;
    if (zg.bitVar.bitCnt)
        fileSize++;
    if (zg.bitVar.bitCnt > BITS_IN_CHAR)
        fileSize++;

    return(fileSize);
}


/**
*
* name      BitWrite - Write 'bitCnt' bits from 'bitPtr'
*
* synopsis  errCode = BitWrite(bitPtr, bitCnt)
*           USHORT *bitPtr;             buffer that contains the bits to write
*           SHORT bitCnt;               number of bits to be written
*
* purpose   To Write a certain number of bits to a file
*
*
* returns   errCode = Error return code
*
**/
ERROR_CODE BitWrite(USHORT *bitPtr, SHORT bitCnt)
{
    ERROR_CODE errCode;
    SHORT numBits;
    UBYTE bits;

    /* assume no errors */
    errCode = ERR_NONE;

    for (;;) {
        /*
        * do up to USHRT_BITS bits at a time
        */
        if ((numBits = bitCnt) > USHRT_BITS)
            numBits = USHRT_BITS;

        *(USHORT *)zg.bitVar.bufPtr |= *bitPtr << (bits = zg.bitVar.bitCnt);

        if ((zg.bitVar.bitCnt += (UBYTE)numBits) >= USHRT_BITS) {

            zg.bitVar.bitCnt -= USHRT_BITS;
            zg.bitVar.bufPtr += sizeof(USHORT);

            *(USHORT *)zg.bitVar.bufPtr |= (*bitPtr >> (UBYTE)(USHRT_BITS - bits));
        }

        /*
        * check for more source bits
        */
        if (!(bitCnt -= numBits))
            break;

        /* next word in source */
        bitPtr++;
    }

    return (errCode);
}

/**
*
* name      BitWriteBytes - to write bytes to out file after a byte aligning
*
* synopsis  errCode = BitWriteBytes(bufer, size)
*           UBYTE *buffer               buffer of bytes to output
*           SHORT size                  number of bytes to write
*
* purpose   To write 'size' bytes from 'buffer' to a the output file
*
*
* returns   errCode - Error Return Code
*
**/
ERROR_CODE BitWriteBytes(UBYTE *buffer, SHORT size, UBYTE *pOutBuf, INT nBufSize)
{
    /*
    * byte align buffer pointer by moving past extra bits
    */
    if (zg.bitVar.bitCnt)
        zg.bitVar.bufPtr++;
    if (zg.bitVar.bitCnt > BITS_IN_CHAR)
        zg.bitVar.bufPtr++;

    /* we are now byte aligned */
    zg.bitVar.bitCnt = 0;

    for (;;) {

        /*
        * if done
        */
        if (!size) {
            /*
            * setup for bit operations again
            */
            if ((zg.bitVar.bufPtr - pOutBuf) & 1) {
                zg.bitVar.bufPtr--;
                zg.bitVar.bitCnt = BITS_IN_CHAR;
            }
            break;
        }

        /* copy byte */
        *zg.bitVar.bufPtr++ = *buffer++;
        size--;
    }

    return(ERR_NONE);
}

/**
*
* name	    BitWriteQuick - Write 'codeSize' bits from 'bitPtr'
*
* synopsis  errCode = BitWriteQuick(bitPtr, codeSize)
*	    USHORT *bitPtr;
*	    UBYTE codeSize;
*
* purpose   To Allow the Writing of a certain number of bits to a file
*
*
* returns   errCode = Error return code
*
**/
ERROR_CODE BitWriteQuick(USHORT *bitPtr, UBYTE codeSize)
{
    UBYTE bits;

    *(USHORT *)zg.bitVar.bufPtr |= *bitPtr << (bits = zg.bitVar.bitCnt);

    if ((zg.bitVar.bitCnt += codeSize) >= USHRT_BITS) {

    	zg.bitVar.bitCnt -= USHRT_BITS;
    	zg.bitVar.bufPtr += sizeof(USHORT);

    	*(USHORT *)zg.bitVar.bufPtr |= (*bitPtr >> (UBYTE)(USHRT_BITS - bits));
    }

    return(ERR_NONE);
}
#endif


} // namespace Bagel
