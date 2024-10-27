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

#include "audio/adlib_ms.h"

#include "common/debug.h"

bool OplInstrumentOperatorDefinition::isEmpty() {
	return freqMultMisc == 0 && level == 0 && decayAttack == 0 &&
		   releaseSustain == 0 && waveformSelect == 0;
}

bool OplInstrumentDefinition::isEmpty() {
	if (rhythmType != RHYTHM_TYPE_UNDEFINED) {
		return operator0.isEmpty() &&
			(rhythmType != RHYTHM_TYPE_BASS_DRUM || operator1.isEmpty());
	} else if (!fourOperator) {
		return operator0.isEmpty() && operator1.isEmpty();
	} else {
		return operator0.isEmpty() && operator1.isEmpty() &&
			operator2.isEmpty() && operator3.isEmpty();
	}
}

uint8 OplInstrumentDefinition::getNumberOfOperators() {
	if (rhythmType == RHYTHM_TYPE_UNDEFINED) {
		return fourOperator ? 4 : 2;
	} else {
		// The bass drum rhythm instrument uses 2 operators; the others use
		// only 1.
		return rhythmType == RHYTHM_TYPE_BASS_DRUM ? 2 : 1;
	}
}

OplInstrumentOperatorDefinition &OplInstrumentDefinition::getOperatorDefinition(uint8 operatorNum) {
	assert((!fourOperator && operatorNum < 2) || operatorNum < 4);

	switch (operatorNum) {
	case 0:
		return operator0;
	case 1:
		return operator1;
	case 2:
		return operator2;
	case 3:
		return operator3;
	default:
		// Should not happen.
		return operator0;
	}
}

void AdLibBnkInstrumentOperatorDefinition::toOplInstrumentOperatorDefinition(OplInstrumentOperatorDefinition &operatorDef, uint8 waveformSelect) {
	// Combine the separate fields of the BNK format into complete register values.
	operatorDef.freqMultMisc = frequencyMultiplier | (keyScalingRate == 0 ? 0 : 0x10) |
		(envelopeGainType == 0 ? 0 : 0x20) | (vibrato == 0 ? 0 : 0x40) | (amplitudeModulation == 0 ? 0 : 0x80);
	operatorDef.level = level | (keyScalingLevel << 6);
	operatorDef.decayAttack = decay | (attack << 4);
	operatorDef.releaseSustain = release | (sustain << 4);
	operatorDef.waveformSelect = waveformSelect;
}

void AdLibBnkInstrumentDefinition::toOplInstrumentDefinition(OplInstrumentDefinition &instrumentDef) {
	instrumentDef.fourOperator = false;

	operator0.toOplInstrumentOperatorDefinition(instrumentDef.operator0, waveformSelect0);
	operator1.toOplInstrumentOperatorDefinition(instrumentDef.operator1, waveformSelect1);

	instrumentDef.connectionFeedback0 = (operator0.connection == 0 ? 1 : 0) | (operator0.feedback << 1);
	// BNK does not support 4 operator.
	instrumentDef.connectionFeedback1 = 0;
	
	// TODO Figure out if this is the same as rhythmVoiceNumber
	instrumentDef.rhythmNote = 0;
	instrumentDef.rhythmType = RHYTHM_TYPE_UNDEFINED;
}

void AdLibIbkInstrumentDefinition::toOplInstrumentDefinition(OplInstrumentDefinition &instrumentDef) {
	instrumentDef.fourOperator = false;

	instrumentDef.operator0.freqMultMisc = o0FreqMultMisc;
	instrumentDef.operator0.level = o0Level;
	instrumentDef.operator0.decayAttack = o0DecayAttack;
	instrumentDef.operator0.releaseSustain = o0ReleaseSustain;
	instrumentDef.operator0.waveformSelect = o0WaveformSelect;

	instrumentDef.operator1.freqMultMisc = o1FreqMultMisc;
	instrumentDef.operator1.level = o1Level;
	instrumentDef.operator1.decayAttack = o1DecayAttack;
	instrumentDef.operator1.releaseSustain = o1ReleaseSustain;
	instrumentDef.operator1.waveformSelect = o1WaveformSelect;

	instrumentDef.connectionFeedback0 = connectionFeedback;

	instrumentDef.rhythmNote = rhythmNote;
	OplInstrumentRhythmType convRhythmType;
	switch (rhythmType) {
		case 6:
			convRhythmType = RHYTHM_TYPE_BASS_DRUM;
			break;
		case 7:
			convRhythmType = RHYTHM_TYPE_SNARE_DRUM;
			break;
		case 8:
			convRhythmType = RHYTHM_TYPE_TOM_TOM;
			break;
		case 9:
			convRhythmType = RHYTHM_TYPE_CYMBAL;
			break;
		case 10:
			convRhythmType = RHYTHM_TYPE_HI_HAT;
			break;
		case 0:
		default:
			convRhythmType = RHYTHM_TYPE_UNDEFINED;
			break;
	}
	instrumentDef.rhythmType = convRhythmType;
	// TODO Add support for transpose
}

