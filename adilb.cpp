#include "stdafx.h"
#include "scumm.h"
#include "sound.h"

void AdlibChannel::set_transpose(int8 transpose) {
	AdlibChannel *s;

	for(s=this; s; s = s->_next) {
		_se->adlib_note_on(s->_channel, s->_note + transpose, s->_part->_mod_eff);
	}
}

void AdlibChannel::set_mod(int mod) {
	AdlibChannel *s;

	for(s=this; s; s = s->_next) {
		_se->adlib_note_on(s->_channel, s->_note + s->_part->_transpose_eff, mod);
	}
}

void AdlibChannel::off() {
	AdlibChannel *tmp;

	_se->adlib_key_off(_channel);

	tmp = _prev;

	if (_next)
		_next->_prev = tmp;
	if (tmp)
		tmp->_next = _next;
	else
		_part->_s3 = _next;
	_part = NULL;
}

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

void AdlibChannel::key_on(Part *part, byte note, byte velocity) {
	Instrument *instr = &part->_instr;
	int c;
	byte vol_1,vol_2;

	_twochan = instr->feedback&1;
	_note = note;
	_waitforpedal = false;
	_duration = instr->duration;
	if (_duration != 0)
		_duration *= 63;

	vol_1 = (instr->oplvl_1&0x3F) + lookup_table_a[velocity>>1][instr->waveform_1>>2];
	if (vol_1 > 0x3F)
		vol_1 = 0x3F;
	_vol_1 = vol_1;

	vol_2 = (instr->oplvl_2&0x3F) + lookup_table_a[velocity>>1][instr->waveform_2>>2];
	if (vol_2 > 0x3F)
		vol_2 = 0x3F;
	_vol_2 = vol_2;

	c = part->_vol_eff >> 2;
	
	vol_2 = volume_table[lookup_table_a[vol_2][c]];
	if (_twochan)
		vol_1 = volume_table[lookup_table_a[vol_1][c]];
	
	_se->adlib_setup_channel(_channel, instr, vol_1, vol_2);
	_se->adlib_note_on_ex(_channel, part->_transpose_eff + note, part->_mod_eff);

	if (instr->flags_a & 0x80) {
		unk5(&_s10a, &_s11a, instr->flags_a, &instr->s12a);
	} else {
		_s10a.active = 0;
	}
	
	if (instr->flags_b & 0x80) {
		unk5(&_s10b, &_s11b, instr->flags_b, &instr->s12b); 
	} else {
		_s10b.active = 0;
	}
}

const byte s35_table_1[16] = {
29,28,27,0,
3,4,7,8,
13,16,17,20,
21,30,31,0
};

const uint16 s35_table_2[16] = {
0x2FF,0x1F,0x7,0x3F,
0x0F,0x0F,0x0F,0x3,
0x3F,0x0F,0x0F,0x0F,
0x3,0x3E,0x1F, 0
};

static const uint16 s102_table_1[] = {
1, 2, 4, 5,
6, 7, 8, 9,
10, 12, 14, 16,
18, 21, 24, 30,
36, 50, 64, 82,
100, 136, 160, 192,
240, 276, 340, 460,
600, 860, 1200, 1600
};

int random_nr(int a) {
	static byte _rand_seed = 1;
	if (_rand_seed&1) {
		_rand_seed>>=1;
		_rand_seed ^= 0xB8;
	} else {
		_rand_seed>>=1;
	}
	return _rand_seed * a >> 8;
}

