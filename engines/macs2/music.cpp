/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/macs2/music.h"
#include "audio/fmopl.h"
#include "audio/midiparser.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "engines/macs2/midiparser_macs2.h"

#define CALLBACKS_PER_SECOND 120

namespace Macs2 {

Music::Music() : _opl(nullptr), _parser(nullptr), _playing(false),
				 _masterVolume(0), _numOplChannels(9), _instrumentDataOffset(0) {
	memset(_regShadow, 0, sizeof(_regShadow));
	memset(_voiceAge, 1, sizeof(_voiceAge));
	memset(_voiceMidiChannel, 0xFF, sizeof(_voiceMidiChannel));
	memset(_voiceInstrument, 0xFF, sizeof(_voiceInstrument));
	memset(_voiceNote, 0xFF, sizeof(_voiceNote));
	memset(_channelPrograms, 0, sizeof(_channelPrograms));
	memset(_channelPitchBend, 0, sizeof(_channelPitchBend));
}

Music::~Music() {
	deinit();
}

void Music::init() {
	_opl = OPL::Config::create();
	_opl->init();
	_opl->start(new Common::Functor0Mem<void, Music>(this, &Music::onTimer), CALLBACKS_PER_SECOND);
}

void Music::deinit() {
	stopMusic();
	if (_opl) {
		_opl->stop();
		delete _opl;
		_opl = nullptr;
	}
}

void Music::onTimer() {
	if (_parser) {
		// Binary adlibISRHandler (1000:1a9f): g_bAdlibPlaybackReady is set when the
		// song stream loops back to the start (0xF0/0x2F meta or timer expiry), not on
		// the first timer tick after playMusicSlot clears the flag.
		const uint32 tickBefore = _parser->getTick();
		_parser->onTimer();
		if (_playing && !_adlibPlaybackReady && _parser->getTick() < tickBefore)
			_adlibPlaybackReady = true;
	}
	updateDebugState();
}

void Music::writeReg(byte reg, byte value) {
	if (_opl) {
		_opl->writeReg(reg, value);
		_regShadow[reg] = value;
	}
}

void Music::silenceAll() {
	// Key-off all 9 channels
	for (int i = 0; i <= 8; i++) {
		writeReg(i + 0xB0, readReg(i + 0xB0) & 0xDF);
	}
	// Silence all operator volumes
	for (int i = 0; i < (int)_opSlotTable.size() && i <= 0x11; i++) {
		writeReg(_opSlotTable[i] + 0x40, 0xFF);
	}
}

void Music::playSongData(const Common::Array<uint8> &data) {
	stopMusic();

	_songData = data;

	_parser = new MidiParser_Macs2();
	_parser->setMidiDriver(this);
	_parser->setTimerRate(1000000 / CALLBACKS_PER_SECOND);
	_parser->property(MidiParser::mpAutoLoop, 1);

	// Enable waveform select
	writeReg(0x01, 0x20);
	writeReg(0xBD, 0);
	_numOplChannels = 9;

	// Reset state
	memset(_voiceAge, 1, sizeof(_voiceAge));
	memset(_voiceMidiChannel, 0xFF, sizeof(_voiceMidiChannel));
	memset(_voiceInstrument, 0xFF, sizeof(_voiceInstrument));
	memset(_voiceNote, 0xFF, sizeof(_voiceNote));
	memset(_channelPrograms, 0, sizeof(_channelPrograms));
	memset(_channelPitchBend, 0, sizeof(_channelPitchBend));

	// Load instrument data from the song file header
	if (_songData.size() > 0x08) {
		_instrumentDataOffset = READ_LE_UINT16(_songData.data() + 0x06);
	}

	if (!_parser->loadMusic(_songData.data(), _songData.size())) {
		delete _parser;
		_parser = nullptr;
		return;
	}

	_playing = true;
	_adlibPlaybackReady = false;
}

void Music::stopMusic() {
	_playing = false;
	_adlibPlaybackReady = true;
	if (_parser) {
		_parser->unloadMusic();
		delete _parser;
		_parser = nullptr;
	}
	silenceAll();
	_songData.clear();
	_masterVolume = 0;
}

void Music::setVolume(uint16 volume) {
	_masterVolume = volume & 0x3F;
}

// --- MidiDriver_BASE interface ---

void Music::send(uint32 b) {
	byte cmd = b & 0xF0;
	byte channel = b & 0x0F;
	byte param1 = (b >> 8) & 0xFF;
	byte param2 = (b >> 16) & 0xFF;

	switch (cmd) {
	case 0x90:
		if (param2 == 0)
			noteOff(channel, param1);
		else
			noteOn(channel, param1, param2);
		break;
	case 0x80:
		noteOff(channel, param1);
		break;
	case 0xC0:
		programChange(channel, param1);
		break;
	case 0xB0:
		controlChange(channel, param1, param2);
		break;
	default:
		break;
	}
}

void Music::metaEvent(byte type, const byte *data, uint16 length) {
	(void)data;
	(void)length;
	if (type == 0x2F && _playing) {
		// End-of-track meta (binary adlibISRHandler 0xF0/0x2F path).
		_adlibPlaybackReady = true;
	}
}

// --- Music playback logic (matching original macs2 behavior) ---

void Music::noteOn(byte channel, byte note, byte velocity) {
	if (_numOplChannels == 9 || channel < 0x0B) {
		// Melodic note-on
		uint8 voice = _numOplChannels; // sentinel

		// Find existing voice with same channel+note
		for (uint8 v = 0; v < _numOplChannels; v++) {
			if (_voiceAge[v] == 0 && _voiceMidiChannel[v] == channel && _voiceNote[v] == note) {
				voice = v;
				break;
			}
		}

		// If not found, steal oldest voice
		if (voice == _numOplChannels) {
			uint8 maxAge = 0;
			for (uint8 v = 0; v < _numOplChannels; v++) {
				if (_voiceAge[v] != 0)
					_voiceAge[v]++;
				if (_voiceAge[v] > maxAge) {
					maxAge = _voiceAge[v];
					voice = v;
				}
			}
			if (maxAge == 0)
				return;

			_voiceAge[voice] = 0;
			_voiceMidiChannel[voice] = channel;
			if (_channelPrograms[channel] != _voiceInstrument[voice]) {
				_voiceInstrument[voice] = _channelPrograms[channel];
				loadInstrument(voice, _voiceInstrument[voice]);
			}
		}

		if (voice >= _numOplChannels)
			return;

		_voiceNote[voice] = note;

		// Volume calculation matching original
		uint8 velAtten = (uint8)((0x3F - ((velocity & 0x7F) >> 1)) >> 1) >> 1;
		uint16 instBase = (uint16)_channelPrograms[channel] << 4;

		uint8 op2Base = 0;
		uint8 op1Base = 0;
		if ((uint32)(_instrumentDataOffset + instBase + 4) <= _songData.size()) {
			const byte *instPtr = _songData.data() + _instrumentDataOffset + instBase;
			op2Base = instPtr[2] & 0x3F;
			op1Base = instPtr[3] & 0x3F;
		}

		uint8 vol2 = op2Base + (uint8)((uint16)(velAtten * (0x3F - op2Base)) / 0x3F) + _masterVolume;
		uint8 vol1 = op1Base + (uint8)((uint16)(velAtten * (0x3F - op1Base)) / 0x3F) + _masterVolume;
		if (vol1 > 0x3F)
			vol1 = 0x3F;
		if (vol2 > 0x3F)
			vol2 = 0x3F;

		// Key off, set volumes, then key on
		writeReg(voice + 0xB0, 0);
		byte reg2 = readReg(_opMap2[voice] + 0x40);
		writeReg(_opMap2[voice] + 0x40, (reg2 & 0xC0) + vol1);
		byte reg1 = readReg(_opMap1[voice] + 0x40);
		writeReg(_opMap1[voice] + 0x40, (reg1 & 0xC0) + vol2);

		_channelPitchBend[channel] = 0;
		setFrequency(voice, note, 0);
	} else {
		// Percussion note-on
		uint16 instBase = (uint16)_channelPrograms[channel] << 4;
		uint8 percIdx = channel - 0x0B;

		if (percIdx >= _percOpMap.size())
			return;

		uint8 opIdx = _percOpMap[percIdx];

		if (channel == 0x0B) {
			// Bass drum: load full instrument
			loadInstrument(_percFreqChannel[percIdx], _channelPrograms[0x0B]);
		} else if ((uint32)_instrumentDataOffset + instBase + 9 <= _songData.size()) {
			const byte *instPtr = _songData.data() + _instrumentDataOffset + instBase;
			writeReg(opIdx + 0x20, instPtr[0]);
			writeReg(opIdx + 0x40, instPtr[2]);
			writeReg(opIdx + 0x60, instPtr[4]);
			writeReg(opIdx + 0x80, instPtr[6]);
			writeReg(opIdx + 0xE0, instPtr[8]);
		}

		// Percussion volume
		uint8 vol = _masterVolume;
		if ((uint32)(_instrumentDataOffset + instBase + 3) < _songData.size()) {
			byte volByte = _songData[_instrumentDataOffset + instBase + 3];
			uint8 volIdx = ((volByte & 0x3F) >> 4) * 8 + (velocity >> 4);
			if (volIdx < _percVolTable.size())
				vol = _percVolTable[volIdx] + _masterVolume;
		}
		if (vol > 0x3F)
			vol = 0x3F;

		uint8 freqChan = _percFreqChannel[percIdx];
		writeReg(freqChan + 0xB0, 0);
		byte regVal = readReg(opIdx + 0x40);
		writeReg(opIdx + 0x40, vol + (regVal & 0xC0));
		setFrequency(freqChan, note, 0);

		byte bdVal = readReg(0xBD);
		writeReg(0xBD, bdVal | (1 << (0xF - channel)));
	}
}

void Music::noteOff(byte channel, byte note) {
	// Age all voices
	for (uint8 v = 0; v < _numOplChannels; v++) {
		if (_voiceAge[v] != 0)
			_voiceAge[v]++;
	}

	if (_numOplChannels == 9 || channel < 0x0B) {
		for (uint8 v = 0; v < _numOplChannels; v++) {
			if (_voiceAge[v] == 0 && _voiceMidiChannel[v] == channel && _voiceNote[v] == note) {
				// Write frequency without key-on
				uint16 freq = ((uint16)_freqTableHi[note] << 8) | _freqTableLo[note];
				writeReg(v + 0xA0, freq & 0xFF);
				writeReg(v + 0xB0, (freq >> 8) & 0xDF); // clear key-on bit
				_voiceAge[v] = 1;
				break;
			}
		}
	} else {
		// Percussion note-off
		byte bdVal = readReg(0xBD);
		writeReg(0xBD, bdVal & ~(1 << (0xF - channel)));
	}
}

void Music::programChange(byte channel, byte program) {
	_channelPrograms[channel] = program;
}

void Music::controlChange(byte channel, byte control, byte value) {
	switch (control) {
	case 0x66:
		// Loop count - handled by parser's autoloop
		break;
	case 0x67:
		if (value != 0) {
			_numOplChannels = 6;
			writeReg(0xBD, 0x20);
		} else {
			_numOplChannels = 9;
			writeReg(0xBD, 0);
		}
		break;
	case 0x68:
		_channelPitchBend[channel] = value;
		for (uint8 v = 0; v < _numOplChannels; v++) {
			if (_voiceMidiChannel[v] == channel && _voiceAge[v] == 0)
				setFrequency(v, _voiceNote[v], value);
		}
		break;
	case 0x69:
		_channelPitchBend[channel] = (uint8)(-(int8)value);
		for (uint8 v = 0; v < _numOplChannels; v++) {
			if (_voiceMidiChannel[v] == channel && _voiceAge[v] == 0)
				setFrequency(v, _voiceNote[v], (uint8)(-(int8)value));
		}
		break;
	default:
		break;
	}
}

void Music::loadInstrument(uint8 voice, uint8 program) {
	uint16 instBase = (uint16)program << 4;
	if ((uint32)_instrumentDataOffset + instBase + 11 > _songData.size())
		return;

	const byte *inst = _songData.data() + _instrumentDataOffset + instBase;
	uint8 op1 = _opMap1[voice];
	uint8 op2 = _opMap2[voice];

	writeReg(op1 + 0x20, inst[0]);
	writeReg(op2 + 0x20, inst[1]);
	writeReg(op1 + 0x40, inst[2]);
	writeReg(op2 + 0x40, inst[3]);
	writeReg(op1 + 0x60, inst[4]);
	writeReg(op2 + 0x60, inst[5]);
	writeReg(op1 + 0x80, inst[6]);
	writeReg(op2 + 0x80, inst[7]);
	writeReg(op1 + 0xE0, inst[8]);
	writeReg(op2 + 0xE0, inst[9]);
	writeReg(voice + 0xC0, inst[10]);
}

void Music::setFrequency(uint8 voice, uint8 note, uint8 pitchBend) {
	if (note == 0xFF || note >= _freqTableLo.size())
		return;

	uint16 freq = ((uint16)_freqTableHi[note] << 8) | _freqTableLo[note];

	if (pitchBend != 0) {
		if (pitchBend < 0x80) {
			uint8 nextNote = (note < 0x7F) ? note + 1 : 0x7F;
			uint16 nextFreq = ((uint16)_freqTableHi[nextNote] << 8) | _freqTableLo[nextNote];
			freq += (uint16)((uint64)pitchBend * (nextFreq - freq) >> 7);
		} else {
			uint8 prevNote = (note > 0) ? note - 1 : 0;
			uint16 prevFreq = ((uint16)_freqTableHi[prevNote] << 8) | _freqTableLo[prevNote];
			freq -= (uint16)((uint64)pitchBend * (freq - prevFreq) >> 7);
		}
	}

	writeReg(voice + 0xA0, freq & 0xFF);
	writeReg(voice + 0xB0, (freq >> 8) | 0x20); // key-on
}

void Music::updateDebugState() {
	_debug.masterVolume = _masterVolume;
	_debug.numOplChannels = _numOplChannels;

	for (int i = 0; i < 9; i++) {
		_debug.voices[i].note = _voiceNote[i];
		_debug.voices[i].channel = _voiceMidiChannel[i];
		_debug.voices[i].active = (_voiceAge[i] == 0);
		_debug.voices[i].volume = _regShadow[_opMap2.size() > (uint)i ? _opMap2[i] + 0x40 : 0] & 0x3F;
		_debug.regHistory[i][_debug.ringPos] = (_regShadow[i + 0xB0] & 0x20) ? 1.0f - (_debug.voices[i].volume / 63.0f) : 0.0f;
	}
	_debug.ringPos = (_debug.ringPos + 1) % kDebugRingSize;
}

void Music::readDataFromExecutable(Common::MemoryReadStream *fileStream) {
	constexpr uint32 size = 255;

	_opSlotTable.resize(size);
	loadData(fileStream, 0x0001B669, size, _opSlotTable.data());

	_opMap1.resize(size);
	loadData(fileStream, 0x0001B68D, size, _opMap1.data());

	_opMap2.resize(size);
	loadData(fileStream, 0x0001B696, size, _opMap2.data());

	_freqTableLo.resize(size);
	loadData(fileStream, 0x0001B69F, size, _freqTableLo.data());

	_freqTableHi.resize(size);
	loadData(fileStream, 0x0001B71F, size, _freqTableHi.data());

	_percVolTable = {28, 25, 23, 18, 14, 11, 8, 2, 50, 42, 37, 35, 34, 32, 30, 2, 55, 50, 49, 48, 45, 43, 40, 2, 60, 60, 58, 56, 54, 52, 50, 2};
	_percOpMap = {19, 20, 18, 21, 17};
	_percFreqChannel = {6, 7, 8, 8, 7};
}

void Music::loadData(Common::MemoryReadStream *fileStream, int64 pos, uint16 size, void *target) {
	fileStream->seek(pos, SEEK_SET);
	fileStream->read(target, size);
}

} // End of namespace Macs2
