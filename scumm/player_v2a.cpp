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

#include "stdafx.h"
#include "base/engine.h"
#include "player_v2a.h"
#include "scumm.h"

#define BASE_FREQUENCY 3579545

////////////////////////////////////////
//
// V2 Amiga sound/music driver
//
////////////////////////////////////////

static uint32	CRCtable[256];
static void	InitCRC (void)
{
	const uint32 poly = 0xEDB88320;
	int i, j;
	uint32 n;
	for (i = 0; i < 256; i++)
	{
		n = i;
		for (j = 0; j < 8; j++)
			n = (n & 1) ? ((n >> 1) ^ poly) : (n >> 1);
		CRCtable[i] = n;
	}
}
static uint32 GetCRC (byte *data, int len)
{
	uint32 CRC = 0xFFFFFFFF;
	int i;
	for (i = 0; i < len; i++)
		CRC = (CRC >> 8) ^ CRCtable[(CRC ^ data[i]) & 0xFF];
	return CRC ^ 0xFFFFFFFF;
}

class V2A_Sound {
public:
	V2A_Sound() : _id(0), _mod(NULL) { }

	virtual void start(Player_MOD *mod, int id, const byte *data) = 0;
	virtual bool update() = 0;
	virtual void stop() = 0;
protected:
	int _id;
	Player_MOD *_mod;
};

class V2A_Sound_Unsupported : public V2A_Sound {
public:
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		warning("player_v2a - sound %i not supported yet", id);
	}
	virtual bool update() { return false; }
	virtual void stop() { }
};

template<int numChan>
class V2A_Sound_Base : public V2A_Sound {
public:
	V2A_Sound_Base() : _offset(0), _size(0), _data(0) { }
	V2A_Sound_Base(uint16 offset, uint16 size) : _offset(offset), _size(size), _data(0) { }
	virtual void stop() {
		assert(_id);
		for (int i = 0; i < numChan; i++)
			_mod->stopChannel(_id | (i << 8));
		_id = 0;
		free(_data);
		_data = 0;
	}
protected:
	const uint16 _offset;
	const uint16 _size;

	char *_data;
};

class V2A_Sound_Music : public V2A_Sound {
public:
	V2A_Sound_Music(uint16 instoff, uint16 voloff, uint16 chan1off, uint16 chan2off, uint16 chan3off, uint16 chan4off, uint16 sampoff, bool looped) :
		_instoff(instoff), _voloff(voloff), _chan1off(chan1off), _chan2off(chan2off), _chan3off(chan3off), _chan4off(chan4off), _sampoff(sampoff), _looped(looped) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;

		_data = (char *)malloc(READ_LE_UINT16(data));
		memcpy(_data, data, READ_LE_UINT16(data));

		_chan[0].dataptr_i = _chan1off;
		_chan[1].dataptr_i = _chan2off;
		_chan[2].dataptr_i = _chan3off;
		_chan[3].dataptr_i = _chan4off;
		for (int i = 0; i < 4; i++) {
			_chan[i].dataptr = _chan[i].dataptr_i;
			_chan[i].volbase = 0;
			_chan[i].volptr = 0;
			_chan[i].chan = 0;
			_chan[i].dur = 0;
			_chan[i].ticks = 0;
		}
		update();
	}
	virtual bool update() {
		assert(_id);
		int i, j = 0;
		for (i = 0; i < 4; i++) {
			if (_chan[i].dur) {
				if (!--_chan[i].dur) {
					_mod->stopChannel(_id | (_chan[i].chan << 8));
				} else {
					_mod->setChannelVol(_id | (_chan[i].chan << 8),
						READ_BE_UINT16(_data + _chan[i].volbase + (_chan[i].volptr++ << 1)));
					if (_chan[i].volptr == 0) {
						_mod->stopChannel(_id | (_chan[i].chan << 8));
						_chan[i].dur = 0;
					}
				}
			}
			if (!_chan[i].dataptr) {
				j++;
				continue;
			}
			if (READ_BE_UINT16(_data + _chan[i].dataptr) <= _chan[i].ticks) {
				if (READ_BE_UINT16(_data + _chan[i].dataptr + 2) == 0xFFFF) {
					if (_looped) {
						_chan[i].dataptr = _chan[i].dataptr_i;
						_chan[i].ticks = 0;
						if (READ_BE_UINT16(_data + _chan[i].dataptr) > 0) {
							_chan[i].ticks++;
							continue;
						}
					} else {
						_chan[i].dataptr = 0;
						j++;
						continue;
					}
				}
				int freq = BASE_FREQUENCY / READ_BE_UINT16(_data + _chan[i].dataptr + 2);
				int inst = READ_BE_UINT16(_data + _chan[i].dataptr + 8);
				_chan[i].volbase = _voloff + (READ_BE_UINT16(_data + _instoff + (inst << 5)) << 9);
				_chan[i].volptr = 0;
				_chan[i].chan = READ_BE_UINT16(_data + _chan[i].dataptr + 6) & 0x3;

				if (_chan[i].dur)	// if there's something playing, stop it
					_mod->stopChannel(_id | (_chan[i].chan << 8));

				_chan[i].dur = READ_BE_UINT16(_data + _chan[i].dataptr + 4);

				int vol = READ_BE_UINT16(_data + _chan[i].volbase + (_chan[i].volptr++ << 1));

				int pan;
				if ((_chan[i].chan == 0) || (_chan[i].chan == 3))
					pan = -127;
				else	pan = 127;
				int offset = READ_BE_UINT16(_data + _instoff + (inst << 5) + 0x14);
				int len = READ_BE_UINT16(_data + _instoff + (inst << 5) + 0x18);
				int loopoffset = READ_BE_UINT16(_data + _instoff + (inst << 5) + 0x16);
				int looplen = READ_BE_UINT16(_data + _instoff + (inst << 5) + 0x10);

				int size = len + looplen;
				char *data = (char *)malloc(size);
				memcpy(data, _data + _sampoff + offset, len);
				memcpy(data + len, _data + _sampoff + loopoffset, looplen);

				_mod->startChannel(_id | (_chan[i].chan << 8), data, size, freq, vol, len, looplen + len, pan);
				_chan[i].dataptr += 16;
			}
			_chan[i].ticks++;
		}
		if (j == 4)
			return false;
		return true;
	}
	virtual void stop() {
		assert(_id);
		for (int i = 0; i < 4; i++) {
			if (_chan[i].dur)
				_mod->stopChannel(_id | (_chan[i].chan << 8));
		}
		free(_data);
		_id = 0;
	}
