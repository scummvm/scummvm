/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef ANIMATION_H
#define ANIMATION_H

#ifndef _MSC_VER
#include <inttypes.h>
#endif
#ifdef USE_MPEG2
extern "C" {
	#include <mpeg2dec/mpeg2.h>
}
#endif

namespace Sword2 {

#define SQR(x) ((x) * (x))

#define SHIFT 3
#define BITDEPTH (1 << (8 - SHIFT))
#define ROUNDADD (1 << (SHIFT - 1))

#define BUFFER_SIZE 4096


class AnimationState {
public:
  int palnum;

  byte lookup[2][BITDEPTH * BITDEPTH * BITDEPTH];
  byte *lut;
  byte *lut2;
  int lutcalcnum;

  int framenum;

  #ifdef USE_MPEG2
  mpeg2dec_t *decoder;
  const mpeg2_info_t *info;
  #endif
  File *mpgfile;

  int curpal;
  int cr;
  int pos;

  struct {
    int cnt;
    int end;
    byte pal[4 * 256];
  } palettes[50];

  byte buffer[BUFFER_SIZE];

  PlayingSoundHandle bgSound;

public:
	void buildLookup(int p, int lines);

};

} // End of namespace Sword2

#endif
