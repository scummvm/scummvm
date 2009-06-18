/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

/* This file is part of libmspack.
 * (C) 2003-2004 Stuart Caie.
 *
 * This source code is adopted and striped for Residual project.
 *
 * The deflate method was created by Phil Katz. MSZIP is equivalent to the
 * deflate method.
 *
 * libmspack is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) version 2.1
 *
 * For further details, see the file COPYING.LIB distributed with libmspack
 */

#include "tools/patchex/mszip.h"

static const unsigned short lit_lengths[29] = {
  3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27,
  31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};

static const unsigned short dist_offsets[30] = {
  1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385,
  513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};

static const unsigned char lit_extrabits[29] = {
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2,
  2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};

static const unsigned char dist_extrabits[30] = {
  0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6,
  6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

static const unsigned char bitlen_order[19] = {
  16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

static const unsigned short bit_mask[17] = {
 0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
 0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

#define STORE_BITS do {                                                 \
  zip->i_ptr      = i_ptr;                                              \
  zip->i_end      = i_end;                                              \
  zip->bit_buffer = bit_buffer;                                         \
  zip->bits_left  = bits_left;                                          \
} while (0)

#define RESTORE_BITS do {                                               \
  i_ptr      = zip->i_ptr;                                              \
  i_end      = zip->i_end;                                              \
  bit_buffer = zip->bit_buffer;                                         \
  bits_left  = zip->bits_left;                                          \
} while (0)

#define ENSURE_BITS(nbits) do {                                         \
  while (bits_left < (nbits)) {                                         \
    if (i_ptr >= i_end) {                                               \
      if (zipd_read_input(zip)) return zip->error;                      \
      i_ptr = zip->i_ptr;                                               \
      i_end = zip->i_end;                                               \
    }                                                                   \
    bit_buffer |= *i_ptr++ << bits_left; bits_left  += 8;               \
  }                                                                     \
} while (0)

#define PEEK_BITS(nbits)   (bit_buffer & ((1<<(nbits))-1))
#define PEEK_BITS_T(nbits) (bit_buffer & bit_mask[(nbits)])

#define REMOVE_BITS(nbits) ((bit_buffer >>= (nbits)), (bits_left -= (nbits)))

#define READ_BITS(val, nbits) do {                                      \
  ENSURE_BITS(nbits); (val) = PEEK_BITS(nbits); REMOVE_BITS(nbits);     \
} while (0)

#define READ_BITS_T(val, nbits) do {                                    \
  ENSURE_BITS(nbits); (val) = PEEK_BITS_T(nbits); REMOVE_BITS(nbits);   \
} while (0)

static int zipd_read_input(struct mszipd_stream *zip) {
  int read = zip->sys->read(zip->input, &zip->inbuf[0], (int)zip->inbuf_size);
  if (read < 0) return zip->error = MSPACK_ERR_READ;
  zip->i_ptr = &zip->inbuf[0];
  zip->i_end = &zip->inbuf[read];

  return MSPACK_ERR_OK;
}

#define INF_ERR_BLOCKTYPE   (-1)
#define INF_ERR_COMPLEMENT  (-2)
#define INF_ERR_FLUSH       (-3)
#define INF_ERR_BITBUF      (-4)
#define INF_ERR_SYMLENS     (-5)
#define INF_ERR_BITLENTBL   (-6)
#define INF_ERR_LITERALTBL  (-7)
#define INF_ERR_DISTANCETBL (-8)
#define INF_ERR_BITOVERRUN  (-9)
#define INF_ERR_BADBITLEN   (-10)
#define INF_ERR_LITCODE     (-11)
#define INF_ERR_DISTCODE    (-12)
#define INF_ERR_DISTANCE    (-13)
#define INF_ERR_HUFFSYM     (-14)

