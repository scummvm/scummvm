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

#ifndef SCUMM_HE_SOUND_HE_H
#define SCUMM_HE_SOUND_HE_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "scumm/sound.h"
#include "scumm/he/mixer_he.h"
#include "audio/audiostream.h"

namespace Scumm {

#define HSND_DYN_SOUND_CHAN      -1
#define HSND_TALKIE_SLOT          1
#define HSND_SOUND_STOPPED        1
#define HSND_SOUND_ENDED          2
#define HSND_SOUND_TIMEOUT        3
#define HSND_TIMER_SLOT           4
#define HSND_MAX_CALLBACK_SCRIPTS 20
#define HSND_MAX_CHANNELS         8
#define HSND_CHANNEL_0            10000
#define HSND_CHANNEL_1            10001
#define HSND_CHANNEL_2            10002
#define HSND_CHANNEL_3            10003
#define HSND_CHANNEL_4            10004
#define HSND_CHANNEL_5            10005
#define HSND_CHANNEL_6            10006
#define HSND_CHANNEL_7            10007
#define HSND_MAX_SOUND_VARS       26
#define HSND_DEFAULT_FREQUENCY    11025
#define HSND_BASE_FREQ_FACTOR     1024
#define HSND_SOUND_FREQ_BASE      1024
#define HSND_MAX_VOLUME           255
#define HSND_SOUND_PAN_LEFT       0
#define HSND_SOUND_PAN_CENTER     64
#define HSND_SOUND_PAN_RIGHT      127
#define HSND_FIRST_SPOOLING_SOUND 4000
#define HSND_MAX_FREQ_RATIO       4

#define HSND_MAX_SPOOLING_CODE_SIZE 16384

#define HSND_SBNG_TYPE_ALL 0xF8
#define HSND_SBNG_DATA_ALL 0x07

#define HSND_SBNG_MAGIC_NUMBER 0x2000
#define HSND_SBNG_MAGIC_MASK   0xF000

#define HSND_SBNG_END      0x00
#define HSND_SBNG_FACE     0x10
#define HSND_SBNG_SET_SET  0x20
#define HSND_SBNG_SET_ADD  0x30
#define HSND_SBNG_SET_SUB  0x38
#define HSND_SBNG_SET_MUL  0x40
#define HSND_SBNG_SET_DIV  0x50
#define HSND_SBNG_SET_INC  0x60
#define HSND_SBNG_SET_DEC  0x68
#define HSND_SBNG_VARORVAL 0x03
#define HSND_SBNG_VARVAL   0x02

#define HSND_SNDVAR_TOKENS 26

#define XSH2_FLAG_HAS_PRIORITY 0x01

#define WAVE_FORMAT_PCM       1
#define WAVE_FORMAT_IMA_ADPCM 17
#define WAVE_RIFF_HEADER_LEN  44

#define HSND_RES_OFFSET_ID1              0 // uint32, DIGI or MIDI header
#define HSND_RES_OFFSET_LEN1             4 // uint32
#define HSND_RES_OFFSET_ID               8 // uint32, HSHD header
#define HSND_RES_OFFSET_LEN2            12 // uint32
#define HSND_RES_OFFSET_COMPRESSED      16 // uint8, header data
#define HSND_RES_OFFSET_FORMATTED       17 // uint8
#define HSND_RES_OFFSET_KILL_PRIO       18 // uint8
#define HSND_RES_OFFSET_DEAD_PRIO       19 // uint8
#define HSND_RES_OFFSET_LEFT_VOLUME     20 // uint8
#define HSND_RES_OFFSET_RIGHT_VOLUME    21 // uint8
#define HSND_RES_OFFSET_FREQUENCY       22 // uint16
#define HSND_RES_OFFSET_MONDO_OFFSET    24 // uint32
#define HSND_RES_OFFSET_BITS_PER_SAMPLE 28 // uint8
#define HSND_RES_OFFSET_SAMPLE_CHANNELS 29 // uint8
#define HSND_RES_OFFSET_UNUSED3         30 // uint8
#define HSND_RES_OFFSET_UNUSED4         31 // uint8
#define HSND_RES_OFFSET_ID3             32 // uint32, SDAT header
#define HSND_RES_OFFSET_LEN3            36 // uint32
#define HSND_RES_OFFSET_SOUND_DATA      40

// Used both in SoundHE and HEMixer
struct HESoundModifiers {
	HESoundModifiers(int mFrequencyShift, int mPan, int mVolume) {
		assert(mFrequencyShift >= HSND_SOUND_FREQ_BASE / HSND_MAX_FREQ_RATIO);
		assert(mFrequencyShift <= HSND_SOUND_FREQ_BASE * HSND_MAX_FREQ_RATIO);
		assert(mPan >= HSND_SOUND_PAN_LEFT && mPan <= HSND_SOUND_PAN_RIGHT);
		assert(mVolume >= 0 && mVolume <= HSND_MAX_VOLUME);
		frequencyShift = mFrequencyShift;
		pan = mPan;
		volume = mVolume;
	}

	HESoundModifiers() {
		frequencyShift = HSND_SOUND_FREQ_BASE;
		pan = HSND_SOUND_PAN_CENTER;
		volume = HSND_MAX_VOLUME;
	}