private:
	const uint16 _instoff;
	const uint16 _voloff;
	const uint16 _chan1off;
	const uint16 _chan2off;
	const uint16 _chan3off;
	const uint16 _chan4off;
	const uint16 _sampoff;
	const bool _looped;

	char *_data;
	struct tchan {
		uint16 dataptr_i;
		uint16 dataptr;
		uint16 volbase;
		uint8 volptr;
		uint16 chan;
		uint16 dur;
		uint16 ticks;
	} _chan[4];
};

class V2A_Sound_Single : public V2A_Sound_Base<1> {
public:
	V2A_Sound_Single(uint16 offset, uint16 size, uint16 freq, uint8 vol) :
		V2A_Sound_Base<1>(offset, size), _freq(freq), _vol(vol) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, data + _offset, _size);
		int vol = (_vol << 2) | (_vol >> 4);
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _freq, vol, 0, 0);
		_ticks = 1 + (60 * _size * _freq) / BASE_FREQUENCY;
	}
	virtual bool update() {
		assert(_id);
		_ticks--;
		if (!_ticks) {
			return false;
		}
		return true;
	}
private:
	const uint16 _freq;
	const uint8 _vol;

	int _ticks;
};

class V2A_Sound_SingleLooped : public V2A_Sound_Base<1> {
public:
	V2A_Sound_SingleLooped(uint16 offset, uint16 size, uint16 freq, uint8 vol, uint16 loopoffset, uint16 loopsize) :
		V2A_Sound_Base<1>(offset, size), _loopoffset(loopoffset), _loopsize(loopsize), _freq(freq), _vol(vol) { }
	V2A_Sound_SingleLooped(uint16 offset, uint16 size, uint16 freq, uint8 vol) :
		V2A_Sound_Base<1>(offset, size), _loopoffset(0), _loopsize(size), _freq(freq), _vol(vol) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, data + _offset, _size);
		int vol = (_vol << 2) | (_vol >> 4);
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _freq, vol, _loopoffset, _loopoffset + _loopsize);
	}
	virtual bool update() {
		assert(_id);
		return true;
	}
private:
	const uint16 _loopoffset;
	const uint16 _loopsize;
	const uint16 _freq;
	const uint8 _vol;
};

class V2A_Sound_MultiLooped : public V2A_Sound_Base<2> {
public:
	V2A_Sound_MultiLooped(uint16 offset, uint16 size, uint16 freq1, uint8 vol1, uint16 freq2, uint8 vol2) :
		V2A_Sound_Base<2>(offset, size), _freq1(freq1), _vol1(vol1), _freq2(freq2), _vol2(vol2) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data1 = (char *)malloc(_size);
		char *tmp_data2 = (char *)malloc(_size);
		memcpy(tmp_data1, data + _offset, _size);
		memcpy(tmp_data2, data + _offset, _size);
		int vol1 = (_vol1 << 1) | (_vol1 >> 5);
		int vol2 = (_vol2 << 1) | (_vol2 >> 5);
		_mod->startChannel(_id | 0x000, tmp_data1, _size, BASE_FREQUENCY / _freq1, vol1, 0, _size, -127);
		_mod->startChannel(_id | 0x100, tmp_data2, _size, BASE_FREQUENCY / _freq2, vol2, 0, _size, 127);
	}
	virtual bool update() {
		assert(_id);
		return true;
	}
private:
	const uint16 _freq1;
	const uint8 _vol1;
	const uint16 _freq2;
	const uint8 _vol2;
};

class V2A_Sound_MultiLoopedDuration : public V2A_Sound_MultiLooped {
public:
	V2A_Sound_MultiLoopedDuration(uint16 offset, uint16 size, uint16 freq1, uint8 vol1, uint16 freq2, uint8 vol2, uint16 numframes) :
		V2A_Sound_MultiLooped(offset, size, freq1, vol1, freq2, vol2), _duration(numframes) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		V2A_Sound_MultiLooped::start(mod, id, data);
		_ticks = 0;
	}
	virtual bool update() {
		assert(_id);
		_ticks++;
		if (_ticks >= _duration)
			return false;
		return true;
	}
private:
	const uint16 _duration;

	int _ticks;
};

class V2A_Sound_SingleLoopedPitchbend : public V2A_Sound_Base<1> {
public:
	V2A_Sound_SingleLoopedPitchbend(uint16 offset, uint16 size, uint16 freq1, uint16 freq2, uint8 vol, uint8 step) :
		V2A_Sound_Base<1>(offset, size), _freq1(freq1), _freq2(freq2), _vol(vol), _step(step) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, data + _offset, _size);
		int vol = (_vol << 2) | (_vol >> 4);
		_curfreq = _freq1;
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _curfreq, vol, 0, _size);
	}
	virtual bool update() {
		assert(_id);
		if (_freq1 < _freq2) {
			_curfreq += _step;
			if (_curfreq > _freq2)
				_curfreq = _freq2;
			else
				_mod->setChannelFreq(_id, BASE_FREQUENCY / _curfreq);
		} else {
			_curfreq -= _step;
			if (_curfreq < _freq2)
				_curfreq = _freq2;
			else
				_mod->setChannelFreq(_id, BASE_FREQUENCY / _curfreq);
		}
		return true;
	}
private:
	const uint16 _freq1;
	const uint16 _freq2;
	const uint8 _vol;
	const uint16 _step;

	uint16 _curfreq;
};

class V2A_Sound_Special_FastPitchbendDownAndFadeout : public V2A_Sound_Base<1> {
public:
	V2A_Sound_Special_FastPitchbendDownAndFadeout(uint16 offset, uint16 size, uint16 freq, uint8 vol) :
		V2A_Sound_Base<1>(offset, size), _freq(freq), _vol(vol) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, data + _offset, _size);
		_curvol = (_vol << 3) | (_vol >> 3);
		_curfreq = _freq;
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _curfreq, _curvol, 0, _size);
	}
	virtual bool update() {
		assert(_id);
		_curfreq += 2;
		_mod->setChannelFreq(_id, BASE_FREQUENCY / _curfreq);
		_curvol--;
		if (_curvol == 0)
			return false;
		_mod->setChannelVol(_id, _curvol >> 1);
		return true;
	}
private:
	const uint16 _freq;
	const uint8 _vol;

	uint16 _curfreq;
	uint16 _curvol;
};

class V2A_Sound_Special_LoopedFadeinFadeout : public V2A_Sound_Base<1> {
public:
	V2A_Sound_Special_LoopedFadeinFadeout(uint16 offset, uint16 size, uint16 freq, uint8 fadeinrate, uint8 fadeoutrate) :
		V2A_Sound_Base<1>(offset, size), _freq(freq), _fade1(fadeinrate), _fade2(fadeoutrate) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, data + _offset, _size);
		_curvol = 1;
		_dir = 0;
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _freq, 1, 0, _size);
	}
	virtual bool update() {
		assert(_id);
		if (_dir == 0) {
			_curvol += _fade1;
			if (_curvol > 0x3F) {
				_curvol = 0x3F;
				_dir = 1;
			}
		} else {
			_curvol -= _fade2;
			if (_curvol < 1)
				return false;
		}
		_mod->setChannelVol(_id, (_curvol << 2) | (_curvol >> 4));
		return true;
	}
