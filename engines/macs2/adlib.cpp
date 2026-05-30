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

void Adlib::adlibWriteReg(byte registerIndex, byte value) {

	// _opl->write(0x388, registerIndex);
	_opl->writeReg(registerIndex, value);
	gArray229C[registerIndex] = value;
}

void Adlib::adlibSetInstrument() {
	uint16 local_counter; // bp-2h (2-byte local variable)

	// First loop section (27EF-27F2 labels)
	local_counter = 0;
	do {
		// if (local_counter >= 9)
		// originally

		// Body of first loop
		uint16 param = local_counter + 0xB0;
		// the first time I saw it)
		uint8 result = adlibGetOperator(param);
		result &= 0xDF; // Convert to uppercase
		adlibWriteReg(param, result);

		// l0017_27EF
		local_counter++;
	} while (local_counter <= 8); // cmp 8h, jnz 27EFh

	// Second loop section (2813-281D labels)
	local_counter = 0;
	do {

		uint8 mem_value = gArray69[local_counter];
		uint16 param = mem_value + 0x40;

		adlibWriteReg(param, 0xFF);

		// l0017_281A
		local_counter++;
	} while (local_counter <= 0x11); // cmp 11h, jnz 281Ah
}

uint16 Adlib::adlibStopMusic() {
	// Ignoring this code for now, maybe just fancy sync stuff not needed on the emulator
	// Reset all register to 0
	for (int i = 2; i < 256; i++) {
		adlibWriteReg(i, 0);
	}

	// Waveform select of register 1
	adlibWriteReg(0x1, 0x20);
	return 0;
}

uint8 Adlib::adlibGetOperator(uint8 arg1) {
	return gArray229C[arg1];
}

uint16 Adlib::adlibTickHandler() {
	// Local variables (BP-2 = return value, BP-4/BG-6 temps)
	uint16 return_value;
	// uint16 temp_var;
	uint16 loop_counter;

	if (_isInitialized != 0) {
		return_value = 3;
		goto CLEANUP_2648;
	}

	// [Original label: l0017_2510]
	return_value = adlibStopMusic();
	if (return_value != 0) {
		goto CLEANUP_2648;
	}

	// [Original label: l0017_2527]
	// Following code corresponds to the memory/port operations
	adlibWriteReg(0x1, 0x20);
	// for now

	// Original clears interrupts here
	shMem2250 = shMem2244;
	_masterVolume = 0;
	_streamBytesConsumed = _streamBytesConsumedHi = 0;
	_loopCount = 0;
	_numOplChannels = 9;
	adlibWriteReg(0xBD, 0);
	_currentEventStatus = 0;
	_currentEventStatusHi = 0;

	for (loop_counter = 0; loop_counter <= 0x0F; ++loop_counter) {
		_channelPrograms[loop_counter] = 0;
		_channelPitchBend[loop_counter] = 0;
	}

	// Loop 2: Initialize array elements [Original labels: 259A-25C3]
	for (loop_counter = 0; loop_counter <= 8; ++loop_counter) {
		_voiceAge[loop_counter] = 1;
		_voiceMidiChannel[loop_counter] = 0xFF;
		_voiceInstrument[loop_counter] = 0xFF;
		_voiceNote[loop_counter] = 0xFF;
	}

	adlibReadDeltaTime();

	// Device/hardware operations (timer/speaker?)
	if ((_timerFrequency > 0x12) && (_timerFrequency != 0)) {
		_timerFrequency += 9;
		uint32 temp = _timerFrequency;
		temp = temp % 0x12;
		_timerFrequency -= temp;
		_timerSubdivCounter = 0;
		// Original: _timerSubdivThreshold = _timerFrequency / 0x12, then PIT is reprogrammed so that
		// Since our OPL callback fires at a fixed 120 Hz, we compute _timerSubdivThreshold
		// to produce the same effective rate: 120 / _timerSubdivThreshold = ~20 Hz → _timerSubdivThreshold = 6.
		_timerSubdivThreshold = CALLBACKS_PER_SECOND / 20;

		// The original reprograms the PIT here: divisor = 0x10AE3C / _timerFrequency.
		// This changes the ISR rate, but the _timerSubdivThreshold divider compensates so the
		// effective music event rate is always ~19.65 Hz regardless of tempo.
		// We achieve the same by fixing _timerSubdivThreshold = CALLBACKS_PER_SECOND / 20 above.
	}

	// This part sets an interrupt
	_isInitialized = 1;
	_statusFlags = 0x10;
	return_value = 0;

CLEANUP_2648:
	return return_value;
}

