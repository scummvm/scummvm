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

#include "sherlock/sherlock.h"
#include "sherlock/tattoo/drivers/tattoo_mididriver.h"

#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/fmopl.h"
#include "audio/softsynth/emumidi.h"

namespace Sherlock {

#define SHERLOCK_MILES_MIDI_CHANNEL_COUNT 16
#define SHERLOCK_MILES_ADLIB_PHYSICAL_FMVOICES_COUNT 9
#define SHERLOCK_MILES_ADLIB_VIRTUAL_FMVOICES_COUNT 16

#define SHERLOCK_MILES_ADLIB_VIRTUAL_FMVOICES_COUNT_MAX 20

#define SHERLOCK_MILES_ADLIB_PERCUSSION_BANK 127

enum kMilesAdLibUpdateFlags {
	kMilesAdLibUpdateFlags_None    = 0,
	kMilesAdLibUpdateFlags_Reg_20  = 1 << 0,
	kMilesAdLibUpdateFlags_Reg_40  = 1 << 1,
	kMilesAdLibUpdateFlags_Reg_60  = 1 << 2, // register 0x6x + 0x8x
	kMilesAdLibUpdateFlags_Reg_C0  = 1 << 3,
	kMilesAdLibUpdateFlags_Reg_E0  = 1 << 4,
	kMilesAdLibUpdateFlags_Reg_A0  = 1 << 5, // register 0xAx + 0xBx
	kMilesAdLibUpdateFlags_Reg_All = 0x3F
};

byte milesOperator1Register[SHERLOCK_MILES_ADLIB_PHYSICAL_FMVOICES_COUNT] = {
	0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12
};

byte milesOperator2Register[SHERLOCK_MILES_ADLIB_PHYSICAL_FMVOICES_COUNT] = {
	0x03, 0x04, 0x05, 0x0B, 0x0C, 0x0D, 0x13, 0x14, 0x15
};

struct InstrumentEntry {
	byte bankId;
	byte patchId;
	int16 transposition;
	byte reg20op1;
	byte reg40op1;
	byte reg60op1;
	byte reg80op1;
	byte regE0op1;
	byte reg20op2;
	byte reg40op2;
	byte reg60op2;
	byte reg80op2;
	byte regE0op2;
	byte regC0;
};

// hardcoded, dumped from ADLIB.MDI
uint16 milesFrequencyLookUpTable[] = {
	0x02B2, 0x02B4, 0x02B7, 0x02B9, 0x02BC, 0x02BE, 0x02C1, 0x02C3, 0x02C6, 0x02C9, 0x02CB, 0x02CE,
	0x02D0, 0x02D3, 0x02D6, 0x02D8, 0x02DB, 0x02DD, 0x02E0, 0x02E3, 0x02E5, 0x02E8, 0x02EB, 0x02ED,
	0x02F0, 0x02F3, 0x02F6, 0x02F8, 0x02FB, 0x02FE, 0x0301, 0x0303, 0x0306, 0x0309, 0x030C, 0x030F,
	0x0311, 0x0314, 0x0317, 0x031A, 0x031D, 0x0320, 0x0323, 0x0326, 0x0329, 0x032B, 0x032E, 0x0331,
	0x0334, 0x0337, 0x033A, 0x033D, 0x0340, 0x0343, 0x0346, 0x0349, 0x034C, 0x034F, 0x0352, 0x0356,
	0x0359, 0x035C, 0x035F, 0x0362, 0x0365, 0x0368, 0x036B, 0x036F, 0x0372, 0x0375, 0x0378, 0x037B,
	0x037F, 0x0382, 0x0385, 0x0388, 0x038C, 0x038F, 0x0392, 0x0395, 0x0399, 0x039C, 0x039F, 0x03A3,
	0x03A6, 0x03A9, 0x03AD, 0x03B0, 0x03B4, 0x03B7, 0x03BB, 0x03BE, 0x03C1, 0x03C5, 0x03C8, 0x03CC,
	0x03CF, 0x03D3, 0x03D7, 0x03DA, 0x03DE, 0x03E1, 0x03E5, 0x03E8, 0x03EC, 0x03F0, 0x03F3, 0x03F7,
	0x03FB, 0x03FE, 0xFE01, 0xFE03, 0xFE05, 0xFE07, 0xFE08, 0xFE0A, 0xFE0C, 0xFE0E, 0xFE10, 0xFE12,
	0xFE14, 0xFE16, 0xFE18, 0xFE1A, 0xFE1C, 0xFE1E, 0xFE20, 0xFE21, 0xFE23, 0xFE25, 0xFE27, 0xFE29,
	0xFE2B, 0xFE2D, 0xFE2F, 0xFE31, 0xFE34, 0xFE36, 0xFE38, 0xFE3A, 0xFE3C, 0xFE3E, 0xFE40, 0xFE42,
	0xFE44, 0xFE46, 0xFE48, 0xFE4A, 0xFE4C, 0xFE4F, 0xFE51, 0xFE53, 0xFE55, 0xFE57, 0xFE59, 0xFE5C,
	0xFE5E, 0xFE60, 0xFE62, 0xFE64, 0xFE67, 0xFE69, 0xFE6B, 0xFE6D, 0xFE6F, 0xFE72, 0xFE74, 0xFE76,
	0xFE79, 0xFE7B, 0xFE7D, 0xFE7F, 0xFE82, 0xFE84, 0xFE86, 0xFE89, 0xFE8B, 0xFE8D, 0xFE90, 0xFE92,
	0xFE95, 0xFE97, 0xFE99, 0xFE9C, 0xFE9E, 0xFEA1, 0xFEA3, 0xFEA5, 0xFEA8, 0xFEAA, 0xFEAD, 0xFEAF
};

// hardcoded, dumped from ADLIB.MDI
uint16 milesVolumeSensitivityTable[] = {
	82, 85, 88, 91, 94, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 127
};


class MidiDriver_Miles_AdLib : public MidiDriver_Emulated {
public:
	MidiDriver_Miles_AdLib(Audio::Mixer *mixer, InstrumentEntry *instrumentTablePtr, uint16 instrumentTableCount);
	virtual ~MidiDriver_Miles_AdLib();

