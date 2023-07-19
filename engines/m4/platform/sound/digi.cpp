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
#include "m4/fileio/extensions.h"
#include "m4/vars.h"

namespace M4 {
namespace Sound {

Digi::~Digi() {
	unload_sounds();
}

void Digi::preload_sounds(const char **names) {
	if (!_sounds.empty())
		unload_sounds();

	if (names) {
		for (; *names; ++names)
			preload(*names, NOWHERE);
	}
}

void Digi::unload_sounds() {
	_mixer->stopAll();

	for (auto it = _sounds.begin(); it != _sounds.end(); ++it)
		delete it->_value._data;

	_sounds.clear();
}

bool Digi::preload(const Common::String &name, int roomNum) {
	MemHandle workHandle;
	int32 assetSize;

	Common::String fileName = expand_name_2_RAW(name, roomNum);
	if (_sounds.contains(fileName))
		return true;

	// Load in the sound
	if ((workHandle = rget(fileName, &assetSize)) == nullptr)
		error("Could not find sound - %s", fileName.c_str());

	HLock(workHandle);
	const byte *pSrc = (byte *)*workHandle;
	byte *pDest = (byte *)malloc(assetSize);
	Common::copy(pSrc, pSrc + assetSize, pDest);

	HUnLock(workHandle);

	_sounds[name] = DigiEntry(pDest, assetSize);
	return false;
}

void Digi::unload(const Common::String &name) {
	if (_sounds.contains(name)) {
		delete _sounds[name]._data;
		_sounds.erase(name);
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
	Common::String fileName = expand_name_2_RAW(name, room_num);

	digi_preload(fileName, room_num);
	DigiEntry &entry = _sounds[fileName];

	// Create new audio stream
	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
			Audio::makeRawStream(entry._data, entry._size, 11025, Audio::FLAG_UNSIGNED),
			loop ? 0 : 1);
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_channels[channel]._soundHandle, stream, -1, vol);

	if (trigger < 0 || trigger > 32767)
		trigger = -1;
	_channels[channel]._trigger = kernel_trigger_create(trigger);

	_sounds.erase(fileName);
	return 0;
}

Common::String Digi::expand_name_2_RAW(const Common::String &name, int32 room_num) {
	Common::String tempName = f_extension_new(name, "RAW");

	if (!_G(kernel).hag_mode) {
		if (room_num == -1)
			room_num = extract_room_num(name);

		return Common::String::format("%d\\%s", room_num, tempName.c_str());

	} else {
		return tempName;
	}
}


void Digi::stop(uint channel) {
	assert(channel < 4);
	_mixer->stopHandle(_channels[channel]._soundHandle);
	_channels[channel]._trigger = -1;
}

void Digi::flush_mem() {
	// No implementation
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

} // namespace Sound

bool digi_preload(const Common::String &name, int roomNum) {
	return _G(digi).preload(name, roomNum);
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

} // namespace M4
