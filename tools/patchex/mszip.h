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

#ifndef MSPACK_MSZIP_H
#define MSPACK_MSZIP_H 1

#include "tools/patchex/mspack.h"

#define MSZIP_FRAME_SIZE          (32768)
#define MSZIP_MAX_HUFFBITS        (16)
#define MSZIP_LITERAL_MAXSYMBOLS  (288)
#define MSZIP_LITERAL_TABLEBITS   (9)
#define MSZIP_DISTANCE_MAXSYMBOLS (32)
#define MSZIP_DISTANCE_TABLEBITS  (6)

#if (1 << MSZIP_LITERAL_TABLEBITS) < (MSZIP_LITERAL_MAXSYMBOLS * 2)
# define MSZIP_LITERAL_TABLESIZE (MSZIP_LITERAL_MAXSYMBOLS * 4)
#else
# define MSZIP_LITERAL_TABLESIZE ((1 << MSZIP_LITERAL_TABLEBITS) + \
				  (MSZIP_LITERAL_MAXSYMBOLS * 2))
#endif

#if (1 << MSZIP_DISTANCE_TABLEBITS) < (MSZIP_DISTANCE_MAXSYMBOLS * 2)
# define MSZIP_DISTANCE_TABLESIZE (MSZIP_DISTANCE_MAXSYMBOLS * 4)
#else
# define MSZIP_DISTANCE_TABLESIZE ((1 << MSZIP_DISTANCE_TABLEBITS) + \
				  (MSZIP_DISTANCE_MAXSYMBOLS * 2))
#endif

struct mszipd_stream {
  struct mspack_system *sys;
  struct mspack_file   *input;
  struct mspack_file   *output;
  unsigned int window_posn;

  int (*flush_window)(struct mszipd_stream *, unsigned int);

  int error, repair_mode, bytes_output;

  unsigned char *inbuf, *i_ptr, *i_end, *o_ptr, *o_end;
  unsigned int bit_buffer, bits_left, inbuf_size;

  unsigned char  LITERAL_len[MSZIP_LITERAL_MAXSYMBOLS];
  unsigned char  DISTANCE_len[MSZIP_DISTANCE_MAXSYMBOLS];

  unsigned short LITERAL_table [MSZIP_LITERAL_TABLESIZE];
  unsigned short DISTANCE_table[MSZIP_DISTANCE_TABLESIZE];

  unsigned char window[MSZIP_FRAME_SIZE];
};

extern struct mszipd_stream *mszipd_init(struct mspack_system *system,
					struct mspack_file *input,
					struct mspack_file *output,
					int input_buffer_size,
					int repair_mode);

extern int mszipd_decompress(struct mszipd_stream *zip, off_t out_bytes);

void mszipd_free(struct mszipd_stream *zip);

#endif