// These are the melodic instrument definitions used by the Win95 SB16 driver.
OplInstrumentDefinition MidiDriver_ADLIB_Multisource::OPL_INSTRUMENT_BANK[128] = {
	// 0x00
	{ false, { 0x01, 0x8F, 0xF2, 0xF4, 0x00 }, { 0x01, 0x06, 0xF2, 0xF7, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x4B, 0xF2, 0xF4, 0x00 }, { 0x01, 0x00, 0xF2, 0xF7, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x49, 0xF2, 0xF4, 0x00 }, { 0x01, 0x00, 0xF2, 0xF6, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x81, 0x12, 0xF2, 0xF7, 0x00 }, { 0x41, 0x00, 0xF2, 0xF7, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x57, 0xF1, 0xF7, 0x00 }, { 0x01, 0x00, 0xF2, 0xF7, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x93, 0xF1, 0xF7, 0x00 }, { 0x01, 0x00, 0xF2, 0xF7, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x80, 0xA1, 0xF2, 0x00 }, { 0x16, 0x0E, 0xF2, 0xF5, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x92, 0xC2, 0xF8, 0x00 }, { 0x01, 0x00, 0xC2, 0xF8, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x08
	{ false, { 0x0C, 0x5C, 0xF6, 0xF4, 0x00 }, { 0x81, 0x00, 0xF3, 0xF5, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x07, 0x97, 0xF3, 0xF2, 0x00 }, { 0x11, 0x80, 0xF2, 0xF1, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x17, 0x21, 0x54, 0xF4, 0x00 }, { 0x01, 0x00, 0xF4, 0xF4, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x98, 0x62, 0xF3, 0xF6, 0x00 }, { 0x81, 0x00, 0xF2, 0xF6, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x18, 0x23, 0xF6, 0xF6, 0x00 }, { 0x01, 0x00, 0xE7, 0xF7, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x15, 0x91, 0xF6, 0xF6, 0x00 }, { 0x01, 0x00, 0xF6, 0xF6, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x45, 0x59, 0xD3, 0xF3, 0x00 }, { 0x81, 0x80, 0xA3, 0xF3, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x03, 0x49, 0x75, 0xF5, 0x01 }, { 0x81, 0x80, 0xB5, 0xF5, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x10
	{ false, { 0x71, 0x92, 0xF6, 0x14, 0x00 }, { 0x31, 0x00, 0xF1, 0x07, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x72, 0x14, 0xC7, 0x58, 0x00 }, { 0x30, 0x00, 0xC7, 0x08, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x70, 0x44, 0xAA, 0x18, 0x00 }, { 0xB1, 0x00, 0x8A, 0x08, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x23, 0x93, 0x97, 0x23, 0x01 }, { 0xB1, 0x00, 0x55, 0x14, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x61, 0x13, 0x97, 0x04, 0x01 }, { 0xB1, 0x80, 0x55, 0x04, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x24, 0x48, 0x98, 0x2A, 0x01 }, { 0xB1, 0x00, 0x46, 0x1A, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x61, 0x13, 0x91, 0x06, 0x01 }, { 0x21, 0x00, 0x61, 0x07, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x13, 0x71, 0x06, 0x00 }, { 0xA1, 0x89, 0x61, 0x07, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x18
	{ false, { 0x02, 0x9C, 0xF3, 0x94, 0x01 }, { 0x41, 0x80, 0xF3, 0xC8, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x03, 0x54, 0xF3, 0x9A, 0x01 }, { 0x11, 0x00, 0xF1, 0xE7, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x23, 0x5F, 0xF1, 0x3A, 0x00 }, { 0x21, 0x00, 0xF2, 0xF8, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x03, 0x87, 0xF6, 0x22, 0x01 }, { 0x21, 0x80, 0xF3, 0xF8, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x03, 0x47, 0xF9, 0x54, 0x00 }, { 0x21, 0x00, 0xF6, 0x3A, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x23, 0x4A, 0x91, 0x41, 0x01 }, { 0x21, 0x05, 0x84, 0x19, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x23, 0x4A, 0x95, 0x19, 0x01 }, { 0x21, 0x00, 0x94, 0x19, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x09, 0xA1, 0x20, 0x4F, 0x00 }, { 0x84, 0x80, 0xD1, 0xF8, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x20
	{ false, { 0x21, 0x1E, 0x94, 0x06, 0x00 }, { 0xA2, 0x00, 0xC3, 0xA6, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x12, 0xF1, 0x28, 0x00 }, { 0x31, 0x00, 0xF1, 0x18, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x8D, 0xF1, 0xE8, 0x00 }, { 0x31, 0x00, 0xF1, 0x78, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x5B, 0x51, 0x28, 0x00 }, { 0x32, 0x00, 0x71, 0x48, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x8B, 0xA1, 0x9A, 0x00 }, { 0x21, 0x40, 0xF2, 0xDF, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x8B, 0xA2, 0x16, 0x00 }, { 0x21, 0x08, 0xA1, 0xDF, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x8B, 0xF4, 0xE8, 0x00 }, { 0x31, 0x00, 0xF1, 0x78, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x12, 0xF1, 0x28, 0x00 }, { 0x31, 0x00, 0xF1, 0x18, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x28
	{ false, { 0x31, 0x15, 0xDD, 0x13, 0x01 }, { 0x21, 0x00, 0x56, 0x26, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x16, 0xDD, 0x13, 0x01 }, { 0x21, 0x00, 0x66, 0x06, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x71, 0x49, 0xD1, 0x1C, 0x01 }, { 0x31, 0x00, 0x61, 0x0C, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x4D, 0x71, 0x12, 0x01 }, { 0x23, 0x80, 0x72, 0x06, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xF1, 0x40, 0xF1, 0x21, 0x01 }, { 0xE1, 0x00, 0x6F, 0x16, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x02, 0x1A, 0xF5, 0x75, 0x01 }, { 0x01, 0x80, 0x85, 0x35, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x02, 0x1D, 0xF5, 0x75, 0x01 }, { 0x01, 0x80, 0xF3, 0xF4, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x10, 0x41, 0xF5, 0x05, 0x01 }, { 0x11, 0x00, 0xF2, 0xC3, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x30
	{ false, { 0x21, 0x9B, 0xB1, 0x25, 0x01 }, { 0xA2, 0x01, 0x72, 0x08, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xA1, 0x98, 0x7F, 0x03, 0x01 }, { 0x21, 0x00, 0x3F, 0x07, 0x01 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xA1, 0x93, 0xC1, 0x12, 0x00 }, { 0x61, 0x00, 0x4F, 0x05, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x18, 0xC1, 0x22, 0x00 }, { 0x61, 0x00, 0x4F, 0x05, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x5B, 0xF4, 0x15, 0x00 }, { 0x72, 0x83, 0x8A, 0x05, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xA1, 0x90, 0x74, 0x39, 0x00 }, { 0x61, 0x00, 0x71, 0x67, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x71, 0x57, 0x54, 0x05, 0x00 }, { 0x72, 0x00, 0x7A, 0x05, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x90, 0x00, 0x54, 0x63, 0x00 }, { 0x41, 0x00, 0xA5, 0x45, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x38
	{ false, { 0x21, 0x92, 0x85, 0x17, 0x00 }, { 0x21, 0x01, 0x8F, 0x09, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x94, 0x75, 0x17, 0x00 }, { 0x21, 0x05, 0x8F, 0x09, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x94, 0x76, 0x15, 0x00 }, { 0x61, 0x00, 0x82, 0x37, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x43, 0x9E, 0x17, 0x01 }, { 0x21, 0x00, 0x62, 0x2C, 0x01 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x9B, 0x61, 0x6A, 0x00 }, { 0x21, 0x00, 0x7F, 0x0A, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x61, 0x8A, 0x75, 0x1F, 0x00 }, { 0x22, 0x06, 0x74, 0x0F, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xA1, 0x86, 0x72, 0x55, 0x01 }, { 0x21, 0x83, 0x71, 0x18, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x4D, 0x54, 0x3C, 0x00 }, { 0x21, 0x00, 0xA6, 0x1C, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x40
	{ false, { 0x31, 0x8F, 0x93, 0x02, 0x01 }, { 0x61, 0x00, 0x72, 0x0B, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x8E, 0x93, 0x03, 0x01 }, { 0x61, 0x00, 0x72, 0x09, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x91, 0x93, 0x03, 0x01 }, { 0x61, 0x00, 0x82, 0x09, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x8E, 0x93, 0x0F, 0x01 }, { 0x61, 0x00, 0x72, 0x0F, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x4B, 0xAA, 0x16, 0x01 }, { 0x21, 0x00, 0x8F, 0x0A, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x90, 0x7E, 0x17, 0x01 }, { 0x21, 0x00, 0x8B, 0x0C, 0x01 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x81, 0x75, 0x19, 0x01 }, { 0x32, 0x00, 0x61, 0x19, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x32, 0x90, 0x9B, 0x21, 0x00 }, { 0x21, 0x00, 0x72, 0x17, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x48
	{ false, { 0xE1, 0x1F, 0x85, 0x5F, 0x00 }, { 0xE1, 0x00, 0x65, 0x1A, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xE1, 0x46, 0x88, 0x5F, 0x00 }, { 0xE1, 0x00, 0x65, 0x1A, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xA1, 0x9C, 0x75, 0x1F, 0x00 }, { 0x21, 0x00, 0x75, 0x0A, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x8B, 0x84, 0x58, 0x00 }, { 0x21, 0x00, 0x65, 0x1A, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xE1, 0x4C, 0x66, 0x56, 0x00 }, { 0xA1, 0x00, 0x65, 0x26, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x62, 0xCB, 0x76, 0x46, 0x00 }, { 0xA1, 0x00, 0x55, 0x36, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x62, 0x99, 0x57, 0x07, 0x00 }, { 0xA1, 0x00, 0x56, 0x07, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3B, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x62, 0x93, 0x77, 0x07, 0x00 }, { 0xA1, 0x00, 0x76, 0x07, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3B, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x50
	{ false, { 0x22, 0x59, 0xFF, 0x03, 0x02 }, { 0x21, 0x00, 0xFF, 0x0F, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x0E, 0xFF, 0x0F, 0x01 }, { 0x21, 0x00, 0xFF, 0x0F, 0x01 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x22, 0x46, 0x86, 0x55, 0x00 }, { 0x21, 0x80, 0x64, 0x18, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x45, 0x66, 0x12, 0x00 }, { 0xA1, 0x00, 0x96, 0x0A, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x8B, 0x92, 0x2A, 0x01 }, { 0x22, 0x00, 0x91, 0x2A, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xA2, 0x9E, 0xDF, 0x05, 0x00 }, { 0x61, 0x40, 0x6F, 0x07, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x20, 0x1A, 0xEF, 0x01, 0x00 }, { 0x60, 0x00, 0x8F, 0x06, 0x02 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x8F, 0xF1, 0x29, 0x00 }, { 0x21, 0x80, 0xF4, 0x09, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x58
	{ false, { 0x77, 0xA5, 0x53, 0x94, 0x00 }, { 0xA1, 0x00, 0xA0, 0x05, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x61, 0x1F, 0xA8, 0x11, 0x00 }, { 0xB1, 0x80, 0x25, 0x03, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x61, 0x17, 0x91, 0x34, 0x00 }, { 0x61, 0x00, 0x55, 0x16, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x71, 0x5D, 0x54, 0x01, 0x00 }, { 0x72, 0x00, 0x6A, 0x03, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x97, 0x21, 0x43, 0x00 }, { 0xA2, 0x00, 0x42, 0x35, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xA1, 0x1C, 0xA1, 0x77, 0x01 }, { 0x21, 0x00, 0x31, 0x47, 0x01 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x89, 0x11, 0x33, 0x00 }, { 0x61, 0x03, 0x42, 0x25, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xA1, 0x15, 0x11, 0x47, 0x01 }, { 0x21, 0x00, 0xCF, 0x07, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x60
	{ false, { 0x3A, 0xCE, 0xF8, 0xF6, 0x00 }, { 0x51, 0x00, 0x86, 0x02, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x32, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x15, 0x21, 0x23, 0x01 }, { 0x21, 0x00, 0x41, 0x13, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x06, 0x5B, 0x74, 0x95, 0x00 }, { 0x01, 0x00, 0xA5, 0x72, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x22, 0x92, 0xB1, 0x81, 0x00 }, { 0x61, 0x83, 0xF2, 0x26, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x41, 0x4D, 0xF1, 0x51, 0x01 }, { 0x42, 0x00, 0xF2, 0xF5, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x61, 0x94, 0x11, 0x51, 0x01 }, { 0xA3, 0x80, 0x11, 0x13, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x61, 0x8C, 0x11, 0x31, 0x00 }, { 0xA1, 0x80, 0x1D, 0x03, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xA4, 0x4C, 0xF3, 0x73, 0x01 }, { 0x61, 0x00, 0x81, 0x23, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x68
	{ false, { 0x02, 0x85, 0xD2, 0x53, 0x00 }, { 0x07, 0x03, 0xF2, 0xF6, 0x01 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x11, 0x0C, 0xA3, 0x11, 0x01 }, { 0x13, 0x80, 0xA2, 0xE5, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x11, 0x06, 0xF6, 0x41, 0x01 }, { 0x11, 0x00, 0xF2, 0xE6, 0x02 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x93, 0x91, 0xD4, 0x32, 0x00 }, { 0x91, 0x00, 0xEB, 0x11, 0x01 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x04, 0x4F, 0xFA, 0x56, 0x00 }, { 0x01, 0x00, 0xC2, 0x05, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x21, 0x49, 0x7C, 0x20, 0x00 }, { 0x22, 0x00, 0x6F, 0x0C, 0x01 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x31, 0x85, 0xDD, 0x33, 0x01 }, { 0x21, 0x00, 0x56, 0x16, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x20, 0x04, 0xDA, 0x05, 0x02 }, { 0x21, 0x81, 0x8F, 0x0B, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x70
	{ false, { 0x05, 0x6A, 0xF1, 0xE5, 0x00 }, { 0x03, 0x80, 0xC3, 0xE5, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x07, 0x15, 0xEC, 0x26, 0x00 }, { 0x02, 0x00, 0xF8, 0x16, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x05, 0x9D, 0x67, 0x35, 0x00 }, { 0x01, 0x00, 0xDF, 0x05, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x18, 0x96, 0xFA, 0x28, 0x00 }, { 0x12, 0x00, 0xF8, 0xE5, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x10, 0x86, 0xA8, 0x07, 0x00 }, { 0x00, 0x03, 0xFA, 0x03, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x11, 0x41, 0xF8, 0x47, 0x02 }, { 0x10, 0x03, 0xF3, 0x03, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x8E, 0xF1, 0x06, 0x02 }, { 0x10, 0x00, 0xF3, 0x02, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x00, 0x1F, 0x00, 0x00 }, { 0xC0, 0x00, 0x1F, 0xFF, 0x03 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x78
	{ false, { 0x06, 0x80, 0xF8, 0x24, 0x00 }, { 0x03, 0x88, 0x56, 0x84, 0x02 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x00, 0xF8, 0x00, 0x00 }, { 0xD0, 0x05, 0x34, 0x04, 0x03 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x00, 0xF6, 0x00, 0x00 }, { 0xC0, 0x00, 0x1F, 0x02, 0x03 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xD5, 0x95, 0x37, 0xA3, 0x00 }, { 0xDA, 0x40, 0x56, 0x37, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x35, 0x5C, 0xB2, 0x61, 0x02 }, { 0x14, 0x08, 0xF4, 0x15, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x00, 0xF6, 0x00, 0x00 }, { 0xD0, 0x00, 0x4F, 0xF5, 0x03 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x26, 0x00, 0xFF, 0x01, 0x00 }, { 0xE4, 0x00, 0x12, 0x16, 0x01 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0xF3, 0xF0, 0x00 }, { 0x00, 0x00, 0xF6, 0xC9, 0x02 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED }
};

// These are the rhythm instrument definitions used by the Win95 SB16 driver.
OplInstrumentDefinition MidiDriver_ADLIB_Multisource::OPL_RHYTHM_BANK[62] = {
	// GS percussion start
	// 0x1B
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	// 0x20
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED },

	// GM percussion start
	// 0x23
	{ false, { 0x10, 0x44, 0xF8, 0x77, 0x02 }, { 0x11, 0x00, 0xF3, 0x06, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x23, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x10, 0x44, 0xF8, 0x77, 0x02 }, { 0x11, 0x00, 0xF3, 0x06, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x23, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x02, 0x07, 0xF9, 0xFF, 0x00 }, { 0x11, 0x00, 0xF8, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x34, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0xFC, 0x05, 0x02 }, { 0x00, 0x00, 0xFA, 0x17, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x30, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x02, 0xFF, 0x07, 0x00 }, { 0x01, 0x00, 0xFF, 0x08, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x3A, RHYTHM_TYPE_UNDEFINED },
	// 0x28
	{ false, { 0x00, 0x00, 0xFC, 0x05, 0x02 }, { 0x00, 0x00, 0xFA, 0x17, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x3C, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0xF6, 0x0C, 0x00 }, { 0x00, 0x00, 0xF6, 0x06, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x2F, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0C, 0x00, 0xF6, 0x08, 0x00 }, { 0x12, 0x00, 0xFB, 0x47, 0x02 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x2B, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0xF6, 0x0C, 0x00 }, { 0x00, 0x00, 0xF6, 0x06, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x31, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0C, 0x00, 0xF6, 0x08, 0x00 }, { 0x12, 0x05, 0x7B, 0x47, 0x02 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x2B, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0xF6, 0x0C, 0x00 }, { 0x00, 0x00, 0xF6, 0x06, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x33, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0C, 0x00, 0xF6, 0x02, 0x00 }, { 0x12, 0x00, 0xCB, 0x43, 0x02 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3A, 0x00, 0x2B, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0xF6, 0x0C, 0x00 }, { 0x00, 0x00, 0xF6, 0x06, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x36, RHYTHM_TYPE_UNDEFINED },
	// 0x30
	{ false, { 0x00, 0x00, 0xF6, 0x0C, 0x00 }, { 0x00, 0x00, 0xF6, 0x06, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x39, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x00, 0xF6, 0x00, 0x00 }, { 0xD0, 0x00, 0x9F, 0x02, 0x03 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x48, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0xF6, 0x0C, 0x00 }, { 0x00, 0x00, 0xF6, 0x06, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x34, 0x00, 0x3C, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x08, 0xF8, 0x42, 0x00 }, { 0x07, 0x4A, 0xF4, 0xE4, 0x03 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x4C, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x00, 0xF5, 0x30, 0x00 }, { 0xD0, 0x0A, 0x9F, 0x02, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x54, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x0A, 0xE4, 0xE4, 0x03 }, { 0x07, 0x5D, 0xF5, 0xE5, 0x01 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x24, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x02, 0x03, 0xB4, 0x04, 0x00 }, { 0x05, 0x0A, 0x97, 0xF7, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x4C, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x4E, 0x00, 0xF6, 0x00, 0x00 }, { 0x9E, 0x00, 0x9F, 0x02, 0x03 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x54, RHYTHM_TYPE_UNDEFINED },
	// 0x38
	{ false, { 0x11, 0x45, 0xF8, 0x37, 0x02 }, { 0x10, 0x08, 0xF3, 0x05, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x38, 0x00, 0x53, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x00, 0xF6, 0x00, 0x00 }, { 0xD0, 0x00, 0x9F, 0x02, 0x03 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x54, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x80, 0x00, 0xFF, 0x03, 0x03 }, { 0x10, 0x0D, 0xFF, 0x14, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3C, 0x00, 0x18, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x08, 0xF8, 0x42, 0x00 }, { 0x07, 0x4A, 0xF4, 0xE4, 0x03 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x4D, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x06, 0x0B, 0xF5, 0x0C, 0x00 }, { 0x02, 0x00, 0xF5, 0x08, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x3C, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x00, 0xFA, 0xBF, 0x00 }, { 0x02, 0x00, 0xC8, 0x97, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x37, 0x00, 0x41, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x51, 0xFA, 0x87, 0x00 }, { 0x01, 0x00, 0xFA, 0xB7, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x3B, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x54, 0xFA, 0x8D, 0x00 }, { 0x02, 0x00, 0xF8, 0xB8, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x33, RHYTHM_TYPE_UNDEFINED },
	// 0x40
	{ false, { 0x01, 0x59, 0xFA, 0x88, 0x00 }, { 0x02, 0x00, 0xF8, 0xB6, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x36, 0x00, 0x2D, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x01, 0x00, 0xF9, 0x0A, 0x03 }, { 0x00, 0x00, 0xFA, 0x06, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x47, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x80, 0xF9, 0x89, 0x03 }, { 0x00, 0x00, 0xF6, 0x6C, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x3C, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x03, 0x80, 0xF8, 0x88, 0x03 }, { 0x0C, 0x08, 0xF6, 0xB6, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3F, 0x00, 0x3A, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x03, 0x85, 0xF8, 0x88, 0x03 }, { 0x0C, 0x00, 0xF6, 0xB6, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3F, 0x00, 0x35, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x40, 0x76, 0x4F, 0x00 }, { 0x00, 0x08, 0x77, 0x18, 0x02 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x40, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x0E, 0x40, 0xC8, 0x49, 0x00 }, { 0x03, 0x00, 0x9B, 0x69, 0x02 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x47, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0xD7, 0xDC, 0xAD, 0x05, 0x03 }, { 0xC7, 0x00, 0x8D, 0x05, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x3D, RHYTHM_TYPE_UNDEFINED },
	// 0x48
	{ false, { 0xD7, 0xDC, 0xA8, 0x04, 0x03 }, { 0xC7, 0x00, 0x88, 0x04, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x3D, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x80, 0x00, 0xF6, 0x06, 0x03 }, { 0x11, 0x00, 0x67, 0x17, 0x03 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x30, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x80, 0x00, 0xF5, 0x05, 0x02 }, { 0x11, 0x09, 0x46, 0x16, 0x03 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x3E, 0x00, 0x30, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x06, 0x3F, 0x00, 0xF4, 0x00 }, { 0x15, 0x00, 0xF7, 0xF5, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x31, 0x00, 0x45, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x06, 0x3F, 0x00, 0xF4, 0x03 }, { 0x12, 0x00, 0xF7, 0xF5, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x30, 0x00, 0x44, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x3F, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x4A, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x3C, RHYTHM_TYPE_UNDEFINED },
	// 0x50
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x50, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x40, RHYTHM_TYPE_UNDEFINED },
	// GM percussion end

	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x45, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x49, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x4B, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x44, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x30, RHYTHM_TYPE_UNDEFINED },
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x35, RHYTHM_TYPE_UNDEFINED },
	// 0x58
	{ false, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00 }, 0x00, 0x00, 0x00, RHYTHM_TYPE_UNDEFINED }
	// GS percussion end
};

// Rhythm mode uses OPL channels 6, 7 and 8. The remaining channels are
// available for melodic instruments.
const uint8 MidiDriver_ADLIB_Multisource::MELODIC_CHANNELS_OPL2[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
const uint8 MidiDriver_ADLIB_Multisource::MELODIC_CHANNELS_OPL2_RHYTHM[] = { 0, 1, 2, 3, 4, 5 };
const uint8 MidiDriver_ADLIB_Multisource::MELODIC_CHANNELS_OPL3[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
const uint8 MidiDriver_ADLIB_Multisource::MELODIC_CHANNELS_OPL3_RHYTHM[] = { 0, 1, 2, 3, 4, 5, 9, 10, 11, 12, 13, 14, 15, 16, 17 };

const uint8 MidiDriver_ADLIB_Multisource::OPL_REGISTER_RHYTHM_OFFSETS[OPL_NUM_RHYTHM_INSTRUMENTS] = { 0x11, 0x15, 0x12, 0x14, 0x10 };

const uint8 MidiDriver_ADLIB_Multisource::OPL_RHYTHM_INSTRUMENT_CHANNELS[OPL_NUM_RHYTHM_INSTRUMENTS] = { 7, 8, 8, 7, 6 };

// These are the note frequency values used by the Win95 SB16 driver.
const uint16 MidiDriver_ADLIB_Multisource::OPL_NOTE_FREQUENCIES[12] = {
	0x0AB7, 0x0B5A, 0x0C07, 0x0CBE, 0x0D80, 0x0E4D, 0x0F27, 0x100E, 0x1102, 0x1205, 0x1318, 0x143A
};

// These are the volume values used by the Win95 SB16 driver.
const uint8 MidiDriver_ADLIB_Multisource::OPL_VOLUME_LOOKUP[32] = {
	0x50, 0x3F, 0x28, 0x24, 0x20, 0x1C, 0x17, 0x15, 0x13, 0x11, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A,
	0x09, 0x08, 0x07, 0x06, 0x05, 0x05, 0x04, 0x04, 0x03, 0x03, 0x02, 0x02, 0x01, 0x01, 0x00, 0x00
};

MidiDriver_ADLIB_Multisource::MidiChannelControlData::MidiChannelControlData() {
	init();
}

void MidiDriver_ADLIB_Multisource::MidiChannelControlData::init() {
	program = 0;
	channelPressure = 0;
	pitchBend = MIDI_PITCH_BEND_DEFAULT;

	modulation = 0;
	volume = 0;
	panning = MIDI_PANNING_DEFAULT;
	expression = MIDI_EXPRESSION_DEFAULT;
	sustain = false;
	rpn = MIDI_RPN_NULL;

	pitchBendSensitivity = GM_PITCH_BEND_SENSITIVITY_DEFAULT;
	pitchBendSensitivityCents = 0;
	masterTuningFine = MIDI_MASTER_TUNING_FINE_DEFAULT;
	masterTuningCoarse = MIDI_MASTER_TUNING_COARSE_DEFAULT;
}

MidiDriver_ADLIB_Multisource::ActiveNote::ActiveNote() {
	init();
}

void MidiDriver_ADLIB_Multisource::ActiveNote::init() {
	noteActive = false;
	noteSustained = false;

	note = 0;
	velocity = 0;
	channel = 0xFF;
	source = 0xFF;

	oplNote = 0;
	oplFrequency = 0;
	noteCounterValue = 0;

	instrumentId = 0;
	instrumentDef = nullptr;

	channelAllocated = false;
}

bool MidiDriver_ADLIB_Multisource::detectOplType(OPL::Config::OplType oplType) {
	return OPL::Config::detect(oplType) >= 0;
}

MidiDriver_ADLIB_Multisource::MidiDriver_ADLIB_Multisource(OPL::Config::OplType oplType, int timerFrequency) :
		_oplType(oplType),
		_opl(nullptr),
		_isOpen(false),
		_accuracyMode(ACCURACY_MODE_SB16_WIN95),
		_allocationMode(ALLOCATION_MODE_DYNAMIC),
		_instrumentWriteMode(INSTRUMENT_WRITE_MODE_NOTE_ON),
		_rhythmModeIgnoreNoteOffs(false),
		_channel10Melodic(false),
		_defaultChannelVolume(0),
		_noteSelect(NOTE_SELECT_MODE_0),
		_modulationDepth(MODULATION_DEPTH_HIGH),
		_vibratoDepth(VIBRATO_DEPTH_HIGH),
		_rhythmMode(false),
		_instrumentBank(OPL_INSTRUMENT_BANK),
		_rhythmBank(OPL_RHYTHM_BANK),
		_rhythmBankFirstNote(GS_RHYTHM_FIRST_NOTE),
		_rhythmBankLastNote(GS_RHYTHM_LAST_NOTE),
		_melodicChannels(nullptr),
		_numMelodicChannels(0),
		_noteCounter(1),
		_oplFrequencyConversionFactor(pow(2, 20) / 49716.0f),
		_timerFrequency(timerFrequency) {
	memset(_channelAllocations, 0xFF, sizeof(_channelAllocations));
	Common::fill(_shadowRegisters, _shadowRegisters + sizeof(_shadowRegisters), 0);
	_timerRate = 1000000 / _timerFrequency;
}

MidiDriver_ADLIB_Multisource::~MidiDriver_ADLIB_Multisource() {
	close();
}

int MidiDriver_ADLIB_Multisource::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	int8 detectResult = OPL::Config::detect(_oplType);
	
	if (detectResult == -1 && _oplType == OPL::Config::kDualOpl2) {
		// Try to emulate dual OPL2 on OPL3
		// TODO Implement this in fmopl
		detectResult = OPL::Config::detect(OPL::Config::kOpl3);
	}

	if (detectResult == -1)
		return MERR_DEVICE_NOT_AVAILABLE;

	// Create the emulator / hardware interface.
	_opl = OPL::Config::create(_oplType);

	if (!_opl)
		return MERR_CANNOT_CONNECT;

	_isOpen = true;

	// Initialize emulator / hardware interface.
	if (!_opl->init())
		return MERR_CANNOT_CONNECT;

	// Set the melodic channels applicable for the OPL chip type.
	determineMelodicChannels();

	// Set default MIDI channel volume on control data.
	for (int i = 0; i < MAXIMUM_SOURCES; i++) {
		for (int j = 0; j < MIDI_CHANNEL_COUNT; j++) {
			_controlData[i][j].volume = _defaultChannelVolume;
		}
		applyControllerDefaults(i);
	}

	// Set default OPL register values.
	initOpl();

	// Start the emulator / hardware interface. This will also start the timer
	// callbacks.
	_opl->start(new Common::Functor0Mem<void, MidiDriver_ADLIB_Multisource>(this, &MidiDriver_ADLIB_Multisource::onTimer), _timerFrequency);

	return 0;
}

bool MidiDriver_ADLIB_Multisource::isOpen() const {
	return _isOpen;
}

void MidiDriver_ADLIB_Multisource::close() {
	if (!_isOpen)
		return;

	_isOpen = false;

	stopAllNotes(true);

	if (_opl) {
		_opl->stop();
		delete _opl;
		_opl = nullptr;
	}
}

uint32 MidiDriver_ADLIB_Multisource::property(int prop, uint32 param) {
	switch (prop) {
	case PROP_OPL_ACCURACY_MODE:
		if (param == 0xFFFF)
			return _accuracyMode;

		switch (param) {
		case ACCURACY_MODE_GM:
			_accuracyMode = ACCURACY_MODE_GM;
			break;
		case ACCURACY_MODE_SB16_WIN95:
		default:
			_accuracyMode = ACCURACY_MODE_SB16_WIN95;
		}

		break;
	case PROP_OPL_CHANNEL_ALLOCATION_MODE:
		if (param == 0xFFFF)
			return _allocationMode;

		switch (param) {
		case ALLOCATION_MODE_STATIC:
			_allocationMode = ALLOCATION_MODE_STATIC;
			break;
		case ALLOCATION_MODE_DYNAMIC:
		default:
			_allocationMode = ALLOCATION_MODE_DYNAMIC;
		}

		break;
	case PROP_OPL_RHYTHM_MODE_IGNORE_NOTE_OFF:
		if (param == 0xFFFF)
			return _rhythmModeIgnoreNoteOffs;

		_rhythmModeIgnoreNoteOffs = (param != 0);
		break;
	default:
		return MidiDriver_Multisource::property(prop, param);
	}
	return 0;
}

uint32 MidiDriver_ADLIB_Multisource::getBaseTempo() {
	return _timerRate;
}

MidiChannel *MidiDriver_ADLIB_Multisource::allocateChannel() {
	// This driver does not use MidiChannel objects.
	return nullptr;
}

MidiChannel *MidiDriver_ADLIB_Multisource::getPercussionChannel() {
	// This driver does not use MidiChannel objects.
	return nullptr;
}

void MidiDriver_ADLIB_Multisource::send(int8 source, uint32 b) {
	byte command = b & 0xF0;

	if (source == -1) {
		// Source -1 is a shorthand to set controller values for all sources.
		if (command == MIDI_COMMAND_NOTE_OFF || command == MIDI_COMMAND_NOTE_ON) {
			// Notes should not be sent using source -1, but use source 0 in
			// case this happens.
			source = 0;
		} else {
			// Send controller event using all sources.
			for (int i = 0; i < MAXIMUM_SOURCES; i++) {
				send(i, b);
			}
			return;
		}
	}

	// Extract the MIDI bytes.
	byte channel = b & 0x0F;
	byte op1 = (b >> 8) & 0xFF;
	byte op2 = (b >> 16) & 0xFF;

	switch (command) {
	case MIDI_COMMAND_NOTE_OFF:
		noteOff(channel, op1, op2, source);
		break;
	case MIDI_COMMAND_NOTE_ON:
		noteOn(channel, op1, op2, source);
		break;
	case MIDI_COMMAND_POLYPHONIC_AFTERTOUCH: // Not supported by GM
		polyAftertouch(channel, op1, op2, source);
		break;
	case MIDI_COMMAND_CONTROL_CHANGE:
		controlChange(channel, op1, op2, source);
		break;
	case MIDI_COMMAND_PROGRAM_CHANGE:
		programChange(channel, op1, source);
		break;
	case MIDI_COMMAND_CHANNEL_AFTERTOUCH:
		channelAftertouch(channel, op1, source);
		break;
	case MIDI_COMMAND_PITCH_BEND:
		pitchBend(channel, op1, op2, source);
		break;
	case MIDI_COMMAND_SYSTEM:
		// The only supported system event is SysEx and that should be sent
		// using the sysEx functions.
		warning("MidiDriver_ADLIB_Multisource: send received system event (not processed): %x", b);
		break;
	default:
		warning("MidiDriver_ADLIB_Multisource: Received unknown event %02x", command);
		break;
	}
}

void MidiDriver_ADLIB_Multisource::noteOff(uint8 channel, uint8 note, uint8 velocity, uint8 source) {
	_activeNotesMutex.lock();

	if (_rhythmMode && channel == MIDI_RHYTHM_CHANNEL) {
		if (!_rhythmModeIgnoreNoteOffs) {
			// Find the OPL rhythm instrument playing this note.
			for (int i = 0; i < OPL_NUM_RHYTHM_INSTRUMENTS; i++) {
				if (_activeRhythmNotes[i].noteActive && _activeRhythmNotes[i].source == source &&
						_activeRhythmNotes[i].note == note) {
					writeKeyOff(0, static_cast<OplInstrumentRhythmType>(i + 1));
					break;
				}
			}
		}
	} else {
		// Find the OPL channel playing this note.
		for (int i = 0; i < _numMelodicChannels; i++) {
			uint8 oplChannel = _melodicChannels[i];
			if (_activeNotes[oplChannel].noteActive && _activeNotes[oplChannel].source == source &&
					_activeNotes[oplChannel].channel == channel && _activeNotes[oplChannel].note == note) {
				if (_controlData[source][channel].sustain) {
					// Sustain controller is on. Sustain the note instead of
					// ending it.
					_activeNotes[oplChannel].noteSustained = true;
				} else {
					writeKeyOff(oplChannel);
				}
			}
		}
	}

	_activeNotesMutex.unlock();
}

void MidiDriver_ADLIB_Multisource::noteOn(uint8 channel, uint8 note, uint8 velocity, uint8 source) {
	if (velocity == 0) {
		// Note on with velocity 0 is a note off.
		noteOff(channel, note, velocity, source);
		return;
	}

	InstrumentInfo instrument = determineInstrument(channel, source, note);
	// If rhythm mode is on and the note is on the rhythm channel, this note
	// will be played using the OPL rhythm register.
	bool rhythmNote = _rhythmMode && channel == MIDI_RHYTHM_CHANNEL;

	if (!instrument.instrumentDef || instrument.instrumentDef->isEmpty() ||
			(rhythmNote && instrument.instrumentDef->rhythmType == RHYTHM_TYPE_UNDEFINED)) {
		// Instrument definition contains no data or it is not suitable for
		// rhythm mode, so the note cannot be played.
		return;
	}

	_activeNotesMutex.lock();

	// Determine the OPL channel to use and the active note data to update.
	uint8 oplChannel = 0xFF;
	ActiveNote *activeNote = nullptr;
	if (rhythmNote) {
		activeNote = &_activeRhythmNotes[instrument.instrumentDef->rhythmType - 1];
	} else {
		// Allocate a melodic OPL channel.
		oplChannel = allocateOplChannel(channel, source, instrument.instrumentId);
		if (oplChannel != 0xFF)
			activeNote = &_activeNotes[oplChannel];
	}
	if (activeNote != nullptr) {
		if (activeNote->noteActive) {
			// Turn off the note currently playing on this OPL channel or
			// rhythm instrument.
			writeKeyOff(oplChannel, instrument.instrumentDef->rhythmType);
		}

		// Update the active note data.
		activeNote->noteActive = true;
		activeNote->noteSustained = false;
		activeNote->note = note;
		activeNote->velocity = velocity;
		activeNote->channel = channel;
		activeNote->source = source;

		activeNote->oplNote = instrument.oplNote;
		// Increase the note counter when playing a new note.
		activeNote->noteCounterValue = _noteCounter++;
		activeNote->instrumentId = instrument.instrumentId;
		activeNote->instrumentDef = instrument.instrumentDef;

		if (_instrumentWriteMode == INSTRUMENT_WRITE_MODE_NOTE_ON) {
			// Write out the instrument definition, volume and panning.
			writeInstrument(oplChannel, instrument);
		}

		// Calculate and write frequency and block and write key on bit.
		writeFrequency(oplChannel, instrument.instrumentDef->rhythmType);

		if (rhythmNote)
			// Update the rhythm register.
			writeRhythm();
	}

	_activeNotesMutex.unlock();
}

void MidiDriver_ADLIB_Multisource::polyAftertouch(uint8 channel, uint8 note, uint8 pressure, uint8 source) {
	// Because this event is not required by General MIDI and not implemented
	// in the Win95 SB16 driver, there is no default implementation.
}

void MidiDriver_ADLIB_Multisource::controlChange(uint8 channel, uint8 controller, uint8 value, uint8 source) {
	// Call the function for handling each controller.
	switch (controller) {
	case MIDI_CONTROLLER_MODULATION:
		modulation(channel, value, source);
		break;
	case MIDI_CONTROLLER_DATA_ENTRY_MSB:
		dataEntry(channel, value, 0xFF, source);
		break;
	case MIDI_CONTROLLER_VOLUME:
		volume(channel, value, source);
		break;
	case MIDI_CONTROLLER_PANNING:
		panning(channel, value, source);
		break;
	case MIDI_CONTROLLER_EXPRESSION:
		expression(channel, value, source);
		break;
	case MIDI_CONTROLLER_DATA_ENTRY_LSB:
		dataEntry(channel, 0xFF, value, source);
		break;
	case MIDI_CONTROLLER_SUSTAIN:
		sustain(channel, value, source);
		break;
	case MIDI_CONTROLLER_RPN_LSB:
		registeredParameterNumber(channel, 0xFF, value, source);
		break;
	case MIDI_CONTROLLER_RPN_MSB:
		registeredParameterNumber(channel, value, 0xFF, source);
		break;
	case MIDI_CONTROLLER_ALL_SOUND_OFF:
		allSoundOff(channel, source);
		break;
	case MIDI_CONTROLLER_RESET_ALL_CONTROLLERS:
		resetAllControllers(channel, source);
		break;
	case MIDI_CONTROLLER_ALL_NOTES_OFF:
	case MIDI_CONTROLLER_OMNI_OFF:
	case MIDI_CONTROLLER_OMNI_ON:
	case MIDI_CONTROLLER_MONO_ON:
	case MIDI_CONTROLLER_POLY_ON:
		// The omni/mono/poly events also act as an all notes off.
		allNotesOff(channel, source);
		break;
	default:
		//debug("MidiDriver_ADLIB_Multisource::controlChange - Unsupported controller %X", controller);
		break;
	}
}

void MidiDriver_ADLIB_Multisource::programChange(uint8 channel, uint8 program, uint8 source) {
	// Just set the MIDI program value; this event does not affect active notes.
	_controlData[source][channel].program = program;

	if (_instrumentWriteMode == INSTRUMENT_WRITE_MODE_PROGRAM_CHANGE && !(_rhythmMode && channel == MIDI_RHYTHM_CHANNEL)) {
		InstrumentInfo instrument = determineInstrument(channel, source, 0);

		if (!instrument.instrumentDef || instrument.instrumentDef->isEmpty()) {
			// Instrument definition contains no data.
			return;
		}

		_activeNotesMutex.lock();

		// Determine the OPL channel to use and the active note data to update.
		uint8 oplChannel = 0xFF;
		ActiveNote *activeNote = nullptr;
		// Allocate a melodic OPL channel.
		oplChannel = allocateOplChannel(channel, source, instrument.instrumentId);
		if (oplChannel != 0xFF) {
			activeNote = &_activeNotes[oplChannel];
			if (activeNote->noteActive) {
				// Turn off the note currently playing on this OPL channel or
				// rhythm instrument.
				writeKeyOff(oplChannel, instrument.instrumentDef->rhythmType);
			}

			// Update the active note data.
			activeNote->channel = channel;
			activeNote->source = source;
			activeNote->instrumentId = instrument.instrumentId;
			activeNote->instrumentDef = instrument.instrumentDef;

			writeInstrument(oplChannel, instrument);
		}

		_activeNotesMutex.unlock();
	}
}

void MidiDriver_ADLIB_Multisource::channelAftertouch(uint8 channel, uint8 pressure, uint8 source) {
	// Even though this event is required by General MIDI, it is not implemented
	// in the Win95 SB16 driver, so there is no default implementation.
}

void MidiDriver_ADLIB_Multisource::pitchBend(uint8 channel, uint8 pitchBendLsb, uint8 pitchBendMsb, uint8 source) {
	_controlData[source][channel].pitchBend = ((uint16)pitchBendMsb) << 7 | pitchBendLsb;

	// Recalculate and write the frequencies of the active notes on this MIDI
	// channel to let the new pitch bend value take effect.
	recalculateFrequencies(channel, source);
}

void MidiDriver_ADLIB_Multisource::sysEx(const byte *msg, uint16 length) {
	if (length >= 4 && msg[0] == 0x7E && msg[2] == 0x09 && msg[3] == 0x01) {
		// F0 7E <device ID> 09 01 F7
		// General MIDI System On
		
		// Reset the MIDI context and the OPL chip.

		stopAllNotes(true);

		for (int i = 0; i < MAXIMUM_SOURCES; i++) {
			for (int j = 0; j < MIDI_CHANNEL_COUNT; j++) {
				_controlData[i][j].init();
			}
		}

		setRhythmMode(false);

		for (int i = 0; i < _numMelodicChannels; i++) {
			_activeNotes[_melodicChannels[i]].init();
		}

		memset(_channelAllocations, 0xFF, sizeof(_channelAllocations));
		_noteCounter = 1;

		initOpl();
	} else {
		// Ignore other SysEx messages.
		warning("MidiDriver_ADLIB_Multisource::sysEx - Unrecognized SysEx");
	}
}

void MidiDriver_ADLIB_Multisource::metaEvent(int8 source, byte type, byte *data, uint16 length) {
	if (type == MIDI_META_END_OF_TRACK && source >= 0)
		// Stop hanging notes and release resources used by this source.
		deinitSource(source);
}

void MidiDriver_ADLIB_Multisource::deinitSource(uint8 source) {
	// Turn off sustained notes.
	for (int i = 0; i < MIDI_CHANNEL_COUNT; i++) {
		sustain(i, 0, source);
	}

	// Stop fades and turn off non-sustained notes.
	MidiDriver_Multisource::deinitSource(source);

	_allocationMutex.lock();

	// Deallocate channels
	for (int i = 0; i < MIDI_CHANNEL_COUNT; i++) {
		_channelAllocations[source][i] = 0xFF;
	}
	for (int i = 0; i < _numMelodicChannels; i++) {
		uint8 oplChannel = _melodicChannels[i];
		if (_activeNotes[oplChannel].channelAllocated && _activeNotes[oplChannel].source == source) {
			_activeNotes[oplChannel].channelAllocated = false;
		}
	}

	_allocationMutex.unlock();

	applyControllerDefaults(source);
}

void MidiDriver_ADLIB_Multisource::applyControllerDefaults(uint8 source) {
	if (source == 0xFF) {
		// Apply controller defaults for all sources.
		for (int i = 0; i < MAXIMUM_SOURCES; i++) {
			applyControllerDefaults(i);
		}
	} else {
		for (int i = 0; i < MIDI_CHANNEL_COUNT; i++) {
			if (_controllerDefaults.program[i] >= 0) {
				_controlData[source][i].program = _controllerDefaults.program[i];
			}
			if (_controllerDefaults.channelPressure >= 0) {
				_controlData[source][i].channelPressure = _controllerDefaults.channelPressure;
			}
			if (_controllerDefaults.pitchBend >= 0) {
				_controlData[source][i].pitchBend = _controllerDefaults.pitchBend;
			}
			if (_controllerDefaults.modulation >= 0) {
				_controlData[source][i].modulation = _controllerDefaults.modulation;
			}
			if (_controllerDefaults.volume >= 0) {
				_controlData[source][i].volume = _controllerDefaults.volume;
			}
			if (_controllerDefaults.panning >= 0) {
				_controlData[source][i].panning = _controllerDefaults.panning;
			}
			if (_controllerDefaults.expression >= 0) {
				_controlData[source][i].expression = _controllerDefaults.expression;
			}
			if (_controllerDefaults.rpn >= 0) {
				_controlData[source][i].rpn = _controllerDefaults.rpn;
			}
			if (_controllerDefaults.pitchBendSensitivity >= 0) {
				_controlData[source][i].pitchBendSensitivity = _controllerDefaults.pitchBendSensitivity;
				_controlData[source][i].pitchBendSensitivityCents = 0;
			}
			// Controller defaults not supported by this driver:
			// instrument bank, drumkit.
			// Sustain is turned off by deinitSource.
		}
	}
}

void MidiDriver_ADLIB_Multisource::modulation(uint8 channel, uint8 modulation, uint8 source) {
	// Even though this controller is required by General MIDI, it is not
	// implemented in the Win95 SB16 driver, so there is no default
	// implementation.
}

void MidiDriver_ADLIB_Multisource::dataEntry(uint8 channel, uint8 dataMsb, uint8 dataLsb, uint8 source) {
	// Set the data on the currently active RPN.
	switch (_controlData[source][channel].rpn) {
	case MIDI_RPN_PITCH_BEND_SENSITIVITY:
		// MSB = semitones, LSB = cents.
		if (dataMsb != 0xFF) {
			_controlData[source][channel].pitchBendSensitivity = dataMsb;
		}
		if (dataLsb != 0xFF) {
			_controlData[source][channel].pitchBendSensitivityCents = dataLsb;
		}
		// Apply the new pitch bend sensitivity to any active notes.
		recalculateFrequencies(channel, source);
		break;
	case MIDI_RPN_MASTER_TUNING_FINE:
		// MSB and LSB are combined to a fraction of a semitone.
		if (dataMsb != 0xFF) {
			_controlData[source][channel].masterTuningFine &= 0x00FF;
			_controlData[source][channel].masterTuningFine |= dataMsb << 8;
		}
		if (dataLsb != 0xFF) {
			_controlData[source][channel].masterTuningFine &= 0xFF00;
			_controlData[source][channel].masterTuningFine |= dataLsb;
		}
		// Apply the new master tuning to any active notes.
		recalculateFrequencies(channel, source);
		break;
	case MIDI_RPN_MASTER_TUNING_COARSE:
		// MSB = semitones, LSB is ignored.
		if (dataMsb != 0xFF) {
			_controlData[source][channel].masterTuningCoarse = dataMsb;
		}
		// Apply the new master tuning to any active notes.
		recalculateFrequencies(channel, source);
		break;
	default:
		// Ignore data entry if null or an unknown RPN is active.
		break;
	}
}

void MidiDriver_ADLIB_Multisource::volume(uint8 channel, uint8 volume, uint8 source) {
	if (_controlData[source][channel].volume == volume)
		return;

	_controlData[source][channel].volume = volume;
	// Apply the new channel volume to any active notes.
	recalculateVolumes(channel, source);
}

void MidiDriver_ADLIB_Multisource::panning(uint8 channel, uint8 panning, uint8 source) {
	if (_controlData[source][channel].panning == panning)
		return;

	_controlData[source][channel].panning = panning;

	_activeNotesMutex.lock();

	// Apply the new channel panning to any active notes.
	if (_rhythmMode && channel == MIDI_RHYTHM_CHANNEL) {
		for (int i = 0; i < OPL_NUM_RHYTHM_INSTRUMENTS; i++) {
			if (_activeRhythmNotes[i].noteActive && _activeRhythmNotes[i].source == source) {
				writePanning(0xFF, static_cast<OplInstrumentRhythmType>(i + 1));
			}
		}
	} else {
		for (int i = 0; i < _numMelodicChannels; i++) {
			uint8 oplChannel = _melodicChannels[i];
			if (_activeNotes[oplChannel].noteActive && _activeNotes[oplChannel].channel == channel &&
					_activeNotes[oplChannel].source == source) {
				writePanning(oplChannel);
			}
		}
	}

	_activeNotesMutex.unlock();
}

void MidiDriver_ADLIB_Multisource::expression(uint8 channel, uint8 expression, uint8 source) {
	if (_controlData[source][channel].expression == expression)
		return;

	_controlData[source][channel].expression = expression;
	// Apply the new expression value to any active notes.
	recalculateVolumes(channel, source);
}

void MidiDriver_ADLIB_Multisource::sustain(uint8 channel, uint8 sustain, uint8 source) {
	if (sustain >= 0x40) {
		// Turn on sustain.
		_controlData[source][channel].sustain = true;
	} else if (_controlData[source][channel].sustain) {
		// Sustain is currently on. Turn it off.
		_controlData[source][channel].sustain = false;

		_activeNotesMutex.lock();

		// Turn off any sustained notes on this channel.
		for (int i = 0; i < _numMelodicChannels; i++) {
			uint8 oplChannel = _melodicChannels[i];
			if (_activeNotes[oplChannel].noteActive && _activeNotes[oplChannel].noteSustained &&
					_activeNotes[oplChannel].channel == channel && _activeNotes[oplChannel].source == source) {
				writeKeyOff(oplChannel);
			}
		}

		_activeNotesMutex.unlock();
	}
}

void MidiDriver_ADLIB_Multisource::registeredParameterNumber(uint8 channel, uint8 rpnMsb, uint8 rpnLsb, uint8 source) {
	// Set the currently active RPN. MSB and LSB combined form the RPN number.
	if (rpnMsb != 0xFF) {
		_controlData[source][channel].rpn &= 0x00FF;
		_controlData[source][channel].rpn |= rpnMsb << 8;
	}
	if (rpnLsb != 0xFF) {
		_controlData[source][channel].rpn &= 0xFF00;
		_controlData[source][channel].rpn |= rpnLsb;
	}
}

void MidiDriver_ADLIB_Multisource::allSoundOff(uint8 channel, uint8 source) {
	// It is not possible to immediately terminate the sound on an OPL chip
	// (skipping the "release" of the notes), so just turn the notes off.
	stopAllNotes(source, channel);
}

void MidiDriver_ADLIB_Multisource::resetAllControllers(uint8 channel, uint8 source) {
	modulation(channel, 0, source);
	expression(channel, MIDI_EXPRESSION_DEFAULT, source);
	sustain(channel, 0, source);
	registeredParameterNumber(channel, MIDI_RPN_NULL >> 8, MIDI_RPN_NULL & 0xFF, source);
	pitchBend(channel, MIDI_PITCH_BEND_DEFAULT & 0x7F, MIDI_PITCH_BEND_DEFAULT >> 7, source);
	channelAftertouch(channel, 0, source);
	// TODO Polyphonic aftertouch should also be reset; not implemented because
	// polyphonic aftertouch is not implemented.
}

void MidiDriver_ADLIB_Multisource::allNotesOff(uint8 channel, uint8 source) {
	_activeNotesMutex.lock();

	// Execute a note off for all active notes on this MIDI channel. This will
	// turn the notes off if sustain is off and sustain the notes if it is on.
	if (_rhythmMode && channel == MIDI_RHYTHM_CHANNEL) {
		for (int i = 0; i < OPL_NUM_RHYTHM_INSTRUMENTS; i++) {
			if (_activeRhythmNotes[i].noteActive && _activeRhythmNotes[i].source == source) {
				noteOff(channel, _activeRhythmNotes[i].note, 0, source);
			}
		}
	} else {
		for (int i = 0; i < _numMelodicChannels; i++) {
			uint8 oplChannel = _melodicChannels[i];
			if (_activeNotes[oplChannel].noteActive && !_activeNotes[oplChannel].noteSustained && 
					_activeNotes[oplChannel].source == source && _activeNotes[oplChannel].channel == channel) {
				noteOff(channel, _activeNotes[oplChannel].note, 0, source);
			}
		}
	}

	_activeNotesMutex.unlock();
}

void MidiDriver_ADLIB_Multisource::stopAllNotes(bool stopSustainedNotes) {
	// Just write the key off bit on all OPL channels. No special handling is
	// needed to make sure sustained notes are turned off.
	for (int i = 0; i < _numMelodicChannels; i++) {
		// Force the register write to prevent accidental hanging notes.
		writeKeyOff(_melodicChannels[i], RHYTHM_TYPE_UNDEFINED, true);
	}
	if (_rhythmMode) {
		for (int i = 0; i < 5; i++) {
			_activeRhythmNotes[i].noteActive = false;
		}
		writeRhythm(true);
	}
}

void MidiDriver_ADLIB_Multisource::stopAllNotes(uint8 source, uint8 channel) {
	_activeNotesMutex.lock();

	// Write the key off bit for all active notes on this MIDI channel and
	// source.
	for (int i = 0; i < _numMelodicChannels; i++) {
		uint8 oplChannel = _melodicChannels[i];
		if (_activeNotes[oplChannel].noteActive && (source == 0xFF || _activeNotes[oplChannel].source == source) &&
			(channel == 0xFF || _activeNotes[oplChannel].channel == channel)) {
			writeKeyOff(oplChannel);
		}
	}
	if (_rhythmMode && !_rhythmModeIgnoreNoteOffs && (channel == 0xFF || channel == MIDI_RHYTHM_CHANNEL)) {
		bool rhythmChanged = false;
		for (int i = 0; i < 5; i++) {
			if (_activeRhythmNotes[i].noteActive && (source == 0xFF || _activeRhythmNotes[i].source == source)) {
				_activeRhythmNotes[i].noteActive = false;
				rhythmChanged = true;
			}
		}
		if (rhythmChanged)
			writeRhythm();
	}
	
	_activeNotesMutex.unlock();
}

void MidiDriver_ADLIB_Multisource::applySourceVolume(uint8 source) {
	// Recalculate the volume of the active notes on all MIDI channels of this
	// source.
	recalculateVolumes(0xFF, source);
}

void MidiDriver_ADLIB_Multisource::initOpl() {
	// Clear test flags and enable waveform select for OPL2 chips.
	writeRegister(OPL_REGISTER_TEST, _oplType == OPL::Config::kOpl3 ? 0 : 0x20, true);
	if (_oplType != OPL::Config::kOpl2) {
		writeRegister(OPL_REGISTER_TEST | OPL_REGISTER_SET_2_OFFSET, _oplType == OPL::Config::kOpl3 ? 0 : 0x20, true);
	}

	// Clear, stop and mask the timers and reset the interrupt.
	writeRegister(OPL_REGISTER_TIMER1, 0, true);
	writeRegister(OPL_REGISTER_TIMER2, 0, true);
	writeRegister(OPL_REGISTER_TIMERCONTROL, 0x60, true);
	writeRegister(OPL_REGISTER_TIMERCONTROL, 0x80, true);
	if (_oplType == OPL::Config::kDualOpl2) {
		writeRegister(OPL_REGISTER_TIMER1 | OPL_REGISTER_SET_2_OFFSET, 0, true);
		writeRegister(OPL_REGISTER_TIMER2 | OPL_REGISTER_SET_2_OFFSET, 0, true);
		writeRegister(OPL_REGISTER_TIMERCONTROL | OPL_REGISTER_SET_2_OFFSET, 0x60, true);
		writeRegister(OPL_REGISTER_TIMERCONTROL | OPL_REGISTER_SET_2_OFFSET, 0x80, true);
	}

	if (_oplType == OPL::Config::kOpl3) {
		// Turn off 4 operator mode for all channels.
		writeRegister(OPL3_REGISTER_CONNECTIONSELECT, 0, true);
		// Enable "new" OPL3 functionality.
		writeRegister(OPL3_REGISTER_NEW, 1, true);
	}

	// Set note select mode and disable CSM mode for OPL2 chips.
	writeRegister(OPL_REGISTER_NOTESELECT_CSM, _noteSelect << 6, true);
	if (_oplType == OPL::Config::kDualOpl2) {
		writeRegister(OPL_REGISTER_NOTESELECT_CSM | OPL_REGISTER_SET_2_OFFSET, _noteSelect << 6, true);
	}

	// Set operator registers to default values.
	for (int i = 0; i < 5; i++) {
		uint8 baseReg = 0;
		uint8 value = 0;
		switch (i) {
		case 0:
			baseReg = OPL_REGISTER_BASE_FREQMULT_MISC;
			break;
		case 1:
			baseReg = OPL_REGISTER_BASE_LEVEL;
			// Set volume to the default MIDI channel volume.
			// Convert from MIDI to OPL register value.
			value = 0x3F - (_defaultChannelVolume >> 1);
			break;
		case 2:
			baseReg = OPL_REGISTER_BASE_DECAY_ATTACK;
			break;
		case 3:
			baseReg = OPL_REGISTER_BASE_RELEASE_SUSTAIN;
			break;
		case 4:
			baseReg = OPL_REGISTER_BASE_WAVEFORMSELECT;
			break;
		}

		for (int j = 0; j < (_oplType == OPL::Config::kOpl2 ? OPL2_NUM_CHANNELS : OPL3_NUM_CHANNELS); j++) {
			writeRegister(baseReg + determineOperatorRegisterOffset(j, 0), value, true);
			writeRegister(baseReg + determineOperatorRegisterOffset(j, 1), value, true);
		}
	}

	// Set channel registers to default values.
	for (int i = 0; i < 3; i++) {
		uint8 baseReg = 0;
		uint8 value = 0;
		switch (i) {
		case 0:
			baseReg = OPL_REGISTER_BASE_FNUMLOW;
			break;
		case 1:
			baseReg = OPL_REGISTER_BASE_FNUMHIGH_BLOCK_KEYON;
			break;
		case 2:
			baseReg = OPL_REGISTER_BASE_CONNECTION_FEEDBACK_PANNING;
			if (_oplType == OPL::Config::kOpl3) {
				// Set default panning to center.
				value = OPL_PANNING_CENTER;
			}
			break;
		}

		for (int j = 0; j < (_oplType == OPL::Config::kOpl2 ? OPL2_NUM_CHANNELS : OPL3_NUM_CHANNELS); j++) {
			writeRegister(baseReg + determineChannelRegisterOffset(j), value, true);
		}
	}

	// Set rhythm mode, modulation and vibrato depth.
	writeRhythm(true);
}

void MidiDriver_ADLIB_Multisource::recalculateFrequencies(uint8 channel, uint8 source) {
	_activeNotesMutex.lock();

	// Calculate and write the frequency of all active notes on this MIDI
	// channel and source.
	if (_rhythmMode && channel == MIDI_RHYTHM_CHANNEL) {
		// Always rewrite bass drum frequency if it is active.
		if (_activeRhythmNotes[RHYTHM_TYPE_BASS_DRUM - 1].noteActive && _activeRhythmNotes[RHYTHM_TYPE_BASS_DRUM - 1].source == source) {
			writeFrequency(0xFF, RHYTHM_TYPE_BASS_DRUM);
		}

		// Snare drum and hi-hat share the same frequency setting. If both are
		// active, use the most recently played instrument.
		OplInstrumentRhythmType rhythmType = RHYTHM_TYPE_UNDEFINED;
		bool snareActive = _activeRhythmNotes[RHYTHM_TYPE_SNARE_DRUM - 1].noteActive && _activeRhythmNotes[RHYTHM_TYPE_SNARE_DRUM - 1].source == source;
		bool hiHatActive = _activeRhythmNotes[RHYTHM_TYPE_HI_HAT - 1].noteActive && _activeRhythmNotes[RHYTHM_TYPE_HI_HAT - 1].source == source;
		if (snareActive && hiHatActive) {
			rhythmType = (_activeRhythmNotes[RHYTHM_TYPE_SNARE_DRUM - 1].noteCounterValue >=
				_activeRhythmNotes[RHYTHM_TYPE_HI_HAT - 1].noteCounterValue ? RHYTHM_TYPE_SNARE_DRUM : RHYTHM_TYPE_HI_HAT);
		} else if (snareActive) {
			rhythmType = RHYTHM_TYPE_SNARE_DRUM;
		} else if (hiHatActive) {
			rhythmType = RHYTHM_TYPE_HI_HAT;
		}
		if (rhythmType != RHYTHM_TYPE_UNDEFINED)
			writeFrequency(0xFF, rhythmType);

		// Tom tom and cymbal share the same frequency setting. If both are 
		// active, use the most recently played instrument.
		rhythmType = RHYTHM_TYPE_UNDEFINED;
		bool tomTomActive = _activeRhythmNotes[RHYTHM_TYPE_TOM_TOM - 1].noteActive && _activeRhythmNotes[RHYTHM_TYPE_TOM_TOM - 1].source == source;
		bool cymbalActive = _activeRhythmNotes[RHYTHM_TYPE_CYMBAL - 1].noteActive && _activeRhythmNotes[RHYTHM_TYPE_CYMBAL - 1].source == source;
		if (tomTomActive && cymbalActive) {
			rhythmType = (_activeRhythmNotes[RHYTHM_TYPE_TOM_TOM - 1].noteCounterValue >=
				_activeRhythmNotes[RHYTHM_TYPE_CYMBAL - 1].noteCounterValue ? RHYTHM_TYPE_TOM_TOM : RHYTHM_TYPE_CYMBAL);
		} else if (tomTomActive) {
			rhythmType = RHYTHM_TYPE_TOM_TOM;
		} else if (cymbalActive) {
			rhythmType = RHYTHM_TYPE_CYMBAL;
		}
		if (rhythmType != RHYTHM_TYPE_UNDEFINED)
			writeFrequency(0xFF, rhythmType);
	} else {
		for (int i = 0; i < _numMelodicChannels; i++) {
			uint8 oplChannel = _melodicChannels[i];
			if (_activeNotes[oplChannel].noteActive && _activeNotes[oplChannel].channel == channel &&
					_activeNotes[oplChannel].source == source) {
				writeFrequency(oplChannel);
			}
		}
	}

	_activeNotesMutex.unlock();
}

void MidiDriver_ADLIB_Multisource::recalculateVolumes(uint8 channel, uint8 source) {
	_activeNotesMutex.lock();

	// Calculate and write the volume of all operators of all active notes on
	// this MIDI channel and source. 
	for (int i = 0; i < _numMelodicChannels; i++) {
		uint8 oplChannel = _melodicChannels[i];
		if (_activeNotes[oplChannel].noteActive &&
				(channel == 0xFF || _activeNotes[oplChannel].channel == channel) &&
				(source == 0xFF || _activeNotes[oplChannel].source == source)) {
			for (int j = 0; j < _activeNotes[oplChannel].instrumentDef->getNumberOfOperators(); j++) {
				writeVolume(oplChannel, j);
			}
		}
	}
	if (_rhythmMode && (channel == 0xFF || channel == MIDI_RHYTHM_CHANNEL)) {
		for (int i = 0; i < OPL_NUM_RHYTHM_INSTRUMENTS; i++) {
			if (_activeRhythmNotes[i].noteActive && (source == 0xFF || _activeRhythmNotes[i].source == source)) {
				for (int j = 0; j < _activeRhythmNotes[i].instrumentDef->getNumberOfOperators(); j++) {
					writeVolume(0xFF, j, static_cast<OplInstrumentRhythmType>(i + 1));
				}
			}
		}
	}

	_activeNotesMutex.unlock();
}

MidiDriver_ADLIB_Multisource::InstrumentInfo MidiDriver_ADLIB_Multisource::determineInstrument(uint8 channel, uint8 source, uint8 note) {
	InstrumentInfo instrument = { 0, nullptr, 0 };

	if (!_channel10Melodic && channel == MIDI_RHYTHM_CHANNEL) {
		// On the rhythm channel, the note played indicates which instrument
		// should be used.
		if (note < _rhythmBankFirstNote || note > _rhythmBankLastNote)
			// No rhythm instrument assigned to this note number.
			return instrument;

		// Set the high bit for rhythm instrument IDs.
		instrument.instrumentId = 0x80 | note;
		instrument.instrumentDef = &_rhythmBank[note - _rhythmBankFirstNote];
		// Get the note to play from the instrument definition.
		instrument.oplNote = instrument.instrumentDef->rhythmNote;
	} else {
		// On non-rhythm channels, use the active instrument (program) on the
		// MIDI channel.
		byte program = _controlData[source][channel].program;
		if (_instrumentRemapping)
			// Apply instrument remapping (if specified).
			program = _instrumentRemapping[program];
		instrument.instrumentId = program;
		instrument.instrumentDef = &_instrumentBank[instrument.instrumentId];
		instrument.oplNote = note;
	}

	return instrument;
}

uint8 MidiDriver_ADLIB_Multisource::allocateOplChannel(uint8 channel, uint8 source, uint8 instrumentId) {

	uint8 allocatedChannel = 0xFF;
	if (_allocationMode == ALLOCATION_MODE_DYNAMIC) {
		// In dynamic channel allocation mode, each note is allocated a new
		// OPL channel. The following criteria are used, in this order:
		// - The channel with the lowest number that has not yet been used to
		//   play a note (note counter value is 0).
		// - The channel with the lowest note counter value that is not
		//   currently playing a note.
		// - The channel with the lowest note counter value that is playing a
		//   note using the same instrument.
		// - The channel with the lowest note counter value (i.e. playing the
		//   oldest note).
		// This will always return a channel; if a note is currently playing,
		// it will be aborted.

		uint8 unusedChannel = 0xFF, inactiveChannel = 0xFF, instrumentChannel = 0xFF, lowestCounterChannel = 0xFF;
		uint32 inactiveNoteCounter = 0xFFFF, instrumentNoteCounter = 0xFFFF, lowestNoteCounter = 0xFFFF;
		for (int i = 0; i < _numMelodicChannels; i++) {
			uint8 oplChannel = _melodicChannels[i];
			if (_activeNotes[oplChannel].channelAllocated)
				// Channel has been statically allocated. Try the next channel.
				continue;

			if (_activeNotes[oplChannel].noteCounterValue == 0) {
				// This channel is unused. No need to look any further.
				unusedChannel = oplChannel;
				break;
			}
			if (!_activeNotes[oplChannel].noteActive && _activeNotes[oplChannel].noteCounterValue < inactiveNoteCounter) {
				// A channel not playing a note with a lower note counter value
				// has been found.
				inactiveNoteCounter = _activeNotes[oplChannel].noteCounterValue;
				inactiveChannel = oplChannel;
				continue;
			}
			if (_activeNotes[oplChannel].noteActive && _activeNotes[oplChannel].instrumentId == instrumentId &&
					_activeNotes[oplChannel].noteCounterValue < instrumentNoteCounter) {
				// A channel playing a note using the same instrument with a
				// lower note counter value has been found.
				instrumentNoteCounter = _activeNotes[oplChannel].noteCounterValue;
				instrumentChannel = oplChannel;
			}
			if (_activeNotes[oplChannel].noteActive && _activeNotes[oplChannel].noteCounterValue < lowestNoteCounter) {
				// A channel playing a note with a lower note counter value has
				// been found.
				lowestNoteCounter = _activeNotes[oplChannel].noteCounterValue;
				lowestCounterChannel = oplChannel;
			}
		}

		if (unusedChannel != 0xFF)
			// An unused channel has been found. Use this.
			allocatedChannel = unusedChannel;
		else if (inactiveChannel != 0xFF)
			// An inactive channel has been found. Use this.
			allocatedChannel = inactiveChannel;
		else if (instrumentChannel != 0xFF)
			// An active channel using the same instrument has been found.
			// Use this.
			allocatedChannel = instrumentChannel;
		else
			// Just use the channel playing the oldest note.
			allocatedChannel = lowestCounterChannel;
	} else {
		// In static allocation mode, each MIDI channel of each source is
		// allocated a fixed OPL channel to use. All notes on that MIDI channel
		// are played using the allocated OPL channel. If a new MIDI channel
		// needs an OPL channel and all OPL channels have already been
		// allocated, allocation will fail.

		allocatedChannel = 0xFF;

		_allocationMutex.lock();

		if (_channelAllocations[source][channel] != 0xFF) {
			// An OPL channel has already been allocated to this MIDI channel
			// for this source. Use the previously allocated channel.
			allocatedChannel = _channelAllocations[source][channel];
		} else {
			// No OPL channel has been allocated yet. Find a free OPL channel.
			for (int i = 0; i < _numMelodicChannels; i++) {
				uint8 oplChannel = _melodicChannels[i];
				if (!_activeNotes[oplChannel].channelAllocated) {
					// Found a free channel. Allocate this.
					_activeNotes[oplChannel].channelAllocated = true;
					_activeNotes[oplChannel].source = source;
					_activeNotes[oplChannel].channel = channel;

					_channelAllocations[source][channel] = oplChannel;

					allocatedChannel = oplChannel;

					break;
				}
			}
			// If no free channel could be found, allocatedChannel will be 0xFF.
		}

		_allocationMutex.unlock();
	}

	return allocatedChannel;
}

void MidiDriver_ADLIB_Multisource::determineMelodicChannels() {
	if (_oplType == OPL::Config::kOpl2 || _oplType == OPL::Config::kDualOpl2) {
		_numMelodicChannels = OPL2_NUM_CHANNELS;
		if (_rhythmMode) {
			// Rhythm mode uses 3 OPL channels for rhythm instruments.
			_numMelodicChannels -= 3;
			_melodicChannels = MELODIC_CHANNELS_OPL2_RHYTHM;
		} else {
			// Use all available OPL channels as melodic channels.
			_melodicChannels = MELODIC_CHANNELS_OPL2;
		}
	} else {
		_numMelodicChannels = OPL3_NUM_CHANNELS;
		if (_rhythmMode) {
			_numMelodicChannels -= 3;
			_melodicChannels = MELODIC_CHANNELS_OPL3_RHYTHM;
		} else {
			_melodicChannels = MELODIC_CHANNELS_OPL3;
		}
	}
}

uint16 MidiDriver_ADLIB_Multisource::calculateFrequency(uint8 channel, uint8 source, uint8 note) {
	// Split note into octave and octave note.
	uint8 octaveNote = note % 12;
	uint8 octave = note / 12;

	// Calculate OPL octave (block) and frequency (F-num).
	uint8 block;
	uint32 oplFrequency;

	if (_accuracyMode == ACCURACY_MODE_SB16_WIN95) {
		// Frequency calculation using the algorithm of the Win95 SB16 driver.

		// Look up the octave note OPL frequency. These values assume octave 5.
		oplFrequency = OPL_NOTE_FREQUENCIES[octaveNote];
		// Correct for octaves other than 5 by doubling or halving the OPL
		// frequency for each octave higher or lower, respectively.
		if (octave > 5) {
			oplFrequency <<= (octave - 5);
		} else {
			oplFrequency >>= (5 - octave);
		}
		// The resulting value is likely larger than the 10 bit length of the
		// F-num in the OPL registers. This is correct later by increasing the
		// block.
		block = 1;
	} else {
		// Frequency calculation using a more accurate algorithm.

		// Calculate the note frequency in Hertz by relating it to a known
		// frequency (in this case A4 (0x45) = 440 Hz). Formula is
		// freq * 2 ^ (semitones / 12).
		float noteFrequency = 440 * (pow(2, (note - 0x45) / 12.0f));
		// Convert the frequency in Hz to the format used by the OPL registers.
		// Note that the resulting value is double the actual frequency because
		// of the use of block 0 (which halves the frequency). This allows for
		// slightly higher precision in the pitch bend calculation.
		oplFrequency = round(noteFrequency * _oplFrequencyConversionFactor);
		block = 0;
	}

	// Calculate and apply pitch bend and tuning.
	oplFrequency += calculatePitchBend(channel, source, oplFrequency);

	// Shift the frequency down to the 10 bits used by the OPL registers.
	// Increase the block to compensate.
	while (oplFrequency > 0x3FF) {
		oplFrequency >>= 1;
		block++;
	}
	// Maximum supported block value is 7, so clip higher values. The highest
	// MIDI notes exceed the maximum OPL frequency, so these will be transposed
	// down 1 or 2 octaves.
	block = MIN(block, (uint8)7);

	// Combine the block and frequency in the OPL Ax and Bx register format.
	return oplFrequency | (block << 10);
}

int32 MidiDriver_ADLIB_Multisource::calculatePitchBend(uint8 channel, uint8 source, uint16 oplFrequency) {
	int32 pitchBend;

	if (_accuracyMode == ACCURACY_MODE_SB16_WIN95) {
		// Pitch bend calculation using the algorithm of the Win95 SB16 driver.

		// Convert the 14 bit MIDI pitch bend value to a 16 bit signed value.
		// WORKAROUND The conversion to signed in the Win95 SB16 driver is
		// slightly inaccurate and causes minimum pitch bend to underflow to
		// maximum pitch bend. This is corrected here by clipping the result to
		// the int16 minimum value.
		pitchBend = MAX(-0x8000, (_controlData[source][channel].pitchBend << 2) - 0x8001);
		// Scale pitch bend by 0x1F (up) or 0x1B (down), which is a fixed
		// distance of 2 semitones up or down (pitch bend sensitivity is not
		// supported by this algorithm).
		pitchBend *= (pitchBend > 0 ? 0x1F : 0x1B);
		pitchBend >>= 8;
		// Scale by the OPL note frequency.
		pitchBend *= oplFrequency;
		pitchBend >>= 0xF;
	} else {
		// Pitch bend calculation using a more accurate algorithm.

		// Calculate the pitch bend in cents.
		int16 signedPitchBend = _controlData[source][channel].pitchBend - 0x2000;
		uint16 pitchBendSensitivityCents = (_controlData[source][channel].pitchBendSensitivity * 100) +
			_controlData[source][channel].pitchBendSensitivityCents;
		// Pitch bend upwards has 1 less resolution than downwards
		// (0x2001-0x3FFF vs 0x0000-0x1FFF).
		float pitchBendCents = signedPitchBend * pitchBendSensitivityCents /
			(signedPitchBend > 0 ? 8191.0f : 8192.0f);
		// Calculate the tuning in cents.
		float tuningCents = ((_controlData[source][channel].masterTuningCoarse - 0x40) * 100) +
			((_controlData[source][channel].masterTuningFine - 0x2000) * 100 / 8192.0f);

		// Calculate pitch bend (formula is freq * 2 ^ (cents / 1200)).
		// Note that if unrealistically large values for pitch bend sensitivity
		// and/or tuning are used, the result could overflow int32. Since this is
		// far into the ultrasonic frequencies, this should not occur in practice.
		pitchBend = round(oplFrequency * pow(2, (pitchBendCents + tuningCents) / 1200.0f) - oplFrequency);
	}

	return pitchBend;
}

uint8 MidiDriver_ADLIB_Multisource::calculateVolume(uint8 channel, uint8 source, uint8 velocity, OplInstrumentDefinition &instrumentDef, uint8 operatorNum) {
	// Get the volume (level) for this operator from the instrument definition.
	uint8 operatorDefVolume = instrumentDef.getOperatorDefinition(operatorNum).level & 0x3F;

	// Determine if volume settings should be applied to this operator. Carrier
	// operators in FM synthesis and all operators in additive synthesis need
	// to have volume settings applied; modulator operators just use the
	// instrument definition volume.
	bool applyVolume = false;
	if (instrumentDef.rhythmType != RHYTHM_TYPE_UNDEFINED) {
		applyVolume = (instrumentDef.rhythmType != RHYTHM_TYPE_BASS_DRUM || operatorNum == 1);
	} else if (instrumentDef.fourOperator) {
		// 4 operator instruments have 4 different operator connections.
		uint8 connection = (instrumentDef.connectionFeedback0 & 0x01) | ((instrumentDef.connectionFeedback1 & 0x01) << 1);
		switch (connection) {
		case 0x00:
			// 4FM
			// Operator 3 is a carrier.
			applyVolume = (operatorNum == 3);
			break;
		case 0x01:
			// 1ADD+3FM
			// Operator 0 is additive and operator 3 is a carrier.
			applyVolume = (operatorNum == 0 || operatorNum == 3);
			break;
		case 0x10:
			// 2FM+2FM
			// Operators 1 and 3 are carriers.
			applyVolume = (operatorNum == 1 || operatorNum == 3);
			break;
		case 0x11:
			// 1ADD+2FM+1ADD
			// Operators 0 and 3 are additive and operator 2 is a carrier.
			applyVolume = (operatorNum == 0 || operatorNum == 2 || operatorNum == 3);
			break;
		default:
			// Should not happen.
			applyVolume = false;
		}
	} else {
		// 2 operator instruments have 2 different operator connections:
		// additive (0x01) or FM (0x00) synthesis.  Carrier operators in FM
		// synthesis and all operators in additive synthesis need to have
		// volume settings applied; modulator operators just use the instrument
		// definition volume. In FM synthesis connection, operator 1 is a
		// carrier.
		applyVolume = (instrumentDef.connectionFeedback0 & 0x01) == 0x01 || operatorNum == 1;
	}
	if (!applyVolume)
		// No need to apply volume settings; just use the instrument definition
		// operator volume.
		return operatorDefVolume;

	// Calculate the volume based on note velocity, channel volume and
	// expression.
	uint8 unscaledVolume = calculateUnscaledVolume(channel, source, velocity, instrumentDef, operatorNum);

	uint8 invertedVolume = 0x3F - unscaledVolume;
	// Scale by source volume.
	invertedVolume = (invertedVolume * _sources[source].volume) / _sources[source].neutralVolume;
	if (_userVolumeScaling) {
		if (_userMute) {
			invertedVolume = 0;
		} else {
			// Scale by user volume.
			uint16 userVolume = (_sources[source].type == SOURCE_TYPE_SFX ? _userSfxVolume : _userMusicVolume); // Treat SOURCE_TYPE_UNDEFINED as music
			invertedVolume = (invertedVolume * userVolume) >> 8;
		}
	}
	// Source volume scaling might clip volume, so reduce to maximum.
	invertedVolume = MIN((uint8)0x3F, invertedVolume);
	uint8 scaledVolume = 0x3F - invertedVolume;

	return scaledVolume;
}

uint8 MidiDriver_ADLIB_Multisource::calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, OplInstrumentDefinition &instrumentDef, uint8 operatorNum) {
	uint8 unscaledVolume;
	// Get the volume (level) for this operator from the instrument definition.
	uint8 operatorVolume = instrumentDef.getOperatorDefinition(operatorNum).level & 0x3F;

	if (_accuracyMode == ACCURACY_MODE_SB16_WIN95) {
		// Volume calculation using the algorithm of the Win95 SB16 driver.

		// Shift velocity and channel volume to a 5 bit value and look up the OPL
		// volume value.
		uint8 velocityVolume = OPL_VOLUME_LOOKUP[velocity >> 2];
		uint8 channelVolume = OPL_VOLUME_LOOKUP[_controlData[source][channel].volume >> 2];
		// Add velocity and channel OPL volume to get the unscaled volume. The
		// operator volume is an additional (negative) volume adjustment to balance
		// the instruments.
		// Note that large OPL volume values can exceed the 0x3F limit; this is
		// handled below. (0x3F means maximum attenuation - no sound.)
		unscaledVolume = velocityVolume + channelVolume + operatorVolume;
	} else {
		// Volume calculation using an algorithm more accurate to the General MIDI
		// standard.

		// Calculate the volume in dB according to the GM formula:
		// 40 log(velocity * volume * expression / 127 ^ 3)
		// Note that velocity is not specified in detail in the MIDI standards;
		// we use the same volume curve as channel volume and expression.
		float volumeDb = 40 * log10((velocity * _controlData[source][channel].volume * _controlData[source][channel].expression) / 2048383.0f);
		// Convert to OPL volume (every unit is 0.75 dB attenuation). The
		// operator volume is an additional (negative) volume adjustment to balance
		// the instruments.
		unscaledVolume = volumeDb / -0.75f + operatorVolume;
	}

	// Clip the volume to the maximum value.
	return MIN((uint8)0x3F, unscaledVolume);
}

uint8 MidiDriver_ADLIB_Multisource::calculatePanning(uint8 channel, uint8 source) {
	if (_oplType != OPL::Config::kOpl3)
		return 0;

	// MIDI panning is converted to OPL panning using these values:
	// 0x00...L...0x2F 0x30...C...0x50 0x51...R...0x7F
	if (_controlData[source][channel].panning <= OPL_MIDI_PANNING_LEFT_LIMIT) {
		return OPL_PANNING_LEFT;
	} else if (_controlData[source][channel].panning >= OPL_MIDI_PANNING_RIGHT_LIMIT) {
		return OPL_PANNING_RIGHT;
	} else {
		return OPL_PANNING_CENTER;
	}
}

void MidiDriver_ADLIB_Multisource::setRhythmMode(bool rhythmMode) {
	if (_rhythmMode == rhythmMode)
		return;

	_allocationMutex.lock();
	_activeNotesMutex.lock();

	if (!_rhythmMode && rhythmMode) {
		// Rhythm mode is turned on.

		// Reset the OPL channels that will be used for rhythm mode.
		for (int i = 6; i <= 8; i++) {
			writeKeyOff(i);
			for (int j = 0; j < MAXIMUM_SOURCES; j++) {
				_channelAllocations[j][i] = 0xFF;
			}
			_activeNotes[i].init();
		}
		// Initialize the rhythm note data.
		for (int i = 0; i < OPL_NUM_RHYTHM_INSTRUMENTS; i++) {
			_activeRhythmNotes[i].init();
		}
	} else if (_rhythmMode && !rhythmMode) {
		// Rhythm mode is turned off.
		// Turn off any active rhythm notes.
		for (int i = 0; i < OPL_NUM_RHYTHM_INSTRUMENTS; i++) {
			_activeRhythmNotes[i].noteActive = false;
		}
	}
	_rhythmMode = rhythmMode;

	determineMelodicChannels();
	writeRhythm();

	_activeNotesMutex.unlock();
	_allocationMutex.unlock();
}

uint16 MidiDriver_ADLIB_Multisource::determineOperatorRegisterOffset(uint8 oplChannel, uint8 operatorNum, OplInstrumentRhythmType rhythmType, bool fourOperator) {
	assert(!fourOperator || oplChannel < 6);
	assert(fourOperator || operatorNum < 2);

	uint16 offset = 0;
	if (rhythmType != RHYTHM_TYPE_UNDEFINED) {
		// Look up the offset for rhythm instruments.
		offset = OPL_REGISTER_RHYTHM_OFFSETS[rhythmType - 1];
		if (rhythmType == RHYTHM_TYPE_BASS_DRUM && operatorNum == 1)
			// Bass drum is the only rhythm instrument with 2 operators.
			offset += 3;
	} else if (fourOperator) {
		// 4 operator register offset for each channel and operator:
		// 
		// Channel  | 0 | 1 | 2 | 0 | 1 | 2 | 0 | 1 | 2 | 0 | 1 | 2 |
		// Operator | 0         | 1         | 2         | 3         |
		// Register | 0 | 1 | 2 | 3 | 4 | 5 | 8 | 9 | A | B | C | D |
		//
		// Channels 3-5 are in the second register set (add 0x100 to the register).
		offset += (oplChannel / 3) * OPL_REGISTER_SET_2_OFFSET;
		offset += operatorNum / 2 * 8;
		offset += (operatorNum % 2) * 3;
		offset += oplChannel % 3;
	} else {
		// 2 operator register offset for each channel and operator:
		//
		// Channel  | 0 | 1 | 2 | 0 | 1 | 2 | 3 | 4 | 5 | 3 | 4 | 5 | 6 | 7 | 8 | 6 | 7 | 8 |
		// Operator | 0         | 1         | 0         | 1         | 0         | 1         |
		// Register | 0 | 1 | 2 | 3 | 4 | 5 | 8 | 9 | A | B | C | D |10 |11 |12 |13 |14 |15 |
		//
		// Channels 9-17 are in the second register set (add 0x100 to the register).
		offset += (oplChannel / 9) * OPL_REGISTER_SET_2_OFFSET;
		offset += (oplChannel % 9) / 3 * 8;
		offset += (oplChannel % 9) % 3;
		offset += operatorNum * 3;
	}

	return offset;
}

uint16 MidiDriver_ADLIB_Multisource::determineChannelRegisterOffset(uint8 oplChannel, bool fourOperator) {
	assert(!fourOperator || oplChannel < 6);

	// In 4 operator mode, only the first three channel registers are used in
	// each register set.
	uint8 numChannelsPerSet = fourOperator ? 3 : 9;
	uint16 offset = (oplChannel / numChannelsPerSet) * OPL_REGISTER_SET_2_OFFSET;
	return offset + (oplChannel % numChannelsPerSet);
}

void MidiDriver_ADLIB_Multisource::writeInstrument(uint8 oplChannel, InstrumentInfo instrument) {
	ActiveNote *activeNote = (instrument.instrumentDef->rhythmType == RHYTHM_TYPE_UNDEFINED ? &_activeNotes[oplChannel] : &_activeRhythmNotes[instrument.instrumentDef->rhythmType - 1]);
	activeNote->instrumentDef = instrument.instrumentDef;

	// Calculate operator volumes and write operator definitions to
	// the OPL registers.
	for (int i = 0; i < instrument.instrumentDef->getNumberOfOperators(); i++) {
		uint16 operatorOffset = determineOperatorRegisterOffset(oplChannel, i, instrument.instrumentDef->rhythmType, instrument.instrumentDef->fourOperator);
		const OplInstrumentOperatorDefinition &operatorDef = instrument.instrumentDef->getOperatorDefinition(i);
		writeRegister(OPL_REGISTER_BASE_FREQMULT_MISC + operatorOffset, operatorDef.freqMultMisc);
		writeVolume(oplChannel, i, instrument.instrumentDef->rhythmType);
		writeRegister(OPL_REGISTER_BASE_DECAY_ATTACK + operatorOffset, operatorDef.decayAttack);
		writeRegister(OPL_REGISTER_BASE_RELEASE_SUSTAIN + operatorOffset, operatorDef.releaseSustain);
		writeRegister(OPL_REGISTER_BASE_WAVEFORMSELECT + operatorOffset, operatorDef.waveformSelect);
	}

	// Determine and write panning and write feedback and connection.
	writePanning(oplChannel, instrument.instrumentDef->rhythmType);
}

void MidiDriver_ADLIB_Multisource::writeKeyOff(uint8 oplChannel, OplInstrumentRhythmType rhythmType, bool forceWrite) {
	_activeNotesMutex.lock();

	ActiveNote *activeNote = nullptr;
	if (rhythmType == RHYTHM_TYPE_UNDEFINED) {
		// Melodic instrument.
		activeNote = &_activeNotes[oplChannel];
		// Rewrite the current Bx register value with the key on bit set to 0.
		writeRegister(OPL_REGISTER_BASE_FNUMHIGH_BLOCK_KEYON + determineChannelRegisterOffset(oplChannel),
			(activeNote->oplFrequency >> 8) & OPL_MASK_FNUMHIGH_BLOCK, forceWrite);
	} else {
		// Rhythm instrument.
		activeNote = &_activeRhythmNotes[rhythmType - 1];
	}

	// Update the active note data.
	activeNote->noteActive = false;
	activeNote->noteSustained = false;
	// Register the current note counter value when turning off a note.
	activeNote->noteCounterValue = _noteCounter;

	if (rhythmType != RHYTHM_TYPE_UNDEFINED) {
		// Rhythm instrument. Rewrite the rhythm register.
		writeRhythm();
	}

	_activeNotesMutex.unlock();
}

void MidiDriver_ADLIB_Multisource::writeRhythm(bool forceWrite) {
	uint8 value = (_modulationDepth << 7) | (_vibratoDepth << 6) | ((_rhythmMode ? 1 : 0) << 5);
	if (_rhythmMode) {
		// Add the key on bits for each rhythm instrument.
		for (int i = 0; i < OPL_NUM_RHYTHM_INSTRUMENTS; i++) {
			value |= ((_activeRhythmNotes[i].noteActive ? 1 : 0) << i);
		}
	}

	writeRegister(OPL_REGISTER_RHYTHM, value, forceWrite);
	if (_oplType == OPL::Config::kDualOpl2) {
		writeRegister(OPL_REGISTER_RHYTHM | OPL_REGISTER_SET_2_OFFSET, value, forceWrite);
	}
}

void MidiDriver_ADLIB_Multisource::writeVolume(uint8 oplChannel, uint8 operatorNum, OplInstrumentRhythmType rhythmType) {
	ActiveNote *activeNote = (rhythmType == RHYTHM_TYPE_UNDEFINED ? &_activeNotes[oplChannel] : &_activeRhythmNotes[rhythmType - 1]);

	// Calculate operator volume.
	uint16 registerOffset = determineOperatorRegisterOffset(
		oplChannel, operatorNum, rhythmType, activeNote->instrumentDef->fourOperator);
	const OplInstrumentOperatorDefinition &operatorDef =
		activeNote->instrumentDef->getOperatorDefinition(operatorNum);
	uint8 level = calculateVolume(activeNote->channel, activeNote->source, activeNote->velocity,
		*activeNote->instrumentDef, operatorNum);

	// Add key scaling level from the operator definition to the calculated
	// level.
	writeRegister(OPL_REGISTER_BASE_LEVEL + registerOffset, level | (operatorDef.level & ~OPL_MASK_LEVEL));
}

void MidiDriver_ADLIB_Multisource::writePanning(uint8 oplChannel, OplInstrumentRhythmType rhythmType) {
	ActiveNote *activeNote;
	if (rhythmType != RHYTHM_TYPE_UNDEFINED) {
		activeNote = &_activeRhythmNotes[rhythmType - 1];
		oplChannel = OPL_RHYTHM_INSTRUMENT_CHANNELS[rhythmType - 1];
	} else {
		activeNote = &_activeNotes[oplChannel];
	}

	// Calculate channel panning.
	uint16 registerOffset = determineChannelRegisterOffset(
		oplChannel, activeNote->instrumentDef->fourOperator);
	uint8 panning = calculatePanning(activeNote->channel, activeNote->source);

	// Add connection and feedback from the instrument definition to the
	// calculated panning.
	writeRegister(OPL_REGISTER_BASE_CONNECTION_FEEDBACK_PANNING + registerOffset,
		panning | (activeNote->instrumentDef->connectionFeedback0 & ~OPL_MASK_PANNING));
	if (activeNote->instrumentDef->fourOperator)
		// TODO Not sure if panning is necessary here.
		writeRegister(OPL_REGISTER_BASE_CONNECTION_FEEDBACK_PANNING + registerOffset + 3,
			panning | (activeNote->instrumentDef->connectionFeedback1 & ~OPL_MASK_PANNING));
}

void MidiDriver_ADLIB_Multisource::writeFrequency(uint8 oplChannel, OplInstrumentRhythmType rhythmType) {
	_activeNotesMutex.lock();

	ActiveNote *activeNote;
	if (rhythmType != RHYTHM_TYPE_UNDEFINED) {
		activeNote = &_activeRhythmNotes[rhythmType - 1];
		oplChannel = OPL_RHYTHM_INSTRUMENT_CHANNELS[rhythmType - 1];
	} else {
		activeNote = &_activeNotes[oplChannel];
	}

	// Calculate the frequency.
	uint16 channelOffset = determineChannelRegisterOffset(oplChannel, activeNote->instrumentDef->fourOperator);
	uint16 frequency = calculateFrequency(activeNote->channel, activeNote->source, activeNote->oplNote);
	activeNote->oplFrequency = frequency;

	// Write the low 8 frequency bits.
	writeRegister(OPL_REGISTER_BASE_FNUMLOW + channelOffset, frequency & 0xFF);
	// Write the high 2 frequency bits and block and add the key on bit.
	writeRegister(OPL_REGISTER_BASE_FNUMHIGH_BLOCK_KEYON + channelOffset,
		(frequency >> 8) | (rhythmType == RHYTHM_TYPE_UNDEFINED && activeNote->noteActive ? OPL_MASK_KEYON : 0));

	_activeNotesMutex.unlock();
}

void MidiDriver_ADLIB_Multisource::writeRegister(uint16 reg, uint8 value, bool forceWrite) {
	//debug("Writing register %X %X", reg, value);

	// Write the value to the register if it is a timer register, if forceWrite
	// is specified or if the new register value is different from the current
	// value.
	if ((reg >= 1 && reg <= 3) || (_oplType == OPL::Config::kDualOpl2 && reg >= 0x101 && reg <= 0x103) || 
			forceWrite || _shadowRegisters[reg] != value) {
		_shadowRegisters[reg] = value;
		_opl->writeReg(reg, value);
	}
}