static int make_decode_table(unsigned int nsyms, unsigned int nbits,
			     unsigned char *length, unsigned short *table)
{
  register unsigned int leaf, reverse, fill;
  register unsigned short sym, next_sym;
  register unsigned char bit_num;
  unsigned int pos         = 0;
  unsigned int table_mask  = 1 << nbits;
  unsigned int bit_mask    = table_mask >> 1;

  for (bit_num = 1; bit_num <= nbits; bit_num++) {
    for (sym = 0; sym < nsyms; sym++) {
      if (length[sym] != bit_num) continue;

      fill = length[sym]; reverse = pos >> (nbits - fill); leaf = 0;
      do {leaf <<= 1; leaf |= reverse & 1; reverse >>= 1;} while (--fill);

      if((pos += bit_mask) > table_mask) return 1;

      fill = bit_mask; next_sym = 1 << bit_num;
      do { table[leaf] = sym; leaf += next_sym; } while (--fill);
    }
    bit_mask >>= 1;
  }

  if (pos == table_mask) return 0;

  for (sym = pos; sym < table_mask; sym++) {
    reverse = sym; leaf = 0; fill = nbits;
    do { leaf <<= 1; leaf |= reverse & 1; reverse >>= 1; } while (--fill);
    table[leaf] = 0xFFFF;
  }

  next_sym = ((table_mask >> 1) < nsyms) ? nsyms : (table_mask >> 1);

  pos <<= 16;
  table_mask <<= 16;
  bit_mask = 1 << 15;

  for (bit_num = nbits+1; bit_num <= MSZIP_MAX_HUFFBITS; bit_num++) {
    for (sym = 0; sym < nsyms; sym++) {
      if (length[sym] != bit_num) continue;

      reverse = pos >> 16; leaf = 0; fill = nbits;
      do {leaf <<= 1; leaf |= reverse & 1; reverse >>= 1;} while (--fill);

      for (fill = 0; fill < (bit_num - nbits); fill++) {
	if (table[leaf] == 0xFFFF) {
	  table[(next_sym << 1)     ] = 0xFFFF;
	  table[(next_sym << 1) + 1 ] = 0xFFFF;
	  table[leaf] = next_sym++;
	}
	leaf = (table[leaf] << 1) | ((pos >> (15 - fill)) & 1);
      }
      table[leaf] = sym;

      if ((pos += bit_mask) > table_mask) return 1;
    }
    bit_mask >>= 1;
  }

  return (pos != table_mask) ? 1 : 0;
}

#define READ_HUFFSYM(tbl, var) do {                                     \
  ENSURE_BITS(MSZIP_MAX_HUFFBITS);                                      \
  sym = zip->tbl##_table[PEEK_BITS(MSZIP_##tbl##_TABLEBITS)];		\
  if (sym >= MSZIP_##tbl##_MAXSYMBOLS) {                                \
    i = MSZIP_##tbl##_TABLEBITS - 1;					\
    do {                                                                \
      if (i++ > MSZIP_MAX_HUFFBITS) {					\
        return INF_ERR_HUFFSYM;                                         \
      }                                                                 \
      sym = zip->tbl##_table[(sym << 1) | ((bit_buffer >> i) & 1)];	\
    } while (sym >= MSZIP_##tbl##_MAXSYMBOLS);                          \
  }                                                                     \
  (var) = sym;                                                          \
  i = zip->tbl##_len[sym];                                              \
  REMOVE_BITS(i);                                                       \
} while (0)

