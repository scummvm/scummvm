/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2008 The ScummVM-Residual Team (www.scummvm.org)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "common/zlib.h"

#if defined(USE_ZLIB)

#ifdef __SYMBIAN32__
#include <zlib\zlib.h>
#else
#include <zlib.h>
#endif

namespace Common {

int uncompress(byte *dst, unsigned long *dstLen, const byte *src, unsigned long srcLen) {
	return ::uncompress(dst, dstLen, src, srcLen);
}

} // end of namespace Common

#endif

