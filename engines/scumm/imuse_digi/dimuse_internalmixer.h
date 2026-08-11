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

#if !defined(SCUMM_IMUSE_DIGI_MIXER_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_MIXER_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "common/serializer.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/music.h"
#include "scumm/sound.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"

namespace Audio {
class AudioStream;
class Mixer;
class QueuingAudioStream;
}

namespace Scumm {

class IMuseDigiInternalMixer {

private:
	int32 *_amp8Table = nullptr;
	int32 *_amp12Table = nullptr;
	int32 *_softLMID = nullptr;
	int32 *_softLTable = nullptr;

	uint8 *_mixBuf = nullptr;

	Audio::Mixer *_mixer;
	Audio::SoundHandle _channelHandle;
	int _mixBufSize = 0;
	int _radioChatter = 0;
	int _outWordSize = 0;
	int _outChannelCount = 0;
	int _sampleRate = 0;
	int _stereoReverseFlag = 0;
	bool _isEarlyDiMUSE = false;
	bool _lowLatencyMode = false;

	void mixBits8Mono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable, bool ftIs11025Hz);
	void mixBits12Mono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable);
	void mixBits16Mono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable);

	void mixBits8ConvertToMono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable);
	void mixBits12ConvertToMono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable);
	void mixBits16ConvertToMono(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable);

	void mixBits8ConvertToStereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *leftAmpTable, int32 *rightAmpTable, bool ftIs11025Hz);
	void mixBits12ConvertToStereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *leftAmpTable, int32 *rightAmpTable);
	void mixBits16ConvertToStereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *leftAmpTable, int32 *rightAmpTable);

	void mixBits8Stereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable);
	void mixBits12Stereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable);
	void mixBits16Stereo(uint8 *srcBuf, int32 inFrameCount, int feedSize, int32 mixBufStartIndex, int32 *ampTable);

public:
	IMuseDigiInternalMixer(Audio::Mixer *mixer, int sampleRate, bool isEarlyDiMUSE, bool lowLatencyMode = false);
	~IMuseDigiInternalMixer();
	int  init(int bytesPerSample, int numChannels, uint8 *mixBuf, int mixBufSize, int sizeSampleKB, int mixChannelsNum);
	void setRadioChatter();
	void clearRadioChatter();
	int  clearMixerBuffer();

	void mix(uint8 *srcBuf, int32 inFrameCount, int wordSize, int channelCount, int feedSize, int32 mixBufStartIndex, int volume, int pan, bool ftIs11025Hz);
	int  loop(uint8 **destBuffer, int len);
	Audio::QueuingAudioStream *_stream;

	// For low latency audio
	void setCurrentMixerBuffer(uint8 *newBuf);
	void endStream(int idx);
	Audio::QueuingAudioStream *getStream(int idx);

	Audio::QueuingAudioStream *_separateStreams[DIMUSE_MAX_TRACKS];
	Audio::SoundHandle _separateChannelHandles[DIMUSE_MAX_TRACKS];
};

} // End of namespace Scumm

#endif