static int zip_read_lens(struct mszipd_stream *zip) {
  register unsigned int bit_buffer;
  register int bits_left;
  unsigned char *i_ptr, *i_end;

  unsigned short bl_table[(1 << 7)];
  unsigned char bl_len[19];

  unsigned char lens[MSZIP_LITERAL_MAXSYMBOLS + MSZIP_DISTANCE_MAXSYMBOLS];
  unsigned int lit_codes, dist_codes, code, last_code=0, bitlen_codes, i, run;

  RESTORE_BITS;

  READ_BITS(lit_codes,    5); lit_codes    += 257;
  READ_BITS(dist_codes,   5); dist_codes   += 1;
  READ_BITS(bitlen_codes, 4); bitlen_codes += 4;
  if (lit_codes  > MSZIP_LITERAL_MAXSYMBOLS)  return INF_ERR_SYMLENS;
  if (dist_codes > MSZIP_DISTANCE_MAXSYMBOLS) return INF_ERR_SYMLENS;

  for (i = 0; i < bitlen_codes; i++) READ_BITS(bl_len[bitlen_order[i]], 3);
  while (i < 19) bl_len[bitlen_order[i++]] = 0;

  if (make_decode_table(19, 7, &bl_len[0], &bl_table[0])) {
    return INF_ERR_BITLENTBL;
  }

  for (i = 0; i < (lit_codes + dist_codes); i++) {
    ENSURE_BITS(7);
    code = bl_table[PEEK_BITS(7)];
    REMOVE_BITS(bl_len[code]);

    if (code < 16) lens[i] = last_code = code;
    else {
      switch (code) {
      case 16: READ_BITS(run, 2); run += 3;  code = last_code; break;
      case 17: READ_BITS(run, 3); run += 3;  code = 0;         break;
      case 18: READ_BITS(run, 7); run += 11; code = 0;         break;
      default: return INF_ERR_BADBITLEN;
      }
      if ((i + run) > (lit_codes + dist_codes)) return INF_ERR_BITOVERRUN;
      while (run--) lens[i++] = code;
      i--;
    }
  }

  i = lit_codes;
  zip->sys->copy(&lens[0], &zip->LITERAL_len[0], i);
  while (i < MSZIP_LITERAL_MAXSYMBOLS) zip->LITERAL_len[i++] = 0;

  i = dist_codes;
  zip->sys->copy(&lens[lit_codes], &zip->DISTANCE_len[0], i);
  while (i < MSZIP_DISTANCE_MAXSYMBOLS) zip->DISTANCE_len[i++] = 0;

  STORE_BITS;
  return 0;
}

