/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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

#ifndef BOMP_H
#define BOMP_H

#include "common/scummsys.h"

namespace Scumm {

int32 setupBompScale(byte *scaling, int32 size, byte scale);

void bompApplyMask(byte *line_buffer, byte *mask, byte maskbit, int32 size, byte transparency);
void bompApplyShadow(int shadowMode, const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency);

void decompressBomp(byte *dst, const byte *src, int w, int h);
void bompDecodeLine(byte *dst, const byte *src, int size);
void bompDecodeLineReverse(byte *dst, const byte *src, int size);

} // End of namespace Scumm

#endif