void Adlib::adlibSetFrequency(uint8 bpp0A, StreamHandler *sh) {
	uint8 bp1 = gArray8d[bpp0A];
	uint8 bp2 = gArray96[bpp0A];

	uint8 value = sh->peekByteAtOffset(0, SEEK_CUR);
	adlibWriteReg(bp1 + 0x20, value);

	value = sh->peekByteAtOffset(1, SEEK_CUR);
	adlibWriteReg(bp2 + 0x20, value);

	value = sh->peekByteAtOffset(2, SEEK_CUR);
	adlibWriteReg(bp1 + 0x40, value);

	value = sh->peekByteAtOffset(3, SEEK_CUR);
	adlibWriteReg(bp2 + 0x40, value);

	value = sh->peekByteAtOffset(4, SEEK_CUR);
	adlibWriteReg(bp1 + 0x60, value);

	value = sh->peekByteAtOffset(5, SEEK_CUR);
	adlibWriteReg(bp2 + 0x60, value);

	value = sh->peekByteAtOffset(6, SEEK_CUR);
	adlibWriteReg(bp1 + 0x80, value);

	value = sh->peekByteAtOffset(7, SEEK_CUR);
	adlibWriteReg(bp2 + 0x80, value);

	value = sh->peekByteAtOffset(8, SEEK_CUR);
	adlibWriteReg(bp1 + 0xE0, value);

	value = sh->peekByteAtOffset(9, SEEK_CUR);
	adlibWriteReg(bp2 + 0xE0, value);

	value = sh->peekByteAtOffset(0xA, SEEK_CUR);
	adlibWriteReg(bpp0A + 0xC0, value);
}

void Adlib::adlibSetupChannel(uint16 bppA, uint8 bpp8, uint16 bpp6) {
	uint16 bp4;
	uint8 bp6;
	uint8 al = gArray9F[bpp8];
	uint16 dx = al;
	al = gArray11F[bpp8];
	uint16 bp2 = (al << 0x8) + dx;
	if (bpp6 != 0) {
		if (bpp6 < 0x80) {
			if (bpp8 < 0x7F) {
				bp6 = bpp8 + 1;
			} else {
				bp6 = 0x7F;
			}
			dx = gArray9F[bp6];
			bp4 = (gArray11F[bp6] << 0x8) + dx;
			// eax:edx = eax:edx * ebx:ecx
			uint64 product = bpp6 * (bp4 - bp2);
			// Right shift done by 0D7A proc
			product = product >> 0x7;
			bp2 = bp2 + product;
		} else {
			if (bpp8 > 0) {
			} else {
				bp6 = 0;
			}

			bp4 = (gArray11F[bp6] << 0x8) + gArray9F[bp6];
			// eax:edx = eax:edx * ebx:ecx
			uint64 product = bpp6 * (bp2 - bp4);
			// Right shift done by 0D7A proc
			product = product >> 0x7;
			bp2 = bp2 - product;
		}
	}

	// My version relies on the 16 bit value being correctly cast to 8 bit
	adlibWriteReg(bppA + 0xA0, bp2 & 0xFF);
	adlibWriteReg(bppA + 0xB0, (bp2 >> 0x8) | 0x20);
}

