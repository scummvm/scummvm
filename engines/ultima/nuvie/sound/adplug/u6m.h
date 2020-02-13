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

#ifndef NUVIE_SOUND_ADPLUG_U6M
#define NUVIE_SOUND_ADPLUG_U6M

#include "ultima/nuvie/sound/adplug/adplug_player.h"
#include "common/stack.h"

namespace Ultima {
namespace Nuvie {

class Cu6mPlayer: public CPlayer {
public:
	static CPlayer *factory(Copl *newopl);

	Cu6mPlayer(Copl *newopl) : CPlayer(newopl), song_data(0), driver_active(0),
		songend(0), song_pos(0), loop_position(0), read_delay(0) {
	}

	~Cu6mPlayer() override;



	bool load(const Std::string &filename) override;
	bool update() override;
	void rewind(int subsong) override;
	float getrefresh() override;

	Std::string gettype() override {
		return Std::string("Ultima 6 Music");
	};


protected:

	struct byte_pair {
		unsigned char lo;
		unsigned char hi;
	};

	struct subsong_info { // information about a subsong
		int continue_pos;
		int subsong_repetitions;
		int subsong_start;
	};

	struct data_block { //
		long size;
		unsigned char *data;
	};


	// class variables
	long played_ticks;

	unsigned char *song_data;   // the uncompressed .m file (the "song")
	bool driver_active;         // flag to prevent reentrancy
	bool songend;               // indicates song end
	int song_pos;               // current offset within the song
	int loop_position;          // position of the loop point
	int read_delay;             // delay (in timer ticks) before further song data is read
	Common::Stack<subsong_info> subsong_stack;

	int instrument_offsets[9];  // offsets of the adlib instrument data
	// vibrato ("vb")
	unsigned char vb_current_value[9];
	unsigned char vb_double_amplitude[9];
	unsigned char vb_multiplier[9];
	unsigned char vb_direction_flag[9];
	// mute factor ("mf") = not(volume)
	unsigned char carrier_mf[9];
	signed char carrier_mf_signed_delta[9];
	unsigned char carrier_mf_mod_delay_backup[9];
	unsigned char carrier_mf_mod_delay[9];
	// frequency
	byte_pair channel_freq[9];  // adlib freq settings for each channel
	signed char channel_freq_signed_delta[9];

	// protected functions used by update()
	void command_loop();
	unsigned char read_song_byte();
	signed char read_signed_song_byte();
	void dec_clip(int &);
	byte_pair expand_freq_byte(unsigned char);
	void set_adlib_freq(int channel, byte_pair freq_word);
	void set_adlib_freq_no_update(int channel, byte_pair freq_word);
	void set_carrier_mf(int channel, unsigned char mute_factor);
	void set_modulator_mf(int channel, unsigned char mute_factor);
	void freq_slide(int channel);
	void vibrato(int channel);
	void mf_slide(int channel);

	void command_0(int channel);
	void command_1(int channel);
	void command_2(int channel);
	void command_3(int channel);
	void command_4(int channel);
	void command_5(int channel);
	void command_6(int channel);
	void command_7(int channel);
	void command_81();
	void command_82();
	void command_83();
	void command_85();
	void command_86();
	void command_E();
	void command_F();

	void out_adlib(unsigned char adlib_register, unsigned char adlib_data);
	void out_adlib_opcell(int channel, bool carrier, unsigned char adlib_register, unsigned char out_byte);

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
