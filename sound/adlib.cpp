/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "scumm.h"
#include "sound.h"
#include "fmopl.h"

#if defined USE_ADLIB

static byte lookup_table[64][32];
const byte volume_table[] = {
0, 4, 7, 11,
13, 16, 18, 20,
22, 24, 26, 27,
29, 30, 31, 33,
34, 35, 36, 37,
38, 39, 40, 41,
42, 43, 44, 44,
45, 46, 47, 47,
48, 49, 49, 50,
51, 51, 52, 53,
53, 54, 54, 55,
55, 56, 56, 57,
57, 58, 58, 59,
59, 60, 60, 60,
61, 61, 62, 62,
62, 63, 63, 63
};

int lookup_volume(int a, int b) {
	if (b==0)
		return 0;

	if (b==31)
		return a;
	
	if (a<-63 || a>63) {
		return b * (a+1) >> 5;
	}
	
	if (b<0) {
		if (a<0) {
			return lookup_table[-a][-b];
		} else {
			return -lookup_table[a][-b];
		}
	} else {
		if (a<0) {
			return -lookup_table[-a][b];
		} else {
			return lookup_table[a][b];
		}
	}
}

void create_lookup_table() {
	int i,j;
	int sum;

	for (i=0; i<64; i++) {
		sum = i;
		for (j=0; j<32; j++) {
			lookup_table[i][j] = sum >> 5;
			sum += i;
		}
	}
	for (i=0; i<64; i++)
		lookup_table[i][0] = 0;
}

MidiChannelAdl *AdlibSoundDriver::allocate_midichan(byte pri) {
	MidiChannelAdl *ac,*best=NULL;
	int i;

	for (i=0; i<9; i++) {
		if (++_midichan_index >= 9)
			_midichan_index = 0;
		ac = &_midi_channels[_midichan_index];
		if (!ac->_part)
			return ac;
		if (!ac->_next) {
			if (ac->_part->_pri_eff <= pri) {
				pri = ac->_part->_pri_eff;
				best = ac;
			}
		}
	}

	if (best)
		mc_off(best);
	else
		;//debug(1, "Denying adlib channel request");
	return best;
}

void AdlibSoundDriver::init(SoundEngine *eng) {
	int i;
	MidiChannelAdl *mc;

	_se = eng;

	for(i=0,mc=_midi_channels; i!=ARRAYSIZE(_midi_channels);i++,mc++) {
		mc->_channel = i;
		mc->_s11a.s10 = &mc->_s10b;
		mc->_s11b.s10 = &mc->_s10a;
	}

	_adlib_reg_cache = (byte*)calloc(256,1);
	_opl = OPLCreate(OPL_TYPE_YM3812,3579545,22050);
	adlib_write(1,0x20);
	adlib_write(8,0x40);
	adlib_write(0xBD, 0x00);
	create_lookup_table();
}

void AdlibSoundDriver::adlib_write(byte port, byte value) {
	if (_adlib_reg_cache[port] == value)
		return;
	_adlib_reg_cache[port] = value;

	OPLWriteReg(_opl, port, value);
}

void AdlibSoundDriver::adlib_key_off(int chan) {
	byte port = chan + 0xB0;
	adlib_write(port, adlib_read(port)&~0x20);
}

struct AdlibSetParams {
	byte a,b,c,d;
};

static const byte channel_mappings[9] = {
	0, 1, 2, 8,
	9,10,16,17,
	18
};

static const byte channel_mappings_2[9] = {
	3, 4, 5, 11,
	12,13,19,20,
	21
};

static const AdlibSetParams adlib_setparam_table[] = {
{0x40,0,63,63}, /* level */
{0xE0,2,0,0},   /* unused */
{0x40,6,192,0}, /* level key scaling */
{0x20,0,15,0},  /* modulator frequency multiple */
{0x60,4,240,15},/* attack rate */
{0x60,0,15,15}, /* decay rate */
{0x80,4,240,15}, /* sustain level */
{0x80,0,15,15}, /* release rate */
{0xE0,0,3,0},   /* waveform select */
{0x20,7,128,0}, /* amp mod */
{0x20,6,64,0},  /* vib */
{0x20,5,32,0},  /* eg typ */
{0x20,4,16,0},  /* ksr */
{0xC0,0,1,0},   /* decay alg */
{0xC0,1,14,0}   /* feedback */
};

