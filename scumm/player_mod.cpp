/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "player_mod.h"

////////////////////////////////////////
//
// Generic Amiga MOD mixer - provides a 60Hz 'update' routine.
//
////////////////////////////////////////

Player_MOD::Player_MOD(Scumm *scumm) {
	int i;
	_mixer = scumm->_mixer;
	_samplerate = scumm->_system->property(OSystem::PROP_GET_SAMPLE_RATE, 0);
	_mixamt = 0;
	_mixpos = 0;

	for (i = 0; i < MOD_MAXCHANS; i++) {
		_channels[i].id = 0;
		_channels[i].vol = 0;
		_channels[i].freq = 0;
		_channels[i].ptr = NULL;
		_channels[i].converter = NULL;
		_channels[i].input = NULL;
	}

	_playproc = NULL;
	_playparam = NULL;

	_mixer->setupPremix(premix_proc, this);
}

Player_MOD::~Player_MOD() {
	// Detach the premix callback handler
	_mixer->setupPremix(0, 0);
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (!_channels[i].id)
			continue;
		free(_channels[i].ptr);
		delete _channels[i].converter;
		delete _channels[i].input;
	}
}

void Player_MOD::setMasterVolume (int vol) {
	_maxvol = vol;
}

void Player_MOD::setUpdateProc(ModUpdateProc *proc, void *param, int freq) {
	_playproc = proc;
	_playparam = param;
	_mixamt = _samplerate / freq;
}
void Player_MOD::clearUpdateProc() {
	_playproc = NULL;
	_playparam = NULL;
	_mixamt = 0;
}

void Player_MOD::startChannel (int id, void *data, int size, int rate, uint8 vol, int loopStart, int loopEnd, int8 pan) {
	int i;
	if (id == 0)
		error("player_mod - attempted to start channel id 0");

	for (i = 0; i < MOD_MAXCHANS; i++) {
		if (!_channels[i].id)
			break;
	}
	if (i == MOD_MAXCHANS) {
		warning("player_mod - too many music channels playing (%i max)",MOD_MAXCHANS);
		return;
	}
	_channels[i].id = id;
	_channels[i].vol = vol;
	_channels[i].pan = pan;
	_channels[i].ptr = data;
	_channels[i].freq = rate;
	_channels[i].input = makeLinearInputStream(SoundMixer::FLAG_AUTOFREE | (loopStart != loopEnd ? SoundMixer::FLAG_LOOP : 0), (const byte*)data, size, loopStart, loopEnd - loopStart);
	_channels[i].converter = makeRateConverter(rate, _mixer->getOutputRate(), false, false);
}

void Player_MOD::stopChannel(int id) {
	if (id == 0)
		error("player_mod - attempted to stop channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			free(_channels[i].ptr);
			_channels[i].ptr = NULL;
			delete _channels[i].converter;
			_channels[i].converter = NULL;
			delete _channels[i].input;
			_channels[i].input = NULL;
			_channels[i].id = 0;
			_channels[i].vol = 0;
			_channels[i].freq = 0;
		}
	}
}
void Player_MOD::setChannelVol(int id, uint8 vol) {
	if (id == 0)
		error("player_mod - attempted to set volume for channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			_channels[i].vol = vol;
			break;
		}
	}
}

void Player_MOD::setChannelPan(int id, int8 pan) {
	if (id == 0)
		error("player_mod - attempted to set pan for channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			_channels[i].pan = pan;
			break;
		}
	}
}

void Player_MOD::setChannelFreq(int id, int freq) {
	if (id == 0)
		error("player_mod - attempted to set frequency for channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			_channels[i].freq = freq;
			delete _channels[i].converter;
			_channels[i].converter = makeRateConverter(freq, _mixer->getOutputRate(), false, false);
			break;
		}
	}
}

void Player_MOD::premix_proc(void *param, int16 *buf, uint len) {
	((Player_MOD *) param)->do_mix(buf, len);
}

void Player_MOD::do_mix (int16 *data, uint len) {
	int i;
	int dpos = 0;
	uint dlen = 0;
	memset(data, 0, 2 * len * sizeof(int16));
	while (len) {
		if (_playproc) {
			dlen = _mixamt - _mixpos;
			if (!_mixpos)
				_playproc(_playparam);
			if (dlen <= len) {
				_mixpos = 0;
				len -= dlen;
			} else {
				_mixpos = _mixamt - len;
				dlen = len;
				len = 0;
			}
		} else {
			dlen = len;
			len = 0;
		}
		for (i = 0; i < MOD_MAXCHANS; i++)
			if (_channels[i].id) {
				st_volume_t vol_l = (127 - _channels[i].pan) * _channels[i].vol / 127;
				st_volume_t vol_r = (127 + _channels[i].pan) * _channels[i].vol / 127;
				_channels[i].converter->flow(*_channels[i].input, &data[dpos*2], dlen, vol_l, vol_r);
			}
		dpos += dlen;
	}
}
