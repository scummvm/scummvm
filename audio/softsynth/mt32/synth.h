/* Copyright (c) 2003-2005 Various contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MT32EMU_SYNTH_H
#define MT32EMU_SYNTH_H

#include "common/scummsys.h"

class revmodel;

namespace MT32Emu {

class File;
class Partial;
class PartialManager;
class Part;

enum ReportType {
	// Errors
	ReportType_errorControlROM = 1,
	ReportType_errorPCMROM,
	ReportType_errorSampleRate,

	// Progress
	ReportType_progressInit,

	// HW spec
	ReportType_availableSSE,
	ReportType_available3DNow,
	ReportType_usingSSE,
	ReportType_using3DNow,

	// General info
	ReportType_lcdMessage,
	ReportType_devReset,
	ReportType_devReconfig,
	ReportType_newReverbMode,
	ReportType_newReverbTime,
	ReportType_newReverbLevel
};

struct SynthProperties {
	// Sample rate to use in mixing
	int sampleRate;

	// Flag to activate reverb.  True = use reverb, False = no reverb
	bool useReverb;
	// True to use software set reverb settings, False to set reverb settings in
	// following parameters
	bool useDefaultReverb;
	// When not using the default settings, this specifies one of the 4 reverb types
	// 1 = Room 2 = Hall 3 = Plate 4 = Tap
	unsigned char reverbType;
	// This specifies the delay time, from 0-7 (not sure of the actual MT-32's measurement)
	unsigned char reverbTime;
	// This specifies the reverb level, from 0-7 (not sure of the actual MT-32's measurement)
	unsigned char reverbLevel;
	// The name of the directory in which the ROM and data files are stored (with trailing slash/backslash)
	// Not used if "openFile" is set. May be NULL in any case.
	char *baseDir;
	// This is used as the first argument to all callbacks
	void *userData;
	// Callback for reporting various errors and information. May be NULL
	int (*report)(void *userData, ReportType type, const void *reportData);
	// Callback for debug messages, in vprintf() format
	void (*printDebug)(void *userData, const char *fmt, va_list list);
	// Callback for providing an implementation of File, opened and ready for use
	// May be NULL, in which case a default implementation will be used.
	File *(*openFile)(void *userData, const char *filename, File::OpenMode mode);
	// Callback for closing a File. May be NULL, in which case the File will automatically be close()d/deleted.
	void (*closeFile)(void *userData, File *file);
};

// This is the specification of the Callback routine used when calling the RecalcWaveforms
// function
typedef void (*recalcStatusCallback)(int percDone);

// This external function recreates the base waveform file (waveforms.raw) using a specifed
// sampling rate.  The callback routine provides interactivity to let the user know what
// percentage is complete in regenerating the waveforms.  When a NULL pointer is used as the
// callback routine, no status is reported.
bool RecalcWaveforms(char * baseDir, int sampRate, recalcStatusCallback callBack);

typedef float (*iir_filter_type)(float input,float *hist1_ptr, float *coef_ptr);

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

const unsigned int CONTROL_ROM_SIZE = 64 * 1024;

struct ControlROMPCMStruct
{
	Bit8u pos;
	Bit8u len;
	Bit8u pitchLSB;
	Bit8u pitchMSB;
};

struct ControlROMMap {
	Bit16u idPos;
	Bit16u idLen;
	const char *idBytes;
	Bit16u pcmTable;
	Bit16u pcmCount;
	Bit16u timbreAMap;
	Bit16u timbreAOffset;
	Bit16u timbreBMap;
	Bit16u timbreBOffset;
	Bit16u timbreRMap;
	Bit16u timbreRCount;
	Bit16u rhythmSettings;
	Bit16u rhythmSettingsCount;
	Bit16u reserveSettings;
	Bit16u panSettings;
	Bit16u programSettings;
};

enum MemoryRegionType {
	MR_PatchTemp, MR_RhythmTemp, MR_TimbreTemp, MR_Patches, MR_Timbres, MR_System, MR_Display, MR_Reset
};

class MemoryRegion {
public:
	MemoryRegionType type;
	Bit32u startAddr, entrySize, entries;

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
};


class Synth {
friend class Part;
friend class RhythmPart;
friend class Partial;
friend class Tables;
private:
	bool isEnabled;

	iir_filter_type iirFilter;

	PCMWaveEntry *pcmWaves; // Array

	const ControlROMMap *controlROMMap;
	Bit8u controlROMData[CONTROL_ROM_SIZE];
	Bit16s *pcmROMData;
	int pcmROMSize; // This is in 16-bit samples, therefore half the number of bytes in the ROM

	Bit8s chantable[32];

	#if MT32EMU_MONITOR_PARTIALS == 1
	static Bit32s samplepos = 0;
	#endif

	Tables tables;

	MemParams mt32ram, mt32default;

	revmodel *reverbModel;

	float masterTune;
	Bit16u masterVolume;

	bool isOpen;

	PartialManager *partialManager;
	Part *parts[9];

	Bit16s tmpBuffer[MAX_SAMPLE_OUTPUT * 2];
	float sndbufl[MAX_SAMPLE_OUTPUT];
	float sndbufr[MAX_SAMPLE_OUTPUT];
	float outbufl[MAX_SAMPLE_OUTPUT];
	float outbufr[MAX_SAMPLE_OUTPUT];

	SynthProperties myProp;

	bool loadPreset(File *file);
	void initReverb(Bit8u newRevMode, Bit8u newRevTime, Bit8u newRevLevel);
	void doRender(Bit16s * stream, Bit32u len);

	void playAddressedSysex(unsigned char channel, const Bit8u *sysex, Bit32u len);
	void readSysex(unsigned char channel, const Bit8u *sysex, Bit32u len);
	void writeMemoryRegion(const MemoryRegion *region, Bit32u addr, Bit32u len, const Bit8u *data);
	void readMemoryRegion(const MemoryRegion *region, Bit32u addr, Bit32u len, Bit8u *data);

	bool loadControlROM(const char *filename);
	bool loadPCMROM(const char *filename);
	bool dumpTimbre(File *file, const TimbreParam *timbre, Bit32u addr);
	int dumpTimbres(const char *filename, int start, int len);

	bool initPCMList(Bit16u mapAddress, Bit16u count);
	bool initRhythmTimbres(Bit16u mapAddress, Bit16u count);
	bool initTimbres(Bit16u mapAddress, Bit16u offset, int startTimbre);
	bool initRhythmTimbre(int drumNum, const Bit8u *mem, unsigned int memLen);
	bool refreshSystem();

protected:
	int report(ReportType type, const void *reportData);
	File *openFile(const char *filename, File::OpenMode mode);
	void closeFile(File *file);
	void printDebug(const char *fmt, ...) GCC_PRINTF(2, 3);

public:
	static Bit8u calcSysexChecksum(const Bit8u *data, Bit32u len, Bit8u checksum);

	Synth();
	~Synth();

	// Used to initialize the MT-32. Must be called before any other function.
	// Returns true if initialization was sucessful, otherwise returns false.
	bool open(SynthProperties &useProp);

	// Closes the MT-32 and deallocates any memory used by the synthesizer
	void close(void);

	// Sends a 4-byte MIDI message to the MT-32 for immediate playback
	void playMsg(Bit32u msg);
	void playMsgOnPart(unsigned char part, unsigned char code, unsigned char note, unsigned char velocity);

	// Sends a string of Sysex commands to the MT-32 for immediate interpretation
	// The length is in bytes
	void playSysex(const Bit8u *sysex, Bit32u len);
	void playSysexWithoutFraming(const Bit8u *sysex, Bit32u len);
	void playSysexWithoutHeader(unsigned char device, unsigned char command, const Bit8u *sysex, Bit32u len);
	void writeSysex(unsigned char channel, const Bit8u *sysex, Bit32u len);

	// This callback routine is used to have the MT-32 generate samples to the specified
	// output stream.  The length is in whole samples, not bytes. (I.E. in 16-bit stereo,
	// one sample is 4 bytes)
	void render(Bit16s * stream, Bit32u len);

	const Partial *getPartial(unsigned int partialNum) const;

	void readMemory(Bit32u addr, Bit32u len, Bit8u *data);

	// partNum should be 0..7 for Part 1..8, or 8 for Rhythm
	const Part *getPart(unsigned int partNum) const;
};

}

#endif