private:
	const uint16 _freq;
	const uint16 _fade1;
	const uint16 _fade2;

	int _curvol;
	int _dir;
};

class V2A_Sound_Special_MultiLoopedFadeinFadeout : public V2A_Sound_Base<2> {
public:
	V2A_Sound_Special_MultiLoopedFadeinFadeout(uint16 offset, uint16 size, uint16 freq1, uint16 freq2, uint8 fadeinrate, uint8 fadeoutrate) :
		V2A_Sound_Base<2>(offset, size), _freq1(freq1), _freq2(freq2), _fade1(fadeinrate), _fade2(fadeoutrate) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data1 = (char *)malloc(_size);
		char *tmp_data2 = (char *)malloc(_size);
		memcpy(tmp_data1, data + _offset, _size);
		memcpy(tmp_data2, data + _offset, _size);
		_curvol = 1;
		_dir = 0;
		_mod->startChannel(_id | 0x000, tmp_data1, _size, BASE_FREQUENCY / _freq1, 1, 0, _size, -127);
		_mod->startChannel(_id | 0x100, tmp_data2, _size, BASE_FREQUENCY / _freq2, 1, 0, _size, 127);
	}
	virtual bool update() {
		assert(_id);
		if (_dir == 0) {
			_curvol += _fade1;
			if (_curvol > 0x3F) {
				_curvol = 0x3F;
				_dir = 1;
			}
		} else {
			_curvol -= _fade2;
			if (_curvol < 1)
				return false;
		}
		_mod->setChannelVol(_id | 0x000, (_curvol << 1) | (_curvol >> 5));
		_mod->setChannelVol(_id | 0x100, (_curvol << 1) | (_curvol >> 5));
		return true;
	}
private:
	const uint16 _freq1;
	const uint16 _freq2;
	const uint16 _fade1;
	const uint16 _fade2;

	int _curvol;
	int _dir;
};

class V2A_Sound_Special_PitchbendDownThenFadeout : public V2A_Sound_Base<1> {
public:
	V2A_Sound_Special_PitchbendDownThenFadeout(uint16 offset, uint16 size, uint16 freq1, uint16 freq2, uint16 step) :
		V2A_Sound_Base<1>(offset, size), _freq1(freq1), _freq2(freq2), _step(step) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, data + _offset, _size);
		_curfreq = _freq1;
		_curvol = 0x3F;
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _curfreq, (_curvol << 2) | (_curvol >> 4), 0, _size);
	}
	virtual bool update() {
		assert(_id);
		if (_curfreq > _freq2)
			_curvol = 0x3F + _freq2 - _curfreq;
		if (_curvol < 1)
			return false;
		_curfreq += _step;
		_mod->setChannelFreq(_id, BASE_FREQUENCY / _curfreq);
		_mod->setChannelVol(_id, (_curvol << 2) | (_curvol >> 4));
		return true;
	}
private:
	const uint16 _freq1;
	const uint16 _freq2;
	const uint16 _step;

	uint16 _curfreq;
	int _curvol;
};

class V2A_Sound_Special_PitchbendDownAndBackUp : public V2A_Sound_Base<1> {
public:
	V2A_Sound_Special_PitchbendDownAndBackUp(uint16 offset, uint16 size, uint16 freq1, uint16 freq2, uint16 step, uint8 vol) :
		V2A_Sound_Base<1>(offset, size), _freq1(freq1), _freq2(freq2), _step(step), _vol(vol) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, data + _offset, _size);
		int vol = (_vol << 2) | (_vol >> 4);
		_curfreq = _freq1;
		_dir = 2;
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _curfreq, vol, 0, _size);
	}
	virtual bool update() {
		assert(_id);
		if (_dir == 2) {
			_curfreq += _step;
			if (_curfreq > _freq2) {
				_curfreq = _freq2;
				_dir = 1;
			}
			_mod->setChannelFreq(_id, BASE_FREQUENCY / _curfreq);
		} else if (_dir == 1) {
			_curfreq -= _step;
			if (_curfreq < _freq1) {
				_curfreq = _freq1;
				_dir = 0;
			}
			_mod->setChannelFreq(_id, BASE_FREQUENCY / _curfreq);
		}
		return true;
	}
private:
	const uint16 _freq1;
	const uint16 _freq2;
	const uint16 _step;
	const uint8 _vol;

	uint16 _curfreq;
	int _dir;
};

class V2A_Sound_Special_SlowPitchbendDownAndFadeout : public V2A_Sound_Base<1> {
public:
	V2A_Sound_Special_SlowPitchbendDownAndFadeout(uint16 offset, uint16 size, uint16 freq1, uint16 freq2) :
		V2A_Sound_Base<1>(offset, size), _freq1(freq1), _freq2(freq2) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, data + _offset, _size);
		_curfreq = _freq1;
		_curvol = 0x3F;
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _curfreq, (_curvol << 2) | (_curvol >> 4), 0, _size);
	}
	virtual bool update() {
		assert(_id);
		_curfreq++;
		if (!(_curfreq & 3))
			_curvol--;
		if ((_curfreq == _freq2) || (_curvol == 0))
			return false;
		_mod->setChannelFreq(_id, BASE_FREQUENCY / _curfreq);
		_mod->setChannelVol(_id, (_curvol << 2) | (_curvol >> 4));
		return true;
	}
private:
	const uint16 _freq1;
	const uint16 _freq2;

	uint16 _curfreq;
	uint8 _curvol;
};

class V2A_Sound_Special_MultiLoopedDurationMulti : public V2A_Sound_Base<2> {
public:
	V2A_Sound_Special_MultiLoopedDurationMulti(uint16 offset, uint16 size, uint16 freq1, uint8 vol1, uint16 freq2, uint8 vol2, uint16 numframes, uint8 playwidth, uint8 loopwidth) :
		V2A_Sound_Base<2>(offset, size), _freq1(freq1), _vol1(vol1), _freq2(freq2), _vol2(vol2), _duration(numframes), _playwidth(playwidth), _loopwidth(loopwidth) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		_data = (char *)malloc(READ_LE_UINT16(data));
		memcpy(_data, data, READ_LE_UINT16(data));
		soundon();
		_ticks = 0;
		_loop = 0;
	}
	virtual bool update() {
		assert(_id);
		if (_loop == _playwidth) {
			_mod->stopChannel(_id | 0x000);
			_mod->stopChannel(_id | 0x100);
		}
		if (_loop == _loopwidth) {
			_loop = 0;
			soundon();
		}
		_loop++;
		_ticks++;
		if (_ticks >= _duration)
			return false;
		return true;
	}
