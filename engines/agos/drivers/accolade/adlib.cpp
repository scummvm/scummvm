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

#include "agos/drivers/accolade/mididriver.h"
#include "agos/drivers/accolade/adlib.h"

#include "audio/fmopl.h"
#include "audio/mididrv.h"

namespace AGOS {

#define AGOS_ADLIB_VOICES_MELODIC_COUNT 6
#define AGOS_ADLIB_VOICES_PERCUSSION_START 6
#define AGOS_ADLIB_VOICES_PERCUSSION_COUNT 5
#define AGOS_ADLIB_VOICES_PERCUSSION_CYMBAL 9

// 5 instruments on top of the regular MIDI ones
// used by the MUSIC.DRV variant for percussion instruments
#define AGOS_ADLIB_EXTRA_INSTRUMENT_COUNT 5

const byte operator1Register[AGOS_ADLIB_VOICES_COUNT] = {
	0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x14, 0x12, 0x15, 0x11
};

const byte operator2Register[AGOS_ADLIB_VOICES_COUNT] = {
	0x03, 0x04, 0x05, 0x0B, 0x0C, 0x0D, 0x13, 0xFF, 0xFF, 0xFF, 0xFF
};

// percussion:
//  voice  6 - base drum - also uses operator 13h
//  voice  7 - snare drum
//  voice  8 - tom tom
//  voice  9 - cymbal
//  voice 10 - hi hat
const byte percussionBits[AGOS_ADLIB_VOICES_PERCUSSION_COUNT] = {
	0x10, 0x08, 0x04, 0x02, 0x01
};

// hardcoded, dumped from Accolade music system
// same for INSTR.DAT + MUSIC.DRV, except that MUSIC.DRV does the lookup differently
const byte percussionKeyNoteChannelTable[] = {
	0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x0A, 0x08, 0x0A, 0x08,
	0x0A, 0x08, 0x08, 0x09, 0x08, 0x09, 0x0F, 0x0F, 0x0A, 0x0F,
	0x0A, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x0A, 0x0F, 0x0F, 0x08, 0x0F, 0x08
};

// hardcoded, dumped from Accolade music system (INSTR.DAT variant)
const uint16 frequencyLookUpTable[12] = {
	0x02B2, 0x02DB, 0x0306, 0x0334, 0x0365, 0x0399, 0x03CF,
	0xFE05, 0xFE23, 0xFE44, 0xFE67, 0xFE8B
};

// hardcoded, dumped from Accolade music system (MUSIC.DRV variant)
const uint16 frequencyLookUpTableMusicDrv[12] = {
	0x0205, 0x0223, 0x0244, 0x0267, 0x028B, 0x02B2, 0x02DB,
	0x0306, 0x0334, 0x0365, 0x0399, 0x03CF
};

//
// Accolade adlib music driver
//
// Remarks:
//
// There are at least 2 variants of this sound system.
// One for the games Elvira 1 + Elvira 2
// It seems it was also used for the game "Altered Destiny"
// Another one for the games Waxworks + Simon, the Sorcerer 1 Demo
//
// First one uses the file INSTR.DAT for instrument data, channel mapping etc.
// Second one uses the file MUSIC.DRV, which actually contains driver code + instrument data + channel mapping, etc.
//
// The second variant supported dynamic channel allocation for the FM voice channels, but this
// feature was at least definitely disabled for Simon, the Sorcerer 1 demo and for the Waxworks demo too.
//
// I have currently not implemented dynamic channel allocation.

MidiDriver_Accolade_AdLib::MidiDriver_Accolade_AdLib()
		: _masterVolume(15), _opl(0),
		  _adlibTimerProc(0), _adlibTimerParam(0), _isOpen(false) {
	memset(_channelMapping, 0, sizeof(_channelMapping));
	memset(_instrumentMapping, 0, sizeof(_instrumentMapping));
	memset(_instrumentVolumeAdjust, 0, sizeof(_instrumentVolumeAdjust));
	memset(_percussionKeyNoteMapping, 0, sizeof(_percussionKeyNoteMapping));

	_instrumentTable = NULL;
	_instrumentCount = 0;
	_musicDrvMode = false;
	_percussionReg = 0x20;
}

MidiDriver_Accolade_AdLib::~MidiDriver_Accolade_AdLib() {
	if (_instrumentTable) {
		delete[] _instrumentTable;
		_instrumentCount = 0;
	}
}

int MidiDriver_Accolade_AdLib::open() {
//	debugC(kDebugLevelAdLibDriver, "AdLib: starting driver");

	_opl = OPL::Config::create(OPL::Config::kOpl2);

	if (!_opl)
		return -1;

	_opl->init();

	_isOpen = true;

	_opl->start(new Common::Functor0Mem<void, MidiDriver_Accolade_AdLib>(this, &MidiDriver_Accolade_AdLib::onTimer));

	resetAdLib();

	// Finally set up default instruments
	for (byte FMvoiceNr = 0; FMvoiceNr < AGOS_ADLIB_VOICES_COUNT; FMvoiceNr++) {
		if (FMvoiceNr < AGOS_ADLIB_VOICES_PERCUSSION_START) {
			// Regular FM voices with instrument 0
			programChangeSetInstrument(FMvoiceNr, 0, 0);
		} else {
			byte percussionInstrument;
			if (!_musicDrvMode) {
				// INSTR.DAT: percussion voices with instrument 1, 2, 3, 4 and 5
				percussionInstrument = FMvoiceNr - AGOS_ADLIB_VOICES_PERCUSSION_START + 1;
			} else {
				// MUSIC.DRV: percussion voices with instrument 0x80, 0x81, 0x82, 0x83 and 0x84
				percussionInstrument = FMvoiceNr - AGOS_ADLIB_VOICES_PERCUSSION_START + 0x80;
			}
			programChangeSetInstrument(FMvoiceNr, percussionInstrument, percussionInstrument);
		}
	}

	// driver initialization does this here:
	// INSTR.DAT
	// noteOn(9, 0x29, 0);
	// noteOff(9, 0x26, false);
	// MUSIC.DRV
	// noteOn(9, 0x26, 0);
	// noteOff(9, 0x26, false);

	return 0;
}

void MidiDriver_Accolade_AdLib::close() {
	delete _opl;
	_isOpen = false;
}

void MidiDriver_Accolade_AdLib::setVolume(byte volume) {
	// Set the master volume in range from -128 to 127
	_masterVolume = CLIP<int>(-128 + volume, -128, 127);
	for (int i = 0; i < AGOS_ADLIB_VOICES_COUNT; i++) {
		// Adjust channel volume with the master volume and re-set registers
		byte adjustedVelocity = _channels[i].velocity * ((float) (128 + _masterVolume) / 128);
		noteOnSetVolume(i, 1, adjustedVelocity);
		if (i <= AGOS_ADLIB_VOICES_PERCUSSION_START) {
			// Set second operator for FM voices + first percussion
			noteOnSetVolume(i, 2, adjustedVelocity);
		}
	}
}

void MidiDriver_Accolade_AdLib::onTimer() {
	if (_adlibTimerProc)
		(*_adlibTimerProc)(_adlibTimerParam);
}

void MidiDriver_Accolade_AdLib::resetAdLib() {
	// The original driver sent 0x00 to register 0x00 up to 0xF5
	setRegister(0xBD, 0x00); // Disable rhythm

	// reset FM voice instrument data
	resetAdLibOperatorRegisters(0x20, 0);
	resetAdLibOperatorRegisters(0x60, 0);
	resetAdLibOperatorRegisters(0x80, 0);
	resetAdLibFMVoiceChannelRegisters(0xA0, 0);
	resetAdLibFMVoiceChannelRegisters(0xB0, 0);
	resetAdLibFMVoiceChannelRegisters(0xC0, 0);
	resetAdLibOperatorRegisters(0xE0, 0);
	resetAdLibOperatorRegisters(0x40, 0x3F); // original driver sent 0x00

	setRegister(0x01, 0x20); // enable waveform control on both operators
	setRegister(0x04, 0x60); // Timer control

	setRegister(0x08, 0);    // select FM music mode
	setRegister(0xBD, 0x20); // Enable rhythm

	// reset our percussion register
	_percussionReg = 0x20;
}

void MidiDriver_Accolade_AdLib::resetAdLibOperatorRegisters(byte baseRegister, byte value) {
	byte operatorIndex;

	for (operatorIndex = 0; operatorIndex < 0x16; operatorIndex++) {
		switch (operatorIndex) {
		case 0x06:
		case 0x07:
		case 0x0E:
		case 0x0F:
			break;
		default:
			setRegister(baseRegister + operatorIndex, value);
		}
	}
}

void MidiDriver_Accolade_AdLib::resetAdLibFMVoiceChannelRegisters(byte baseRegister, byte value) {
	byte FMvoiceChannel;

	for (FMvoiceChannel = 0; FMvoiceChannel < AGOS_ADLIB_VOICES_COUNT; FMvoiceChannel++) {
		setRegister(baseRegister + FMvoiceChannel, value);
	}
}

// MIDI messages can be found at http://www.midi.org/techspecs/midimessages.php
void MidiDriver_Accolade_AdLib::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	byte mappedChannel    = _channelMapping[channel];
	byte mappedInstrument = 0;

