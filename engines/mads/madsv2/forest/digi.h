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
namespace MADSV2 {
namespace Forest {

class DigiPlayer {
private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _slots[8];
public:
	DigiPlayer(Audio::Mixer *mixer) : _mixer(mixer) {
	}

	void play(const char *name, int slot);
	void stop(int slot);
};

extern int digi_val2;
extern int digi_timing_index;
extern bool digi_flag1, digi_flag2;

extern void digi_install();
extern void digi_play(const char *name, int slot);
extern void digi_play_build(int room, char thing, int num, int slot);
extern void digi_play_build_ii(char thing, int num, int slot);
extern void digi_stop(int which_one);
extern void digi_uninstall();
extern void digi_initial_volume(int vol);
extern void digi_set_volume(int vol, int slot);
inline void digi_read_another_chunk() {}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS

#endif
