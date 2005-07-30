/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

/* Various decompression routines */
#ifndef CODECS_H
#define CODECS_H

// THIS CODE WAS TAKEN FROM FreeKyra Tools Module

#include "common/stdafx.h"
#include "common/scummsys.h"

namespace Kyra {
class Compression
{
public:
	static int decode80(const uint8* image_in, uint8* image_out);
	static int decode40(const uint8* image_in, uint8* image_out);
	static int decode3(const uint8* image_in, uint8* image_out, int s);
	static int decode2(const uint8* s, uint8* d, int cb_s);
};
} // end of namespace Kyra

#endif

