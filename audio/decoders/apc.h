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

#ifndef AUDIO_DECODERS_APC_H
#define AUDIO_DECODERS_APC_H

#include "common/scummsys.h"
#include "audio/audiostream.h"

namespace Common {
class SeekableReadStream;
} // End of namespace Common

namespace Audio {

class APCStream : public PacketizedAudioStream {
public:
	virtual bool init(Common::SeekableReadStream &header) = 0;
};

/**
 * Create a PacketizedAudioStream that decodes Cryo APC sound from stream
 *
 * @param header       The stream containing the header
 *                     queuePacket must be called after
 * @return             A new PacketizedAudioStream, or nullptr on error
 */
PacketizedAudioStream *makeAPCStream(Common::SeekableReadStream &header);

/**
 * Create a PacketizedAudioStream that decodes Cryo APC sound using predefined settings
 * This is used by HNM6 video decoder and shouldn't be called elsewhere.
 *
 * @param sampleRate   The sample rate of the stream
 * @param stereo       Whether the stream will be stereo
 * @return             A new APCStream
 */
APCStream *makeAPCStream(uint sampleRate, bool stereo);

} // End of namespace Audio

#endif

