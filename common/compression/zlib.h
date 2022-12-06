/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_ZLIB_H
#define COMMON_ZLIB_H

#include "common/scummsys.h"

namespace Common {

/**
 * @defgroup common_zlib zlib
 * @ingroup common
 *
 * @brief API for zlib operations.
 *
 * @{
 */

class WriteStream;

/**
 * Take an arbitrary WriteStream and wrap it in a custom stream which provides
 * transparent on-the-fly compression. The compressed data is written in the
 * gzip format, unless ZLIB support has been disabled, in which case the given
 * stream is returned unmodified (and in particular, not wrapped).
 * The created stream also becomes responsible for freeing the passed stream.
 *
 * It is safe to call this with a NULL parameter (in this case, NULL is
 * returned).
 */
WriteStream *wrapCompressedWriteStream(WriteStream *toBeWrapped);

/** @} */

} // End of namespace Common

#endif