void Adlib::OnTimer() {

	// Original ISR always runs; skips event processing via _statusFlags check.
	if (shMem2250 == nullptr) {
		return;
	}

	_timerSubdivCounter++;

	if (_timerSubdivCounter >= _timerSubdivThreshold) {
		// Every nth time we execute this code
		_timerSubdivCounter = 0;

		_isTimerTick = true;
	} else {
		_isTimerTick = false;
	}
	if (!_isTimerTick) {
		// interrupt
	}
	_statusFlags = _statusFlags & 0xDF;
	if (_statusFlags & 0x2) {
		// [2258] & 2 was not zero
		_statusFlags |= 0x40;
	}

	if (!(_statusFlags & 0xC3)) {
		if (_nextEventTimer != 0) {
			_nextEventTimer--;
			// we don't need
			return;
		}

		for (;;) {
			uint8 current = shMem2250->peekByte();

			if (current & 0x80) {
				// The first bit of the read value was 0
				_currentMidiStatus = _currentEventStatus = shMem2250->peekByte();
				shMem2250 = adlibSeekStream(shMem2250, 1);
				_streamBytesConsumed++;
			}
			uint8 bp1;
			uint8 bp2;
			uint8 bp3 = _currentMidiStatus & 0x0F;
			uint8 bp6 = _currentMidiStatus;
			uint8 bp4 = shMem2250->peekByte();
			StreamHandler *bp10 = adlibSeekStream(shMem2250, 1);
			StreamHandler *bp12;
			uint8 bp5 = bp10->peekByte();

			if ((bp6 & 0xF0) == 0x90) {
				if (bp5 != 0) {
					shMem2250 = adlibSeekStream(shMem2250, 0x2);
					_streamBytesConsumed += 2;

					if (_numOplChannels == 0x09 || bp3 < 0x0B) {
						uint8 bp8 = 0;
						do {
							if (_numOplChannels <= bp8) {
								break;
							}
							if (_voiceAge[bp8] == 0) {
								uint8 v = _voiceMidiChannel[bp8];
								if (v == bp3) {
									uint8 v2 = _voiceNote[bp8];
									if (v2 == bp4) {
										break;
									}
								}
							}
							bp8++;
						} while (true);
						if (_numOplChannels == bp8) {
							uint16 bp0C = 0;
							bp8 = _numOplChannels;
							uint16 bp16 = _numOplChannels - 1;
							if (bp16 > 0) {
								uint16 bp0A = 0;
								do {
									if (_voiceAge[bp0A] != 0) {
										_voiceAge[bp0A]++;
									}
									if (_voiceAge[bp0A] > bp0C) {
										bp0C = _voiceAge[bp0A];
										bp8 = bp0A;
									}
									if (bp0A == bp16) {
										break;
									}
									// Original has this at the top but skips
									// it for the first round
									bp0A++;
								} while (true);
							}
							if (bp0C != 0) {
								_voiceAge[bp8] = 0;
								_voiceMidiChannel[bp8] = bp3;
								if (_channelPrograms[bp3] != _voiceInstrument[bp8]) {
									_voiceInstrument[bp8] = _channelPrograms[bp3];
									StreamHandler *shBP12 = adlibSeekStream(shMem2248, _voiceInstrument[bp8] << 0x4);
									adlibSetFrequency(bp8, shBP12);
								}
							}
						}
						if (_numOplChannels != bp8) {
							_voiceNote[bp8] = bp4;
							uint8 value = _channelPrograms[bp3];
							bp10 = adlibSeekStream(shMem2248, value << 0x4);
							uint8 temp = bp5;
							temp &= 0x7F;
							// dx
							uint8 temp2 = temp >> 0x1;
							temp = 0x3F;
							temp -= temp2;
							temp = temp >> 0x1;
							bp1 = temp;
							bp1 = bp1 >> 1;

							bp12 = adlibSeekStream(bp10, 0x2);

							temp = _masterVolume;
							// bx
							temp2 = temp;
							temp = bp12->peekByte();
							temp &= 0x3F;
							// dx
							uint8 temp3 = temp;
							temp = 0x3F;
							temp -= temp3;
							temp3 = temp;
							uint16 tempW = bp1;
							tempW *= temp3;
							tempW /= 0x3F;
							tempW += temp2;
							// dx word
							uint16 temp2W = tempW;
							// ax
							temp = bp12->peekByte() & 0x3F;
							bp2 = temp2W + temp;

							bp12 = adlibSeekStream(bp10, 0x3);
							temp = _masterVolume;
							// bx
							temp2 = temp;
							temp = bp12->peekByte();
							temp &= 0x3F;
							// dx
							temp3 = temp;
							temp = 0x3F;
							temp -= temp3;
							temp3 = temp;
							tempW = bp1;
							tempW *= temp3;
							tempW /= 0x3F;
							tempW += temp2;
							// dx word
							temp2W = tempW;
							// ax
							temp = bp12->peekByte() & 0x3F;
							bp1 = temp2W + temp;
							if (bp1 > 0x3F) {
								bp1 = 0x3F;
							}
							if (bp2 > 0x3F) {
								bp2 = 0x3F;
							}
							adlibWriteReg(bp8 + 0xb0, 0);

							uint8 result = adlibGetOperator(gArray96[bp8] + 0x40);
							adlibWriteReg(gArray96[bp8] + 0x40, (result & 0xC0) + bp1);
							result = adlibGetOperator(gArray8d[bp8] + 0x40);

							adlibWriteReg(gArray8d[bp8] + 0x40,
										  (result & 0xC0) + bp2);

							_channelPitchBend[bp3] = 0;
							adlibSetupChannel(bp8, bp4, _channelPitchBend[bp3]);
						}
					} else {
						// Percussion mode (channels >= 0xB)
						StreamHandler *shI = adlibSeekStream(shMem2248, _channelPrograms[bp3] << 0x4);
						uint8 opIdx = gArray57[bp3 - 0xB]; // operator index

						if (bp3 == 0xB) {
							// Bass drum: just set frequency
							adlibSetFrequency(gArray5C[bp3 - 0xB], shI);
						} else {
							// Other percussion: load instrument registers
							adlibWriteReg(opIdx + 0x20, shI->peekByte());
							StreamHandler *sh2 = adlibSeekStream(shI, 2);
							adlibWriteReg(opIdx + 0x40, sh2->peekByte());
							StreamHandler *sh4 = adlibSeekStream(shI, 4);
							adlibWriteReg(opIdx + 0x60, sh4->peekByte());
							StreamHandler *sh6 = adlibSeekStream(shI, 6);
							adlibWriteReg(opIdx + 0x80, sh6->peekByte());
							StreamHandler *sh8 = adlibSeekStream(shI, 8);
							adlibWriteReg(opIdx + 0xE0, sh8->peekByte());
						}

						// Volume calculation for percussion
						StreamHandler *sh3 = adlibSeekStream(shMem2248, (_channelPrograms[bp3] << 0x4) + 3);
						uint8 volIdx = ((sh3->peekByte() & 0x3F) >> 4) * 8 + (bp5 >> 4);
						if (volIdx < gArray37.size()) {
							bp1 = gArray37[volIdx] + _masterVolume;
							if (bp1 > 0x3F)
								bp1 = 0x3F;
						} else {
							bp1 = _masterVolume;
						}

						// Key off, set volume, trigger
						adlibWriteReg(gArray5C[bp3 - 0xB] + 0xB0, 0);
						uint8 reg40val = adlibGetOperator(opIdx + 0x40);
						adlibWriteReg(opIdx + 0x40, bp1 + (reg40val & 0xC0));
						adlibSetupChannel(gArray5C[bp3 - 0xB], bp4, 0);

						// Set percussion bit in register 0xBD
						uint8 bdVal = adlibGetOperator(0xBD);
						adlibWriteReg(0xBD, bdVal | (1 << (0xF - bp3)));
					}
				} else {
					// l0017_2097
					bp6 = 0x80;
				}
			}
			if ((bp6 & 0xF0) == 0x80) {
				shMem2250 = adlibSeekStream(shMem2250, 0x2);
				_streamBytesConsumed += 2;
				uint8 bp16 = _numOplChannels - 1;
				if (_numOplChannels > 0) {
					for (uint8 bp0A = 0; bp0A <= bp16; bp0A++) {
						if (_voiceAge[bp0A] != 0) {
							_voiceAge[bp0A]++;
						}
					}
				}
				if (_numOplChannels == 0x09 || bp3 < 0x0B) {
					uint8 bp8 = 0;
					while (_numOplChannels > bp8) {
						if (_voiceAge[bp8] == 0 && _voiceMidiChannel[bp8] == bp3 && _voiceNote[bp8] == bp4) {
							break;
						}
						bp8++;
					}
					if (_numOplChannels != bp8) {
						adlibSetupChannel(bp8, bp4, _channelPitchBend[bp3]);
						_voiceAge[bp8] = 1;
					}
				} else {
					// Percussion note-off: clear bit in register 0xBD
					uint8 bdVal = adlibGetOperator(0xBD);
					adlibWriteReg(0xBD, bdVal & ~(1 << (0xF - bp3)));
				}
			}
			if (((bp6 & 0xF0) == 0xE0) || (bp6 & 0xF0) == 0xA0) {
				shMem2250 = adlibSeekStream(shMem2250, 0x2);
				_streamBytesConsumed += 0x2;
			}
			if ((bp6 & 0xF0) == 0xB0) { // Scope ends 231E
				shMem2250 = adlibSeekStream(shMem2250, 0x2);
				_streamBytesConsumed += 0x2;

				// Big if-else that ends at 231E
				if (bp4 == 0x66) {
					_loopCount = bp5;
					_statusFlags = _statusFlags | 0x20;
				} else if (bp4 == 0x67) {
					if (bp5 != 0) {
						_numOplChannels = 6;
						adlibWriteReg(0xBD, 0x20);
					} else {
						_numOplChannels = 9;
						adlibWriteReg(0xBD, 0);
					}
					//				// TODO: Continue from here
					//				if (bp5 != 0) {
					//					_numOplChannels = 0x6;
					//					adlibWriteReg(0xBD, 0x20);
					//				} else {
					//					_numOplChannels = 0x9;
					//					adlibWriteReg(0xBD, 0);
				} else if (bp4 == 0x69) {
					{
						uint8 bv = (uint8)(-(int8)bp5);
						_channelPitchBend[bp3] = bv;
						for (uint8 i = 0; i < _numOplChannels; i++) {
							if (_voiceMidiChannel[i] == bp3 && _voiceAge[i] == 0)
								adlibSetupChannel(i, _voiceNote[i], bv);
						}
					}
					//					bp5 = -bp5;
					//					_channelPitchBend[bp3] = bp5;
					//					uint8 bp16 = _numOplChannels - 1;
					//					if (0 <= bp16) {
					//						for (uint8 bp8 = 0; bp8 != bp16; bp8++) {
					//							if (_voiceMidiChannel[bp8] != bp3) {
					//								continue;
					//							}
					//							if (_voiceAge[bp8] != 0) {
					//								continue;
					//							}

					//							adlibSetupChannel(bp8, _voiceNote[bp8], bp5);
					//						}
					//					}
				} else if (bp4 == 0x68) {
					{
						_channelPitchBend[bp3] = bp5;
						for (uint8 i = 0; i < _numOplChannels; i++) {
							if (_voiceMidiChannel[i] == bp3 && _voiceAge[i] == 0)
								adlibSetupChannel(i, _voiceNote[i], bp5);
						}
					}
					//					_channelPitchBend[bp3] = bp5;
					//					uint16 bp16 = _numOplChannels - 1;
					//					if (0 <= bp16) {
					//						for (uint8 bp8 = 0; bp8 != bp16; bp8++) {
					//							if (_voiceMidiChannel[bp8] != bp3) {
					//								continue;
					//							}
					//							if (_voiceAge[bp8] != 0) {
					//								continue;
					//							}
					//							adlibSetupChannel(bp8, _voiceNote[bp8], bp5);
					//						}
					//					}
					//				}
					//
				}
			}
			if ((bp6 & 0xF0) == 0xC0) {
				shMem2250 = adlibSeekStream(shMem2250, 0x1);
				_streamBytesConsumed++;
				_channelPrograms[bp3] = bp4;
			}
			if ((bp6 & 0xF0) == 0xD0) {
				shMem2250 = adlibSeekStream(shMem2250, 0x1);
				_streamBytesConsumed++;
				//		Macs2::StreamHandler *sh2252;
				//		Macs2::StreamHandler *sh225A;
				//		Macs2::StreamHandler *shResult = adlibSeekStream(sh2252, 0x1);
				//		sh2252 = shResult;
				//		// TODO: Check if this is the right way to handle the plus operation
				//		sh225A->seek(1, SEEK_CUR);
			}
			if ((bp6 & 0xF0) == 0xF0) {
				if (bp4 == 0x2F) {
					shMem2250 = shMem2244;
					_streamBytesConsumed = 0;
					_masterVolume = 0;
					_loopCount = 0;
					_playbackReady = 1;
					adlibReadDeltaTime();
				} else {
					shMem2250 = adlibSeekStream(shMem2250, 0x1);
					_streamBytesConsumed++;
				}
				//		if (bp4 == 0x2F) {
				//			Macs2::StreamHandler *sh2244;
				//			Macs2::StreamHandler *sh2250;
				//			sh2250 = sh2244;
				//			// TODO: Setting 225C and 225A to 0 - ?
				//			// TODO: Setting 2259 and 2242 to 1 - Probably understood these wrong
				//			adlibReadDeltaTime();
				//		} else {
				//		}
			} else {
				adlibReadDeltaTime();
			}

			// this inequality by using the msw and lsw
			if (_nextEventTimer > 0x0FFF) {
				shMem2250 = shMem2244;
				_streamBytesConsumed = 0;
				_loopCount = 0;
				_playbackReady = 1;
				adlibReadDeltaTime();
			}

			if (_nextEventTimer != 0) {
				break;
			}
		}
	} else {
		// This is the jump target from 1B00 from before the big loop
		if ((_statusFlags & 0xC2) != 0) {
			_statusFlags &= ~0xC2;
			// Original calls sbFillBuffer() here - not needed for OPL emulation
			// Func1A74();
		}
		adlibSetInstrument();
	}

	// l0017_2422

	if (_isTimerTick == 0) {
		// l0017_243F
		// Original sends EOI (out 0x20, 0x20) if not a timer tick
	}
	// Just epilogue and interrupt return
}

