/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011, 2012, 2013 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MT32EMU_SYNTH_H
#define MT32EMU_SYNTH_H

//#include <cstdarg>

namespace MT32Emu {

class TableInitialiser;
class Partial;
class PartialManager;
class Part;
class ROMImage;
class BReverbModel;

/**
 * Methods for emulating the connection between the LA32 and the DAC, which involves
 * some hacks in the real devices for doubling the volume.
 * See also http://en.wikipedia.org/wiki/Roland_MT-32#Digital_overflow
 */
enum DACInputMode {
	// Produces samples at double the volume, without tricks.
	// * Nicer overdrive characteristics than the DAC hacks (it simply clips samples within range)
	// * Higher quality than the real devices
	DACInputMode_NICE,

	// Produces samples that exactly match the bits output from the emulated LA32.
	// * Nicer overdrive characteristics than the DAC hacks (it simply clips samples within range)
	// * Much less likely to overdrive than any other mode.
	// * Half the volume of any of the other modes, meaning its volume relative to the reverb
	//   output when mixed together directly will sound wrong.
	// * Output gain is ignored for both LA32 and reverb output.
	// * Perfect for developers while debugging :)
	DACInputMode_PURE,

	// Re-orders the LA32 output bits as in early generation MT-32s (according to Wikipedia).
	// Bit order at DAC (where each number represents the original LA32 output bit number, and XX means the bit is always low):
	// 15 13 12 11 10 09 08 07 06 05 04 03 02 01 00 XX
	DACInputMode_GENERATION1,

	// Re-orders the LA32 output bits as in later generations (personally confirmed on my CM-32L - KG).
	// Bit order at DAC (where each number represents the original LA32 output bit number):
	// 15 13 12 11 10 09 08 07 06 05 04 03 02 01 00 14
	DACInputMode_GENERATION2
};

enum MIDIDelayMode {
	// Process incoming MIDI events immediately.
	MIDIDelayMode_IMMEDIATE,

	// Delay incoming short MIDI messages as if they where transferred via a MIDI cable to a real hardware unit and immediate sysex processing.
	// This ensures more accurate timing of simultaneous NoteOn messages.
	MIDIDelayMode_DELAY_SHORT_MESSAGES_ONLY,

	// Delay all incoming MIDI events as if they where transferred via a MIDI cable to a real hardware unit.
	MIDIDelayMode_DELAY_ALL
};

const Bit8u SYSEX_MANUFACTURER_ROLAND = 0x41;

const Bit8u SYSEX_MDL_MT32 = 0x16;
const Bit8u SYSEX_MDL_D50 = 0x14;

const Bit8u SYSEX_CMD_RQ1 = 0x11; // Request data #1
const Bit8u SYSEX_CMD_DT1 = 0x12; // Data set 1
const Bit8u SYSEX_CMD_WSD = 0x40; // Want to send data
const Bit8u SYSEX_CMD_RQD = 0x41; // Request data
const Bit8u SYSEX_CMD_DAT = 0x42; // Data set
const Bit8u SYSEX_CMD_ACK = 0x43; // Acknowledge
const Bit8u SYSEX_CMD_EOD = 0x45; // End of data
const Bit8u SYSEX_CMD_ERR = 0x4E; // Communications error
const Bit8u SYSEX_CMD_RJC = 0x4F; // Rejection

const int MAX_SYSEX_SIZE = 512;

const unsigned int CONTROL_ROM_SIZE = 64 * 1024;

struct ControlROMPCMStruct {
	Bit8u pos;
	Bit8u len;
	Bit8u pitchLSB;
	Bit8u pitchMSB;
};

struct ControlROMMap {
	Bit16u idPos;
	Bit16u idLen;
	const char *idBytes;
	Bit16u pcmTable; // 4 * pcmCount bytes
	Bit16u pcmCount;
	Bit16u timbreAMap; // 128 bytes
	Bit16u timbreAOffset;
	bool timbreACompressed;
	Bit16u timbreBMap; // 128 bytes
	Bit16u timbreBOffset;
	bool timbreBCompressed;
	Bit16u timbreRMap; // 2 * timbreRCount bytes
	Bit16u timbreRCount;
	Bit16u rhythmSettings; // 4 * rhythmSettingsCount bytes
	Bit16u rhythmSettingsCount;
	Bit16u reserveSettings; // 9 bytes
	Bit16u panSettings; // 8 bytes
	Bit16u programSettings; // 8 bytes
	Bit16u rhythmMaxTable; // 4 bytes
	Bit16u patchMaxTable; // 16 bytes
	Bit16u systemMaxTable; // 23 bytes
	Bit16u timbreMaxTable; // 72 bytes
};

enum MemoryRegionType {
	MR_PatchTemp, MR_RhythmTemp, MR_TimbreTemp, MR_Patches, MR_Timbres, MR_System, MR_Display, MR_Reset
};

enum ReverbMode {
	REVERB_MODE_ROOM,
	REVERB_MODE_HALL,
	REVERB_MODE_PLATE,
	REVERB_MODE_TAP_DELAY
};

class MemoryRegion {
private:
	Synth *synth;
	Bit8u *realMemory;
	Bit8u *maxTable;
public:
	MemoryRegionType type;
	Bit32u startAddr, entrySize, entries;

	MemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable, MemoryRegionType useType, Bit32u useStartAddr, Bit32u useEntrySize, Bit32u useEntries) {
		synth = useSynth;
		realMemory = useRealMemory;
		maxTable = useMaxTable;
		type = useType;
		startAddr = useStartAddr;
		entrySize = useEntrySize;
		entries = useEntries;
	}
	int lastTouched(Bit32u addr, Bit32u len) const {
		return (offset(addr) + len - 1) / entrySize;
	}
	int firstTouchedOffset(Bit32u addr) const {
		return offset(addr) % entrySize;
	}
	int firstTouched(Bit32u addr) const {
		return offset(addr) / entrySize;
	}
	Bit32u regionEnd() const {
		return startAddr + entrySize * entries;
	}
	bool contains(Bit32u addr) const {
		return addr >= startAddr && addr < regionEnd();
	}
	int offset(Bit32u addr) const {
		return addr - startAddr;
	}
	Bit32u getClampedLen(Bit32u addr, Bit32u len) const {
		if (addr + len > regionEnd())
			return regionEnd() - addr;
		return len;
	}
	Bit32u next(Bit32u addr, Bit32u len) const {
		if (addr + len > regionEnd()) {
			return regionEnd() - addr;
		}
		return 0;
	}
	Bit8u getMaxValue(int off) const {
		if (maxTable == NULL)
			return 0xFF;
		return maxTable[off % entrySize];
	}
	Bit8u *getRealMemory() const {
		return realMemory;
	}
	bool isReadable() const {
		return getRealMemory() != NULL;
	}
	void read(unsigned int entry, unsigned int off, Bit8u *dst, unsigned int len) const;
	void write(unsigned int entry, unsigned int off, const Bit8u *src, unsigned int len, bool init = false) const;
};

class PatchTempMemoryRegion : public MemoryRegion {
public:
	PatchTempMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_PatchTemp, MT32EMU_MEMADDR(0x030000), sizeof(MemParams::PatchTemp), 9) {}
};
class RhythmTempMemoryRegion : public MemoryRegion {
public:
	RhythmTempMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_RhythmTemp, MT32EMU_MEMADDR(0x030110), sizeof(MemParams::RhythmTemp), 85) {}
};
class TimbreTempMemoryRegion : public MemoryRegion {
public:
	TimbreTempMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_TimbreTemp, MT32EMU_MEMADDR(0x040000), sizeof(TimbreParam), 8) {}
};
class PatchesMemoryRegion : public MemoryRegion {
public:
	PatchesMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_Patches, MT32EMU_MEMADDR(0x050000), sizeof(PatchParam), 128) {}
};
class TimbresMemoryRegion : public MemoryRegion {
public:
	TimbresMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_Timbres, MT32EMU_MEMADDR(0x080000), sizeof(MemParams::PaddedTimbre), 64 + 64 + 64 + 64) {}
};
class SystemMemoryRegion : public MemoryRegion {
public:
	SystemMemoryRegion(Synth *useSynth, Bit8u *useRealMemory, Bit8u *useMaxTable) : MemoryRegion(useSynth, useRealMemory, useMaxTable, MR_System, MT32EMU_MEMADDR(0x100000), sizeof(MemParams::System), 1) {}
};
class DisplayMemoryRegion : public MemoryRegion {
public:
	DisplayMemoryRegion(Synth *useSynth) : MemoryRegion(useSynth, NULL, NULL, MR_Display, MT32EMU_MEMADDR(0x200000), MAX_SYSEX_SIZE - 1, 1) {}
};
class ResetMemoryRegion : public MemoryRegion {
public:
	ResetMemoryRegion(Synth *useSynth) : MemoryRegion(useSynth, NULL, NULL, MR_Reset, MT32EMU_MEMADDR(0x7F0000), 0x3FFF, 1) {}
};

