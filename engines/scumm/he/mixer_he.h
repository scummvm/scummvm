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

#if !defined(SCUMM_HE_MIXER_HE_H)
#define SCUMM_HE_MIXER_HE_H

#include "scumm/he/sound_he.h"
#include "scumm/he/intern_he.h"
#include "scumm/resource.h"

#include "common/util.h"
#include "common/file.h"
#include "common/debug.h"
#include "common/memstream.h"

#include "audio/decoders/adpcm.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Audio {
class AudioStream;
class Mixer;
class QueuingAudioStream;
} // namespace Audio

namespace Scumm {

#define CHANNEL_EMPTY_FLAGS        0x00000000
#define CHANNEL_ACTIVE             0x00000001
#define CHANNEL_FINISHED           0x00000002
#define CHANNEL_LOOPING            0x00000004
#define CHANNEL_LAST_CHUNK         0x00000008
#define CHANNEL_SPOOLING           0x00000010
#define CHANNEL_SPOOL_READ         0x00000020
#define CHANNEL_SPOOL_CRITICAL     0x00000040
#define CHANNEL_CALLBACK_EARLY     0x00000080
#define CHANNEL_SOFT_REMIX         0x00000100

#define MILES_MAX_CHANNELS         8
#define MILES_CHUNK_SIZE           4096
#define MILES_MAX_QUEUED_STREAMS   16

struct HESoundModifiers;
class ScummEngine_v60he;

struct MilesModifiers {
	int frequencyShift;
	int pan;
	int volume;
};

class HEMilesChannel {
protected:
	struct MilesStream {
		Audio::QueuingAudioStream *streamObj = nullptr;
		Audio::SoundHandle streamHandle;
		bool loopFlag = false;
		Common::File *fileHandle = nullptr;
		uint32 dataLength = 0;
		uint32 curDataPos = 0;
		uint32 dataOffset = 0;
	};

public:
	MilesModifiers _modifiers;
	MilesStream _stream;
	int _baseFrequency;
	Audio::SoundHandle _audioHandle;
	bool _audioHandleActive = false;
	uint32 _lastPlayPosition;
	uint32 _playFlags;
	int _dataOffset;
	int _globType;
	int _globNum;

	uint16 _blockAlign = 0;
	uint16 _numChannels = 1;
	uint16 _bitsPerSample = 8;
	uint16 _dataFormat = 1;

	HEMilesChannel() {
		clearChannelData();
	}

	~HEMilesChannel() {
		clearChannelData();
	}

	void startSpoolingChannel(const char *filename, long offset, int flags, HESoundModifiers modifiers, Audio::Mixer *mixer);
	void clearChannelData();
	void closeFileHandle();
	void serviceStream();
	byte getOutputFlags();
};

class HEMixer {
protected:
	ScummEngine *_vm;
	Audio::Mixer *_mixer;
	bool _useMilesSoundSystem = false;
	bool _mixerPaused = false;

	HEMilesChannel _milesChannels[MILES_MAX_CHANNELS];

public:
	HEMixer(Audio::Mixer *mixer, ScummEngine_v60he *vm, bool useMiles);
	~HEMixer();

	void *getMilesSoundSystemObject();
	bool initSoftMixerSubSystem();
	void deinitSoftMixerSubSystem();
	void endNeglectProcess();
	void startLongNeglectProcess();
	bool forceMusicBufferFill();
	bool isMixerDisabled();
	bool stopChannel(int channel);
	void stopAllChannels();
	bool changeChannelVolume(int channel, int volume, bool soft);
	void softRemixAllChannels();
	void premixUntilCritical();
	bool pauseMixerSubSystem(bool paused);
	void feedMixer();
	void serviceAllStreams();
	int getChannelCurrentPosition(int channel);

	bool startChannelNew(
		int channel, int globType, int globNum, uint32 soundData, uint32 offset,
		int sampleLen, int frequency, int bitsPerSample, int sampleChannels,
		const HESoundModifiers &modifiers, int callbackId, int32 flags, ...);

	bool startChannel(
		int channel, int globType, int globNum, uint32 sampleDataOffset,
		int sampleLen, int frequency, int volume, int callbackId, int32 flags, ...);

	bool startSpoolingChannel(
		int channel, Common::File &spoolingFile, int sampleLen, int frequency,
		int volume, int callbackID, int32 flags, ...);

	bool isMilesActive();
	bool changeChannelVolume(int channel, int newVolume, int soft_flag);
	void milesStartSpoolingChannel(int channel, const char *filename, long offset, int flags, HESoundModifiers modifiers);
	int  hsFindSoundQueue(int sound);
	bool mixerStartChannel(
		int channel, int globType, int globNum, uint32 sampleDataOffset,
		int sampleLen, int frequency, int volume, int callbackID, uint32 flags, ...);

	bool milesStartChannel(
		int channel, int globType, int globNum, uint32 sound_data, uint32 offset,
		int sampleLen, int bitsPerSample, int sampleChannels,
		int frequency, HESoundModifiers modifiers, int callbackID, uint32 flags, ...);

	bool milesStopChannel(int channel);
	void milesStopAllSounds();
	void milesModifySound(int channel, int offset, HESoundModifiers modifiers, int flags);
	void milesStopAndCallback(int channel, int messageId);
	void milesRestoreChannel(int channel);
	void milesFeedMixer();
	bool milesPauseMixerSubSystem(bool paused);
	byte *milesGetAudioDataFromResource(int globType, int globNum, uint32 dataOffset, uint16 &compType, uint16 &blockAlign);
};

} // End of namespace Scumm

#endif
