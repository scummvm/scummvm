/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "common/engine.h"
#include "player_v1.h"
#include "scumm.h"
#include "sound/mixer.h"

#define TIMER_BASE_FREQ 1193000
#define FIXP_SHIFT  16

////////////////////////////////////////
//
// V1 PC-Speaker player
//
////////////////////////////////////////


Player_V1::Player_V1(Scumm *scumm) : Player_V2(scumm) {	
	/* pcjr not yet supported */
	set_pcjr(false);

	_freq_current = 0;
}

Player_V1::~Player_V1() {
}

void Player_V1::chainSound(int nr, byte *data) {
	int offset = _pcjr ? READ_LE_UINT16(data+4) : 6;

	current_nr = nr;
	current_data = data;
	_next_cmd = data + (_pcjr ? offset + 2 : offset + 4);
	_repeat_ptr = _next_cmd;
	_music_timer = 0;

	debug(4, "Chaining new sound %d", nr);
	parse_chunk();
}

void Player_V1::startSound(int nr, byte *data) {
	mutex_up();

	int offset = _pcjr ? READ_LE_UINT16(data+4) : 6;
	int cprio = current_data ? *(current_data + offset) & 0x7f : 0;
	int prio  = *(data + _header_len) & 0x7f;
	int nprio = next_data ? *(next_data + _header_len) & 0x7f : 0;

	int restartable = *(data + _header_len) & 0x80;

	if (!current_nr || cprio <= prio) {
		int tnr = current_nr;
		int tprio = cprio;
		byte *tdata  = current_data;

		chainSound(nr, data);
		nr   = tnr;
		prio = tprio;
		data = tdata;
		restartable = data ? *(data + _header_len) & 0x80 : 0;
	}
	
	if (!current_nr) {
		nr = 0;
		next_nr = 0;
		next_data = 0;
	}
	
	if (nr != current_nr
	    && restartable
	    && (!next_nr
		|| nprio <= prio)) {

		next_nr = nr;
		next_data = data;
	}

	mutex_down();
}

void Player_V1::restartSound() {
	if (*(current_data + _header_len) & 0x80) {
		/* current sound is restartable */
		chainSound(current_nr, current_data);
	} else {
		chainNextSound();
	}
}

int Player_V1::getMusicTimer() const {
	return _music_timer;
}

void Player_V1::parse_chunk() {
	set_mplex(3000);
	_forced_level = 0;

 parse_again:
	_chunk_type = READ_LE_UINT16(_next_cmd);
	debug(4, "parse_chunk: sound %d, offset %4x, chunk %x", 
		  current_nr, _next_cmd - current_data, _chunk_type);

	_next_cmd += 2;
	switch (_chunk_type) {
	case 0xffff:
		current_nr = 0;
		current_data = 0;
		_freq_current = 0;
		chainNextSound();
		break;
	case 0xfffe:
		_repeat_ptr = _next_cmd;
		goto parse_again;

	case 0xfffd:
		_next_cmd = _repeat_ptr;
		goto parse_again;

	case 0xfffc:
		/* handle reset. We don't need this don't we? */
		goto parse_again;

	case 0:
		_time_left = 1;
		set_mplex(READ_LE_UINT16(_next_cmd));
		_next_cmd += 2;
		break;
	case 1:
		set_mplex(READ_LE_UINT16(_next_cmd));
		_freq_start = READ_LE_UINT16(_next_cmd + 2);
		_freq_end   = READ_LE_UINT16(_next_cmd + 4);
		_freq_delta = READ_LE_UINT16(_next_cmd + 6);
		_repeat_ctr = READ_LE_UINT16(_next_cmd + 8);
		_freq_current = _freq_start;
		_next_cmd += 10;
		debug(4, "chunk 1: mplex %d, freq %d -> %d step %d  x %d", 
			  _mplex, _freq_start, _freq_end, _freq_delta, _repeat_ctr);
		break;
	case 2:
		_freq_start = READ_LE_UINT16(_next_cmd);
		_freq_end   = READ_LE_UINT16(_next_cmd + 2);
		_freq_delta = READ_LE_UINT16(_next_cmd + 4);
		_freq_current = 0;
		_next_cmd += 6;
		debug(4, "chunk 2: %d -> %d step %d", 
			  _freq_start, _freq_end, _freq_delta);
		break;
	case 3:
		_freq_start = READ_LE_UINT16(_next_cmd);
		_freq_end   = READ_LE_UINT16(_next_cmd + 2);
		_freq_delta = READ_LE_UINT16(_next_cmd + 4);
		_freq_current = 0;
		_next_cmd += 6;
		debug(4, "chunk 3: %d -> %d step %d", 
			  _freq_start, _freq_end, _freq_delta);
		break;
	}
}

