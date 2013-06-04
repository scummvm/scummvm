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

#ifndef ZVISION_ZORK_RAW_H
#define ZVISION_ZORK_RAW_H

#include "common/scummsys.h"
#include "common/types.h"

#include "common/list.h"


namespace Common {
class SeekableReadStream;
}

namespace ZVision {

class SeekableAudioStream;

/**
 * Creates an audio stream, which plays from the given buffer.
 *
 * @param buffer Buffer to play from.
 * @param size   Size of the buffer in bytes.
 * @param rate   Rate of the sound data.
 * @param disposeAfterUse Whether to free the buffer after use (with free!).
 * @return The new SeekableAudioStream (or 0 on failure).
 */
Audio::SeekableAudioStream *makeRawZorkStream(const byte *buffer, uint32 size,
                                   int rate,
                                   DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

/**
 * Creates an audio stream, which plays from the given stream.
 *
 * @param stream Stream object to play from.
 * @param rate   Rate of the sound data.
 * @param disposeAfterUse Whether to delete the stream after use.
 * @return The new SeekableAudioStream (or 0 on failure).
 */
Audio::SeekableAudioStream *makeRawZorkStream(Common::SeekableReadStream *stream,
                                   int rate,
                                   DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

} // End of namespace Audio

#endif