	// MidiDriver
	int open();
	void close();
	void send(uint32 b);
	MidiChannel *allocateChannel() { return NULL; }
	MidiChannel *getPercussionChannel() { return NULL; }

	// AudioStream
	bool isStereo() const { return false; }
	int getRate() const { return _mixer->getOutputRate(); }
	int getPolyphony() const { return SHERLOCK_MILES_ADLIB_PHYSICAL_FMVOICES_COUNT; }
	bool hasRhythmChannel() const { return false; }

	// MidiDriver_Emulated
	void generateSamples(int16 *buf, int len);

	void setVolume(byte volume);
	virtual uint32 property(int prop, uint32 param);

private:
	// Structure to hold information about current status of MIDI Channels
	struct MidiChannelEntry {
		byte   currentPatchBank;
		const  InstrumentEntry *currentInstrumentPtr;
		uint16 currentPitchBender;
		byte   currentPitchRange;
		byte   currentVoiceProtection;

		byte   currentVolume;
		byte   currentVolumeExpression;

		byte   currentModulation;
		byte   currentSustain;

		byte   currentActiveVoicesCount;

		MidiChannelEntry() : currentPatchBank(0),
							currentInstrumentPtr(NULL),
							currentPitchBender(SHERLOCK_MILES_PITCHBENDER_DEFAULT),
							currentVoiceProtection(0),
							currentVolume(0), currentVolumeExpression(0),
							currentModulation(0),
							currentSustain(0),
							currentActiveVoicesCount(0) { }
	};

	// Structure to hold information about current status of virtual FM Voices
	struct VirtualFmVoiceEntry {
		bool   inUse;
		byte   actualMidiChannel;

		const  InstrumentEntry *currentInstrumentPtr;

		bool   isPhysical;
		byte   physicalFmVoice;

		uint16 currentPriority;

		byte   currentNote;
		int16  currentTransposition;
		byte   currentVelocity;

		bool   sustained;

		VirtualFmVoiceEntry(): inUse(false),
								actualMidiChannel(0),
								currentInstrumentPtr(NULL),
								isPhysical(false), physicalFmVoice(0),
								currentPriority(0),
								currentNote(0),
								currentTransposition(0),
								currentVelocity(0),
								sustained(false) { }
	};

	// Structure to hold information about current status of physical FM Voices
	struct PhysicalFmVoiceEntry {
		bool   inUse;
		byte   virtualFmVoice;

		byte   currentA0hReg;
		byte   currentB0hReg;

		PhysicalFmVoiceEntry(): inUse(false),
								virtualFmVoice(0),
								currentA0hReg(0), currentB0hReg(0) { }
	};

	OPL::OPL *_opl;
	int _masterVolume;

	// stores information about all MIDI channels (not the actual OPL FM voice channels!)
	MidiChannelEntry _midiChannels[SHERLOCK_MILES_MIDI_CHANNEL_COUNT];

	// stores information about all virtual OPL FM voices
	VirtualFmVoiceEntry _virtualFmVoices[SHERLOCK_MILES_ADLIB_VIRTUAL_FMVOICES_COUNT];

	// stores information about all physical OPL FM voices
	PhysicalFmVoiceEntry _physicalFmVoices[SHERLOCK_MILES_ADLIB_PHYSICAL_FMVOICES_COUNT];

	// holds all instruments
	InstrumentEntry *_instrumentTablePtr;
	uint16           _instrumentTableCount;

protected:
	void onTimer();

private:
	void resetData();
	void resetAdLib();
	void resetAdLib_OperatorRegisters(byte baseRegister, byte value);
	void resetAdLib_FMVoiceChannelRegisters(byte baseRegister, byte value);

	void setRegister(int reg, int value);

	int16 searchFreeVirtualFmVoiceChannel();
	int16 searchFreePhysicalFmVoiceChannel();

	void noteOn(byte midiChannel, byte note, byte velocity);
	void noteOff(byte midiChannel, byte note);
	//void voiceOnOff(byte fmVoiceChannel, bool KeyOn, byte note, byte velocity);

	void prioritySort();

	void releaseFmVoice(byte virtualFmVoice);

	void updatePhysicalFmVoice(byte virtualFmVoice, bool keyOn, uint16 registerUpdateFlags);

	void controlChange(byte midiChannel, byte controllerNumber, byte controllerValue);
	void programChange(byte midiChannel, byte patchId);

	const InstrumentEntry *searchInstrument(byte bankId, byte patchId);

