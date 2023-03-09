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

#ifndef MM_SHARED_XEEN_SOUND_H
#define MM_SHARED_XEEN_SOUND_H

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "mm/shared/xeen/file.h"
#include "mm/shared/xeen/sound_driver.h"

namespace MM {
namespace Shared {
namespace Xeen {

class Sound {
private:
	SoundDriver *_SoundDriver;
	const byte *_effectsData;
	Common::Array<uint16> _effectsOffsets;
	const byte *_songData;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	byte _musicPercent;
	int _musicVolume, _sfxVolume;
private:
	/**
	 * Loads effects data that was embedded in the music driver
	 */
	void loadEffectsData();

	/**
	 * Updates any playing music
	 */
	void update();

	/**
	 * Updates the music and sound effects playing volume
	 */
	void updateVolume();
public:
	bool _fxOn;
	bool _musicOn;
	Common::String _currentMusic;
	int _musicSide;
	bool _subtitles;
public:
	Sound(Audio::Mixer *mixer);
	virtual ~Sound();

	/**
	 * Starts an effect playing
	 */
	void playFX(uint effectId);

	/**
	 * Stops any currently playing FX
	 */
	void stopFX();

	/**
	 * Executes special music command
	 */
	int songCommand(uint commandId, byte musicVolume = 0, byte sfxVolume = 0);

	/**
	 * Stops any currently playing music
	 */
	void stopSong() {
		songCommand(STOP_SONG);
	}

	/**
	 * Sets the in-game music volume percent. This is separate from the ScummVM volume
	 */
	void setMusicPercent(byte percent);

	/**
	 * Plays a song
	 */
	void playSong(Common::SeekableReadStream &stream);

	/**
	 * Plays a song
	 */
	void playSong(const Common::String &name, int param = 0);

	/**
	 * Returns true if music is playing
	 */
	bool isMusicPlaying() const;

	/**
	 * Sets whether music is on
	 */
	void setMusicOn(bool isOn);

	/**
	* Sets whether sound effects is on
	*/
	void setFxOn(bool isOn);

	/**
	 * Called to reload sound settings
	 */
	void updateSoundSettings();

	/**
	 * Stops all playing music, FX, and sound samples
	 */
	void stopAllAudio();

	/**
	 * Play a given sound
	 */
	void playSound(Common::SeekableReadStream &s, int unused = 0);

	/**
	 * Play a given sound
	 */
	void playSound(const Common::String &name, int unused = 0);
#ifdef ENABLE_XEEN
	/**
	 * Play a given sound
	 */
	void playSound(const Common::String &name, int ccNum, int unused);
#endif
	/**
	 * Stop playing a sound loaded from a .m file
	 * @remarks		In the original, passing 1 to playSound stopped the sound
	 */
	void stopSound();

	/**
	 * Returns true if a sound file is currently playing
	 * @remarks		In the original, passing 0 to playSound returned play status
	 */
	bool isSoundPlaying() const;

	/**
	 * Play a given voice file
	 */
#ifdef ENABLE_XEEN
	void playVoice(const Common::String &name, int ccMode = -1);
#else
	void playVoice(const Common::String &name);
#endif
};

} // namespace Xeen
} // namespace Shared
} // namespace MM

#endif
