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
 * $URL$
 * $Id$
 *
 */

#include "common/file.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/sound/adlib.h"

namespace Gob {

const unsigned char AdLib::_operators[] = {0, 1, 2, 8, 9, 10, 16, 17, 18};
const unsigned char AdLib::_volRegNums[] = {
	3,  4,  5,
	11, 12, 13,
	19, 20, 21
};

AdLib::AdLib(Audio::Mixer &mixer) : _mixer(&mixer) {
	init();
}

AdLib::~AdLib() {
	Common::StackLock slock(_mutex);

	_mixer->stopHandle(_handle);
	OPLDestroy(_opl);
	if (_data && _freeData)
		delete[] _data;
}

void AdLib::init() {
	_index = -1;
	_data = 0;
	_playPos = 0;
	_dataSize = 0;

	_rate = _mixer->getOutputRate();

	_opl = makeAdlibOPL(_rate);

	_first = true;
	_ended = false;
	_playing = false;

	_freeData = false;

	_repCount = -1;
	_samplesTillPoll = 0;

	for (int i = 0; i < 16; i ++)
		_pollNotes[i] = 0;
	setFreqs();

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_handle,
			this, -1, 255, 0, false, true);
}

int AdLib::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock slock(_mutex);
	int samples;
	int render;

	if (!_playing || (numSamples < 0)) {
		memset(buffer, 0, numSamples * sizeof(int16));
		return numSamples;
	}
	if (_first) {
		memset(buffer, 0, numSamples * sizeof(int16));
		pollMusic();
		return numSamples;
	}

	samples = numSamples;
	while (samples && _playing) {
		if (_samplesTillPoll) {
			render = (samples > _samplesTillPoll) ?  (_samplesTillPoll) : (samples);
			samples -= render;
			_samplesTillPoll -= render;
			YM3812UpdateOne(_opl, buffer, render);
			buffer += render;
		} else {
			pollMusic();
			if (_ended) {
				memset(buffer, 0, samples * sizeof(int16));
				samples = 0;
			}
		}
	}

	if (_ended) {
		_first = true;
		_ended = false;

		rewind();

		_samplesTillPoll = 0;
		if (_repCount == -1) {
			reset();
			setVoices();
		} else if (_repCount > 0) {
			_repCount--;
			reset();
			setVoices();
		}
		else
			_playing = false;
	}
	return numSamples;
}

void AdLib::writeOPL(byte reg, byte val) {
	debugC(6, kDebugSound, "writeOPL(%02X, %02X)", reg, val);
	OPLWriteReg(_opl, reg, val);
}

void AdLib::setFreqs() {
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
				val = (((0x2710L + lin * 0x18) * 0xCB78 / 0x3D090) << 0xE) *
					9 / 0x1B503;
			_freqs[lin][col] = (short)((val + 4) >> 3);
			val = val * 0x6A / 0x64;
		}
	}
}

void AdLib::reset() {
	_first = true;
	OPLResetChip(_opl);
	_samplesTillPoll = 0;

	setFreqs();
	// Set frequencies and octave to 0; notes off
	for (int i = 0; i < 9; i++) {
		writeOPL(0xA0 | i, 0);
		writeOPL(0xB0 | i, 0);
		writeOPL(0xE0 | _operators[i]     , 0);
		writeOPL(0xE0 |(_operators[i] + 3), 0);
	}

	// Authorize the control of the waveformes
	writeOPL(0x01, 0x20);
}

void AdLib::setKey(byte voice, byte note, bool on, bool spec) {
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
		// + E000 =  FD00 negatif
		// *   19 = xB500
		// / 2000 =    -2 => Ligne 17h, colonne -1

		//     2E
		// <<   7 =  1700
		// + E000 =  F700 negatif
		// *   19 = x1F00
		// / 2000 =
		short a;
		short lin;
		short col;

		a = (note << 7) + 0xE000; // Volontairement tronque
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
	note = MIN((byte) 0x5F, note);
	octa = note / 12;
	freq = _freqs[_notLin[voice]][note - octa * 12];

	writeOPL(0xA0 + voice,  freq & 0xFF);
	writeOPL(0xB0 + voice, (freq >> 8) | (octa << 2) | 0x20 * on);

	if (!freq)
		warning("AdLib: Voice %d, note %02X unknown", voice, note);
}