void AdlibSoundDriver::adlib_set_param(int channel, byte param, int value) {
	const AdlibSetParams *as;
	byte port;

	assert(channel>=0 && channel<9);

	if (param <= 12) {
		port = channel_mappings_2[channel];
	} else if (param <= 25) {
		param -= 13;
		port = channel_mappings[channel];
	} else if (param <= 27) {
		param -= 13;
		port = channel;
	} else if (param==28 || param==29) {
		if (param==28)
			value -= 15;
		else
			value -= 383;
		value <<= 4;
		channel_table_2[channel] = value;
		adlib_playnote(channel, curnote_table[channel] + value);
		return;
	}else {
		return;
	}

	as = &adlib_setparam_table[param];
	if (as->d)
		value = as->d - value;
	port += as->a;
	adlib_write(port, (adlib_read(port) & ~as->c) | (((byte)value)<<as->b));
}

static const byte octave_numbers[] = {
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1,
2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3,
4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 5, 5, 5, 5,
5, 5, 5, 5, 5, 5, 5, 5,
6, 6, 6, 6, 6, 6, 6, 6,
6, 6, 6, 6, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7
};

static const byte note_numbers[]= {
 3,   4,   5,   6,   7,   8,   9,  10,
11,  12,  13,  14,   3,   4,   5,   6,
 7,   8,   9,  10,  11,  12,  13,  14,
 3,   4,   5,   6,   7,   8,   9,  10,
11,  12,  13,  14,   3,   4,   5,   6,
 7,   8,   9,  10,  11,  12,  13,  14,
 3,   4,   5,   6,   7,   8,   9,  10,
11,  12,  13,  14,   3,   4,   5,   6,
 7,   8,   9,  10,  11,  12,  13,  14,
 3,   4,   5,   6,   7,   8,   9,  10,
11,  12,  13,  14,   3,   4,   5,   6,
 7,   8,   9,  10,  11,  12,  13,  14,
 3,   4,   5,   6,   7,   8,   9,  10,
11,  12,  13,  14,   3,   4,   5,   6,
 7,   8,   9,  10,  11,  12,  13,  14,
 3,   4,   5,   6,   7,   8,   9,  10
};

static const byte note_to_f_num[] = {
 90,  91,  92,  92,  93,  94,  94,  95,
 96,  96,  97,  98,  98,  99, 100, 101,
101, 102, 103, 104, 104, 105, 106, 107,
107, 108, 109, 110, 111, 111, 112, 113,
114, 115, 115, 116, 117, 118, 119, 120,
121, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 132, 133, 134,
135, 136, 137, 138, 139, 140, 141, 142,
143, 145, 146, 147, 148, 149, 150, 151,
152, 153, 154, 155, 157, 158, 159, 160,
161, 162, 163, 165, 166, 167, 168, 169,
171, 172, 173, 174, 176, 177, 178, 180,
181, 182, 184, 185, 186, 188, 189, 190,
192, 193, 194, 196, 197, 199, 200, 202,
203, 205, 206, 208, 209, 211, 212, 214,
215, 217, 218, 220, 222, 223, 225, 226,
228, 230, 231, 233, 235, 236, 238, 240,
242, 243, 245, 247, 249, 251, 252, 254,
};

void AdlibSoundDriver::adlib_playnote(int channel, int note) {
	byte old,oct,notex;
	int note2;
	int i;

	note2 = (note>>7) - 4;

	oct = octave_numbers[note2]<<2;
	notex = note_numbers[note2];

	old = adlib_read(channel + 0xB0);
	if (old&0x20) {
		old &= ~0x20;
		if (oct > old) {
			if (notex < 6) {
				notex += 12;
				oct -= 4;
			}
		} else if (oct < old) {
			if (notex > 11) {
				notex -= 12;
				oct += 4;
			}
		}
	}
	
	i = (notex<<3) + ((note>>4)&0x7);
	adlib_write(channel + 0xA0, note_to_f_num[i]);
	adlib_write(channel + 0xB0, oct|0x20);
}