StreamHandler *Adlib::adlibSeekStream(StreamHandler *inHandler, uint16 seekDelta) {
	StreamHandler *result = new StreamHandler(*inHandler);
	uint16 pos = result->pos();
	if (seekDelta > 0xFFF8) {
		// the actual game
		pos &= 0xF;
	}
	// l0017_19EA
	pos += seekDelta;
	result->seek(pos, SEEK_SET);

	return result;
}

void Adlib::adlibPlaySong(StreamHandler *song) {
	StreamHandler *sh = adlibSeekStream(song, 0x6);
	uint16 delta = sh->peekWord();
	shMem2248 = adlibSeekStream(song, delta);
	sh = adlibSeekStream(song, 0x8);
	delta = sh->peekWord();
	shMem2244 = adlibSeekStream(song, delta);
	sh = adlibSeekStream(song, 0x24);
	_currentEventStatusHi = sh->peekWord();
	sh = adlibSeekStream(song, 0xC);
	_timerFrequency = sh->peekWord();
	adlibTickHandler();
}

void Adlib::adlibReadDeltaTime() {

	_nextEventTimer = 0;
	uint8 bp1;
	uint8 continueCondition;
	do {
		bp1 = shMem2250->peekByte();

		_nextEventTimer = _nextEventTimer << 7;
		_nextEventTimer += bp1 & 0x7F;
		shMem2250 = adlibSeekStream(shMem2250, 1);
		_streamBytesConsumed++;
		continueCondition = bp1 & 0x80;
	} while (continueCondition != 0);
}

