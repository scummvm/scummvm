/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_ZLIB_H
#define COMMON_ZLIB_H

#include "common/scummsys.h"

namespace Common {

class SeekableReadStream;
class WriteStream;

#if defined(USE_ZLIB)

/**
 * Thin wrapper around zlib's uncompress() function. This wrapper makes
 * it possible to uncompress data in engines without being forced to link
 * them against zlib, thus simplifying the build system.
 *
 * @return true on success (i.e. Z_OK), false otherwise
 */
bool uncompress(byte *dst, unsigned long *dstLen, const byte *src, unsigned long srcLen);

#endif

/**
 * Take an arbitrary SeekableReadStream and wrap it in a custom stream which
 * provides transparent on-the-fly decompression. Assumes the data it
 * retrieves from the wrapped stream to be either uncompressed or in gzip
 * format. In the former case, the original stream is returned unmodified
 * (and in particular, not wrapped).
 *
 * It is safe to call this with a NULL parameter (in this case, NULL is
 * returned).
 */
SeekableReadStream *wrapCompressedReadStream(SeekableReadStream *toBeWrapped);

/**
 * Take an arbitrary WriteStream and wrap it in a custom stream which provides
 * transparent on-the-fly compression. The compressed data is written in the
 * gzip format, unless ZLIB support has been disabled, in which case the given
 * stream is returned unmodified (and in particular, not wrapped).
 *
 * It is safe to call this with a NULL parameter (in this case, NULL is
 * returned).
 */
WriteStream *wrapCompressedWriteStream(WriteStream *toBeWrapped);

}	// End of namespace Common

#endif
