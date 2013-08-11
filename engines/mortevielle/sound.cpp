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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "mortevielle/mortevielle.h"
#include "mortevielle/sound.h"

#include "audio/decoders/raw.h"
#include "common/scummsys.h"

namespace Mortevielle {

SoundManager::SoundManager(Audio::Mixer *mixer) {
	_mixer = mixer;
	_audioStream = nullptr;
	_ambiantNoiseBuf = nullptr;
	_noiseBuf = nullptr;
}

SoundManager::~SoundManager() {
	if (_audioStream)
		_audioStream->finish();
	free(_ambiantNoiseBuf);
	free(_noiseBuf);
}

/**
 * Decode music data
 */
int SoundManager::decodeMusic(const byte *PSrc, byte *PDest, int size) {
	static const int tab[16] = { -96, -72, -48, -32, -20, -12, -8, -4, 0, 4, 8, 12, 20, 32, 48, 72 };

	uint seed = 128;
	int v;
	int decompSize = 0;
	int skipSize = 0;

	for (int idx1 = 0; idx1 < size; ++idx1) {
		byte srcByte = *PSrc++;
		v = tab[srcByte >> 4];
		seed += v;
		*PDest++ = seed & 0xff;

		v = tab[srcByte & 0xf];
		seed += v;
		*PDest++ = seed & 0xff;

		if (srcByte == 0)
			skipSize += 2;
		else {
			decompSize += skipSize + 2;
			skipSize = 0;
		}
	}
	return decompSize;
}

/**
 * Load sonmus.mor file
 * @remarks	Originally called 'charge_son'
 */
void SoundManager::loadAmbiantSounds() {
	Common::File f;
	if (!f.open("sonmus.mor"))
		error("Missing file - sonmus.mor");

	free(_ambiantNoiseBuf);
	int size = f.size();
	byte *compMusicBuf1 = (byte *)malloc(sizeof(byte) * size);
	_ambiantNoiseBuf = (byte *)malloc(sizeof(byte) * size * 2);
	f.read(compMusicBuf1, size);
	f.close();

	decodeMusic(compMusicBuf1, _ambiantNoiseBuf, size);
	free(compMusicBuf1);
}

/**
 * Speech function - Load Noise files
 * @remarks	Originally called 'charge_bruit' and 'charge_bruit5'
 */
void SoundManager::loadNoise() {
	Common::File f1, f2;

	if (!f1.open("bruits"))               //Translation: "noise"
		error("Missing file - bruits");
	if (!f2.open("bruit5"))
		error("Missing file - bruit5");

	_noiseBuf = (byte *)malloc(sizeof(byte) * (f1.size() + f2.size()));
	assert(f1.size() > 32000);

	f1.read(_noiseBuf, 32000); // 250 * 128
	f2.read(&_noiseBuf[32000], f2.size());
	f1.read(&_noiseBuf[32000 + f2.size()], f1.size() - 32000); // 19072

	f1.close();
	f2.close();
}

void SoundManager::regenbruit() {
	int i = 69876;
	for (int j = 0; j < 100; j++) {
		_vm->_speechManager._cfiphBuffer[j] = READ_BE_UINT16(&_noiseBuf[i]);
		i += 2;
	}
}

void SoundManager::litph(tablint &t, int typ, int tempo) {
	// Skip speech
	if (_vm->_speechManager._soundType == 0)
		return;

	if (!_vm->_speechManager._buildingSentence) {
		if (_mixer->isSoundHandleActive(_soundHandle))
			_mixer->stopHandle(_soundHandle);
		_vm->_speechManager._buildingSentence = true;
	}
	int freq = tempo * 10 * 25.2;
	int i = 0;
	while (i < _vm->_speechManager._ptr_oct) {
		int idx = _vm->_mem[(kAdrTroct * 16) + i];
		i++;
		switch(idx) {
		case 0: {
			int val = _vm->_mem[(kAdrTroct * 16) + i];
			i++;
			if (_vm->_speechManager._soundType == 0)
				warning("TODO: vclas");
			else if (_vm->_speechManager._soundType == 1) {
				debugC(5, kMortevielleSounds, "litph - duson");
				const static int noiseAdr[] = {0,     17224,
											   17224, 33676,
											   33676, 51014,
											   51014, 59396,
											   59396, 61286,
											   61286, 69875};
				if (val > 5) {
					warning("unhandled index %d", val);
				} else {
					if (!_audioStream)
						_audioStream = Audio::makeQueuingAudioStream(freq, false);
					_audioStream->queueBuffer(&_noiseBuf[noiseAdr[val * 2]], noiseAdr[(val * 2) + 1] - noiseAdr[(val * 2)], DisposeAfterUse::NO, Audio::FLAG_UNSIGNED);
				}
			} else { // 2
				debugC(5, kMortevielleSounds, "litph - vadson");
				const static int ambiantNoiseAdr[] = {0,     14020, 
													  14020, 18994,
													  18994, 19630,
													  19630, 22258,
													  22258, 37322,
													  37322, 44472,
													  44472, 52324,
													  52324, 59598,
													  59598, 69748};
				if (val > 8) {
					warning("unhandled index %d", val);
				} else {
					if (!_audioStream)
						_audioStream = Audio::makeQueuingAudioStream(freq, false);
					_audioStream->queueBuffer(&_ambiantNoiseBuf[ambiantNoiseAdr[val * 2]], ambiantNoiseAdr[(val * 2) + 1] - ambiantNoiseAdr[(val * 2)], DisposeAfterUse::NO, Audio::FLAG_UNSIGNED);
				}
			}
			break;
			}
		case 2: {
			int val = _vm->_mem[(kAdrTroct * 16) + i];
			i++;
			int tmpidx = (val * 12) + 268;
			val = _vm->_mem[(kAdrTroct * 16) + i];
			i++;
			warning("TODO: reech %d %d", tmpidx, val);
			}
			break;
		case 4:
			if (_vm->_speechManager._soundType) {
				i += 2;
			} else {
				// Speech
				warning("TODO: Interphoneme: consonne:%d voyelle:%d", _vm->_mem[(kAdrTroct * 16) + i], _vm->_mem[(kAdrTroct * 16) + i + 1]);
				i += 2;
			}
			break;
		case 6:
			warning("TODO: pari2");
			break;
		default:
			if (idx == 62)
				warning("TODO: blab");
			else if (idx == 35) {
				if (i < _vm->_speechManager._ptr_oct)
					warning("unexpected 35");
				i = _vm->_speechManager._ptr_oct;
			} else
				warning("Other code: %d", idx);
			break;
		}
	}
}

void SoundManager::playSong(const byte* buf, uint size, uint loops) {
	int freq = kTempoMusic * 10 * 25.2;
	Audio::SeekableAudioStream *raw = Audio::makeRawStream(buf, size, freq, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(raw, loops);
	Audio::SoundHandle songHandle;
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &songHandle, stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);

	while (_mixer->isSoundHandleActive(songHandle) && !_vm->keyPressed() && !_vm->_mouseClick && !_vm->shouldQuit())
		;
	// In case the handle is still active, stop it.
	_mixer->stopHandle(songHandle);
}

void SoundManager::setParent(MortevielleEngine *vm) {
	_vm = vm;
}
} // End of namespace Mortevielle