void AdlibSoundDriver::adlib_note_on(int chan, byte note, int mod) {
	int code;
	assert(chan>=0 && chan<9);
	code = (note<<7) + mod;
	curnote_table[chan] = code;
	adlib_playnote(chan, channel_table_2[chan] + code);
}

void AdlibSoundDriver::adlib_note_on_ex(int chan, byte note, int mod) {
	int code;
	assert(chan>=0 && chan<9);
	code = (note<<7) + mod;
	curnote_table[chan] = code;
	channel_table_2[chan] = 0;
	adlib_playnote(chan, code);
}

void AdlibSoundDriver::adlib_key_onoff(int channel) {
	byte val;
	byte port = channel + 0xB0;
	assert(channel>=0 && channel<9);

	val = adlib_read(port);
	adlib_write(port, val&~0x20);
	adlib_write(port, val|0x20);
}

void AdlibSoundDriver::adlib_setup_channel(int chan, Instrument *instr, byte vol_1, byte vol_2) {
	byte port;

	assert(chan>=0 && chan<9);

	port = channel_mappings[chan];
	adlib_write(port + 0x20, instr->flags_1);
	adlib_write(port + 0x40, (instr->oplvl_1|0x3F) - vol_1);
	adlib_write(port + 0x60, ~instr->atdec_1);
	adlib_write(port + 0x80, ~instr->sustrel_1);
	adlib_write(port + 0xE0, instr->waveform_1);
	
	port = channel_mappings_2[chan];
	adlib_write(port + 0x20, instr->flags_2);
	adlib_write(port + 0x40, (instr->oplvl_2|0x3F) - vol_2);
	adlib_write(port + 0x60, ~instr->atdec_2);
	adlib_write(port + 0x80, ~instr->sustrel_2);
	adlib_write(port + 0xE0, instr->waveform_2);

	adlib_write((byte)chan + 0xC0, instr->feedback);
}

int AdlibSoundDriver::adlib_read_param(int chan, byte param) {
	const AdlibSetParams *as;
	byte val;
	byte port;

	assert(chan>=0 && chan<9);

	if (param <= 12) {
		port = channel_mappings_2[chan];
	} else if (param <= 25) {
		param -= 13;
		port = channel_mappings[chan];
	} else if (param <= 27) {
		param -= 13;
		port = chan;
	} else if (param==28) {
		return 0xF;
	} else if (param==29) {
		return 0x17F;
	} else {
		return 0;
	}

	as = &adlib_setparam_table[param];
	val = adlib_read(port + as->a);
	val &= as->c;
	val >>= as->b;
	if (as->d)
		val = as->d - val;

	return val;
}

void AdlibSoundDriver::generate_samples(int16 *data, int len) {
	int step;

	if (!_opl) {
		memset(data, 0, len*sizeof(int16));
		return;
	}

	do {
		step = len;
		if (step > _next_tick)
			step = _next_tick;
		YM3812UpdateOne(_opl,data,step);
		
		if(!(_next_tick -= step)) {
			_se->on_timer();
			reset_tick();
		}
		data += step;
	} while (len-=step);
}

void AdlibSoundDriver::reset_tick() {
	_next_tick = 88;
}

void AdlibSoundDriver::on_timer() {
	MidiChannelAdl *mc;
	int i;

	_adlib_timer_counter += 0xD69;
	while (_adlib_timer_counter >= 0x411B) {
		_adlib_timer_counter -= 0x411B;
		mc = _midi_channels;
		for (i=0; i!=ARRAYSIZE(_midi_channels); i++,mc++) {
			if (!mc->_part)
				continue;
			if (mc->_duration && (mc->_duration -= 0x11) <= 0) {
				mc_off(mc);
				return;
			}
			if (mc->_s10a.active) {
				mc_inc_stuff(mc, &mc->_s10a, &mc->_s11a);
			}
			if (mc->_s10b.active) {
				mc_inc_stuff(mc, &mc->_s10b, &mc->_s11b);
			}
		}
	}
}