void struct10_setup(Struct10 *s10) {
	int b,c,d,e,f,g,h;
	byte t;

	b = s10->unk3;
	f = s10->active - 1;

	t = s10->unk7[f];
	e = s102_table_1[lookup_table_a[t&0x7F][b]];
	if (t&0x80) {
		e = random_nr(e);
	}
	if (e==0)
		e++;
	
	s10->num_steps = s10->speed_lo_max = e;

	if (f != 2) {
		c = s10->unk2;
		g = s10->start_value;
		t = s10->unk8[f];
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

byte struct10_unk3(Struct10 *s10, Struct11 *s11) {
	byte result = 0;
	int i;

	if (s10->unk6 && (s10->unk6-=17)<=0) {
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
	assert(s10->num_steps>=0);
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

void struct10_unk1(Struct10 *s10, Struct12 *s12) {
	s10->active = 1;
	s10->cur_val = 0;
	s10->modwheel_last = 31;
	s10->unk6 = s12->a;
	if (s10->unk6)
		s10->unk6 *= 63;
	s10->unk7[0] = s12->b;
	s10->unk7[1] = s12->d;
	s10->unk7[2] = s12->f;
	s10->unk7[3] = s12->g;

	s10->unk8[0] = s12->c;
	s10->unk8[1] = s12->e;
	s10->unk8[2] = 0;
	s10->unk8[3] = s12->h;
		
	struct10_setup(s10);
}


void AdlibChannel::unk5(Struct10 *s10, Struct11 *s11, byte flags, Struct12 *s12) {
	s11->modify_val = 0;
	s11->unk4 = flags & 0x40;
	s10->loop = flags & 0x20;
	s11->unk5 = flags & 0x10;
	s11->param = s35_table_1[flags&0xF];
	s10->unk2 = s35_table_2[flags&0xF];
	s10->unk3 = 31;
	if (s11->unk4) {
		s10->modwheel = _part->_modwheel>>2;
	} else {
		s10->modwheel = 31;
	}

	switch(s11->param) {
	case 0:
		s10->start_value = _vol_2;
		break;
	case 13:
		s10->start_value = _vol_1;
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
		s10->start_value = _se->adlib_read_param(_channel, s11->param);
	}

	struct10_unk1(s10, s12);
}


void AdlibChannel::unk1(Struct10 *s10, Struct11 *s11) {
	byte code;

	code = struct10_unk3(s10,s11);

	if (code&1) {
		switch(s11->param) {
		case 0:
			_vol_2 = s10->start_value + s11->modify_val;
			_se->adlib_set_param(_channel, 0, volume_table[lookup_table_a[_vol_2][_part->_vol_eff>>2]]);
			break;
		case 13:
			_vol_1 = s10->start_value + s11->modify_val;
			if (_twochan) {
				_se->adlib_set_param(_channel, 13, volume_table[lookup_table_a[_vol_1][_part->_vol_eff>>2]]);
			} else {
				_se->adlib_set_param(_channel, 13, _vol_1);
			}
			break;
		case 30:
			s11->s10->modwheel = s11->modify_val;
			break;
		case 31:
			s11->s10->unk3 = s11->modify_val;
			break;
		default:
			_se->adlib_set_param(_channel, s11->param, s10->start_value + s11->modify_val);
			break;
		}
	}

	if (code&2 && s11->unk5)
		_se->adlib_key_onoff(_channel);
}


int HookDatas::query_param(int param, byte chan) {
	switch(param) {
	case 18:
		return _jump;
	case 19:
		return _transpose;
	case 20:
		return _part_onoff[chan];
	case 21:
		return _part_volume[chan];
	case 22:
		return _part_program[chan];
	case 23:
		return _part_transpose[chan];
	default:
		return -1;
	}
}

int HookDatas::set(byte cls, byte value, byte chan) {
	switch(cls) {
	case 0:
		_jump = value;
		break;
	case 1:
		_transpose = value;
		break;
	case 2:
		if (chan<16)
			_part_onoff[chan] = value;
		else if (chan==16)
			memset(_part_onoff, value, 16);
		break;
	case 3:
		if (chan<16)
			_part_volume[chan] = value;
		else if (chan==16)
			memset(_part_volume, value, 16);
		break;
	case 4:
		if (chan<16)
			_part_program[chan] = value;
		else if (chan==16)
			memset(_part_program, value, 16);
		break;
	case 5:
		if (chan<16)
			_part_transpose[chan] = value;
		else if (chan==16)
			memset(_part_transpose, value, 16);
		break;
	default:
		return -1;
	}
	return 0;
}


VolumeFader *SoundEngine::allocate_volume_fader() {
	VolumeFader *vf;
	int i;

	i = 8;
	vf = volume_fader;
	while (vf->active) {
		vf++;
		if (!--i)
			return NULL;
	}

	vf->active = true;
	_active_volume_faders = true;
	return vf;
}


uint32 SoundEngine::proc_16() {
	return _dword_6d8;
}

void SoundEngine::proc_7(uint32 a) {
	_dword_6d8 = a;
}

Player *SoundEngine::get_player_byid(int id) {
	int i;
	Player *s6,*found=NULL;

	for(i=0,s6=players; i<8; i++,s6++) {
		if (s6->_active && s6->_id==(uint16)id) {
			if(found)
				return NULL;
			found = s6;
		}
	}
	return found;
}

int SoundEngine::proc_15(uint a) {
	if (a<8)
		return table_1[a];
	return -1;
}

int SoundEngine::initialize(byte *ptr) {
	int i;
	if (_initialized)
		return -1;
	_master_volume = 127;
	for (i=0; i<8; i++)
		channel_volume[i] = channel_real_volume[i] = table_1[i] = 127;

	create_lookup_table();

	init_players();
	init_sustaining_notes();
	init_volume_fader();
	init_queue();
	init_7();
	init_parts();

	_initialized = true;
	
	return 0;
}

void SoundEngine::init_queue() {
	_queue_adding = false;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
}

void SoundEngine::init_7() {
	_scanning = false;
	_hardware_type = 1;
}


void AdlibChannel::set_volume(int a) {
	AdlibChannel *s;
	for(s=this; s; s = s->_next) {
		_se->adlib_set_param(s->_channel, 0, volume_table[lookup_table_a[s->_vol_2][a>>2]]);
		if (s->_twochan) {
			_se->adlib_set_param(s->_channel, 13, volume_table[lookup_table_a[s->_vol_1][a>>2]]);
		}
	}
}

void AdlibChannel::set_modwheel(int a) {
	AdlibChannel *s;
	for(s=this; s; s = s->_next) {
		if (s->_s10a.active && s->_s11a.unk4)
			s->_s10a.modwheel = a>>2;
		if (s->_s10b.active && s->_s11b.unk4)
			s->_s10b.modwheel = a>>2;
	}
}

void AdlibChannel::release_pedal() {
	AdlibChannel *s;
	for(s=this; s; s = s->_next) {
		if (s->_waitforpedal)
			s->off();
	}
}

void AdlibChannel::key_off(byte data) {
	AdlibChannel *s;
	for(s=this; s; s = s->_next) {
		if (s->_note == data)
			s->off();
	}
}

void AdlibChannel::key_off_pedal(byte data) {
	AdlibChannel *s;
	for(s=this; s; s = s->_next) {
		if (s->_note == data)
			s->_waitforpedal = true;
	}
}

void AdlibChannel::set_param(byte param, int value) {
	AdlibChannel *s;

	if (param < 28)
		for(s=this; s; s=s->_next)
			_se->adlib_set_param(s->_channel, param, value);
}


void SoundEngine::adlib_write(byte port, byte value) {
	if (_adlib_reg_cache[port] == value)
		return;
	_adlib_reg_cache[port] = value;
	OPLWriteReg(_opl, port, value);
}

void SoundEngine::adlib_key_off(int chan) {
	byte port = chan + 0xB0;
	adlib_write(port, adlib_read(port)&~0x20);
}

void SoundEngine::adlib_init() {
	_adlib_reg_cache = (byte*)calloc(256,1);
	_opl = OPLCreate(OPL_TYPE_YM3812,3579545,22050);
	adlib_write(1,0x20);
	adlib_write(8,0x40);
	adlib_write(0xBD, 0x00);
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

void SoundEngine::adlib_set_param(int channel, byte param, int value) {
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

void SoundEngine::adlib_playnote(int channel, int note) {
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

void SoundEngine::adlib_note_on(int chan, byte note, int mod) {
	int code;
	assert(chan>=0 && chan<9);
	code = (note<<7) + mod;
	curnote_table[chan] = code;
	adlib_playnote(chan, channel_table_2[chan] + code);
}

void SoundEngine::adlib_note_on_ex(int chan, byte note, int mod) {
	int code;
	assert(chan>=0 && chan<9);
	code = (note<<7) + mod;
	curnote_table[chan] = code;
	channel_table_2[chan] = 0;
	adlib_playnote(chan, code);
}

void SoundEngine::adlib_setup_channel(int chan, Instrument *instr, byte vol_1, byte vol_2) {
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

int SoundEngine::adlib_read_param(int chan, byte param) {
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