	// Ignore everything that is outside of our channel range
	if (mappedChannel >= AGOS_ADLIB_VOICES_COUNT)
		return;

	switch (command) {
	case 0x80:
		noteOff(mappedChannel, op1, false);
		break;
	case 0x90:
		// Convert noteOn with velocity 0 to a noteOff
		if (op2 == 0)
			return noteOff(mappedChannel, op1, false);

		noteOn(mappedChannel, op1, op2);
		break;
	case 0xb0: // Control change
		// Doesn't seem to be implemented
		break;
	case 0xc0: // Program Change
		mappedInstrument = _instrumentMapping[op1];
		programChange(mappedChannel, mappedInstrument, op1);
		break;
	case 0xa0: // Polyphonic key pressure (aftertouch)
	case 0xd0: // Channel pressure (aftertouch)
		// Aftertouch doesn't seem to be implemented
		break;
	case 0xe0:
		// No pitch bend change
		break;
	case 0xf0: // SysEx
		warning("ADLIB: SysEx: %x", b);
		break;
	default:
		warning("ADLIB: Unknown event %02x", command);
	}
}

void MidiDriver_Accolade_AdLib::setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) {
	_adlibTimerProc = timerProc;
	_adlibTimerParam = timerParam;
}

void MidiDriver_Accolade_AdLib::noteOn(byte FMvoiceChannel, byte note, byte velocity) {
	byte adjustedNote     = note;
	byte adjustedVelocity = velocity;
	byte regValueA0h      = 0;
	byte regValueB0h      = 0;

	// adjust velocity
	int16 channelVolumeAdjust = _channels[FMvoiceChannel].volumeAdjust;
	channelVolumeAdjust += adjustedVelocity;
	channelVolumeAdjust = CLIP<int16>(channelVolumeAdjust, 0, 0x7F);
	
	// adjust velocity with the master volume
	byte volumeAdjust = adjustedVelocity * ((float) (128 + _masterVolume) / 128);
	
	adjustedVelocity = volumeAdjust;

	if (!_musicDrvMode) {
		// INSTR.DAT
		// force note-off
		noteOff(FMvoiceChannel, note, true);

	} else {
		// MUSIC.DRV
		if (FMvoiceChannel < AGOS_ADLIB_VOICES_PERCUSSION_START) {
			// force note-off, but only for actual FM voice channels
			noteOff(FMvoiceChannel, note, true);
		}
	}

	if (FMvoiceChannel != 9) {
		// regular FM voice

		if (!_musicDrvMode) {
			// INSTR.DAT: adjust key note
			while (adjustedNote < 24)
				adjustedNote += 12;
			adjustedNote -= 12;
		}

	} else {
		// percussion channel
		// MUSIC.DRV variant didn't do this adjustment, it directly used a pointer
		adjustedNote -= 36;
		if (adjustedNote > 40) { // Security check
			warning("ADLIB: bad percussion channel note");
			return;
		}

		byte percussionChannel = percussionKeyNoteChannelTable[adjustedNote];
		if (percussionChannel >= AGOS_ADLIB_VOICES_COUNT)
			return; // INSTR.DAT variant checked for ">" instead of ">=", which seems to have been a bug

		// Map the keynote accordingly
		adjustedNote = _percussionKeyNoteMapping[adjustedNote];
		// Now overwrite the FM voice channel
		FMvoiceChannel = percussionChannel;
	}

	if (!_musicDrvMode) {
		// INSTR.DAT

		// Save this key note
		_channels[FMvoiceChannel].currentNote = adjustedNote;

		adjustedVelocity += 24;
		if (adjustedVelocity > 120)
			adjustedVelocity = 120;
		adjustedVelocity = adjustedVelocity >> 1; // divide by 2

	} else {
		// MUSIC.DRV
		adjustedVelocity = adjustedVelocity >> 1; // divide by 2
	}

	// Save velocity in the case volume will need to be changed
	_channels[FMvoiceChannel].velocity = adjustedVelocity;
	// Set volume of voice channel
	noteOnSetVolume(FMvoiceChannel, 1, adjustedVelocity);
	if (FMvoiceChannel <= AGOS_ADLIB_VOICES_PERCUSSION_START) {
		// Set second operator for FM voices + first percussion
		noteOnSetVolume(FMvoiceChannel, 2, adjustedVelocity);
	}

	if (FMvoiceChannel >= AGOS_ADLIB_VOICES_PERCUSSION_START) {
		// Percussion
		byte percussionIdx = FMvoiceChannel - AGOS_ADLIB_VOICES_PERCUSSION_START;

		// Enable bit of the requested percussion type
		assert(percussionIdx < AGOS_ADLIB_VOICES_PERCUSSION_COUNT);
		_percussionReg |= percussionBits[percussionIdx];
		setRegister(0xBD, _percussionReg);
	}

	if (FMvoiceChannel < AGOS_ADLIB_VOICES_PERCUSSION_CYMBAL) {
		// FM voice, Base Drum, Snare Drum + Tom Tom
		byte adlibNote = adjustedNote;
		byte adlibOctave = 0;
		byte adlibFrequencyIdx = 0;
		uint16 adlibFrequency = 0;

		if (!_musicDrvMode) {
			// INSTR.DAT
			if (adlibNote >= 0x60)
				adlibNote = 0x5F;

			adlibOctave = (adlibNote / 12) - 1;
			adlibFrequencyIdx = adlibNote % 12;
			adlibFrequency = frequencyLookUpTable[adlibFrequencyIdx];

			if (adlibFrequency & 0x8000)
				adlibOctave++;
			if (adlibOctave & 0x80) {
				adlibOctave++;
				adlibFrequency = adlibFrequency >> 1;
			}

		} else {
			// MUSIC.DRV variant
			if (adlibNote >= 19)
				adlibNote -= 19;

			adlibOctave = (adlibNote / 12);
			adlibFrequencyIdx = adlibNote % 12;
			// additional code, that will lookup octave and do a multiplication with it
			// noteOn however calls the frequency calculation in a way that it multiplies with 0
			adlibFrequency = frequencyLookUpTableMusicDrv[adlibFrequencyIdx];
		}

		regValueA0h = adlibFrequency & 0xFF;
		regValueB0h = ((adlibFrequency & 0x300) >> 8) | (adlibOctave << 2);
		if (FMvoiceChannel < AGOS_ADLIB_VOICES_PERCUSSION_START) {
			// set Key-On flag for regular FM voices, but not for percussion
			regValueB0h |= 0x20;
		}

		setRegister(0xA0 + FMvoiceChannel, regValueA0h);
		setRegister(0xB0 + FMvoiceChannel, regValueB0h);
		_channels[FMvoiceChannel].currentA0hReg = regValueA0h;
		_channels[FMvoiceChannel].currentB0hReg = regValueB0h;

		if (_musicDrvMode) {
			// MUSIC.DRV
			if (FMvoiceChannel < AGOS_ADLIB_VOICES_MELODIC_COUNT) {
				_channels[FMvoiceChannel].currentNote = adjustedNote;
			}
		}
	}
}

