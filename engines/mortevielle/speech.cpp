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
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/endian.h"
#include "common/file.h"
#include "mortevielle/speech.h"
#include "mortevielle/sound.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/var_mor.h"

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

SpeechManager::SpeechManager() {
	_typlec = 0;
	_phonemeNumb = 0;
}

void SpeechManager::spfrac(int wor) {
	_queue[2]._rep = (uint)wor >> 12;
	if ((_typlec == 0) && (_queue[2]._code != 9))
		if (((_queue[2]._code > 4) && (_queue[2]._val != 20) && (_queue[2]._rep != 3) && (_queue[2]._rep != 6) && (_queue[2]._rep != 9)) ||
				((_queue[2]._code < 5) && ((_queue[2]._val != 19) && (_queue[2]._val != 22) && (_queue[2]._rep != 4) && (_queue[2]._rep != 9)))) {
			++_queue[2]._rep;
		}

	_queue[2]._freq = ((uint)wor >> 6) & 7;
	_queue[2]._acc = ((uint)wor >> 9) & 7;
}

void SpeechManager::charg_car(int &currWordNumb) {
	int wor = swap(READ_LE_UINT16(&g_vm->_mem[kAdrWord + currWordNumb]));
	int int_ = wor & 0x3f; // 63

	if ((int_ >= 0) && (int_ <= 13)) {
		_queue[2]._val = int_;
		_queue[2]._code = 5;
	} else if ((int_ >= 14) && (int_ <= 21)) {
		_queue[2]._val = int_;
		_queue[2]._code = 6;
	} else if ((int_ >= 22) && (int_ <= 47)) {
		int_ = int_ - 22;
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


void SpeechManager::entroct(byte o) {
	g_vm->_mem[kAdrTroct * 16 + _ptr_oct] = o;
	++_ptr_oct;
}

void SpeechManager::veracf(byte b) {
	;
}

void SpeechManager::cctable(tablint &t) {
	float tb[257];

	tb[0] = 0;
	for (int k = 0; k <= 255; ++k) {
		tb[k + 1] = g_vm->_addFix + tb[k];
		t[255 - k] = abs((int)tb[k] + 1);
	}
}

void SpeechManager::regenbruit() {
	int i = kOffsetB3 + 8590;
	int j = 0;
	do {
		_cfiphBuffer[j] = READ_LE_UINT16(&g_vm->_mem[kAdrNoise3 + i]);
		i += 2;
		++j;
	} while (i < kOffsetB3 + 8790);
}

/**
 * Load sonmus.mor file
 * @remarks	Originally called 'charge_son'
 */
void SpeechManager::loadMusicSound() {
	Common::File f;

	if (!f.open("sonmus.mor"))
		error("Missing file - sonmus.mor");
	
	f.read(&g_vm->_mem[0x7414 * 16 + 0], 273);

	g_vm->_soundManager.decodeMusic(&g_vm->_mem[0x7414 * 16], &g_vm->_mem[kAdrNoise * 16], 273);
	f.close();
}

/**
 * Load phoneme sound file
 * @remarks	Originally called 'charge_phbruit'
 */
void SpeechManager::loadPhonemeSounds() {
	Common::File f;

	if (!f.open("phbrui.mor"))
		error("Missing file - phbrui.mor");

	for (int i = 1; i <= 3; ++i)
		_cfiphBuffer[i] = f.readSint16LE();

	f.close();
}

/**
 * Speech function - Load Noise file
 * @remarks	Originally called 'charge_bruit'
 */
void SpeechManager::loadNoise() {
	Common::File f;
	int i;

	if (!f.open("bruits"))               //Translation: "noise"
		error("Missing file - bruits");

	f.read(&g_vm->_mem[kAdrNoise * 16 + 0], 250);
	for (i = 0; i <= 19013; ++i)
		g_vm->_mem[kAdrNoise * 16 + 32000 + i] = g_vm->_mem[kAdrNoise5 + i];
	f.read(&g_vm->_mem[kAdrNoise1 * 16 + kOffsetB1], 149);

	f.close();
}

void SpeechManager::trait_car() {
	byte d3;
	int d2, i;

	switch (_queue[1]._code) {
	case 9:
		if (_queue[1]._val != ord('#'))
			for (i = 0; i <= _queue[1]._rep; ++i)
				entroct(_queue[1]._val);
		break;
	case 5:
	case 6:
		if (_queue[1]._code == 6)
			d3 = _tabdph[(_queue[1]._val - 14) << 1];
		else
			d3 = kNullValue;
		if (_queue[0]._code >= 5) {
			veracf(_queue[1]._acc);
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
			veracf(_queue[1]._acc);
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
		veracf(_queue[2]._acc);
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
		veracf(_queue[1]._acc);
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
			veracf(_queue[2]._acc);
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
		veracf(_queue[1]._acc);
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
			veracf(_queue[2]._acc);
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
void SpeechManager::moveQueue() {
	_queue[0] = _queue[1];
	_queue[1] = _queue[2];
	_queue[2]._val = 32;
	_queue[2]._code = 9;
}

/**
 * initialize the queue
 * @remarks	Originally called 'init_chariot'
 */
void SpeechManager::initQueue() {
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
void SpeechManager::handlePhoneme() {
	const int deca[3] = {300, 30, 40};

	int startPos = swap(_cfiphBuffer[_phonemeNumb - 1]) + deca[_typlec];
	int endPos = swap(_cfiphBuffer[_phonemeNumb]) + deca[_typlec];
	int wordCount = endPos - startPos;
	for (int i = (uint)startPos >> 1, currWord = 0; i < (int)((uint)endPos >> 1); i++, currWord += 2)
		WRITE_LE_UINT16(&g_vm->_mem[kAdrWord + currWord], _cfiphBuffer[i]);

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
	entroct(ord('#'));
}

/**
 * Start speech
 * @remarks	Originally called 'parole'
 */
void SpeechManager::startSpeech(int rep, int ht, int typ) {
	int savph[501];
	int tempo;

	if (g_vm->_soundOff)
		return;

	_phonemeNumb = rep;
	int haut = ht;
	_typlec = typ;
	if (_typlec != 0) {
		for (int i = 0; i <= 500; ++i)
			savph[i] = _cfiphBuffer[i];
		tempo = kTempoNoise;
	} else if (haut > 5)
		tempo = kTempoF;
	else
		tempo = kTempoM;
	g_vm->_addFix = (float)((tempo - 8)) / 256;
	cctable(_tbi);
	switch (typ) {
	case 1:
		loadNoise();
		/*if zuul then zzuul(kAdrNoise,0,1095);*/
		regenbruit();
		break;
	case 2:
		loadMusicSound();
		loadPhonemeSounds();
		break;
	default:
		break;
	}
	handlePhoneme();
	g_vm->_soundManager.litph(_tbi, typ, tempo);
	if (_typlec != 0)
		for (int i = 0; i <= 500; ++i) {
			_cfiphBuffer[i] = savph[i];
			_mlec = _typlec;
		}
	g_vm->setPal(g_vm->_numpal);
}

} // End of namespace Mortevielle
