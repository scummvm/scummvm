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
 *  - sherlock (3DO version of Serrated Scalpel)
 */

#ifndef AUDIO_3DO_SDX2_H
#define AUDIO_3DO_SDX2_H

#include "common/scummsys.h"
#include "common/types.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class SeekableAudioStream;

struct audio_3DO_ADP4_PersistentSpace {
	int16 lastSample;
	int16 stepIndex;
};

struct audio_3DO_SDX2_PersistentSpace {
	int16 lastSample1;
	int16 lastSample2;
};


/**
 * Try to decode 3DO ADP4 data from the given seekable stream and create a SeekableAudioStream
 * from that data.
 *
 * @param stream			the SeekableReadStream from which to read the 3DO ADP4 data
 * @size					how many bytes to read from stream
 * @sampleRate				sample rate
 * @audioFlags				flags, that specify the type of output
 * @param					disposeAfterUse	whether to delete the stream after use
 * @return					a new SeekableAudioStream, or NULL, if an error occurred
 */
SeekableAudioStream *make3DO_ADP4Stream(
	Common::SeekableReadStream *stream,
	uint32 size,
	uint16 sampleRate,
	byte audioFlags,
	DisposeAfterUse::Flag disposeAfterUse,
	audio_3DO_ADP4_PersistentSpace *persistentSpace = NULL
);

/**
 * Try to decode 3DO SDX2 data from the given seekable stream and create a SeekableAudioStream
 * from that data.
 *
 * @param stream			the SeekableReadStream from which to read the 3DO SDX2 data
 * @size					how many bytes to read from stream
 * @sampleRate				sample rate
 * @audioFlags				flags, that specify the type of output
 * @param					disposeAfterUse	whether to delete the stream after use
 * @return					a new SeekableAudioStream, or NULL, if an error occurred
 */
SeekableAudioStream *make3DO_SDX2Stream(
	Common::SeekableReadStream *stream,
	uint32 size,
	uint16 sampleRate,
	byte audioFlags,
	DisposeAfterUse::Flag disposeAfterUse,
	audio_3DO_SDX2_PersistentSpace *persistentSpace = NULL
);

} // End of namespace Audio

#endif