// 100% the same for INSTR.DAT and MUSIC.DRV variants
// except for a bug, that was introduced for MUSIC.DRV
void MidiDriver_Accolade_AdLib::noteOnSetVolume(byte FMvoiceChannel, byte operatorNr, byte adjustedVelocity) {
	byte operatorReg = 0;
	byte regValue40h = 0;
	const InstrumentEntry *curInstrument = NULL;

	regValue40h = (63 - adjustedVelocity) & 0x3F;

	if ((operatorNr == 1) && (FMvoiceChannel <= AGOS_ADLIB_VOICES_PERCUSSION_START)) {
		// first operator of FM voice channels or first percussion channel
		curInstrument = _channels[FMvoiceChannel].currentInstrumentPtr;
		if (!(curInstrument->regC0 & 0x01)) { // check, if both operators produce sound
			// only one does, instrument wants fixed volume
			if (operatorNr == 1) {
				regValue40h = curInstrument->reg40op1;
			} else {
				regValue40h = curInstrument->reg40op2;
			}

			// not sure, if we are supposed to implement these bugs, or not
#if 0
			if (!_musicDrvMode) {
				// Table is 16 bytes instead of 18 bytes
				if ((FMvoiceChannel == 7) || (FMvoiceChannel == 9)) {
					regValue40h = 0;
					warning("volume set bug (original)");
				}
			}
			if (_musicDrvMode) {
				// MUSIC.DRV variant has a bug, which will overwrite these registers
				// for all operators above 11 / 0Bh, which means percussion will always
				// get a value of 0 (the table holding those bytes was 12 bytes instead of 18
				if (FMvoiceChannel >= AGOS_ADLIB_VOICES_PERCUSSION_START) {
					regValue40h = 0;
					warning("volume set bug (original)");
				}
			}
#endif
		}
	}

	if (operatorNr == 1) {
		operatorReg = operator1Register[FMvoiceChannel];
	} else {
		operatorReg = operator2Register[FMvoiceChannel];
	}
	assert(operatorReg != 0xFF); // Security check
	setRegister(0x40 + operatorReg, regValue40h);
}