	int frequencyShift;
	int pan;
	int volume;
};

class ScummEngine_v60he;
class HEMixer;

class SoundHE : public Sound {
protected:
	ScummEngine_v60he *_vm;

	int _overrideFreq;
	Common::Mutex *_mutex;
	HEMixer *_heMixer;

	struct HESoundCallbackItem {
		int32 sound;
		int32 channel;
		int32 whatFrame;
	};
	HESoundCallbackItem _soundCallbackScripts[HSND_MAX_CALLBACK_SCRIPTS];

	struct HESpoolingMusicItem {
		int32 song;
		int32 offset;
		int32 size;

		char filename[128];
	};
	HESpoolingMusicItem *_heSpoolingMusicTable;

	struct PCMWaveFormat {
		uint16 wFormatTag;
		uint16 wChannels;
		uint32 dwSamplesPerSec;
		uint32 dwAvgBytesPerSec;
		uint16 wBlockAlign;
		uint16 wBitsPerSample;
	};

	int32 _heSpoolingMusicCount;

	Common::File _heSpoolingMusicFile;
	byte _heSpoolingCodeBuffer[HSND_MAX_SPOOLING_CODE_SIZE];

public: // Used by createSound()
	struct {
		int sound;
		int codeOffset;
		byte *codeBuffer;
		int priority;
		int frequency;
		int timeout;
		bool hasSoundTokens;
		int soundVars[HSND_MAX_SOUND_VARS];
		int age;

		void clearChannel() {
			sound = 0;
			codeOffset = 0;
			codeBuffer = nullptr;
			priority = 0;
			frequency = 0;
			timeout = 0;
			hasSoundTokens = false;
			memset(soundVars, 0, sizeof(soundVars));
			age = 0;
		}

	} _heChannel[HSND_MAX_CHANNELS];

	bool _useMilesSoundSystem = false;

	// Used throughout script functions
	int32 _createSndId;
	int32 _createSndLastAppend;
	int32 _createSndLastPos;
	int32 _baseSndSize;

public:
	SoundHE(ScummEngine *parent, Audio::Mixer *mixer, Common::Mutex *mutex);
	~SoundHE() override;

	void startSound(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0, int heFreq = 0, int hePan = 0, int heVol = 0) override;
	void addSoundToQueue(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0, int heFreq = 0, int hePan = 0, int heVol = 0) override;
	void modifySound(int sound, int offset, int frequencyShift, int pan, int volume, int flags) override;

	int isSoundRunning(int sound) const override;
	bool isSoundInUse(int sound) const override;
	void stopSound(int sound) override;
	void stopAllSounds() override;
	int hsFindSoundChannel(int sound) const;
	void setupSound() override;

	bool getHEMusicDetails(int id, int &musicOffs, int &musicSize);
	int getNextDynamicChannel();
	bool isSoundCodeUsed(int sound);
	int getSoundPosition(int sound);
	int getSoundVar(int sound, int var);
	void setSoundVar(int sound, int var, int val);
	void triggerSound(int soundId, int heOffset, int heChannel, int heFlags, HESoundModifiers modifiers);
	void triggerSpoolingSound(int soundId, int heOffset, int heChannel, int heFlags, HESoundModifiers modifiers);
	void triggerDigitalSound(int soundId, int heOffset, int heChannel, int heFlags);
	void triggerMidiSound(int soundId, int heOffset);
	void triggerRIFFSound(int soundId, int heOffset, int heChannel, int heFlags, HESoundModifiers modifiers);
	void triggerXSOUSound(int soundId, int heOffset, int heChannel, int heFlags);

	void hsStartDigitalSound(int sound, int offset, byte *addr, int sound_data, int globType, int globNum,
							 int sampleCount, int frequency, int channel, int priority, int soundCode,
							 int flags, int bitsPerSample, int soundChannelCount, HESoundModifiers modifiers);
	void hsStopDigitalSound(int sound);

	void handleSoundFrame();
	void feedMixer();
	void unqueueSoundCallbackScripts();
	void checkSoundTimeouts();
	void digitalSoundCallback(int message, int channel);
	void queueSoundCallbackScript(int sound, int channel, int message);
	void runSoundCode();
	void processSoundOpcodes(int sound, byte *codePtr, int *soundVars);
	void setOverrideFreq(int freq);
	void setupHEMusicFile();
	void playVoice(uint32 offset, uint32 length);
	void playVoiceFile(char *filename);
	void stopDigitalSound(int sound);
	void createSound(int snd1id, int snd2id);
	int getChannelPosition(int channel);

	byte *findWavBlock(uint32 tag, const byte *block);

protected:
	void processSoundQueues() override;

private:
	int _heTalkOffset;
	bool _stopActorTalkingFlag = false;
	bool _inSoundCallbackFlag = false;
	int _soundCallbacksQueueSize = 0;
	int _soundAlreadyInQueueCount = 0;
	int _inUnqueueCallbackScripts = 0;
	int _soundsDebugFrameCounter = 0;
	int _scummOverrideFrequency = 0;


	Audio::RewindableAudioStream *tryLoadAudioOverride(int soundID, int *duration = nullptr);
};


} // End of namespace Scumm

#endif
