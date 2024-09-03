
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

#ifndef M4_PLATFORM_SOUND_DIGI_H
#define M4_PLATFORM_SOUND_DIGI_H

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "common/hashmap.h"
#include "m4/m4_types.h"

namespace M4 {
namespace Sound {

#define MAX_CHANNELS 4

/**
 * M4 Digital player digivolves to ScummVM-digital player
 */
class Digi {
	/**
	 * Digital sound entry
	 */
	struct DigiEntry {
		Common::String _filename;
		byte *_data = nullptr;
		size_t _size = 0;
		bool _isFootsteps = false;
		DigiEntry() {}
		DigiEntry(Common::String &name, byte *data, size_t size) :
			_filename(name), _data(data), _size(size) {}
	};
	/**
	 * Sound channel
	 */
	struct Channel {
		Audio::SoundHandle _soundHandle;
		int _trigger = -1;
		Common::String _name;
	};
private:
	Audio::Mixer *_mixer;
	Channel _channels[4];
	Common::HashMap<Common::String, DigiEntry> _sounds;

	int32 play(const Common::String &name, uint channel, int32 vol, int32 trigger,
		int32 room_num, bool loop);

public:
	Digi(Audio::Mixer *mixer) : _mixer(mixer) {}
	~Digi();

	/**
	 * Preload a digi sample into memory buffer for play back later.
	 */
	bool preload(const Common::String &name, bool isFootsteps, int roomNum = -1);

	/**
	 * A room can designate one or more sounds to be randomly played when
	 * the player walks around
	 */
	void loadFootstepSounds(const char **names);

	void unload_sounds();
	void unload(const Common::String &name);
	void task();

	// digi_play and digi_play_loop play a particular sound file in a given channel,
	// at a particular volume. The room_num parameter tells us what directory the sound
	// is stored in (all sounds are AIFFs). Trigger is an integer that is fed into
	// kernel_dispatch_trigger when the sound has finished playing
	// If the sound has been preloaded it will be played from memory, otherwise it will
	// be streamed from disk

	int32 play(const Common::String &name, uint channel, int32 vol, int32 trigger, int32 room_num = -1);
	int32 play_loop(const Common::String &name, uint channel, int32 vol, int32 trigger, int32 room_num = -1);
	void playFootsteps();
	void stop(uint channel, bool calledFromUnload = false);
	void flush_mem();

	void read_another_chunk();
	bool play_state(int channel) const;
	void change_volume(int channel, int vol);

	void set_overall_volume(int vol);
	int get_overall_volume();
	int32 ticks_to_play(const char *name, int roomNum = -1);
	void change_panning(int val1, int val2);
};

} // namespace Sound

bool digi_preload(const Common::String &name, int roomNum = -1);
void digi_unload(const Common::String &name);
int32 digi_play(const char *name, uint channel, int32 vol = 255,
	int32 trigger = -1, int32 room_num = -1);
int32 digi_play_loop(const char *name, uint channel, int32 vol = 255,
	int32 trigger = -1, int32 room_num = -1);
void digi_read_another_chunk();
void digi_stop(int channel);
bool digi_play_state(int channel);
void digi_change_volume(int channel, int vol);
void digi_set_overall_volume(int vol);
int digi_get_overall_volume();
int32 digi_ticks_to_play(const char *name, int roomNum = -1);
void digi_change_panning(int val1, int val2);

} // namespace M4

#endif