private:
	const uint16 _freq1;
	const uint8 _vol1;
	const uint16 _freq2;
	const uint8 _vol2;
	const uint16 _duration;
	const uint8 _playwidth;
	const uint8 _loopwidth;

	int _ticks;
	int _loop;

	void soundon() {
		char *tmp_data1 = (char *)malloc(_size);
		char *tmp_data2 = (char *)malloc(_size);
		memcpy(tmp_data1, _data + _offset, _size);
		memcpy(tmp_data2, _data + _offset, _size);
		int vol1 = (_vol1 << 1) | (_vol1 >> 5);
		int vol2 = (_vol2 << 1) | (_vol2 >> 5);
		_mod->startChannel(_id | 0x000, tmp_data1, _size, BASE_FREQUENCY / _freq1, vol1, 0, _size, -127);
		_mod->startChannel(_id | 0x100, tmp_data2, _size, BASE_FREQUENCY / _freq2, vol2, 0, _size, 127);
	}
};

class V2A_Sound_Special_SingleDurationMulti : public V2A_Sound_Base<1> {
public:
	V2A_Sound_Special_SingleDurationMulti(uint16 offset, uint16 size, uint16 freq, uint8 vol, uint8 loopwidth, uint8 numloops) :
		V2A_Sound_Base<1>(offset, size), _freq(freq), _vol(vol), _loopwidth(loopwidth), _numloops(numloops) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		_data = (char *)malloc(READ_LE_UINT16(data));
		memcpy(_data, data, READ_LE_UINT16(data));
		soundon();
		_loop = 0;
		_loopctr = 0;
	}
	virtual bool update() {
		assert(_id);
		_loop++;
		if (_loop == _loopwidth) {
			_loop = 0;
			_loopctr++;
			if (_loopctr == _numloops)
				return false;
			_mod->stopChannel(_id);
			soundon();
		}
		return true;
	}
private:
	const uint16 _freq;
	const uint8 _vol;
	const uint8 _loopwidth;
	const uint8 _numloops;

	int _loop;
	int _loopctr;

	void soundon() {
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, _data + _offset, _size);
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _freq, (_vol << 2) | (_vol >> 4), 0, 0);
	}
};

class V2A_Sound_Special_SingleDurationMultiDurations : public V2A_Sound_Base<1> {
public:
	V2A_Sound_Special_SingleDurationMultiDurations(uint16 offset, uint16 size, uint16 freq, uint8 vol, uint8 numdurs, const uint8 *durations, bool looped) :
		V2A_Sound_Base<1>(offset, size), _freq(freq), _vol(vol), _numdurs(numdurs), _durations(durations), _looped(looped) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		_data = (char *)malloc(READ_LE_UINT16(data));
		memcpy(_data, data, READ_LE_UINT16(data));
		soundon();
		_curdur = 0;
		_ticks = _durations[_curdur++];
	}
	virtual bool update() {
		assert(_id);
		_ticks--;
		if (!_ticks) {
			if (_curdur == _numdurs) {
				if (_looped)
					_curdur = 0;
				else
					return false;
			}
			_mod->stopChannel(_id);
			soundon();
			_ticks = _durations[_curdur++];
		}
		return true;
	}
private:
	const uint16 _freq;
	const uint8 _vol;
	const uint8 _numdurs;
	const uint8 *_durations;
	const bool _looped;

	int _ticks;
	int _curdur;

	void soundon() {
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, _data + _offset, _size);
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _freq, (_vol << 2) | (_vol >> 4), 0, 0);
	}
};

class V2A_Sound_Special_TwinSirenMulti : public V2A_Sound_Base<2> {
public:
	V2A_Sound_Special_TwinSirenMulti(uint16 offset1, uint16 size1, uint16 offset2, uint16 size2, uint16 freq1, uint16 freq2, uint8 vol) :
		_offset1(offset1), _size1(size1), _offset2(offset2), _size2(size2), _freq1(freq1), _freq2(freq2), _vol(vol) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		_data = (char *)malloc(READ_LE_UINT16(data));
		memcpy(_data, data, READ_LE_UINT16(data));
		
		_loopnum = 1;
		_step = 2;
		_curfreq = _freq1;

		soundon(_data + _offset1, _size1);
	}
	virtual bool update() {
		assert(_id);
		_mod->setChannelFreq(_id | 0x000, BASE_FREQUENCY / _curfreq);
		_mod->setChannelFreq(_id | 0x100, BASE_FREQUENCY / (_curfreq + 3));
		_curfreq -= _step;
		if (_loopnum == 7) {
			if ((BASE_FREQUENCY / _curfreq) >= 65536)
				return false;
			else
				return true;
		}
		if (_curfreq >= _freq2)
			return true;
		const char steps[8] = {0,2,2,3,4,8,15,2};
		_curfreq = _freq1;
		_step = steps[++_loopnum];
		if (_loopnum == 7) {
			_mod->stopChannel(_id | 0x000);
			_mod->stopChannel(_id | 0x100);
			soundon(_data + _offset2, _size2);
		}
		return true;
	}
private:
	const uint16 _offset1;
	const uint16 _size1;
	const uint16 _offset2;
	const uint16 _size2;
	const uint16 _freq1;
	const uint16 _freq2;
	const uint8 _vol;

	int _curfreq;
	uint16 _loopnum;
	uint16 _step;

	void soundon(const char *data, int size) {
		char *tmp_data1 = (char *)malloc(size);
		char *tmp_data2 = (char *)malloc(size);
		memcpy(tmp_data1, data, size);
		memcpy(tmp_data2, data, size);
		int vol = (_vol << 1) | (_vol >> 5);
		_mod->startChannel(_id | 0x000, tmp_data1, size, BASE_FREQUENCY / _curfreq, vol, 0, size, -127);
		_mod->startChannel(_id | 0x100, tmp_data2, size, BASE_FREQUENCY / (_curfreq + 3), vol, 0, size, 127);
	}
};

class V2A_Sound_Special_QuadSiren : public V2A_Sound_Base<4> {
public:
	V2A_Sound_Special_QuadSiren(uint16 offset1, uint16 size1, uint16 offset2, uint16 size2, uint8 vol) :
		_offset1(offset1), _size1(size1), _offset2(offset2), _size2(size2), _vol(vol) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;

		_freq1 = 0x02D0;
		_step1 = -0x000A;
		_freq2 = 0x0122;
		_step2 = 0x000A;
		_freq3 = 0x02BC;
		_step3 = -0x0005;
		_freq4 = 0x010E;
		_step4 = 0x0007;

