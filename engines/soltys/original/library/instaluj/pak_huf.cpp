/***********************************************************
	huf.c -- static Huffman
***********************************************************/
#include <stdlib.h>
#include <mem.h>
#include "c:\work\instaluj.cpp\ar.h"

#define NP (DICBIT + 1)
#define NT (CODE_BIT + 3)
#define PBIT 4  /* smallest integer such that (1U << PBIT) > NP */
#define TBIT 5  /* smallest integer such that (1U << TBIT) > NT */
#if NT > NP
	#define NPT NT
#else
	#define NPT NP
#endif

ushort left[2 * NC - 1], right[2 * NC - 1];
static uchar c_len[NC], pt_len[NPT];
static uint   blocksize;
static ushort c_table[4096], pt_table[256];


/***** decoding *****/

static void read_pt_len(int nn, int nbit, int i_special)
{
	int i, c, n;
	uint mask;

	n = getbits(nbit);
	if (n == 0) {
		c = getbits(nbit);
		memset(pt_len, '\0', nn*sizeof(*pt_len));
		memset(pt_table, c, 256*sizeof(*pt_table));
	} else {
		i = 0;
		while (i < n) {
			c = bitbuf >> (BITBUFSIZ - 3);
			if (c == 7) {
				mask = 1U << (BITBUFSIZ - 1 - 3);
				while (mask & bitbuf) {  mask >>= 1;  c++;  }
			}
			fillbuf((c < 7) ? 3 : c - 3);
			pt_len[i++] = c;
			if (i == i_special) {
				c = getbits(2);
				while (--c >= 0) pt_len[i++] = 0;
			}
		}
		while (i < nn) pt_len[i++] = 0;
		make_table(nn, pt_len, 8, pt_table);
	}
}

static void read_c_len(void)
{
	int i, c, n;
	uint mask;

	n = getbits(CBIT);
	if (n == 0) {
		c = getbits(CBIT);
		memset(c_len, '\0', NC*sizeof(*c_len));
		memset(c_table, c, 4096*sizeof(*c_table));
	} else {
		i = 0;
		while (i < n) {
			c = pt_table[bitbuf >> (BITBUFSIZ - 8)];
			if (c >= NT) {
				mask = 1U << (BITBUFSIZ - 1 - 8);
				do {
					if (bitbuf & mask) c = right[c];
					else               c = left [c];
					mask >>= 1;
				} while (c >= NT);
			}
			fillbuf(pt_len[c]);
			if (c <= 2) {
				if      (c == 0) c = 1;
				else if (c == 1) c = getbits(4) + 3;
				else             c = getbits(CBIT) + 20;
				while (--c >= 0) c_len[i++] = 0;
			} else c_len[i++] = c - 2;
		}
		while (i < NC) c_len[i++] = 0;
		make_table(NC, c_len, 12, c_table);
	}
}

uint decode_c(void)
{
	uint j, mask;

	if (blocksize == 0) {
		blocksize = getbits(16);
		read_pt_len(NT, TBIT, 3);
		read_c_len();
		read_pt_len(NP, PBIT, -1);
	}
	blocksize--;
	j = c_table[bitbuf >> (BITBUFSIZ - 12)];
	if (j >= NC) {
		mask = 1U << (BITBUFSIZ - 1 - 12);
		do {
			if (bitbuf & mask) j = right[j];
			else               j = left [j];
			mask >>= 1;
		} while (j >= NC);
	}
	fillbuf(c_len[j]);
	return j;
}

uint decode_p(void)
{
	uint j, mask;

	j = pt_table[bitbuf >> (BITBUFSIZ - 8)];
	if (j >= NP) {
		mask = 1U << (BITBUFSIZ - 1 - 8);
		do {
			if (bitbuf & mask) j = right[j];
			else               j = left [j];
			mask >>= 1;
		} while (j >= NP);
	}
	fillbuf(pt_len[j]);
	if (j != 0) j = (1U << (j - 1)) + getbits(j - 1);
	return j;
}

void huf_decode_start(void)
{
	init_getbits();  blocksize = 0;
}
