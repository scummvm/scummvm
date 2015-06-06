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

/**
 * @file
 * Sound decoder used in engines:
 *  - bbvs
 *  - pegasus
 *  - saga
 *  - sci
 *  - sword1
 */

#ifndef AUDIO_AIFF_H
#define AUDIO_AIFF_H

#include "common/scummsys.h"
#include "common/types.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class RewindableAudioStream;

/**
 * Try to load an AIFF from the given seekable stream and create an AudioStream
 * from that data.
 *
 * @param stream			the SeekableReadStream from which to read the AIFF data
 * @param disposeAfterUse	whether to delete the stream after use
 * @return	a new SeekableAudioStream, or NULL, if an error occurred
 */
RewindableAudioStream *makeAIFFStream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse);

} // End of namespace Audio

#endif