void MidiDriver_Accolade_AdLib::noteOff(byte FMvoiceChannel, byte note, bool dontCheckNote) {
	byte adjustedNote = note;
	byte regValueB0h = 0;

	if (FMvoiceChannel < AGOS_ADLIB_VOICES_PERCUSSION_START) {
		// regular FM voice

		if (!_musicDrvMode) {
			// INSTR.DAT: adjust key note
			while (adjustedNote < 24)
				adjustedNote += 12;
			adjustedNote -= 12;
		}

		if (!dontCheckNote) {
			// check, if current note is also the current actually playing channel note
			if (_channels[FMvoiceChannel].currentNote != adjustedNote)
				return; // not the same -> ignore this note off command
		}

		regValueB0h = _channels[FMvoiceChannel].currentB0hReg & 0xDF; // Remove "key on" bit
		setRegister(0xB0 + FMvoiceChannel, regValueB0h);

	} else {
		// percussion
		adjustedNote -= 36;
		if (adjustedNote > 40) { // Security check
			warning("ADLIB: bad percussion channel note");
			return;
		}

		byte percussionChannel = percussionKeyNoteChannelTable[adjustedNote];
		if (percussionChannel > AGOS_ADLIB_VOICES_COUNT)
			return;

		byte percussionIdx = percussionChannel - AGOS_ADLIB_VOICES_PERCUSSION_START;

		// Disable bit of the requested percussion type
		assert(percussionIdx < AGOS_ADLIB_VOICES_PERCUSSION_COUNT);
		_percussionReg &= ~percussionBits[percussionIdx];
		setRegister(0xBD, _percussionReg);
	}
}

