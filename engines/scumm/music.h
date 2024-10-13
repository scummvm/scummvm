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

#ifndef SCUMM_MUSIC_H
#define SCUMM_MUSIC_H

#include "common/serializer.h"
#include "common/scummsys.h"

namespace Scumm {

/**
 * Pure virtual base class for the various music/sound engines used in Scumm
 * games. In particular, the iMuse code provides a subclass of this. There are
 * several other subclasses providing music and sound capabilities for
 * several Scumm games.
 * Having this base class for all music engines allows uniform access to the
 * core music/sound functionality, thus simplifying the client code.
 *
 * Instantiated by class Scumm.
 */
class MusicEngine : public Common::Serializable {
public:
	~MusicEngine() override {}

	/**
	 * Set the output volume for music.
	 * Also used, if the inheriting class doesn't
	 * distinguish between music and sfx.
	 * @param vol		the new output volume
	 */
	virtual void setMusicVolume(int vol) = 0;

	/**
	 * Set the output volume for sound effects.
	 * No need to implement this in the inheriting
	 * class if it doesn't distinguish between
	 * music and sfx.
	 * @param vol		the new output volume
	 */
	virtual void setSfxVolume(int vol) {}

	/**
	 * Start playing the sound with the given id.
	 * @param sound		the sound to start
	 */
	virtual void startSound(int sound) = 0;

	/**
	 * Start playing the sound with the given id and track id.
	 * @param sound		the sound to start
	 * @param track		the track to start
	 */
	virtual void startSoundWithTrackID(int sound, int track) { startSound(sound); }

	/**
	 * Stop playing the sound with the given id.
	 * @param sound		the sound to stop
	 */
	virtual void stopSound(int sound) = 0;

	/**
	 * Start playing all currently playing sounds.
	 */
	virtual void stopAllSounds() = 0;

	/**
	 * Query the status of the sound with the given id. Usually this is just
	 * a boolean telling us whether the sound is playing or not.
	 * @param sound		the sound to for which we want the status
	 * @return the status of the specified sound
	 */
	virtual int  getSoundStatus(int sound) const = 0;

	/**
	 * Get the value of the music timer. Used for synchronising scripts with
	 * the music/sound.
	 * @return the music timer
	 */
	virtual int  getMusicTimer() { return 0; }

	/**
	 * Set sound quality if applicable (used for Macintosh sound)
	 * @param qual	quality setting (range and meaning are specific to the respective player)
	 */
	virtual void setQuality(int qual) {}

	/**
	 * Notify player that the sound is supposed to be enabled/disabled (used for Macintosh sound)
	 */
	virtual void toggleMusic(bool enable) {}
	virtual void toggleSoundEffects(bool enable) {}

	/**
	 * Save or load the music state.
	 */
	void saveLoadWithSerializer(Common::Serializer &ser) override {}

	/**
	 * Performs necessary post-load operations on the sound engine, like restarting of music
	 * tracks or looping pcm sounds. Some targets get this done via scripts but others don't.
	 * Currently, this is used for FM-Towns and Mac.
	 */
	virtual void restoreAfterLoad() {}
};

} // End of namespace Scumm

#endif
