/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011, 2012, 2013, 2014 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
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
//#include <cstring>

namespace MT32Emu {

class Analog;
class BReverbModel;
class MemoryRegion;
class MidiEventQueue;
class Part;
class Poly;
class Partial;
class PartialManager;

class PatchTempMemoryRegion;
class RhythmTempMemoryRegion;
class TimbreTempMemoryRegion;
class PatchesMemoryRegion;
class TimbresMemoryRegion;
class SystemMemoryRegion;
class DisplayMemoryRegion;
class ResetMemoryRegion;

struct ControlROMMap;
struct PCMWaveEntry;
struct MemParams;

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
	// * Half the volume of any of the other modes.
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

// Methods for emulating the effective delay of incoming MIDI messages introduced by a MIDI interface.
enum MIDIDelayMode {
	// Process incoming MIDI events immediately.
	MIDIDelayMode_IMMEDIATE,

	// Delay incoming short MIDI messages as if they where transferred via a MIDI cable to a real hardware unit and immediate sysex processing.
	// This ensures more accurate timing of simultaneous NoteOn messages.
	MIDIDelayMode_DELAY_SHORT_MESSAGES_ONLY,

	// Delay all incoming MIDI events as if they where transferred via a MIDI cable to a real hardware unit.
	MIDIDelayMode_DELAY_ALL
};

// Methods for emulating the effects of analogue circuits of real hardware units on the output signal.
enum AnalogOutputMode {
	// Only digital path is emulated. The output samples correspond to the digital signal at the DAC entrance.
	AnalogOutputMode_DIGITAL_ONLY,
	// Coarse emulation of LPF circuit. High frequencies are boosted, sample rate remains unchanged.
	AnalogOutputMode_COARSE,
	// Finer emulation of LPF circuit. Output signal is upsampled to 48 kHz to allow emulation of audible mirror spectra above 16 kHz,
	// which is passed through the LPF circuit without significant attenuation.
	AnalogOutputMode_ACCURATE,
	// Same as AnalogOutputMode_ACCURATE mode but the output signal is 2x oversampled, i.e. the output sample rate is 96 kHz.
	// This makes subsequent resampling easier. Besides, due to nonlinear passband of the LPF emulated, it takes fewer number of MACs
	// compared to a regular LPF FIR implementations.
	AnalogOutputMode_OVERSAMPLED
};

enum ReverbMode {
	REVERB_MODE_ROOM,
	REVERB_MODE_HALL,
	REVERB_MODE_PLATE,
	REVERB_MODE_TAP_DELAY
};

enum PartialState {
	PartialState_INACTIVE,
	PartialState_ATTACK,
	PartialState_SUSTAIN,
	PartialState_RELEASE
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

const int MAX_SYSEX_SIZE = 512; // FIXME: Does this correspond to a real MIDI buffer used in h/w devices?

const unsigned int CONTROL_ROM_SIZE = 64 * 1024;

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

	const ControlROMFeatureSet *controlROMFeatures;
	const ControlROMMap *controlROMMap;
	Bit8u controlROMData[CONTROL_ROM_SIZE];
	Bit16s *pcmROMData;
	size_t pcmROMSize; // This is in 16-bit samples, therefore half the number of bytes in the ROM

	unsigned int partialCount;
	Bit8s chantable[32]; // FIXME: Need explanation why 32 is set, obviously it should be 16

	MidiEventQueue *midiQueue;
	volatile Bit32u lastReceivedMIDIEventTimestamp;
	volatile Bit32u renderedSampleCount;

	MemParams &mt32ram, &mt32default;

	BReverbModel *reverbModels[4];
	BReverbModel *reverbModel;
	bool reverbOverridden;

	MIDIDelayMode midiDelayMode;
	DACInputMode dacInputMode;

	float outputGain;
	float reverbOutputGain;

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

	Analog *analog;

	Bit32u addMIDIInterfaceDelay(Bit32u len, Bit32u timestamp);

	void produceLA32Output(Sample *buffer, Bit32u len);
	void convertSamplesToOutput(Sample *buffer, Bit32u len);
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

