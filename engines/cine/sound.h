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

#ifndef CINE_SOUND_H_
#define CINE_SOUND_H_

#include "common/util.h"
#include "common/mutex.h"
#include "audio/mixer.h"
#include "audio/mididrv.h"

namespace Audio {
class AudioStream;
}

namespace Cine {

class CineEngine;

class Sound {
public:

	Sound(Audio::Mixer *mixer, CineEngine *vm) : _mixer(mixer), _vm(vm), _musicType(MT_INVALID) {}
	virtual ~Sound() {}

	virtual MusicType musicType();
	virtual void loadMusic(const char *name) = 0;
	virtual void playMusic() = 0;
	virtual void stopMusic() = 0;
	virtual void fadeOutMusic() = 0;

	virtual void playSound(int mode, int channel, int param3, int param4, int param5, int size) = 0;
	virtual void playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat) = 0;
	virtual void stopSound(int channel) = 0;
	virtual void setBgMusic(int num) = 0;

protected:

	Audio::Mixer *_mixer;
	CineEngine *_vm;
	MusicType _musicType;
};

class PCSoundDriver;
class PCSoundFxPlayer;

class PCSound : public Sound {
public:

	PCSound(Audio::Mixer *mixer, CineEngine *vm);
	~PCSound() override;

	void loadMusic(const char *name) override;
	void playMusic() override;
	void stopMusic() override;
	void fadeOutMusic() override;

	void playSound(int mode, int channel, int param3, int param4, int param5, int size) override;
	void playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat) override;
	void stopSound(int channel) override;
	void setBgMusic(int num) override;

protected:

	PCSoundDriver *_soundDriver;
	PCSoundFxPlayer *_player;

	uint8 _currentMusic, _currentMusicStatus, _currentBgSlot;
};

class PaulaSound : public Sound {
public:

	PaulaSound(Audio::Mixer *mixer, CineEngine *vm);
	~PaulaSound() override;

	void loadMusic(const char *name) override;
	void playMusic() override;
	void stopMusic() override;
	void fadeOutMusic() override;

	void playSound(int mode, int channel, int param3, int param4, int param5, int size) override;
	void playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat) override;
	void stopSound(int channel) override;
	void setBgMusic(int num) override;

	enum {
		PAULA_FREQ = 3579545,
		NUM_CHANNELS = 4
	};

protected:

	struct SfxChannel {
		Audio::SoundHandle handle;
		int volume;
		int volumeStep;
		int curStep;
		int stepCount;

		void initialize(int vol, int volStep, int stepCnt) {
			volume     = vol;
			volumeStep = volStep;
			curStep    = stepCount = stepCnt;
		}
	};
	SfxChannel _channelsTable[NUM_CHANNELS];
	static const int _channelBalance[NUM_CHANNELS];
	Common::Mutex _sfxMutex;
	int _sfxTimer;
	static void sfxTimerProc(void *param);
	void sfxTimerCallback();

	Common::Mutex _musicMutex;
	int _musicTimer;
	int _musicFadeTimer;
	static void musicTimerProc(void *param);
	void musicTimerCallback();
	Audio::SoundHandle _moduleHandle;
	Audio::AudioStream *_moduleStream;
};

extern Sound *g_sound;

} // End of namespace Cine

#endif /* CINE_SOUND_H_ */
