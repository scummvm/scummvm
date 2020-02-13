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

#ifndef NUVIE_SOUND_ORIGIN_FX_ADLIB_DRIVER_H
#define NUVIE_SOUND_ORIGIN_FX_ADLIB_DRIVER_H

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"

namespace Ultima {
namespace Nuvie {

class Copl;
class Configuration;

class OriginFXAdLibDriver {
public:
	OriginFXAdLibDriver(Configuration *cfg, Copl *newopl);
	~OriginFXAdLibDriver();

private:

	Configuration *config;
	Copl *opl;

	unsigned char num_tim_records;
	unsigned char *adlib_tim_data;

	struct adlib_instrument {
		sint8 channel;
		sint8 note;
		uint8 byte_68;
		sint16 word_121;
		uint8 byte_137;
		sint16 word_cb;
		sint16 word_3c;
		unsigned char *tim_data;
	};

	adlib_instrument adlib_ins[11];

	int adlib_num_active_channels; //either 6 or 9.
	unsigned char *midi_chan_tim_ptr[32];
	uint8 midi_chan_tim_off_10[32];
	sint16 midi_chan_tim_off_11[32];
	sint16 midi_chan_pitch[32];
	sint16 midi_chan_volume[29];

	uint8 byte_73[13];
	uint8 adlib_bd_status;

public:
	void init();
	void play_note(uint8 channel, sint8 note, uint8 velocity);
	void control_mode_change(uint8 channel, uint8 function, uint8 value);
	void program_change(sint8 channel, uint8 program_number);
	void pitch_bend(uint8 channel, uint8 pitch_lsb, uint8 pitch_msb);
	void interrupt_vector();

private:
	sint16 read_sint16(unsigned char *buf);
	void midi_write_adlib(unsigned int r, unsigned char v);

	void load_tim_file();
	unsigned char *get_tim_data(uint8 program_number);

	uint8 adlib_voice_op(sint8 voice);
	uint8 adlib_voice_op1(sint8 voice);
	uint16 sub_60D(sint16 val);
	uint16 sub_4BF(uint8 channel, uint8 note, uint8 velocity, unsigned char *cur_tim_ptr);
	void sub_45E(sint16 voice);
	void sub_48E(sint16 voice, uint8 val);
	void write_adlib_instrument(sint8 voice, unsigned char *tim_data);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