		char *tmp_data1 = (char *)malloc(_size1);
		char *tmp_data2 = (char *)malloc(_size2);
		char *tmp_data3 = (char *)malloc(_size1);
		char *tmp_data4 = (char *)malloc(_size2);
		memcpy(tmp_data1, data + _offset1, _size1);
		memcpy(tmp_data2, data + _offset2, _size2);
		memcpy(tmp_data3, data + _offset1, _size1);
		memcpy(tmp_data4, data + _offset2, _size2);
		_mod->startChannel(_id | 0x000, tmp_data1, _size1, BASE_FREQUENCY / _freq1, _vol, 0, _size1, -127);
		_mod->startChannel(_id | 0x100, tmp_data2, _size2, BASE_FREQUENCY / _freq2, _vol, 0, _size2, 127);
		_mod->startChannel(_id | 0x200, tmp_data3, _size1, BASE_FREQUENCY / _freq3, _vol, 0, _size1, 127);
		_mod->startChannel(_id | 0x300, tmp_data4, _size2, BASE_FREQUENCY / _freq4, _vol, 0, _size2, -127);
	}
	virtual bool update() {
		assert(_id);
		updatefreq(_freq1,_step1,0x00AA,0x00FA);
		updatefreq(_freq2,_step2,0x019A,0x03B6);
		updatefreq(_freq3,_step3,0x00AA,0x00FA);
		updatefreq(_freq4,_step4,0x019A,0x03B6);
		_mod->setChannelFreq(_id | 0x000, BASE_FREQUENCY / _freq1);
		_mod->setChannelFreq(_id | 0x100, BASE_FREQUENCY / _freq2);
		_mod->setChannelFreq(_id | 0x200, BASE_FREQUENCY / _freq3);
		_mod->setChannelFreq(_id | 0x300, BASE_FREQUENCY / _freq4);
		return true;
	}
private:
	const uint16 _offset1;
	const uint16 _size1;
	const uint16 _offset2;
	const uint16 _size2;
	const uint8 _vol;

	uint16 _freq1;
	int16 _step1;
	uint16 _freq2;
	int16 _step2;
	uint16 _freq3;
	int16 _step3;
	uint16 _freq4;
	int16 _step4;

	void updatefreq (uint16 &freq, int16 &step, uint16 min, uint16 max) {
		freq += step;
		if (freq <= min) {
			freq = min;
			step = -step;
		}
		if (freq >= max) {
			freq = max;
			step = -step;
		}
	}
};

class V2A_Sound_Special_QuadFreqLooped : public V2A_Sound_Base<4> {
public:
	V2A_Sound_Special_QuadFreqLooped(uint16 offset, uint16 size, uint16 freq1, uint16 freq2, uint16 freq3, uint16 freq4, uint8 vol) :
		V2A_Sound_Base<4>(offset, size), _freq1(freq1), _freq2(freq2), _freq3(freq3), _freq4(freq4), _vol(vol) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		
		char *tmp_data1 = (char *)malloc(_size);
		char *tmp_data2 = (char *)malloc(_size);
		char *tmp_data3 = (char *)malloc(_size);
		char *tmp_data4 = (char *)malloc(_size);
		memcpy(tmp_data1, data + _offset, _size);
		memcpy(tmp_data2, data + _offset, _size);
		memcpy(tmp_data3, data + _offset, _size);
		memcpy(tmp_data4, data + _offset, _size);
		_mod->startChannel(_id | 0x000, tmp_data1, _size, BASE_FREQUENCY / _freq1, _vol, 0, _size, -127);
		_mod->startChannel(_id | 0x100, tmp_data2, _size, BASE_FREQUENCY / _freq2, _vol, 0, _size, 127);
		_mod->startChannel(_id | 0x200, tmp_data3, _size, BASE_FREQUENCY / _freq3, _vol, 0, _size, 127);
		_mod->startChannel(_id | 0x300, tmp_data4, _size, BASE_FREQUENCY / _freq4, _vol, 0, _size, -127);
	}
	virtual bool update() {
		assert(_id);
		return true;
	}
protected:
	const uint16 _freq1;
	const uint16 _freq2;
	const uint16 _freq3;
	const uint16 _freq4;
	const uint8 _vol;
};

class V2A_Sound_Special_QuadFreqFadeout : public V2A_Sound_Special_QuadFreqLooped {
public:
	V2A_Sound_Special_QuadFreqFadeout(uint16 offset, uint16 size, uint16 freq1, uint16 freq2, uint16 freq3, uint16 freq4, uint8 vol, uint16 dur) :
		V2A_Sound_Special_QuadFreqLooped(offset, size, freq1, freq2, freq3, freq4, vol), _dur(dur) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		V2A_Sound_Special_QuadFreqLooped::start(mod, id, data);
		_ticks = _dur;
	}
	virtual bool update() {
		assert(_id);
		if (!--_ticks)
			return false;
		if (_ticks < _vol) {
			_mod->setChannelVol(_id | 0x000, _ticks);
			_mod->setChannelVol(_id | 0x100, _ticks);
			_mod->setChannelVol(_id | 0x200, _ticks);
			_mod->setChannelVol(_id | 0x300, _ticks);
		}
		return true;
	}
private:
	const uint16 _dur;

	int _ticks;
};

class V2A_Sound_Special_SingleFadeout : public V2A_Sound_Base<1> {
public:
	V2A_Sound_Special_SingleFadeout(uint16 offset, uint16 size, uint16 freq, uint8 vol) :
		V2A_Sound_Base<1>(offset, size), _freq(freq), _vol(vol) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, data + _offset, _size);
		_curvol = _vol << 2;
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _freq, (_curvol << 2) | (_curvol >> 4), 0, _size);
	}
	virtual bool update() {
		assert(_id);
		if (!--_curvol)
			return false;
		_mod->setChannelVol(_id, _curvol);
		return true;
	}
private:
	const uint16 _freq;
	const uint8 _vol;

	int _curvol;
};

