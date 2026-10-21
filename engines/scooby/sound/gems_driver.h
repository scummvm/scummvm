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

#ifndef SCOOBY_GEMS_DRIVER_H
#define SCOOBY_GEMS_DRIVER_H

#include "common/scummsys.h"

#include "scooby/sound/genesis_sound_chip.h"

namespace Audio {
class Mixer;
}

namespace Scooby {

class ScoobyDooRom;

// A C++ port of the GEMS 2.5 Z80 sound driver (Sega of America / Extended Play, 5/21/92) that the
// cartridge copies to Z80 RAM from ROM 0x001BB94A-0x001BD1CF. The ROM image is byte-identical to the
// published GEMS 2.5 Z80.ASM, so this port follows that source routine by routine; the original
// label names are kept in the comments.
//
// The 68000 side talks to the driver through a 64-byte command FIFO in Z80 RAM. The public methods
// below reproduce the 68000 library routines (GEMS.A) that the game calls; each one writes the same
// command bytes into the FIFO, and the driver consumes them immediately, as the Z80 main loop would.
//
// Differences from the Z80 implementation:
//   - Z80 timing is not emulated. The 60 Hz VBlank work runs from a YM2612 Timer A callback, and DAC
//     samples are clocked per rendered chip sample at the Timer A rate the original programs.
//   - The 68000-space read caches (the 16-byte sequence buffers and XFER68K banking) are replaced by
//     direct ROM reads; ROM contents never change, so the bytes read are identical.
//   - YM2612 LFO (register 0x22) is not forwarded; ScummVM's FM core does not implement it.
class GemsDriver : public GenesisSoundChipClient {
public:
	GemsDriver(const ScoobyDooRom &rom, Audio::Mixer *mixer);
	~GemsDriver() override;

	// Ghidra gemsInit (0x001F6EA8): resets the driver and sends the four bank pointers (cmd 11).
	bool init(uint32 patchBank, uint32 envelopeBank, uint32 sequenceBank, uint32 sampleBank);

	// Ghidra gemsStartSongInternal (0x001F6EF8): command 16.
	void startSong(uint8 song);
	// Ghidra gemsStopSongInternal (0x001F6F16): command 18. Song 255 stops every song.
	void stopSong(uint8 song);
	// Ghidra gemsPauseAll (0x001F6F2A): command 12.
	void pauseAll();
	// Ghidra gemsResumeAll (0x001F6F3E): command 13.
	void resumeAll();
	// Ghidra gemsStopAll (0x001F6F52): command 22.
	void stopAll();

	// Applies the ScummVM music volume (0-255) to all output.
	void setVolume(int volume);

	// GenesisSoundChipClient
	void onVerticalBlank() override;
	void onRenderSample() override;

private:
	// 68000 side (GEMS.A).
	void putCommandByte(uint8 value);
	void putCommand(uint8 command);
	void putCommand(uint8 command, uint8 argument);
	void processCommands();

	// Z80 RAM helpers.
	uint8 &ram(uint32 address);
	uint8 *ccb(int channel);
	uint8 *patch(int channel);
	uint8 readRom(uint32 address) const;
	static uint16 readWord(const uint8 *bytes) { return static_cast<uint16>(bytes[0] | (bytes[1] << 8)); }
	static void writeWord(uint8 *bytes, uint16 value) {
		bytes[0] = static_cast<uint8>(value);
		bytes[1] = static_cast<uint8>(value >> 8);
	}
	static uint32 readPointer(const uint8 *bytes) {
		return static_cast<uint32>(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16));
	}
	uint16 readRomWord(uint32 address) const;
	uint32 bankPointer(int index) const;

	// YM2612 / PSG access.
	void fmWrite(uint8 part, uint8 reg, uint8 value);
	void fmKeyOff(uint8 voice);
	void psgWrite(uint8 value);

	// main loop and command handlers.
	uint8 getCByte();
	int getCcbChannel();
	void resetDriver();
	void doTicks();
	void commandNoteOn();
	void commandNoteOff();
	void commandPitchBendVirtualChannel();
	void commandPause();
	void commandMute();
	void commandSampleRate();

	// Sequencer.
	uint8 getSByte(uint8 *channelBlock);
	void updateSequencers();
	void runSequencer(int channel);
	void startSequence(uint8 sequence);
	void stopSequence(uint8 sequence);
	void pauseSequence(uint8 sequence);
	void resumeAllSequences();
	void clipAll();
	void clipVoiceTable(uint8 *table, bool psg);
	void setTempo(uint8 beatsPerMinute);
	void fetchPatch(int channel);
	void patchLoad(uint8 patchNumber);
	void voiceTimers();
	void voiceTimerTable(uint8 *table, bool psg);

