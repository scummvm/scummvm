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

class AIFFHeader {
public:
	~AIFFHeader();
	static AIFFHeader *readAIFFHeader(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);
	RewindableAudioStream *makeAIFFStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);

	uint32 getFrameCount() const { return _frameCount; }
	uint32 getFrameRate() const { return _rate; }

private:
	uint16 _channels = 0;
	uint32 _frameCount = 0;
	uint16 _bitsPerSample = 0;
	uint32 _rate = 0;
	uint32 _codec = 0;
	Common::SeekableReadStream *_dataStream = nullptr;
};

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