	// partNum should be 0..7 for Part 1..8, or 8 for Rhythm
	const Part *getPart(unsigned int partNum) const;

public:
	static inline Sample clipSampleEx(SampleEx sampleEx) {
#if MT32EMU_USE_FLOAT_SAMPLES
		return sampleEx;
#else
		// Clamp values above 32767 to 32767, and values below -32768 to -32768
		// FIXME: Do we really need this stuff? I think these branches are very well predicted. Instead, this introduces a chain.
		// The version below is actually a bit faster on my system...
		//return ((sampleEx + 0x8000) & ~0xFFFF) ? (sampleEx >> 31) ^ 0x7FFF : (Sample)sampleEx;
		return ((-0x8000 <= sampleEx) && (sampleEx <= 0x7FFF)) ? (Sample)sampleEx : (sampleEx >> 31) ^ 0x7FFF;
#endif
	}

	static inline void muteSampleBuffer(Sample *buffer, Bit32u len) {
		if (buffer == NULL) return;

#if MT32EMU_USE_FLOAT_SAMPLES
		// FIXME: Use memset() where compatibility is guaranteed (if this turns out to be a win)
		while (len--) {
			*(buffer++) = 0.0f;
		}
#else
		memset(buffer, 0, len * sizeof(Sample));
#endif
	}

	static Bit32u getShortMessageLength(Bit32u msg);
	static Bit8u calcSysexChecksum(const Bit8u *data, const Bit32u len, const Bit8u initChecksum = 0);

	// Optionally sets callbacks for reporting various errors, information and debug messages
	Synth(ReportHandler *useReportHandler = NULL);
	~Synth();

	// Used to initialise the MT-32. Must be called before any other function.
	// Returns true if initialization was sucessful, otherwise returns false.
	// controlROMImage and pcmROMImage represent Control and PCM ROM images for use by synth.
	// usePartialCount sets the maximum number of partials playing simultaneously for this session (optional).
	// analogOutputMode sets the mode for emulation of analogue circuitry of the hardware units (optional).
	bool open(const ROMImage &controlROMImage, const ROMImage &pcmROMImage, unsigned int usePartialCount = DEFAULT_MAX_PARTIALS, AnalogOutputMode analogOutputMode = AnalogOutputMode_COARSE);

	// Overloaded method which opens the synth with default partial count.
	bool open(const ROMImage &controlROMImage, const ROMImage &pcmROMImage, AnalogOutputMode analogOutputMode);

	// Closes the MT-32 and deallocates any memory used by the synthesizer
	void close(bool forced = false);

	// All the enqueued events are processed by the synth immediately.
	void flushMIDIQueue();

	// Sets size of the internal MIDI event queue. The queue size is set to the minimum power of 2 that is greater or equal to the size specified.
	// The queue is flushed before reallocation.
	// Returns the actual queue size being used.
	Bit32u setMIDIEventQueueSize(Bit32u);

	// Enqueues a MIDI event for subsequent playback.
	// The MIDI event will be processed not before the specified timestamp.
	// The timestamp is measured as the global rendered sample count since the synth was created (at the native sample rate 32000 Hz).
	// The minimum delay involves emulation of the delay introduced while the event is transferred via MIDI interface
	// and emulation of the MCU busy-loop while it frees partials for use by a new Poly.
	// Calls from multiple threads must be synchronised, although, no synchronisation is required with the rendering thread.
	// The methods return false if the MIDI event queue is full and the message cannot be enqueued.

	// Enqueues a single short MIDI message. The message must contain a status byte.
	bool playMsg(Bit32u msg, Bit32u timestamp);
	// Enqueues a single well formed System Exclusive MIDI message.
	bool playSysex(const Bit8u *sysex, Bit32u len, Bit32u timestamp);

	// Overloaded methods for the MIDI events to be processed ASAP.
	bool playMsg(Bit32u msg);
	bool playSysex(const Bit8u *sysex, Bit32u len);

	// WARNING:
	// The methods below don't ensure minimum 1-sample delay between sequential MIDI events,
	// and a sequence of NoteOn and immediately succeeding NoteOff messages is always silent.
	// A thread that invokes these methods must be explicitly synchronised with the thread performing sample rendering.

	// Sends a short MIDI message to the synth for immediate playback. The message must contain a status byte.
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
	// Sets override reverb mode. In this mode, emulation ignores sysexes (or the related part of them) which control the reverb parameters.
	// This mode is in effect until it is turned off. When the synth is re-opened, the override mode is unchanged but the state
	// of the reverb model is reset to default.
	void setReverbOverridden(bool reverbOverridden);
	bool isReverbOverridden() const;
	// Forces reverb model compatibility mode. By default, the compatibility mode corresponds to the used control ROM version.
	// Invoking this method with the argument set to true forces emulation of old MT-32 reverb circuit.
	// When the argument is false, emulation of the reverb circuit used in new generation of MT-32 compatible modules is enforced
	// (these include CM-32L and LAPC-I).
	void setReverbCompatibilityMode(bool mt32CompatibleMode);
	bool isMT32ReverbCompatibilityMode() const;
	void setDACInputMode(DACInputMode mode);
	DACInputMode getDACInputMode() const;
	void setMIDIDelayMode(MIDIDelayMode mode);
	MIDIDelayMode getMIDIDelayMode() const;

