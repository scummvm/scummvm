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

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/wave.h"
#include "common/memstream.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "trecision/sound.h"
#include "trecision/trecision.h"
#include "trecision/defines.h"

namespace Trecision {

SoundManager::SoundManager(TrecisionEngine *vm) : _vm(vm) {
	for (int i = 0; i < NUMSAMPLES; ++i)
		_sfxStream[i] = nullptr;

	_timer = 0;

	for (int i = 0; i < SAMPLEVOICES; ++i) {
		_samplePlaying[i] = 0;
		_sampleVolume[i] = 0;
	}

	_stepChannel = kSoundChannelStep;
	_backChannel = kSoundChannelBack;
	_soundFadeStatus = SFADNONE;

	_soundFadeInVal = 0;
	_soundFadeOutVal = 0;

	if (!_speechFile.open("nlspeech.cd0"))
		warning("SoundManager - nlspeech.cd0 is missing - skipping");
}

SoundManager::~SoundManager() {
	_speechFile.close();
}

void SoundManager::soundTimer() {
	uint32 ctime = g_system->getMillis() / 8;  // only one time out of 8

	if (ctime > _timer)
		_timer = ctime;
	else
		return;

	// only if it's a fading
	if (!_soundFadeStatus)
		return;
	
	if (_soundFadeStatus & SFADOUT) {
		if (!g_system->getMixer()->isSoundHandleActive(_soundHandle[_backChannel])) {
			_soundFadeStatus &= (~SFADOUT);
		} else {
			_soundFadeOutVal -= FADMULT;

			if (_soundFadeOutVal > 0)
				g_system->getMixer()->setChannelVolume(_soundHandle[_backChannel], VOLUME(_soundFadeOutVal / FADMULT));
			else {
				_soundFadeOutVal = 0;
				g_system->getMixer()->setChannelVolume(_soundHandle[_backChannel], VOLUME(_soundFadeOutVal));

				_soundFadeStatus &= (~SFADOUT);
			}
		}
	}
	if (_soundFadeStatus & SFADIN) {
		_soundFadeInVal += FADMULT;

		if (_soundFadeInVal > _gSample[_samplePlaying[_stepChannel]]._volume * FADMULT)
			_soundFadeInVal = _gSample[_samplePlaying[_stepChannel]]._volume * FADMULT;

		g_system->getMixer()->setChannelVolume(_soundHandle[_stepChannel], VOLUME(_soundFadeInVal / FADMULT));

		for (int a = 2; a < SAMPLEVOICES; a++) {
			if (_samplePlaying[a] != 0) {
				_sampleVolume[a] += FADMULT;

				if (_sampleVolume[a] > _gSample[_samplePlaying[a]]._volume * FADMULT)
					_sampleVolume[a] = _gSample[_samplePlaying[a]]._volume * FADMULT;

				g_system->getMixer()->setChannelVolume(_soundHandle[a], VOLUME(_sampleVolume[a] / FADMULT));
			}
		}
	}
}

void SoundManager::stopSoundSystem() {
	g_system->getMixer()->stopAll();
}

void SoundManager::loadAudioWav(int num, const Common::String &fileName) {
	assert(num != 0xFFFF);
	Common::SeekableReadStream *stream = _vm->_dataFile.createReadStreamForMember(fileName);
	const int size = stream->size();
	byte *buf = new byte[size];
	stream->read(buf, size);
	delete stream;
	_sfxStream[num] = Audio::makeWAVStream(new Common::MemoryReadStream(buf, size), DisposeAfterUse::YES);
}

void SoundManager::play(int num) {
	int channel = 2;
	if (g_system->getMixer()->isSoundHandleActive(_soundHandle[channel])) {
		g_system->getMixer()->stopHandle(_soundHandle[channel]);
		_samplePlaying[channel] = 0;
	}

	int volume = VOLUME(_gSample[num]._volume);

	if (_gSample[num]._flag & kSoundFlagSoundOn) {
		volume = 0;
		_sampleVolume[channel] = 0;
	}

	Audio::AudioStream *stream = _sfxStream[num];
	Audio::Mixer::SoundType type = (_gSample[num]._flag & kSoundFlagBgMusic) ? Audio::Mixer::kMusicSoundType : Audio::Mixer::kSFXSoundType;
	if (stream != nullptr && _gSample[num]._flag & kSoundFlagSoundLoop)
		stream = Audio::makeLoopingAudioStream(_sfxStream[num], 0);

	g_system->getMixer()->playStream(type, &_soundHandle[channel], stream, -1, volume, 0, DisposeAfterUse::NO);

	_samplePlaying[channel] = num;
}

void SoundManager::stop(int num) {
	for (int a = 2; a < kSoundChannelSpeech; a++) {
		if (_samplePlaying[a] == num) {
			g_system->getMixer()->stopHandle(_soundHandle[a]);
			_samplePlaying[a] = 0;
		}
	}
}

void SoundManager::stopAll() {
	for (int a = 0; a < SAMPLEVOICES; a++) {
		g_system->getMixer()->stopHandle(_soundHandle[a]);
		_samplePlaying[a] = 0;
	}

	_soundFadeOutVal = SFADNONE;
	_soundFadeStatus = 0;
}

void SoundManager::fadeOut() {
	for (int a = 0; a < SAMPLEVOICES; a++) {	// Turns off all channels except background
		if (a != _backChannel) {
			g_system->getMixer()->stopHandle(_soundHandle[a]);
			_samplePlaying[a] = 0;
		}
	}

	_soundFadeOutVal = g_system->getMixer()->getChannelVolume(_soundHandle[_backChannel]) * FADMULT;
	_soundFadeStatus = SFADOUT;
}

void SoundManager::fadeIn(int num) {
	Audio::AudioStream *stream = _sfxStream[num];
	Audio::Mixer::SoundType type = _gSample[num]._flag & kSoundFlagBgMusic ? Audio::Mixer::kMusicSoundType : Audio::Mixer::kSFXSoundType;
	if (stream != nullptr && _gSample[num]._flag & kSoundFlagSoundLoop)
		stream = Audio::makeLoopingAudioStream(_sfxStream[num], 0);

	g_system->getMixer()->playStream(type, &_soundHandle[_stepChannel], stream, -1, 0, 0, DisposeAfterUse::NO);

	_samplePlaying[_stepChannel] = num;

	_soundFadeInVal = 0;
	_soundFadeStatus |= SFADIN;
}

void SoundManager::waitEndFading() {
	while ((_soundFadeInVal != (_gSample[_samplePlaying[_stepChannel]]._volume * FADMULT)) && (_samplePlaying[_stepChannel] != 0) && (_soundFadeOutVal != 0))
		_vm->checkSystem();
	_soundFadeStatus = SFADNONE;

	g_system->getMixer()->stopHandle(_soundHandle[_backChannel]);

	g_system->getMixer()->setChannelVolume(_soundHandle[_stepChannel], VOLUME(_gSample[_samplePlaying[_stepChannel]]._volume));
	_samplePlaying[_backChannel] = 0;

	for (uint8 a = 2; a < kSoundChannelSpeech; a++) {
		if (_samplePlaying[a] != 0)
			g_system->getMixer()->setChannelVolume(_soundHandle[a], VOLUME(_gSample[_samplePlaying[a]]._volume));
	}

	SWAP(_stepChannel, _backChannel);

	if (_vm->_curRoom == kRoom41D)
		_vm->readExtraObj41D();
}

void SoundManager::soundStep(int midx, int midz, int act, int frame, uint16 *list) {
	bool stepRight = false;
	bool stepLeft = false;

	switch (act) {
	case hWALK:
		if (frame == 3)
			stepLeft = true;
		else if (frame == 8)
			stepRight = true;
		break;

	case hWALKIN:
		if (frame == 3)
			stepLeft = true;
		else if (frame == 9)
			stepRight = true;
		break;

	case hWALKOUT:
		if (frame == 5)
			stepLeft = true;
		else if (frame == 10)
			stepRight = true;
		break;

	case hSTOP0:
	case hSTOP1:
	case hSTOP2:
	case hSTOP3:
	case hSTOP9:
		if (frame >= _vm->_defActionLen[act] - 1)
			stepLeft = true;
	case hSTOP4:
	case hSTOP5:
	case hSTOP6:
	case hSTOP7:
	case hSTOP8:
		if (frame >= _vm->_defActionLen[act] - 1)
			stepRight = true;
		break;
	default:
		break;
	}

	if (!stepRight && !stepLeft)
		return;

	int b;
	for (int a = 0; a < MAXSOUNDSINROOM; a++) {
		b = list[a];

		if (stepRight && (_gSample[b]._flag & kSoundFlagStepRight))
			break;
		if (stepLeft && (_gSample[b]._flag & kSoundFlagStepLeft))
			break;
		if (b == 0)
			return;
	}

	midz = ((int)(_gSample[b]._volume) * 1000) / ABS(midz);

	if (midz > 255)
		midz = 255;

	g_system->getMixer()->stopHandle(_soundHandle[_stepChannel]);
	_sfxStream[b]->rewind();

	int panpos = ((midx - 320) * 127 / 320) / 2;
	Audio::Mixer::SoundType type = (_gSample[b]._flag & kSoundFlagBgMusic) ? Audio::Mixer::kMusicSoundType : Audio::Mixer::kSFXSoundType;

	g_system->getMixer()->playStream(type, &_soundHandle[_stepChannel], _sfxStream[b], -1, VOLUME(midz), panpos, DisposeAfterUse::NO);
}

int32 SoundManager::talkStart(const Common::String &name) {
	if (!_speechFile.isOpen())
		return 0;

	talkStop();

	Common::SeekableReadStream *stream = _speechFile.createReadStreamForMember(name);
	if (!stream)
		return 0;

	Audio::SeekableAudioStream *speechStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle[kSoundChannelSpeech], speechStream);
	_vm->_characterSpeakTime = _vm->readTime();

