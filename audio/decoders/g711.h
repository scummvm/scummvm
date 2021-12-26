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
 *  - trecision
 */

#ifndef AUDIO_G711_H
#define AUDIO_G711_H

#include "common/scummsys.h"
#include "common/types.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class SeekableAudioStream;

/**
 * Takes an input stream containing G711 A-law compressed sound data and creates
 * a SeekableAudioStream from that.
 *
 * @param stream            the SeekableReadStream from which to read the PCM data
 * @param disposeAfterUse   whether to delete the stream after use
 * @param rate              the sampling rate
 * @param channels          the number of channels
 * @return   a new SeekableAudioStream, or NULL, if an error occurred
 */
SeekableAudioStream *makeALawStream(
    Common::SeekableReadStream *stream,
    DisposeAfterUse::Flag disposeAfterUse,
    int rate,
    int channels);

/**
 * Takes an input stream containing G711 μ-law compressed sound data and creates
 * a SeekableAudioStream from that.
 *
 * @param stream            the SeekableReadStream from which to read the PCM data
 * @param disposeAfterUse   whether to delete the stream after use
 * @param rate              the sampling rate
 * @param channels          the number of channels
 * @return   a new SeekableAudioStream, or NULL, if an error occurred
 */
SeekableAudioStream *makeMuLawStream(
    Common::SeekableReadStream *stream,
    DisposeAfterUse::Flag disposeAfterUse,
    int rate,
    int channels);

} // End of namespace Audio

#endif
