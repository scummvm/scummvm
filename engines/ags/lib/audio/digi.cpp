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

#include "ags/lib/audio/digi.h"
#include "common/textconsole.h"

namespace AGS3 {

DIGI_DRIVER *digi_driver;

DIGI_DRIVER *digi_input_driver;

int digi_card;

int digi_input_card;


int detect_digi_driver(int driver_id) {
	return 0;
}

SAMPLE *load_wav_pf(PACKFILE *f) {
	warning("TODO: load_wav_pf");
	return nullptr;
}

void destroy_sample(SAMPLE *spl) {
	delete spl;
}

int play_sample(SAMPLE *spl, int vol, int pan, int freq, int loop) {
	warning("TODO: play_sample");
	return 0;
}

void stop_sample(SAMPLE *spl) {
	warning("TODO: stop_sample");
}


void voice_start(int voice) {
	warning("TODO: voice_start");
}

void voice_stop(int voice) {
	warning("TODO: voice_stop");
}

int voice_get_position(int voice) {
	warning("TODO: voice_get_position");
	return 0;
}

void voice_set_position(int voice, int position) {
	warning("TODO: voice_set_position");
}

void voice_set_volume(int voice, int volume) {
	warning("TODO: voice_set_volume");
}

int voice_get_frequency(int voice) {
	warning("TODO: voice_get_frequency");
	return 0;
}

void voice_set_pan(int voice, int pan) {
	warning("TODO: voice_set_pan");
}

} // namespace AGS3
