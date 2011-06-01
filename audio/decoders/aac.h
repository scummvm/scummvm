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
 * $URL$
 * $Id$
 *
 */

/**
 * @file
 * Sound decoder used in engines:
 *  - groovie
 */

#ifndef SOUND_AAC_H
#define SOUND_AAC_H

#include "common/scummsys.h"
#include "common/types.h"

#ifdef USE_FAAD

namespace Common {
	class SeekableReadStream;
}

namespace Audio {

class AudioStream;

/**
 * Create a new AudioStream from the AAC data in the given stream.
 *
 * @param stream			the SeekableReadStream from which to read the AAC data
 * @param disposeStream     whether to delete the stream after use
 * @param extraData         the SeekableReadStream from which to read the AAC extra data
 * @param disposeExtraData  whether to delete the extra data stream after use
 * @return	a new AudioStream, or NULL, if an error occurred
 */
AudioStream *makeAACStream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeStream,
	Common::SeekableReadStream *extraData,
	DisposeAfterUse::Flag disposeExtraData = DisposeAfterUse::NO);

} // End of namespace Audio

#endif // #ifdef USE_FAAD
#endif // #ifndef SOUND_AAC_H
