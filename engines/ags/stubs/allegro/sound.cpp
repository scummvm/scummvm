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

#include "ags/stubs/allegro/sound.h"
#include "common/textconsole.h"

namespace AGS3 {

_DRIVER_INFO _digi_driver_list[] = {
	{ 0, nullptr, 0     }
};


int install_sound(int digi, int midi, const char *cfg_path) {
	// TODO: install_sound
	return 0;
}

void remove_sound() {
	// TODO: remove_sound
}

void reserve_voices(int digi_voices, int midi_voices) {
	error("reserve_voices");
}

void set_volume_per_voice(int scale) {
	error("set_volume_per_voice");

}

int install_sound_input(int digi, int midi) {
	error("install_sound_input");
}

void remove_sound_input() {
	error("remove_sound_input");
}

void set_volume(int digi_volume, int midi_volume) {
	error("set_volume");
}

void set_hardware_volume(int digi_volume, int midi_volume) {
	error("set_hardware_volume");
}

void get_volume(int *digi_volume, int *midi_volume) {
	error("get_volume");
}

void get_hardware_volume(int *digi_volume, int *midi_volume) {
	error("get_hardware_volume");
}

void set_mixer_quality(int quality) {
	error("set_mixer_quality");
}

int get_mixer_quality() {
	error("get_mixer_quality");
}

int get_mixer_frequency() {
	error("get_mixer_frequency");
}

int get_mixer_bits() {
	error("get_mixer_bits");
}

int get_mixer_channels() {
	error("get_mixer_channels");
}

int get_mixer_voices() {
	error("get_mixer_voices");
}

int get_mixer_buffer_length() {
	error("get_mixer_buffer_length");
}

} // namespace AGS3
