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

#ifndef __CGE_SOUND__
#define __CGE_SOUND__

#include "cge/wav.h"
#include "cge/snddrv.h"

namespace CGE {

#define BAD_SND_TEXT    97
#define BAD_MIDI_TEXT   98


class Sound {
public:
	SmpInfo _smpinf;
	Sound();
	~Sound();
	void open();
	void close();
	void play(DataCk *wav, int pan, int cnt = 1);
	void stop();
};


class Fx {
	Emm _emm;
	struct Han {
		int _ref;
		DataCk *_wav;
	} *_cache;
	int _size;
	DataCk *load(int idx, int ref);
	int find(int ref);
public:
	DataCk *_current;
	Fx(int size = 16);
	~Fx();
	void clear();
	void preload(int ref0);
	DataCk *operator[](int ref);
};

extern  Sound  _sound;
extern  Fx     _fx;


void loadMidi(int ref);
void killMidi();

} // End of namespace CGE

#endif

