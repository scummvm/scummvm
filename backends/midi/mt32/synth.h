/* Copyright (c) 2003-2004 Various contributors
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

#include <stdarg.h>

// Shows number of partials MT-32 is playing
#define MONITORPARTIALS 0

#define ROMSIZE (512 * 1024)
#define PCMSIZE (ROMSIZE / 2)
#define GRAN 512

class revmodel;

namespace MT32Emu {

class File;
class TableInitialiser;
class Partial;
class PartialManager;
class Part;

enum ReportType {
	// Files missing
	ReportType_errorPreset1    = 1,
	ReportType_errorPreset2    = 2,
	ReportType_errorDrumpat    = 3,
	ReportType_errorPatchlog   = 4,
	ReportType_errorMT32ROM    = 5,

	// HW spec
	ReportType_availableSSE    = 6,
	ReportType_available3DNow  = 7,
	ReportType_asingSSE        = 8,
	ReportType_asing3DNow      = 9,

	// General info
	ReportType_lcdMessage      = 10,
	ReportType_devReset        = 11,
	ReportType_devReconfig     = 12,
	ReportType_newReverbMode   = 13,
	ReportType_newReverbTime   = 14,
	ReportType_newReverbLevel  = 15
};

struct SynthProperties {
	// Sample rate to use in mixing
	int SampleRate;

	// Flag to activate reverb.  True = use reverb, False = no reverb
	bool UseReverb;
	// Flag True to use software set reverb settings, Flag False to set reverb settings in
	// following parameters
	bool UseDefault;
	// When not using the default settings, this specifies one of the 4 reverb types
	// 1 = Room 2 = Hall 3 = Plate 4 = Tap
	unsigned char RevType;
	// This specifies the delay time, from 0-7 (not sure of the actual MT-32's measurement)
	unsigned char RevTime;
	// This specifies the reverb level, from 0-7 (not sure of the actual MT-32's measurement)
	unsigned char RevLevel;
	// The name of the directory in which the ROM and data files are stored (with trailing slash/backslash)
	// Not used if "openFile" is set. May be NULL in any case.
	char *baseDir;
	// This is used as the first argument to all callbacks
	void *userData;
	// Callback for reporting various errors and information. May be NULL
	void (*report)(void *userData, ReportType type, void *reportData);
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

typedef float (*iir_filter_type)(float input,float *hist1_ptr, float *coef_ptr, int revLevel);
extern iir_filter_type usefilter;

class Synth {
friend class Part;
friend class Partial;
friend class TableInitialiser;
private:
	bool isEnabled;

	sampleFormat PCM[54];
	sampleTable PCMList[128];
	Bit32u PCMReassign[54];
	Bit32s PCMLoopTable[54];

	Bit16s romfile[PCMSIZE + GRAN];
	Bit8s chantable[32];

	#if MONITORPARTIALS == 1
	static Bit32s samplepos = 0;
	#endif

	MT32RAMFormat mt32ram, mt32default;

	revmodel *reverbModel;

	Bit16s mastervolume;

	char curRevMode;
	char curRevTime;
	Bit32u curRevLevel;

	unsigned char initmode;
	bool isOpen;

	PartialManager *partialManager;
	Part *parts[9];

	Bit16s tmpBuffer[MAX_SAMPLE_OUTPUT * 2];
	float sndbufl[MAX_SAMPLE_OUTPUT];
	float sndbufr[MAX_SAMPLE_OUTPUT];
	float outbufl[MAX_SAMPLE_OUTPUT];
	float outbufr[MAX_SAMPLE_OUTPUT];

	SynthProperties myProp;

	bool loadPreset(const char *filename);
	void initReverb(char newRevMode, char newRevTime);
	void doRender(Bit16s * stream, Bit32u len);
	void playMsgOnPart(unsigned char part, unsigned char code, unsigned char note, unsigned char velocity);
	void playSysexWithoutHeader(unsigned char channel, Bit8u *sysex, Bit32u len);

	bool loadDrums(const char *filename);
	bool loadPCMToROMMap(const char *filename);
	bool loadROM(const char *filename);
	void dumpDrums(const char *filename);
	// Save the system state to a sysex file specified by filename
	int dumpSysex(char *filename);

protected:
	void report(ReportType type, void *reportData);
	File *openFile(const char *filename, File::OpenMode mode);
	void closeFile(File *file);
	void printDebug(const char *fmt, ...);

public:
	Synth();
	~Synth();

	// Used to initialise the MT-32. Must be called before any other function.
	// Returns true if initialization was sucessful, otherwise returns false.
	bool open(SynthProperties &useProp);

	// Closes the MT-32 and deallocates any memory used by the synthesizer
	void close(void);

	// Sends a 4-byte MIDI message to the MT-32 for immediate playback
	void playMsg(Bit32u msg);

	// Sends a string of Sysex commands to the MT-32 for immediate interpretation
	// The length is in bytes
	void playSysex(Bit8u *sysex, Bit32u len);
	void playSysexWithoutFraming(Bit8u *sysex, Bit32u len);

	// This callback routine is used to have the MT-32 generate samples to the specified
	// output stream.  The length is in whole samples, not bytes. (I.E. in 16-bit stereo,
	// one sample is 4 bytes)
	void render(Bit16s * stream, Bit32u len);
};

}

#endif
