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

#include "cge2/sound.h"
#include "common/memstream.h"
#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "cge2/cge2.h"

namespace CGE2 {

DataCk::DataCk(byte *buf, int bufSize) {
	_buf = buf;
	_ckSize = bufSize;
}

DataCk::~DataCk() {
	free(_buf);
}

Sound::Sound(CGE2Engine *vm) : _vm(vm) {
	_audioStream = nullptr;
	_soundRepeatCount = 1;
	open();
}

Sound::~Sound() {
	close();
}

void Sound::close() {
	_vm->_midiPlayer->killMidi();
}

void Sound::open() {
	setRepeat(1);
	if (_vm->_commandHandlerTurbo != nullptr)
		_vm->switchSay();
	play(Audio::Mixer::kSFXSoundType, _vm->_fx->load(99, 99));
}

void Sound::setRepeat(int16 count) {
	_soundRepeatCount = count;
}

int16 Sound::getRepeat() {
	return _soundRepeatCount;
}

void Sound::play(Audio::Mixer::SoundType soundType, DataCk *wav, int pan) {
	if (wav) {
		stop();
		_smpinf._saddr = &*(wav->addr());
		_smpinf._slen = (uint16)wav->size();
		_smpinf._span = pan;
		_smpinf._counter = getRepeat();
		sndDigiStart(&_smpinf, soundType);
	}
}

void Sound::sndDigiStart(SmpInfo *PSmpInfo, Audio::Mixer::SoundType soundType) {
	// Create an audio stream wrapper for sound
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(PSmpInfo->_saddr,
		PSmpInfo->_slen, DisposeAfterUse::NO);
	_audioStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	// Decide which handle to use
	Audio::SoundHandle *handle = nullptr;
	switch (soundType) {
	case Audio::Mixer::kSFXSoundType:
		handle = &_sfxHandle;
		break;
	case Audio::Mixer::kSpeechSoundType:
		handle = &_speechHandle;
		break;
	default:
		error("Wrong sound type passed to sndDigiStart()");
	}

	// Start the new sound
	_vm->_mixer->playStream(soundType, handle,
		Audio::makeLoopingAudioStream(_audioStream, (uint)PSmpInfo->_counter));

	// CGE pan:
	// 8 = Center
	// Less = Left
	// More = Right
	_vm->_mixer->setChannelBalance(*handle, (int8)CLIP(((PSmpInfo->_span - 8) * 16), -127, 127));
}

void Sound::stop() {
	sndDigiStop(_sfxHandle);
	sndDigiStop(_speechHandle);
	_audioStream = nullptr;
}

void Sound::checkSoundHandles() {
	if (!_vm->_mixer->isSoundHandleActive(_speechHandle) && !_vm->_mixer->isSoundHandleActive(_sfxHandle))
		_smpinf._counter = 0;
}

void Sound::sndDigiStop(Audio::SoundHandle &handle) {
	if (_vm->_mixer->isSoundHandleActive(handle))
		_vm->_mixer->stopHandle(handle);
}

Fx::Fx(CGE2Engine *vm, int size) : _current(nullptr), _vm(vm) {
}

Fx::~Fx() {
	clear();
}

void Fx::clear() {
	if (_current)
		delete _current;
	_current = nullptr;
}

Common::String Fx::name(int ref, int sub) {
	const char *fxname = "%.2dfx%.2d.WAV";
	const char *subName = "%.2dfx%.2d?.WAV";
	const char *p = (sub) ? subName : fxname;
	Common::String filename = Common::String::format(p, ref >> 8, ref & 0xFF);
	if (sub)
		filename.setChar('@' + sub, 6);
	return filename;
}

bool Fx::exist(int ref, int sub) {
	return _vm->_resman->exist(name(ref, sub).c_str());
}

DataCk *Fx::load(int ref, int sub) {
	Common::String filename = name(ref, sub);
	EncryptedStream file(_vm, filename.c_str());
	clear();
	return (_current = loadWave(&file));
}

DataCk *Fx::loadWave(EncryptedStream *file) {
	byte *data = (byte *)malloc(file->size());

	if (!data)
		return 0;

	file->read(data, file->size());

	return new DataCk(data, file->size());
}

MusicPlayer::MusicPlayer(CGE2Engine *vm) : _vm(vm) {
	_data = nullptr;
	_isGM = false;

	MidiPlayer::createDriver();

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		// TODO: Load cmf.ins with the instrument table.  It seems that an
		// interface for such an operation is supported for AdLib.  Maybe for
		// this card, setting instruments is necessary.

		_driver->setTimerCallback(this, &timerCallback);
	}
	_dataSize = -1;
}

MusicPlayer::~MusicPlayer() {
	killMidi();
}

void MusicPlayer::killMidi() {
	Audio::MidiPlayer::stop();

	free(_data);
	_data = nullptr;
}

void MusicPlayer::loadMidi(int ref) {
	if (_vm->_midiNotify != nullptr)
		(_vm->*_vm->_midiNotify)();

	// Work out the filename and check the given MIDI file exists
	Common::String filename = Common::String::format("%.2dSG%.2d.MID", ref >> 8, ref & 0xFF);
	if (!_vm->_resman->exist(filename.c_str()))
		return;

	// Stop any currently playing MIDI file
	killMidi();

	// Read in the data for the file
	EncryptedStream mid(_vm, filename.c_str());
	_dataSize = mid.size();
	_data = (byte *)malloc(_dataSize);
	mid.read(_data, _dataSize);

	// Start playing the music
	sndMidiStart();
}

void MusicPlayer::sndMidiStart() {
	_isGM = true;

	MidiParser *parser = MidiParser::createParser_SMF();
	if (parser->loadMusic(_data, _dataSize)) {
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		syncVolume();

		// Al the tracks are supposed to loop
		_isLooping = true;
		_isPlaying = true;
	}
}

void MusicPlayer::send(uint32 b) {
	if (((b & 0xF0) == 0xC0) && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}

	Audio::MidiPlayer::send(b);
}

void MusicPlayer::sendToChannel(byte channel, uint32 b) {
	if (!_channelsTable[channel]) {
		_channelsTable[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channelsTable[channel])
			_channelsTable[channel]->volume(_channelsVolume[channel] * _masterVolume / 255);
	}

	if (_channelsTable[channel])
		_channelsTable[channel]->send(b);
}

} // End of namespace CGE2
