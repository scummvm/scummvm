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
	// Floppy speech filenames from TOS1.DIG
	char _sfxFloppyDigFilenameTbl[120][14];

	Audio::Mixer *_mixer;
	Audio::SoundHandle _speechHandle;
	Audio::SoundHandle _sfxHandle;
	MusicPlayer *_musicPlayer;
	Common::Array<uint8> _didSpeech;
	bool _useFloppyMusic;
	bool _useCdSfx;
	bool _useFloppySfx;
	uint8 _lastPlayedDigitalSfx;

public:
	explicit Sound(Audio::Mixer *mixer);
	~Sound();

	int init();

	Common::Error sync(Common::Serializer &s);

	bool isMuted() const;
	void playTosSpeech(int tosIdx);
	void stopSpeech();
	bool isPlayingSpeech() const;
	bool isPlayingSfx() const;
	bool isPlayingSfx(uint8 sfxId) const;
	bool isPlayingMusic();
	void resetSpeech();
	void playMusic(MusicId musicId, bool loop = true);
	void playMusic(StartMusicId musicId);
	void playMusic(Common::String const &filename, Common::String const *instrBankFilename = nullptr, uint8 priority = 0xFF, bool loop = false);
	void stopMusic();
	void pauseMusic(bool pause);
	void playSfx(uint8 sfxId, uint8 priority, int unk2);
	void playDosFloppySfx(byte sfxId, uint8 priority);
	void stopSfx();
	bool isUsingCdSfx() const;
	bool isUsingFloppySfx() const;
	void syncSoundSettings();
	void killAllSound();
	void startFadeOut();
	bool isFading();

private:
	void loadTosDigData(Common::SeekableReadStream *in, int32 size);
	void playDosCDSfx(int sfxId);
	void playFloppySpeech(int tosIdx);
	int convertCdSpeechToFloppySfxId(int cdSfxId);
};

} // namespace Darkseed

#endif // DARKSEED_SOUND_H
