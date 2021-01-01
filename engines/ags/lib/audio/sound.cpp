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

#include "ags/lib/audio/sound.h"
#include "common/textconsole.h"
#include "ags/ags.h"
#include "audio/mixer.h"

namespace AGS3 {

int install_sound(int digi, int midi, const char *cfg_path) {
	// TODO: install_sound
	return 0;
}

void remove_sound() {
	// TODO: remove_sound
}

void reserve_voices(int digi_voices, int midi_voices) {
	// TODO: reserve_voices
}

void set_volume_per_voice(int scale) {
	// TODO: set_volume_per_voice
}

int install_sound_input(int digi, int midi) {
	// TODO: install_sound_input
	return 0;
}

void remove_sound_input() {
	// TODO: remove_sound_input
}

void set_volume(int digi_volume, int midi_volume) {
	// TODO: set_volume
}

void set_hardware_volume(int digi_volume, int midi_volume) {
	// TODO: set_hardware_volume
}

void get_volume(int *digi_volume, int *midi_volume) {
	// TODO: get_volume
	*digi_volume = 255;
	*midi_volume = 255;
}

void get_hardware_volume(int *digi_volume, int *midi_volume) {
	// TODO: get_hardware_volume
}

void set_mixer_quality(int quality) {
	// TODO: set_mixer_quality
}

int get_mixer_quality() {
	warning("TODO: get_mixer_quality");
	return 0;
}

int get_mixer_frequency() {
	return ::AGS::g_vm->_mixer->getOutputRate();
}

int get_mixer_bits() {
	return 16;
}

int get_mixer_channels() {
	warning("TODO: get_mixer_channels");
	return 10;
}

int get_mixer_voices() {
	warning("TODO: get_mixer_voices");
	return 1;
}

int get_mixer_buffer_length() {
	warning("TODO: get_mixer_buffer_length");
	return 22040;
}

void stop_audio_stream(AUDIOSTREAM *stream) {
	warning("TODO: stop_audio_stream");
}

} // namespace AGS3