const byte param_table_1[16] = {
29,28,27,0,
3,4,7,8,
13,16,17,20,
21,30,31,0
};

const uint16 param_table_2[16] = {
0x2FF,0x1F,0x7,0x3F,
0x0F,0x0F,0x0F,0x3,
0x3F,0x0F,0x0F,0x0F,
0x3,0x3E,0x1F, 0
};

static const uint16 num_steps_table[] = {
1, 2, 4, 5,
6, 7, 8, 9,
10, 12, 14, 16,
18, 21, 24, 30,
36, 50, 64, 82,
100, 136, 160, 192,
240, 276, 340, 460,
600, 860, 1200, 1600
};

int AdlibSoundDriver::random_nr(int a) {
	static byte _rand_seed = 1;
	if (_rand_seed&1) {
		_rand_seed>>=1;
		_rand_seed ^= 0xB8;
	} else {
		_rand_seed>>=1;
	}
	return _rand_seed * a >> 8;
}

void AdlibSoundDriver::struct10_setup(Struct10 *s10) {
	int b,c,d,e,f,g,h;
	byte t;

	b = s10->unk3;
	f = s10->active - 1;

	t = s10->table_a[f];
	e = num_steps_table[lookup_table[t&0x7F][b]];
	if (t&0x80) {
		e = random_nr(e);
	}
	if (e==0)
		e++;
	
	s10->num_steps = s10->speed_lo_max = e;

	if (f != 2) {
		c = s10->param;
		g = s10->start_value;
		t = s10->table_b[f];
		d = lookup_volume(c, (t&0x7F) - 31);
		if (t&0x80) {
			d = random_nr(d);
		}
		if (d+g > c) {
			h = c - g;
		} else {
			h = d;
			if (d+g<0)
				h = -g;
		}
		h -= s10->cur_val;
	} else {
		h = 0;
	}

	s10->speed_hi = h / e;
	if (h<0) {
		h = -h;
		s10->direction = -1;
	} else {
		s10->direction = 1;
	}

	s10->speed_lo = h % e;
	s10->speed_lo_counter = 0;
}

byte AdlibSoundDriver::struct10_ontimer(Struct10 *s10, Struct11 *s11) {
	byte result = 0;
	int i;

	if (s10->count && (s10->count-=17)<=0) {
		s10->active = 0;
		return 0;
	}

	i = s10->cur_val + s10->speed_hi;
	s10->speed_lo_counter += s10->speed_lo;
	if (s10->speed_lo_counter >= s10->speed_lo_max) {
		s10->speed_lo_counter -= s10->speed_lo_max;
		i += s10->direction;
	}
	if (s10->cur_val != i || s10->modwheel != s10->modwheel_last) {
		s10->cur_val = i;
		s10->modwheel_last = s10->modwheel;
		i = lookup_volume(i, s10->modwheel_last);
		if (i != s11->modify_val) {
			s11->modify_val = i;
			result = 1;
		}
	}

	if (!--s10->num_steps) {
		s10->active++;
		if (s10->active > 4) {
			if (s10->loop) {
				s10->active = 1;
				result |= 2;
				struct10_setup(s10);
			} else {
				s10->active = 0;
			}
		} else {
			struct10_setup(s10);
		}
	}

	return result;
}

void AdlibSoundDriver::struct10_init(Struct10 *s10, InstrumentExtra *ie) {
	s10->active = 1;
	s10->cur_val = 0;
	s10->modwheel_last = 31;
	s10->count = ie->a;
	if (s10->count)
		s10->count *= 63;
	s10->table_a[0] = ie->b;
	s10->table_a[1] = ie->d;
	s10->table_a[2] = ie->f;
	s10->table_a[3] = ie->g;

	s10->table_b[0] = ie->c;
	s10->table_b[1] = ie->e;
	s10->table_b[2] = 0;
	s10->table_b[3] = ie->h;
		
	struct10_setup(s10);
}

