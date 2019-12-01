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

#include "ultima8/misc/pent_include.h"
#include "FluidSynthMidiDriver.h"

#ifdef USE_FLUIDSYNTH_MIDI

namespace Ultima8 {

//#include <cstring>

const MidiDriver::MidiDriverDesc FluidSynthMidiDriver::desc =
    MidiDriver::MidiDriverDesc("FluidSynth", createInstance);

// MidiDriver method implementations

FluidSynthMidiDriver::FluidSynthMidiDriver()
	: LowLevelMidiDriver(), _settings(0), _synth(0), _soundFont(-1) {
}

void FluidSynthMidiDriver::setInt(const char *name, int val) {
	//char *name2 = strdup(name);
	char *name2 = const_cast<char *>(name);

	fluid_settings_setint(_settings, name2, val);
	//std::free(name2);
}

void FluidSynthMidiDriver::setNum(const char *name, double val) {
	//char *name2 = strdup(name);
	char *name2 = const_cast<char *>(name);

	fluid_settings_setnum(_settings, name2, val);
	//std::free(name2);
}

void FluidSynthMidiDriver::setStr(const char *name, const char *val) {
	//char *name2 = strdup(name);
	//char *val2 = strdup(val);
	char *name2 = const_cast<char *>(name);
	char *val2 = const_cast<char *>(val);

	fluid_settings_setstr(_settings, name2, val2);
	//std::free(name2);
	//std::free(val2);
}

int FluidSynthMidiDriver::open() {

	if (!stereo) {
		perr << "FluidSynth only works with Stereo output" << std::endl;
		return -1;
	}

	std::string soundfont = getConfigSetting("soundfont", "");

	if (soundfont == "") {
		perr << "FluidSynth requires a 'soundfont' setting" << std::endl;
		return -2;
	}

	_settings = new_fluid_settings();

	// The default gain setting is ridiculously low, but we can't set it
	// too high either or sound will be clipped. This may need tuning...

	setNum("synth.gain", 2.1);
	setNum("synth.sample-rate", sample_rate);

	_synth = new_fluid_synth(_settings);

	// In theory, this ought to reduce CPU load... but it doesn't make any
	// noticeable difference for me, so disable it for now.

	// fluid_synth_set_interp_method(_synth, -1, FLUID_INTERP_LINEAR);
	// fluid_synth_set_reverb_on(_synth, 0);
	// fluid_synth_set_chorus_on(_synth, 0);

	_soundFont = fluid_synth_sfload(_synth, soundfont.c_str(), 1);
	if (_soundFont == -1) {
		perr << "Failed loading custom sound font '" << soundfont << "'" << std::endl;
		return -3;
	}

	return 0;
}

void FluidSynthMidiDriver::close() {

	if (_soundFont != -1)
		fluid_synth_sfunload(_synth, _soundFont, 1);
	_soundFont = -1;

	delete_fluid_synth(_synth);
	_synth = 0;
	delete_fluid_settings(_settings);
	_settings = 0;
}

void FluidSynthMidiDriver::send(uint32 b) {
	//uint8 param3 = (uint8) ((b >> 24) & 0xFF);
	uint32 param2 = (uint8)((b >> 16) & 0xFF);
	uint32 param1 = (uint8)((b >>  8) & 0xFF);
	uint8 cmd    = (uint8)(b & 0xF0);
	uint8 chan   = (uint8)(b & 0x0F);

	switch (cmd) {
	case 0x80:  // Note Off
		fluid_synth_noteoff(_synth, chan, param1);
		break;
	case 0x90:  // Note On
		fluid_synth_noteon(_synth, chan, param1, param2);
		break;
	case 0xA0:  // Aftertouch
		break;
	case 0xB0:  // Control Change
		fluid_synth_cc(_synth, chan, param1, param2);
		break;
	case 0xC0:  // Program Change
		fluid_synth_program_change(_synth, chan, param1);
		break;
	case 0xD0:  // Channel Pressure
		break;
	case 0xE0:  // Pitch Bend
		fluid_synth_pitch_bend(_synth, chan, (param2 << 7) | param1);
		break;
	case 0xF0:  // SysEx
		// We should never get here! SysEx information has to be
		// sent via high-level semantic methods.
		perr << "FluidSynthMidiDriver: Receiving SysEx command on a send() call" << std::endl;
		break;
	default:
		perr << "FluidSynthMidiDriver: Unknown send() command 0x" << std::hex << cmd << std::dec << std::endl;
		break;
	}
}

void FluidSynthMidiDriver::lowLevelProduceSamples(int16 *samples, uint32 num_samples) {
	fluid_synth_write_s16(_synth, num_samples, samples, 0, 2, samples, 1, 2);
}

} // End of namespace Ultima8

#endif
