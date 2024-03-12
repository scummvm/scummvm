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

/**
 * @file
 * Sound decoder used in engines:
 *  - sludge
 */

#ifndef AUDIO_UNIVERSALTRACKER_H
#define AUDIO_UNIVERSALTRACKER_H

#include "common/scummsys.h"
#include "common/types.h"

#if defined(USE_MIKMOD) && defined(USE_OPENMPT)
#error "MikMod and OpenMPT are mutually exclusive"
#endif

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class RewindableAudioStream;
/**
 * Create a new AudioStream from the Impulse Tracker data in the given stream.
 *
 * @param stream            the SeekableReadStream from which to read the Ogg Vorbis data
 * @param disposeAfterUse   whether to delete the stream after use
 * @return  a new AudioStream, or NULL, if an error occurred
 */
RewindableAudioStream *makeUniversalTrackerStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate = 48000);

} // End of namespace Audio

#endif // #ifndef AUDIO_UNIVERSALTRACKER_H