void AdLib::setVolume(byte voice, byte volume) {
	volume = 0x3F - (volume * 0x7E + 0x7F) / 0xFE;
	writeOPL(0x40 + _volRegNums[voice], volume);
}

void AdLib::pollMusic() {
	if ((_playPos > (_data + _dataSize)) && (_dataSize != 0xFFFFFFFF)) {
		_ended = true;
		return;
	}

	interpret();
}

void AdLib::unload() {
	_playing = false;
	_index = -1;

	if (_data && _freeData)
		delete[] _data;

	_freeData = false;
}

bool AdLib::isPlaying() const {
	return _playing;
}

bool AdLib::getRepeating() const {
	return _repCount != 0;
}

void AdLib::setRepeating(int32 repCount) {
	_repCount = repCount;
}

int AdLib::getIndex() const {
	return _index;
}

void AdLib::startPlay() {
	if (_data) _playing = true;
}

void AdLib::stopPlay() {
	Common::StackLock slock(_mutex);
	_playing = false;
}

ADLPlayer::ADLPlayer(Audio::Mixer &mixer) : AdLib(mixer) {
}

ADLPlayer::~ADLPlayer() {
}

bool ADLPlayer::load(const char *fileName) {
	Common::File song;

	unload();
	song.open(fileName);
	if (!song.isOpen())
		return false;

	_freeData = true;
	_dataSize = song.size();
	_data = new byte[_dataSize];
	song.read(_data, _dataSize);
	song.close();

	reset();
	setVoices();
	_playPos = _data + 3 + (_data[1] + 1) * 0x38;

	return true;
}

bool ADLPlayer::load(byte *data, uint32 size, int index) {
	unload();
	_repCount = 0;

	_dataSize = size;
	_data = data;
	_index = index;

	reset();
	setVoices();
	_playPos = _data + 3 + (_data[1] + 1) * 0x38;

	return true;
}

void ADLPlayer::unload() {
	AdLib::unload();
}

