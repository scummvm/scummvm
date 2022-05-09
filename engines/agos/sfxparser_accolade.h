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

#ifndef AGOS_SFXPARSER_ACCOLADE_H
#define AGOS_SFXPARSER_ACCOLADE_H

#include "agos/drivers/accolade/adlib.h"
#include "agos/drivers/accolade/mt32.h"

#include "common/mutex.h"
#include "common/stream.h"

namespace AGOS {

class SfxParser_Accolade {
public:
	// Size in bytes of MT-32 instrument data in the SFX data.
	static const byte INSTRUMENT_SIZE_MT32 = 0xF9;
	// Number of script ticks per second.
	static const uint16 SCRIPT_TIMER_FREQUENCY = 292;
	// Number of microseconds per script tick.
	static const uint16 SCRIPT_TIMER_RATE = 1000000 / SCRIPT_TIMER_FREQUENCY;

protected:
	// Size in bytes of AdLib instrument data in the SFX data.
	static const byte INSTRUMENT_SIZE_ADLIB = 0x09;
	// Maximum number of words in an SFX script.
	static const byte MAX_SCRIPT_SIZE = 0x30;
	// Maximum number of simultaneous sources for OPL2.
	static const byte OPL2_NUM_SOURCES = 2;
	// Maximum number of simultaneous sources for OPL3.
	static const byte OPL3_NUM_SOURCES = 4;

	// Data for a single SFX. Taken from the game's SFX bank.
	struct SfxData {
		// The instrument data for the used sound device (OPL or MT-32).
		byte instrumentDefinition[INSTRUMENT_SIZE_MT32];
		// The SFX script.
		int16 scriptData[MAX_SCRIPT_SIZE];
		// The size in words of the SFX script.
		int scriptSize;
	};

	// State data a SFX playback slot.
	struct SfxSlot {
		SfxSlot();

		// The data of the SFX currently played by this slot.
		SfxData *sfxData;

		// True if this slot has been allocated to playing a SFX.
		bool allocated;
		// True if SFX playback is active.
		bool active;
		// The source used to send data to the MIDI driver.
		int8 source;
		// Current position in the SFX script.
		byte scriptPos;

		// Current playback time in microseconds.
		uint32 playTime;
		// The timestamp of the last processed script tick.
		uint32 lastEventTime;
		// The last MIDI note that was sent as a note on event.
		int16 lastPlayedNote;
		// The current MIDI note (upper byte) and note fraction (1/256th notes;
		// lower byte) value.
		uint16 currentNoteFraction;
		// True if the allocated channel on the MIDI device has been changed to
		// the SFX instrument.
		bool programChanged;

		// Delta to the note fraction. This is added to/subtracted from the
		// note fraction every script tick.
		int16 noteFractionDelta;
		// The vibrato time. The number of script ticks it takes for the note
		// difference to go from lowest to highest (or the other way around).
		int16 vibratoTime;
		// The number of script ticks that have passed since the vibrato has
		// started.
		int16 vibratoCounter;
		// Vibrato delta to the note fraction. This is added to/subtracted
		// from the note fraction every script tick.
		int16 vibratoDelta;
		// The number of ticks that remain before the next script event is
		// processed.
		int16 waitCounter;
		// The script position at which the current loop has started.
		byte loopStart;
		// The number of times the looped section will be repeated (-1 for
		// infinite loop).
		int16 loopCounter;

		// Completely clears the SFX slot data.
		void clear();
		// Resets the SFX slot data as needed by the reset opcode.
		void reset();
		// True if the current position is at the end of the script.
		bool atEndOfScript();
		// Reads the next script word. Specify the opCode flag to return a
		// valid opcode.
		int16 readScript(bool opCode);
	};

public:
	SfxParser_Accolade();
	virtual ~SfxParser_Accolade();

	// Loads the specified sound effects bank (FXB file).
	void load(Common::SeekableReadStream *in, int32 size);

	// Sets the MIDI driver that should be used to output the SFX.
	virtual void setMidiDriver(MidiDriver_Multisource *driver) = 0;
	// Sets the number of microseconds between timer callbacks.
	void setTimerRate(uint32 rate);

	// Starts playback of the specified sound effect.
	void play(uint8 sfxNumber);
	// Stops all active SFX.
	void stopAll();
	// Pauses or unpauses all active SFX.
	void pauseAll(bool paused);

	void onTimer();
	static void timerCallback(void *data);

protected:
	// Stops the sound effect playing in the specified slot.
	void stop(SfxSlot *sfxSlot);
	// Processes the specified opcode for the specified slot.
	void processOpCode(SfxSlot *sfxSlot, byte opCode);

	// Returns the number of sources available for SFX playback.
	virtual byte getNumberOfSfxSources() = 0;
	// Reads the SFX instrument data into the specified SfxData from the
	// specified SFX bank data. This is positioned at the start of the data of
	// a sound effect in the bank; when the function returns is should be
	// positioned right after all instrument data for the sound effect.
	virtual void readInstrument(SfxData *sfxData, Common::SeekableReadStream *in) = 0;

	// Loads the SFX instrument for the specified slot into the channel
	// allocated to the sound effect. Returns true if the channel needs to be
	// changed to the new instrument when the driver is ready.
	virtual bool loadInstrument(SfxSlot *sfxSlot) = 0;
	// Changes the channel allocated to the sound effect to the SFX instrument.
	virtual void changeInstrument(SfxSlot *sfxSlot) { };
	// Starts a note at the current note / note fraction for the slot.
	virtual void noteOn(SfxSlot *sfxSlot);
	// Stops the current note for the slot.
	virtual void noteOff(SfxSlot *sfxSlot);
	// Updates the note / note fraction for the slot.
	virtual void updateNote(SfxSlot *sfxSlot) { };

	Common::Mutex _mutex;

	MidiDriver_Multisource *_driver;
	uint32 _timerRate;

	// Array of SFX data loaded from the SFX bank.
	SfxData *_sfxData;
	// The number of SFX data loaded.
	uint16 _numSfx;
	// The slots available for SFX playback.
	SfxSlot _sfxSlots[4];
	// The slot numbers allocated to the available SFX sources. -1 if no slot
	// is using the source.
	int8 _sourceAllocations[4];

	// True if SFX playback is paused.
	bool _paused;
};

class SfxParser_Accolade_AdLib : public SfxParser_Accolade {
public:
	SfxParser_Accolade_AdLib() : _adLibDriver(nullptr) { }

protected:
	void setMidiDriver(MidiDriver_Multisource *driver) override;
	byte getNumberOfSfxSources() override;
	void readInstrument(SfxData *sfxData, Common::SeekableReadStream *in) override;
	bool loadInstrument(SfxSlot *sfxSlot) override;
	void noteOn(SfxSlot *sfxSlot) override;
	void updateNote(SfxSlot *sfxSlot) override;

	MidiDriver_Accolade_AdLib *_adLibDriver;
};

class SfxParser_Accolade_MT32 : public SfxParser_Accolade {
public:
	SfxParser_Accolade_MT32() : _mt32Driver(nullptr) { }

protected:
	void setMidiDriver(MidiDriver_Multisource *driver) override;
	byte getNumberOfSfxSources() override;
	void readInstrument(SfxData *sfxData, Common::SeekableReadStream *in) override;
	bool loadInstrument(SfxSlot *sfxSlot) override;
	void changeInstrument(SfxSlot *sfxSlot) override;

	MidiDriver_Accolade_MT32 *_mt32Driver;
};

} // End of namespace AGOS

#endif