	return TIME(speechStream->getLength().msecs());
}

void SoundManager::talkStop() {
	g_system->getMixer()->stopHandle(_soundHandle[kSoundChannelSpeech]);
}

void SoundManager::loadRoomSounds() {
	for (uint16 a = 0; a < MAXSOUNDSINROOM; a++) {
		uint16 b = _vm->_room[_vm->_curRoom]._sounds[a];

		if (b == 0)
			break;

		if (_gSample[b]._name.equalsIgnoreCase("RUOTE2C.WAV"))
			break;

		loadAudioWav(b, _gSample[b]._name);

		if (_gSample[b]._flag & kSoundFlagBgMusic)
			fadeIn(b);
		else if (_gSample[b]._flag & kSoundFlagSoundOn)
			play(b);
	}
}

void SoundManager::syncGameStream(Common::Serializer &ser) {
	for (int a = 0; a < MAXSAMPLE; a++) {
		ser.syncAsByte(_gSample[a]._volume);
		ser.syncAsByte(_gSample[a]._flag);
	}
}

void SoundManager::loadSamples(Common::File *file) {
	for (int i = 0; i < MAXSAMPLE; ++i) {
		for (int j = 0; j < 14; j++)
			_gSample[i]._name += file->readByte();
		_gSample[i]._volume = file->readByte();
		_gSample[i]._flag = file->readByte();
		_gSample[i]._panning = file->readSByte();
	}
}

} // End of namespace Trecision
