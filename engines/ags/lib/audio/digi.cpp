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

/*------------------------------------------------------------------*/

DIGI_DRIVER::DIGI_DRIVER() :
	id(0), name(nullptr),
	desc(nullptr),
	ascii_name(nullptr),
	voices(0),
	basevoice(0),
	max_voices(0),
	def_voices(0),
	detect(nullptr),
	init(nullptr),
	exit_(nullptr),
	set_mixer_volume(nullptr),
	get_mixer_volume(nullptr),
	lock_voice(nullptr),
	unlock_voice(nullptr),
	buffer_size(nullptr),
	init_voice(nullptr),
	release_voice(nullptr),
	start_voice(nullptr),
	stop_voice(nullptr),
	loop_voice(nullptr),
	get_position(nullptr),
	set_position(nullptr),
	get_volume(nullptr),
	set_volume(nullptr),
	ramp_volume(nullptr),
	stop_volume_ramp(nullptr),
	get_frequency(nullptr),
	set_frequency(nullptr),
	sweep_frequency(nullptr),
	stop_frequency_sweep(nullptr),
	get_pan(nullptr),
	set_pan(nullptr),
	sweep_pan(nullptr),
	stop_pan_sweep(nullptr),
	set_echo(nullptr),
	set_tremolo(nullptr),
	set_vibrato(nullptr),
	rec_cap_bits(0),
	rec_cap_stereo(0),
	rec_cap_rate(nullptr),
	rec_cap_parm(nullptr),
	rec_source(nullptr),
	rec_start(nullptr),
	rec_stop(nullptr),
	rec_read(nullptr) {
}

/*------------------------------------------------------------------*/

class ScummVMDigiDriver : public DIGI_DRIVER {
};

/*------------------------------------------------------------------*/

DIGI_DRIVER *digi_driver;

DIGI_DRIVER *digi_input_driver;

int digi_card;

int digi_input_card;

static byte dummy_driver_data[1] = { 0 };

BEGIN_DIGI_DRIVER_LIST
	{ SCUMMVM_ID, &dummy_driver_data, true },
END_DIGI_DRIVER_LIST


int detect_digi_driver(int driver_id) {
	assert(driver_id == SCUMMVM_ID);
	digi_driver = new ScummVMDigiDriver();
	return 16;
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
