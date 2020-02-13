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

#include "audio/fmopl.h"
#include "audio/mididrv.h"

namespace AGOS {

#define AGOS_ADLIB_VOICES_COUNT 11

struct InstrumentEntry {
	byte reg20op1; // Amplitude Modulation / Vibrato / Envelope Generator Type / Keyboard Scaling Rate / Modulator Frequency Multiple
	byte reg40op1; // Level Key Scaling / Total Level
	byte reg60op1; // Attack Rate / Decay Rate
	byte reg80op1; // Sustain Level / Release Rate
	byte reg20op2; // Amplitude Modulation / Vibrato / Envelope Generator Type / Keyboard Scaling Rate / Modulator Frequency Multiple
	byte reg40op2; // Level Key Scaling / Total Level
	byte reg60op2; // Attack Rate / Decay Rate
	byte reg80op2; // Sustain Level / Release Rate
	byte regC0;    // Feedback / Algorithm, bit 0 - set -> both operators in use
};

class MidiDriver_Accolade_AdLib : public MidiDriver {
public:
	MidiDriver_Accolade_AdLib();
	~MidiDriver_Accolade_AdLib() override;

	// MidiDriver
	int open() override;
	void close() override;
	void send(uint32 b) override;
	MidiChannel *allocateChannel() override { return NULL; }
	MidiChannel *getPercussionChannel() override { return NULL; }

	bool isOpen() const override { return _isOpen; }
	uint32 getBaseTempo() override { return 1000000 / OPL::OPL::kDefaultCallbackFrequency; }

	void setVolume(byte volume);
	uint32 property(int prop, uint32 param) override;

	bool setupInstruments(byte *instrumentData, uint16 instrumentDataSize, bool useMusicDrvFile);

	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) override;

private:
	bool _musicDrvMode;

	// from INSTR.DAT/MUSIC.DRV - simple mapping between MIDI channel and MT32 channel
	byte _channelMapping[AGOS_MIDI_CHANNEL_COUNT];
	// from INSTR.DAT/MUSIC.DRV - simple mapping between MIDI instruments and MT32 instruments
	byte _instrumentMapping[AGOS_MIDI_INSTRUMENT_COUNT];
	// from INSTR.DAT/MUSIC.DRV - volume adjustment per instrument
	signed char _instrumentVolumeAdjust[AGOS_MIDI_INSTRUMENT_COUNT];
	// simple mapping between MIDI key notes and MT32 key notes
	byte _percussionKeyNoteMapping[AGOS_MIDI_KEYNOTE_COUNT];

	// from INSTR.DAT/MUSIC.DRV - adlib instrument data
	InstrumentEntry *_instrumentTable;
	byte            _instrumentCount;

	struct ChannelEntry {
		const  InstrumentEntry *currentInstrumentPtr;
		byte   currentNote;
		byte   currentA0hReg;
		byte   currentB0hReg;
		int16  volumeAdjust;
		byte   velocity;

		ChannelEntry() : currentInstrumentPtr(NULL), currentNote(0),
						currentA0hReg(0), currentB0hReg(0), volumeAdjust(0), velocity(0) { }
	};

	byte _percussionReg;

	OPL::OPL *_opl;
	int _masterVolume;

	Common::TimerManager::TimerProc _adlibTimerProc;
	void *_adlibTimerParam;

	bool _isOpen;

	// stores information about all FM voice channels
	ChannelEntry _channels[AGOS_ADLIB_VOICES_COUNT];

	void onTimer();

	void resetAdLib();
	void resetAdLibOperatorRegisters(byte baseRegister, byte value);
	void resetAdLibFMVoiceChannelRegisters(byte baseRegister, byte value);

	void programChange(byte FMvoiceChannel, byte mappedInstrumentNr, byte MIDIinstrumentNr);
	void programChangeSetInstrument(byte FMvoiceChannel, byte mappedInstrumentNr, byte MIDIinstrumentNr);
	void setRegister(int reg, int value);
	void noteOn(byte FMvoiceChannel, byte note, byte velocity);
	void noteOnSetVolume(byte FMvoiceChannel, byte operatorReg, byte adjustedVelocity);
	void noteOff(byte FMvoiceChannel, byte note, bool dontCheckNote);
};

} // End of namespace AGOS