class ReportHandler {
friend class Synth;

public:
	virtual ~ReportHandler() {}

protected:

	// Callback for debug messages, in vprintf() format
	virtual void printDebug(const char *fmt, va_list list);

	// Callbacks for reporting various errors and information
	virtual void onErrorControlROM() {}
	virtual void onErrorPCMROM() {}
	virtual void showLCDMessage(const char *message);
	virtual void onMIDIMessagePlayed() {}
	virtual void onDeviceReset() {}
	virtual void onDeviceReconfig() {}
	virtual void onNewReverbMode(Bit8u /* mode */) {}
	virtual void onNewReverbTime(Bit8u /* time */) {}
	virtual void onNewReverbLevel(Bit8u /* level */) {}
	virtual void onPolyStateChanged(int /* partNum */) {}
	virtual void onProgramChanged(int /* partNum */, int /* bankNum */, const char * /* patchName */) {}
};

/**
 * Used to safely store timestamped MIDI events in a local queue.
 */
struct MidiEvent {
	Bit32u shortMessageData;
	const Bit8u *sysexData;
	Bit32u sysexLength;
	Bit32u timestamp;

	~MidiEvent();
	void setShortMessage(Bit32u shortMessageData, Bit32u timestamp);
	void setSysex(const Bit8u *sysexData, Bit32u sysexLength, Bit32u timestamp);
};

/**
 * Simple queue implementation using a ring buffer to store incoming MIDI event before the synth actually processes it.
 * It is intended to:
 * - get rid of prerenderer while retaining graceful partial abortion
 * - add fair emulation of the MIDI interface delays
 * - extend the synth interface with the default implementation of a typical rendering loop.
 * THREAD SAFETY:
 * It is safe to use either in a single thread environment or when there are only two threads - one performs only reading
 * and one performs only writing. More complicated usage requires external synchronisation.
 */
class MidiEventQueue {
private:
	MidiEvent *ringBuffer;
	Bit32u ringBufferSize;
	volatile Bit32u startPosition;
	volatile Bit32u endPosition;

public:
	MidiEventQueue(Bit32u ringBufferSize = DEFAULT_MIDI_EVENT_QUEUE_SIZE);
	~MidiEventQueue();
	void reset();
	bool pushShortMessage(Bit32u shortMessageData, Bit32u timestamp);
	bool pushSysex(const Bit8u *sysexData, Bit32u sysexLength, Bit32u timestamp);
	const MidiEvent *peekMidiEvent();
	void dropMidiEvent();
};

class Synth {
friend class Part;
friend class RhythmPart;
friend class Poly;
friend class Partial;
friend class PartialManager;
friend class Tables;
friend class MemoryRegion;
friend class TVA;
friend class TVF;
friend class TVP;
private:
	PatchTempMemoryRegion *patchTempMemoryRegion;
	RhythmTempMemoryRegion *rhythmTempMemoryRegion;
	TimbreTempMemoryRegion *timbreTempMemoryRegion;
	PatchesMemoryRegion *patchesMemoryRegion;
	TimbresMemoryRegion *timbresMemoryRegion;
	SystemMemoryRegion *systemMemoryRegion;
	DisplayMemoryRegion *displayMemoryRegion;
	ResetMemoryRegion *resetMemoryRegion;

	Bit8u *paddedTimbreMaxTable;

	bool isEnabled;

	PCMWaveEntry *pcmWaves; // Array

	const ControlROMMap *controlROMMap;
	Bit8u controlROMData[CONTROL_ROM_SIZE];
	Bit16s *pcmROMData;
	size_t pcmROMSize; // This is in 16-bit samples, therefore half the number of bytes in the ROM

