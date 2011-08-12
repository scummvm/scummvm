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
#include "common/memstream.h"
#include "audio/decoders/raw.h"

namespace CGE {

Sound::Sound(CGEEngine *vm) : _vm(vm) {
	_audioStream = NULL;
	open();
}


Sound::~Sound() {
	close();
}


void Sound::close() {
	_vm->_midiPlayer.killMidi();
}


void Sound::open() {
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

void Sound::sndDigiStart(SmpInfo *PSmpInfo) {
	// Create an audio stream wrapper for sound
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(PSmpInfo->_saddr, 
		PSmpInfo->_slen, DisposeAfterUse::NO);
	_audioStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	// Start the new sound
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream);
}

void Sound::stop() {
	sndDigiStop(&_smpinf);
}

void Sound::sndDigiStop(SmpInfo *PSmpInfo) {
	if (_vm->_mixer->isSoundHandleActive(_soundHandle))
		_vm->_mixer->stopHandle(_soundHandle);
	_audioStream = NULL;
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

MusicPlayer::MusicPlayer() {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB);
	_driver = MidiDriver::createMidi(dev);
	_driver->open();
	_midiParser = NULL;
	_data = NULL;
}

MusicPlayer::~MusicPlayer() {
	killMidi();
	_driver->close();
	delete _driver;
}

void MusicPlayer::killMidi() {
	if (_midiParser) {
		// Stop MIDI playback
		_midiParser->unloadMusic();
		_driver->setTimerCallback(NULL, NULL);
		_driver->close();
		delete _midiParser;
		delete _data;

		// Reset playback objects
		_data = NULL;
		_midiParser = NULL;
	}
}

void MusicPlayer::loadMidi(int ref) {
	// Work out the filename and check the given MIDI file exists
	Common::String filename = Common::String::format("%.2d.MID", ref);
	if (!INI_FILE::exist(filename.c_str()))
		return;

	// Stop any currently playing MIDI file
	killMidi();

	// Read in the data for the file
	INI_FILE mid(filename.c_str());
	_dataSize = mid.size();
	_data = (byte *)malloc(_dataSize);
	mid.read(_data, _dataSize);

	// Start playing the music
	sndMidiStart();
}

void MusicPlayer::sndMidiStart() {
	_midiParser = MidiParser::createParser_SMF();

	if (_midiParser->loadMusic(_data, _dataSize)) {
		_midiParser->setTrack(0);
		_midiParser->setMidiDriver(_driver);
		_midiParser->setTimerRate(_driver->getBaseTempo());

		_midiParser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
		_driver->setTimerCallback(_midiParser, MidiParser::timerCallback);
	}	
}

} // End of namespace CGE