	// Envelopes and pitch.
	void triggerEnvelope(uint8 envelope, uint8 channel, uint8 channelFlags);
	void doEnvelopes();
	void setPitchBend(int channel, uint8 low, uint8 high);
	void applyBends();
	uint16 getFrequency(bool psg, uint8 note, int channel);

	// Notes.
	void noteOn(int channel, uint8 note);
	void noteOnFm(int channel, uint8 note);
	void noteOnPsg(int channel, uint8 note, bool noise);
	void noteOnDigital(int channel, uint8 note);
	int allocateVoice(uint8 *table, int channel);
	int allocateSpecificVoice(uint8 *entry, int channel);
	void storeVoice(uint8 *entry, uint8 flags, int channel, uint8 note);
	void noteOff(uint8 channel, uint8 note);
	int deallocateVoice(uint8 *table, uint8 channel, uint8 note);
	void writeFmPatch(uint8 part, uint8 channelInPart, const uint8 *patchData);

	// Digital samples.
	void dacSample();
	void fillDacFifo();
	void fillDacFifoNeeded();
	void stopDac();
	void noteOffDigital();
	void transferSample(uint8 fifoOffset, uint32 source, int count);

	void doPsgEnvelopes();

	const ScoobyDooRom &_rom;
	GenesisSoundChip *_chip;
	bool _ready;

	// Z80 RAM 0x1B20-0x1FFF: MBOXES, CMDFIFO, CCB, CH0BUF, ENV0BUF, DACFIFO.
	uint8 _highRam[0x2000 - 0x1B20];
	uint8 _cmdWritePtr; // CMDWPTR
	uint8 _cmdReadPtr;  // CMDRPTR
	uint8 _ramDummy;

	// PATCHDATA: one 39-byte patch buffer per GEMS channel, plus the slot PATCHLOAD leaves CHPATPTR at.
	uint8 _patchData[17 * 39];
	int _currentPatch; // CHPATPTR as a channel index.

	// PTBL68K, ETBL68K, STBL68K, DTBL68K.
	uint8 _bankPointers[12];

	// PSG pseudo registers (psgcom ... whdflg), 11 groups of four bytes.
	uint8 _psgRegisters[44];

	// Voice tables, 7 bytes per voice, 0xFF terminated.
	uint8 _fmVoices[6 * 7 + 1];
	uint8 _psgVoices[3 * 7 + 1];
	uint8 _noiseVoices[1 * 7 + 1];

	// PBTBL: pitch bend, envelope bend, retrigger flags for 16 channels.
	uint8 _pitchBends[80];
	uint8 _needBend;

	// ECB: four envelope control blocks.
	uint8 _envelopeBlocks[29];

	// Timebase.
	uint16 _subBeatsPerTick; // SBPT
	uint16 _subBeatAccumulator; // SBPTACC
	uint8 _tickCount; // TICKCNT
	uint8 _timebaseFlags; // TBASEFLAGS

	uint8 _masterAttenuation; // MASTERATN
	uint8 _noteOnAttenuation; // noteonatten
	uint8 _noteOnVoice; // noteonvoice
	uint8 _carriers; // CARRIERS
	uint16 _noteOnFrequency; // noteonffreq
	uint8 _startSequenceNumber; // stseqsnum
	uint8 _lowestPriorityVoice; // avlowestp as an entry offset
	uint8 *_lowestPriorityTable;

	// Digital sample state.
	uint8 _sampleHeader[12]; // SAMPFLAGS, SAMPPTR, SAMPSKIP, SAMPFIRST, SAMPLOOP, SAMPEND
	bool _dacmeEnabled; // DACME != RET
	bool _fillEnabled; // FILLDACFIFO != RET
	uint8 _dacReadPtr; // E'
	uint8 _dacWritePtr; // DACFIFOWPTR
	uint8 _dacNibblePattern; // C'
	bool _dacNibbleMode; // DACMEPROC
	uint8 _fdfState; // FDFSTATE
	uint32 _samplePtr; // SAMPLEPTR
	uint16 _sampleCounter; // SAMPLECTR
	uint32 _dacPeriod; // Timer A period in render samples.
	uint32 _dacPhase;
	uint8 _channel3Mode; // CH3 mode bits written to register 0x27.
};

} // namespace Scooby

#endif // SCOOBY_GEMS_DRIVER_H
