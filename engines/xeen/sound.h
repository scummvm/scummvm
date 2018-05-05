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

#ifndef XEEN_SOUND_H
#define XEEN_SOUND_H

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "xeen/files.h"
#include "xeen/sound_driver.h"

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
	Common::String _currentMusic, _priorMusic;
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
	void stopSong() { songCommand(STOP_SONG); }

	/**
	 * Restart a previously playing song (which must still be loaded)
	 */
	void restartSong() { songCommand(RESTART_SONG); }

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
	 * Plays a song
	 */
	void playSong(const byte *data) {
		_SoundDriver->playSong(data);
	}

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

	/**
	 * Play a given sound
	 */
	void playSound(const Common::String &name, int ccNum, int unused);

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
	void playVoice(const Common::String &name, int ccMode = -1);
};

} // End of namespace Xeen

#endif /* XEEN_SOUND_H */