void Player_V1::next_speaker_cmd() {
	uint16 lsr;
	switch (_chunk_type) {
	case 0:
		if (--_time_left)
			return;
		_time_left = READ_LE_UINT16(_next_cmd);
		_next_cmd += 2;
		if (_time_left == 0xfffb) {
			/* handle 0xfffb?? */
			_time_left = READ_LE_UINT16(_next_cmd);
			_next_cmd += 2;
		}
		debug(4, "next_speaker_cmd: chunk %d, offset %4x: notelen %d", 
			  _chunk_type, _next_cmd - 2 - current_data, _time_left);

		if (_time_left == 0) {
			parse_chunk();
		} else {
			_freq_current = READ_LE_UINT16(_next_cmd);
			_next_cmd += 2;
			debug(4, "freq_current: %d", _freq_current);
		}
		break;

	case 1:
		_freq_current = (_freq_current + _freq_delta) & 0xffff;
		if (_freq_current == _freq_end) {
			if (!--_repeat_ctr)
				parse_chunk();
			_freq_current = _freq_start;
		}
		break;

	case 2:
		_freq_start = (_freq_start + _freq_delta) & 0xffff;
		if (_freq_start == _freq_end) {
			parse_chunk();
		}
		set_mplex(_freq_start);
		_forced_level ^= 1;
		break;
	case 3:
		_freq_start = (_freq_start + _freq_delta) & 0xffff;
		if (_freq_start == _freq_end) {
			parse_chunk();
		}
		lsr = _random_lsr + 0x9248;
		lsr = (lsr >> 3) | (lsr << 13);
		_random_lsr = lsr;
		set_mplex((_freq_start & lsr) | 0x180);
		_forced_level ^= 1;
		break;
	}
}

void Player_V1::set_mplex (uint mplex) {
	if (mplex == 0)
		mplex = 65536;
	_mplex = mplex;
	_tick_len = (_sample_rate << FIXP_SHIFT) 
		/ (TIMER_BASE_FREQ / mplex);
}

void Player_V1::do_mix (int16 *data, uint len) {
	mutex_up();
	uint step;

	do {
		step = len;
		if (step > (_next_tick >> FIXP_SHIFT))
			step = (_next_tick >> FIXP_SHIFT);
		generateSpkSamples(data, step);
		data += step;
		_next_tick -= step << FIXP_SHIFT;

		if (!(_next_tick >> FIXP_SHIFT)) {
			_next_tick += _tick_len;
			next_speaker_cmd();
		}
	} while (len -= step);
	mutex_down();
}

void Player_V1::generateSpkSamples(int16 *data, uint len) {
	uint i;

	memset (data, 0, sizeof(int16) * len);
	if (_freq_current == 0) {
		if (_forced_level) {
			for (i = 0; i < len; i++) {
				data[i] = _volumetable[0];
			}
		} else if (!_level) {
			return;
		}
	} else {
		squareGenerator(0, _freq_current, 0, 0, data, len);
	}
	lowPassFilter(data, len);
}