void MidiDriver_Accolade_AdLib::programChange(byte FMvoiceChannel, byte mappedInstrumentNr, byte MIDIinstrumentNr) {
	if (mappedInstrumentNr >= _instrumentCount) {
		warning("ADLIB: tried to set non-existent instrument");
		return; // out of range
	}

	// setup instrument
	//warning("ADLIB: program change for FM voice channel %d, instrument id %d", FMvoiceChannel, mappedInstrumentNr);

	if (FMvoiceChannel < AGOS_ADLIB_VOICES_PERCUSSION_START) {
		// Regular FM voice
		programChangeSetInstrument(FMvoiceChannel, mappedInstrumentNr, MIDIinstrumentNr);

	} else {
		// Percussion
		// set default instrument (again)
		byte percussionInstrumentNr = 0;
		const InstrumentEntry *instrumentPtr;

		if (!_musicDrvMode) {
			// INSTR.DAT: percussion default instruments start at instrument 1
			percussionInstrumentNr = FMvoiceChannel - AGOS_ADLIB_VOICES_PERCUSSION_START + 1;
		} else {
			// MUSIC.DRV: percussion default instruments start at instrument 0x80
			percussionInstrumentNr = FMvoiceChannel - AGOS_ADLIB_VOICES_PERCUSSION_START + 0x80;
		}
		if (percussionInstrumentNr >= _instrumentCount) {
			warning("ADLIB: tried to set non-existent instrument");
			return;
		}
		instrumentPtr = &_instrumentTable[percussionInstrumentNr];
		_channels[FMvoiceChannel].currentInstrumentPtr = instrumentPtr;
		_channels[FMvoiceChannel].volumeAdjust         = _instrumentVolumeAdjust[percussionInstrumentNr];
	}
}

