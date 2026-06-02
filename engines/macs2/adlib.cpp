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

#include "adlib.h"
#include "audio/fmopl.h"
#include "audio/mixer.h"
#include "common/debug.h"
#include <common/memstream.h>

#define CALLBACKS_PER_SECOND 120

namespace Macs2 {

// --- OPL register access ---

void Adlib::adlibWriteReg(byte registerIndex, byte value) {
	_opl->writeReg(registerIndex, value);
	_regShadow[registerIndex] = value;
}

uint8 Adlib::adlibGetOperator(uint8 reg) {
	return _regShadow[reg];
}

// --- adlibSetInstrument (1000:27e4) ---
// Key-off all 9 channels, then set all operator volume registers to 0xFF (silence)
void Adlib::adlibSetInstrument() {
	for (uint16 i = 0; i <= 8; i++) {
		uint8 val = adlibGetOperator(i + 0xB0);
		adlibWriteReg(i + 0xB0, val & 0xDF); // clear key-on bit
	}
	for (uint16 i = 0; i <= 0x11; i++) {
		adlibWriteReg(_opSlotTable[i] + 0x40, 0xFF);
	}
}

// --- adlibSetFrequency (1000:2839) ---
// Load 11 instrument parameters from stream into OPL registers for a voice
void Adlib::adlibSetFrequency(uint8 voiceIndex, StreamHandler *sh) {
	uint8 op1 = _opMap1[voiceIndex];
	uint8 op2 = _opMap2[voiceIndex];

	adlibWriteReg(op1 + 0x20, sh->peekByteAtOffset(0, SEEK_CUR));
	adlibWriteReg(op2 + 0x20, sh->peekByteAtOffset(1, SEEK_CUR));
	adlibWriteReg(op1 + 0x40, sh->peekByteAtOffset(2, SEEK_CUR));
	adlibWriteReg(op2 + 0x40, sh->peekByteAtOffset(3, SEEK_CUR));
	adlibWriteReg(op1 + 0x60, sh->peekByteAtOffset(4, SEEK_CUR));
	adlibWriteReg(op2 + 0x60, sh->peekByteAtOffset(5, SEEK_CUR));
	adlibWriteReg(op1 + 0x80, sh->peekByteAtOffset(6, SEEK_CUR));
	adlibWriteReg(op2 + 0x80, sh->peekByteAtOffset(7, SEEK_CUR));
	adlibWriteReg(op1 + 0xE0, sh->peekByteAtOffset(8, SEEK_CUR));
	adlibWriteReg(op2 + 0xE0, sh->peekByteAtOffset(9, SEEK_CUR));
	adlibWriteReg(voiceIndex + 0xC0, sh->peekByteAtOffset(0xA, SEEK_CUR));
}

// --- adlibSetupChannel (1000:294e) ---
// Set frequency registers with key-on (bit 0x20 in B0)
void Adlib::adlibSetupChannel(uint16 voiceIndex, uint8 note, uint16 pitchBend) {
	uint16 freq = (_freqTableHi[note] << 8) | _freqTableLo[note];

	if (pitchBend != 0) {
		if (pitchBend < 0x80) {
			uint8 nextNote = (note < 0x7F) ? note + 1 : 0x7F;
			uint16 nextFreq = (_freqTableHi[nextNote] << 8) | _freqTableLo[nextNote];
			uint64 product = (uint64)pitchBend * (nextFreq - freq);
			freq += (uint16)(product >> 7);
		} else {
			uint8 prevNote = (note > 0) ? note - 1 : 0;
			uint16 prevFreq = (_freqTableHi[prevNote] << 8) | _freqTableLo[prevNote];
			uint64 product = (uint64)pitchBend * (freq - prevFreq);
			freq -= (uint16)(product >> 7);
		}
	}

	adlibWriteReg(voiceIndex + 0xA0, freq & 0xFF);
	adlibWriteReg(voiceIndex + 0xB0, (freq >> 8) | 0x20); // key-on
}

// --- adlibProcessEvent (1000:2a80) ---
// Set frequency registers WITHOUT key-on (used for note-off)
void Adlib::adlibProcessEvent(uint8 pitchBend, uint8 note, uint8 voiceIndex) {
	uint16 freq = (_freqTableHi[note] << 8) | _freqTableLo[note];

	if (pitchBend != 0) {
		if (pitchBend < 0x80) {
			uint8 nextNote = (note < 0x7F) ? note + 1 : 0x7F;
			uint16 nextFreq = (_freqTableHi[nextNote] << 8) | _freqTableLo[nextNote];
			uint64 product = (uint64)pitchBend * (nextFreq - freq);
			freq += (uint16)(product >> 7);
		} else {
			uint8 prevNote = (note > 0) ? note - 1 : 0;
			uint16 prevFreq = (_freqTableHi[prevNote] << 8) | _freqTableLo[prevNote];
			uint64 product = (uint64)pitchBend * (freq - prevFreq);
			freq -= (uint16)(product >> 7);
		}
	}

	adlibWriteReg(voiceIndex + 0xA0, freq & 0xFF);
	adlibWriteReg(voiceIndex + 0xB0, (freq >> 8) & 0xDF); // key-off (clear bit 5)
}

// --- adlibTickHandler (1000:24fd) — initialization ---
uint16 Adlib::adlibTickHandler() {
	if (_isInitialized != 0)
		return 3;

	// Waveform select enable
	adlibWriteReg(0x01, 0x20);

	_playbackPos = _songStartPtr;
	_masterVolume = 0;
	_streamBytesConsumed = 0;
	_streamBytesConsumedHi = 0;
	_loopCount = 0;
	_numOplChannels = 9;
	adlibWriteReg(0xBD, 0);
	_currentEventStatus = 0;
	_currentEventStatusHi = 0;

	for (uint16 i = 0; i <= 0x0F; i++) {
		_channelPrograms[i] = 0;
		_channelPitchBend[i] = 0;
	}

	for (uint16 i = 0; i <= 8; i++) {
		_voiceAge[i] = 1;
		_voiceMidiChannel[i] = 0xFF;
		_voiceInstrument[i] = 0xFF;
		_voiceNote[i] = 0xFF;
	}

	adlibReadDeltaTime();

	if (_timerFrequency > 0x12 && _timerFrequency != 0) {
		_timerFrequency += 9;
		_timerFrequency -= _timerFrequency % 0x12;
		_timerSubdivCounter = 0;
		// Binary: threshold = timerFrequency / 18
		// ScummVM OPL callback fires at CALLBACKS_PER_SECOND Hz.
		// Original PIT divisor = 0x10AE3C / timerFrequency, giving ISR rate = 1193180 / divisor Hz.
		// Effective music rate = ISR rate / threshold.
		// We need: CALLBACKS_PER_SECOND / our_threshold = ISR_rate / binary_threshold
		// our_threshold = CALLBACKS_PER_SECOND * binary_threshold / ISR_rate
		// Simplified: since binary ISR rate ≈ timerFrequency * 18 / something,
		// and the effective event rate = 18 Hz always, we need our_threshold = CALLBACKS_PER_SECOND / 18.
		_timerSubdivThreshold = _timerFrequency / 0x12;
		// Scale for our fixed callback rate: binary threshold assumes PIT-rate ISR.
		// PIT divisor = 0x10AE3C / timerFrequency → ISR Hz = 1193180*timerFrequency/0x10AE3C
		// Our rate is fixed at CALLBACKS_PER_SECOND. Ratio:
		// our_threshold = binary_threshold * CALLBACKS_PER_SECOND * 0x10AE3C / (1193180 * timerFrequency)
		// This simplifies to: CALLBACKS_PER_SECOND / (1193180 / (0x10AE3C / timerFrequency))
		// = CALLBACKS_PER_SECOND * (0x10AE3C / timerFrequency) / 1193180
		// For typical timerFrequency=108: PIT divisor=0x10AE3C/108=10120, ISR=1193180/10120≈117.9Hz
		// binary threshold=108/18=6, effective rate=117.9/6≈19.65Hz
		// our threshold = 120/19.65 ≈ 6.1 → 6
		// General formula: our_threshold = CALLBACKS_PER_SECOND * 0x10AE3C / (1193180 * timerFrequency) * (timerFrequency/18)
		// = CALLBACKS_PER_SECOND * 0x10AE3C / (1193180 * 18)
		// = 120 * 1093180 / 21477240 ≈ 6.1
		// So for any valid timerFrequency, threshold ≈ 6. Use the direct ratio:
		uint32 pitDivisor = 0x10AE3C / _timerFrequency;
		uint32 isrHz = 1193180 / pitDivisor;
		_timerSubdivThreshold = CALLBACKS_PER_SECOND * (_timerFrequency / 0x12) / isrHz;
		if (_timerSubdivThreshold < 1)
			_timerSubdivThreshold = 1;
	}

	_isInitialized = 1;
	_statusFlags = 0x10;
	return 0;
}

// --- adlibReadDeltaTime ---
// Read variable-length delta time from stream (same as MIDI VLQ)
void Adlib::adlibReadDeltaTime() {
	_nextEventTimer = 0;
	uint8 b;
	do {
		b = _playbackPos->peekByte();
		_nextEventTimer = (_nextEventTimer << 7) + (b & 0x7F);
		_playbackPos = adlibSeekStream(_playbackPos, 1);
		_streamBytesConsumed++;
	} while (b & 0x80);
}

// --- adlibSeekStream ---
StreamHandler *Adlib::adlibSeekStream(StreamHandler *inHandler, uint16 seekDelta) {
	StreamHandler *result = new StreamHandler(*inHandler);
	uint16 pos = result->pos();
	if (seekDelta > 0xFFF8)
		pos &= 0xF;
	pos += seekDelta;
	result->seek(pos, SEEK_SET);
	return result;
}

// --- OnTimer (ISR handler, 1000:1a9f) ---
void Adlib::OnTimer() {
	if (_playbackPos == nullptr)
		return;

	_timerSubdivCounter++;
	bool timerTick = (_timerSubdivCounter >= _timerSubdivThreshold);
	if (timerTick)
		_timerSubdivCounter = 0;
	_isTimerTick = timerTick;

	_statusFlags &= 0xDF;
	if (_statusFlags & 0x02)
		_statusFlags |= 0x40;

	if ((_statusFlags & 0xC3) != 0) {
		// Stop requested or busy: silence all and return
		if ((_statusFlags & 0xC2) != 0)
			_statusFlags &= ~0xC2;
		adlibSetInstrument();
		return;
	}

	if (_nextEventTimer != 0) {
		_nextEventTimer--;
		return;
	}

	// Process events until next non-zero delta
	for (;;) {
		// Running status: if high bit set, update status byte
		uint8 peek = _playbackPos->peekByte();
		if (peek & 0x80) {
			_currentMidiStatus = peek;
			_currentEventStatus = peek;
			_currentEventStatusHi = 0;
			_playbackPos = adlibSeekStream(_playbackPos, 1);
			_streamBytesConsumed++;
		}

		uint8 status = _currentMidiStatus;
		uint8 channel = status & 0x0F;
		uint8 eventType = status; // tracks note-on→note-off conversion

		uint8 data1 = _playbackPos->peekByte();
		StreamHandler *afterData1 = adlibSeekStream(_playbackPos, 1);
		uint8 data2 = afterData1->peekByte();

		// --- Note On (0x90) ---
		if ((status & 0xF0) == 0x90) {
			if (data2 == 0) {
				// Velocity 0 = note off
				eventType = 0x80;
			} else {
				_playbackPos = adlibSeekStream(_playbackPos, 2);
				_streamBytesConsumed += 2;

				if (_numOplChannels == 9 || channel < 0x0B) {
					// --- Melodic note-on ---
					uint8 voice = 0;
					// Find existing voice with same channel+note (age==0)
					while (voice < _numOplChannels) {
						if (_voiceAge[voice] == 0 && _voiceMidiChannel[voice] == channel && _voiceNote[voice] == data1)
							break;
						voice++;
					}
					// If not found, steal oldest voice
					if (voice == _numOplChannels) {
						uint8 maxAge = 0;
						voice = _numOplChannels; // sentinel
						for (uint8 v = 0; v < _numOplChannels; v++) {
							if (_voiceAge[v] != 0)
								_voiceAge[v]++;
							if (_voiceAge[v] > maxAge) {
								maxAge = _voiceAge[v];
								voice = v;
							}
						}
						if (maxAge != 0) {
							_voiceAge[voice] = 0;
							_voiceMidiChannel[voice] = channel;
							if (_channelPrograms[channel] != _voiceInstrument[voice]) {
								_voiceInstrument[voice] = _channelPrograms[channel];
								StreamHandler *instData = adlibSeekStream(_instrumentDataPtr, _voiceInstrument[voice] << 4);
								adlibSetFrequency(voice, instData);
							}
						}
					}
					// Play the note
					if (voice < _numOplChannels) {
						_voiceNote[voice] = data1;
						StreamHandler *instData = adlibSeekStream(_instrumentDataPtr, _channelPrograms[channel] << 4);
						// Volume calculation: velocity → attenuation
						uint8 velAtten = (uint8)((0x3F - ((data2 & 0x7F) >> 1)) >> 1) >> 1;

						// Operator 2 volume (carrier)
						StreamHandler *op2Data = adlibSeekStream(instData, 2);
						uint8 op2Base = op2Data->peekByte() & 0x3F;
						uint8 vol2 = op2Base + (uint8)((uint16)(velAtten * (0x3F - op2Base)) / 0x3F) + _masterVolume;

						// Operator 1 volume (modulator)
						StreamHandler *op1Data = adlibSeekStream(instData, 3);
						uint8 op1Base = op1Data->peekByte() & 0x3F;
						uint8 vol1 = op1Base + (uint8)((uint16)(velAtten * (0x3F - op1Base)) / 0x3F) + _masterVolume;

						if (vol1 > 0x3F) vol1 = 0x3F;
						if (vol2 > 0x3F) vol2 = 0x3F;

						// Key off, set volumes, then key on with note
						adlibWriteReg(voice + 0xB0, 0);
						uint8 reg2 = adlibGetOperator(_opMap2[voice] + 0x40);
						adlibWriteReg(_opMap2[voice] + 0x40, (reg2 & 0xC0) + vol1);
						uint8 reg1 = adlibGetOperator(_opMap1[voice] + 0x40);
						adlibWriteReg(_opMap1[voice] + 0x40, (reg1 & 0xC0) + vol2);

						_channelPitchBend[channel] = 0;
						adlibSetupChannel(voice, data1, _channelPitchBend[channel]);
					}
				} else {
					// --- Percussion note-on (channels >= 0xB) ---
					StreamHandler *instData = adlibSeekStream(_instrumentDataPtr, _channelPrograms[channel] << 4);
					uint8 opIdx = _percOpMap[channel - 0xB];

					if (channel == 0xB) {
						// Bass drum: load full instrument
						StreamHandler *bdInst = adlibSeekStream(_instrumentDataPtr, _channelPrograms[0xB] << 4);
						adlibSetFrequency(_percFreqChannel[channel - 0xB], bdInst);
					} else {
						// Other percussion: load individual operator registers
						adlibWriteReg(opIdx + 0x20, instData->peekByte());
						StreamHandler *sh2 = adlibSeekStream(instData, 2);
						adlibWriteReg(opIdx + 0x40, sh2->peekByte());
						StreamHandler *sh4 = adlibSeekStream(instData, 4);
						adlibWriteReg(opIdx + 0x60, sh4->peekByte());
						StreamHandler *sh6 = adlibSeekStream(instData, 6);
						adlibWriteReg(opIdx + 0x80, sh6->peekByte());
						StreamHandler *sh8 = adlibSeekStream(instData, 8);
						adlibWriteReg(opIdx + 0xE0, sh8->peekByte());
					}

					// Percussion volume
					StreamHandler *volData = adlibSeekStream(_instrumentDataPtr, (_channelPrograms[channel] << 4) + 3);
					uint8 volIdx = ((volData->peekByte() & 0x3F) >> 4) * 8 + (data2 >> 4);
					uint8 vol = _masterVolume;
					if (volIdx < _percVolTable.size())
						vol = _percVolTable[volIdx] + _masterVolume;
					if (vol > 0x3F) vol = 0x3F;

					// Key off, set volume, set frequency, trigger percussion
					adlibWriteReg(_percFreqChannel[channel - 0xB] + 0xB0, 0);
					uint8 regVal = adlibGetOperator(opIdx + 0x40);
					adlibWriteReg(opIdx + 0x40, vol + (regVal & 0xC0));
					adlibSetupChannel(_percFreqChannel[channel - 0xB], data1, 0);

					// Set percussion bit in 0xBD
					uint8 bdVal = adlibGetOperator(0xBD);
					adlibWriteReg(0xBD, bdVal | (1 << (0xF - channel)));
				}
			}
		}

		// --- Note Off (0x80) ---
		if ((eventType & 0xF0) == 0x80) {
			_playbackPos = adlibSeekStream(_playbackPos, 2);
			_streamBytesConsumed += 2;

			// Age all voices
			for (uint8 v = 0; v < _numOplChannels; v++) {
				if (_voiceAge[v] != 0)
					_voiceAge[v]++;
			}

			if (_numOplChannels == 9 || channel < 0x0B) {
				// Find the voice playing this note
				uint8 voice = 0;
				while (voice < _numOplChannels) {
					if (_voiceAge[voice] == 0 && _voiceMidiChannel[voice] == channel && _voiceNote[voice] == data1)
						break;
					voice++;
				}
				if (voice < _numOplChannels) {
					// Note off: write frequency WITHOUT key-on
					adlibProcessEvent(_channelPitchBend[channel], data1, voice);
					_voiceAge[voice] = 1;
				}
			} else {
				// Percussion note-off: clear bit in 0xBD
				uint8 bdVal = adlibGetOperator(0xBD);
				adlibWriteReg(0xBD, bdVal & ~(1 << (0xF - channel)));
			}
		}

		// --- Pitch Wheel / Aftertouch (0xE0, 0xA0) ---
		if ((eventType & 0xF0) == 0xE0 || (eventType & 0xF0) == 0xA0) {
			_playbackPos = adlibSeekStream(_playbackPos, 2);
			_streamBytesConsumed += 2;
		}

		// --- Control Change (0xB0) ---
		if ((eventType & 0xF0) == 0xB0) {
			_playbackPos = adlibSeekStream(_playbackPos, 2);
			_streamBytesConsumed += 2;

			if (data1 == 0x66) {
				_loopCount = data2;
				_statusFlags |= 0x20;
			} else if (data1 == 0x67) {
				if (data2 != 0) {
					_numOplChannels = 6;
					adlibWriteReg(0xBD, 0x20);
				} else {
					_numOplChannels = 9;
					adlibWriteReg(0xBD, 0);
				}
			} else if (data1 == 0x69) {
				_channelPitchBend[channel] = (uint8)(-(int8)data2);
				for (uint8 v = 0; v < _numOplChannels; v++) {
					if (_voiceMidiChannel[v] == channel && _voiceAge[v] == 0)
						adlibSetupChannel(v, _voiceNote[v], (uint8)(-(int8)data2));
				}
			} else if (data1 == 0x68) {
				_channelPitchBend[channel] = data2;
				for (uint8 v = 0; v < _numOplChannels; v++) {
					if (_voiceMidiChannel[v] == channel && _voiceAge[v] == 0)
						adlibSetupChannel(v, _voiceNote[v], data2);
				}
			}
		}

		// --- Program Change (0xC0) ---
		if ((eventType & 0xF0) == 0xC0) {
			_playbackPos = adlibSeekStream(_playbackPos, 1);
			_streamBytesConsumed++;
			_channelPrograms[channel] = data1;
		}

		// --- Channel Pressure (0xD0) ---
		if ((eventType & 0xF0) == 0xD0) {
			_playbackPos = adlibSeekStream(_playbackPos, 1);
			_streamBytesConsumed++;
		}

		// --- System/Meta (0xF0) ---
		if ((eventType & 0xF0) == 0xF0) {
			// Binary: BOTH 0x2F and other meta events reset to song start
			_playbackPos = _songStartPtr;
			_streamBytesConsumed = 0;
			_streamBytesConsumedHi = 0;
			_loopCount = 0;
			_playbackReady = 1;
			adlibReadDeltaTime();
		} else {
			adlibReadDeltaTime();
		}

		// Safety: if delta > 0xFFF, reset to start (corrupted stream)
		if (_nextEventTimer > 0x0FFF) {
			_playbackPos = _songStartPtr;
			_streamBytesConsumed = 0;
			_streamBytesConsumedHi = 0;
			_loopCount = 0;
			_playbackReady = 1;
			adlibReadDeltaTime();
		}

		if (_nextEventTimer != 0)
			break;
	}
}

// --- Public API ---

void Adlib::Init() {
	_opl = OPL::Config::create();
	_opl->init();

	_regShadow.resize(256);
	_channelPrograms.resize(0x10);
	_channelPitchBend.resize(0x10);
	_voiceAge.resize(9);
	_voiceMidiChannel.resize(9);
	_voiceInstrument.resize(9);
	_voiceNote.resize(9);

	_opl->start(new Common::Functor0Mem<void, Adlib>(this, &Adlib::OnTimer), CALLBACKS_PER_SECOND);
}

void Adlib::Deinit() {
	_opl->stop();
	delete _activeSongStream;
	_activeSongStream = nullptr;
	delete _opl;
}

void Adlib::PlaySongData(const Common::Array<uint8> &data) {
	// Stop previous playback
	_isInitialized = 0;
	delete _activeSongStream;

	_activeSongData = data;
	_activeSongStream = new StreamHandler(&_activeSongData);

	// Parse song header (adlibPlaySong, 1000:244d)
	StreamHandler *sh6 = adlibSeekStream(_activeSongStream, 0x6);
	uint16 instOffset = sh6->peekWord();
	_instrumentDataPtr = adlibSeekStream(_activeSongStream, instOffset);

	StreamHandler *sh8 = adlibSeekStream(_activeSongStream, 0x8);
	uint16 dataOffset = sh8->peekWord();
	_songStartPtr = adlibSeekStream(_activeSongStream, dataOffset);

	StreamHandler *sh24 = adlibSeekStream(_activeSongStream, 0x24);
	_currentEventStatusHi = sh24->peekWord();

	StreamHandler *shC = adlibSeekStream(_activeSongStream, 0xC);
	_timerFrequency = shC->peekWord();

	adlibTickHandler();
}

void Adlib::StopMusic() {
	// Binary adlibStopMusic (1000:264d): set stop flag, ISR handles cleanup
	_statusFlags |= 2;
	// In ScummVM we can't spin-wait for ISR, so directly clean up
	_playbackPos = nullptr;
	delete _activeSongStream;
	_activeSongStream = nullptr;
	_activeSongData.clear();
	_isInitialized = 0;
	_masterVolume = 0;
}

void Adlib::SetVolume(uint16 volume) {
	// Binary adlibSetVolume (1000:2663): g_bAdlibMasterVolume = param & 0x3F
	_masterVolume = volume & 0x3F;
}

void Adlib::ReadDataFromExecutable(Common::MemoryReadStream *fileStream) {
	constexpr uint32 size = 255;

	_opSlotTable.resize(size);
	LoadData(fileStream, 0x0001B669, size, _opSlotTable.data());

	_opMap1.resize(size);
	LoadData(fileStream, 0x0001B68D, size, _opMap1.data());

	_opMap2.resize(size);
	LoadData(fileStream, 0x0001B696, size, _opMap2.data());

	_freqTableLo.resize(size);
	LoadData(fileStream, 0x0001B69F, size, _freqTableLo.data());

	_freqTableHi.resize(size);
	LoadData(fileStream, 0x0001B71F, size, _freqTableHi.data());

	// Percussion lookup tables
	_percVolTable = {28, 25, 23, 18, 14, 11, 8, 2, 50, 42, 37, 35, 34, 32, 30, 2, 55, 50, 49, 48, 45, 43, 40, 2, 60, 60, 58, 56, 54, 52, 50, 2};
	_percOpMap = {19, 20, 18, 21, 17};
	_percFreqChannel = {6, 7, 8, 8, 7};
}

void Adlib::LoadData(Common::MemoryReadStream *fileStream, int64 pos, uint16 size, void *target) {
	fileStream->seek(pos, SEEK_SET);
	fileStream->read(target, size);
}

// --- StreamHandler implementation ---

inline StreamHandler::StreamHandler(Common::MemorySeekableReadWriteStream *s) : _stream(s), _pos(s->pos()) {
}

StreamHandler::StreamHandler(Common::Array<uint8> *data) {
	_stream = new Common::MemorySeekableReadWriteStream(data->data(), data->size());
	_pos = _stream->pos();
}

bool StreamHandler::eos() const {
	_stream->seek(_pos);
	return _stream->eos();
}

uint32 StreamHandler::read(void *dataPtr, uint32 dataSize) {
	_stream->seek(_pos);
	return _stream->read(dataPtr, dataSize);
}

int64 StreamHandler::pos() const {
	return _pos;
}

int64 StreamHandler::size() const {
	return _stream->size();
}

bool StreamHandler::seek(int64 offset, int whence) {
	_stream->seek(_pos, SEEK_SET);
	bool result = _stream->seek(offset, whence);
	_pos = _stream->pos();
	return result;
}

byte StreamHandler::peekByte() {
	_stream->seek(_pos, SEEK_SET);
	byte result = readByte();
	return result;
}

byte StreamHandler::peekByteAtOffset(int64 offset, int whence) {
	int64 oldPos = _pos;
	seek(offset, whence);
	byte result = peekByte();
	seek(oldPos, SEEK_SET);
	return result;
}

uint16 StreamHandler::peekWord() {
	_stream->seek(_pos, SEEK_SET);
	uint16 result = readUint16LE();
	return result;
}

} // End of namespace Macs2
