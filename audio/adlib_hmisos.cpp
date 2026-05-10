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

#include "audio/adlib_hmisos.h"

const byte MidiDriver_ADLIB_HMISOS::INSTRUMENT_BANK_SIGNATURE[8] = { 0x00, 0x00, 'A', 'D', 'L', 'I', 'B', '-' };

MidiDriver_ADLIB_HMISOS::MidiDriver_ADLIB_HMISOS(OPL::Config::OplType oplType, int timerFrequency) :
		MidiDriver_ADLIB_Multisource(oplType, timerFrequency) {
	memset(_sosInstrumentBank, 0, sizeof(_sosInstrumentBank));
	memset(_sosRhythmBank, 0, sizeof(_sosRhythmBank));

	_instrumentWriteMode = INSTRUMENT_WRITE_MODE_NOTE_ON;
	_modulationDepth = MODULATION_DEPTH_LOW;
	_vibratoDepth = VIBRATO_DEPTH_LOW;
	_rhythmModeRewriteSharedRegister = true;
}

int MidiDriver_ADLIB_HMISOS::open() {

	return MidiDriver_ADLIB_Multisource::open();
}

bool MidiDriver_ADLIB_HMISOS::loadInstrumentBanks(Common::SeekableReadStream *instrumentBankStream, Common::SeekableReadStream *rhythmBankStream) {
	int numInstruments = loadInstrumentBank(instrumentBankStream, false);
	if (numInstruments < 0)
		return false;
	_instrumentBank = _sosInstrumentBank;

	if (rhythmBankStream != nullptr) {
		numInstruments = loadInstrumentBank(rhythmBankStream, true);
		if (numInstruments < 0)
			return false;
		_rhythmBank = _sosRhythmBank;
		_rhythmBankFirstNote = 0;
		_rhythmBankLastNote = numInstruments;
	}

	return true;
}

int MidiDriver_ADLIB_HMISOS::loadInstrumentBank(Common::SeekableReadStream *stream, bool rhythmBank) {
	OplInstrumentDefinition *instrumentBank = rhythmBank ? _sosRhythmBank : _sosInstrumentBank;

	byte signature[8];
	stream->read(signature, 8);

	if (memcmp(INSTRUMENT_BANK_SIGNATURE, signature, 8)) {
		warning("Invalid HMI SOS AdLib instrument bank signature");
		return -1;
	}

	uint16 instrumentsUsed = stream->readUint16LE();
	if (instrumentsUsed > 128)
		instrumentsUsed = 128;
	//uint16 instrumentsTotal = stream->readUint16LE();
	stream->skip(2);
	uint32 namesOffset = stream->readUint32LE();
	uint32 instDataOffset = stream->readUint32LE();

	stream->seek(namesOffset, SEEK_SET);
	byte rhythmNotes[128];
	for (uint16 i = 0; i < instrumentsUsed; i++) {
		stream->skip(2); // Instrument data offset
		// The flags byte is used to specify the rhythm note
		rhythmNotes[i] = stream->readByte();
		stream->skip(9); // Instrument name
	}

	stream->seek(instDataOffset, SEEK_SET);
	AdLibBnkInstrumentDefinition bnkInstDef;
	for (uint16 i = 0; i < instrumentsUsed; i++) {
		stream->skip(2); // Instrument type and voice number

		bnkInstDef.operator0.keyScalingLevel = stream->readByte();
		bnkInstDef.operator0.frequencyMultiplier = stream->readByte();
		bnkInstDef.operator0.feedback = stream->readByte();
		bnkInstDef.operator0.attack = stream->readByte();
		bnkInstDef.operator0.sustain = stream->readByte();
		bnkInstDef.operator0.envelopeGainType = stream->readByte();
		bnkInstDef.operator0.decay = stream->readByte();
		bnkInstDef.operator0.release = stream->readByte();
		bnkInstDef.operator0.level = stream->readByte();
		bnkInstDef.operator0.amplitudeModulation = stream->readByte();
		bnkInstDef.operator0.vibrato = stream->readByte();
		bnkInstDef.operator0.keyScalingRate = stream->readByte();
		// Connection bit in SOS BNK is not flipped
		bnkInstDef.operator0.connection = stream->readByte() == 0 ? 1 : 0;

		bnkInstDef.operator1.keyScalingLevel = stream->readByte();
		bnkInstDef.operator1.frequencyMultiplier = stream->readByte();
		bnkInstDef.operator1.feedback = stream->readByte();
		bnkInstDef.operator1.attack = stream->readByte();
		bnkInstDef.operator1.sustain = stream->readByte();
		bnkInstDef.operator1.envelopeGainType = stream->readByte();
		bnkInstDef.operator1.decay = stream->readByte();
		bnkInstDef.operator1.release = stream->readByte();
		bnkInstDef.operator1.level = stream->readByte();
		bnkInstDef.operator1.amplitudeModulation = stream->readByte();
		bnkInstDef.operator1.vibrato = stream->readByte();
		bnkInstDef.operator1.keyScalingRate = stream->readByte();
		bnkInstDef.operator1.connection = stream->readByte() == 0 ? 1 : 0;

		bnkInstDef.waveformSelect0 = stream->readByte();
		bnkInstDef.waveformSelect1 = stream->readByte();

		bnkInstDef.toOplInstrumentDefinition(instrumentBank[i]);
		instrumentBank[i].rhythmNote = rhythmNotes[i];
	}

	return instrumentsUsed;
}