void MidiDriver_Accolade_AdLib::programChangeSetInstrument(byte FMvoiceChannel, byte mappedInstrumentNr, byte MIDIinstrumentNr) {
	const InstrumentEntry *instrumentPtr;
	byte op1Reg = 0;
	byte op2Reg = 0;

	if (mappedInstrumentNr >= _instrumentCount) {
		warning("ADLIB: tried to set non-existent instrument");
		return; // out of range
	}

	// setup instrument
	instrumentPtr = &_instrumentTable[mappedInstrumentNr];
	//warning("set instrument for FM voice channel %d, instrument id %d", FMvoiceChannel, mappedInstrumentNr);

	op1Reg = operator1Register[FMvoiceChannel];
	op2Reg = operator2Register[FMvoiceChannel];

	setRegister(0x20 + op1Reg, instrumentPtr->reg20op1);
	setRegister(0x40 + op1Reg, instrumentPtr->reg40op1);
	setRegister(0x60 + op1Reg, instrumentPtr->reg60op1);
	setRegister(0x80 + op1Reg, instrumentPtr->reg80op1);

	if (FMvoiceChannel <= AGOS_ADLIB_VOICES_PERCUSSION_START) {
		// set 2nd operator as well for FM voices and first percussion voice
		setRegister(0x20 + op2Reg, instrumentPtr->reg20op2);
		setRegister(0x40 + op2Reg, instrumentPtr->reg40op2);
		setRegister(0x60 + op2Reg, instrumentPtr->reg60op2);
		setRegister(0x80 + op2Reg, instrumentPtr->reg80op2);

		if (!_musicDrvMode) {
			// set Feedback / Algorithm as well
			setRegister(0xC0 + FMvoiceChannel, instrumentPtr->regC0);
		} else {
			if (FMvoiceChannel < AGOS_ADLIB_VOICES_PERCUSSION_START) {
				// set Feedback / Algorithm as well for regular FM voices only
				setRegister(0xC0 + FMvoiceChannel, instrumentPtr->regC0);
			}
		}
	}

	// Remember instrument
	_channels[FMvoiceChannel].currentInstrumentPtr = instrumentPtr;
	_channels[FMvoiceChannel].volumeAdjust         = _instrumentVolumeAdjust[MIDIinstrumentNr];
}