	void pitchBendChange(byte MIDIchannel, byte parameter1, byte parameter2);
};

MidiDriver_Miles_AdLib::MidiDriver_Miles_AdLib(Audio::Mixer *mixer, InstrumentEntry *instrumentTablePtr, uint16 instrumentTableCount)
	: MidiDriver_Emulated(mixer), _masterVolume(15), _opl(0) {

	_instrumentTablePtr = instrumentTablePtr;
	_instrumentTableCount = instrumentTableCount;

	resetData();
}

MidiDriver_Miles_AdLib::~MidiDriver_Miles_AdLib() {
	delete[] _instrumentTablePtr; // is created in factory MidiDriver_Miles_AdLib_create()
}

int MidiDriver_Miles_AdLib::open() {
	int rate = _mixer->getOutputRate();

	_opl = OPL::Config::create(OPL::Config::kOpl2);

	if (!_opl)
		return -1;

	_opl->init(rate);

	MidiDriver_Emulated::open();

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO);

	resetAdLib();

	return 0;
}

void MidiDriver_Miles_AdLib::close() {
	_mixer->stopHandle(_mixerSoundHandle);

	delete _opl;
}

void MidiDriver_Miles_AdLib::setVolume(byte volume) {
	_masterVolume = volume;
	//renewNotes(-1, true);
}

void MidiDriver_Miles_AdLib::onTimer() {
}

void MidiDriver_Miles_AdLib::resetData() {
	memset(_midiChannels, 0, sizeof(_midiChannels));
	memset(_virtualFmVoices, 0, sizeof(_virtualFmVoices));
	memset(_physicalFmVoices, 0, sizeof(_physicalFmVoices));

	for (byte midiChannel = 0; midiChannel < SHERLOCK_MILES_MIDI_CHANNEL_COUNT; midiChannel++) {
		_midiChannels[midiChannel].currentPitchBender = SHERLOCK_MILES_PITCHBENDER_DEFAULT;
		_midiChannels[midiChannel].currentPitchRange = 12;
		// Miles Audio 2: hardcoded pitch range as a global (not channel specific), set to 12
		// Miles Audio 3: pitch range per MIDI channel
		_midiChannels[midiChannel].currentVolumeExpression = 127;
	}

}

void MidiDriver_Miles_AdLib::resetAdLib() {
	setRegister(0x01, 0x20); // enable waveform control on both operators
	setRegister(0x04, 0xE0); // Timer control

	setRegister(0x08, 0); // select FM music mode
	setRegister(0xBD, 0); // disable Rhythm

	// reset FM voice instrument data
	resetAdLib_OperatorRegisters(0x20, 0);
	resetAdLib_OperatorRegisters(0x60, 0);
	resetAdLib_OperatorRegisters(0x80, 0);
	resetAdLib_FMVoiceChannelRegisters(0xA0, 0);
	resetAdLib_FMVoiceChannelRegisters(0xB0, 0);
	resetAdLib_FMVoiceChannelRegisters(0xC0, 0);
	resetAdLib_OperatorRegisters(0xE0, 0);
	resetAdLib_OperatorRegisters(0x40, 0x3F);
}

void MidiDriver_Miles_AdLib::resetAdLib_OperatorRegisters(byte baseRegister, byte value) {
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

void MidiDriver_Miles_AdLib::resetAdLib_FMVoiceChannelRegisters(byte baseRegister, byte value) {
	byte FMvoiceChannel;

	for (FMvoiceChannel = 0; FMvoiceChannel < SHERLOCK_MILES_ADLIB_PHYSICAL_FMVOICES_COUNT; FMvoiceChannel++) {
		setRegister(baseRegister + FMvoiceChannel, value);
	}
}

// MIDI messages can be found at http://www.midi.org/techspecs/midimessages.php
void MidiDriver_Miles_AdLib::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	switch (command) {
	case 0x80:
		noteOff(channel, op1);
		break;
	case 0x90:
		noteOn(channel, op1, op2);
		break;
	case 0xb0: // Control change
		controlChange(channel, op1, op2);
		break;
	case 0xc0: // Program Change
		programChange(channel, op1);
		break;
	case 0xa0: // Polyphonic key pressure (aftertouch)
	case 0xd0: // Channel pressure (aftertouch)
		// Aftertouch doesn't seem to be implemented in the Sherlock Holmes adlib driver
		break;
	case 0xe0:
		pitchBendChange(channel, op1, op2);
		break;
	case 0xf0: // SysEx
		warning("MILES-ADLIB: SysEx: %x", b);
		break;
	default:
		warning("MILES-ADLIB: Unknown event %02x", command);
	}
}

void MidiDriver_Miles_AdLib::generateSamples(int16 *data, int len) {
	_opl->readBuffer(data, len);
}

int16 MidiDriver_Miles_AdLib::searchFreeVirtualFmVoiceChannel() {
	for (byte virtualFmVoice = 0; virtualFmVoice < SHERLOCK_MILES_ADLIB_VIRTUAL_FMVOICES_COUNT; virtualFmVoice++) {
		if (!_virtualFmVoices[virtualFmVoice].inUse)
			return virtualFmVoice;
	}
	return -1;
}

int16 MidiDriver_Miles_AdLib::searchFreePhysicalFmVoiceChannel() {
	for (byte physicalFmVoice = 0; physicalFmVoice < SHERLOCK_MILES_ADLIB_PHYSICAL_FMVOICES_COUNT; physicalFmVoice++) {
		if (!_physicalFmVoices[physicalFmVoice].inUse)
			return physicalFmVoice;
	}
	return -1;
}

