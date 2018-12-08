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
#include "common/array.h"

namespace Glk {

class Sounds;

/**
 * Holds the data for a playing sound
 */
struct SoundChannel {
	Sounds *_owner;
	Audio::AudioStream *_stream;
	glui32 _rock;

	/**
	 * Destructor
	 */
	SoundChannel() : _stream(nullptr), _rock(0) {}

	/**
	 * Destructor
	 */
	~SoundChannel();

	/**
	 * Play a sound
	 */
	void play(uint soundNum);
};
typedef SoundChannel *schanid_t;

/**
 * Sound manager
 */
class Sounds {
	friend struct SoundChannel;
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
	schanid_t create(glui32 rock = 0);

	/**
	 * Used to iterate over the current list of sound channels
	 */
	schanid_t iterate(schanid_t chan, glui32 *rockptr = nullptr);
};

} // End of namespace Glk

#endif