void MidiDriver_Accolade_AdLib::setRegister(int reg, int value) {
	_opl->writeReg(reg, value);
	//warning("OPL %x %x (%d)", reg, value, value);
}

uint32 MidiDriver_Accolade_AdLib::property(int prop, uint32 param) {
	return 0;
}

// Called right at the start, we get an INSTR.DAT entry
bool MidiDriver_Accolade_AdLib::setupInstruments(byte *driverData, uint16 driverDataSize, bool useMusicDrvFile) {
	uint16 channelMappingOffset         = 0;
	uint16 channelMappingSize           = 0;
	uint16 instrumentMappingOffset      = 0;
	uint16 instrumentMappingSize        = 0;
	uint16 instrumentVolumeAdjustOffset = 0;
	uint16 instrumentVolumeAdjustSize   = 0;
	uint16 keyNoteMappingOffset         = 0;
	uint16 keyNoteMappingSize           = 0;
	uint16 instrumentCount              = 0;
	uint16 instrumentDataOffset         = 0;
	uint16 instrumentDataSize           = 0;
	uint16 instrumentEntrySize          = 0;

	if (!useMusicDrvFile) {
		// INSTR.DAT: we expect at least 354 bytes
		if (driverDataSize < 354)
			return false;

		// Data is like this:
		// 128 bytes  instrument mapping
		// 128 bytes  instrument volume adjust (signed!)
		//  16 bytes  unknown
		//  16 bytes  channel mapping
		//  64 bytes  key note mapping (not used for MT32)
		//   1 byte   instrument count
		//   1 byte   bytes per instrument
		//   x bytes  no instruments used for MT32

		channelMappingOffset         = 256 + 16;
		channelMappingSize           = 16;
		instrumentMappingOffset      = 0;
		instrumentMappingSize        = 128;
		instrumentVolumeAdjustOffset = 128;
		instrumentVolumeAdjustSize   = 128;
		keyNoteMappingOffset         = 256 + 16 + 16;
		keyNoteMappingSize           = 64;

		byte instrDatInstrumentCount    = driverData[256 + 16 + 16 + 64];
		byte instrDatBytesPerInstrument = driverData[256 + 16 + 16 + 64 + 1];

		// We expect 9 bytes per instrument
		if (instrDatBytesPerInstrument != 9)
			return false;
		// And we also expect at least one adlib instrument
		if (!instrDatInstrumentCount)
			return false;

		instrumentCount      = instrDatInstrumentCount;
		instrumentDataOffset = 256 + 16 + 16 + 64 + 2;
		instrumentDataSize   = instrDatBytesPerInstrument * instrDatInstrumentCount;
		instrumentEntrySize  = instrDatBytesPerInstrument;

	} else {
		// MUSIC.DRV: we expect at least 468 bytes
		if (driverDataSize < 468)
			return false;

		// music.drv is basically a driver, but with a few fixed locations for certain data

		channelMappingOffset         = 396;
		channelMappingSize           = 16;
		instrumentMappingOffset      = 140;
		instrumentMappingSize        = 128;
		instrumentVolumeAdjustOffset = 140 + 128;
		instrumentVolumeAdjustSize   = 128;
		keyNoteMappingOffset         = 376 + 36; // adjust by 36, because we adjust keyNote before mapping (see noteOn)
		keyNoteMappingSize           = 64;

		// seems to have used 128 + 5 instruments
		// 128 regular ones and an additional 5 for percussion
		instrumentCount         = 128 + AGOS_ADLIB_EXTRA_INSTRUMENT_COUNT;
		instrumentDataOffset    = 722;
		instrumentEntrySize     = 9;
		instrumentDataSize      = instrumentCount * instrumentEntrySize;
	}

	// Channel mapping
	if (channelMappingSize) {
		// Get these 16 bytes for MIDI channel mapping
		if (channelMappingSize != sizeof(_channelMapping))
			return false;

		memcpy(_channelMapping, driverData + channelMappingOffset, sizeof(_channelMapping));
	} else {
		// Set up straight mapping
		for (uint16 channelNr = 0; channelNr < sizeof(_channelMapping); channelNr++) {
			_channelMapping[channelNr] = channelNr;
		}
	}

	if (instrumentMappingSize) {
		// And these for instrument mapping
		if (instrumentMappingSize > sizeof(_instrumentMapping))
			return false;

		memcpy(_instrumentMapping, driverData + instrumentMappingOffset, instrumentMappingSize);
	}
	// Set up straight mapping for the remaining data
	for (uint16 instrumentNr = instrumentMappingSize; instrumentNr < sizeof(_instrumentMapping); instrumentNr++) {
		_instrumentMapping[instrumentNr] = instrumentNr;
	}

	if (instrumentVolumeAdjustSize) {
		if (instrumentVolumeAdjustSize != sizeof(_instrumentVolumeAdjust))
			return false;

		memcpy(_instrumentVolumeAdjust, driverData + instrumentVolumeAdjustOffset, instrumentVolumeAdjustSize);
	}

	// Get key note mapping, if available
	if (keyNoteMappingSize) {
		if (keyNoteMappingSize != sizeof(_percussionKeyNoteMapping))
			return false;

		memcpy(_percussionKeyNoteMapping, driverData + keyNoteMappingOffset, keyNoteMappingSize);
	}

	// Check, if there are enough bytes left to hold all instrument data
	if (driverDataSize < (instrumentDataOffset + instrumentDataSize))
		return false;

	// We release previous instrument data, just in case
	if (_instrumentTable)
		delete[] _instrumentTable;

	_instrumentTable = new InstrumentEntry[instrumentCount];
	_instrumentCount = instrumentCount;

	byte            *instrDATReadPtr    = driverData + instrumentDataOffset;
	InstrumentEntry *instrumentWritePtr = _instrumentTable;

	for (uint16 instrumentNr = 0; instrumentNr < _instrumentCount; instrumentNr++) {
		memcpy(instrumentWritePtr, instrDATReadPtr, sizeof(InstrumentEntry));
		instrDATReadPtr += instrumentEntrySize;
		instrumentWritePtr++;
	}

	// Enable MUSIC.DRV-Mode (slightly different behaviour)
	if (useMusicDrvFile)
		_musicDrvMode = true;

	if (_musicDrvMode) {
		// Extra code for MUSIC.DRV

		// This was done during "programChange" in the original driver
		instrumentWritePtr = _instrumentTable;
		for (uint16 instrumentNr = 0; instrumentNr < _instrumentCount; instrumentNr++) {
			instrumentWritePtr->reg80op1 |= 0x03; // set release rate
			instrumentWritePtr->reg80op2 |= 0x03;
			instrumentWritePtr++;
		}
	}
	return true;
}

MidiDriver *MidiDriver_Accolade_AdLib_create(Common::String driverFilename) {
	byte  *driverData = NULL;
	uint16 driverDataSize = 0;
	bool   isMusicDrvFile = false;

	MidiDriver_Accolade_readDriver(driverFilename, MT_ADLIB, driverData, driverDataSize, isMusicDrvFile);
	if (!driverData)
		error("ACCOLADE-ADLIB: error during readDriver()");

	MidiDriver_Accolade_AdLib *driver = new MidiDriver_Accolade_AdLib();
	if (driver) {
		if (!driver->setupInstruments(driverData, driverDataSize, isMusicDrvFile)) {
			delete driver;
			driver = nullptr;
		}
	}

	delete[] driverData;
	return driver;
}

} // End of namespace AGOS