void MidiDriver_Miles_AdLib::noteOn(byte midiChannel, byte note, byte velocity) {
	const InstrumentEntry *instrumentPtr = NULL;

	//warning("Note On: channel %d, note %d, velocity %d", midiChannel, note, velocity);
	if (velocity == 0) {
		noteOff(midiChannel, note);
		return;
	}

	if (midiChannel == 9) {
		// percussion channel
		// search for instrument according to given note
		instrumentPtr = searchInstrument(SHERLOCK_MILES_ADLIB_PERCUSSION_BANK, note);
	} else {
		// directly get instrument of channel
		instrumentPtr = _midiChannels[midiChannel].currentInstrumentPtr;
	}
	if (!instrumentPtr) {
		warning("MILES-ADLIB: noteOn: invalid instrument");
		return;
	}

	// look for free virtual FM voice
	int16 virtualFmVoice = searchFreeVirtualFmVoiceChannel();

	if (virtualFmVoice == -1) {
		// Out of virtual voices,  can't do anything about it
		return;
	}

	// Scale back velocity
	velocity = (velocity & 0x7F) >> 3;
	velocity = milesVolumeSensitivityTable[velocity];

	if (midiChannel != 9) {
		_virtualFmVoices[virtualFmVoice].currentNote = note;
		_virtualFmVoices[virtualFmVoice].currentTransposition = instrumentPtr->transposition;
	} else {
		// Percussion channel
		_virtualFmVoices[virtualFmVoice].currentNote = instrumentPtr->transposition;
		_virtualFmVoices[virtualFmVoice].currentTransposition = 0;
	}

	_virtualFmVoices[virtualFmVoice].inUse = true;
	_virtualFmVoices[virtualFmVoice].actualMidiChannel = midiChannel;
	_virtualFmVoices[virtualFmVoice].currentInstrumentPtr = instrumentPtr;
	_virtualFmVoices[virtualFmVoice].currentVelocity = velocity;
	_virtualFmVoices[virtualFmVoice].isPhysical = false;
	_virtualFmVoices[virtualFmVoice].sustained = false;
	_virtualFmVoices[virtualFmVoice].currentPriority = 32767;

	int16 physicalFmVoice = searchFreePhysicalFmVoiceChannel();
	if (physicalFmVoice == -1) {
		// None found
		// go through priorities and reshuffle voices
		prioritySort();
		return;
	}

	// Another voice active on this MIDI channel
	_midiChannels[midiChannel].currentActiveVoicesCount++;

	// Mark virtual FM-Voice as being connected to physical FM-Voice
	_virtualFmVoices[virtualFmVoice].isPhysical = true;
	_virtualFmVoices[virtualFmVoice].physicalFmVoice = physicalFmVoice;

	// Mark physical FM-Voice as being connected to virtual FM-Voice
	_physicalFmVoices[physicalFmVoice].inUse = true;
	_physicalFmVoices[physicalFmVoice].virtualFmVoice = virtualFmVoice;

	// Update the physical FM-Voice
	updatePhysicalFmVoice(virtualFmVoice, true, kMilesAdLibUpdateFlags_Reg_All);
}

void MidiDriver_Miles_AdLib::noteOff(byte midiChannel, byte note) {
	//warning("Note Off: channel %d, note %d", midiChannel, note);

	// Search through all virtual FM-Voices for current midiChannel + note
	for (byte virtualFmVoice = 0; virtualFmVoice < SHERLOCK_MILES_ADLIB_VIRTUAL_FMVOICES_COUNT; virtualFmVoice++) {
		if (_virtualFmVoices[virtualFmVoice].inUse) {
			if ((_virtualFmVoices[virtualFmVoice].actualMidiChannel == midiChannel) && (_virtualFmVoices[virtualFmVoice].currentNote == note)) {
				// found one
				if (_midiChannels[midiChannel].currentSustain >= 64) {
					_virtualFmVoices[virtualFmVoice].sustained = true;
					continue;
				}
				// 
				releaseFmVoice(virtualFmVoice);
				_virtualFmVoices[virtualFmVoice].inUse = false;
			}
		}
	}
}