	unsigned int partialCount;
	Bit8s chantable[32]; // FIXME: Need explanation why 32 is set, obviously it should be 16

	MidiEventQueue *midiQueue;
	volatile Bit32u lastReceivedMIDIEventTimestamp;
	volatile Bit32u renderedSampleCount;

	MemParams mt32ram, mt32default;

	BReverbModel *reverbModels[4];
	BReverbModel *reverbModel;
	bool reverbEnabled;
	bool reverbOverridden;

	MIDIDelayMode midiDelayMode;
	DACInputMode dacInputMode;

#if MT32EMU_USE_FLOAT_SAMPLES
	float outputGain;
	float reverbOutputGain;
#else
	int outputGain;
	int reverbOutputGain;
#endif

	bool reversedStereoEnabled;

	bool isOpen;

	bool isDefaultReportHandler;
	ReportHandler *reportHandler;

	PartialManager *partialManager;
	Part *parts[9];

	// When a partial needs to be aborted to free it up for use by a new Poly,
	// the controller will busy-loop waiting for the sound to finish.
	// We emulate this by delaying new MIDI events processing until abortion finishes.
	Poly *abortingPoly;

	Bit32u getShortMessageLength(Bit32u msg);
	Bit32u addMIDIInterfaceDelay(Bit32u len, Bit32u timestamp);

	void convertSamplesToOutput(Sample *target, const Sample *source, Bit32u len, bool reverb);
	bool isAbortingPoly() const;
	void doRenderStreams(Sample *nonReverbLeft, Sample *nonReverbRight, Sample *reverbDryLeft, Sample *reverbDryRight, Sample *reverbWetLeft, Sample *reverbWetRight, Bit32u len);

	void readSysex(unsigned char channel, const Bit8u *sysex, Bit32u len) const;
	void initMemoryRegions();
	void deleteMemoryRegions();
	MemoryRegion *findMemoryRegion(Bit32u addr);
	void writeMemoryRegion(const MemoryRegion *region, Bit32u addr, Bit32u len, const Bit8u *data);
	void readMemoryRegion(const MemoryRegion *region, Bit32u addr, Bit32u len, Bit8u *data);

	bool loadControlROM(const ROMImage &controlROMImage);
	bool loadPCMROM(const ROMImage &pcmROMImage);

	bool initPCMList(Bit16u mapAddress, Bit16u count);
	bool initTimbres(Bit16u mapAddress, Bit16u offset, int timbreCount, int startTimbre, bool compressed);
	bool initCompressedTimbre(int drumNum, const Bit8u *mem, unsigned int memLen);

	void refreshSystemMasterTune();
	void refreshSystemReverbParameters();
	void refreshSystemReserveSettings();
	void refreshSystemChanAssign(unsigned int firstPart, unsigned int lastPart);
	void refreshSystemMasterVol();
	void refreshSystem();
	void reset();

	void printPartialUsage(unsigned long sampleOffset = 0);

	void polyStateChanged(int partNum);
	void newTimbreSet(int partNum, Bit8u timbreGroup, const char patchName[]);
	void printDebug(const char *fmt, ...);

public:
	static inline Bit16s clipBit16s(Bit32s sample) {
		// Clamp values above 32767 to 32767, and values below -32768 to -32768
		if ((sample + 32768) & ~65535) {
			return (sample >> 31) ^ 32767;
		}
		return (Bit16s)sample;
	}

	static Bit8u calcSysexChecksum(const Bit8u *data, Bit32u len, Bit8u checksum);

	// Optionally sets callbacks for reporting various errors, information and debug messages
	Synth(ReportHandler *useReportHandler = NULL);
	~Synth();

	// Used to initialise the MT-32. Must be called before any other function.
	// Returns true if initialization was sucessful, otherwise returns false.
	// controlROMImage and pcmROMImage represent Control and PCM ROM images for use by synth.
	// usePartialCount sets the maximum number of partials playing simultaneously for this session.
	bool open(const ROMImage &controlROMImage, const ROMImage &pcmROMImage, unsigned int usePartialCount = DEFAULT_MAX_PARTIALS);

	// Closes the MT-32 and deallocates any memory used by the synthesizer
	void close(void);

