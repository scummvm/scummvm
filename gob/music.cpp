/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 * Original ADL-Player source Copyright (C) 2004 by Dorian Gray
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
 * $Header$
 *
 */

#include "gob/music.h"
#include "gob/gob.h"
#include "gob/game.h"

namespace Gob {

const char *Music::_tracks[][2] = {
	{"avt00.tot",  "mine"},
	{"avt001.tot", "nuit"},
	{"avt002.tot", "campagne"},
	{"avt003.tot", "extsor1"},
	{"avt004.tot", "interieure"},
	{"avt005.tot", "zombie"},
	{"avt006.tot", "zombie"},
	{"avt007.tot", "campagne"},
	{"avt008.tot", "campagne"},
	{"avt009.tot", "extsor1"},
	{"avt010.tot", "extsor1"},
	{"avt011.tot", "interieure"},
	{"avt012.tot", "zombie"},
	{"avt014.tot", "nuit"},
	{"avt015.tot", "interieure"},
	{"avt016.tot", "statue"},
	{"avt017.tot", "zombie"},
	{"avt018.tot", "statue"},
	{"avt019.tot", "mine"},
	{"avt020.tot", "statue"},
	{"avt021.tot", "mine"},
	{"avt022.tot", "zombie"}
};

const char *Music::_tracksToFiles[][2] = {
	{"campagne", "Musmac2.adl"},
	{"extsor1", "Musmac3.adl"},
	{"interieure", "Musmac4.adl"},
	{"mine", "Musmac5.adl"},
	{"nuit", "Musmac6.adl"},
	{"statue", "Musmac2.adl"},
	{"zombie", "Musmac3.adl"}
};

const unsigned char Music::_operators[] = {0, 1, 2, 8, 9, 10, 16, 17, 18};
const unsigned char Music::_volRegNums[] = { 
	3,  4,  5,
	11, 12, 13,
	19, 20, 21
};

Music::Music(GobEngine *vm) : _vm(vm) {
	_data = 0;
	_playPos = 0;
	_dataSize = 0;
	_rate = _vm->_mixer->getOutputRate();
	_opl = makeAdlibOPL(_rate);
	_vm->_mixer->setupPremix(this, Audio::Mixer::kMusicSoundType);
	_first = true;
	_ended = false;
	_playing = false;
	_looping = true;
	_samplesTillPoll = 0;

	setFreqs();
}

Music::~Music(void) {
	if (_data);
		delete _data;
	_vm->_mixer->setupPremix(0);
}

void Music::premixerCall(int16 *buf, uint len) {
	if (!_playing) {
		memset(buf, 0, 2 * len * sizeof(int16));
		return;
	}
	else {
		if (_first) {
			memset(buf, 0, 2 * len * sizeof(int16));
			pollMusic();
			return;
		}
		else {
			uint32 render;
			int16 *data = buf;
			uint datalen = len;
			while (datalen) {
				if (_samplesTillPoll) {
					render = (datalen > _samplesTillPoll) ? (_samplesTillPoll) : (datalen);
					datalen -= render;
					_samplesTillPoll -= render;
					YM3812UpdateOne(_opl, data, render);
					data += render;
				} else {
					pollMusic();
					if (_ended) {
						memset(data, 0, datalen * sizeof(int16));
						datalen = 0;
					}
				}
			}
		}
		if (_ended) {
			_first = true;
			_ended = false;
			_playPos = _data + 3 + (_data[1] + 1) * 0x38;
			_samplesTillPoll = 0;
			if (_looping) {
				reset();
				setVoices();
			}
			else
				_playing = false;
		}
		// Convert mono data to stereo
		for (int i = (len - 1); i >= 0; i--) {
			buf[2 * i] = buf[2 * i + 1] = buf[i];
		}
	}
}

void Music::writeOPL(byte reg, byte val) {
	debug(5, "writeOPL(%02X, %02X)", reg, val);
	OPLWriteReg(_opl, reg, val);
}

void Music::setFreqs(void) {
	byte lin;
	byte col;
	long val = 0;

	// Run through the 11 channels
	for (lin = 0; lin < 11; lin ++) {
		_notes[lin] = 0;
		_notCol[lin] = 0;
		_notLin[lin] = 0;
		_notOn[lin] = false;
	} 
		
	// Run through the 25 lines
	for (lin = 0; lin < 25; lin ++) {
		// Run through the 12 columns
		for (col = 0; col < 12; col ++) {
			if (!col)
				val = (((0x2710L + lin * 0x18) * 0xCB78 / 0x3D090) << 0xE) * 9 / 0x1B503;
			_freqs[lin][col] = (short)((val + 4) >> 3);
			val = val * 0x6A / 0x64;
	//      val = val *  392 / 370;
		} 
	}
}

void Music::reset() {
	// Set frequencies and octave to 0; notes off
	for (int i = 0; i < 9; i++) {
		writeOPL(0xA0 | i, 0);
		writeOPL(0xB0 | i, 0);
		writeOPL(0xE0 | _operators[i]    , 0);
		writeOPL(0xE0 | _operators[i] + 3, 0);
	}

	// Authorize the control of the waveformes
	writeOPL(0x01, 0x20);
}

void Music::setVoices() {
	// Definitions of the 9 instruments
	for (int i = 0; i < 9; i++)
		setVoice(i, i, true);
}

void Music::setVoice(byte voice, byte instr, bool set) {
	unsigned short *strct;
	byte channel;

	// i = 0 :  0  1  2  3  4  5  6  7  8  9 10 11 12 26
	// i = 1 : 13 14 15 16 17 18 19 20 21 22 23 24 25 27
	for (int i = 0; i < 2; i++) {
		strct = (unsigned short*)(_data + 3 + instr * 0x38 + i * 0x1A);
		channel = _operators[voice] + i * 3;
		writeOPL(0xBD, 0x00);
		writeOPL(0x08, 0x00);
		writeOPL(0x40 | channel, ((strct[0] & 3) << 6) | (strct[8] & 0x3F));
		if (!i)
			writeOPL(0xC0 | voice  , ((strct[2] & 7) << 1) | (1 - (strct[12] & 1)));
		writeOPL(0x60 | channel, ((strct[3] & 0xF) << 4) | (strct[6] & 0xF));
		writeOPL(0x80 | channel, ((strct[4] & 0xF) << 4) | (strct[7] & 0xF));
		writeOPL(0x20 | channel, ((strct[9] & 1) << 7) |
			((strct[10] & 1) << 6) | ((strct[5] & 1) << 5) |
			((strct[11] & 1) << 4) |  (strct[1] & 0xF));
		if (!i)
			writeOPL(0xE0 | channel, (strct[26] & 3));
		else
			writeOPL(0xE0 | channel, (strct[14] & 3));
		if (i && set)
			writeOPL(0x40 | channel, 0);
	}
}

void Music::setKey(byte voice, byte note, bool on, bool spec) {
	short freq = 0;
	short octa = 0;

	// Instruction AX
	if (spec) {
		// 0x7F donne 0x16B;
		//     7F
		// <<   7 =  3F80
		// + E000 = 11F80
		// & FFFF =  1F80
		// *   19 = 31380
		// / 2000 =    18 => Ligne 18h, colonne  0 => freq 16B

		// 0x3A donne 0x2AF;
		//     3A
		// <<   7 =  1D00
		// + E000 =  FD00 négatif
		// *   19 = xB500
		// / 2000 =    -2 => Ligne 17h, colonne -1

		//     2E
		// <<   7 =  1700
		// + E000 =  F700 négatif
		// *   19 = x1F00
		// / 2000 =
		short a;
		short lin;
		short col;

		a = (note << 7) + 0xE000; // Volontairement tronqué
		a = (short)((long)a * 25 / 0x2000);
		if (a < 0) {
			col = - ((24 - a) / 25);
			lin = (-a % 25);
			if (lin)
				lin = 25 - lin;
		}
		else {
			col = a / 25;
			lin = a % 25;
		}

		_notCol[voice] = col;
		_notLin[voice] = lin;
		note = _notes[voice];
	}
	// Instructions 0X 9X 8X
	else {
		note -= 12;
		_notOn[voice] = on;
	}

	_notes[voice] = note;
	note += _notCol[voice];
	note = MIN(0x5F, (int)note);
	octa = note / 12;
	freq = _freqs[_notLin[voice]][note - octa * 12];

	writeOPL(0xA0 + voice,  freq & 0xff);
	writeOPL(0xB0 + voice, (freq >> 8) | (octa << 2) | 0x20 * on);

	if (!freq)
		warning("Voice %d, note %02X unknown\n", voice, note);
}

void Music::setVolume(byte voice, byte volume) {
	volume = 0x3F - (volume * 0x7E + 0x7F) / 0xFE;
	writeOPL(0x40 + _volRegNums[voice], volume);
}

void Music::pollMusic(void) {
	unsigned char instr;
	byte channel;
	byte note;
	byte volume;
	uint16 tempo;

	if (_playPos > (_data + _dataSize)) {
		_ended = true;
		return;
	}

	// First tempo, we'll ignore it...
	if (_first) {
		tempo = *(_playPos++);
		// Tempo on 2 bytes
		if (tempo & 0x80)
			tempo = ((tempo & 3) << 8) | *(_playPos++);
	}
	_first = false;

	// Instruction
	instr = *(_playPos++);
	channel = instr & 0x0F;

	switch (instr & 0xF0) {
		// Note on + Volume
		case 0x00:
			note = *(_playPos++);
			_pollNotes[channel] = note;
			setVolume(channel, *(_playPos++));
			setKey(channel, note, true, false);
			break;
		// Note on
		case 0x90:
			note = *(_playPos++);
			_pollNotes[channel] = note;
			setKey(channel, note, true, false);
			break;
		// Last note off
		case 0x80:
			note = _pollNotes[channel];
			setKey(channel, note, false, false);
			break;
		// Frequency on/off
		case 0xA0:
			note = *(_playPos++);
			setKey(channel, note, _notOn[channel], true);
			break;
		// Volume
		case 0xB0:
			volume = *(_playPos++);
			setVolume(channel, volume);
			break;
		// Program change
		case 0xC0:
			setVoice(channel, *(_playPos++), false);
			break;
		// Special
		case 0xF0:
			break;
		default:
			warning("Unknown command in ADL, stopping playback");
			_looping = false;
			_ended = true;
			break;
	}
	// End instruction
	if (instr == 0xFF) {
		_ended = true;
	}

	// Temporization
	tempo = *(_playPos++);
	// End tempo
	if (tempo == 0xFF) {
		_ended = true;
		return;
	}
	// Tempo on 2 bytes
	if (tempo & 0x80)
		tempo = ((tempo & 3) << 8) | *(_playPos++);
	if (!tempo)
		tempo ++;

	_samplesTillPoll = tempo * (_rate / 1000);
}

void Music::startPlay(void) {
	if (!_data)
		return;
	
	_playing = true;
}

void Music::playBgMusic(void) {
	debug(2, "Music::playBgMusic()");
	for (int i = 0; i < ARRAYSIZE(_tracks); i++)
		if (!scumm_stricmp(_vm->_game->_curTotFile, _tracks[i][0])) {
			playTrack(_tracks[i][1]);
			break;
		}
}

void Music::playTrack(const char *trackname) {
	if (_playing) return;
	
	debug(2, "Music::playTrack()");
	unloadMusic();
	for (int i = 0; i < ARRAYSIZE(_tracksToFiles); i++)
		if (!scumm_stricmp(trackname, _tracksToFiles[i][0])) {
			loadMusic(_tracksToFiles[i][1]);
			startPlay();
			break;
		}
}

bool Music::loadMusic(const char *filename) {
	Common::File song;

	song.open(filename);
	if (!song.isOpen())
		return false;

	_dataSize = song.size();
	_data = new byte[_dataSize];
	song.read(_data, _dataSize);
	song.close();

	reset();
	setVoices();
	_playPos = _data + 3 + (_data[1] + 1) * 0x38;
	
	return true;
}

void Music::unloadMusic(void) {
	_playing = false;

	if (_data)
		delete _data;
}

} // End of namespace Gob