void MidiDriver_Miles_AdLib::prioritySort() {
	byte   virtualFmVoice = 0;
	uint16 virtualPriority = 0;
	uint16 virtualPriorities[SHERLOCK_MILES_ADLIB_VIRTUAL_FMVOICES_COUNT_MAX];
	uint16 virtualFmVoicesCount = 0;
	byte   midiChannel = 0;

	memset(&virtualPriorities, 0, sizeof(virtualPriorities));

	//warning("prioritysort");

	// First calculate priorities for all virtual FM voices, that are in use
	for (virtualFmVoice = 0; virtualFmVoice < SHERLOCK_MILES_ADLIB_VIRTUAL_FMVOICES_COUNT; virtualFmVoice++) {
		if (_virtualFmVoices[virtualFmVoice].inUse) {
			virtualFmVoicesCount++;

			midiChannel = _virtualFmVoices[virtualFmVoice].actualMidiChannel;
			if (_midiChannels[midiChannel].currentVoiceProtection >= 64) {
				// Voice protection enabled
				virtualPriority = 0xFFFF;
			} else {
				virtualPriority = _virtualFmVoices[virtualFmVoice].currentPriority;
			}
			byte currentActiveVoicesCount = _midiChannels[midiChannel].currentActiveVoicesCount;
			if (virtualPriority >= currentActiveVoicesCount) {
				virtualPriority -= _midiChannels[midiChannel].currentActiveVoicesCount;
			} else {
				virtualPriority = 0; // overflow, should never happen
			}
			virtualPriorities[virtualFmVoice] = virtualPriority;
		}
	}

	// 	
	while (virtualFmVoicesCount) {
		uint16 unvoicedHighestPriority = 0;
		byte   unvoicedHighestFmVoice = 0;
		uint16 voicedLowestPriority = 65535;
		byte   voicedLowestFmVoice = 0;

		for (virtualFmVoice = 0; virtualFmVoice < SHERLOCK_MILES_ADLIB_VIRTUAL_FMVOICES_COUNT; virtualFmVoice++) {
			if (_virtualFmVoices[virtualFmVoice].inUse) {
				virtualPriority = virtualPriorities[virtualFmVoice];
				if (!_virtualFmVoices[virtualFmVoice].isPhysical) {
					// currently not physical, so unvoiced
					if (virtualPriority >= unvoicedHighestPriority) {
						unvoicedHighestPriority = virtualPriority;
						unvoicedHighestFmVoice  = virtualFmVoice;
					}
				} else {
					// currently physical, so voiced
					if (virtualPriority <= voicedLowestPriority) {
						voicedLowestPriority = virtualPriority;
						voicedLowestFmVoice  = virtualFmVoice;
					}
				}
			}
		}

		if (unvoicedHighestPriority < voicedLowestPriority)
			break; // We are done

		if (unvoicedHighestPriority == 0)
			break;

		// Safety checks
		assert(_virtualFmVoices[voicedLowestFmVoice].isPhysical);
		assert(!_virtualFmVoices[unvoicedHighestFmVoice].isPhysical);

		// Steal this physical voice
		byte physicalFmVoice = _virtualFmVoices[voicedLowestFmVoice].physicalFmVoice;

		//warning("MILES-ADLIB: stealing physical FM-Voice %d from virtual FM-Voice %d for virtual FM-Voice %d", physicalFmVoice, voicedLowestFmVoice, unvoicedHighestFmVoice);
		//warning("priority old %d, priority new %d", unvoicedHighestPriority, voicedLowestPriority);

		releaseFmVoice(voicedLowestFmVoice);
		_virtualFmVoices[voicedLowestFmVoice].inUse = false;

		// Get some data of the unvoiced highest priority virtual FM Voice
		midiChannel = _virtualFmVoices[unvoicedHighestFmVoice].actualMidiChannel;

		// Another voice active on this MIDI channel
		_midiChannels[midiChannel].currentActiveVoicesCount++;

		// Mark virtual FM-Voice as being connected to physical FM-Voice
		_virtualFmVoices[unvoicedHighestFmVoice].isPhysical = true;
		_virtualFmVoices[unvoicedHighestFmVoice].physicalFmVoice = physicalFmVoice;

		// Mark physical FM-Voice as being connected to virtual FM-Voice
		_physicalFmVoices[physicalFmVoice].inUse = true;
		_physicalFmVoices[physicalFmVoice].virtualFmVoice = unvoicedHighestFmVoice;

		// Update the physical FM-Voice
		updatePhysicalFmVoice(unvoicedHighestFmVoice, true, kMilesAdLibUpdateFlags_Reg_All);

		virtualFmVoicesCount--;
	}
}

void MidiDriver_Miles_AdLib::releaseFmVoice(byte virtualFmVoice) {
	// virtual Voice not actually played? -> exit
	if (!_virtualFmVoices[virtualFmVoice].isPhysical) {
		return;
	}

	byte midiChannel = _virtualFmVoices[virtualFmVoice].actualMidiChannel;
	byte physicalFmVoice = _virtualFmVoices[virtualFmVoice].physicalFmVoice;

	// stop note from playing
	updatePhysicalFmVoice(virtualFmVoice, false, kMilesAdLibUpdateFlags_Reg_A0);

	// this virtual FM voice isn't physical anymore
	_virtualFmVoices[virtualFmVoice].isPhysical = false;

	// Remove physical FM-Voice from being active
	_physicalFmVoices[physicalFmVoice].inUse = false;

	// One less voice active on this MIDI channel
	assert(_midiChannels[midiChannel].currentActiveVoicesCount);
	_midiChannels[midiChannel].currentActiveVoicesCount--;	
}