void AdlibSoundDriver::mc_init_stuff(MidiChannelAdl *mc, Struct10 *s10, Struct11 *s11, byte flags, InstrumentExtra *ie) {
	Part *part = mc->_part;

	s11->modify_val = 0;
	s11->flag0x40 = flags & 0x40;
	s10->loop = flags & 0x20;
	s11->flag0x10 = flags & 0x10;
	s11->param = param_table_1[flags&0xF];
	s10->param = param_table_2[flags&0xF];
	s10->unk3 = 31;
	if (s11->flag0x40) {
		s10->modwheel = part->_modwheel>>2;
	} else {
		s10->modwheel = 31;
	}

	switch(s11->param) {
	case 0:
		s10->start_value = mc->_vol_2;
		break;
	case 13:
		s10->start_value = mc->_vol_1;
		break;
	case 30:
		s10->start_value = 31;
		s11->s10->modwheel = 0;
		break;
	case 31:
		s10->start_value = 0;
		s11->s10->unk3 = 0;
		break;
	default:
		s10->start_value = part->_drv->adlib_read_param(mc->_channel, s11->param);
	}

	struct10_init(s10, ie);
}

void AdlibSoundDriver::mc_inc_stuff(MidiChannelAdl *mc, Struct10 *s10, Struct11 *s11) {
	byte code;
	Part *part= mc->_part;

	code = struct10_ontimer(s10,s11);

	if (code&1) {
		switch(s11->param) {
		case 0:
			mc->_vol_2 = s10->start_value + s11->modify_val;
			part->_drv->adlib_set_param(mc->_channel, 0, volume_table[lookup_table[mc->_vol_2][part->_vol_eff>>2]]);
			break;
		case 13:
			mc->_vol_1 = s10->start_value + s11->modify_val;
			if (mc->_twochan) {
				part->_drv->adlib_set_param(mc->_channel, 13, volume_table[lookup_table[mc->_vol_1][part->_vol_eff>>2]]);
			} else {
				part->_drv->adlib_set_param(mc->_channel, 13, mc->_vol_1);
			}
			break;
		case 30:
			s11->s10->modwheel = (char)s11->modify_val;
			break;
		case 31:
			s11->s10->unk3 = (char)s11->modify_val;
			break;
		default:
			part->_drv->adlib_set_param(mc->_channel, s11->param, s10->start_value + s11->modify_val);
			break;
		}
	}

	if (code&2 && s11->flag0x10)
		part->_drv->adlib_key_onoff(mc->_channel);
}

void AdlibSoundDriver::part_changed(Part *part,byte what) {
	MidiChannelAdl *mc;

	if (what & pcProgram) {
		if (part->_program < 32) {
			part_set_instrument(part, &_glob_instr[part->_program]);
		}
	}

	if (what & pcMod) {
		for(mc=part->_mc->adl(); mc; mc=mc->_next) {
			adlib_note_on(mc->_channel, mc->_note + part->_transpose_eff, part->_pitchbend + part->_detune_eff);
		}
	}

	if (what & pcVolume) {
		for(mc=part->_mc->adl(); mc; mc=mc->_next) {
			adlib_set_param(mc->_channel, 0, volume_table[lookup_table[mc->_vol_2][part->_vol_eff>>2]]);
			if (mc->_twochan) {
				adlib_set_param(mc->_channel, 13, volume_table[lookup_table[mc->_vol_1][part->_vol_eff>>2]]);
			}
		}
	}

	if (what & pcPedal) {
		if (!part->_pedal) {
			for(mc=(MidiChannelAdl*)part->_mc; mc; mc=mc->_next) {
				if (mc->_waitforpedal)
					mc_off(mc);
			}
		}
	}

	if (what & pcModwheel) {
		for(mc=(MidiChannelAdl*)part->_mc; mc; mc=mc->_next) {
			if (mc->_s10a.active && mc->_s11a.flag0x40)
				mc->_s10a.modwheel = part->_modwheel>>2;
			if (mc->_s10b.active && mc->_s11b.flag0x40)
				mc->_s10b.modwheel = part->_modwheel>>2;
		}
	}
}

