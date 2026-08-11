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
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#ifndef SCUMM_HE_MIXER_HE_H
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
#include "audio/decoders/wave.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/flac.h"

namespace Scumm {

#define CHANNEL_EMPTY_FLAGS        (0 << 0)
#define CHANNEL_ACTIVE             (1 << 0)
#define CHANNEL_FINISHED           (1 << 1)
#define CHANNEL_LOOPING            (1 << 2)
#define CHANNEL_LAST_CHUNK         (1 << 3)
#define CHANNEL_SPOOLING           (1 << 4)
#define CHANNEL_CALLBACK_EARLY     (1 << 7)
#define CHANNEL_SOFT_REMIX         (1 << 8)

#define MIXER_MAX_CHANNELS         8
#define MIXER_PCM_CHUNK_SIZE       4096
#define MIXER_MAX_QUEUED_STREAMS   8
#define MIXER_DEFAULT_SAMPLE_RATE  11025
#define MIXER_SPOOL_CHUNK_SIZE     (8 * 1024)

#define MILES_MAX_CHANNELS                    8
#define MILES_PCM_CHUNK_SIZE                  4096u
#define MILES_IMA_ADPCM_PER_FRAME_CHUNKS_NUM  4u
#define MILES_MAX_QUEUED_STREAMS              16

struct HESoundModifiers;
struct HESpoolingMusicItem;
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

	bool _isUsingStreamOverride = false;

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
	int _pauseCount = 0;
	Common::HashMap<int32, int32> _offsetsToSongId;

	HEMilesChannel _milesChannels[MILES_MAX_CHANNELS];

	struct HEMixerChannel {
		Audio::SoundHandle handle;
		Audio::QueuingAudioStream *stream = nullptr;
		Common::File *fileHandle = nullptr;
		int number = 0;
		int volume = 0;
		int frequency = 0;
		int globType = 0;
		int globNum = 0;
		int callbackID = 0;
		int endSampleAdjustment = 0;
		uint32 lastReadPosition = 0;
		uint32 initialSpoolingFileOffset = 0;
		uint32 sampleLen = 0;
		uint32 dataOffset = 0;
		uint32 flags = 0;
		bool callbackOnNextFrame = false;
		bool isUsingStreamOverride = false;
		byte *residualData = nullptr; // For early callbacks
	};

	HEMixerChannel _mixerChannels[MIXER_MAX_CHANNELS];


public:
	HEMixer(Audio::Mixer *mixer, ScummEngine_v60he *vm, bool useMiles);
	~HEMixer();

	bool initSoftMixerSubSystem();
	void deinitSoftMixerSubSystem();
	bool stopChannel(int channel);
	void stopAllChannels();
	bool pauseMixerSubSystem(bool paused);
	void feedMixer();
	bool changeChannelVolume(int channel, int volume, bool soft);
	bool startChannelNew(
		int channel, int globType, int globNum, uint32 soundData, uint32 offset,
		int sampleLen, int frequency, int bitsPerSample, int sampleChannels,
		const HESoundModifiers &modifiers, int callbackId, int32 flags, ...);
	bool startChannel(
		int channel, int globType, int globNum, uint32 sampleDataOffset,
		int sampleLen, int frequency, int volume, int callbackId, int32 flags, ...);
	bool startSpoolingChannel(
		int channel, int song, Common::File &spoolingFile, int sampleLen, int frequency,
		int volume, int callbackID, int32 flags);

	bool audioOverrideExists(int globNum, bool justGetInfo,
		int *duration = nullptr, Audio::SeekableAudioStream **outStream = nullptr);

	void setSpoolingSongsTable(HESpoolingMusicItem *heSpoolingMusicTable, int32 tableSize);
	int32 matchOffsetToSongId(int32 offset);

	/* --- MILES MIXER CODE --- */
	bool isMilesActive();
	void milesStartSpoolingChannel(int channel, const char *filename, long offset, int flags, HESoundModifiers modifiers);
	bool milesStartChannel(
		int channel, int globType, int globNum, uint32 sound_data, uint32 offset,
		int sampleLen, int bitsPerSample, int sampleChannels,
		int frequency, HESoundModifiers modifiers, int callbackID, uint32 flags, ...);
	bool milesStopChannel(int channel);
	void milesStopAllSounds();
	void milesModifySound(int channel, int offset, HESoundModifiers modifiers, int flags);
	void milesStopAndCallback(int channel, int messageId);
	void milesFeedMixer();
	void milesServiceAllStreams();
	bool milesPauseMixerSubSystem(bool paused);
	byte *milesGetAudioDataFromResource(int globType, int globNum, uint32 dataOffset, uint16 &compType, uint16 &blockAlign, uint32 &dataSize);

	/* --- SOFTWARE MIXER CODE --- */
	bool mixerInitMyMixerSubSystem();
	void mixerFeedMixer();
	bool mixerIsMixerDisabled();
	bool mixerStopChannel(int channel);
	bool mixerStopAllSounds();
	bool mixerChangeChannelVolume(int channel, int volume, bool soft);
	bool mixerPauseMixerSubSystem(bool paused);
	bool mixerStartChannel(
		int channel, int globType, int globNum, uint32 sampleDataOffset,
		int sampleLen, int frequency, int volume, int callbackID, uint32 flags, ...);
	bool mixerStartSpoolingChannel(
		int channel, int song, Common::File &sampleFileIOHandle, int sampleLen, int frequency,
		int volume, int callbackID, uint32 flags);
	byte mixerGetOutputFlags(bool is3DOMusic = false);
};

} // End of namespace Scumm

#endif