void Adlib::adlibProcessEvent(uint8 blend_param, uint8 index, uint8 reg_base) {
	// Initial value calculation
	uint16 base_value = (gArray11F[index] << 8) | gArray9F[index];
	if (blend_param != 0) {
		if (blend_param < 0x80) { // Forward blend case
			// Calculate next index with clamping
			uint8 next_idx = (index < 0x7F) ? index + 1 : 0x7F;
			uint16 next_value = (gArray11F[next_idx] << 8) | gArray9F[next_idx];
			int16 delta = next_value - base_value;
			base_value += static_cast<uint16>((delta * blend_param) / 7);
		} else { // Reverse blend case
			// Calculate previous index with clamping
			uint8 prev_idx = (index > 0) ? index - 1 : 0;
			uint16 prev_value = (gArray11F[prev_idx] << 8) | gArray9F[prev_idx];

			// Reverse interpolation
			int16 delta = base_value - prev_value;
			base_value -= static_cast<uint16>((delta * blend_param) / 7);
		}
	}

	// Write results to sound chip registers
	uint8 low_reg = 0xA0 + reg_base;  // Low byte register
	uint8 high_reg = 0xB0 + reg_base; // High byte register

	// Write low byte (raw value)
	adlibWriteReg(low_reg, static_cast<uint8>(base_value & 0xFF));

	// Write high byte (masked to clear bit 5)
	adlibWriteReg(high_reg, static_cast<uint8>((base_value >> 8) & 0xDF));
}

