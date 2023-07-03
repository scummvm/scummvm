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
#include "audio/audiostream.h"

namespace Scumm {

#define HSND_SOUND_STOPPED        1
#define HSND_SOUND_ENDED          2
#define HSND_SOUND_TIMEOUT        3
#define HSND_TALKIE_SLOT          4
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
#define HSND_MAX_VOLUME           255
#define HSND_FIRST_SPOOLING_SOUND 4000

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


class ScummEngine_v60he;

class SoundHE : public Sound {
protected:
	ScummEngine_v60he *_vm;

	int _overrideFreq;
	Common::Mutex *_mutex;

	struct HESoundCallbackItem {
		int32 sound;
		int32 channel;
		int32 whatFrame;
	};
	HESoundCallbackItem _soundCallbackScripts[HSND_MAX_CALLBACK_SCRIPTS];

	struct HEMusic {
		int32 id;
		int32 offset;
		int32 size;

		char filename[128];
	};
	HEMusic *_heMusic;
	int16 _heMusicTracks;

	Audio::SoundHandle *_heSoundChannels;

public: // Used by createSound()
	struct {
		int sound;
		int codeOffs;
		byte *codeBuffer;
		int priority;
		int frequency;
		int timer;
		bool hasSoundTokens;
		int soundVars[HSND_MAX_SOUND_VARS];
		int age;

		void clearChannel() {
			sound = 0;
			codeOffs = 0;
			codeBuffer = nullptr;
			priority = 0;
			frequency = 0;
			timer = 0;
			hasSoundTokens = false;
			memset(soundVars, 0, sizeof(soundVars));
			age = 0;
		}

	} _heChannel[HSND_MAX_CHANNELS];

public:
	SoundHE(ScummEngine *parent, Audio::Mixer *mixer, Common::Mutex *mutex);
	~SoundHE() override;

	void addSoundToQueue(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0, int heFreq = 0, int hePan = 0, int heVol = 0) override;
	void addSoundToQueue2(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0, int heFreq = 0, int hePan = 0, int heVol = 0) override;
	void modifySound(int sound, int offset, int frequencyShift, int pan, int volume, int flags) override;

	int isSoundRunning(int sound) const override;
	void stopSound(int sound) override;
	void stopAllSounds() override;
	int findSoundChannel(int sound);
	void setupSound() override;

	bool getHEMusicDetails(int id, int &musicOffs, int &musicSize);
	int findFreeSoundChannel();
	bool isSoundCodeUsed(int sound);
	int getSoundPos(int sound);
	int getSoundVar(int sound, int var);
	void setSoundVar(int sound, int var, int val);
	void playHESound(int soundID, int heOffset, int heChannel, int heFlags, int heFreq, int hePan, int heVol);
	void handleSoundFrame();
	void unqueueSoundCallbackScripts();
	void checkSoundTimeouts();
	void digitalSoundCallback(int message, int channel);
	void queueSoundCallbackScript(int sound, int channel, int message);
	void runSoundCode();
	void processSoundOpcodes(int sound, byte *codePtr, int *soundVars);
	void setOverrideFreq(int freq);
	void setupHEMusicFile();
	void startHETalkSound(uint32 offset);
	void stopSoundChannel(int chan);

protected:
	void processSoundQueues() override;

private:
	int _heTalkOffset;
	bool _stopActorTalkingFlag = false;
	bool _inSoundCallbackFlag = false;
	int _soundCallbacksQueueSize = 0;
	int _soundAlreadyInQueueCount = 0;
	int _soundsDebugFrameCounter = 0;

	Audio::RewindableAudioStream *tryLoadAudioOverride(int soundID, int *duration = nullptr);
};


} // End of namespace Scumm

#endif
