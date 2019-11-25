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

#ifndef GLK_SOUND_H
#define GLK_SOUND_H

#include "glk/glk_types.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/array.h"

namespace Glk {

#define GLK_MAXVOLUME 0x10000

class Sounds;

/**
 * Holds the data for a playing sound
 */
class SoundChannel {
private:
	Sounds *_owner;
	uint _soundNum;
	uint _notify;
	Audio::SoundHandle _handle;
	uint _defaultVolume;
public:
	uint _rock;
	gidispatch_rock_t _dispRock;
public:
	/**
	 * Constructor
	 */
	SoundChannel(Sounds *owner, uint volume);

	/**
	 * Destructor
	 */
	~SoundChannel();

	/**
	 * Play a sound
	 */
	uint play(uint soundNum, uint repeats = 1, uint notify = 0);

	/**
	 * Stop playing sound
	 */
	void stop();

	/**
	 * Poll for whether a playing sound was finished
	 */
	void poll();

	/**
	 * Change the volume
	 * @param volume		Volume from 0 (silence) to 0x10000 (full volume)
	 * @param duration		Optional duration for a gradual volume change
	 * @param notify		If non-zero, triggers a evtype_VolumeNotify when
	 *						the volume change duration finishes
	 */
	void setVolume(uint volume, uint duration = 0, uint notify = 0);

	/**
	 * Pause playback
	 */
	void pause();

	/**
	 * Unpause playback
	 */
	void unpause();
};
typedef SoundChannel *schanid_t;

/**
 * Sound manager
 */
class Sounds {
	friend class SoundChannel;
private:
	Common::Array<schanid_t> _sounds;
private:
	/**
	 * Remove a sound from the sounds list
	 */
	void removeSound(schanid_t snd);
public:
	~Sounds();

	/**
	 * Create a new channel
	 */
	schanid_t create(uint rock = 0, uint volume = GLK_MAXVOLUME);

	/**
	 * Used to iterate over the current list of sound channels
	 */
	schanid_t iterate(schanid_t chan, uint *rockptr = nullptr);

	/**
	 * Poll for whether any playing sounds are finished
	 */
	void poll();
};

} // End of namespace Glk

#endif
