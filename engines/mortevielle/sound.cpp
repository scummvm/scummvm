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
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "mortevielle/mortevielle.h"
#include "mortevielle/sound.h"
#include "mortevielle/dialogs.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#ifdef USE_TTS
#include "common/text-to-speech.h"
#endif

namespace Mortevielle {

	const byte _tnocon[364] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	const byte _intcon[26] = {1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
	const byte _typcon[26] = {0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3};
	const byte _tabdph[16] = {0, 10, 2, 0, 2, 10, 3, 0, 3, 7, 5, 0, 6, 7, 7, 10};
	const byte _tabdbc[18] = {7, 23, 7, 14, 13, 9, 14, 9, 5, 12, 6, 12, 13, 4, 0, 4, 5, 9};

SoundManager::SoundManager(MortevielleEngine *vm, Audio::Mixer *mixer) {
	_vm = vm;
	_mixer = mixer;
	_audioStream = nullptr;
	_ambiantNoiseBuf = nullptr;
	_noiseBuf = nullptr;
#ifdef USE_TTS
	_ttsMan = g_system->getTextToSpeechManager();
	if (_ttsMan) {
		_ttsMan->setLanguage(ConfMan.get("language"));
		_ttsMan->stop();
		_ttsMan->setRate(0);
		_ttsMan->setPitch(0);
		_ttsMan->setVolume(100);
	}
#endif //USE_TTS

	_soundType = 0;
	_phonemeNumb = 0;

	for (int i = 0; i < 3; i++) {
		_queue[i]._val = 0;
		_queue[i]._code = 0;
		_queue[i]._acc = 0;
		_queue[i]._freq = 0;
		_queue[i]._rep = 0;
	}
	_buildingSentence = false;
	_ptr_oct = 0;
	_cfiphBuffer = nullptr;
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
	int decompSize = 0;
	int skipSize = 0;

	for (int idx1 = 0; idx1 < size; ++idx1) {
		byte srcByte = *PSrc++;
		int v = tab[srcByte >> 4];
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
	Common::File f1, f5;

	if (!f5.open("bruit5"))
		error("Missing file - bruit5");

	if (f1.open("bruits")) { //Translation: "noise"
		assert(f1.size() > 32000);
		_noiseBuf = (byte *)malloc(sizeof(byte) * (f1.size() + f5.size()));

		f1.read(_noiseBuf, 32000); // 250 * 128
		f5.read(&_noiseBuf[32000], f5.size());
		f1.read(&_noiseBuf[32000 + f5.size()], f1.size() - 32000); // 19072

		f1.close();
	} else {
		Common::File f2, f3, f4;
		if (!f1.open("bruit1") || !f2.open("bruit2") || !f3.open("bruit3") || !f4.open("bruit4"))
			error("Missing file - bruits");

		assert(f4.size() == 32000);
		_noiseBuf = (byte *)malloc(sizeof(byte) * (f1.size() + f2.size() + f3.size() + f4.size() + f5.size()));

		f4.read(_noiseBuf, f4.size());
		int pos = f4.size();
		f5.read(&_noiseBuf[pos], f5.size());
		pos += f5.size();
		f1.read(&_noiseBuf[pos], f1.size());
		pos += f1.size();
		f2.read(&_noiseBuf[pos], f2.size());
		pos += f2.size();
		f3.read(&_noiseBuf[pos], f3.size());

		f1.close();
		f2.close();
		f3.close();
		f4.close();
	}
	f5.close();
}

void SoundManager::regenbruit() {
	int i = 69876;
	for (int j = 0; j < 100; j++) {
		_cfiphBuffer[j] = READ_BE_UINT16(&_noiseBuf[i]);
		i += 2;
	}
}

void SoundManager::litph(tablint &t, int typ, int tempo) {
	if (!_buildingSentence) {
		if (_mixer->isSoundHandleActive(_soundHandle))
			_mixer->stopHandle(_soundHandle);
#ifdef USE_TTS
		if (_ttsMan) {
			if (_ttsMan->isSpeaking())
				_ttsMan->stop();
		}
#endif // USE_TTS
		_buildingSentence = true;
	}
	int freq = tempo * 252; // 25.2 * 10
	int i = 0;
	while (i < _ptr_oct) {
		int idx = _troctBuf[i];
		i++;
		switch(idx) {
		case 0: {
			int val = _troctBuf[i];
			i++;
			if (_soundType == 1) {
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
			i++;
			break;
			}
		case 2: {
			int val = _troctBuf[i];
			i++;
			int tmpidx = (val * 12) + 268;
			val = _troctBuf[i];
			i++;
			warning("TODO: reech %d %d", tmpidx, val);
			}
			break;
		case 4:
			if (_soundType) {
				i += 2;
			}
			break;
		case 6:
			warning("TODO: pari2");
			i += 2;
			break;
		default:
			static byte emptyBuf[19] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			if (idx == 62)
				warning("TODO: blab");
			else if (idx == 32) {
				if (!_audioStream)
					_audioStream = Audio::makeQueuingAudioStream(freq, false);
				_audioStream->queueBuffer(emptyBuf, 19, DisposeAfterUse::NO, Audio::FLAG_UNSIGNED);
			} else if (idx == 35) {
				if (i < _ptr_oct)
					warning("unexpected 35 - stop the buffering");
				i = _ptr_oct;
			} else if (idx == 46) {
				if (!_audioStream)
					_audioStream = Audio::makeQueuingAudioStream(freq, false);
				for (int j = 0; j < 10; j++)
					_audioStream->queueBuffer(emptyBuf, 19, DisposeAfterUse::NO, Audio::FLAG_UNSIGNED);
			} else {
				warning("Other code: %d - %d %d", idx, _troctBuf[i], _troctBuf[i + 1]);
			}
			break;
		}
	}
}

void SoundManager::playSong(const byte* buf, uint size, uint loops) {
	int freq = kTempoMusic * 252; // 25.2 * 10
	Audio::SeekableAudioStream *raw = Audio::makeRawStream(buf, size, freq, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(raw, loops);
	Audio::SoundHandle songHandle;
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &songHandle, stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);

	while (_mixer->isSoundHandleActive(songHandle) && !_vm->keyPressed() && !_vm->_mouseClick && !_vm->shouldQuit())
		;
	// In case the handle is still active, stop it.
	_mixer->stopHandle(songHandle);
}

void SoundManager::spfrac(int wor) {
	_queue[2]._rep = (uint)wor >> 12;
	_queue[2]._freq = ((uint)wor >> 6) & 7;
	_queue[2]._acc = ((uint)wor >> 9) & 7;
}

void SoundManager::charg_car(int &currWordNumb) {
	assert(currWordNumb < 1712);
	int wor = READ_BE_UINT16(&_wordBuf[currWordNumb]);
	int int_ = wor & 0x3f; // 63

	if ((int_ >= 0) && (int_ <= 13)) {
		_queue[2]._val = int_;
		_queue[2]._code = 5;
	} else if ((int_ >= 14) && (int_ <= 21)) {
		_queue[2]._val = int_;
		_queue[2]._code = 6;
	} else if ((int_ >= 22) && (int_ <= 47)) {
		int_ -= 22;
		_queue[2]._val = int_;
		_queue[2]._code = _typcon[int_];
	} else if ((int_ >= 48) && (int_ <= 56)) {
		_queue[2]._val = int_ - 22;
		_queue[2]._code = 4;
	} else {
		switch (int_) {
		case 60:
			_queue[2]._val = 32;  /*  " "  */
			_queue[2]._code = 9;
			break;
		case 61:
			_queue[2]._val = 46;  /*  "."  */
			_queue[2]._code = 9;
			break;
		case 62:
			_queue[2]._val = 35;  /*  "#"  */
			_queue[2]._code = 9;
		default:
			break;
		}
	}

	spfrac(wor);
	currWordNumb += 2;
}


void SoundManager::entroct(byte o) {
	assert(_ptr_oct < 10576);
	_troctBuf[_ptr_oct] = o;
	++_ptr_oct;
}

void SoundManager::cctable(tablint &t) {
	float tb[257];

	tb[0] = 0;
	for (int k = 0; k <= 255; ++k) {
		tb[k + 1] = _vm->_addFix + tb[k];
		t[255 - k] = abs((int)tb[k] + 1);
	}
}

/**
 * Load phoneme sound file
 * @remarks	Originally called 'charge_phbruit'
 */
void SoundManager::loadPhonemeSounds() {
	Common::File f;

	if (!f.open("phbrui.mor"))
		error("Missing file - phbrui.mor");

	for (int i = 1; i <= f.size() / 2; ++i)
		_cfiphBuffer[i] = f.readUint16BE();

	f.close();
}

void SoundManager::trait_car() {
	byte d3;
	int d2, i;

	switch (_queue[1]._code) {
	case 9:
		if (_queue[1]._val != (int)'#') {
			for (i = 0; i <= _queue[1]._rep; ++i)
				entroct(_queue[1]._val);
		}
		break;
	case 5:
	case 6:
		if (_queue[1]._code == 6)
			d3 = _tabdph[(_queue[1]._val - 14) << 1];
		else
			d3 = kNullValue;
		if (_queue[0]._code >= 5) {
			if (_queue[0]._code == 9) {
				entroct(4);
				if (d3 == kNullValue)
					entroct(_queue[1]._val);
				else
					entroct(d3);
				entroct(22);
			}
		}

		switch (_queue[1]._rep) {
		case 0:
			entroct(0);
			entroct(_queue[1]._val);
			if (d3 == kNullValue)
				if (_queue[2]._code == 9)
					entroct(2);
				else
					entroct(4);
			else if (_queue[2]._code == 9)
				entroct(0);
			else
				entroct(1);
			break;
		case 4:
		case 5:
		case 6:
			if (_queue[1]._rep != 4) {
				i = _queue[1]._rep - 5;
				do {
					--i;
					entroct(0);
					if (d3 == kNullValue)
						entroct(_queue[1]._val);
					else
						entroct(d3);
					entroct(3);
				} while (i >= 0);
			}
			if (d3 == kNullValue) {
				entroct(4);
				entroct(_queue[1]._val);
				entroct(0);
			} else {
				entroct(0);
				entroct(_queue[1]._val);
				entroct(3);
			}

			break;
		case 7:
		case 8:
		case 9:
			if (_queue[1]._rep != 7) {
				i = _queue[1]._rep - 8;
				do {
					--i;
					entroct(0);
					if (d3 == kNullValue)
						entroct(_queue[1]._val);
					else
						entroct(d3);
					entroct(3);
				} while (i >= 0);
			}
			if (d3 == kNullValue) {
				entroct(0);
				entroct(_queue[1]._val);
				entroct(2);
			} else {
				entroct(0);
				entroct(_queue[1]._val);
				entroct(0);
			}
			break;
		case 1:
		case 2:
		case 3:
			if (_queue[1]._rep != 1) {
				i = _queue[1]._rep - 2;
				do {
					--i;
					entroct(0);
					if (d3 == kNullValue)
						entroct(_queue[1]._val);
					else
						entroct(d3);
					entroct(3);
				} while (i >= 0);
			}
			entroct(0);
			entroct(_queue[1]._val);
			if (_queue[2]._code == 9)
				entroct(0);
			else
				entroct(1);

			break;
		default:
			break;
		}     //  switch  c2.rep
		break;

	case 2:
	case 3:
		d3 = _queue[1]._code + 5; //  7 ou 8  => Corresponding vowel
		if (_queue[0]._code > 4) {
			if (_queue[0]._code == 9) {
				entroct(4);
				entroct(d3);
				entroct(22);
			}
		}
		i = _queue[1]._rep;
		assert(i >= 0);
		if (i != 0) {
			do {
				--i;
				entroct(0);
				entroct(d3);
				entroct(3);
			} while (i > 0);
		}
		if (_queue[2]._code == 6) {
			entroct(4);
			entroct(_tabdph[(_queue[2]._val - 14) << 1]);
			entroct(_queue[1]._val);
		} else {
			entroct(4);
			if (_queue[2]._val == 4)
				entroct(3);
			else
				entroct(_queue[2]._val);
			entroct(_queue[1]._val);
		}
		break;
	case 0:
	case 1:
		switch (_queue[2]._code) {
		case 2:
			d2 = 7;
			break;
		case 3:
			d2 = 8;
			break;
		case 6:
			d2 = _tabdph[(_queue[2]._val - 14) << 1];
			break;
		case 5:
			d2 = _queue[2]._val;
			break;
		default:
			d2 = 10;
			break;
		}       //  switch  c3._code
		d2 = (d2 * 26) + _queue[1]._val;
		if (_tnocon[d2] == 0)
			d3 = 2;
		else
			d3 = 6;
		if (_queue[1]._rep >= 5) {
			_queue[1]._rep -= 5;
			d3 = 8 - d3;       // Swap 2 and 6
		}
		if (_queue[1]._code == 0) {
			i = _queue[1]._rep;
			if (i != 0) {
				do {
					--i;
					entroct(d3);
					entroct(_queue[1]._val);
					entroct(3);
				} while (i > 0);
			}
			entroct(d3);
			entroct(_queue[1]._val);
			entroct(4);
		} else {
			entroct(d3);
			entroct(_queue[1]._val);
			entroct(3);
			i = _queue[1]._rep;
			if (i != 0) {
				do {
					--i;
					entroct(d3);
					entroct(_queue[1]._val);
					entroct(4);
				} while (i > 0);
			}
		}
		if (_queue[2]._code == 9) {
			entroct(d3);
			entroct(_queue[1]._val);
			entroct(5);
		} else if ((_queue[2]._code != 0) && (_queue[2]._code != 1) && (_queue[2]._code != 4)) {
			switch (_queue[2]._code) {
			case 3:
				d2 = 8;
				break;
			case 6:
				d2 = _tabdph[(_queue[2]._val - 14) << 1];
				break;
			case 5:
				d2 = _queue[2]._val;
				break;
			default:
				d2 = 7;
				break;
			}     //  switch c3._code
			if (d2 == 4)
				d2 = 3;

			if (_intcon[_queue[1]._val] != 0)
				++_queue[1]._val;

			if ((_queue[1]._val == 17) || (_queue[1]._val == 18))
				_queue[1]._val = 16;

			entroct(4);
			entroct(d2);
			entroct(_queue[1]._val);
		}

		break;
	case 4:
		i = _queue[1]._rep;
		if (i != 0) {
			do {
				--i;
				entroct(2);
				entroct(_queue[1]._val);
				entroct(3);
			} while (i > 0);
		}
		entroct(2);
		entroct(_queue[1]._val);
		entroct(4);
		if (_queue[2]._code == 9) {
			entroct(2);
			entroct(_queue[1]._val);
			entroct(5);
		} else if ((_queue[2]._code != 0) && (_queue[2]._code != 1) && (_queue[2]._code != 4)) {
			switch (_queue[2]._code) {
			case 3:
				d2 = 8;
				break;
			case 6:
				d2 = _tabdph[(_queue[2]._val - 14) << 1];
				break;
			case 5:
				d2 = _queue[2]._val;
				break;
			default:
				d2 = 7;
				break;
			}     //  switch c3._code

			if (d2 == 4)
				d2 = 3;

			if (_intcon[_queue[1]._val] != 0)
				++_queue[1]._val;

			entroct(4);
			entroct(d2);
			entroct(_tabdbc[((_queue[1]._val - 26) << 1) + 1]);
		}

		break;
	default:
		break;
	}     // switch c2.code
}

/**
 * Make the queue evolve by 1 value
 * @remarks	Originally called 'rot_chariot'
 */
void SoundManager::moveQueue() {
	_queue[0] = _queue[1];
	_queue[1] = _queue[2];
	_queue[2]._val = 32;
	_queue[2]._code = 9;
}

/**
 * initialize the queue
 * @remarks	Originally called 'init_chariot'
 */
void SoundManager::initQueue() {
	_queue[2]._rep = 0;
	_queue[2]._freq = 0;
	_queue[2]._acc = 0;
	moveQueue();
	moveQueue();
}

/**
 * Handle a phoneme
 * @remarks	Originally called 'trait_ph'
 */
void SoundManager::handlePhoneme() {
	const uint16 deca[3] = {300, 30, 40};

	uint16 startPos = _cfiphBuffer[_phonemeNumb - 1] + deca[_soundType];
	uint16 endPos = _cfiphBuffer[_phonemeNumb] + deca[_soundType];
	int wordCount = endPos - startPos;

	startPos /= 2;
	endPos /= 2;
	assert((endPos - startPos) < 1711);
	for (int i = startPos, currWord = 0; i < endPos; i++, currWord += 2)
		WRITE_BE_UINT16(&_wordBuf[currWord], _cfiphBuffer[i]);

	_ptr_oct = 0;
	int currWord = 0;
	initQueue();

	do {
		moveQueue();
		charg_car(currWord);
		trait_car();
	} while (currWord < wordCount);

	moveQueue();
	trait_car();
	entroct((int)'#');

#ifdef DEBUG
	warning("---");
	for (int i = 0; i < _ptr_oct; ) {
		if ((_troctBuf[i] == 32) || (_troctBuf[i] == 35) || (_troctBuf[i] == 46)) {
			warning("%d", _troctBuf[i]);
			i++;
		} else {
			warning("%d %d %d", _troctBuf[i], _troctBuf[i + 1], _troctBuf[i + 1]);
			i += 3;
		}
	}
	warning("---");
#endif
}

/**
 * Start speech
 * @remarks	Originally called 'parole'
 */
void SoundManager::startSpeech(int rep, int character, int typ) {
	if (_vm->_soundOff)
		return;

	_soundType = typ;

	if (typ == 0) {
		// Speech
#ifdef USE_TTS
		const int haut[9] = { 0, 0, 1, -3, 6, -2, 2, 7, -1 };
		const int voiceIndices[9] = { 0, 1, 2, 3, 0, 4, 5, 1, 6 };
		if (!_ttsMan)
			return;
		Common::Array<int> voices;
		int pitch = haut[character];
		bool male;
		if (haut[character] > 5) {
			voices = _ttsMan->getVoiceIndicesByGender(Common::TTSVoice::FEMALE);
			male = false;
			pitch -= 6;
		} else {
			voices = _ttsMan->getVoiceIndicesByGender(Common::TTSVoice::MALE);
			male = true;
		}
		pitch *= 5;
		// If there is no voice available for the given gender, just set it to the 0th
		// voice
		if (voices.empty())
			_ttsMan->setVoice(0);
		else {
			int voiceIndex = voiceIndices[character] % voices.size();
			_ttsMan->setVoice(voices[voiceIndex]);
		}
		// If the selected voice is a different gender, than we want, just try to
		// set the pitch so it may sound a little bit closer to the gender we want
		if (!((_ttsMan->getVoice().getGender() == Common::TTSVoice::MALE) == male)) {
			if (male)
				pitch -= 50;
			else
				pitch += 50;
		}

		_ttsMan->setPitch(pitch);
		_ttsMan->say(_vm->getString(rep + kDialogStringIndex), "CP850");
#endif // USE_TTS
		return;
	}
	uint16 savph[501];
	int tempo;

	_phonemeNumb = rep;
	for (int i = 0; i <= 500; ++i)
		savph[i] = _cfiphBuffer[i];
	tempo = kTempoNoise;
	_vm->_addFix = (float)((tempo - 8)) / 256;
	cctable(_tbi);
	switch (typ) {
	case 1:
		regenbruit();
		break;
	case 2:
		loadPhonemeSounds();
		break;
	default:
		break;
	}
	handlePhoneme();
	litph(_tbi, typ, tempo);

	_buildingSentence = false;
	_audioStream->finish();
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream);
	_audioStream = nullptr;

	for (int i = 0; i <= 500; ++i)
		_cfiphBuffer[i] = savph[i];
	_vm->setPal(_vm->_numpal);
}

void SoundManager::waitSpeech() {
	if (_soundType == 0) {
#ifdef USE_TTS
		if (!_ttsMan)
			return;
		while (_ttsMan->isSpeaking() && !_vm->keyPressed() && !_vm->_mouseClick && !_vm->shouldQuit())
			;
		// In case the TTS is still speaking, stop it.
		_ttsMan->stop();

#endif // USE_TTS
	} else {
		while (_mixer->isSoundHandleActive(_soundHandle) && !_vm->keyPressed() && !_vm->_mouseClick && !_vm->shouldQuit())
			;
		// In case the handle is still active, stop it.
		_mixer->stopHandle(_soundHandle);
	}
	if (!_vm->keyPressed() && !_vm->_mouseClick && !_vm->shouldQuit())
		g_system->delayMillis(600);
}
} // End of namespace Mortevielle