void MidiDriver_Miles_AdLib::updatePhysicalFmVoice(byte virtualFmVoice, bool keyOn, uint16 registerUpdateFlags) {
	byte midiChannel = _virtualFmVoices[virtualFmVoice].actualMidiChannel;

	if (!_virtualFmVoices[virtualFmVoice].isPhysical) {
		// virtual FM-Voice has no physical FM-Voice assigned? -> ignore
		return;
	}

	byte                   physicalFmVoice = _virtualFmVoices[virtualFmVoice].physicalFmVoice;
	const InstrumentEntry *instrumentPtr = _virtualFmVoices[virtualFmVoice].currentInstrumentPtr;

	byte op1Reg = milesOperator1Register[physicalFmVoice];
	byte op2Reg = milesOperator2Register[physicalFmVoice];

	uint16 compositeVolume = 0;

	if (registerUpdateFlags & kMilesAdLibUpdateFlags_Reg_40) {
		// Calculate new volume
		byte midiVolume = _midiChannels[midiChannel].currentVolume;
		byte midiVolumeExpression = _midiChannels[midiChannel].currentVolumeExpression;
		compositeVolume = midiVolume * midiVolumeExpression * 2;

		compositeVolume = compositeVolume >> 8; // get upmost 8 bits
		if (compositeVolume)
			compositeVolume++; // round up in case result wasn't 0

		compositeVolume = compositeVolume * _virtualFmVoices[virtualFmVoice].currentVelocity * 2;
		compositeVolume = compositeVolume >> 8; // get upmost 8 bits
		if (compositeVolume)
			compositeVolume++; // round up in case result wasn't 0
	}

	if (registerUpdateFlags & kMilesAdLibUpdateFlags_Reg_20) {
		// Amplitude Modulation / Vibrato / Envelope Generator Type / Keyboard Scaling Rate / Modulator Frequency Multiple
		byte reg20op1 = instrumentPtr->reg20op1;
		byte reg20op2 = instrumentPtr->reg20op2;

		if (_midiChannels[midiChannel].currentModulation >= 64) {
			// set bit 6 (Vibrato)
			reg20op1 |= 0x40;
			reg20op2 |= 0x40;
		}
		setRegister(0x20 + op1Reg, reg20op1);
		setRegister(0x20 + op2Reg, reg20op2);
	}

	if (registerUpdateFlags & kMilesAdLibUpdateFlags_Reg_40) {
		// Volume (Level Key Scaling / Total Level)
		byte reg40op1 = instrumentPtr->reg40op1;
		byte reg40op2 = instrumentPtr->reg40op2;

		uint16 volumeOp1 = (~reg40op1) & 0x3F;
		uint16 volumeOp2 = (~reg40op2) & 0x3F;

		if (instrumentPtr->regC0 & 1) {
			// operator 2 enabled
			// scale volume factor
			volumeOp1 = (volumeOp1 * compositeVolume) / 127;
			// 2nd operator always scaled
		}

		volumeOp2 = (volumeOp2 * compositeVolume) / 127;

		volumeOp1 = (~volumeOp1) & 0x3F; // negate it, so we get the proper value for the register
		volumeOp2 = (~volumeOp2) & 0x3F; // ditto
		reg40op1  = (reg40op1 & 0xC0) | volumeOp1; // keep "scaling level" and merge in our volume
		reg40op2  = (reg40op2 & 0xC0) | volumeOp2;

		setRegister(0x40 + op1Reg, reg40op1);
		setRegister(0x40 + op2Reg, reg40op2);
	}

	if (registerUpdateFlags & kMilesAdLibUpdateFlags_Reg_60) {
		// Attack Rate / Decay Rate
		// Sustain Level / Release Rate
		byte reg60op1 = instrumentPtr->reg60op1;
		byte reg60op2 = instrumentPtr->reg60op2;
		byte reg80op1 = instrumentPtr->reg80op1;
		byte reg80op2 = instrumentPtr->reg80op2;

		setRegister(0x60 + op1Reg, reg60op1);
		setRegister(0x60 + op2Reg, reg60op2);
		setRegister(0x80 + op1Reg, reg80op1);
		setRegister(0x80 + op2Reg, reg80op2);
	}

	if (registerUpdateFlags & kMilesAdLibUpdateFlags_Reg_E0) {
		// Waveform Select
		byte regE0op1 = instrumentPtr->regE0op1;
		byte regE0op2 = instrumentPtr->regE0op2;

		setRegister(0xE0 + op1Reg, regE0op1);
		setRegister(0xE0 + op2Reg, regE0op2);
	}

	if (registerUpdateFlags & kMilesAdLibUpdateFlags_Reg_C0) {
		// Feedback / Algorithm
		byte regC0 = instrumentPtr->regC0;

		setRegister(0xC0 + physicalFmVoice, regC0);
	}

	if (registerUpdateFlags & kMilesAdLibUpdateFlags_Reg_A0) {
		// Frequency / Key-On
		// Octave / F-Number / Key-On
		if (!keyOn) {
			// turn off note
			byte regB0 = _physicalFmVoices[physicalFmVoice].currentB0hReg & 0x1F; // remove bit 5 "key on"
			setRegister(0xB0 + physicalFmVoice, regB0);

		} else {
			// turn on note, calculate frequency, octave...
			int16 pitchBender = _midiChannels[midiChannel].currentPitchBender;
			byte  pitchRange = _midiChannels[midiChannel].currentPitchRange;
			int16 currentNote = _virtualFmVoices[virtualFmVoice].currentNote;
			int16 physicalNote = 0;
			int16 halfTone = 0;
			uint16 frequency = 0;
			uint16 frequencyIdx = 0;
			byte   octave = 0;

			pitchBender -= 0x2000;
			pitchBender = pitchBender >> 5; // divide by 32
			pitchBender = pitchBender * pitchRange; // pitchrange 12: now +0x0C00 to -0xC00
			// difference between Miles Audio 2 + 3
			// Miles Audio 2 used a pitch range of 12, which was basically hardcoded
			// Miles Audio 3 used an array, which got set by control change events

			currentNote += _virtualFmVoices->currentTransposition;

			// Normalize note
			currentNote -= 24;
			do {
				currentNote += 12;
			} while (currentNote < 0);
			currentNote += 12;

			do {
				currentNote -= 12;
			} while (currentNote > 95);

			// combine note + pitchbender, also adjust by 8 for rounding
			currentNote = (currentNote << 8) + pitchBender + 8;

			currentNote = currentNote >> 4; // get actual note

			// Normalize
			currentNote -= (12 * 16);
			do {
				currentNote += (12 * 16);
			} while (currentNote < 0);

			currentNote += (12 * 16);
			do {
				currentNote -= (12 * 16);
			} while (currentNote > ((96 * 16) - 1));

			physicalNote = currentNote >> 4;

			halfTone = physicalNote % 12; // remainder of physicalNote / 12

			frequencyIdx = (halfTone << 4) + (currentNote & 0x0F);
			assert(frequencyIdx < sizeof(milesFrequencyLookUpTable));
			frequency = milesFrequencyLookUpTable[frequencyIdx];

			octave = (physicalNote / 12) - 1;

			if (frequency & 0x8000)
				octave++;

			if (octave & 0x80) {
				octave++;
				frequency = frequency >> 1;
			}

			byte regA0 = frequency & 0xFF;
			byte regB0 = ((frequency >> 8) & 0x03) | (octave << 2) | 0x20;

			setRegister(0xA0 + physicalFmVoice, regA0);
			setRegister(0xB0 + physicalFmVoice, regB0);

			_physicalFmVoices[physicalFmVoice].currentB0hReg = regB0;
		}
	}

	//warning("end of update voice");
}