class V2A_Sound_Special_SlowPitchbendThenSlowFadeout : public V2A_Sound_Base<1> {
public:
	V2A_Sound_Special_SlowPitchbendThenSlowFadeout(uint16 offset, uint16 size, uint16 freq1, uint16 freq2) :
		V2A_Sound_Base<1>(offset, size), _freq1(freq1), _freq2(freq2) { }
	virtual void start(Player_MOD *mod, int id, const byte *data) {
		_mod = mod;
		_id = id;
		char *tmp_data = (char *)malloc(_size);
		memcpy(tmp_data, data + _offset, _size);
		_curfreq = _freq1;
		_curvol = 0x3F;
		_mod->startChannel(_id, tmp_data, _size, BASE_FREQUENCY / _curfreq, (_curvol << 2) | (_curvol >> 4), 0, _size);
		_ticks = 0;
	}
	virtual bool update() {
		assert(_id);
		_ticks++;
		if (_ticks < 4)
			return true;
		_ticks = 0;
		if (_curfreq == _freq2) {
			_curvol--;
			if (_curvol == 0)
				return false;
			_mod->setChannelVol(_id, (_curvol << 2) | (_curvol >> 4));
		}
		else {
			if (_freq1 < _freq2)
				_curfreq++;
			else
				_curfreq--;
			_mod->setChannelFreq(_id, BASE_FREQUENCY / _curfreq);
		}
		return true;
	}
private:
	const uint16 _freq1;
	const uint16 _freq2;

	uint16 _curfreq;
	int _curvol;
	int _ticks;
};

