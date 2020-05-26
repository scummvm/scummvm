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
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/sound/adplug/opl.h"
#include "ultima/nuvie/sound/origin_fx_adib_driver.h"

namespace Ultima {
namespace Nuvie {

const uint8 adlib_BD_cmd_tbl[] = { 0, 1, 0, 1, 0, 1, 16, 8, 4, 2, 1 };


OriginFXAdLibDriver::OriginFXAdLibDriver(Configuration *cfg, Copl *newopl) {

	const uint8 byte_73_init[] = {1, 2, 3, 4, 5, 6, 7, 8, 0xB, 0xFF, 0xFF, 0, 0xC};

	config = cfg;
	opl = newopl;
	adlib_tim_data = NULL;
	adlib_num_active_channels = 9;
	memset(midi_chan_tim_ptr, 0, sizeof(midi_chan_tim_ptr));
	memset(midi_chan_pitch, 0, sizeof(midi_chan_pitch));
	memset(midi_chan_tim_off_10, 0, sizeof(midi_chan_tim_off_10));
	memset(midi_chan_tim_off_11, 0, sizeof(midi_chan_tim_off_11));

	adlib_bd_status = 0;
	memcpy(byte_73, byte_73_init, sizeof(byte_73_init));

	for (int i = 0; i < 29; i++) {
		midi_chan_volume[i] = 0x100;
	}

	memset(adlib_ins, 0, sizeof(adlib_ins));

	for (int i = 0; i < 11; i++) {
		adlib_ins[i].note = -1;
		adlib_ins[i].channel = -1;
		adlib_ins[i].byte_68 = 1;
	}

	load_tim_file();

	init();
}

OriginFXAdLibDriver::~OriginFXAdLibDriver() {
	if (adlib_tim_data)
		delete [] adlib_tim_data;
}

void OriginFXAdLibDriver::init() {
	opl->init();

	for (int i = 0; i < 256; i++) {
		midi_write_adlib(i, 0);
	}

	midi_write_adlib(0x01, 0x20);
	midi_write_adlib(0xBD, 0);
	midi_write_adlib(0x8, 0);
}

sint16 OriginFXAdLibDriver::read_sint16(unsigned char *buf) {
	return (buf[1] << 8) | buf[0];
}

void OriginFXAdLibDriver::load_tim_file() {
	U6Lib_n f;
	Std::string filename;

	nuvie_game_t game_type = get_game_type(config);
	if (game_type == NUVIE_GAME_SE) {
		config_get_path(config, "savage.tim", filename);
	} else { // game_type == NUVIE_GAME_MD
		config_get_path(config, "md.tim", filename);
	}

	f.open(filename, 4, game_type);
	unsigned char *buf = f.get_item(1);
	adlib_tim_data = new unsigned char [f.get_item_size(1) - 1];
	num_tim_records = buf[0];
	memcpy(adlib_tim_data, &buf[1], f.get_item_size(1) - 1);
	free(buf);

	for (int i = 0; i < 32; i++) {
		midi_chan_tim_ptr[i] = adlib_tim_data;
	}

	program_change(0x9 , 0x80);
	program_change(0xa , 0x72);
	program_change(0xb , 0x83);
	program_change(0xc , 0x71);
	program_change(0xd , 0x86);
	program_change(0xe , 0x87);
	program_change(0xf , 0x85);
	program_change(0x10 , 0x84);
	program_change(0x11 , 0x81);
	program_change(0x12 , 0x88);
	program_change(0x13 , 0x8D);
	program_change(0x14 , 0x8F);
	program_change(0x15 , 0x90);
	program_change(0x16 , 0x91);
	program_change(0x17 , 0x93);
	program_change(0x18 , 0x8C);
	program_change(0x19 , 0x8B);
}

unsigned char *OriginFXAdLibDriver::get_tim_data(uint8 program_number) {
	for (int i = 0; i < num_tim_records; i++) {
		if (adlib_tim_data[i * 48 + 0x2f] == program_number) {
			return &adlib_tim_data[i * 48];
		}
	}

	return adlib_tim_data;
}

void OriginFXAdLibDriver::midi_write_adlib(unsigned int r, unsigned char v) {
	opl->write(r, v);
}

void OriginFXAdLibDriver::program_change(sint8 channel, uint8 program_number) {
	unsigned char *tim_data = get_tim_data(program_number);
	int i, j;

	debug("Program change channel: %d program: %d", channel, program_number);
	for (i = 0; i < 11; i++) {
		if (adlib_ins[i].channel == channel) {
			play_note(channel, adlib_ins[i].note, 0); //note off.
			adlib_ins[i].channel = -1;
			adlib_ins[i].tim_data = NULL;

		}
	}

	midi_chan_tim_ptr[channel] = tim_data;
	midi_chan_tim_off_10[channel] = tim_data[0x10];
	midi_chan_tim_off_11[channel] = tim_data[0x11];

	if (tim_data[0xb] != 0 && adlib_num_active_channels == 9) {
		midi_write_adlib(0xa6, 0);
		midi_write_adlib(0xb6, 0);
		midi_write_adlib(0xa7, 0);
		midi_write_adlib(0xb7, 0xa);
		midi_write_adlib(0xa8, 0x54);
		midi_write_adlib(0xb8, 0x9);

		adlib_num_active_channels = 6;
		for (i = 6; i < 9; i++) {
			for (j = 0; j < 13; j++) {
				if (byte_73[j] == i) {
					byte_73[j] = byte_73[i];
					byte_73[i] = (uint8)-1;
					break;
				}
			}
		}
		adlib_bd_status = 0x20;
		midi_write_adlib(0xbd, adlib_bd_status);
	}
}

void OriginFXAdLibDriver::pitch_bend(uint8 channel, uint8 pitch_lsb, uint8 pitch_msb) {
	unsigned char *cur_tim_ptr = midi_chan_tim_ptr[channel];

	midi_chan_pitch[channel] = ((sint16)((pitch_msb << 7) + pitch_lsb - 8192) * cur_tim_ptr[0xe]) / 256;
	debug("pitch_bend: c=%d, pitch=%d %d,%d,%d", channel, midi_chan_pitch[channel], pitch_msb, pitch_lsb, cur_tim_ptr[0xe]);

	for (int i = 0; i < adlib_num_active_channels; i++) {
		if (adlib_ins[i].byte_68 > 1 && adlib_ins[i].channel == channel) {
			sint16 var_4 = 0;

			if (adlib_ins[i].tim_data != NULL) {
				var_4 = read_sint16(&adlib_ins[i].tim_data[0x24]);
			}

			uint16 var_2 = sub_60D(adlib_ins[i].word_3c + midi_chan_pitch[channel] + adlib_ins[i].word_cb + adlib_ins[i].word_121 + var_4);
			var_2 += 0x2000;
			midi_write_adlib(0xa0 + i, var_2 & 0xff);
			midi_write_adlib(0xb0 + i, var_2 >> 8);
		}

	}
}

void OriginFXAdLibDriver::control_mode_change(uint8 channel, uint8 function, uint8 value) {
	uint8 c = channel;
	debug("control_mode_change: c=%d, func=%2x, value=%d", channel, function, value);
	if (c == 9) {
		c++;
		do {
			control_mode_change(c, function, value);
			c++;
		} while (c <= 25);
		c = 9;
	}

	if (function == 1) {
		midi_chan_tim_off_11[channel] = ((((sint16)midi_chan_tim_ptr[channel][0xf]) * value) / 128) + (sint16)midi_chan_tim_ptr[channel][0x11];
	} else if (function == 7) {
		midi_chan_volume[c] = value + 128;
	} else if (function == 0x7b) {
		bool var_6 = false;
		for (int i = 0; i < 0xb; i++) {
			if (adlib_ins[i].byte_68 > 1) {
				if (adlib_ins[i].channel == channel) {
					play_note(channel, adlib_ins[i].note, 0); //note off
				} else if (i >= adlib_num_active_channels) {
					var_6 = true;
				}
			}
		}

		if (var_6 && adlib_num_active_channels < 9) {
			midi_write_adlib(0xbd, 0);
			adlib_num_active_channels = 9;
			byte_73[6] = 7;
			byte_73[7] = 8;
			byte_73[8] = byte_73[0xb];
			byte_73[0xb] = 6;
		}

	} else if (function == 0x79) {
		control_mode_change(channel, 1, 0);
		control_mode_change(channel, 7, 0x7f);
		pitch_bend(channel, 0, 0x40);
	}
}
void OriginFXAdLibDriver::play_note(uint8 channel, sint8 note, uint8 velocity) {
	unsigned char *cur_tim_ptr = midi_chan_tim_ptr[channel];
	for (; cur_tim_ptr != NULL; cur_tim_ptr += 48) {
		sint8 voice = sub_4BF(channel, note, velocity, cur_tim_ptr);
		sint16 var_4 = voice;
		if (voice > 8) {
			var_4 = 0x11 - voice;
		}

		if (voice >= 0) {
			sint16 var_a = read_sint16(&cur_tim_ptr[0x24]);

			if (velocity != 0) {
				adlib_ins[voice].word_121 = 0;
				adlib_ins[voice].byte_137 = 0;
				adlib_ins[voice].word_cb = read_sint16(&cur_tim_ptr[0x12]);
			}

			sint8 cl = cur_tim_ptr[0x27];
			if (cl < 0) {
				adlib_ins[voice].word_3c = (-((sint16)(note - 60) * 256) / (1 << -(cl + 1))) + 0x3c00;
			} else {
				adlib_ins[voice].word_3c = (((sint16)(note - 60) * 256) / (1 << cl)) + 0x3c00;
			}

			uint16 var_2 = sub_60D(adlib_ins[voice].word_3c + midi_chan_pitch[channel] + adlib_ins[voice].word_cb + adlib_ins[voice].word_121 + var_a);
			if (velocity == 0) {
				if (voice < adlib_num_active_channels || voice <= 6) {
					midi_write_adlib(0xa0 + var_4, var_2 & 0xff);
					midi_write_adlib(0xb0 + var_4, var_2 >> 8);
				} else {
					adlib_bd_status &= ~adlib_BD_cmd_tbl[voice];
				}
			} else {
				uint16 var_6 = cur_tim_ptr[6];
				if (cur_tim_ptr[0xc] != 0 || midi_chan_volume[channel] < 0x100) {
					sint16 di = 0x3f - ((midi_chan_volume[channel] * (0x3f - (((sint16)(63 - velocity) / (sint16)(1 << (7 - cur_tim_ptr[0xc]))) + (var_6 & 0x3f)))) >> 8); //fixme this was 0x14 in dosbox var_6 = 8

					di = 63 - velocity;
					di = di / (1 << (7 - cur_tim_ptr[0xc]));
					di += var_6 & 0x3f;

					sint16 ax = (0x3f - di) * midi_chan_volume[channel];
					ax = ax / 256;
					di = 0x3f - ax;

					if (di > 0x3f) {
						di = 0x3f;
					}
					if (di < 0) {
						di = 0;
					}

					midi_write_adlib(0x40 + adlib_voice_op1(voice), (var_6 & 0xc0) + di);
				}

				var_6 = cur_tim_ptr[1];
				if (cur_tim_ptr[0xd] != 0) {
					sint16 di = (0x3f - velocity) / (sint16)(1 << (7 - cur_tim_ptr[0xd])) + (var_6 & 0x3f);
					if (di > 0x3f) {
						di = 0x3f;
					}
					if (di < 0) {
						di = 0;
					}

					midi_write_adlib(0x40 + adlib_voice_op(voice), (var_6 & 0xc0) + di);
				}

				if (cur_tim_ptr[0xb] == 0 || voice == 6) {
					if (cur_tim_ptr[0xb] == 0) {
						var_2 += 0x2000;
					}
					midi_write_adlib(0xa0 + var_4, var_2 & 0xff);
					midi_write_adlib(0xb0 + var_4, var_2 >> 8);
				}

				if (cur_tim_ptr[0xb] != 0) {
					adlib_bd_status |= adlib_BD_cmd_tbl[voice];
				}
			}

			if (cur_tim_ptr[0xb] != 0) {
				midi_write_adlib(0xbd, adlib_bd_status);
			}
		}

		if (cur_tim_ptr[0x26] == 0)
			break;
	}
}

uint16 OriginFXAdLibDriver::sub_60D(sint16 val) {
	static const uint16 word_20f[] = {0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287, 0x2AE, 0x2D7, 0x302, 0x330, 0x360, 0x393, 0x3CA};

	sint16 var_2 = val / 256;

	sint16 si = ((var_2 + 6) / 0xc) - 2;
	if (si > 7) {
		si = 7;
	}
	if (si < 0) {
		si = 0;
	}

	uint16 di = word_20f[(var_2 + 6) % 0xc];
	if ((val & 0xff) != 0) {
		int offset = ((var_2 - 18) % 0xc) + 1;
		// FIXME: This offset is negative near the end of the Savage Empire Origin FX
		// intro.. what should it do?
		if (offset >= 0)
			di += ((word_20f[offset] - di) * (val & 0xff)) / 256;
	}

	return (si << 10) + di;
}

uint16 OriginFXAdLibDriver::sub_4BF(uint8 channel, uint8 note, uint8 velocity, unsigned char *cur_tim_ptr) {
	sint16 si = -1;

	if (adlib_num_active_channels >= 9 || cur_tim_ptr[0xb] == 0) {
		if (velocity == 0) {
			for (si = 0; si < adlib_num_active_channels; si++) {
				if (adlib_ins[si].byte_68 > 1 && adlib_ins[si].note == note && adlib_ins[si].channel == channel && adlib_ins[si].tim_data == cur_tim_ptr) {
					adlib_ins[si].byte_68 = 0;
					sub_45E(si);
					sub_48E(si, 0xb);
					break;
				}
			}
			if (si == adlib_num_active_channels) {
				si = -1;
			}
		} else {
			if (byte_73[0xb] == 0xb) {
				if (midi_chan_tim_ptr[channel] == cur_tim_ptr) {
					si = byte_73[0xc];
					byte_73[0xc] = byte_73[si];
					sub_48E(si, 0xc);
					midi_write_adlib(0xa0 + si, 0);
					midi_write_adlib(0xb0 + si, 0);
				}
			} else {
				si = byte_73[0xb];
				byte_73[0xb] = byte_73[si];
				sub_48E(si, 0xc);
			}

			if (si >= 0) {
				adlib_ins[si].byte_68 = 2;
				adlib_ins[si].note = note;
			}
		}
	} else {
		si = cur_tim_ptr[0xb];
		adlib_bd_status &= ~adlib_BD_cmd_tbl[cur_tim_ptr[0xb]];
		midi_write_adlib(0xbd, adlib_bd_status);
	}

	if (si >= 0) {
		if (adlib_ins[si].channel != channel || adlib_ins[si].tim_data != cur_tim_ptr) { //changing instruments
			write_adlib_instrument(si, cur_tim_ptr);
			adlib_ins[si].channel = channel;
			adlib_ins[si].tim_data = cur_tim_ptr;
		}
	}

	return si;
}

void OriginFXAdLibDriver::sub_45E(sint16 voice) {
	for (int i = 0; i < 0xd; i++) {
		if (byte_73[i] == voice) {
			byte_73[i] = byte_73[voice];
			byte_73[voice] = voice;
		}
	}
}

void OriginFXAdLibDriver::sub_48E(sint16 voice, uint8 val) {
	for (int i = 0; i < 0xd; i++) {
		if (byte_73[i] == val) {
			byte_73[i] = voice;
			byte_73[voice] = val;
			break;
		}
	}
}

uint8 OriginFXAdLibDriver::adlib_voice_op(sint8 voice) {
	const uint8 opp_tbl[] = {0, 1, 2, 8, 9, 0xA, 0x10, 0x11, 0x12, 0, 1, 2, 8, 9, 0xA, 0x10, 0x14, 0x12, 0x15, 0x11};
	return opp_tbl[adlib_num_active_channels < 9 ? voice + 9 : voice];
}

uint8 OriginFXAdLibDriver::adlib_voice_op1(sint8 voice) {
	const uint8 opp1_tbl[] = {3, 4, 5, 0xB, 0xC, 0xD, 0x13, 0x14, 0x15, 3, 4, 5, 0xB, 0xC, 0xD, 0x13, 0x14, 0x12, 0x15, 0x11};
	return opp1_tbl[adlib_num_active_channels < 9 ? voice + 9 : voice];
}

void OriginFXAdLibDriver::write_adlib_instrument(sint8 voice, unsigned char *tim_data) {
	uint8 opadd = adlib_voice_op(voice);
	uint8 opadd1 = adlib_voice_op1(voice);
	unsigned char *cur_tim_ptr = tim_data;

	midi_write_adlib(0x20 + opadd, *cur_tim_ptr++);
	midi_write_adlib(0x40 + opadd, *cur_tim_ptr++);
	midi_write_adlib(0x60 + opadd, *cur_tim_ptr++);
	midi_write_adlib(0x80 + opadd, *cur_tim_ptr++);
	midi_write_adlib(0xe0 + opadd, *cur_tim_ptr++);

	if (adlib_num_active_channels == 9 || tim_data[0xb] < 7) {
		midi_write_adlib(0x20 + opadd1, *cur_tim_ptr++);
		midi_write_adlib(0x40 + opadd1, *cur_tim_ptr++);
		midi_write_adlib(0x60 + opadd1, *cur_tim_ptr++);
		midi_write_adlib(0x80 + opadd1, *cur_tim_ptr++);
		midi_write_adlib(0xe0 + opadd1, *cur_tim_ptr++);
		midi_write_adlib(0xc0 + voice, *cur_tim_ptr++);
	}

}

void OriginFXAdLibDriver::interrupt_vector() {
	const uint8 byte_229[] = {24, 0, 18, 20, 22, 0, 0, 0};

	for (int i = 0; i < adlib_num_active_channels; i++) {
		unsigned char *cur_tim_data = NULL;
		bool update_adlib = false;
		sint8 channel = adlib_ins[i].channel;
		if (channel < 0 || channel >= 32) {
			continue;
		}
		uint8 var_8 = byte_229[adlib_ins[i].byte_68];
		sint16 var_10 = 0;
		if (adlib_ins[i].tim_data == NULL) {
			cur_tim_data = adlib_tim_data;
		} else {
			cur_tim_data = adlib_ins[i].tim_data;
			var_10 = read_sint16(&cur_tim_data[0x24]);
		}

		if (var_8 != 0) {
			sint16 var_a = read_sint16(&cur_tim_data[var_8 * 2 - 16]);
			sint16 var_c = read_sint16(&cur_tim_data[(var_8 + 1) * 2 - 16]);

			sint16 tmp = (var_c > adlib_ins[i].word_cb) ? var_c - adlib_ins[i].word_cb : adlib_ins[i].word_cb - var_c;
			if (tmp >= var_a) {
				if (adlib_ins[i].word_cb >= var_c) {
					adlib_ins[i].word_cb -= var_a;
				} else {
					adlib_ins[i].word_cb += var_a;
				}
			} else {
				adlib_ins[i].word_cb = var_c;
				adlib_ins[i].byte_68++;
			}

			update_adlib = true;
		}

		if (midi_chan_tim_off_10[channel] != 0) {
			adlib_ins[i].byte_137 += midi_chan_tim_off_10[channel];
			sint8 var_11 = adlib_ins[i].byte_137;
			if (var_11 > 63 || var_11 < -64) {
				var_11 = -128 - var_11;
			}

			adlib_ins[i].word_121 = (midi_chan_tim_off_11[channel] * var_11) / 16;
			update_adlib = true;
		}

		if (update_adlib || var_10 != 0) {
			uint16 adlib_cmd_data = sub_60D(adlib_ins[i].word_3c + midi_chan_pitch[channel] + adlib_ins[i].word_cb + adlib_ins[i].word_121 + var_10);
			if (adlib_ins[i].byte_68 > 1) {
				adlib_cmd_data += 0x2000;
			}
			midi_write_adlib(0xa0 + i, adlib_cmd_data & 0xff);
			midi_write_adlib(0xb0 + i, adlib_cmd_data >> 8);
		}
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
