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

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "m4/platform/sound/digi.h"
#include "m4/adv_r/adv_file.h"
#include "m4/core/imath.h"
#include "m4/fileio/extensions.h"
#include "m4/vars.h"

namespace M4 {
namespace Sound {

Digi::~Digi() {
	unload_sounds();
}

void Digi::loadFootstepSounds(const char **names) {
	if (!_sounds.empty())
		unload_sounds();

	if (names) {
		for (; *names; ++names)
			preload(*names, true, NOWHERE);
	}
}

void Digi::unload_sounds() {
	_mixer->stopAll();

	for (auto it = _sounds.begin(); it != _sounds.end(); ++it) {
		rtoss(it->_value._filename);
		free(it->_value._data);
	}

	_sounds.clear();
}

bool Digi::preload(const Common::String &name, bool isFootsteps, int roomNum) {
	MemHandle workHandle;
	int32 assetSize;

	if (_sounds.contains(name))
		return true;

	// Load in the sound
	Common::String fileName = expand_name_2_RAW(name, roomNum);
	if ((workHandle = rget(fileName, &assetSize)) == nullptr)
		error("Could not find sound - %s", fileName.c_str());

	HLock(workHandle);
	const byte *pSrc = (byte *)*workHandle;
	byte *pDest = (byte *)malloc(assetSize);
	Common::copy(pSrc, pSrc + assetSize, pDest);

	HUnLock(workHandle);

	_sounds[name] = DigiEntry(fileName, pDest, assetSize);
	_sounds[name]._isFootsteps = isFootsteps;
	return false;
}

void Digi::unload(const Common::String &name) {
	if (_sounds.contains(name)) {
		// Stop it if it's playing
		for (int channel = 0; channel < MAX_CHANNELS; ++channel) {
			if (_channels[channel]._name == name)
				stop(channel, true);
		}

		// Remove the underlying resource
		if (!_sounds[name]._filename.empty() && !_sounds[name]._isFootsteps) {
			rtoss(_sounds[name]._filename);
			_sounds[name]._filename.clear();

			// Delete the sound entry
			free(_sounds[name]._data);
			_sounds.erase(name);
		}
	}
}

void Digi::task() {
	// No implementation
}

int32 Digi::play(const Common::String &name, uint channel, int32 vol, int32 trigger, int32 room_num) {
	return play(name, channel, vol, trigger, room_num, false);
}

int32 Digi::play_loop(const Common::String &name, uint channel, int32 vol, int32 trigger, int32 room_num) {
	return play(name, channel, vol, trigger, room_num, true);
}

int32 Digi::play(const Common::String &name, uint channel, int32 vol, int32 trigger, int32 room_num, bool loop) {
	assert(channel < 4);

	// Assure no prior sound for the channel is playing
	stop(channel);

	// Load in the new sound
	preload(name, false, room_num);
	DigiEntry &entry = _sounds[name];
	Channel &c = _channels[channel];

	// Create new audio stream
	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
			Audio::makeRawStream(entry._data, entry._size, 11025, Audio::FLAG_UNSIGNED,
				DisposeAfterUse::NO),
			loop ? 0 : 1);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &c._soundHandle, stream,
		-1, vol);

	if (trigger < 0 || trigger > 32767)
		trigger = -1;
	c._trigger = kernel_trigger_create(trigger);
	c._name = name;

	return 0;
}

void Digi::playFootsteps() {
	// Get a list of the walking sounds
	Common::Array<Common::String> names;

	for (auto it = _sounds.begin(); it != _sounds.end(); ++it) {
		if (it->_value._isFootsteps)
			names.push_back(it->_key);
	}

	if (!names.empty()) {
		play(names[imath_ranged_rand(0, (int)names.size() - 1)].c_str(),
			1, 100, NO_TRIGGER, GLOBAL_SCENE);
	}
}

void Digi::stop(uint channel, bool calledFromUnload) {
	assert(channel < 4);

	Channel &c = _channels[channel];
	if (!c._name.empty()) {
		Common::String name = c._name;

		_mixer->stopHandle(c._soundHandle);
		c._trigger = -1;
		c._name.clear();

		if (!calledFromUnload) {
			digi_unload(name);
		}
	}
}

void Digi::flush_mem() {
	unload_sounds();
}

void Digi::read_another_chunk() {
	// For ScummVM, the audio data is completely loaded for each sound. But we still
	// need to check whether a sound has finished so it's trigger can be dispatched
	for (int channel = 0; channel < MAX_CHANNELS; ++channel) {
		Channel &c = _channels[channel];

		// Check if the channel has a sound playing that finished
		if (c._trigger != -1 && !_mixer->isSoundHandleActive(c._soundHandle)) {
			int trigger = c._trigger;
			c._trigger = -1;
			stop(channel);

			// Dispatch the trigger
			kernel_trigger_dispatchx(trigger);
		}
	}
}

bool Digi::play_state(int channel) const {
	return _mixer->isSoundHandleActive(_channels[channel]._soundHandle);
}

void Digi::change_volume(int channel, int vol) {
	_mixer->setChannelVolume(_channels[channel]._soundHandle, vol);
}

void Digi::set_overall_volume(int vol) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, vol);
}

int Digi::get_overall_volume() {
	return _mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType);
}

int32 Digi::ticks_to_play(const char *name, int roomNum) {
	// Get the file and retrieve it's size
	Common::String filename = expand_name_2_RAW(name, roomNum);
	SysFile sf(filename);
	double size = sf.size();
	sf.close();

	term_message("  digi_ticks_to_play");
	term_message("  %s", filename.c_str());
	term_message("  size = %f, room = %d", size, roomNum);

	return (int32)floor(size * 0.000090702946 * 60.0);
}

void Digi::change_panning(int val1, int val2) {
	warning("TODO: digi_change_panning");
}

} // namespace Sound

bool digi_preload(const Common::String &name, int roomNum) {
	return _G(digi).preload(name, false, roomNum);
}

void digi_unload(const Common::String &name) {
	_G(digi).unload(name);
}

int32 digi_play(const char *name, uint channel, int32 vol, int32 trigger, int32 room_num) {
	return _G(digi).play(name, channel, vol, trigger, room_num);
}

int32 digi_play_loop(const char *name, uint channel, int32 vol, int32 trigger, int32 room_num) {
	return _G(digi).play_loop(name, channel, vol, trigger, room_num);
}

void digi_read_another_chunk() {
	return _G(digi).read_another_chunk();
}

void digi_stop(int slot) {
	_G(digi).stop(slot);
}

bool digi_play_state(int channel) {
	return _G(digi).play_state(channel);
}

void digi_change_volume(int channel, int vol) {
	_G(digi).change_volume(channel, vol);
}

void digi_set_overall_volume(int vol) {
	_G(digi).set_overall_volume(vol);
}

int digi_get_overall_volume() {
	return _G(digi).get_overall_volume();
}

int32 digi_ticks_to_play(const char *name, int roomNum) {
	return _G(digi).ticks_to_play(name, roomNum);
}

void digi_change_panning(int val1, int val2) {
	_G(digi).change_panning(val1, val2);
}

} // namespace M4