static int inflate(struct mszipd_stream *zip) {
  unsigned int last_block, block_type, distance, length, this_run, i;

  register unsigned int bit_buffer;
  register int bits_left;
  register unsigned short sym;
  unsigned char *i_ptr, *i_end;

  RESTORE_BITS;

  do {
    READ_BITS(last_block, 1);

    READ_BITS(block_type, 2);

    if (block_type == 0) {
      unsigned char lens_buf[4];

      i = bits_left & 7; REMOVE_BITS(i);

      for (i = 0; (bits_left >= 8); i++) {
	if (i == 4) return INF_ERR_BITBUF;
	lens_buf[i] = PEEK_BITS(8);
	REMOVE_BITS(8);
      }
      if (bits_left != 0) return INF_ERR_BITBUF;
      while (i < 4) {
	if (i_ptr >= i_end) {
	  if (zipd_read_input(zip)) return zip->error;
	  i_ptr = zip->i_ptr;
	  i_end = zip->i_end;
	}
	lens_buf[i++] = *i_ptr++;
      }

      length = lens_buf[0] | (lens_buf[1] << 8);
      i      = lens_buf[2] | (lens_buf[3] << 8);
      if (length != (~i & 0xFFFF)) return INF_ERR_COMPLEMENT;

      while (length > 0) {
	if (i_ptr >= i_end) {
	  if (zipd_read_input(zip)) return zip->error;
	  i_ptr = zip->i_ptr;
	  i_end = zip->i_end;
	}

	this_run = length;
	if (this_run > (unsigned int)(i_end - i_ptr)) this_run = i_end - i_ptr;
	if (this_run > (MSZIP_FRAME_SIZE - zip->window_posn))
	  this_run = MSZIP_FRAME_SIZE - zip->window_posn;

	zip->sys->copy(i_ptr, &zip->window[zip->window_posn], this_run);
	zip->window_posn += this_run;
	i_ptr    += this_run;
	length   -= this_run;

	if (zip->window_posn == MSZIP_FRAME_SIZE) {
	  if (zip->flush_window(zip, MSZIP_FRAME_SIZE)) return INF_ERR_FLUSH;
	  zip->window_posn = 0;
	}
      }
    }
    else if ((block_type == 1) || (block_type == 2)) {
      unsigned int window_posn, match_posn, code;

      if (block_type == 1) {
	i = 0;
	while (i < 144) zip->LITERAL_len[i++] = 8;
	while (i < 256) zip->LITERAL_len[i++] = 9;
	while (i < 280) zip->LITERAL_len[i++] = 7;
	while (i < 288) zip->LITERAL_len[i++] = 8;
	for (i = 0; i < 32; i++) zip->DISTANCE_len[i] = 5;
      }
      else {
	STORE_BITS;
	if ((i = zip_read_lens(zip))) return i;
	RESTORE_BITS;
      }

      if (make_decode_table(MSZIP_LITERAL_MAXSYMBOLS, MSZIP_LITERAL_TABLEBITS,
			    &zip->LITERAL_len[0], &zip->LITERAL_table[0]))
      {
	return INF_ERR_LITERALTBL;
      }

      if (make_decode_table(MSZIP_DISTANCE_MAXSYMBOLS,MSZIP_DISTANCE_TABLEBITS,
			    &zip->DISTANCE_len[0], &zip->DISTANCE_table[0]))
      {
	return INF_ERR_DISTANCETBL;
      }

      window_posn = zip->window_posn;
      while (1) {
	READ_HUFFSYM(LITERAL, code);
	if (code < 256) {
	  zip->window[window_posn++] = (unsigned char) code;
	  if (window_posn == MSZIP_FRAME_SIZE) {
	    if (zip->flush_window(zip, MSZIP_FRAME_SIZE)) return INF_ERR_FLUSH;
	    window_posn = 0;
	  }
	}
	else if (code == 256) {
	  break;
	}
	else {
	  code -= 257;
	  if (code > 29) return INF_ERR_LITCODE;
	  READ_BITS_T(length, lit_extrabits[code]);
	  length += lit_lengths[code];

	  READ_HUFFSYM(DISTANCE, code);
	  if (code > 30) return INF_ERR_DISTCODE;
	  READ_BITS_T(distance, dist_extrabits[code]);
	  distance += dist_offsets[code];

	  match_posn = ((distance > window_posn) ? MSZIP_FRAME_SIZE : 0)
	    + window_posn - distance;

	  if (length < 12) {
	    while (length--) {
	      zip->window[window_posn++] = zip->window[match_posn++];
	      match_posn &= MSZIP_FRAME_SIZE - 1;

	      if (window_posn == MSZIP_FRAME_SIZE) {
		if (zip->flush_window(zip, MSZIP_FRAME_SIZE))
		  return INF_ERR_FLUSH;
		window_posn = 0;
	      }
	    }
	  }
	  else {
	    unsigned char *runsrc, *rundest;
	    do {
	      this_run = length;
	      if ((match_posn + this_run) > MSZIP_FRAME_SIZE)
		this_run = MSZIP_FRAME_SIZE - match_posn;
	      if ((window_posn + this_run) > MSZIP_FRAME_SIZE)
		this_run = MSZIP_FRAME_SIZE - window_posn;

	      rundest = &zip->window[window_posn]; window_posn += this_run;
	      runsrc  = &zip->window[match_posn];  match_posn  += this_run;
	      length -= this_run;
	      while (this_run--) *rundest++ = *runsrc++;

	      if (window_posn == MSZIP_FRAME_SIZE) {
		if (zip->flush_window(zip, MSZIP_FRAME_SIZE))
		  return INF_ERR_FLUSH;
		window_posn = 0;
	      }
	      if (match_posn == MSZIP_FRAME_SIZE) match_posn = 0;
	    } while (length > 0);
	  }

	}

      }
      zip->window_posn = window_posn;
    }
    else {
      return INF_ERR_BLOCKTYPE;
    }
  } while (!last_block);

  if (zip->window_posn) {
    if (zip->flush_window(zip, zip->window_posn)) return INF_ERR_FLUSH;
  }
  STORE_BITS;

  return 0;
}

