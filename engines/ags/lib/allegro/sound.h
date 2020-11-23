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

#ifndef AGS_LIB_ALLEGRO_SOUND_H
#define AGS_LIB_ALLEGRO_SOUND_H

#include "common/scummsys.h"
#include "ags/lib/allegro/base.h"
#include "ags/lib/allegro/alconfig.h"

namespace AGS3 {

AL_FUNC(void, reserve_voices, (int digi_voices, int midi_voices));
AL_FUNC(void, set_volume_per_voice, (int scale));

AL_FUNC(int, install_sound, (int digi, int midi, AL_CONST char *cfg_path));
AL_FUNC(void, remove_sound, (void));

AL_FUNC(int, install_sound_input, (int digi, int midi));
AL_FUNC(void, remove_sound_input, (void));

AL_FUNC(void, set_volume, (int digi_volume, int midi_volume));
AL_FUNC(void, set_hardware_volume, (int digi_volume, int midi_volume));

AL_FUNC(void, get_volume, (int *digi_volume, int *midi_volume));
AL_FUNC(void, get_hardware_volume, (int *digi_volume, int *midi_volume));

AL_FUNC(void, set_mixer_quality, (int quality));
AL_FUNC(int, get_mixer_quality, (void));
AL_FUNC(int, get_mixer_frequency, (void));
AL_FUNC(int, get_mixer_bits, (void));
AL_FUNC(int, get_mixer_channels, (void));
AL_FUNC(int, get_mixer_voices, (void));
AL_FUNC(int, get_mixer_buffer_length, (void));

} // namespace AGS3

#endif
