/***********************************************************
	io.c -- input/output
***********************************************************/
#include "c:\work\instaluj.cpp\ar.h"
#include <cfile.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dos.h>
#include <io.h>

//#define CRCPOLY  0xA001  /* ANSI CRC-16 */
//                         /* CCITT: 0x8408 */
//#define UPDATE_CRC(c) \
//	crc = crctable[(crc ^ (c)) & 0xFF] ^ (crc >> CHAR_BIT)

//FHAN arcfile, infile, outfile;
uint /*crc,*/ bitbuf;

//static ushort crctable[UCHAR_MAX + 1];
static uint  subbitbuf;
static int   bitcount;



void fillbuf(int n)  /* Shift bitbuf n bits left, read n bits */
{
  extern CFILE *file;
	bitbuf <<= n;
	while (n > bitcount)
	    {
		bitbuf |= subbitbuf << (n -= bitcount);
		if (compsize != 0)
		    {
			compsize--;
			subbitbuf = file->Read();
		    }
		else subbitbuf = 0;
		bitcount = CHAR_BIT;
	    }
	bitbuf |= subbitbuf >> (bitcount -= n);
}

uint getbits(int n)
{
	uint x;

	x = bitbuf >> (BITBUFSIZ - n);  fillbuf(n);
	return x;
}


void init_getbits(void)
{
	bitbuf = 0;  subbitbuf = 0;  bitcount = 0;
	fillbuf(BITBUFSIZ);
}

