//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// LZW compression -- the LZW/GIF patent has expired, so we can use it now!!!
//
//=============================================================================

#include <stdlib.h>
#include "ac/common.h" // quit
#include "util/stream.h"

using namespace AGS::Common;

#ifdef _MANAGED
// ensure this doesn't get compiled to .NET IL
#pragma unmanaged
#endif

int insert(int, int);
void _delete(int);

#define N 4096
#define F 16
#define THRESHOLD 3
#define min(xx,yy) ((yy<xx) ? yy : xx)

#define dad (node+1)
#define lson (node+1+N)
#define rson (node+1+N+N)
#define root (node+1+N+N+N)
#define NIL -1

char *lzbuffer;
int *node;
int pos;
long outbytes = 0, maxsize = 0, putbytes = 0;

int insert(int i, int run)
{
  int c, j, k, l, n, match;
  int *p;

  c = NIL;

  k = l = 1;
  match = THRESHOLD - 1;
  p = &root[(unsigned char)lzbuffer[i]];
  lson[i] = rson[i] = NIL;
  while ((j = *p) != NIL) {
    for (n = min(k, l); n < run && (c = (lzbuffer[j + n] - lzbuffer[i + n])) == 0; n++) ;

    if (n > match) {
      match = n;
      pos = j;
    }

    if (c < 0) {
      p = &lson[j];
      k = n;
    } else if (c > 0) {
      p = &rson[j];
      l = n;
    } else {
      dad[j] = NIL;
      dad[lson[j]] = lson + i - node;
      dad[rson[j]] = rson + i - node;
      lson[i] = lson[j];
      rson[i] = rson[j];
      break;
    }
  }

  dad[i] = p - node;
  *p = i;
  return match;
}

void _delete(int z)
{
  int j;

  if (dad[z] != NIL) {
    if (rson[z] == NIL)
      j = lson[z];
    else if (lson[z] == NIL)
      j = rson[z];
    else {
      j = lson[z];
      if (rson[j] != NIL) {
        do {
          j = rson[j];
        } while (rson[j] != NIL);

        node[dad[j]] = lson[j];
        dad[lson[j]] = dad[j];
        lson[j] = lson[z];
        dad[lson[z]] = lson + j - node;
      }

      rson[j] = rson[z];
      dad[rson[z]] = rson + j - node;
    }

    dad[j] = dad[z];
    node[dad[z]] = j;
    dad[z] = NIL;
  }
}

void lzwcompress(Stream *lzw_in, Stream *out)
{
  int ch, i, run, len, match, size, mask;
  char buf[17];

  lzbuffer = (char *)malloc(N + F + (N + 1 + N + N + 256) * sizeof(int));       // 28.5 k !
  if (lzbuffer == nullptr) {
    quit("unable to compress: out of memory");
  }

  node = (int *)(lzbuffer + N + F);
  for (i = 0; i < 256; i++)
    root[i] = NIL;

  for (i = NIL; i < N; i++)
    dad[i] = NIL;

  size = mask = 1;
  buf[0] = 0;
  i = N - F - F;

  for (len = 0; len < F && (ch = lzw_in->ReadByte()) != -1; len++) {
    lzbuffer[i + F] = ch;
    i = (i + 1) & (N - 1);
  }

  run = len;

  do {
    ch = lzw_in->ReadByte();
    if (i >= N - F) {
      _delete(i + F - N);
      lzbuffer[i + F] = lzbuffer[i + F - N] = ch;
    } else {
      _delete(i + F);
      lzbuffer[i + F] = ch;
    }

    match = insert(i, run);
    if (ch == -1) {
      run--;
      len--;
    }

    if (len++ >= run) {
      if (match >= THRESHOLD) {
        buf[0] |= mask;
        // possible fix: change int* to short* ??
        *(short *)(buf + size) = ((match - 3) << 12) | ((i - pos - 1) & (N - 1));
        size += 2;
        len -= match;
      } else {
        buf[size++] = lzbuffer[i];
        len--;
      }

      if (!((mask += mask) & 0xFF)) {
        out->WriteArray(buf, size, 1);
        outbytes += size;
        size = mask = 1;
        buf[0] = 0;
      }
    }
    i = (i + 1) & (N - 1);
  } while (len > 0);

  if (size > 1) {
    out->WriteArray(buf, size, 1);
    outbytes += size;
  }

  free(lzbuffer);
}

int expand_to_mem = 0;
unsigned char *membfptr = nullptr;
void myputc(int ccc, Stream *out)
{
  if (maxsize > 0) {
    putbytes++;
    if (putbytes > maxsize)
      return;
  }

  outbytes++;
  if (expand_to_mem) {
    membfptr[0] = ccc;
    membfptr++;
  } else
    out->WriteInt8(ccc);
}

void lzwexpand(Stream *lzw_in, Stream *out)
{
  int bits, ch, i, j, len, mask;
  char *lzbuffer;
//  printf(" UnShrinking: %s ",filena);
  putbytes = 0;

  lzbuffer = (char *)malloc(N);
  if (lzbuffer == nullptr) {
    quit("compress.cpp: unable to decompress: insufficient memory");
  }
  i = N - F;

  // this end condition just checks for EOF, which is no good to us
  while ((bits = lzw_in->ReadByte()) != -1) {
    for (mask = 0x01; mask & 0xFF; mask <<= 1) {
      if (bits & mask) {
        // MACPORT FIX: read to short and expand
        short jshort = 0;
        jshort = lzw_in->ReadInt16();
        j = jshort;

        len = ((j >> 12) & 15) + 3;
        j = (i - j - 1) & (N - 1);

        while (len--) {
          myputc(lzbuffer[i] = lzbuffer[j], out);
          j = (j + 1) & (N - 1);
          i = (i + 1) & (N - 1);
        }
      } else {
        ch = lzw_in->ReadByte();
        myputc(lzbuffer[i] = ch, out);
        i = (i + 1) & (N - 1);
      }

      if ((putbytes >= maxsize) && (maxsize > 0))
        break;

      if ((lzw_in->EOS()) && (maxsize > 0))
        quit("Read error decompressing image - file is corrupt");
    }                           // end for mask

    if ((putbytes >= maxsize) && (maxsize > 0))
      break;
  }

  free(lzbuffer);
  expand_to_mem = 0;
}

unsigned char *lzwexpand_to_mem(Stream *in)
{
  unsigned char *membuff = (unsigned char *)malloc(maxsize + 10);
  expand_to_mem = 1;
  membfptr = membuff;
  lzwexpand(in, nullptr);
  return membuff;
}