void Adlib::Init() {
	_opl = OPL::Config::create();
	_opl->init();

	gArray229C.resize(256);

	_channelPrograms.resize(0x10);
	_channelPitchBend.resize(0x10);

	_voiceAge.resize(0x9);
	_voiceMidiChannel.resize(0x9);
	_voiceInstrument.resize(0x9);
	_voiceNote.resize(0x9);

	_opl->start(new Common::Functor0Mem<void, Adlib>(this, &Adlib::OnTimer), CALLBACKS_PER_SECOND);

	// adlibReadDeltaTime();

	/* _opl->writeReg(0x20, 0x01);
	_opl->writeReg(0x40, 0x10);
	_opl->writeReg(0x60, 0xF0);
	_opl->writeReg(0x80, 0x77);
	_opl->writeReg(0xA0, 0x98);
	_opl->writeReg(0x23, 0x01);
	_opl->writeReg(0x43, 0x00);
	_opl->writeReg(0x63, 0xF0);
	_opl->writeReg(0x83, 0x77);
	_opl->writeReg(0xB0, 0x31); */

	// return;

	// Stop any previous playback before reinitializing
	adlibStopMusic();
	// adlibTickHandler();
	// adlibSetFrequency(0, 0);

	// Test tone registers (from Adlib programming guide, not from game):
	// Probably this https://bespin.org/~qz/pc-gpe/adlib.txt
	adlibWriteRegr(0x92, 0x20);
	adlibWriteRegr(0x01, 0x23);
	adlibWriteRegr(0x52, 0x40);
	adlibWriteRegr(0x00, 0x43);
	adlibWriteRegr(0xdc, 0x60);
	adlibWriteRegr(0xf5, 0x63);
	adlibWriteRegr(0x23, 0x80);
	adlibWriteRegr(0x13, 0x83);
	adlibWriteRegr(0x80, 0xe0);
	adlibWriteRegr(0x81, 0xe3);
	adlibWriteRegr(0x0c, 0xc0);
	adlibWriteRegr(0x00, 0xb0);
	adlibWriteRegr(0x04, 0x43);
	adlibWriteRegr(0x54, 0x40);
	adlibWriteRegr(0x98, 0xa0);
	adlibWriteRegr(0x29, 0xb0);

	// End of test tone setup
	// Trying to hardcode the delta
	/*  adlibWriteReg(0xb0, 0x00);
	adlibWriteReg(0x43, 0x04);
	adlibWriteReg(0x40, 0x54);

	// Just for the hell of it, try to send the first note on to see if it works
	adlibWriteReg(0xa0, 0x98);
	adlibWriteReg(0xb0, 0x29);
	_opl->writeReg(0xB0, 0x31); */
}

