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

#include "cge/general.h"
#include "cge/snddrv.h"
#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/fmopl.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/midiplayer.h"
#include "audio/mixer.h"
#include "common/memstream.h"

namespace CGE {

class CGEEngine;

class DataCk {
	byte *_buf;
	int _ckSize;
public:
	DataCk(byte *buf, int size);
	~DataCk();
	inline const byte *addr() {
		return _buf;
	}
	inline int size() {
		return _ckSize;
	}
};

DataCk *loadWave(XFile *file);

class Sound {
public:
	SmpInfo _smpinf;
	Sound(CGEEngine *vm);
	~Sound();
	void open();
	void close();
	void play(DataCk *wav, int pan, int cnt = 1);
	void stop();
private:
	CGEEngine *_vm;
	Audio::SoundHandle _soundHandle;
	Audio::RewindableAudioStream *_audioStream;

	void sndDigiStart(SmpInfo *PSmpInfo);
	void sndDigiStop(SmpInfo *PSmpInfo);
};


class Fx {
	struct Han {
		int _ref;
		DataCk *_wav;
	} *_cache;
	int _size;
	DataCk *load(int idx, int ref);
	int find(int ref);
public:
	DataCk *_current;
	Fx(int size);
	~Fx();
	void clear();
	void preload(int ref0);
	DataCk *operator[](int ref);
};

class MusicPlayer: public Audio::MidiPlayer {
private:
	byte *_data;
	int _dataSize;
	bool _isGM;

	// Start MIDI File
	void sndMidiStart();

	// Stop MIDI File
	void sndMidiStop();
public:
	MusicPlayer();
	~MusicPlayer();

	void loadMidi(int ref);
	void killMidi();

	virtual void send(uint32 b);
	virtual void sendToChannel(byte channel, uint32 b);
};

} // End of namespace CGE

#endif

