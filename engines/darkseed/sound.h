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

#ifndef DARKSEED_SOUND_H
#define DARKSEED_SOUND_H

#include "darkseed/music.h"

#include "audio/mixer.h"
#include "common/array.h"
#include "common/error.h"
#include "common/serializer.h"

namespace Darkseed {
enum class MusicId : uint8 {
	kNone = 0,
	kLab,
	kVictory,
	kCemetry,
	kLeech,
	kExt,
	kQuiet,
	kPackage,
	kDth,
	kLibrary,
	kRadio,
	kOutdoor,
	kTown,
};

enum class StartMusicId : uint8 {
	kCredits = 0,
	kAlien,
	kImplant,
	kLaunch,
	kNight2,
	kNight3,
	kBook,
	kDoll
};

class Sound {
	Audio::Mixer *_mixer;
	Audio::SoundHandle _speechHandle;
	Audio::SoundHandle _sfxHandle;
	MusicPlayer *_musicPlayer;
	Common::Array<uint8> _didSpeech;

public:
	explicit Sound(Audio::Mixer *mixer);
	~Sound();

	int init();

	Common::Error sync(Common::Serializer &s);

	void playTosSpeech(int tosIdx);
	void stopSpeech();
	bool isPlayingSpeech() const;
	bool isPlayingSfx() const;
	bool isPlayingMusic();
	void resetSpeech();
	void playMusic(MusicId musicId, bool loop = true);
	void playMusic(StartMusicId musicId);
	void playMusic(Common::String const &filename, bool loop = false);
	void stopMusic();
	void playSfx(uint8 sfxId, int unk1, int unk2);
	void stopSfx();
	void syncSoundSettings();
	void killAllSound();
private:
	void playDosCDSfx(int sfxId);
	void playFloppySpeech(int tosIdx);
};

} // namespace Darkseed

#endif // DARKSEED_SOUND_H
