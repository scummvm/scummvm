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

#ifndef MADS_FOREST_DIGI_H
#define MADS_FOREST_DIGI_H

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace MADS {
namespace Forest {

#define MAX_DIGI_CHANNELS 3
#define MAX_DIGI_VOLUME 0x7fff

class DigiPlayer {
	struct DigiChannel {
		Audio::SoundHandle _soundHandle;
		bool _isPlaying = false;
		int _triggerId;
		bool _endTrigger = true;
	};
private:
	Audio::Mixer *_mixer;
	DigiChannel _channels[MAX_DIGI_CHANNELS];
	int _initialVolume = MAX_DIGI_VOLUME;

public:
	DigiPlayer(Audio::Mixer *mixer);

	/**
	 * Play a digital sound
	 * @param name	Resource name
	 * @param slot	Channel to play it in
	*/
	void play(const char *name, int slot);

	/**
	 * Stop any playing digital sound in a particular channel
	 * @param slot	Channel to stop
	 */
	void stop(int slot);

	/**
	 * Polls for any playing sounds that are finished
	 */
	void poll();

	/**
	 * Sets the initial volume for new digi sounds
	 */
	void setInitialVolume(int vol);

	/**
	 * Sets the volume for a specific digi channel
	 */
	void setVolume(int vol, int slot);
};

extern int digi_val2;
extern int digi_trigger_effect;
extern bool digi_trigger_dialog;
extern bool digi_flag1, digi_flag2;

extern void digi_play(const char *name, int slot);
extern void digi_play_build(int roomNum, char thing, int num, int slot);
extern void digi_play_build_ii(char thing, int num, int slot);
extern void digi_stop(int which_one);
extern void digi_initial_volume(int vol);
extern void digi_set_volume(int vol, int slot);
extern void digi_read_another_chunk();

} // namespace Forest
} // namespace MADS

#endif