struct soundObj {
	~soundObj() { delete sound; }
	uint32 crc;
	V2A_Sound *sound;
};
static soundObj sndobjs[] = {
	{0x8FAB08C4,new V2A_Sound_SingleLooped(0x006C,0x2B58,0x016E,0x3F)},	// Maniac 17
	{0xB673160A,new V2A_Sound_SingleLooped(0x006C,0x1E78,0x01C2,0x1E)},	// Maniac 38
	{0x4DB1D0B2,new V2A_Sound_MultiLooped(0x0072,0x1BC8,0x023D,0x3F,0x0224,0x3F)},	// Maniac 20
	{0x754D75EF,new V2A_Sound_Single(0x0076,0x0738,0x01FC,0x3F)},	// Maniac 10
	{0x6E3454AF,new V2A_Sound_Single(0x0076,0x050A,0x017C,0x3F)},	// Maniac 12
	{0x92F0BBB6,new V2A_Sound_Single(0x0076,0x3288,0x012E,0x3F)},	// Maniac 41
	{0xE1B13982,new V2A_Sound_MultiLoopedDuration(0x0078,0x0040,0x007C,0x3F,0x007B,0x3F,0x001E)},	// Maniac 21
	{0x288B16CF,new V2A_Sound_MultiLoopedDuration(0x007A,0x0040,0x007C,0x3F,0x007B,0x3F,0x000A)},	// Maniac 11
	{0xA7565268,new V2A_Sound_MultiLoopedDuration(0x007A,0x0040,0x00F8,0x3F,0x00F7,0x3F,0x000A)},	// Maniac 19
	{0x7D419BFC,new V2A_Sound_MultiLoopedDuration(0x007E,0x0040,0x012C,0x3F,0x0149,0x3F,0x001E)},	// Maniac 22
	{0x1B52280C,new V2A_Sound_Single(0x0098,0x0A58,0x007F,0x32)},	// Maniac 6
	{0x38D4A810,new V2A_Sound_Single(0x0098,0x2F3C,0x0258,0x32)},	// Maniac 7
	{0x09F98FC2,new V2A_Sound_Single(0x0098,0x0A56,0x012C,0x32)},	// Maniac 16
	{0x90440A65,new V2A_Sound_Single(0x0098,0x0208,0x0078,0x28)},	// Maniac 28
	{0x985C76EF,new V2A_Sound_Single(0x0098,0x0D6E,0x00C8,0x32)},	// Maniac 30
	{0x76156137,new V2A_Sound_Single(0x0098,0x2610,0x017C,0x39)},	// Maniac 39
	{0x5D95F88C,new V2A_Sound_Single(0x0098,0x0A58,0x007F,0x1E)},	// Maniac 65
	{0x92D704EA,new V2A_Sound_SingleLooped(0x009C,0x29BC,0x012C,0x3F,0x1BD4,0x0DE8)},	// Maniac 15
	{0x92F5513C,new V2A_Sound_Single(0x009E,0x0DD4,0x01F4,0x3F)},	// Maniac 13
	{0xCC2F3B5A,new V2A_Sound_Single(0x009E,0x00DE,0x01AC,0x3F)},	// Maniac 43
	{0x153207D3,new V2A_Sound_Single(0x009E,0x0E06,0x02A8,0x3F)},	// Maniac 67
	{0xC4F370CE,new V2A_Sound_Single(0x00AE,0x0330,0x01AC,0x3F)},	// Maniac 8
	{0x928C4BAC,new V2A_Sound_Single(0x00AE,0x08D6,0x01AC,0x3F)},	// Maniac 9
	{0x62D5B11F,new V2A_Sound_Single(0x00AE,0x165C,0x01CB,0x3F)},	// Maniac 27
	{0x3AB22CB5,new V2A_Sound_Single(0x00AE,0x294E,0x012A,0x3F)},	// Maniac 62
	{0x2D70BBE9,new V2A_Sound_SingleLoopedPitchbend(0x00B4,0x1702,0x03E8,0x0190,0x3F,5)},	// Maniac 64
	{0xFA4C1B1C,new V2A_Sound_Special_FastPitchbendDownAndFadeout(0x00B2,0x1702,0x0190,0x3F)},	// Maniac 69
	{0x19D50D67,new V2A_Sound_Special_LoopedFadeinFadeout(0x00B6,0x0020,0x00C8,16,2)},	// Maniac 14
	{0x3E6FBE15,new V2A_Sound_Special_PitchbendDownThenFadeout(0x00B2,0x0010,0x007C,0x016D,1)},	// Maniac 25
	{0x5305753C,new V2A_Sound_Special_PitchbendDownThenFadeout(0x00B2,0x0010,0x007C,0x016D,7)},	// Maniac 36
	{0x28895106,new V2A_Sound_Special_PitchbendDownAndBackUp(0x00C0,0x00FE,0x00E9,0x0111,4,0x0A)},	// Maniac 59
	{0xB641ACF6,new V2A_Sound_Special_SlowPitchbendDownAndFadeout(0x00C8,0x0100,0x00C8,0x01C2)},	// Maniac 61
	{0xE1A91583,new V2A_Sound_Special_MultiLoopedDurationMulti(0x00D0,0x0040,0x007C,0x3F,0x007B,0x3F,0x3C,5,6)},	// Maniac 23
	{0x64816ED5,new V2A_Sound_Special_MultiLoopedDurationMulti(0x00D0,0x0040,0x00BE,0x37,0x00BD,0x37,0x3C,5,6)},	// Maniac 24
	{0x639D72C2,new V2A_Sound_Special_SingleDurationMulti(0x00D0,0x10A4,0x0080,0x3F,0x28,3)},	// Maniac 46
	{0xE8826D92,new V2A_Sound_Special_SingleDurationMultiDurations(0x00EC,0x025A,0x023C,0x3F,8,(const uint8 *)"\x20\x41\x04\x21\x08\x10\x13\x07", true)},	// Maniac 45
	{0xEDFF3D41,new V2A_Sound_Single(0x00F8,0x2ADE,0x01F8,0x3F)},	// Maniac 42 (this should echo, but it's barely noticeable and I don't feel like doing it)
	{0x15606D06,new V2A_Sound_Special_QuadSiren(0x0148,0x0020,0x0168,0x0020,0x3F)},	// Maniac 32
	{0x753EAFE3,new V2A_Sound_Special_TwinSirenMulti(0x017C,0x0010,0x018C,0x0020,0x00C8,0x0080,0x3F)},	// Maniac 44
	{0xB1AB065C,new V2A_Sound_Music(0x0032,0x00B2,0x08B2,0x1222,0x1A52,0x23C2,0x3074,false)},	// Maniac 50
	{0x091F5D9C,new V2A_Sound_Music(0x0032,0x0132,0x0932,0x1802,0x23D2,0x3EA2,0x4F04,false)},	// Maniac 58

	{0x8E2C8AB3,new V2A_Sound_SingleLooped(0x005C,0x0F26,0x0168,0x3C)},	// Zak 41
	{0x3792071F,new V2A_Sound_SingleLooped(0x0060,0x1A18,0x06A4,0x3F)},	// Zak 88
	{0xF192EDE9,new V2A_Sound_SingleLooped(0x0062,0x0054,0x01FC,0x1E)},	// Zak 68
	{0xC43B0245,new V2A_Sound_Special_QuadFreqLooped(0x006C,0x166E,0x00C8,0x0190,0x0320,0x0640,0x32)},	// Zak 70
	{0xCEB51670,new V2A_Sound_SingleLooped(0x00AC,0x26DC,0x012C,0x3F)},	// Zak 42
	{0x10347B51,new V2A_Sound_SingleLooped(0x006C,0x00E0,0x0594,0x3F)},	// Zak 18
	{0x9D2FADC0,new V2A_Sound_MultiLooped(0x0072,0x1FC8,0x016A,0x3F,0x01CE,0x3F)},	// Zak 80
	{0xFAD2C676,new V2A_Sound_MultiLooped(0x0076,0x0010,0x0080,0x3F,0x0090,0x3B)},	// Zak 40
	{0x01508B48,new V2A_Sound_Single(0x0076,0x0D8C,0x017C,0x3F)},	// Zak 90
	{0x9C18DC46,new V2A_Sound_Single(0x0076,0x0D8C,0x015E,0x3F)},	// Zak 91
	{0xF98F7EAC,new V2A_Sound_Single(0x0076,0x0D8C,0x0140,0x3F)},	// Zak 92
	{0xC925FBEF,new V2A_Sound_MultiLoopedDuration(0x0080,0x0010,0x0080,0x3F,0x0090,0x3B,0x0168)},	// Zak 53
	{0xCAB35257,new V2A_Sound_Special_QuadFreqFadeout(0x00DA,0x425C,0x023C,0x08F0,0x0640,0x0478,0x3F,0x012C)},	// Zak 101
	{0xA31FE4FD,new V2A_Sound_Single(0x0094,0x036A,0x00E1,0x3F)},	// Zak 97
	{0x0A1AE0F5,new V2A_Sound_Single(0x009E,0x0876,0x0168,0x3F)},	// Zak 5
	{0xD01A66CB,new V2A_Sound_Single(0x009E,0x04A8,0x0168,0x3F)},	// Zak 47
	{0x5497B912,new V2A_Sound_Single(0x009E,0x0198,0x01F4,0x3F)},	// Zak 39
	{0x2B50362F,new V2A_Sound_Single(0x009E,0x09B6,0x023D,0x3F)},	// Zak 67
	{0x7BFB6E72,new V2A_Sound_Single(0x009E,0x0D14,0x0078,0x3F)},	// Zak 69
	{0xB803A792,new V2A_Sound_Single(0x009E,0x2302,0x02BC,0x3F)},	// Zak 78
	{0x7AB82E39,new V2A_Sound_SingleLooped(0x00A0,0x2A3C,0x016E,0x3F,0x1018,0x1A24)},	// Zak 100
	{0x28057CEC,new V2A_Sound_Single(0x0098,0x0FEC,0x0140,0x32)},	// Zak 63
	{0x1180A2FC,new V2A_Sound_Single(0x0098,0x0F06,0x0190,0x32)},	// Zak 64
	{0x12616755,new V2A_Sound_Single(0x0098,0x14C8,0x023C,0x14)},	// Zak 9
	{0x642723AA,new V2A_Sound_Special_SingleFadeout(0x00A2,0x1702,0x01F4,0x3F)},	// Zak 37
	{0xDEE56848,new V2A_Sound_Single(0x009A,0x0F86,0x0100,0x3F)},	// Zak 93
	{0xF9BE27B8,new V2A_Sound_Special_SingleFadeout(0x011C,0x1704,0x0228,0x3F)},	// Zak 113
	{0xC73487B2,new V2A_Sound_Single(0x00B0,0x18BA,0x0478,0x3F)},	// Zak 81
	{0x32D8F925,new V2A_Sound_Single(0x00B0,0x2E46,0x00F0,0x3F)},	// Zak 94
	{0x988C83A5,new V2A_Sound_Single(0x00B0,0x0DE0,0x025B,0x3F)},	// Zak 106
	{0x8F1E3B3D,new V2A_Sound_Single(0x00B0,0x05FE,0x04E2,0x3F)},	// Zak 107
	{0x0A2A7646,new V2A_Sound_Single(0x00B0,0x36FE,0x016E,0x3F)},	// Zak 43
	{0x6F1FC435,new V2A_Sound_Single(0x00B0,0x2808,0x044C,0x3F)},	// Zak 108
	{0x870EFC29,new V2A_Sound_Unsupported()},	// Zak 55
	{0xED773699,new V2A_Sound_Special_LoopedFadeinFadeout(0x00B4,0x0020,0x012C,8,4)},	// Zak 3
	{0x0BF59774,new V2A_Sound_Special_MultiLoopedFadeinFadeout(0x00BE,0x0020,0x00F8,0x00F7,8,1)},	// Zak 72
	{0x656FFEDE,new V2A_Sound_Special_MultiLoopedFadeinFadeout(0x00BE,0x0020,0x00C4,0x00C3,8,1)},	// Zak 73
	{0xFC4D41E5,new V2A_Sound_Special_MultiLoopedFadeinFadeout(0x00BE,0x0020,0x00A5,0x00A4,8,1)},	// Zak 74
	{0xC0DD2089,new V2A_Sound_Special_MultiLoopedFadeinFadeout(0x00BE,0x0020,0x009C,0x009B,8,1)},	// Zak 75
	{0x627DFD92,new V2A_Sound_Special_MultiLoopedFadeinFadeout(0x00BE,0x0020,0x008B,0x008A,8,1)},	// Zak 76
	{0x703E05C1,new V2A_Sound_Special_MultiLoopedFadeinFadeout(0x00BE,0x0020,0x007C,0x007B,8,1)},	// Zak 77
	{0xB0F77006,new V2A_Sound_Unsupported()},	// Zak 52
	{0x5AE9D6A7,new V2A_Sound_Special_SlowPitchbendThenSlowFadeout(0x00CA,0x22A4,0x0113,0x0227)},	// Zak 109
	{0xABE0D3B0,new V2A_Sound_Special_SlowPitchbendThenSlowFadeout(0x00CE,0x22A4,0x0227,0x0113)},	// Zak 105
	{0x788CC749,new V2A_Sound_Unsupported()},	// Zak 71
	{0x2E2AB1FA,new V2A_Sound_Unsupported()},	// Zak 99
	{0x1304CF20,new V2A_Sound_Special_SingleDurationMultiDurations(0x00DC,0x0624,0x023C,0x3C,2,(const uint8 *)"\x14\x11",false)},	// Zak 79
	{0xAE68ED91,new V2A_Sound_Unsupported()},	// Zak 54
	{0xA4F40F97,new V2A_Sound_Unsupported()},	// Zak 61
	{0x348F85CE,new V2A_Sound_Unsupported()},	// Zak 62
	{0xD473AB86,new V2A_Sound_Special_SingleDurationMultiDurations(0x0122,0x03E8,0x00BE,0x3F,7,(const uint8 *)"\x0F\x0B\x04\x0F\x1E\x0F\x66",false)},	// Zak 46
	{0x84A0BA90,new V2A_Sound_Unsupported()},	// Zak 110
	{0x92680D9F,new V2A_Sound_Unsupported()},	// Zak 32
	{0xABFFDB02,new V2A_Sound_Unsupported()},	// Zak 86
	{0x41045447,new V2A_Sound_Unsupported()},	// Zak 98
	{0xC8EEBD34,new V2A_Sound_Unsupported()},	// Zak 82
	{0x42F9469F,new V2A_Sound_Music(0x05F6,0x0636,0x0456,0x0516,0x05D6,0x05E6,0x0A36,true)},	// Zak 96
	{0x038BBD78,new V2A_Sound_Music(0x054E,0x05CE,0x044E,0x04BE,0x052E,0x053E,0x0BCE,true)},	// Zak 85
	{0x06FFADC5,new V2A_Sound_Music(0x0626,0x0686,0x0446,0x04F6,0x0606,0x0616,0x0C86,true)},	// Zak 87
	{0xCE20ECF0,new V2A_Sound_Music(0x0636,0x0696,0x0446,0x0576,0x0616,0x0626,0x0E96,true)},	// Zak 114
	{0xBDA01BB6,new V2A_Sound_Music(0x0678,0x06B8,0x0458,0x0648,0x0658,0x0668,0x0EB8,false)},	// Zak 33
	{0x59976529,new V2A_Sound_Music(0x088E,0x092E,0x048E,0x05EE,0x074E,0x07EE,0x112E,true)},	// Zak 49
	{0xED1EED02,new V2A_Sound_Music(0x08D0,0x0950,0x0440,0x07E0,0x08B0,0x08C0,0x1350,false)},	// Zak 112
	{0x5A16C037,new V2A_Sound_Music(0x634A,0x64CA,0x049A,0x18FA,0x398A,0x511A,0x6CCA,false)},	// Zak 95
	{0x00000000,NULL}
};