	// All the enqueued events are processed by the synth immediately.
	void flushMIDIQueue();

	// Sets size of the internal MIDI event queue.
	// The queue is flushed before reallocation.
	void setMIDIEventQueueSize(Bit32u);

	// Enqueues a MIDI event for subsequent playback.
	// The minimum delay involves the delay introduced while the event is transferred via MIDI interface
	// and emulation of the MCU busy-loop while it frees partials for use by a new Poly.
	// Calls from multiple threads must be synchronised, although,
	// no synchronisation is required with the rendering thread.

	// The MIDI event will be processed not before the specified timestamp.
	// The timestamp is measured as the global rendered sample count since the synth was created.
	bool playMsg(Bit32u msg, Bit32u timestamp);
	bool playSysex(const Bit8u *sysex, Bit32u len, Bit32u timestamp);
	// The MIDI event will be processed ASAP.
	bool playMsg(Bit32u msg);
	bool playSysex(const Bit8u *sysex, Bit32u len);

	// WARNING:
	// The methods below don't ensure minimum 1-sample delay between sequential MIDI events,
	// and a sequence of NoteOn and immediately succeeding NoteOff messages is always silent.

	// Sends a 4-byte MIDI message to the MT-32 for immediate playback.
	void playMsgNow(Bit32u msg);
	void playMsgOnPart(unsigned char part, unsigned char code, unsigned char note, unsigned char velocity);

	// Sends a string of Sysex commands to the MT-32 for immediate interpretation
	// The length is in bytes
	void playSysexNow(const Bit8u *sysex, Bit32u len);
	void playSysexWithoutFraming(const Bit8u *sysex, Bit32u len);
	void playSysexWithoutHeader(unsigned char device, unsigned char command, const Bit8u *sysex, Bit32u len);
	void writeSysex(unsigned char channel, const Bit8u *sysex, Bit32u len);

	void setReverbEnabled(bool reverbEnabled);
	bool isReverbEnabled() const;
	void setReverbOverridden(bool reverbOverridden);
	bool isReverbOverridden() const;
	void setDACInputMode(DACInputMode mode);
	DACInputMode getDACInputMode() const;
	void setMIDIDelayMode(MIDIDelayMode mode);
	MIDIDelayMode getMIDIDelayMode() const;

	// Sets output gain factor. Applied to all output samples and unrelated with the synth's Master volume.
	// Ignored in DACInputMode_PURE
	void setOutputGain(float);
	float getOutputGain() const;

	// Sets output gain factor for the reverb wet output. setOutputGain() doesn't change reverb output gain.
	// Note: We're currently emulate CM-32L/CM-64 reverb quite accurately and the reverb output level closely
	// corresponds to the level of digital capture. Although, according to the CM-64 PCB schematic,
	// there is a difference in the reverb analogue circuit, and the resulting output gain is 0.68
	// of that for LA32 analogue output. This factor is applied to the reverb output gain.
	// Ignored in DACInputMode_PURE
	void setReverbOutputGain(float);
	float getReverbOutputGain() const;

	void setReversedStereoEnabled(bool enabled);
	bool isReversedStereoEnabled();

	// Renders samples to the specified output stream.
	// The length is in frames, not bytes (in 16-bit stereo,
	// one frame is 4 bytes).
	void render(Sample *stream, Bit32u len);

	// Renders samples to the specified output streams (any or all of which may be NULL).
	void renderStreams(Sample *nonReverbLeft, Sample *nonReverbRight, Sample *reverbDryLeft, Sample *reverbDryRight, Sample *reverbWetLeft, Sample *reverbWetRight, Bit32u len);

	// Returns true when there is at least one active partial, otherwise false.
	bool hasActivePartials() const;

	// Returns true if hasActivePartials() returns true, or reverb is (somewhat unreliably) detected as being active.
	bool isActive() const;

	const Partial *getPartial(unsigned int partialNum) const;

	// Returns the maximum number of partials playing simultaneously.
	unsigned int getPartialCount() const;

	void readMemory(Bit32u addr, Bit32u len, Bit8u *data);

	// partNum should be 0..7 for Part 1..8, or 8 for Rhythm
	const Part *getPart(unsigned int partNum) const;
};

}

#endif