void MidiDriver_Miles_AdLib::controlChange(byte midiChannel, byte controllerNumber, byte controllerValue) {
	uint16 registerUpdateFlags = kMilesAdLibUpdateFlags_None;

	switch (controllerNumber) {
	case SHERLOCK_MILES_CONTROLLER_SELECT_PATCH_BANK:
		_midiChannels[midiChannel].currentPatchBank = controllerValue;
		break;

	case SHERLOCK_MILES_CONTROLLER_PROTECT_VOICE:
		_midiChannels[midiChannel].currentVoiceProtection = controllerValue;
		break;

	case SHERLOCK_MILES_CONTROLLER_PROTECT_TIMBRE:
		// It seems that this can get ignored, because we don't cache timbres at all
		break;

	case SHERLOCK_MILES_CONTROLLER_MODULATION:
		_midiChannels[midiChannel].currentModulation = controllerValue;
		registerUpdateFlags = kMilesAdLibUpdateFlags_Reg_20;
		break;

	case SHERLOCK_MILES_CONTROLLER_VOLUME:
		_midiChannels[midiChannel].currentVolume = controllerValue;
		registerUpdateFlags = kMilesAdLibUpdateFlags_Reg_40;
		break;

	case SHERLOCK_MILES_CONTROLLER_EXPRESSION:
		_midiChannels[midiChannel].currentVolumeExpression = controllerValue;
		registerUpdateFlags = kMilesAdLibUpdateFlags_Reg_40;
		break;

	case SHERLOCK_MILES_CONTROLLER_PANNING:
		//warning("MILES-ADLIB: controlChange: panning");
		registerUpdateFlags = kMilesAdLibUpdateFlags_Reg_C0;
		break;

	case SHERLOCK_MILES_CONTROLLER_SUSTAIN:
		_midiChannels[midiChannel].currentSustain = controllerValue;
		if (controllerValue < 64) {
			// release sustain TODO
		}
		break;

	case SHERLOCK_MILES_CONTROLLER_PITCH_RANGE:
		// Miles Audio 3 feature
		_midiChannels[midiChannel].currentPitchRange = controllerValue;
		break;

	case SHERLOCK_MILES_CONTROLLER_RESET_ALL:
		_midiChannels[midiChannel].currentSustain = 0;
		// release sustain TODO
		_midiChannels[midiChannel].currentModulation = 0;
		_midiChannels[midiChannel].currentVolumeExpression = 127;
		_midiChannels[midiChannel].currentPitchBender = SHERLOCK_MILES_PITCHBENDER_DEFAULT;
		registerUpdateFlags = kMilesAdLibUpdateFlags_Reg_20 | kMilesAdLibUpdateFlags_Reg_40 | kMilesAdLibUpdateFlags_Reg_A0;
		break;

	case SHERLOCK_MILES_CONTROLLER_ALL_NOTES_OFF:
		for (byte virtualFmVoice = 0; virtualFmVoice < SHERLOCK_MILES_ADLIB_VIRTUAL_FMVOICES_COUNT; virtualFmVoice++) {
			if (_virtualFmVoices[virtualFmVoice].inUse) {
				// used
				if (_virtualFmVoices[virtualFmVoice].actualMidiChannel == midiChannel) {
					// by our current MIDI channel -> noteOff
					noteOff(midiChannel, _virtualFmVoices[virtualFmVoice].currentNote);
				}
			}
		}
		break;

	default:
		//warning("MILES-ADLIB: Unsupported control change %d", controllerNumber);
		break;
	}

	if (registerUpdateFlags) {
		for (byte virtualFmVoice = 0; virtualFmVoice < SHERLOCK_MILES_ADLIB_VIRTUAL_FMVOICES_COUNT; virtualFmVoice++) {
			if (_virtualFmVoices[virtualFmVoice].inUse) {
				// used
				if (_virtualFmVoices[virtualFmVoice].actualMidiChannel == midiChannel) {
					// by our current MIDI channel -> update
					updatePhysicalFmVoice(virtualFmVoice, true, registerUpdateFlags);
				}
			}
		}
	}
}

void MidiDriver_Miles_AdLib::programChange(byte midiChannel, byte patchId) {
	const InstrumentEntry *instrumentPtr = NULL;
	byte patchBank = _midiChannels[midiChannel].currentPatchBank;

	// we check, if we actually have data for the requested instrument...
	instrumentPtr = searchInstrument(patchBank, patchId);
	if (!instrumentPtr) {
		warning("MILES-ADLIB: unknown instrument requested (%d, %d)", patchBank, patchId);
		return;
	}

	// and remember it in that case for the current MIDI-channel
	_midiChannels[midiChannel].currentInstrumentPtr = instrumentPtr;
}

