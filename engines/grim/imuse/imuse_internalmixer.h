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

#ifndef GRIM_IMUSE_INTERNALMIXER_H
#define GRIM_IMUSE_INTERNALMIXER_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "common/serializer.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "engines/grim/imuse/imuse_engine.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"

namespace Audio {
class AudioStream;
class Mixer;
class QueuingAudioStream;
}

namespace Grim {

class IMuseInternalMixer {

private:
	uint8 _mixerTable[17 * 17];
	int16 *_softLTable = nullptr;

	int32 *_mixBuf = nullptr;

	Audio::Mixer *_mixer;
	Audio::SoundHandle _channelHandle;
	int _mixBufSize = 0;
	int _outChannelCount = 0;
	int _sampleRate = 0;
	bool _lowLatencyMode = false;

	void mix16(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume);
	void mix16Swap(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume);
	void mix16ToStereo(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp);
	void mix16ToStereoSwap(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp);

public:
	IMuseInternalMixer(Audio::Mixer *mixer, int sampleRate, bool lowLatencyMode = false);
	~IMuseInternalMixer();
	int  init(int numChannels, int mixBufSize, int mixChannelsNum);
	int  terminate();
	int  clearMixerBuffer();

	void mix(uint8 *srcBuf, int32 inFrameCount, int wordSize, int channelCount, int swapEndiannessFlag, int feedSize, int32 mixBufStartIndex, int volume, int pan);
	int  loop(uint8 **destBuffer, int len);
	Audio::QueuingAudioStream *_stream;

	// For low latency audio
	void endStream(int idx);
	Audio::QueuingAudioStream *getStream(int idx);

	Audio::QueuingAudioStream *_separateStreams[IMUSE_MAX_TRACKS];
	Audio::SoundHandle _separateChannelHandles[IMUSE_MAX_TRACKS];
};

} // End of namespace Grim

#endif
