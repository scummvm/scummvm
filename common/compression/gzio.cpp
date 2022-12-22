/* gzio.c - decompression support for gzip */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2005,2006,2007,2009  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Most of this file was originally the source file "inflate.c", written
 * by Mark Adler.  It has been very heavily modified.  In particular, the
 * original would run through the whole file at once, and this version can
 * be stopped and restarted on any boundary during the decompression process.
 *
 * The license and header comments that file are included here.
 */

/* inflate.c -- Not copyrighted 1992 by Mark Adler
   version c10p1, 10 January 1993 */

/* You can do whatever you like with this source file, though I would
   prefer that if you modify it and redistribute it that you include
   comments to that effect with your name and the date.  Thank you.
 */

#include "common/endian.h"
#include "common/stream.h"
#include "common/ptr.h"
#include "common/memstream.h"
#include "common/compression/gzio.h"


/* Compression methods (see algorithm.doc) */
#define GRUB_GZ_STORED      0
#define GRUB_GZ_COMPRESSED  1
#define GRUB_GZ_PACKED      2
#define GRUB_GZ_LZHED       3
/* methods 4 to 7 reserved */
#define GRUB_GZ_DEFLATED    8
#define GRUB_GZ_MAX_METHODS 9

/* gzip flag byte */
#define GRUB_GZ_ASCII_FLAG   0x01	/* bit 0 set: file probably ascii text */
#define GRUB_GZ_CONTINUATION 0x02	/* bit 1 set: continuation of multi-part gzip file */
#define GRUB_GZ_EXTRA_FIELD  0x04	/* bit 2 set: extra field present */
#define GRUB_GZ_ORIG_NAME    0x08	/* bit 3 set: original file name present */
#define GRUB_GZ_COMMENT      0x10	/* bit 4 set: file comment present */
#define GRUB_GZ_ENCRYPTED    0x20	/* bit 5 set: file is encrypted */
#define GRUB_GZ_RESERVED     0xC0	/* bit 6,7:   reserved */

#define GRUB_GZ_UNSUPPORTED_FLAGS	(GRUB_GZ_CONTINUATION | GRUB_GZ_ENCRYPTED | GRUB_GZ_RESERVED)

/* inflate block codes */
#define INFLATE_STORED	0
#define INFLATE_FIXED	1
#define INFLATE_DYNAMIC	2

namespace Common {

typedef unsigned char uch;
typedef unsigned short ush;
typedef unsigned long ulg;



/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model).
   Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
   means that v is a literal, 16 < e < 32 means that v is a pointer to
   the next table, which codes e - 16 bits, and lastly e == 99 indicates
   an unused code.  If a code with e == 99 is looked up, this implies an
   error in the data. */
struct huft
{
  uch e;			/* number of extra bits or operation */
  uch b;			/* number of bits in this code or subcode */
  union
    {
      ush n;			/* literal, length base, or distance base */
      struct huft *t;		/* pointer to next level of table */
    }
  v;
};


/* The inflate algorithm uses a sliding 32K byte window on the uncompressed
   stream to find repeated byte strings.  This is implemented here as a
   circular buffer.  The index is updated simply by incrementing and then
   and'ing with 0x7fff (32K-1). */
/* It is left to other modules to supply the 32K area.  It is assumed
   to be usable as if it were declared "uch slide[32768];" or as just
   "uch *slide;" and then malloc'ed in the latter case.  The definition
   must be in unzip.h, included above. */