	// Sets output gain factor for synth output channels. Applied to all output samples and unrelated with the synth's Master volume,
	// it rather corresponds to the gain of the output analog circuitry of the hardware units. However, together with setReverbOutputGain()
	// it offers to the user a capability to control the gain of reverb and non-reverb output channels independently.
	// Ignored in DACInputMode_PURE
	void setOutputGain(float);
	float getOutputGain() const;

	// Sets output gain factor for the reverb wet output channels. It rather corresponds to the gain of the output
	// analog circuitry of the hardware units. However, together with setOutputGain() it offers to the user a capability
	// to control the gain of reverb and non-reverb output channels independently.
	//
	// Note: We're currently emulate CM-32L/CM-64 reverb quite accurately and the reverb output level closely
	// corresponds to the level of digital capture. Although, according to the CM-64 PCB schematic,
	// there is a difference in the reverb analogue circuit, and the resulting output gain is 0.68
	// of that for LA32 analogue output. This factor is applied to the reverb output gain.
	// Ignored in DACInputMode_PURE
	void setReverbOutputGain(float);
	float getReverbOutputGain() const;

	void setReversedStereoEnabled(bool enabled);
	bool isReversedStereoEnabled();

	// Returns actual sample rate used in emulation of stereo analog circuitry of hardware units.
	// See comment for render() below.
	unsigned int getStereoOutputSampleRate() const;

	// Renders samples to the specified output stream as if they were sampled at the analog stereo output.
	// When AnalogOutputMode is set to ACCURATE, the output signal is upsampled to 48 kHz in order
	// to retain emulation accuracy in whole audible frequency spectra. Otherwise, native digital signal sample rate is retained.
	// getStereoOutputSampleRate() can be used to query actual sample rate of the output signal.
	// The length is in frames, not bytes (in 16-bit stereo, one frame is 4 bytes).
	void render(Sample *stream, Bit32u len);

	// Renders samples to the specified output streams as if they appeared at the DAC entrance.
	// No further processing performed in analog circuitry emulation is applied to the signal.
	// NULL may be specified in place of any or all of the stream buffers.
	// The length is in samples, not bytes.
	void renderStreams(Sample *nonReverbLeft, Sample *nonReverbRight, Sample *reverbDryLeft, Sample *reverbDryRight, Sample *reverbWetLeft, Sample *reverbWetRight, Bit32u len);

	// Returns true when there is at least one active partial, otherwise false.
	bool hasActivePartials() const;

	// Returns true if hasActivePartials() returns true, or reverb is (somewhat unreliably) detected as being active.
	bool isActive() const;

	// Returns the maximum number of partials playing simultaneously.
	unsigned int getPartialCount() const;

	// Fills in current states of all the parts into the array provided. The array must have at least 9 entries to fit values for all the parts.
	// If the value returned for a part is true, there is at least one active non-releasing partial playing on this part.
	// This info is useful in emulating behaviour of LCD display of the hardware units.
	void getPartStates(bool *partStates) const;

	// Fills in current states of all the partials into the array provided. The array must be large enough to accommodate states of all the partials.
	void getPartialStates(PartialState *partialStates) const;

	// Fills in information about currently playing notes on the specified part into the arrays provided. The arrays must be large enough
	// to accommodate data for all the playing notes. The maximum number of simultaneously playing notes cannot exceed the number of partials.
	// Argument partNumber should be 0..7 for Part 1..8, or 8 for Rhythm.
	// Returns the number of currently playing notes on the specified part.
	unsigned int getPlayingNotes(unsigned int partNumber, Bit8u *keys, Bit8u *velocities) const;

	// Returns name of the patch set on the specified part.
	// Argument partNumber should be 0..7 for Part 1..8, or 8 for Rhythm.
	const char *getPatchName(unsigned int partNumber) const;

	void readMemory(Bit32u addr, Bit32u len, Bit8u *data);
};

}

#endif
