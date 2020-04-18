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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_SOUND_H
#define CGE2_SOUND_H

#include "audio/midiplayer.h"
#include "audio/mixer.h"

namespace Audio {
class RewindableAudioStream;
}

namespace CGE2 {

class CGE2Engine;
class EncryptedStream;

// sample info
struct SmpInfo {
	const uint8  *_saddr;                              // address
	uint16  _slen;                                     // length
	uint16  _span;                                     // left/right pan (0-15)
	int     _counter;                                  // number of time the sample should be played
};

class DataCk {
	byte *_buf;
	int _ckSize;
public:
	DataCk(byte *buf, int bufSize);
	~DataCk();
	inline const byte *addr() {
		return _buf;
	}
	inline int size() {
		return _ckSize;
	}
};

class Sound {
public:
	SmpInfo _smpinf;

	explicit Sound(CGE2Engine *vm);
	~Sound();
	void open();
	void close();
	void play(Audio::Mixer::SoundType soundType, DataCk *wav, int pan = 8);
	int16 getRepeat();
	void setRepeat(int16 count);
	void stop();
	void checkSoundHandles();
private:
	int _soundRepeatCount;
	CGE2Engine *_vm;
	Audio::SoundHandle _sfxHandle;
	Audio::SoundHandle _speechHandle;
	Audio::RewindableAudioStream *_audioStream;

	void sndDigiStart(SmpInfo *PSmpInfo, Audio::Mixer::SoundType soundType);
	void sndDigiStop(Audio::SoundHandle &handle);
};

class Fx {
	CGE2Engine *_vm;

	DataCk *loadWave(EncryptedStream *file);
	Common::String name(int ref, int sub);
public:
	DataCk *_current;

	Fx(CGE2Engine *vm, int size);
	~Fx();
	void clear();
	bool exist(int ref, int sub = 0);
	DataCk *load(int ref, int sub = 0);
};

class MusicPlayer: public Audio::MidiPlayer {
private:
	CGE2Engine *_vm;
	byte *_data;
	int _dataSize;
	bool _isGM;

	// Start MIDI File
	void sndMidiStart();

	// Stop MIDI File
	void sndMidiStop();
public:
	explicit MusicPlayer(CGE2Engine *vm);
	~MusicPlayer() override;

	void loadMidi(int ref);
	void killMidi();

	void send(uint32 b) override;
	void sendToChannel(byte channel, uint32 b) override;
};

} // End of namespace CGE2

#endif // CGE2_SOUND_H