/* Tables for deflate from PKZIP's appnote.txt. */
static unsigned bitorder_zlib[] =
{				/* Order of the bit length code lengths */
  16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

static unsigned bitorder_clickteam[] =
{				/* Order of the bit length code lengths */
  18, 17, 16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static ush cplens[] =
{				/* Copy lengths for literal codes 257..285 */
  3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
  35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
	/* note: see note #13 above about the 258 in this list. */
static ush cplext[] =
{				/* Extra bits for literal codes 257..285 */
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
  3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99};	/* 99==invalid */
static ush cpdist[] =
{				/* Copy offsets for distance codes 0..29 */
  1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
  257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
  8193, 12289, 16385, 24577};
static ush cpdext[] =
{				/* Extra bits for distance codes */
  0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
  7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
  12, 12, 13, 13};


/*
   Huffman code decoding is performed using a multi-level table lookup.
   The fastest way to decode is to simply build a lookup table whose
   size is determined by the longest code.  However, the time it takes
   to build this table can also be a factor if the data being decoded
   is not very long.  The most common codes are necessarily the
   shortest codes, so those codes dominate the decoding time, and hence
   the speed.  The idea is you can have a shorter table that decodes the
   shorter, more probable codes, and then point to subsidiary tables for
   the longer codes.  The time it costs to decode the longer codes is
   then traded against the time it takes to make longer tables.

   This results of this trade are in the variables lbits and dbits
   below.  lbits is the number of bits the first level table for literal/
   length codes can decode in one step, and dbits is the same thing for
   the distance codes.  Subsequent tables are also less than or equal to
   those sizes.  These values may be adjusted either when all of the
   codes are shorter than that, in which case the longest code length in
   bits is used, or when the shortest code is *longer* than the requested
   table size, in which case the length of the shortest code in bits is
   used.

   There are two different values for the two tables, since they code a
   different number of possibilities each.  The literal/length table
   codes 286 possible values, or in a flat code, a little over eight
   bits.  The distance table codes 30 possible values, or a little less
   than five bits, flat.  The optimum values for speed end up being
   about one bit more than those, so lbits is 8+1 and dbits is 5+1.
   The optimum values may differ though from machine to machine, and
   possibly even between compilers.  Your mileage may vary.
 */


static int lbits = 9;		/* bits in base literal/length lookup table */
static int dbits = 6;		/* bits in base distance lookup table */


/* If BMAX needs to be larger than 16, then h and x[] should be ulg. */
#define BMAX 16			/* maximum bit length of any code (16 for explode) */
#define N_MAX 288		/* maximum number of codes in any set */


/* Macros for inflate() bit peeking and grabbing.
   The usage is:

        NEEDBITS(j)
        x = b & mask_bits[j];
        DUMPBITS(j)

   where NEEDBITS makes sure that b has at least j bits in it, and
   DUMPBITS removes the bits from b.  The macros use the variable k
   for the number of bits in b.  Normally, b and k are register
   variables for speed, and are initialized at the beginning of a
   routine that uses these macros from a global bit buffer and count.

   If we assume that EOB will be the longest code, then we will never
   ask for bits with NEEDBITS that are beyond the end of the stream.
   So, NEEDBITS should not read any more bytes than are needed to
   meet the request.  Then no bytes need to be "returned" to the buffer
   at the end of the last block.

   However, this assumption is not true for fixed blocks--the EOB code
   is 7 bits, but the other literal/length codes can be 8 or 9 bits.
   (The EOB code is shorter than other codes because fixed blocks are
   generally short.  So, while a block always has an EOB, many other
   literal/length codes have a significantly lower probability of
   showing up at all.)  However, by making the first table have a
   lookup of seven bits, the EOB code will be found in that first
   lookup, and so will not require that too many bits be pulled from
   the stream.
 */

static ush mask_bits[] =
{
  0x0000,
  0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
  0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};


#define NEEDBITS(n) do {while(k<(n)){b|=((ulg)parentGetByte())<<k;k+=8;}} while (0)
#define DUMPBITS(n) do {b>>=(n);k-=(n);} while (0)

byte
GzioReadStream::parentGetByte ()
{
  if (_inbufD >= _inbufSize)
    {
      _inbufD = 0;
      _inbufSize = _input->read(_inbuf, INBUFSIZ);
      if (_inbufSize <= 0) {
	_inbufSize = 0;
	return 0;
      }
    }

  return _inbuf[_inbufD++];
}

void
GzioReadStream::parentSeek(int64 off)
{
  _inbufSize = 0;
  _inbufD = 0;
  _input->seek(off);
}

/* more function prototypes */
static int huft_build (unsigned *, unsigned, unsigned, ush *, ush *,
		       struct huft **, int *);
static int huft_free (struct huft *);


/* Given a list of code lengths and a maximum table size, make a set of
   tables to decode that set of codes.  Return zero on success, one if
   the given code set is incomplete (the tables are still built in this
   case), two if the input is invalid (all zero length codes or an
   oversubscribed set of lengths), and three if not enough memory. */

static int
huft_build (unsigned *b,	/* code lengths in bits (all assumed <= BMAX) */
	    unsigned n,		/* number of codes (assumed <= N_MAX) */
	    unsigned s,		/* number of simple-valued codes (0..s-1) */
	    ush * d,		/* list of base values for non-simple codes */
	    ush * e,		/* list of extra bits for non-simple codes */
	    struct huft **t,	/* result: starting table */
	    int *m)		/* maximum lookup bits, returns actual */
{
  unsigned a;			/* counter for codes of length k */
  unsigned c[BMAX + 1];		/* bit length count table */
  unsigned f;			/* i repeats in table every f entries */
  int g;			/* maximum code length */
  int h;			/* table level */
  unsigned i;			/* counter, current code */
  unsigned j;			/* counter */
  int k;			/* number of bits in current code */
  int l;			/* bits per table (returned in m) */
  unsigned *p;			/* pointer into c[], b[], or v[] */
  struct huft *q;		/* points to current table */
  struct huft r = {0, 0, {0}};		/* table entry for structure assignment */
  struct huft *u[BMAX];		/* table stack */
  unsigned v[N_MAX];		/* values in order of bit length */
  int w;			/* bits before this table == (l * h) */
  unsigned x[BMAX + 1];		/* bit offsets, then code stack */
  unsigned *xp;			/* pointer into x */
  int y;			/* number of dummy codes added */
  unsigned z;			/* number of entries in current table */

  /* Generate counts for each bit length */
  memset ((char *) c, 0, sizeof (c));
  p = b;
  i = n;
  do
    {
      c[*p]++;			/* assume all entries <= BMAX */
      p++;			/* Can't combine with above line (Solaris bug) */
    }
  while (--i);
  if (c[0] == n)		/* null input--all zero length codes */
    {
      *t = (struct huft *) NULL;
      *m = 0;
      return 0;
    }

  /* Find minimum and maximum length, bound *m by those */
  l = *m;
  for (j = 1; j <= BMAX; j++)
    if (c[j])
      break;
  k = j;			/* minimum code length */
  if ((unsigned) l < j)
    l = j;
  for (i = BMAX; i; i--)
    if (c[i])
      break;
  g = i;			/* maximum code length */
  if ((unsigned) l > i)
    l = i;
  *m = l;

  /* Adjust last length count to fill out codes, if needed */
  for (y = 1 << j; j < i; j++, y <<= 1)
    if ((y -= c[j]) < 0)
      return 2;			/* bad input: more codes than bits */
  if ((y -= c[i]) < 0)
    return 2;
  c[i] += y;

  /* Generate starting offsets into the value table for each length */
  x[1] = j = 0;
  p = c + 1;
  xp = x + 2;
  while (--i)
    {				/* note that i == g from above */
      *xp++ = (j += *p++);
    }

  /* Make a table of values in order of bit lengths */
  for (i = 0; i < N_MAX; i++)
    v[i] = N_MAX;
  p = b;
  i = 0;
  do
    {
      if ((j = *p++) != 0)
	v[x[j]++] = i;
    }
  while (++i < n);

  /* Generate the Huffman codes and for each, make the table entries */
  x[0] = i = 0;			/* first Huffman code is zero */
  p = v;			/* grab values in bit order */
  h = -1;			/* no tables yet--level -1 */
  w = -l;			/* bits decoded == (l * h) */
  u[0] = (struct huft *) NULL;	/* just to keep compilers happy */
  q = (struct huft *) NULL;	/* ditto */
  z = 0;			/* ditto */

  /* go through the bit lengths (k already is bits in shortest code) */
  for (; k <= g; k++)
    {
      a = c[k];
      while (a--)
	{
	  /* here i is the Huffman code of length k bits for value *p */
	  /* make tables up to required level */
	  while (k > w + l)
	    {
	      h++;
	      w += l;		/* previous table always l bits */

	      /* compute minimum size table less than or equal to l bits */
	      z = (z = (unsigned) (g - w)) > (unsigned) l ? (unsigned) l : z;	/* upper limit on table size */
	      if ((f = 1 << (j = k - w)) > a + 1)	/* try a k-w bit table */
		{		/* too few codes for k-w bit table */
		  f -= a + 1;	/* deduct codes from patterns left */
		  xp = c + k;
		  while (++j < z)	/* try smaller tables up to z bits */
		    {
		      if ((f <<= 1) <= *++xp)
			break;	/* enough codes to use up j bits */
		      f -= *xp;	/* else deduct codes from patterns */
		    }
		}
	      z = 1 << j;	/* table entries for j-bit table */

	      /* allocate and link in new table */
	      q = (struct huft *) calloc (z + 1, sizeof (struct huft));
	      if (! q)
		{
		  if (h)
		    huft_free (u[0]);
		  return 3;
		}

	      *t = q + 1;	/* link to list for huft_free() */
	      *(t = &(q->v.t)) = (struct huft *) NULL;
	      u[h] = ++q;	/* table starts after link */

	      /* connect to last table, if there is one */
	      if (h)
		{
		  x[h] = i;	/* save pattern for backing up */
		  r.b = (uch) l;	/* bits to dump before this table */
		  r.e = (uch) (16 + j);		/* bits in this table */
		  r.v.t = q;	/* pointer to this table */
		  j = i >> (w - l);	/* (get around Turbo C bug) */
		  u[h - 1][j] = r;	/* connect to last table */
		}
	    }

	  /* set up table entry in r */
	  r.b = (uch) (k - w);
	  if (p >= v + n)
	    r.e = 99;		/* out of values--invalid code */
	  else if (*p < s)
	    {
	      r.e = (uch) (*p < 256 ? 16 : 15);		/* 256 is end-of-block code */
	      r.v.n = (ush) (*p);	/* simple code is just the value */
	      p++;		/* one compiler does not like *p++ */
	    }
	  else if (*p < N_MAX)
	    {
	      r.e = (uch) e[*p - s];	/* non-simple--look up in lists */
	      r.v.n = d[*p++ - s];
	    }
	  else
	    {
	      if (h >= 0)
		huft_free (u[0]);
	      return 2;
	    }

	  /* fill code-like entries with r */
	  f = 1 << (k - w);
	  for (j = i >> w; j < z; j += f)
	    q[j] = r;

	  /* backwards increment the k-bit code i */
	  for (j = 1 << (k - 1); i & j; j >>= 1)
	    i ^= j;
	  i ^= j;

	  /* backup over finished tables */
	  while ((i & ((1 << w) - 1)) != x[h])
	    {
	      h--;		/* don't need to update q */
	      w -= l;
	    }
	}
    }

  /* Return true (1) if we were given an incomplete table */
  return y != 0 && g != 1;
}


/* Free the malloc'ed tables built by huft_build(), which makes a linked
   list of the tables it made, with the links in a dummy first entry of
   each table.  */
static int
huft_free (struct huft *t)
{
  struct huft *p, *q;


  /* Go through linked list, freeing from the malloced (t[-1]) address. */
  p = t;
  while (p != (struct huft *) NULL)
    {
      q = (--p)->v.t;
      free ((char *) p);
      p = q;
    }
  return 0;
}


/*
 *  inflate (decompress) the codes in a deflated (compressed) block.
 *  Return an error code or zero if it all goes ok.
 */

int
GzioReadStream::inflate_codes_in_window()
{
  unsigned e;			/* table entry flag/number of extra bits */
  unsigned n, d;		/* length and index for copy */
  unsigned w;			/* current window position */
  struct huft *t;		/* pointer to table entry */
  unsigned ml, md;		/* masks for bl and bd bits */
  ulg b;			/* bit buffer */
  unsigned k;			/* number of bits in bit buffer */

  /* make local copies of globals */
  d = _inflateD;
  n = _inflateN;
  b = _bb;			/* initialize bit buffer */
  k = _bk;
  w = _wp;			/* initialize window position */

  /* inflate the coded data */
  ml = mask_bits[_bl];		/* precompute masks for speed */
  md = mask_bits[_bd];
  for (;;)			/* do until end of block */
    {
      if (! _codeState)
	{
	  if (_tl == NULL)
	    {
	      _err = true;
	      return 1;
	    }

	  NEEDBITS ((unsigned) _bl);
	  if ((e = (t = _tl + ((unsigned) b & ml))->e) > 16)
	    do
	      {
		if (e == 99)
		  {
                    _err = true;
		    return 1;
		  }
		DUMPBITS (t->b);
		e -= 16;
		NEEDBITS (e);
	      }
	    while ((e = (t = t->v.t + ((unsigned) b & mask_bits[e]))->e) > 16);
	  DUMPBITS (t->b);

	  if (e == 16)		/* then it's a literal */
	    {
	      _slide[w++] = (uch) t->v.n;
	      if (w == WSIZE)
		break;
	    }
	  else
	    /* it's an EOB or a length */
	    {
	      /* exit if end of block */
	      if (e == 15)
		{
		  _blockLen = 0;
		  break;
		}

	      /* get length of block to copy */
	      NEEDBITS (e);
	      n = t->v.n + ((unsigned) b & mask_bits[e]);
	      DUMPBITS (e);

	      if (_td == NULL)
		{
		  _err = true;
		  return 1;
		}

	      /* decode distance of block to copy */
	      NEEDBITS ((unsigned) _bd);
	      if ((e = (t = _td + ((unsigned) b & md))->e) > 16)
		do
		  {
		    if (e == 99)
		      {
                        _err = true;
			return 1;
		      }
		    DUMPBITS (t->b);
		    e -= 16;
		    NEEDBITS (e);
		  }
		while ((e = (t = t->v.t + ((unsigned) b & mask_bits[e]))->e)
		       > 16);
	      DUMPBITS (t->b);
	      NEEDBITS (e);
	      d = w - t->v.n - ((unsigned) b & mask_bits[e]);
	      DUMPBITS (e);
	      _codeState++;
	    }
	}

      if (_codeState)
	{
	  /* do the copy */
	  do
	    {
	      n -= (e = (e = WSIZE - ((d &= WSIZE - 1) > w ? d : w)) > n ? n
		    : e);

	      if (w - d >= e)
		{
		  memmove (_slide + w, _slide + d, e);
		  w += e;
		  d += e;
		}
	      else
		/* purposefully use the overlap for extra copies here!! */
		{
		  while (e--)
		    _slide[w++] = _slide[d++];
		}

	      if (w == WSIZE)
		break;
	    }
	  while (n);

	  if (! n)
	    _codeState--;

	  /* did we break from the loop too soon? */
	  if (w == WSIZE)
	    break;
	}
    }

  /* restore the globals from the locals */
  _inflateD = d;
  _inflateN = n;
  _wp = w;			/* restore global window pointer */
  _bb = b;			/* restore global bit buffer */
  _bk = k;

  return ! _blockLen;
}


/* get header for an inflated type 0 (stored) block. */

void
GzioReadStream::init_stored_block ()
{
  ulg b;			/* bit buffer */
  unsigned k;			/* number of bits in bit buffer */

  /* make local copies of globals */
  b = _bb;			/* initialize bit buffer */
  k = _bk;

  /* go to byte boundary */
  DUMPBITS (k & 7);

  /* get the length and its complement */
  NEEDBITS (16);
  _blockLen = ((unsigned) b & 0xffff);
  DUMPBITS (16);
  if (_mode != GzioReadStream::Mode::CLICKTEAM) {
    NEEDBITS (16);
    if (_blockLen != (int) ((~b) & 0xffff))
      _err = true;
    DUMPBITS (16);
  }

  /* restore global variables */
  _bb = b;
  _bk = k;
}


/* get header for an inflated type 1 (fixed Huffman codes) block.  We should
   either replace this with a custom decoder, or at least precompute the
   Huffman tables. */

void
GzioReadStream::init_fixed_block ()
{
  int i;			/* temporary variable */
  unsigned l[288];		/* length list for huft_build */

  /* set up literal table */
  for (i = 0; i < 144; i++)
    l[i] = 8;
  for (; i < 256; i++)
    l[i] = 9;
  for (; i < 280; i++)
    l[i] = 7;
  for (; i < 288; i++)		/* make a complete, but wrong code set */
    l[i] = 8;
  _bl = 7;
  if (huft_build (l, 288, 257, cplens, cplext, &_tl, &_bl) != 0)
    {
      _err = true;
      return;
    }

  /* set up distance table */
  for (i = 0; i < 30; i++)	/* make an incomplete code set */
    l[i] = 5;
  _bd = 5;
  if (huft_build (l, 30, 0, cpdist, cpdext, &_td, &_bd) > 1)
    {
      _err = true;
      huft_free (_tl);
      _tl = 0;
      return;
    }

  /* indicate we're now working on a block */
  _codeState = 0;
  _blockLen++;
}


/* get header for an inflated type 2 (dynamic Huffman codes) block. */

void
GzioReadStream::init_dynamic_block ()
{
  int i;			/* temporary variables */
  unsigned j;
  unsigned l;			/* last length */
  unsigned m;			/* mask for bit lengths table */
  unsigned n;			/* number of lengths to get */
  unsigned nb;			/* number of bit length codes */
  unsigned nl;			/* number of literal/length codes */
  unsigned nd;			/* number of distance codes */
  unsigned ll[286 + 30];	/* literal/length and distance code lengths */
  ulg b;			/* bit buffer */
  unsigned k;			/* number of bits in bit buffer */
  unsigned *bitorder = (_mode == GzioReadStream::Mode::CLICKTEAM) ? bitorder_clickteam : bitorder_zlib;

  /* make local bit buffer */
  b = _bb;
  k = _bk;

  /* read in table lengths */
  NEEDBITS (5);
  nl = 257 + ((unsigned) b & 0x1f);	/* number of literal/length codes */
  DUMPBITS (5);
  NEEDBITS (5);
  nd = 1 + ((unsigned) b & 0x1f);	/* number of distance codes */
  DUMPBITS (5);
  NEEDBITS (4);
  nb = 4 + ((unsigned) b & 0xf);	/* number of bit length codes */
  DUMPBITS (4);
  if (nl > 286 || nd > 30)
    {
      _err = true;
      return;
    }

  /* read in bit-length-code lengths */
  for (j = 0; j < nb; j++)
    {
      NEEDBITS (3);
      ll[bitorder[j]] = (unsigned) b & 7;
      DUMPBITS (3);
    }
  for (; j < 19; j++)
    ll[bitorder[j]] = 0;

  /* build decoding table for trees--single level, 7 bit lookup */
  _bl = 7;
  if (huft_build (ll, 19, 19, NULL, NULL, &_tl, &_bl) != 0)
    {
      _err = true;
      return;
    }

  /* read in literal and distance code lengths */
  n = nl + nd;
  m = mask_bits[_bl];
  i = l = 0;

  if (_tl == NULL)
    {
      _err = true;
      return;
    }

  while ((unsigned) i < n)
    {
      NEEDBITS ((unsigned) _bl);
      j = (_td = _tl + ((unsigned) b & m))->b;
      DUMPBITS (j);
      j = _td->v.n;
      if (j < 16)		/* length of code in bits (0..15) */
	ll[i++] = l = j;	/* save last length in l */
      else if (j == 16)		/* repeat last length 3 to 6 times */
	{
	  NEEDBITS (2);
	  j = 3 + ((unsigned) b & 3);
	  DUMPBITS (2);
	  if ((unsigned) i + j > n)
	    {
              _err = true;
	      goto fail;
	    }
	  while (j--)
	    ll[i++] = l;
	}
      else if (j == 17)		/* 3 to 10 zero length codes */
	{
	  NEEDBITS (3);
	  j = 3 + ((unsigned) b & 7);
	  DUMPBITS (3);
	  if ((unsigned) i + j > n)
	    {
              _err = true;
	      goto fail;
	    }
	  while (j--)
	    ll[i++] = 0;
	  l = 0;
	}
      else
	/* j == 18: 11 to 138 zero length codes */
	{
	  NEEDBITS (7);
	  j = 11 + ((unsigned) b & 0x7f);
	  DUMPBITS (7);
	  if ((unsigned) i + j > n)
	    {
              _err = true;
	      goto fail;
	    }
	  while (j--)
	    ll[i++] = 0;
	  l = 0;
	}
    }

  /* free decoding table for trees */
  huft_free (_tl);
  _td = 0;
  _tl = 0;

  /* restore the global bit buffer */
  _bb = b;
  _bk = k;

  /* build the decoding tables for literal/length and distance codes */
  _bl = lbits;
  if (huft_build (ll, nl, 257, cplens, cplext, &_tl, &_bl) != 0)
    {
      _err = true;
      _tl = 0;
      return;
    }
  _bd = dbits;
  if (huft_build (ll + nl, nd, 0, cpdist, cpdext, &_td, &_bd) != 0)
    {
      huft_free (_tl);
      _tl = 0;
      _td = 0;
      _err = true;
      return;
    }

  /* indicate we're now working on a block */
  _codeState = 0;
  _blockLen++;
  return;

 fail:
  huft_free (_tl);
  _td = NULL;
  _tl = NULL;
}


void
GzioReadStream::get_new_block()
{
  ulg b;			/* bit buffer */
  unsigned k;			/* number of bits in bit buffer */

  /* make local bit buffer */
  b = _bb;
  k = _bk;

  if (_mode == GzioReadStream::Mode::CLICKTEAM) {
    /* read in block type */
    NEEDBITS (3);
    switch ((unsigned) b & 7) {
    case 5:
      _blockType = INFLATE_FIXED;
      break;
    case 6:
      _blockType = INFLATE_DYNAMIC;
      break;
    case 7:
      _blockType = INFLATE_STORED;
      break;
    }
    DUMPBITS (3);

    /* read in last block bit */
    NEEDBITS (1);
    _lastBlock = (int) b & 1;
    DUMPBITS (1);
  } else {
    /* read in last block bit */
    NEEDBITS (1);
    _lastBlock = (int) b & 1;
    DUMPBITS (1);

    /* read in block type */
    NEEDBITS (2);
    _blockType = (unsigned) b & 3;
    DUMPBITS (2);
  }

  /* restore the global bit buffer */
  _bb = b;
  _bk = k;

  switch (_blockType)
    {
    case INFLATE_STORED:
      init_stored_block ();
      break;
    case INFLATE_FIXED:
      init_fixed_block ();
      break;
    case INFLATE_DYNAMIC:
      init_dynamic_block ();
      break;
    default:
      break;
    }
}


void
GzioReadStream::inflate_window ()
{
  /* initialize window */
  _wp = 0;

  /*
   *  Main decompression loop.
   */

  while (_wp < WSIZE && !_err)
    {
      if (! _blockLen)
	{
	  if (_lastBlock)
	    break;

	  get_new_block ();
	}

      if (_blockType > INFLATE_DYNAMIC)
        _err = true;

      if (_err)
	return;

      /*
       *  Expand stored block here.
       */
      if (_blockType == INFLATE_STORED)
	{
	  int w = _wp;

	  /*
	   *  This is basically a glorified pass-through
	   */

	  while (_blockLen && w < WSIZE && !_err)
	    {
	      _slide[w++] = parentGetByte ();
	      _blockLen--;
	    }

	  _wp = w;

	  continue;
	}

      /*
       *  Expand other kind of block.
       */

      if (inflate_codes_in_window ())
	{
	  huft_free (_tl);
	  huft_free (_td);
	  _tl = 0;
	  _td = 0;
	}
    }

  _savedOffset += _wp;
}


void
GzioReadStream::initialize_tables()
{
  _savedOffset = 0;
  parentSeek (_dataOffset);

  /* Initialize the bit buffer.  */
  _bk = 0;
  _bb = 0;

  /* Reset partial decompression code.  */
  _lastBlock = 0;
  _blockLen = 0;

  /* Reset memory allocation stuff.  */
  huft_free (_tl);
  huft_free (_td);
  _tl = NULL;
  _td = NULL;
}


static uint8
mod_31 (uint16 v)
{
  /* At most 2 iterations for any number that
     we can get here.
     In any case faster than real division.  */
  while (v > 0x1f)
    v = (v & 0x1f) + (v >> 5);
  if (v == 0x1f)
    return 0;
  return v;
}

bool
GzioReadStream::test_zlib_header ()
{
  uint8 cmf, flg;

  cmf = parentGetByte ();
  flg = parentGetByte ();

  /* Check that compression method is DEFLATE.  */
  if ((cmf & 0xf) != GRUB_GZ_DEFLATED)
    {
      return false;
    }

  /* Usually it would be: (cmf * 256 + flg) % 31 != 0.  */
  /* But 256 == 8 (31).  */
  /* By multiplying by 4 and using 32 == 1 (31). We get our formula.  */
  if (mod_31 (cmf + flg * 4) != 0)
    {
      return false;
    }

  /* Dictionary isn't supported.  */
  if (flg & 0x20)
    {
      return false;
    }

  _dataOffset = 2;

  return true;
}

int32
GzioReadStream::readAtOffset (int64 offset, byte *buf, uint32 len)
{
  int32 ret = 0;

  /* Do we reset decompression to the beginning of the file?  */
  if (_savedOffset > offset + WSIZE)
    initialize_tables ();

  /*
   *  This loop operates upon uncompressed data only.  The only
   *  special thing it does is to make sure the decompression
   *  window is within the range of data it needs.
   */

  while (len > 0 && !_err)
    {
      uint32 size;
      char *srcaddr;

      while (offset >= _savedOffset)
	{
	  inflate_window ();
	  if (_wp == 0)
	    goto out;
	}

      if (_wp == 0)
	goto out;

      srcaddr = (char *) ((offset & (WSIZE - 1)) + _slide);
      size = _savedOffset - offset;
      if (size > len)
	size = len;

      memmove (buf, srcaddr, size);

      buf += size;
      len -= size;
      ret += size;
      offset += size;
    }

 out:
  if (_err)
    ret = -1;
  return ret;
}

uint32 GzioReadStream::read(void *dataPtr, uint32 dataSize) {
	bool maybeEos = false;
	// Read at most as many bytes as are still available...
	if (dataSize > _uncompressedSize - _streamPos) {
		dataSize = _uncompressedSize - _streamPos;
		maybeEos = true;
	}

	int32 actualRead = readAtOffset(_streamPos, (byte *)dataPtr, dataSize);
	if (actualRead < 0) {
		_err = true;
		return 0;
	}

	_streamPos += actualRead;

	if (maybeEos &&	actualRead == (int32)dataSize)
		_eos = true;

	return actualRead;
}

bool GzioReadStream::seek(int64 offs, int whence) {
	// Pre-Condition
	assert(_streamPos <= _uncompressedSize);
	switch (whence) {
	case SEEK_END:
		_streamPos = _uncompressedSize + offs;
		break;
	case SEEK_SET:
	default:
		_streamPos = offs;
		break;

	case SEEK_CUR:
		_streamPos += offs;
		break;
	}
	// Post-Condition
	assert(_streamPos <= _uncompressedSize);

	// Reset end-of-stream flag on a successful seek
	_eos = false;
	return true;
}

GzioReadStream* GzioReadStream::openDeflate(Common::SeekableReadStream *parent, uint64 uncompressed_size, DisposeAfterUse::Flag disposeParent)
{
  GzioReadStream *gzio = new GzioReadStream(parent, disposeParent, uncompressed_size, GzioReadStream::Mode::ZLIB);

  gzio->initialize_tables ();

  return gzio;
}

GzioReadStream* GzioReadStream::openClickteam(Common::SeekableReadStream *parent, uint64 uncompressed_size, DisposeAfterUse::Flag disposeParent)
{
  GzioReadStream *gzio = new GzioReadStream(parent, disposeParent, uncompressed_size, GzioReadStream::Mode::CLICKTEAM);

  gzio->initialize_tables ();

  return gzio;
}

GzioReadStream* GzioReadStream::openZlib(Common::SeekableReadStream *parent, uint64 uncompressed_size, DisposeAfterUse::Flag disposeParent)
{
  GzioReadStream* gzio = new GzioReadStream(parent, disposeParent, uncompressed_size, GzioReadStream::Mode::ZLIB);

  if (!gzio->test_zlib_header())
    {
      delete gzio;
      return nullptr;
    }

  gzio->initialize_tables();

  return gzio;
}

int32
GzioReadStream::clickteamDecompress (byte *outbuf, uint32 outsize, byte *inbuf, uint32 insize, int64 off)
{
  Common::ScopedPtr<GzioReadStream> gzio(GzioReadStream::openClickteam(new Common::MemoryReadStream(inbuf, insize, DisposeAfterUse::NO), outsize + off, DisposeAfterUse::YES));
  if (!gzio)
    return -1;
  return gzio->readAtOffset(off, outbuf, outsize);
}

int32
GzioReadStream::deflateDecompress (byte *outbuf, uint32 outsize, byte *inbuf, uint32 insize, int64 off)
{
  Common::ScopedPtr<GzioReadStream> gzio(GzioReadStream::openDeflate(new Common::MemoryReadStream(inbuf, insize, DisposeAfterUse::NO), outsize + off, DisposeAfterUse::YES));
  if (!gzio)
    return -1;
  return gzio->readAtOffset(off, outbuf, outsize);
}

int32
GzioReadStream::zlibDecompress (byte *outbuf, uint32 outsize, byte *inbuf, uint32 insize, int64 off)
{
  Common::ScopedPtr<GzioReadStream> gzio(GzioReadStream::openZlib(new Common::MemoryReadStream(inbuf, insize, DisposeAfterUse::NO), outsize + off, DisposeAfterUse::YES));
  if (!gzio)
    return -1;
  return gzio->readAtOffset(off, outbuf, outsize);
}

}