void Adlib::Deinit() {
	_opl->stop();
	delete _activeSongStream;
	_activeSongStream = nullptr;
	delete _opl;
}

void Adlib::SetSong(Macs2::StreamHandler *sh) {
	shMem2250 = sh;
	adlibPlaySong(shMem2250);
}

void Adlib::PlaySongData(const Common::Array<uint8> &data) {
	delete _activeSongStream;
	_activeSongData = data;
	_activeSongStream = new StreamHandler(&_activeSongData);
	SetSong(_activeSongStream);
}

void Adlib::StopMusic() {
	shMem2250 = nullptr;
	delete _activeSongStream;
	_activeSongStream = nullptr;
	_activeSongData.clear();
	SetVolume(0);
}

void Adlib::SetVolume(uint16 volume) {
	if (volume > 100)
		volume = 100;

	// Volume is handled by the mixer
}

void Adlib::ReadDataFromExecutable(Common::MemoryReadStream *fileStream) {
	constexpr uint32 size = 255;
	gArray69.resize(size);
	LoadData(fileStream, 0x0001B669, size, gArray69.data());

	gArray8d.resize(size);
	LoadData(fileStream, 0x0001B68D, size, gArray8d.data());

	gArray96.resize(size);
	LoadData(fileStream, 0x0001B696, size, gArray96.data());

	gArray9F.resize(size);
	LoadData(fileStream, 0x0001B69F, size, gArray9F.data());

	gArray11F.resize(size);
	LoadData(fileStream, 0x0001B71F, size, gArray11F.data());

	// Percussion lookup tables (hardcoded in data segment)
	gArray37 = {28, 25, 23, 18, 14, 11, 8, 2, 50, 42, 37, 35, 34, 32, 30, 2, 55, 50, 49, 48, 45, 43, 40, 2, 60, 60, 58, 56, 54, 52, 50, 2};
	// gArray4c[ch] for ch>=0xB == gArray57[ch-0xB], gArray51[ch] for ch>=0xB == gArray5C[ch-0xB]
	gArray57 = {19, 20, 18, 21, 17}; // percussion alternate channel
	gArray5C = {6, 7, 8, 8, 7};      // percussion frequency channel
}

void Adlib::LoadData(Common::MemoryReadStream *fileStream, int64 pos, uint16 size, void *target) {
	fileStream->seek(pos, SEEK_SET);
	fileStream->read(target, size);
}

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
