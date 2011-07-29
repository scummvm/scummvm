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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/general.h"
#include "cge/sound.h"
#include "cge/text.h"
#include "cge/cfile.h"
#include "cge/vol.h"
#include "cge/cge_main.h"


namespace CGE {

Sound::Sound(CGEEngine *vm) : _vm(vm) {
	open();
}


Sound::~Sound() {
	close();
}


void Sound::close() {
	killMidi();
	sndDone();
}


void Sound::open() {
	sndInit();
	play((*_fx)[30000], 8);
}


void Sound::play(DataCk *wav, int pan, int cnt) {
	if (wav) {
		stop();
		_smpinf._saddr = (uint8 *) &*(wav->addr());
		_smpinf._slen = (uint16)wav->size();
		_smpinf._span = pan;
		_smpinf._sflag = cnt;
		sndDigiStart(&_smpinf);
	}
}


void Sound::stop() {
	sndDigiStop(&_smpinf);
}


Fx::Fx(int size) : _current(NULL) {
	_cache = new Han[size];
	for (_size = 0; _size < size; _size++) {
		_cache[_size]._ref = 0;
		_cache[_size]._wav = NULL;
	}
}


Fx::~Fx() {
	clear();
	delete[] _cache;
}


void Fx::clear() {
	Han *p, * q;
	for (p = _cache, q = p + _size; p < q; p++) {
		if (p->_ref) {
			p->_ref = 0;
			delete p->_wav;
			p->_wav = NULL;
		}
	}
	_current = NULL;
}


int Fx::find(int ref) {
	Han *p, * q;
	int i = 0;
	for (p = _cache, q = p + _size; p < q; p++) {
		if (p->_ref == ref)
			break;
		else
			++i;
	}
	return i;
}


void Fx::preload(int ref0) {
	Han *cacheLim = _cache + _size;
	int ref;

	for (ref = ref0; ref < ref0 + 10; ref++) {
		static char fname[] = "FX00000.WAV";
		wtom(ref, fname + 2, 10, 5);
		INI_FILE file = INI_FILE(fname);
		DataCk *wav = loadWave(&file);
		if (wav) {
			Han *p = &_cache[find(0)];
			if (p >= cacheLim)
				break;
			p->_wav = wav;
			p->_ref = ref;
		}
	}
}


DataCk *Fx::load(int idx, int ref) {
	static char fname[] = "FX00000.WAV";
	wtom(ref, fname + 2, 10, 5);

	INI_FILE file = INI_FILE(fname);
	DataCk *wav = loadWave(&file);
	if (wav) {
		Han *p = &_cache[idx];
		p->_wav = wav;
		p->_ref = ref;
	}
	return wav;
}


DataCk *Fx::operator [](int ref) {
	int i;
	if ((i = find(ref)) < _size)
		_current = _cache[i]._wav;
	else {
		if ((i = find(0)) >= _size) {
			clear();
			i = 0;
		}
		_current = load(i, ref);
	}
	return _current;
}


static uint8 *midi = NULL;


void killMidi() {
	sndMidiStop();
	if (midi) {
		delete[] midi;
		midi = NULL;
	}
}


void loadMidi(int ref) {
	static char fn[] = "00.MID";
	wtom(ref, fn, 10, 2);
	if (INI_FILE::exist(fn)) {
		killMidi();
		INI_FILE mid = fn;
		if (mid._error == 0) {
			uint16 siz = (uint16) mid.size();
			midi = new uint8[siz];
			if (midi) {
				mid.read(midi, siz);
				if (mid._error)
					killMidi();
				else
					sndMidiStart(midi);
			}
		}
	}
}


void *Patch(int pat) {
	void *p = NULL;
	static char fn[] = "PATCH000.SND";

	wtom(pat, fn + 5, 10, 3);
	INI_FILE snd = fn;
	if (!snd._error) {
		uint16 siz = (uint16) snd.size();
		p = (uint8 *) malloc(siz);
		if (p) {
			snd.read(p, siz);
			if (snd._error) {
				free(p);
				p = NULL;
			}
		}
	}
	return p;
}

} // End of namespace CGE