static int mszipd_flush_window(struct mszipd_stream *zip,
			       unsigned int data_flushed)
{
  zip->bytes_output += data_flushed;
  if (zip->bytes_output > MSZIP_FRAME_SIZE) {
    return 1;
  }
  return 0;
}

struct mszipd_stream *mszipd_init(struct mspack_system *system,
				  struct mspack_file *input,
				  struct mspack_file *output,
				  int input_buffer_size,
				  int repair_mode)
{
  struct mszipd_stream *zip;

  if (!system) return NULL;

  input_buffer_size = (input_buffer_size + 1) & -2;
  if (!input_buffer_size) return NULL;

  if (!(zip = (mszipd_stream *)system->alloc(system, sizeof(struct mszipd_stream)))) {
    return NULL;
  }

  zip->inbuf  = (unsigned char *)system->alloc(system, (size_t) input_buffer_size);
  if (!zip->inbuf) {
    system->free(zip);
    return NULL;
  }

  zip->sys             = system;
  zip->input           = input;
  zip->output          = output;
  zip->inbuf_size      = input_buffer_size;
  zip->error           = MSPACK_ERR_OK;
  zip->repair_mode     = repair_mode;
  zip->flush_window    = &mszipd_flush_window;

  zip->i_ptr = zip->i_end = &zip->inbuf[0];
  zip->o_ptr = zip->o_end = NULL;
  zip->bit_buffer = 0; zip->bits_left = 0;
  return zip;
}

int mszipd_decompress(struct mszipd_stream *zip, off_t out_bytes) {
  register unsigned int bit_buffer;
  register int bits_left;
  unsigned char *i_ptr, *i_end;

  int i, state, error;

  if (!zip || (out_bytes < 0)) return MSPACK_ERR_ARGS;
  if (zip->error) return zip->error;

  i = zip->o_end - zip->o_ptr;
  if ((off_t) i > out_bytes) i = (int) out_bytes;
  if (i) {
    if (zip->sys->write(zip->output, zip->o_ptr, i) != i) {
      return zip->error = MSPACK_ERR_WRITE;
    }
    zip->o_ptr  += i;
    out_bytes   -= i;
  }
  if (out_bytes == 0) return MSPACK_ERR_OK;


  while (out_bytes > 0) {
    RESTORE_BITS;

    i = bits_left & 7; REMOVE_BITS(i);
    state = 0;
    do {
      READ_BITS(i, 8);
      if (i == 'C') state = 1;
      else if ((state == 1) && (i == 'K')) state = 2;
      else state = 0;
    } while (state != 2);

    zip->window_posn = 0;
    zip->bytes_output = 0;
    STORE_BITS;
    if ((error = inflate(zip))) {
      if (zip->repair_mode) {
	zip->sys->message(NULL, "MSZIP error, %u bytes of data lost.",
			  MSZIP_FRAME_SIZE - zip->bytes_output);
	for (i = zip->bytes_output; i < MSZIP_FRAME_SIZE; i++) {
	  zip->window[i] = '\0';
	}
	zip->bytes_output = MSZIP_FRAME_SIZE;
      }
      else {
	return zip->error = (error > 0) ? error : MSPACK_ERR_DECRUNCH;
      }
    }
    zip->o_ptr = &zip->window[0];
    zip->o_end = &zip->o_ptr[zip->bytes_output];

    i = (out_bytes < (off_t)zip->bytes_output) ?
      (int)out_bytes : zip->bytes_output;
    if (zip->sys->write(zip->output, zip->o_ptr, i) != i) {
      return zip->error = MSPACK_ERR_WRITE;
    }

    if ((error > 0) && zip->repair_mode) return error;

    zip->o_ptr  += i;
    out_bytes   -= i;
  }

  if (out_bytes) {
    return zip->error = MSPACK_ERR_DECRUNCH;
  }
  return MSPACK_ERR_OK;
}

void mszipd_free(struct mszipd_stream *zip) {
  struct mspack_system *sys;
  if (zip) {
    sys = zip->sys;
    sys->free(zip->inbuf);
    sys->free(zip);
  }
}