void ADLPlayer::interpret() {
	unsigned char instr;
	byte channel;
	byte note;
	byte volume;
	uint16 tempo;

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
			switch (instr & 0x0F) {
			case 0xF: // End instruction
				_ended = true;
				_samplesTillPoll = 0;
				return;
			default:
				warning("ADLPlayer: Unknown special command %X, stopping playback",
						instr & 0x0F);
				_repCount = 0;
				_ended = true;
				break;
			}
			break;
		default:
			warning("ADLPlayer: Unknown command %X, stopping playback",
					instr & 0xF0);
			_repCount = 0;
			_ended = true;
			break;
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

void ADLPlayer::reset() {
	AdLib::reset();
}

void ADLPlayer::rewind() {
	_playPos = _data + 3 + (_data[1] + 1) * 0x38;
}

void ADLPlayer::setVoices() {
	// Definitions of the 9 instruments
	for (int i = 0; i < 9; i++)
		setVoice(i, i, true);
}

void ADLPlayer::setVoice(byte voice, byte instr, bool set) {
	uint16 strct[27];
	byte channel;
	byte *dataPtr;

	// i = 0 :  0  1  2  3  4  5  6  7  8  9 10 11 12 26
	// i = 1 : 13 14 15 16 17 18 19 20 21 22 23 24 25 27
	for (int i = 0; i < 2; i++) {
		dataPtr = _data + 3 + instr * 0x38 + i * 0x1A;
		for (int j = 0; j < 27; j++) {
			strct[j] = READ_LE_UINT16(dataPtr);
			dataPtr += 2;
		}
		channel = _operators[voice] + i * 3;
		writeOPL(0xBD, 0x00);
		writeOPL(0x08, 0x00);
		writeOPL(0x40 | channel, ((strct[0] & 3) << 6) | (strct[8] & 0x3F));
		if (!i)
			writeOPL(0xC0 | voice,
					((strct[2] & 7) << 1) | (1 - (strct[12] & 1)));
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


MDYPlayer::MDYPlayer(Audio::Mixer &mixer) : AdLib(mixer) {
	init();
}

MDYPlayer::~MDYPlayer() {
}

void MDYPlayer::init() {
	_soundMode = 0;

	_timbres = 0;
	_tbrCount = 0;
	_tbrStart = 0;
	_timbresSize = 0;
}

bool MDYPlayer::loadMDY(Common::SeekableReadStream &stream) {
	unloadMDY();

	_freeData = true;

	byte mdyHeader[70];
	stream.read(mdyHeader, 70);

	_tickBeat = mdyHeader[36];
	_beatMeasure = mdyHeader[37];
	_totalTick = mdyHeader[38] + (mdyHeader[39] << 8) + (mdyHeader[40] << 16) + (mdyHeader[41] << 24);
	_dataSize = mdyHeader[42] + (mdyHeader[43] << 8) + (mdyHeader[44] << 16) + (mdyHeader[45] << 24);
	_nrCommand = mdyHeader[46] + (mdyHeader[47] << 8) + (mdyHeader[48] << 16) + (mdyHeader[49] << 24);
// _soundMode is either 0 (melodic) or 1 (percussive)
	_soundMode = mdyHeader[58];
	_pitchBendRangeStep = 25*mdyHeader[59];
	_basicTempo = mdyHeader[60] + (mdyHeader[61] << 8);

	if (_pitchBendRangeStep < 25)
		_pitchBendRangeStep = 25;
	else if (_pitchBendRangeStep > 300)
		_pitchBendRangeStep = 300;

	_data = new byte[_dataSize];
	stream.read(_data, _dataSize);

	reset();
	_playPos = _data;

	return true;
}

bool MDYPlayer::loadMDY(const char *fileName) {
	Common::File song;

	song.open(fileName);
	if (!song.isOpen())
		return false;

	bool loaded = loadMDY(song);

	song.close();

	return loaded;
}

bool MDYPlayer::loadTBR(Common::SeekableReadStream &stream) {
	unloadTBR();

	_timbresSize = stream.size();

	_timbres = new byte[_timbresSize];
	stream.read(_timbres, _timbresSize);

	reset();
	setVoices();

	return true;
}

bool MDYPlayer::loadTBR(const char *fileName) {
	Common::File timbres;

	timbres.open(fileName);
	if (!timbres.isOpen())
		return false;

	bool loaded = loadTBR(timbres);

	timbres.close();

	return loaded;
}

void MDYPlayer::unload() {
	unloadTBR();
	unloadMDY();
}

void MDYPlayer::unloadMDY() {
	AdLib::unload();
}

void MDYPlayer::unloadTBR() {
	delete[] _timbres;

	_timbres = 0;
	_timbresSize = 0;
}

void MDYPlayer::interpret() {
	unsigned char instr;
	byte channel;
	byte note;
	byte volume;
	uint8 tempoMult, tempoFrac;
	uint8 ctrlByte1, ctrlByte2;
	uint8 timbre;

	if (_first) {
		for (int i = 0; i < 11; i ++)
			setVolume(i, 0);

//	TODO : Set pitch range

		_tempo = _basicTempo;
		_wait = *(_playPos++);
		_first = false;
	}
	do {
		instr = *_playPos;
//			printf("instr 0x%X\n", instr);
		switch(instr) {
		case 0xF8:
			_wait = *(_playPos++);
			break;
		case 0xFC:
			_ended = true;
			_samplesTillPoll = 0;
			return;
		case 0xF0:
			_playPos++;
			ctrlByte1 = *(_playPos++);
			ctrlByte2 = *(_playPos++);
			if (ctrlByte1 != 0x7F || ctrlByte2 != 0) {
				_playPos -= 2;
				while (*(_playPos++) != 0xF7)
					;
			} else {
				tempoMult = *(_playPos++);
				tempoFrac = *(_playPos++);
				_tempo = _basicTempo * tempoMult + (unsigned)(((long)_basicTempo * tempoFrac) >> 7);
				_playPos++;
			}
			_wait = *(_playPos++);
			break;
		default:
			if (instr >= 0x80) {
				_playPos++;
			}
			channel = (int)(instr & 0x0f);

			switch(instr & 0xf0) {
			case 0x90:
				note = *(_playPos++);
				volume = *(_playPos++);
				_pollNotes[channel] = note;
				setVolume(channel, volume);
				setKey(channel, note, true, false);
				break;
			case 0x80:
				_playPos += 2;
				note = _pollNotes[channel];
				setKey(channel, note, false, false);
				break;
			case 0xA0:
				setVolume(channel, *(_playPos++));
				break;
			case 0xC0:
				timbre = *(_playPos++);
				setVoice(channel, timbre, false);
				break;
			case 0xE0:
				warning("MDYPlayer: Pitch bend not yet implemented");

				note = *(_playPos)++;
				note += (unsigned)(*(_playPos++)) << 7;

				setKey(channel, note, _notOn[channel], true);

				break;
			case 0xB0:
				_playPos += 2;
				break;
			case 0xD0:
				_playPos++;
				break;
			default:
				warning("MDYPlayer: Bad MIDI instr byte: 0%X", instr);
				while ((*_playPos) < 0x80)
					_playPos++;
				if (*_playPos != 0xF8)
					_playPos--;
				break;
			} //switch instr & 0xF0
			_wait = *(_playPos++);
			break;
		} //switch instr
	} while (_wait == 0);

	if (_wait == 0xF8) {
		_wait = 0xF0;
		if (*_playPos != 0xF8)
			_wait += *(_playPos++);
	}
//		_playPos++;
	_samplesTillPoll = _wait * (_rate / 1000);
}

void MDYPlayer::reset() {
	AdLib::reset();

// _soundMode 1 : Percussive mode.
	if (_soundMode == 1) {
		writeOPL(0xA6, 0);
		writeOPL(0xB6, 0);
		writeOPL(0xA7, 0);
		writeOPL(0xB7, 0);
		writeOPL(0xA8, 0);
		writeOPL(0xB8, 0);

// TODO set the correct frequency for the last 4 percussive voices
	}
}

void MDYPlayer::rewind() {
	_playPos = _data;
}

void MDYPlayer::setVoices() {
	byte *timbrePtr;

	timbrePtr = _timbres;
	debugC(6, kDebugSound, "TBR version: %X.%X", timbrePtr[0], timbrePtr[1]);
	timbrePtr += 2;

	_tbrCount = READ_LE_UINT16(timbrePtr);
	debugC(6, kDebugSound, "Timbres counter: %d", _tbrCount);
	timbrePtr += 2;
	_tbrStart = READ_LE_UINT16(timbrePtr);

	timbrePtr += 2;
	for (int i = 0; i < _tbrCount ; i++)
		setVoice(i, i, true);
}

void MDYPlayer::setVoice(byte voice, byte instr, bool set) {
	uint16 strct[27];
	byte channel;
	byte *timbrePtr;
	char timbreName[10];

	timbreName[9] = '\0';
	for (int j = 0; j < 9; j++)
		timbreName[j] = _timbres[6 + j + (instr * 9)];
	debugC(6, kDebugSound, "Loading timbre %s", timbreName);

	// i = 0 :  0  1  2  3  4  5  6  7  8  9 10 11 12 26
	// i = 1 : 13 14 15 16 17 18 19 20 21 22 23 24 25 27
	for (int i = 0; i < 2; i++) {
		timbrePtr = _timbres + _tbrStart + instr * 0x38 + i * 0x1A;
		for (int j = 0; j < 27; j++) {
			if (timbrePtr >= (_timbres + _timbresSize)) {
				warning("MDYPlayer: Instrument %d out of range (%d, %d)", instr,
						(uint32) (timbrePtr - _timbres), _timbresSize);
				strct[j] = 0;
			} else
				strct[j] = READ_LE_UINT16(timbrePtr);
			timbrePtr += 2;
		}
		channel = _operators[voice] + i * 3;
		writeOPL(0xBD, 0x00);
		writeOPL(0x08, 0x00);
		writeOPL(0x40 | channel, ((strct[0] & 3) << 6) | (strct[8] & 0x3F));
		if (!i)
			writeOPL(0xC0 | voice,
					((strct[2] & 7) << 1) | (1 - (strct[12] & 1)));
		writeOPL(0x60 | channel, ((strct[3] & 0xF) << 4) | (strct[6] & 0xF));
		writeOPL(0x80 | channel, ((strct[4] & 0xF) << 4) | (strct[7] & 0xF));
		writeOPL(0x20 | channel, ((strct[9] & 1) << 7) |
			((strct[10] & 1) << 6) | ((strct[5] & 1) << 5) |
			((strct[11] & 1) << 4) |  (strct[1] & 0xF));
		if (!i)
			writeOPL(0xE0 | channel, (strct[26] & 3));
		else {
			writeOPL(0xE0 | channel, (strct[14] & 3));
			writeOPL(0x40 | channel, 0);
		}
	}
}

} // End of namespace Gob
