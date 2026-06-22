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

#include "audio/decoders/adpcm.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/forest.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/kernel.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

int digi_val2;
int digi_timing_index;
bool digi_flag1, digi_flag2;

DigiPlayer::DigiPlayer(Audio::Mixer *mixer) : _mixer(mixer) {
	_channels[0]._triggerId = 7;
	_channels[1]._triggerId = 8;
	_channels[2]._triggerId = 9;

	digi_val2 = 0;
	digi_timing_index = 0;
	digi_flag1 = digi_flag2 = false;
}

void DigiPlayer::play(const char *name, int slot) {
	stop(slot);
	DigiChannel &c = _channels[slot - 1];

	Common::SeekableReadStream *src;
	src = env_open(Common::String::format("*%s.rac", name).c_str());
	if (!src)
		src = env_open(Common::String::format("*%s.raw", name).c_str());

	if (!src) {
		warning("Could not open digi sound - %s", name);
		return;
	}

	// Skip past the HMI header
	src->seek(0x20);

	Audio::AudioStream *audioStream = Audio::makeADPCMStream(src, DisposeAfterUse::YES,
		src->size() - 0x20, Audio::kADPCMDVI, 11025, 1);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &c._soundHandle, audioStream);
	c._isPlaying = true;
}

void DigiPlayer::stop(int slot) {
	assert(slot >= 1 && slot <= MAX_DIGI_CHANNELS);
	DigiChannel &c = _channels[slot - 1];

	if (c._isPlaying) {
		_mixer->stopHandle(c._soundHandle);
		c._isPlaying = false;

		if (c._endTrigger) {
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(1, c._triggerId);
		}

		c._endTrigger = true;
	}
}

void DigiPlayer::poll() {
	for (int slot = 1; slot <= MAX_DIGI_CHANNELS; ++slot) {
		DigiChannel &c = _channels[slot - 1];

		if (c._isPlaying && !_mixer->isSoundHandleActive(c._soundHandle))
			digi_stop(slot);
	}
}

void digi_play_build_ii(char thing, int num, int slot) {
	Common::String name = Common::String::format("%c0ii%c%03d",
		(thing == '_') ? 's' : 'd', thing, num);
	digi_play(name.c_str(), slot);
}

void digi_play_build(int roomNum, char thing, int num, int slot) {
	Common::String name = Common::String::format("%c%d%c%03d",
		(thing == '_') ? 's' : 'd', roomNum, thing, num);
	digi_play(name.c_str(), slot);
}

void digi_play(const char *name, int slot) {
	g_engine->_digiPlayer.play(name, slot);
}

void digi_stop(int which_one) {
	g_engine->_digiPlayer.stop(which_one);
}

void digi_read_another_chunk() {
	g_engine->_digiPlayer.poll();
}

void digi_initial_volume(int vol) {
}

void digi_set_volume(int vol, int slot) {
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
