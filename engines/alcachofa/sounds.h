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

#ifndef SOUNDS_H
#define SOUNDS_H

#include "scheduler.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"

namespace Alcachofa {

class Character;

using SoundID = uint32;
static constexpr SoundID kInvalidSoundID = 0;
class Sounds {
public:
	Sounds();
	~Sounds();

	void update();
	SoundID playVoice(const Common::String &fileName, byte volume = Audio::Mixer::kMaxChannelVolume);
	SoundID playSFX(const Common::String &fileName, byte volume = Audio::Mixer::kMaxChannelVolume);
	void stopAll();
	void stopVoice();
	void pauseAll(bool paused);
	void fadeOut(SoundID id, uint32 duration);
	void fadeOutVoiceAndSFX(uint32 duration);
	bool isAlive(SoundID id);
	void setVolume(SoundID id, byte volume);
	void setAppropriateVolume(SoundID id,
		MainCharacterKind processCharacter,
		Character *speakingCharacter);
	bool isNoisy(SoundID id, float windowSize, float minDifferences); ///< used for lip-sync

	void startMusic(int musicId);
	void queueMusic(int musicId);
	void fadeMusic(uint32 duration = 500);
	void setMusicToRoom(int roomMusicId);
	Task *waitForMusicToEnd(Process &processd);
	inline bool isMusicPlaying() const { return _isMusicPlaying; }
	inline int musicID() const { return _nextMusicID; }
	inline FakeSemaphore &musicSemaphore() { return _musicSemaphore; }

private:
	struct Playback {;
		void fadeOut(uint32 duration);

		SoundID _id = 0;
		Audio::SoundHandle _handle;
		Audio::Mixer::SoundType _type = Audio::Mixer::SoundType::kPlainSoundType;
		uint32 _fadeStart = 0,
			_fadeDuration = 0;
		int _inputRate;
		Common::Array<int16> _samples; ///< might not be filled, only voice samples are preloaded for lip-sync
	};
	Playback *getPlaybackById(SoundID id);
	SoundID playSoundInternal(const char *fileName, byte volume, Audio::Mixer::SoundType type);

	Common::Array<Playback> _playbacks;
	Audio::Mixer *_mixer;
	SoundID _nextID = 1;

	SoundID _musicSoundID = kInvalidSoundID; // we use another soundID to reuse fading
	bool _isMusicPlaying = false;
	int _nextMusicID = -1;
	FakeSemaphore _musicSemaphore;
};

struct PlaySoundTask final : public Task {
	PlaySoundTask(Process &process, SoundID soundID);
	virtual TaskReturn run() override;
	virtual void debugPrint() override;
private:
	SoundID _soundID;
};

struct WaitForMusicTask final : public Task {
	WaitForMusicTask(Process &process, FakeLock &&lock);
	virtual TaskReturn run() override;
	virtual void debugPrint() override;
private:
	FakeLock _lock;
};

}

#endif // SOUNDS_H
