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

#ifndef SOUND_SHORTEN_H
#define SOUND_SHORTEN_H

#include "common/scummsys.h"

namespace Audio {
class AudioStream;
}

namespace Common {
class ReadStream;
}

namespace Saga2 {

/**
 * Try to load a Shorten file from the given stream. Returns true if
 * successful. In that case, the stream's seek position will be set to the
 * start of the audio data, and size, rate and flags contain information
 * necessary for playback.
 */
byte *loadShortenFromStream(Common::ReadStream &stream, int &size, int &rate, byte &flags);

/**
 * Try to load a Shorten file from the given stream and create an AudioStream
 * from that data.
 *
 * This function uses loadShortenFromStream() internally.
 */
Audio::AudioStream *makeShortenStream(Common::SeekableReadStream &stream);

} // End of namespace Saga2

#endif