const InstrumentEntry *MidiDriver_Miles_AdLib::searchInstrument(byte bankId, byte patchId) {
	const InstrumentEntry *instrumentPtr = _instrumentTablePtr;

	for (uint16 instrumentNr = 0; instrumentNr < _instrumentTableCount; instrumentNr++) {
		if ((instrumentPtr->bankId == bankId) && (instrumentPtr->patchId == patchId)) {
			return instrumentPtr;
		}
		instrumentPtr++;
	}

	return NULL;
}

void MidiDriver_Miles_AdLib::pitchBendChange(byte midiChannel, byte parameter1, byte parameter2) {
	// Miles Audio actually didn't shift parameter 2 1 down in here
	// which means in memory it used a 15-bit pitch bender, which also means the default was 0x4000
	if ((parameter1 & 0x80) || (parameter2 & 0x80)) {
		warning("MILES-ADLIB: invalid pitch bend change");
		return;
	}
	_midiChannels[midiChannel].currentPitchBender = parameter1 | (parameter2 << 7);
}

void MidiDriver_Miles_AdLib::setRegister(int reg, int value) {
	_opl->write(0x220, reg);
	_opl->write(0x221, value);
	//warning("OPL write %x %x (%d)", reg, value, value);
}

uint32 MidiDriver_Miles_AdLib::property(int prop, uint32 param) {
	return 0;
}

MidiDriver *MidiDriver_Miles_AdLib_create(const Common::String instrumentDataFilename, const Common::String instrumentDataFilenameOPL3) {
	// Load adlib instrument data from file SAMPLE.AD (OPL3: SAMPLE.OPL)
	Common::File *fileStream = new Common::File();
	uint32        fileSize = 0;
	byte         *fileDataPtr = NULL;
	uint32        fileDataOffset = 0;
	uint32        fileDataLeft = 0;

	byte curBankId = 0;
	byte curPatchId = 0;

	InstrumentEntry *instrumentTablePtr = NULL;
	uint16           instrumentTableCount = 0;
	InstrumentEntry *instrumentPtr = NULL;
	uint32           instrumentOffset = 0;
	uint16           instrumentDataSize = 0;

	if (!fileStream->open(instrumentDataFilename))
		error("MILES-ADLIB: coult not open instrument file");

	fileSize = fileStream->size();

	fileDataPtr = new byte[fileSize];

	if (fileStream->read(fileDataPtr, fileSize) != fileSize)
		error("MILES-ADLIB: error while reading instrument file");
	fileStream->close();
	delete fileStream;

	// File is like this:
	// [patch:BYTE] [bank:BYTE] [patchoffset:UINT32]
	// ...
	// until patch + bank are both 0xFF, which signals end of header

	// First we check how many entries there are
	fileDataOffset = 0;
	fileDataLeft = fileSize;
	while (1) {
		if (fileDataLeft < 6)
			error("MILES-ADLIB: unexpected EOF in instrument file");

		curPatchId = fileDataPtr[fileDataOffset++];
		curBankId  = fileDataPtr[fileDataOffset++];

		if ((curBankId == 0xFF) && (curPatchId == 0xFF))
			break;

		fileDataOffset += 4; // skip over offset
		instrumentTableCount++;
	}

	if (instrumentTableCount == 0)
		error("MILES-ADLIB: no instruments in instrument file");

	// Allocate space for instruments
	instrumentTablePtr = new InstrumentEntry[instrumentTableCount];

	// Now actually read all entries
	instrumentPtr = instrumentTablePtr;

	fileDataOffset = 0;
	fileDataLeft = fileSize;
	while (1) {
		curPatchId = fileDataPtr[fileDataOffset++];
		curBankId  = fileDataPtr[fileDataOffset++];

		if ((curBankId == 0xFF) && (curPatchId == 0xFF))
			break;

		instrumentOffset = READ_LE_UINT32(fileDataPtr + fileDataOffset);
		fileDataOffset += 4;

		instrumentPtr->bankId = curBankId;
		instrumentPtr->patchId = curPatchId;

		instrumentDataSize = READ_LE_UINT16(fileDataPtr + instrumentOffset);
		if (instrumentDataSize != 14)
			error("MILES-ADLIB: unsupported instrument size");

		instrumentPtr->transposition = (signed char)fileDataPtr[instrumentOffset + 2];
		instrumentPtr->reg20op1 = fileDataPtr[instrumentOffset + 3];
		instrumentPtr->reg40op1 = fileDataPtr[instrumentOffset + 4];
		instrumentPtr->reg60op1 = fileDataPtr[instrumentOffset + 5];
		instrumentPtr->reg80op1 = fileDataPtr[instrumentOffset + 6];
		instrumentPtr->regE0op1 = fileDataPtr[instrumentOffset + 7];
		instrumentPtr->regC0    = fileDataPtr[instrumentOffset + 8];
		instrumentPtr->reg20op2 = fileDataPtr[instrumentOffset + 9];
		instrumentPtr->reg40op2 = fileDataPtr[instrumentOffset + 10];
		instrumentPtr->reg60op2 = fileDataPtr[instrumentOffset + 11];
		instrumentPtr->reg80op2 = fileDataPtr[instrumentOffset + 12];
		instrumentPtr->regE0op2 = fileDataPtr[instrumentOffset + 13];

		// Instrument read, next instrument please
		instrumentPtr++;
	}

	// Free instrument file data
	delete[] fileDataPtr;

	return new MidiDriver_Miles_AdLib(g_system->getMixer(), instrumentTablePtr, instrumentTableCount);
}

} // End of namespace Sherlock
