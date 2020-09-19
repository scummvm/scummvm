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
 */

#ifndef COMMON_BUFFEREDSTREAM_H
#define COMMON_BUFFEREDSTREAM_H

#include "common/stream.h"
#include "common/types.h"

namespace Common {

/**
 * @defgroup common_buffstream Buffered stream
 * @ingroup common
 *
 * @brief  API for implementing a buffered stream.
 *
 * @{
 */

/**
 * Take an arbitrary ReadStream and wrap it in a custom stream that
 * transparently provides buffering.
 * You can specify how big the buffer should be, and whether the wrapped
 * stream should be disposed when the wrapper is disposed.
 *
 * It is safe to call this with a NULL parameter (in this case, NULL is
 * returned).
 *
 * @param parentStream        The ReadStream to wrap in a custom stream.
 * @param bufSize             Size of the buffer.
 * @param disposeParentStream Flag indicating whether to dispose of the wrapped stream.
 */
ReadStream *wrapBufferedReadStream(ReadStream *parentStream, uint32 bufSize, DisposeAfterUse::Flag disposeParentStream);

/**
 * Take an arbitrary SeekableReadStream and wrap it in a custom stream that
 * transparently provides buffering.
 * You can specify how big the buffer should be, and whether the wrapped
 * stream should be disposed when the wrapper is disposed.
 *
 * It is safe to call this with a NULL parameter (in this case, NULL is
 * returned).
 *
 * @param parentStream        The SeekableReadStream to wrap in a custom stream.
 * @param bufSize             Size of the buffer.
 * @param disposeParentStream Flag indicating whether to dispose of the wrapped stream.
 */
SeekableReadStream *wrapBufferedSeekableReadStream(SeekableReadStream *parentStream, uint32 bufSize, DisposeAfterUse::Flag disposeParentStream);

/**
 * Take an arbitrary WriteStream and wrap it in a custom stream that
 * transparently provides buffering.
 * You can specify how big the buffer should be. Currently, the
 * parent stream is \em always disposed when the wrapper is disposed.
 *
 * It is safe to call this with a NULL parameter (in this case, NULL is
 * returned).
 *
 * @param parentStream        The WriteStream to wrap in a custom stream.
 * @param bufSize             Size of the buffer.
 */

WriteStream *wrapBufferedWriteStream(WriteStream *parentStream, uint32 bufSize);

/** @} */

} // End of namespace Common

#endif
