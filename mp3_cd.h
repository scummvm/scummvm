/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef MP3_CD_H
#define MP3_CD_H

// From xing.h in MAD

# define XING_MAGIC	(('X' << 24) | ('i' << 16) | ('n' << 8) | 'g')

struct xing {
  long flags;			/* valid fields (see below) */
  unsigned long frames;		/* total number of frames */
  unsigned long bytes;		/* total number of bytes */
  unsigned char toc[100];	/* 100-point seek table */
  long scale;			/* ?? */
};

enum {
  XING_FRAMES = 0x00000001L,
  XING_BYTES  = 0x00000002L,
  XING_TOC    = 0x00000004L,
  XING_SCALE  = 0x00000008L
};

BOOL mp3_cd_play(Scumm *s, int track, int num_loops, int start_frame, int end_frame);

#endif