void AdlibSoundDriver::mc_key_on(MidiChannel *mc2, byte note, byte velocity) {
	MidiChannelAdl *mc = (MidiChannelAdl*)mc2;
	Part *part = mc->_part;
	Instrument *instr = &_part_instr[part->_slot];
	int c;
	byte vol_1,vol_2;

	mc->_twochan = instr->feedback&1;
	mc->_note = note;
	mc->_waitforpedal = false;
	mc->_duration = instr->duration;
	if (mc->_duration != 0)
		mc->_duration *= 63;

	vol_1 = (instr->oplvl_1&0x3F) + lookup_table[velocity>>1][instr->waveform_1>>2];
	if (vol_1 > 0x3F)
		vol_1 = 0x3F;
	mc->_vol_1 = vol_1;

	vol_2 = (instr->oplvl_2&0x3F) + lookup_table[velocity>>1][instr->waveform_2>>2];
	if (vol_2 > 0x3F)
		vol_2 = 0x3F;
	mc->_vol_2 = vol_2;

	c = part->_vol_eff >> 2;
	
	vol_2 = volume_table[lookup_table[vol_2][c]];
	if (mc->_twochan)
		vol_1 = volume_table[lookup_table[vol_1][c]];
	
	adlib_setup_channel(mc->_channel, instr, vol_1, vol_2);
	adlib_note_on_ex(mc->_channel, part->_transpose_eff + note, part->_detune_eff + part->_pitchbend);

	if (instr->flags_a & 0x80) {
		mc_init_stuff(mc, &mc->_s10a, &mc->_s11a, instr->flags_a, &instr->extra_a);
	} else {
		mc->_s10a.active = 0;
	}
	
	if (instr->flags_b & 0x80) {
		mc_init_stuff(mc, &mc->_s10b, &mc->_s11b, instr->flags_b, &instr->extra_b); 
	} else {
		mc->_s10b.active = 0;
	}
}

void AdlibSoundDriver::set_instrument(uint slot, byte *data) {
	if (slot < 32) {
		memcpy(&_glob_instr[slot], data, sizeof(Instrument));
	}
}


void AdlibSoundDriver::link_mc(Part *part, MidiChannelAdl *mc) {
	mc->_part = part;
	mc->_next = (MidiChannelAdl*)part->_mc;
	part->_mc = mc;
	mc->_prev = NULL;
	
	if (mc->_next)
		mc->_next->_prev = mc;
}

void AdlibSoundDriver::part_key_on(Part *part, byte note, byte velocity) {
	MidiChannelAdl *mc;

	mc = allocate_midichan(part->_pri_eff);
	if (!mc)
		return;

	link_mc(part, mc);
	mc_key_on(mc,note, velocity);
}

void AdlibSoundDriver::part_key_off(Part *part, byte note) {
	MidiChannelAdl *mc;

	for(mc=(MidiChannelAdl*)part->_mc; mc; mc=mc->_next) {
		if (mc->_note==note) {
			if (part->_pedal)
				mc->_waitforpedal = true;
			else
				mc_off(mc);
		}
	}
}

struct AdlibInstrSetParams {
	byte param;
	byte shl;
	byte mask;
};

