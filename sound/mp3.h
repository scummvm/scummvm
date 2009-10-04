/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * Sound decoder used in engines:
 *  - agos
 *  - kyra
 *  - m4
 *  - queen
 *  - saga
 *  - scumm
 *  - sword1
 *  - sword2
 *  - touche
 *  - tucker
 */

#ifndef SOUND_MP3_H
#define SOUND_MP3_H

#include "common/sys.h"

#ifdef USE_MAD

namespace Common {
	class SeekableReadStream;
}

namespace Audio {

class AudioStream;

/**
 * Create a new AudioStream from the MP3 data in the given stream.
 * Allows for looping (which is why we require a SeekableReadStream),
 * and specifying only a portion of the data to be played, based
 * on time offsets.
 *
 * @param stream			the SeekableReadStream from which to read the MP3 data
 * @param disposeAfterUse	whether to delete the stream after use
 * @param startTime			the (optional) time offset in milliseconds from which to start playback
 * @param duration			the (optional) time in milliseconds specifying how long to play
 * @param numLoops			how often the data shall be looped (0 = infinite)
 * @return	a new AudioStream, or NULL, if an error occured
 */
AudioStream *makeMP3Stream(
	Common::SeekableReadStream *stream,
	bool disposeAfterUse,
	uint32 startTime = 0,
	uint32 duration = 0,
	uint numLoops = 1);

} // End of namespace Audio

#endif // #ifdef USE_MAD
#endif // #ifndef SOUND_MP3_H