static V2A_Sound *findSound (unsigned long crc) {
	for (int i = 0; sndobjs[i].crc != 0; i++) {
		if (sndobjs[i].crc == crc)
			return sndobjs[i].sound;
	}
	return NULL;
}

Player_V2A::Player_V2A(ScummEngine *scumm) {
	int i;
	_scumm = scumm;
	_system = scumm->_system;

	InitCRC();

	for (i = 0; i < V2A_MAXSLOTS; i++) {
		_slot[i].id = 0;
		_slot[i].sound = NULL;
	}

	_mod = new Player_MOD(scumm);
	_mod->setUpdateProc(update_proc, this, 60);
}

Player_V2A::~Player_V2A() {
	delete _mod;
}

void Player_V2A::setMasterVolume (int vol) {
	_mod->setMasterVolume(vol);
}

int Player_V2A::getSoundSlot (int id) const {
	int i;
	for (i = 0; i < V2A_MAXSLOTS; i++) {
		if (_slot[i].id == id)
			break;
	}
	if (i == V2A_MAXSLOTS) {
		if (id == 0)
			warning("player_v2a - out of sound slots");
		return -1;
	}
	return i;
}

void Player_V2A::stopAllSounds() {
	for (int i = 0; i < V2A_MAXSLOTS; i++) {
		if (!_slot[i].id)
			continue;
		_slot[i].sound->stop();
		_slot[i].sound = NULL;
		_slot[i].id = 0;
	}
}

void Player_V2A::stopSound(int nr) {
	int i;
	if (nr == 0)
		return;
	i = getSoundSlot(nr);
	if (i == -1)
		return;
	_slot[i].sound->stop();
	_slot[i].sound = NULL;
	_slot[i].id = 0;
}

void Player_V2A::startSound(int nr) {
	assert(_scumm);
	byte *data = _scumm->getResourceAddress(rtSound, nr);
	assert(data);
	uint32 crc = GetCRC(data + 0x0A,READ_BE_UINT16(data + 0x08));
	V2A_Sound *snd = findSound(crc);
	if (snd == NULL) {
		warning("player_v2a - sound %i not recognized yet (crc %08X)",nr,crc);
		return;
	}
	stopSound(nr);
	int i = getSoundSlot();
	if (i == -1)
		return;
	_slot[i].id = nr;
	_slot[i].sound = snd;
	_slot[i].sound->start(_mod,nr,data);
}

void Player_V2A::update_proc(void *param) {
	((Player_V2A *)param)->updateSound();
}

void Player_V2A::updateSound() {
	int i;
	for (i = 0; i < V2A_MAXSLOTS; i++) {
		if ((_slot[i].id) && (!_slot[i].sound->update())) {
			_slot[i].sound->stop();
			_slot[i].sound = NULL;
			_slot[i].id = 0;
		}
	}
}

int Player_V2A::getMusicTimer() const {
	return 0;	// FIXME - need to keep track of playing music resources
}

int Player_V2A::getSoundStatus(int nr) const {
	for (int i = 0; i < V2A_MAXSLOTS; i++) {
		if (_slot[i].id == nr)
			return 1;
	}
	return 0;
}
