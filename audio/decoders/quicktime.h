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

#ifndef AUDIO_QUICKTIME_H
#define AUDIO_QUICKTIME_H

#include "common/quicktime.h"
#include "common/scummsys.h"
#include "common/types.h"

namespace Common {
	class SeekableReadStream;
	class String;
}

namespace Audio {

class AudioStream;
class RewindableAudioStream;
class QueuingAudioStream;

class QuickTimeAudioDecoder : public Common::QuickTimeParser {
public:
	QuickTimeAudioDecoder();
	virtual ~QuickTimeAudioDecoder();

	/**
	 * Load a QuickTime audio file
	 * @param filename	the filename to load
	 */
	bool loadAudioFile(const Common::String &filename);

	/**
	 * Load a QuickTime audio file from a SeekableReadStream
	 * @param stream	the stream to load
	 */
	bool loadAudioStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle);

protected:
	struct AudioSampleDesc : public Common::QuickTimeParser::SampleDesc {
		AudioSampleDesc();

		uint16 channels;
		uint32 sampleRate;
		uint32 samplesPerFrame;
		uint32 bytesPerFrame;
	};

	// Common::QuickTimeParser API
	virtual Common::QuickTimeParser::SampleDesc *readSampleDesc(MOVStreamContext *st, uint32 format);

	AudioStream *createAudioStream(Common::SeekableReadStream *stream);
	bool checkAudioCodecSupport(uint32 tag);
	void init();

	void queueNextAudioChunk();
	uint32 getAudioChunkSampleCount(uint chunk);
	int8 _audioStreamIndex;
	uint _curAudioChunk;
	QueuingAudioStream *_audStream;

	void setAudioStreamPos(const Timestamp &where);
};
	
/**
 * Try to load a QuickTime sound file from the given file name and create a SeekableAudioStream
 * from that data.
 *
 * @param filename			the filename of the file from which to read the data
 * @return	a new SeekableAudioStream, or NULL, if an error occurred
 */
SeekableAudioStream *makeQuickTimeStream(const Common::String &filename);

/**
 * Try to load a QuickTime sound file from the given seekable stream and create a SeekableAudioStream
 * from that data.
 *
 * @param stream			the SeekableReadStream from which to read the data
 * @param disposeAfterUse	whether to delete the stream after use
 * @return	a new SeekableAudioStream, or NULL, if an error occurred
 */
SeekableAudioStream *makeQuickTimeStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

} // End of namespace Audio

#endif
