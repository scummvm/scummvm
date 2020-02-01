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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/sound/adplug/u6m.h"

namespace Ultima {
namespace Nuvie {

Cu6mPlayer::~Cu6mPlayer() {
	if (song_data)
		free(song_data);
}

CPlayer *Cu6mPlayer::factory(Copl *newopl) {
	return new Cu6mPlayer(newopl);
}

bool Cu6mPlayer::load(const Std::string &filename) {
	uint32 decompressed_filesize;
	U6Lzw lzw;

	song_data = lzw.decompress_file(filename, decompressed_filesize);

	rewind(0);
	return (true);
}


bool Cu6mPlayer::update() {
	if (!driver_active) {
		driver_active = true;
		dec_clip(read_delay);
		if (read_delay == 0) {
			command_loop();
		}

		// on all Adlib channels: freq slide/vibrato, mute factor slide
		for (int i = 0; i < 9; i++) {
			if (channel_freq_signed_delta[i] != 0)
				// frequency slide + mute factor slide
			{
				// freq slide
				freq_slide(i);

				// mute factor slide
				if (carrier_mf_signed_delta[i] != 0) {
					mf_slide(i);
				}
			} else
				// vibrato + mute factor slide
			{
				// vibrato
				if ((vb_multiplier[i] != 0) && ((channel_freq[i].hi & 0x20) == 0x20)) {
					vibrato(i);
				}

				// mute factor slide
				if (carrier_mf_signed_delta[i] != 0) {
					mf_slide(i);
				}
			}
		}

		driver_active = false;
	}

	return !songend;
}


void Cu6mPlayer::rewind(int subsong) {
	played_ticks = 0;
	songend = false;

	// set the driver's internal variables
	byte_pair freq_word = {0, 0};

	driver_active = false;
	song_pos = 0;
	loop_position = 0;   // position of the loop point
	read_delay = 0;      // delay (in timer ticks) before further song data is read

	for (int i = 0; i < 9; i++) {
		// frequency
		channel_freq_signed_delta[i] = 0;
		channel_freq[i] = freq_word;  // Adlib freq settings for each channel

		// vibrato ("vb")
		vb_current_value[i] = 0;
		vb_double_amplitude[i] = 0;
		vb_multiplier[i] = 0;
		vb_direction_flag[i] = 0;

		// mute factor ("mf") == ~(volume)
		carrier_mf[i] = 0;
		carrier_mf_signed_delta[i] = 0;
		carrier_mf_mod_delay_backup[i] = 0;
		carrier_mf_mod_delay[i] = 0;
	}

	while (!subsong_stack.empty())      // empty subsong stack
		subsong_stack.pop();

	opl->init();
	out_adlib(1, 32);   // go to OPL2 mode
}


float Cu6mPlayer::getrefresh() {
	return ((float)60);   // the Ultima 6 music driver expects to be called at 60 Hz
}

// ============================================================================================
//
//
//    Functions called by update()
//
//
// ============================================================================================


// This function reads the song data and executes the embedded commands.
void Cu6mPlayer::command_loop() {
	unsigned char command_byte;   // current command byte
	int command_nibble_hi;        // command byte, bits 4-7
	int command_nibble_lo;        // command byte, bite 0-3
	bool repeat_loop = true;      //

	do {
		// extract low and high command nibbles
		command_byte = read_song_byte();   // implicitly increments song_pos
		command_nibble_hi = command_byte >> 4;
		command_nibble_lo = command_byte & 0xf;

		switch (command_nibble_hi) {
		case 0x0:
			command_0(command_nibble_lo);
			break;
		case 0x1:
			command_1(command_nibble_lo);
			break;
		case 0x2:
			command_2(command_nibble_lo);
			break;
		case 0x3:
			command_3(command_nibble_lo);
			break;
		case 0x4:
			command_4(command_nibble_lo);
			break;
		case 0x5:
			command_5(command_nibble_lo);
			break;
		case 0x6:
			command_6(command_nibble_lo);
			break;
		case 0x7:
			command_7(command_nibble_lo);
			break;
		case 0x8:
			switch (command_nibble_lo) {
			case 1:
				command_81();
				break;
			case 2:
				command_82();
				repeat_loop = false;
				break;
			case 3:
				command_83();
				break;
			case 5:
				command_85();
				break;
			case 6:
				command_86();
				break;
			default:
				break; // maybe generate an error?
			}
			break;
		case 0xE:
			command_E();
			break;
		case 0xF:
			command_F();
			break;
		default:
			break; // maybe generate an error?
		}

	} while (repeat_loop);
}


// --------------------------------------------------------
//    The commands supported by the U6 music file format
// --------------------------------------------------------

// ----------------------------------------
// Set octave and frequency, note off
// Format: 0c nn
// c = channel, nn = packed Adlib frequency
// ----------------------------------------
void Cu6mPlayer::command_0(int channel) {
	unsigned char freq_byte;
	byte_pair freq_word;

	freq_byte = read_song_byte();
	freq_word = expand_freq_byte(freq_byte);
	set_adlib_freq(channel, freq_word);
}


// ---------------------------------------------------
// Set octave and frequency, old note off, new note on
// Format: 1c nn
// c = channel, nn = packed Adlib frequency
// ---------------------------------------------------
void Cu6mPlayer::command_1(int channel) {
	unsigned char freq_byte;
	byte_pair freq_word;

	vb_direction_flag[channel] = 0;
	vb_current_value[channel] = 0;

	freq_byte = read_song_byte();
	freq_word = expand_freq_byte(freq_byte);
	set_adlib_freq(channel, freq_word);

	freq_word.hi = freq_word.hi | 0x20; // note on
	set_adlib_freq(channel, freq_word);
}


// ----------------------------------------
// Set octave and frequency, note on
// Format: 2c nn
// c = channel, nn = packed Adlib frequency
// ----------------------------------------
void Cu6mPlayer::command_2(int channel) {
	unsigned char freq_byte;
	byte_pair freq_word;

	freq_byte = read_song_byte();
	freq_word = expand_freq_byte(freq_byte);
	freq_word.hi = freq_word.hi | 0x20; // note on
	set_adlib_freq(channel, freq_word);
}


// --------------------------------------
// Set "carrier mute factor"==not(volume)
// Format: 3c nn
// c = channel, nn = mute factor
// --------------------------------------
void Cu6mPlayer::command_3(int channel) {
	unsigned char mf_byte;

	carrier_mf_signed_delta[channel] = 0;
	mf_byte = read_song_byte();
	set_carrier_mf(channel, mf_byte);
}


// ----------------------------------------
// set "modulator mute factor"==not(volume)
// Format: 4c nn
// c = channel, nn = mute factor
// ----------------------------------------
void Cu6mPlayer::command_4(int channel) {
	unsigned char mf_byte;

	mf_byte = read_song_byte();
	set_modulator_mf(channel, mf_byte);
}


// --------------------------------------------
// Set portamento (pitch slide)
// Format: 5c nn
// c = channel, nn = signed channel pitch delta
// --------------------------------------------
void Cu6mPlayer::command_5(int channel) {
	channel_freq_signed_delta[channel] = read_signed_song_byte();
}


// --------------------------------------------
// Set vibrato paramters
// Format: 6c mn
// c = channel
// m = vibrato double amplitude
// n = vibrato multiplier
// --------------------------------------------
void Cu6mPlayer::command_6(int channel) {
	unsigned char vb_parameters;

	vb_parameters = read_song_byte();
	vb_double_amplitude[channel] = vb_parameters >> 4; // high nibble
	vb_multiplier[channel] = vb_parameters & 0xF; // low nibble
}


// ----------------------------------------
// Assign Adlib instrument to Adlib channel
// Format: 7c nn
// c = channel, nn = instrument number
// ----------------------------------------
void Cu6mPlayer::command_7(int channel) {
	int instrument_offset = instrument_offsets[read_song_byte()];
	out_adlib_opcell(channel, false, 0x20, *(song_data + instrument_offset + 0));
	out_adlib_opcell(channel, false, 0x40, *(song_data + instrument_offset + 1));
	out_adlib_opcell(channel, false, 0x60, *(song_data + instrument_offset + 2));
	out_adlib_opcell(channel, false, 0x80, *(song_data + instrument_offset + 3));
	out_adlib_opcell(channel, false, 0xE0, *(song_data + instrument_offset + 4));
	out_adlib_opcell(channel, true, 0x20, *(song_data + instrument_offset + 5));
	out_adlib_opcell(channel, true, 0x40, *(song_data + instrument_offset + 6));
	out_adlib_opcell(channel, true, 0x60, *(song_data + instrument_offset + 7));
	out_adlib_opcell(channel, true, 0x80, *(song_data + instrument_offset + 8));
	out_adlib_opcell(channel, true, 0xE0, *(song_data + instrument_offset + 9));
	out_adlib(0xC0 + channel, *(song_data + instrument_offset + 10));
}


// -------------------------------------------
// Branch to a new subsong
// Format: 81 nn aa bb
// nn == number of times to repeat the subsong
// aa == subsong offset (low byte)
// bb == subsong offset (high byte)
// -------------------------------------------
void Cu6mPlayer::command_81() {
	subsong_info new_ss_info;

	new_ss_info.subsong_repetitions = read_song_byte();
	new_ss_info.subsong_start = read_song_byte();
	new_ss_info.subsong_start += read_song_byte() << 8;
	new_ss_info.continue_pos = song_pos;

	subsong_stack.push(new_ss_info);
	song_pos = new_ss_info.subsong_start;
}


// ------------------------------------------------------------
// Stop interpreting commands for this timer tick
// Format: 82 nn
// nn == delay (in timer ticks) until further data will be read
// ------------------------------------------------------------
void Cu6mPlayer::command_82() {
	read_delay = read_song_byte();
}


// -----------------------------
// Adlib instrument data follows
// Format: 83 nn <11 bytes>
// nn == instrument number
// -----------------------------
void Cu6mPlayer::command_83() {
	unsigned char instrument_number = read_song_byte();
	instrument_offsets[instrument_number] = song_pos;
	song_pos += 11;
}


// ----------------------------------------------
// Set -1 mute factor slide (upward volume slide)
// Format: 85 cn
// c == channel
// n == slide delay
// ----------------------------------------------
void Cu6mPlayer::command_85() {
	unsigned char data_byte = read_song_byte();
	int channel = data_byte >> 4; // high nibble
	unsigned char slide_delay = data_byte & 0xF; // low nibble
	carrier_mf_signed_delta[channel] = +1;
	carrier_mf_mod_delay[channel] = slide_delay + 1;
	carrier_mf_mod_delay_backup[channel] = slide_delay + 1;
}


// ------------------------------------------------
// Set +1 mute factor slide (downward volume slide)
// Format: 86 cn
// c == channel
// n == slide speed
// ------------------------------------------------
void Cu6mPlayer::command_86() {
	unsigned char data_byte = read_song_byte();
	int channel = data_byte >> 4; // high nibble
	unsigned char slide_delay = data_byte & 0xF; // low nibble
	carrier_mf_signed_delta[channel] = -1;
	carrier_mf_mod_delay[channel] = slide_delay + 1;
	carrier_mf_mod_delay_backup[channel] = slide_delay + 1;
}


// --------------
// Set loop point
// Format: E?
// --------------
void Cu6mPlayer::command_E() {
	loop_position = song_pos;
}


// ---------------------------
// Return from current subsong
// Format: F?
// ---------------------------
void Cu6mPlayer::command_F() {
	if (!subsong_stack.empty()) {
		subsong_info temp = subsong_stack.top();
		subsong_stack.pop();
		temp.subsong_repetitions--;
		if (temp.subsong_repetitions == 0) {
			song_pos = temp.continue_pos;
		} else {
			song_pos = temp.subsong_start;
			subsong_stack.push(temp);
		}
	} else {
		song_pos = loop_position;
		songend = true;
	}
}


// --------------------
// Additional functions
// --------------------

// This function decrements its argument, without allowing it to become negative.
void Cu6mPlayer::dec_clip(int &param) {
	param--;
	if (param < 0) {
		param = 0;
	}
}


// Returns the byte at the current song position.
// Side effect: increments song_pos.
unsigned char Cu6mPlayer::read_song_byte() {
	unsigned char song_byte;
	song_byte = song_data[song_pos];
	song_pos++;
	return (song_byte);
}


// Same as read_song_byte(), except that it returns a signed byte
signed char Cu6mPlayer::read_signed_song_byte() {
	unsigned char song_byte;
	int signed_value;
	song_byte = *(song_data + song_pos);
	song_pos++;
	if (song_byte <= 127) {
		signed_value = song_byte;
	} else {
		signed_value = (int)song_byte - 0x100;
	}
	return ((signed char)signed_value);
}


Cu6mPlayer::byte_pair Cu6mPlayer::expand_freq_byte(unsigned char freq_byte) {
	const byte_pair freq_table[24] = {
		{0x00, 0x00}, {0x58, 0x01}, {0x82, 0x01}, {0xB0, 0x01},
		{0xCC, 0x01}, {0x03, 0x02}, {0x41, 0x02}, {0x86, 0x02},
		{0x00, 0x00}, {0x6A, 0x01}, {0x96, 0x01}, {0xC7, 0x01},
		{0xE4, 0x01}, {0x1E, 0x02}, {0x5F, 0x02}, {0xA8, 0x02},
		{0x00, 0x00}, {0x47, 0x01}, {0x6E, 0x01}, {0x9A, 0x01},
		{0xB5, 0x01}, {0xE9, 0x01}, {0x24, 0x02}, {0x66, 0x02}
	};

	int packed_freq;
	int octave;
	byte_pair freq_word;

	packed_freq = freq_byte & 0x1F;
	octave = freq_byte >> 5;

	// range check (not present in the original U6 music driver)
	if (packed_freq >= 24) {
		packed_freq = 0;
	}

	freq_word.hi = freq_table[packed_freq].hi + (octave << 2);
	freq_word.lo = freq_table[packed_freq].lo;

	return (freq_word);
}


void Cu6mPlayer::set_adlib_freq(int channel, Cu6mPlayer::byte_pair freq_word) {
	out_adlib(0xA0 + channel, freq_word.lo);
	out_adlib(0xB0 + channel, freq_word.hi);
	// update the Adlib register backups
	channel_freq[channel] = freq_word;
}


// this function sets the Adlib frequency, but does not update the register backups
void Cu6mPlayer::set_adlib_freq_no_update(int channel, Cu6mPlayer::byte_pair freq_word) {
	out_adlib(0xA0 + channel, freq_word.lo);
	out_adlib(0xB0 + channel, freq_word.hi);
}


void Cu6mPlayer::set_carrier_mf(int channel, unsigned char mute_factor) {
	out_adlib_opcell(channel, true, 0x40, mute_factor);
	carrier_mf[channel] = mute_factor;
}


void Cu6mPlayer::set_modulator_mf(int channel, unsigned char mute_factor) {
	out_adlib_opcell(channel, false, 0x40, mute_factor);
}


void Cu6mPlayer::freq_slide(int channel) {
	byte_pair freq = channel_freq[channel];

	long freq_word = freq.lo + (freq.hi << 8) + channel_freq_signed_delta[channel];
	if (freq_word < 0) {
		freq_word += 0x10000;
	}
	if (freq_word > 0xFFFF) {
		freq_word -= 0x10000;
	}

	freq.lo = freq_word & 0xFF;
	freq.hi = (freq_word >> 8) & 0xFF;
	set_adlib_freq(channel, freq);
}


void Cu6mPlayer::vibrato(int channel) {
	byte_pair freq;

	if (vb_current_value[channel] >= vb_double_amplitude[channel]) {
		vb_direction_flag[channel] = 1;
	} else if (vb_current_value[channel] <= 0) {
		vb_direction_flag[channel] = 0;
	}

	if (vb_direction_flag[channel] == 0) {
		vb_current_value[channel]++;
	} else {
		vb_current_value[channel]--;
	}

	long freq_word = channel_freq[channel].lo + (channel_freq[channel].hi << 8);
	freq_word += (vb_current_value[channel] - (vb_double_amplitude[channel] >> 1))
	             * vb_multiplier[channel];
	if (freq_word < 0) {
		freq_word += 0x10000;
	}
	if (freq_word > 0xFFFF) {
		freq_word -= 0x10000;
	}

	freq.lo = freq_word & 0xFF;
	freq.hi = (freq_word >> 8) & 0xFF;
	set_adlib_freq_no_update(channel, freq);
}


void Cu6mPlayer::mf_slide(int channel) {
	carrier_mf_mod_delay[channel]--;
	if (carrier_mf_mod_delay[channel] == 0) {
		carrier_mf_mod_delay[channel] = carrier_mf_mod_delay_backup[channel];
		int current_mf = carrier_mf[channel] + carrier_mf_signed_delta[channel];
		if (current_mf > 0x3F) {
			current_mf = 0x3F;
			carrier_mf_signed_delta[channel] = 0;
		} else if (current_mf < 0) {
			current_mf = 0;
			carrier_mf_signed_delta[channel] = 0;
		}

		set_carrier_mf(channel, (unsigned char)current_mf);
	}
}


void Cu6mPlayer::out_adlib(unsigned char adlib_register, unsigned char adlib_data) {
	opl->write(adlib_register, adlib_data);
}


void Cu6mPlayer::out_adlib_opcell(int channel, bool carrier, unsigned char adlib_register, unsigned char out_byte) {
	const unsigned char adlib_channel_to_carrier_offset[9] =
	{0x03, 0x04, 0x05, 0x0B, 0x0C, 0x0D, 0x13, 0x14, 0x15};
	const unsigned char adlib_channel_to_modulator_offset[9] =
	{0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12};

	if (carrier) {
		out_adlib(adlib_register + adlib_channel_to_carrier_offset[channel], out_byte);
	} else {
		out_adlib(adlib_register + adlib_channel_to_modulator_offset[channel], out_byte);
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
