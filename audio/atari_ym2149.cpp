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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "audio/atari_ym2149.h"

#include <mint/falcon.h>
#include <mint/osbind.h>

namespace Audio {

YM2149Atari::YM2149Atari() {
	for (int r = 0; r < 14; r++) {
		_savedRegs[r] = Giaccess(0, r);
	}
}

YM2149Atari::~YM2149Atari() {
	for (int r = 0; r < 14; r++) {
		Giaccess(_savedRegs[r], r | 0x80);
	}
}

bool YM2149Atari::init() {
	// route both PSG channels to the ADC instead of the microphone
	Soundcmd(ADCINPUT, ADCLT|ADCRT);
	// enable and mix both sources (ADC and connection matrix) to the output
	Soundcmd(ADDERIN, MATIN|ADCIN);
	return true;
}

void YM2149Atari::reset() {
	// silence channels/envelope, but keep ports as outputs
	writeReg(7, 0xFF);	// all tone+noise off, PA/PB output
	writeReg(8, 0);		// volume A
	writeReg(9, 0);		// volume B
	writeReg(10, 0);	// volume C
	// skip reg 13 (envelope shape) — writing it retriggers the envelope
}

void YM2149Atari::writeReg(int r, uint8 v) {
	if (r == 7)
		v |= 0xC0;	// keep Port A/B as outputs
	Giaccess(v, r | 0x80);
}

} // End of namespace Audio