#define MKLINE(_a_,_b_,_c_) { (int)&((Instrument*)0)->_a_, _b_, ((1<<(_c_))-1)<<(_b_) }
static const AdlibInstrSetParams adlib_instr_params[69] = {
	MKLINE(oplvl_2,0,6),
	MKLINE(waveform_2,2,5),
	MKLINE(oplvl_2,6,2),
	MKLINE(flags_2,0,4),
	MKLINE(atdec_2,4,4),
	MKLINE(atdec_2,0,4),
	MKLINE(sustrel_2,4,4),
	MKLINE(sustrel_2,0,4),
	MKLINE(waveform_2,0,2),
	MKLINE(flags_2,7,1),
	MKLINE(flags_2,6,1),
	MKLINE(flags_2,5,1),
	MKLINE(flags_2,4,1),

	MKLINE(oplvl_1,0,6),
	MKLINE(waveform_1,2,5),
	MKLINE(oplvl_1,6,2),
	MKLINE(flags_1,0,4),
	MKLINE(atdec_1,4,4),
	MKLINE(atdec_1,0,4),
	MKLINE(sustrel_1,4,4),
	MKLINE(sustrel_1,0,4),
	MKLINE(waveform_1,0,2),
	MKLINE(flags_1,7,1),
	MKLINE(flags_1,6,1),
	MKLINE(flags_1,5,1),
	MKLINE(flags_1,4,1),

	MKLINE(feedback,0,1),
	MKLINE(feedback,1,3),

	MKLINE(flags_a,7,1),
	MKLINE(flags_a,6,1),
	MKLINE(flags_a,5,1),
	MKLINE(flags_a,4,1),
	MKLINE(flags_a,0,4),
	MKLINE(extra_a.a,0,8),
	MKLINE(extra_a.b,0,7),
	MKLINE(extra_a.c,0,7),
	MKLINE(extra_a.d,0,7),
	MKLINE(extra_a.e,0,7),
	MKLINE(extra_a.f,0,7),
	MKLINE(extra_a.g,0,7),
	MKLINE(extra_a.h,0,7),
	MKLINE(extra_a.b,7,1),
	MKLINE(extra_a.c,7,1),
	MKLINE(extra_a.d,7,1),
	MKLINE(extra_a.e,7,1),
	MKLINE(extra_a.f,7,1),
	MKLINE(extra_a.g,7,1),
	MKLINE(extra_a.h,7,1),

	MKLINE(flags_b,7,1),
	MKLINE(flags_b,6,1),
	MKLINE(flags_b,5,1),
	MKLINE(flags_b,4,1),
	MKLINE(flags_b,0,4),
	MKLINE(extra_b.a,0,8),
	MKLINE(extra_b.b,0,7),
	MKLINE(extra_b.c,0,7),
	MKLINE(extra_b.d,0,7),
	MKLINE(extra_b.e,0,7),
	MKLINE(extra_b.f,0,7),
	MKLINE(extra_b.g,0,7),
	MKLINE(extra_b.h,0,7),
	MKLINE(extra_b.b,7,1),
	MKLINE(extra_b.c,7,1),
	MKLINE(extra_b.d,7,1),
	MKLINE(extra_b.e,7,1),
	MKLINE(extra_b.f,7,1),
	MKLINE(extra_b.g,7,1),
	MKLINE(extra_b.h,7,1),

	MKLINE(duration,0,8),
};
#undef MKLINE

void AdlibSoundDriver::part_set_param(Part *part, byte param, int value) {
	const AdlibInstrSetParams *sp = &adlib_instr_params[param];
	byte *p = (byte*)&_part_instr[part->_slot] + sp->param;
	*p = (*p&~sp->mask) | (value<<sp->shl);

	if (param < 28) {
		MidiChannelAdl *mc;

		for(mc=(MidiChannelAdl*)part->_mc; mc; mc=mc->_next) {
			adlib_set_param(mc->_channel, param, value);
		}
	}
}

void AdlibSoundDriver::part_off(Part *part) {
	MidiChannelAdl *mc = (MidiChannelAdl*)part->_mc;
	part->_mc = NULL;
	for(; mc; mc=mc->_next) {
		mc_off(mc);
	}
}

void AdlibSoundDriver::mc_off(MidiChannel *mc2) {
	MidiChannelAdl *mc = (MidiChannelAdl*)mc2, *tmp;

	adlib_key_off(mc->_channel);

	tmp = mc->_prev;

	if (mc->_next)
		mc->_next->_prev = tmp;
	if (tmp)
		tmp->_next = mc->_next;
	else
		mc->_part->_mc = mc->_next;
	mc->_part = NULL;
}

void AdlibSoundDriver::part_set_instrument(Part *part, Instrument *instr) {
	Instrument *i = &_part_instr[part->_slot];
	memcpy(i, instr, sizeof(Instrument));
}

int AdlibSoundDriver::part_update_active(Part *part,uint16 *active) {
	uint16 bits;
	int count = 0;
	MidiChannelAdl *mc;

	bits = 1<<part->_chan;

	for(mc=part->_mc->adl(); mc; mc=mc->_next) {
		if (!(active[mc->_note] & bits)) {
			active[mc->_note] |= bits;
			count++;
		}
	}
	return count;
}

#endif
