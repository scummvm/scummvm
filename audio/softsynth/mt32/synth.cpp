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

// FIXME: Avoid using printf
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

// FIXME: Avoid using vprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_vprintf

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "mt32emu.h"

#if defined(MACOSX) || defined(SOLARIS) || defined(__MINGW32__)
// Older versions of Mac OS X didn't supply a powf function, so using it
// will cause a binary incompatibility when trying to run a binary built
// on a newer OS X release on an older one. And Solaris 8 doesn't provide
// powf, floorf, fabsf etc. at all.
// Cross-compiled MinGW32 toolchains suffer from a cross-compile bug in
// libstdc++. math/stubs.o should be empty, but it comes with a symbol for
// powf, resulting in a linker error because of multiple definitions.
// Hence we re-define them here. The only potential drawback is that it
// might be a little bit slower this way.
#define powf(x,y)	((float)pow(x,y))
#define floorf(x)	((float)floor(x))
#define fabsf(x)	((float)fabs(x))
#endif

namespace MT32Emu {

const int MAX_SYSEX_SIZE = 512;

const ControlROMMap ControlROMMaps[5] = {
	// ID    IDc IDbytes                     PCMmap  PCMc  tmbrA   tmbrAO, tmbrB   tmbrBO, tmbrR   trC  rhythm  rhyC  rsrv    panpot  prog
	{0x4014, 22, "\000 ver1.04 14 July 87 ", 0x3000,  128, 0x8000, 0x0000, 0xC000, 0x4000, 0x3200,  30, 0x73A6,  85,  0x57C7, 0x57D0, 0x57E2}, // MT-32 revision 0
	{0x4014, 22, "\000 ver1.06 31 Aug, 87 ", 0x3000,  128, 0x8000, 0x0000, 0xC000, 0x4000, 0x3200,  30, 0x7414,  85,  0x57D9, 0x57E2, 0x57F4}, // MT-32 revision 0
	{0x4010, 22, "\000 ver1.07 10 Oct, 87 ", 0x3000,  128, 0x8000, 0x0000, 0xC000, 0x4000, 0x3200,  30, 0x73fe,  85,  0x57B1, 0x57BA, 0x57CC}, // MT-32 revision 1
	{0x4010, 22, "\000verX.XX  30 Sep, 88 ", 0x3000,  128, 0x8000, 0x0000, 0xC000, 0x4000, 0x3200,  30, 0x741C,  85,  0x57E5, 0x57EE, 0x5800}, // MT-32 Blue Ridge mod
	{0x2205, 22, "\000CM32/LAPC1.02 891205", 0x8100,  256, 0x8000, 0x8000, 0x8080, 0x8000, 0x8500,  64, 0x8580,  85,  0x4F93, 0x4F9C, 0x4FAE}  // CM-32L
	// (Note that all but CM-32L ROM actually have 86 entries for rhythmTemp)
};

float iir_filter_normal(float input, float *hist1_ptr, float *coef_ptr) {
	float *hist2_ptr;
	float output,new_hist;

	hist2_ptr = hist1_ptr + 1; // next history

	// 1st number of coefficients array is overall input scale factor, or filter gain
	output = input * (*coef_ptr++);

	output = output - *hist1_ptr * (*coef_ptr++);
	new_hist = output - *hist2_ptr * (*coef_ptr++); // poles

	output = new_hist + *hist1_ptr * (*coef_ptr++);
	output = output + *hist2_ptr * (*coef_ptr++);   // zeros

	*hist2_ptr++ = *hist1_ptr;
	*hist1_ptr++ = new_hist;
	hist1_ptr++;
	hist2_ptr++;

	// i = 1
	output = output - *hist1_ptr * (*coef_ptr++);
	new_hist = output - *hist2_ptr * (*coef_ptr++); // poles

	output = new_hist + *hist1_ptr * (*coef_ptr++);
	output = output + *hist2_ptr * (*coef_ptr++);   // zeros

	*hist2_ptr++ = *hist1_ptr;
	*hist1_ptr++ = new_hist;

	return(output);
}

Bit8u Synth::calcSysexChecksum(const Bit8u *data, Bit32u len, Bit8u checksum) {
	for (unsigned int i = 0; i < len; i++) {
		checksum = checksum + data[i];
	}
	checksum = checksum & 0x7f;
	if (checksum)
		checksum = 0x80 - checksum;
	return checksum;
}

Synth::Synth() {
	isOpen = false;
	reverbModel = NULL;
	partialManager = NULL;
	memset(parts, 0, sizeof(parts));
}

Synth::~Synth() {
	close(); // Make sure we're closed and everything is freed
}

int Synth::report(ReportType type, const void *data) {
	if (myProp.report != NULL) {
		return myProp.report(myProp.userData, type, data);
	}
	return 0;
}

void Synth::printDebug(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	if (myProp.printDebug != NULL) {
		myProp.printDebug(myProp.userData, fmt, ap);
	} else {
		vprintf(fmt, ap);
		printf("\n");
	}
	va_end(ap);
}

void Synth::initReverb(Bit8u newRevMode, Bit8u newRevTime, Bit8u newRevLevel) {
	// FIXME:KG: I don't think it's necessary to recreate the reverbModel... Just set the parameters
	delete reverbModel;
	reverbModel = new revmodel();

	switch (newRevMode) {
	case 0:
		reverbModel->setroomsize(.1f);
		reverbModel->setdamp(.75f);
		break;
	case 1:
		reverbModel->setroomsize(.5f);
		reverbModel->setdamp(.5f);
		break;
	case 2:
		reverbModel->setroomsize(.5f);
		reverbModel->setdamp(.1f);
		break;
	case 3:
		reverbModel->setroomsize(1.0f);
		reverbModel->setdamp(.75f);
		break;
	default:
		reverbModel->setroomsize(.1f);
		reverbModel->setdamp(.5f);
		break;
	}
	reverbModel->setdry(1);
	reverbModel->setwet((float)newRevLevel / 8.0f);
	reverbModel->setwidth((float)newRevTime / 8.0f);
}

File *Synth::openFile(const char *filename, File::OpenMode mode) {
	// It should never happen that openFile is NULL in our use case.
	// Just to cover the case where something is horrible wrong we
	// use an assert here.
	assert(myProp.openFile != NULL);
	return myProp.openFile(myProp.userData, filename, mode);
}

void Synth::closeFile(File *file) {
	if (myProp.closeFile != NULL) {
		myProp.closeFile(myProp.userData, file);
	} else {
		file->close();
		delete file;
	}
}

bool Synth::loadPreset(File *file) {
	bool inSys = false;
	Bit8u sysexBuf[MAX_SYSEX_SIZE];
	Bit16u syslen = 0;
	bool rc = true;
	for (;;) {
		Bit8u c;
		if (!file->readBit8u(&c)) {
			if (!file->isEOF()) {
				rc = false;
			}
			break;
		}
		sysexBuf[syslen] = c;
		if (inSys) {
			syslen++;
			if (c == 0xF7) {
				playSysex(&sysexBuf[0], syslen);
				inSys = false;
				syslen = 0;
			} else if (syslen == MAX_SYSEX_SIZE) {
				printDebug("MAX_SYSEX_SIZE (%d) exceeded while processing preset, ignoring message", MAX_SYSEX_SIZE);
				inSys = false;
				syslen = 0;
			}
		} else if (c == 0xF0) {
			syslen++;
			inSys = true;
		}
	}
	return rc;
}

bool Synth::loadControlROM(const char *filename) {
	File *file = openFile(filename, File::OpenMode_read); // ROM File
	if (file == NULL) {
		return false;
	}
	bool rc = (file->read(controlROMData, CONTROL_ROM_SIZE) == CONTROL_ROM_SIZE);

	closeFile(file);
	if (!rc)
		return rc;

	// Control ROM successfully loaded, now check whether it's a known type
	controlROMMap = NULL;
	for (unsigned int i = 0; i < sizeof (ControlROMMaps) / sizeof (ControlROMMaps[0]); i++) {
		if (memcmp(&controlROMData[ControlROMMaps[i].idPos], ControlROMMaps[i].idBytes, ControlROMMaps[i].idLen) == 0) {
			controlROMMap = &ControlROMMaps[i];
			return true;
		}
	}
	return false;
}

bool Synth::loadPCMROM(const char *filename) {
	File *file = openFile(filename, File::OpenMode_read); // ROM File
	if (file == NULL) {
		return false;
	}
	bool rc = true;
	int i;
	for (i = 0; i < pcmROMSize; i++) {
		Bit8u s;
		if (!file->readBit8u(&s)) {
			if (!file->isEOF()) {
				rc = false;
			}
			break;
		}
		Bit8u c;
		if (!file->readBit8u(&c)) {
			if (!file->isEOF()) {
				rc = false;
			} else {
				printDebug("PCM ROM file has an odd number of bytes! Ignoring last");
			}
			break;
		}

		short e;
		int bit;
		int u;
		int order[16] = {0, 9, 1 ,2, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15, 8};

		e = 0;
		for (u = 0; u < 15; u++) {
			if (order[u] < 8)
				bit = (s >> (7 - order[u])) & 0x1;
			else
				bit = (c >> (7  - (order[u] - 8))) & 0x1;
			e = e | (short)(bit << (15 - u));
		}

		/*
		//Bit16s e = (  ((s & 0x7f) << 4) | ((c & 0x40) << 6) | ((s & 0x80) << 6) | ((c & 0x3f))) << 2;
		if (e<0)
			e = -32767 - e;
		int ut = abs(e);
		int dif = 0x7fff - ut;
		x = exp(((float)((float)0x8000-(float)dif) / (float)0x1000));
		e = (int)((float)e * (x/3200));
		*/

		// File is companded (dB?), convert to linear PCM
		// MINDB = -96
		// MAXDB = -15
		float testval;
		testval = (float)((~e) & 0x7fff);
		testval = -(testval / 400.00f);
		//testval = -(testval / 341.32291666666666666666666666667);
		float vol = powf(8, testval / 20) * 32767.0f;

		if (e > 0)
			vol = -vol;

		pcmROMData[i] = (Bit16s)vol;
	}
	if (i != pcmROMSize) {
		printDebug("PCM ROM file is too short (expected %d, got %d)", pcmROMSize, i);
		rc = false;
	}
	closeFile(file);
	return rc;
}

bool Synth::initPCMList(Bit16u mapAddress, Bit16u count) {
	ControlROMPCMStruct *tps = (ControlROMPCMStruct *)&controlROMData[mapAddress];
	for (int i = 0; i < count; i++) {
		int rAddr = tps[i].pos * 0x800;
		int rLenExp = (tps[i].len & 0x70) >> 4;
		int rLen = 0x800 << rLenExp;
		bool rLoop = (tps[i].len & 0x80) != 0;
		//Bit8u rFlag = tps[i].len & 0x0F;
		Bit16u rTuneOffset = (tps[i].pitchMSB << 8) | tps[i].pitchLSB;
		// The number below is confirmed to a reasonable degree of accuracy on CM-32L
		double STANDARDFREQ = 442.0;
		float rTune = (float)(STANDARDFREQ * pow(2.0, (0x5000 - rTuneOffset) / 4056.0 - 9.0 / 12.0));
		//printDebug("%f,%d,%d", (double)pTune, tps[i].pitchCoarse, tps[i].pitchFine);
		if (rAddr + rLen > pcmROMSize) {
			printDebug("Control ROM error: Wave map entry %d points to invalid PCM address 0x%04X, length 0x%04X", i, rAddr, rLen);
			return false;
		}
		pcmWaves[i].addr = rAddr;
		pcmWaves[i].len = rLen;
		pcmWaves[i].loop = rLoop;
		pcmWaves[i].tune = rTune;
	}
	return false;
}

bool Synth::initRhythmTimbre(int timbreNum, const Bit8u *mem, unsigned int memLen) {
	if (memLen < sizeof(TimbreParam::commonParam)) {
		return false;
	}
	TimbreParam *timbre = &mt32ram.timbres[timbreNum].timbre;
	memcpy(&timbre->common, mem, 14);
	unsigned int memPos = 14;
	char drumname[11];
	memset(drumname, 0, 11);
	memcpy(drumname, timbre->common.name, 10);
	for (int t = 0; t < 4; t++) {
		if (((timbre->common.pmute >> t) & 0x1) == 0x1) {
			if (memPos + 58 >= memLen) {
				return false;
			}
			memcpy(&timbre->partial[t], mem + memPos, 58);
			memPos += 58;
		}
	}
	return true;
}

bool Synth::initRhythmTimbres(Bit16u mapAddress, Bit16u count) {
	const Bit8u *drumMap = &controlROMData[mapAddress];
	int timbreNum = 192;
	for (Bit16u i = 0; i < count * 2; i += 2) {
		Bit16u address = (drumMap[i + 1] << 8) | drumMap[i];
		/*
		// This check is nonsensical when the control ROM is the full 64KB addressable by 16-bit absolute pointers (which it is)
		if (address >= CONTROL_ROM_SIZE) {
			printDebug("Control ROM error: Timbre map entry 0x%04x points to invalid timbre address 0x%04x", i, address);
			return false;
		}
		*/
		if (!initRhythmTimbre(timbreNum++, &controlROMData[address], CONTROL_ROM_SIZE - address)) {
			printDebug("Control ROM error: Timbre map entry 0x%04x points to invalid timbre 0x%04x", i, address);
			return false;
		}
	}
	return true;
}

bool Synth::initTimbres(Bit16u mapAddress, Bit16u offset, int startTimbre) {
	for (Bit16u i = mapAddress; i < mapAddress + 0x80; i += 2) {
		Bit16u address = (controlROMData[i + 1] << 8) | controlROMData[i];
		if (address + sizeof(TimbreParam) > CONTROL_ROM_SIZE) {
			printDebug("Control ROM error: Timbre map entry 0x%04x points to invalid timbre address 0x%04x", i, address);
			return false;
		}
		address = address + offset;
		TimbreParam *timbre = &mt32ram.timbres[startTimbre++].timbre;
		memcpy(timbre, &controlROMData[address], sizeof(TimbreParam));
	}
	return true;
}

bool Synth::open(SynthProperties &useProp) {
	if (isOpen)
		return false;

	myProp = useProp;
	if (useProp.baseDir != NULL) {
		myProp.baseDir = new char[strlen(useProp.baseDir) + 1];
		strcpy(myProp.baseDir, useProp.baseDir);
	}

	// This is to help detect bugs
	memset(&mt32ram, '?', sizeof(mt32ram));

	printDebug("Loading Control ROM");
	if (!loadControlROM("CM32L_CONTROL.ROM")) {
		if (!loadControlROM("MT32_CONTROL.ROM")) {
			printDebug("Init Error - Missing or invalid MT32_CONTROL.ROM");
			report(ReportType_errorControlROM, NULL);
			return false;
		}
	}

	// 512KB PCM ROM for MT-32, etc.
	// 1MB PCM ROM for CM-32L, LAPC-I, CM-64, CM-500
	// Note that the size below is given in samples (16-bit), not bytes
	pcmROMSize = controlROMMap->pcmCount == 256 ? 512 * 1024 : 256 * 1024;
	pcmROMData = new Bit16s[pcmROMSize];

	printDebug("Loading PCM ROM");
	if (!loadPCMROM("CM32L_PCM.ROM")) {
		if (!loadPCMROM("MT32_PCM.ROM")) {
			printDebug("Init Error - Missing MT32_PCM.ROM");
			report(ReportType_errorPCMROM, NULL);
			return false;
		}
	}

	printDebug("Initializing Timbre Bank A");
	if (!initTimbres(controlROMMap->timbreAMap, controlROMMap->timbreAOffset, 0)) {
		return false;
	}

	printDebug("Initializing Timbre Bank B");
	if (!initTimbres(controlROMMap->timbreBMap, controlROMMap->timbreBOffset, 64)) {
		return false;
	}

	printDebug("Initializing Timbre Bank R");
	if (!initRhythmTimbres(controlROMMap->timbreRMap, controlROMMap->timbreRCount)) {
		return false;
	}

	printDebug("Initializing Timbre Bank M");
	// CM-64 seems to initialize all bytes in this bank to 0.
	memset(&mt32ram.timbres[128], 0, sizeof (mt32ram.timbres[128]) * 64);

	partialManager = new PartialManager(this);

	pcmWaves = new PCMWaveEntry[controlROMMap->pcmCount];

	printDebug("Initializing PCM List");
	initPCMList(controlROMMap->pcmTable, controlROMMap->pcmCount);

	printDebug("Initializing Rhythm Temp");
	memcpy(mt32ram.rhythmSettings, &controlROMData[controlROMMap->rhythmSettings], controlROMMap->rhythmSettingsCount * 4);

	printDebug("Initializing Patches");
	for (Bit8u i = 0; i < 128; i++) {
		PatchParam *patch = &mt32ram.patches[i];
		patch->timbreGroup = i / 64;
		patch->timbreNum = i % 64;
		patch->keyShift = 24;
		patch->fineTune = 50;
		patch->benderRange = 12;
		patch->assignMode = 0;
		patch->reverbSwitch = 1;
		patch->dummy = 0;
	}

	printDebug("Initializing System");
	// The MT-32 manual claims that "Standard pitch" is 442Hz.
	mt32ram.system.masterTune = 0x40; // Confirmed on CM-64 as 0x4A, but SCUMM games use 0x40 and we don't want to initialize twice
	mt32ram.system.reverbMode = 0; // Confirmed
	mt32ram.system.reverbTime = 5; // Confirmed
	mt32ram.system.reverbLevel = 3; // Confirmed
	memcpy(mt32ram.system.reserveSettings, &controlROMData[controlROMMap->reserveSettings], 9); // Confirmed
	for (Bit8u i = 0; i < 9; i++) {
		// This is the default: {1, 2, 3, 4, 5, 6, 7, 8, 9}
		// An alternative configuration can be selected by holding "Master Volume"
		// and pressing "PART button 1" on the real MT-32's frontpanel.
		// The channel assignment is then {0, 1, 2, 3, 4, 5, 6, 7, 9}
		mt32ram.system.chanAssign[i] = i + 1;
	}
	mt32ram.system.masterVol = 100; // Confirmed
	if (!refreshSystem())
		return false;

	for (int i = 0; i < 8; i++) {
		mt32ram.patchSettings[i].outlevel = 80;
		mt32ram.patchSettings[i].panpot = controlROMData[controlROMMap->panSettings + i];
		memset(mt32ram.patchSettings[i].dummyv, 0, sizeof(mt32ram.patchSettings[i].dummyv));
		parts[i] = new Part(this, i);
		parts[i]->setProgram(controlROMData[controlROMMap->programSettings + i]);
	}
	parts[8] = new RhythmPart(this, 8);

	// For resetting mt32 mid-execution
	mt32default = mt32ram;

	iirFilter = &iir_filter_normal;

#ifdef MT32EMU_HAVE_X86
	bool availableSSE = DetectSIMD();
	bool available3DNow = Detect3DNow();

	if (availableSSE)
		report(ReportType_availableSSE, NULL);
	if (available3DNow)
		report(ReportType_available3DNow, NULL);

	if (available3DNow) {
		printDebug("Detected and using SIMD (AMD 3DNow) extensions");
		iirFilter = &iir_filter_3dnow;
		report(ReportType_using3DNow, NULL);
	} else if (availableSSE) {
		printDebug("Detected and using SIMD (Intel SSE) extensions");
		iirFilter = &iir_filter_sse;
		report(ReportType_usingSSE, NULL);
	}
#endif

	isOpen = true;
	isEnabled = false;

	printDebug("*** Initialisation complete ***");
	return true;
}

void Synth::close(void) {
	if (!isOpen)
		return;

	tables.freeNotes();
	if (partialManager != NULL) {
		delete partialManager;
		partialManager = NULL;
	}

	if (reverbModel != NULL) {
		delete reverbModel;
		reverbModel = NULL;
	}

	for (int i = 0; i < 9; i++) {
		if (parts[i] != NULL) {
			delete parts[i];
			parts[i] = NULL;
		}
	}
	if (myProp.baseDir != NULL) {
		delete myProp.baseDir;
		myProp.baseDir = NULL;
	}

	delete[] pcmWaves;
	delete[] pcmROMData;
	isOpen = false;
}

void Synth::playMsg(Bit32u msg) {
	// FIXME: Implement active sensing
	unsigned char code     = (unsigned char)((msg & 0x0000F0) >> 4);
	unsigned char chan     = (unsigned char) (msg & 0x00000F);
	unsigned char note     = (unsigned char)((msg & 0x00FF00) >> 8);
	unsigned char velocity = (unsigned char)((msg & 0xFF0000) >> 16);
	isEnabled = true;

	//printDebug("Playing chan %d, code 0x%01x note: 0x%02x", chan, code, note);

	signed char part = chantable[chan];
	if (part < 0 || part > 8) {
		printDebug("Play msg on unreg chan %d (%d): code=0x%01x, vel=%d", chan, part, code, velocity);
		return;
	}
	playMsgOnPart(part, code, note, velocity);
}

void Synth::playMsgOnPart(unsigned char part, unsigned char code, unsigned char note, unsigned char velocity) {
	Bit32u bend;

	//printDebug("Synth::playMsg(0x%02x)",msg);
	switch (code) {
	case 0x8:
		//printDebug("Note OFF - Part %d", part);
		// The MT-32 ignores velocity for note off
		parts[part]->stopNote(note);
		break;
	case 0x9:
		//printDebug("Note ON - Part %d, Note %d Vel %d", part, note, velocity);
		if (velocity == 0) {
			// MIDI defines note-on with velocity 0 as being the same as note-off with velocity 40
			parts[part]->stopNote(note);
		} else {
			parts[part]->playNote(note, velocity);
		}
		break;
	case 0xB: // Control change
		switch (note) {
		case 0x01:  // Modulation
			//printDebug("Modulation: %d", velocity);
			parts[part]->setModulation(velocity);
			break;
		case 0x07:  // Set volume
			//printDebug("Volume set: %d", velocity);
			parts[part]->setVolume(velocity);
			break;
		case 0x0A:  // Pan
			//printDebug("Pan set: %d", velocity);
			parts[part]->setPan(velocity);
			break;
		case 0x0B:
			//printDebug("Expression set: %d", velocity);
			parts[part]->setExpression(velocity);
			break;
		case 0x40: // Hold (sustain) pedal
			//printDebug("Hold pedal set: %d", velocity);
			parts[part]->setHoldPedal(velocity>=64);
			break;

		case 0x79: // Reset all controllers
			//printDebug("Reset all controllers");
			//FIXME: Check for accuracy against real thing
			parts[part]->setVolume(100);
			parts[part]->setExpression(127);
			parts[part]->setPan(64);
			parts[part]->setBend(0x2000);
			parts[part]->setHoldPedal(false);
			break;

		case 0x7B: // All notes off
			//printDebug("All notes off");
			parts[part]->allNotesOff();
			break;

		default:
			printDebug("Unknown MIDI Control code: 0x%02x - vel 0x%02x", note, velocity);
			break;
		}

		break;
	case 0xC: // Program change
		//printDebug("Program change %01x", note);
		parts[part]->setProgram(note);
		break;
	case 0xE: // Pitch bender
		bend = (velocity << 7) | (note);
		//printDebug("Pitch bender %02x", bend);
		parts[part]->setBend(bend);
		break;
	default:
		printDebug("Unknown Midi code: 0x%01x - %02x - %02x", code, note, velocity);
		break;
	}

	//midiOutShortMsg(m_out, msg);
}

void Synth::playSysex(const Bit8u *sysex, Bit32u len) {
	if (len < 2) {
		printDebug("playSysex: Message is too short for sysex (%d bytes)", len);
	}
	if (sysex[0] != 0xF0) {
		printDebug("playSysex: Message lacks start-of-sysex (0xF0)");
		return;
	}
	// Due to some programs (e.g. Java) sending buffers with junk at the end, we have to go through and find the end marker rather than relying on len.
	Bit32u endPos;
	for (endPos = 1; endPos < len; endPos++)
	{
		if (sysex[endPos] == 0xF7)
			break;
	}
	if (endPos == len) {
		printDebug("playSysex: Message lacks end-of-sysex (0xf7)");
		return;
	}
	playSysexWithoutFraming(sysex + 1, endPos - 1);
}

void Synth::playSysexWithoutFraming(const Bit8u *sysex, Bit32u len) {
	if (len < 4) {
		printDebug("playSysexWithoutFraming: Message is too short (%d bytes)!", len);
		return;
	}
	if (sysex[0] != SYSEX_MANUFACTURER_ROLAND) {
		printDebug("playSysexWithoutFraming: Header not intended for this device manufacturer: %02x %02x %02x %02x", (int)sysex[0], (int)sysex[1], (int)sysex[2], (int)sysex[3]);
		return;
	}
	if (sysex[2] == SYSEX_MDL_D50) {
		printDebug("playSysexWithoutFraming: Header is intended for model D-50 (not yet supported): %02x %02x %02x %02x", (int)sysex[0], (int)sysex[1], (int)sysex[2], (int)sysex[3]);
		return;
	}
	else if (sysex[2] != SYSEX_MDL_MT32) {
		printDebug("playSysexWithoutFraming: Header not intended for model MT-32: %02x %02x %02x %02x", (int)sysex[0], (int)sysex[1], (int)sysex[2], (int)sysex[3]);
		return;
	}
	playSysexWithoutHeader(sysex[1], sysex[3], sysex + 4, len - 4);
}

void Synth::playSysexWithoutHeader(unsigned char device, unsigned char command, const Bit8u *sysex, Bit32u len) {
	if (device > 0x10) {
		// We have device ID 0x10 (default, but changeable, on real MT-32), < 0x10 is for channels
		printDebug("playSysexWithoutHeader: Message is not intended for this device ID (provided: %02x, expected: 0x10 or channel)", (int)device);
		return;
	}
	if (len < 4) {
		printDebug("playSysexWithoutHeader: Message is too short (%d bytes)!", len);
		return;
	}
	unsigned char checksum = calcSysexChecksum(sysex, len - 1, 0);
	if (checksum != sysex[len - 1]) {
		printDebug("playSysexWithoutHeader: Message checksum is incorrect (provided: %02x, expected: %02x)!", sysex[len - 1], checksum);
		return;
	}
	len -= 1; // Exclude checksum
	switch (command) {
	case SYSEX_CMD_DT1:
		writeSysex(device, sysex, len);
		break;
	case SYSEX_CMD_RQ1:
		readSysex(device, sysex, len);
		break;
	default:
		printDebug("playSysexWithoutFraming: Unsupported command %02x", command);
		return;
	}
}

void Synth::readSysex(unsigned char /*device*/, const Bit8u * /*sysex*/, Bit32u /*len*/) {
}

const MemoryRegion memoryRegions[8] = {
	{MR_PatchTemp,  MT32EMU_MEMADDR(0x030000), sizeof(MemParams::PatchTemp), 9},
	{MR_RhythmTemp, MT32EMU_MEMADDR(0x030110), sizeof(MemParams::RhythmTemp), 85},
	{MR_TimbreTemp, MT32EMU_MEMADDR(0x040000), sizeof(TimbreParam), 8},
	{MR_Patches,    MT32EMU_MEMADDR(0x050000), sizeof(PatchParam), 128},
	{MR_Timbres,    MT32EMU_MEMADDR(0x080000), sizeof(MemParams::PaddedTimbre), 64 + 64 + 64 + 64},
	{MR_System,     MT32EMU_MEMADDR(0x100000), sizeof(MemParams::SystemArea), 1},
	{MR_Display,    MT32EMU_MEMADDR(0x200000), MAX_SYSEX_SIZE - 1, 1},
	{MR_Reset,      MT32EMU_MEMADDR(0x7F0000), 0x3FFF, 1}
};

const int NUM_REGIONS = sizeof(memoryRegions) / sizeof(MemoryRegion);

void Synth::writeSysex(unsigned char device, const Bit8u *sysex, Bit32u len) {
	Bit32u addr = (sysex[0] << 16) | (sysex[1] << 8) | (sysex[2]);
	addr = MT32EMU_MEMADDR(addr);
	sysex += 3;
	len -= 3;
	//printDebug("Sysex addr: 0x%06x", MT32EMU_SYSEXMEMADDR(addr));
	// NOTE: Please keep both lower and upper bounds in each check, for ease of reading

	// Process channel-specific sysex by converting it to device-global
	if (device < 0x10) {
		printDebug("WRITE-CHANNEL: Channel %d temp area 0x%06x", device, MT32EMU_SYSEXMEMADDR(addr));
		if (/*addr >= MT32EMU_MEMADDR(0x000000) && */addr < MT32EMU_MEMADDR(0x010000)) {
			int offset;
			if (chantable[device] == -1) {
				printDebug(" (Channel not mapped to a partial... 0 offset)");
				offset = 0;
			} else if (chantable[device] == 8) {
				printDebug(" (Channel mapped to rhythm... 0 offset)");
				offset = 0;
			} else {
				offset = chantable[device] * sizeof(MemParams::PatchTemp);
				printDebug(" (Setting extra offset to %d)", offset);
			}
			addr += MT32EMU_MEMADDR(0x030000) + offset;
		} else if (/*addr >= 0x010000 && */ addr < MT32EMU_MEMADDR(0x020000)) {
			addr += MT32EMU_MEMADDR(0x030110) - MT32EMU_MEMADDR(0x010000);
		} else if (/*addr >= 0x020000 && */ addr < MT32EMU_MEMADDR(0x030000)) {
			int offset;
			if (chantable[device] == -1) {
				printDebug(" (Channel not mapped to a partial... 0 offset)");
				offset = 0;
			} else if (chantable[device] == 8) {
				printDebug(" (Channel mapped to rhythm... 0 offset)");
				offset = 0;
			} else {
				offset = chantable[device] * sizeof(TimbreParam);
				printDebug(" (Setting extra offset to %d)", offset);
			}
			addr += MT32EMU_MEMADDR(0x040000) - MT32EMU_MEMADDR(0x020000) + offset;
		} else {
			printDebug("PlaySysexWithoutHeader: Invalid channel %d address 0x%06x", device, MT32EMU_SYSEXMEMADDR(addr));
			return;
		}
	}

	// Process device-global sysex (possibly converted from channel-specific sysex above)
	for (;;) {
		// Find the appropriate memory region
		int regionNum;
		const MemoryRegion *region = NULL; // Initialized to please compiler
		for (regionNum = 0; regionNum < NUM_REGIONS; regionNum++) {
			region = &memoryRegions[regionNum];
			if (region->contains(addr)) {
				writeMemoryRegion(region, addr, region->getClampedLen(addr, len), sysex);
				break;
			}
		}
		if (regionNum == NUM_REGIONS) {
			printDebug("Sysex write to unrecognised address %06x, len %d", MT32EMU_SYSEXMEMADDR(addr), len);
			break;
		}
		Bit32u next = region->next(addr, len);
		if (next == 0) {
			break;
		}
		addr += next;
		sysex += next;
		len -= next;
	}
}

void Synth::readMemory(Bit32u addr, Bit32u len, Bit8u *data) {
	int regionNum;
	const MemoryRegion *region = NULL;
	for (regionNum = 0; regionNum < NUM_REGIONS; regionNum++) {
		region = &memoryRegions[regionNum];
		if (region->contains(addr)) {
			readMemoryRegion(region, addr, len, data);
			break;
		}
	}
}

void Synth::readMemoryRegion(const MemoryRegion *region, Bit32u addr, Bit32u len, Bit8u *data) {
	unsigned int first = region->firstTouched(addr);
	//unsigned int last = region->lastTouched(addr, len);
	unsigned int off = region->firstTouchedOffset(addr);
	len = region->getClampedLen(addr, len);

	unsigned int m;

	switch (region->type) {
	case MR_PatchTemp:
		for (m = 0; m < len; m++)
			data[m] = ((Bit8u *)&mt32ram.patchSettings[first])[off + m];
		break;
	case MR_RhythmTemp:
		for (m = 0; m < len; m++)
			data[m] = ((Bit8u *)&mt32ram.rhythmSettings[first])[off + m];
		break;
	case MR_TimbreTemp:
		for (m = 0; m < len; m++)
			data[m] = ((Bit8u *)&mt32ram.timbreSettings[first])[off + m];
		break;
	case MR_Patches:
		for (m = 0; m < len; m++)
			data[m] = ((Bit8u *)&mt32ram.patches[first])[off + m];
		break;
	case MR_Timbres:
		for (m = 0; m < len; m++)
			data[m] = ((Bit8u *)&mt32ram.timbres[first])[off + m];
		break;
	case MR_System:
		for (m = 0; m < len; m++)
			data[m] = ((Bit8u *)&mt32ram.system)[m + off];
		break;
	default:
		for (m = 0; m < len; m += 2) {
			data[m] = 0xff;
			if (m + 1 < len) {
				data[m+1] = (Bit8u)region->type;
			}
		}
		// TODO: Don't care about the others ATM
		break;
	}

}

void Synth::writeMemoryRegion(const MemoryRegion *region, Bit32u addr, Bit32u len, const Bit8u *data) {
	unsigned int first = region->firstTouched(addr);
	unsigned int last = region->lastTouched(addr, len);
	unsigned int off = region->firstTouchedOffset(addr);
	switch (region->type) {
	case MR_PatchTemp:
		for (unsigned int m = 0; m < len; m++) {
			((Bit8u *)&mt32ram.patchSettings[first])[off + m] = data[m];
		}
		//printDebug("Patch temp: Patch %d, offset %x, len %d", off/16, off % 16, len);

		for (unsigned int i = first; i <= last; i++) {
			int absTimbreNum = mt32ram.patchSettings[i].patch.timbreGroup * 64 + mt32ram.patchSettings[i].patch.timbreNum;
			char timbreName[11];
			memcpy(timbreName, mt32ram.timbres[absTimbreNum].timbre.common.name, 10);
			timbreName[10] = 0;
			printDebug("WRITE-PARTPATCH (%d-%d@%d..%d): %d; timbre=%d (%s), outlevel=%d", first, last, off, off + len, i, absTimbreNum, timbreName, mt32ram.patchSettings[i].outlevel);
			if (parts[i] != NULL) {
				if (i != 8) {
					// Note: Confirmed on CM-64 that we definitely *should* update the timbre here,
					// but only in the case that the sysex actually writes to those values
					if (i == first && off > 2) {
						printDebug(" (Not updating timbre, since those values weren't touched)");
					} else {
						parts[i]->setTimbre(&mt32ram.timbres[parts[i]->getAbsTimbreNum()].timbre);
					}
				}
				parts[i]->refresh();
			}
		}
		break;
	case MR_RhythmTemp:
		for (unsigned int m = 0; m < len; m++)
			((Bit8u *)&mt32ram.rhythmSettings[first])[off + m] = data[m];
		for (unsigned int i = first; i <= last; i++) {
			int timbreNum = mt32ram.rhythmSettings[i].timbre;
			char timbreName[11];
			if (timbreNum < 94) {
				memcpy(timbreName, mt32ram.timbres[128 + timbreNum].timbre.common.name, 10);
				timbreName[10] = 0;
			} else {
				strcpy(timbreName, "[None]");
			}
			printDebug("WRITE-RHYTHM (%d-%d@%d..%d): %d; level=%02x, panpot=%02x, reverb=%02x, timbre=%d (%s)", first, last, off, off + len, i, mt32ram.rhythmSettings[i].outlevel, mt32ram.rhythmSettings[i].panpot, mt32ram.rhythmSettings[i].reverbSwitch, mt32ram.rhythmSettings[i].timbre, timbreName);
		}
		if (parts[8] != NULL) {
			parts[8]->refresh();
		}
		break;
	case MR_TimbreTemp:
		for (unsigned int m = 0; m < len; m++)
			((Bit8u *)&mt32ram.timbreSettings[first])[off + m] = data[m];
		for (unsigned int i = first; i <= last; i++) {
			char instrumentName[11];
			memcpy(instrumentName, mt32ram.timbreSettings[i].common.name, 10);
			instrumentName[10] = 0;
			printDebug("WRITE-PARTTIMBRE (%d-%d@%d..%d): timbre=%d (%s)", first, last, off, off + len, i, instrumentName);
			if (parts[i] != NULL) {
				parts[i]->refresh();
			}
		}
		break;
	case MR_Patches:
		for (unsigned int m = 0; m < len; m++)
			((Bit8u *)&mt32ram.patches[first])[off + m] = data[m];
		for (unsigned int i = first; i <= last; i++) {
			PatchParam *patch = &mt32ram.patches[i];
			int patchAbsTimbreNum = patch->timbreGroup * 64 + patch->timbreNum;
			char instrumentName[11];
			memcpy(instrumentName, mt32ram.timbres[patchAbsTimbreNum].timbre.common.name, 10);
			instrumentName[10] = 0;
			Bit8u *n = (Bit8u *)patch;
			printDebug("WRITE-PATCH (%d-%d@%d..%d): %d; timbre=%d (%s) %02X%02X%02X%02X%02X%02X%02X%02X", first, last, off, off + len, i, patchAbsTimbreNum, instrumentName, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7]);
			// FIXME:KG: The below is definitely dodgy. We just guess that this is the patch that the part was using
			// based on a timbre match (but many patches could have the same timbre!)
			// If this refresh is really correct, we should store the patch number in use by each part.
			/*
			for (int part = 0; part < 8; part++) {
				if (parts[part] != NULL) {
					int partPatchAbsTimbreNum = mt32ram.patchSettings[part].patch.timbreGroup * 64 + mt32ram.patchSettings[part].patch.timbreNum;
					if (parts[part]->getAbsTimbreNum() == patchAbsTimbreNum) {
						parts[part]->setPatch(patch);
						parts[part]->RefreshPatch();
					}
				}
			}
			*/
		}
		break;
	case MR_Timbres:
		// Timbres
		first += 128;
		last += 128;
		for (unsigned int m = 0; m < len; m++)
			((Bit8u *)&mt32ram.timbres[first])[off + m] = data[m];
		for (unsigned int i = first; i <= last; i++) {
			char instrumentName[11];
			memcpy(instrumentName, mt32ram.timbres[i].timbre.common.name, 10);
			instrumentName[10] = 0;
			printDebug("WRITE-TIMBRE (%d-%d@%d..%d): %d; name=\"%s\"", first, last, off, off + len, i, instrumentName);
			// FIXME:KG: Not sure if the stuff below should be done (for rhythm and/or parts)...
			// Does the real MT-32 automatically do this?
			for (unsigned int part = 0; part < 9; part++) {
				if (parts[part] != NULL) {
					parts[part]->refreshTimbre(i);
				}
			}
		}
		break;
	case MR_System:
		for (unsigned int m = 0; m < len; m++)
			((Bit8u *)&mt32ram.system)[m + off] = data[m];

		report(ReportType_devReconfig, NULL);

		printDebug("WRITE-SYSTEM:");
		refreshSystem();
		break;
	case MR_Display:
		char buf[MAX_SYSEX_SIZE];
		memcpy(&buf, &data[0], len);
		buf[len] = 0;
		printDebug("WRITE-LCD: %s", buf);
		report(ReportType_lcdMessage, buf);
		break;
	case MR_Reset:
		printDebug("RESET");
		report(ReportType_devReset, NULL);
		partialManager->deactivateAll();
		mt32ram = mt32default;
		for (int i = 0; i < 9; i++) {
			parts[i]->refresh();
		}
		isEnabled = false;
		break;
	}
}

bool Synth::refreshSystem() {
	memset(chantable, -1, sizeof(chantable));

	for (unsigned int i = 0; i < 9; i++) {
		//LOG(LOG_MISC|LOG_ERROR,"Part %d set to MIDI channel %d",i,mt32ram.system.chanAssign[i]);
		if (mt32ram.system.chanAssign[i] == 16 && parts[i] != NULL) {
			parts[i]->allSoundOff();
		} else {
			chantable[(int)mt32ram.system.chanAssign[i]] = (char)i;
		}
	}
	//FIXME:KG: This is just an educated guess.
	// The LAPC-I documentation claims a range of 427.5Hz-452.6Hz (similar to what we have here)
	// The MT-32 documentation claims a range of 432.1Hz-457.6Hz
	masterTune = 440.0f * powf(2.0f, (mt32ram.system.masterTune - 64.0f) / (128.0f * 12.0f));
	printDebug(" Master Tune: %f", (double)masterTune);
	printDebug(" Reverb: mode=%d, time=%d, level=%d", mt32ram.system.reverbMode, mt32ram.system.reverbTime, mt32ram.system.reverbLevel);
	report(ReportType_newReverbMode,  &mt32ram.system.reverbMode);
	report(ReportType_newReverbTime,  &mt32ram.system.reverbTime);
	report(ReportType_newReverbLevel, &mt32ram.system.reverbLevel);

	if (myProp.useDefaultReverb) {
		initReverb(mt32ram.system.reverbMode, mt32ram.system.reverbTime, mt32ram.system.reverbLevel);
	} else {
		initReverb(myProp.reverbType, myProp.reverbTime, mt32ram.system.reverbLevel);
	}

	Bit8u *rset = mt32ram.system.reserveSettings;
	printDebug(" Partial reserve: 1=%02d 2=%02d 3=%02d 4=%02d 5=%02d 6=%02d 7=%02d 8=%02d Rhythm=%02d", rset[0], rset[1], rset[2], rset[3], rset[4], rset[5], rset[6], rset[7], rset[8]);
	int pr = partialManager->setReserve(rset);
	if (pr != 32)
		printDebug(" (Partial Reserve Table with less than 32 partials reserved!)");
	rset = mt32ram.system.chanAssign;
	printDebug(" Part assign:     1=%02d 2=%02d 3=%02d 4=%02d 5=%02d 6=%02d 7=%02d 8=%02d Rhythm=%02d", rset[0], rset[1], rset[2], rset[3], rset[4], rset[5], rset[6], rset[7], rset[8]);
	printDebug(" Master volume: %d", mt32ram.system.masterVol);
	masterVolume = (Bit16u)(mt32ram.system.masterVol * 32767 / 100);
	if (!tables.init(this, pcmWaves, (float)myProp.sampleRate, masterTune)) {
		report(ReportType_errorSampleRate, NULL);
		return false;
	}
	return true;
}

bool Synth::dumpTimbre(File *file, const TimbreParam *timbre, Bit32u address) {
	// Sysex header
	if (!file->writeBit8u(0xF0))
		return false;
	if (!file->writeBit8u(0x41))
		return false;
	if (!file->writeBit8u(0x10))
		return false;
	if (!file->writeBit8u(0x16))
		return false;
	if (!file->writeBit8u(0x12))
		return false;

	char lsb = (char)(address & 0x7f);
	char isb = (char)((address >> 7) & 0x7f);
	char msb = (char)(((address >> 14) & 0x7f) | 0x08);

	//Address
	if (!file->writeBit8u(msb))
		return false;
	if (!file->writeBit8u(isb))
		return false;
	if (!file->writeBit8u(lsb))
		return false;

	//Data
	if (file->write(timbre, 246) != 246)
		return false;

	//Checksum
	unsigned char checksum = calcSysexChecksum((const Bit8u *)timbre, 246, msb + isb + lsb);
	if (!file->writeBit8u(checksum))
		return false;

	//End of sysex
	if (!file->writeBit8u(0xF7))
		return false;
	return true;
}

int Synth::dumpTimbres(const char *filename, int start, int len) {
	File *file = openFile(filename, File::OpenMode_write);
	if (file == NULL)
		return -1;

	for (int timbreNum = start; timbreNum < start + len; timbreNum++) {
		int useaddr = (timbreNum - start) * 256;
		TimbreParam *timbre = &mt32ram.timbres[timbreNum].timbre;
		if (!dumpTimbre(file, timbre, useaddr))
			break;
	}
	closeFile(file);
	return 0;
}

void ProduceOutput1(Bit16s *useBuf, Bit16s *stream, Bit32u len, Bit16s volume) {
#if MT32EMU_USE_MMX > 2
	//FIXME:KG: This appears to introduce crackle
	int donelen = i386_produceOutput1(useBuf, stream, len, volume);
	len -= donelen;
	stream += donelen * 2;
	useBuf += donelen * 2;
#endif
	int end = len * 2;
	while (end--) {
		*stream = *stream + (Bit16s)(((Bit32s)*useBuf++ * (Bit32s)volume)>>15);
		stream++;
	}
}

void Synth::render(Bit16s *stream, Bit32u len) {
	memset(stream, 0, len * sizeof (Bit16s) * 2);
	if (!isEnabled)
		return;
	while (len > 0) {
		Bit32u thisLen = len > MAX_SAMPLE_OUTPUT ? MAX_SAMPLE_OUTPUT : len;
		doRender(stream, thisLen);
		len -= thisLen;
		stream += 2 * thisLen;
	}
}

void Synth::doRender(Bit16s *stream, Bit32u len) {
	partialManager->ageAll();

	if (myProp.useReverb) {
		for (unsigned int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
			if (partialManager->shouldReverb(i)) {
				if (partialManager->produceOutput(i, &tmpBuffer[0], len)) {
					ProduceOutput1(&tmpBuffer[0], stream, len, masterVolume);
				}
			}
		}
		Bit32u m = 0;
		for (unsigned int i = 0; i < len; i++) {
			sndbufl[i] = (float)stream[m] / 32767.0f;
			m++;
			sndbufr[i] = (float)stream[m] / 32767.0f;
			m++;
		}
		reverbModel->processreplace(sndbufl, sndbufr, outbufl, outbufr, len, 1);
		m=0;
		for (unsigned int i = 0; i < len; i++) {
			stream[m] = (Bit16s)(outbufl[i] * 32767.0f);
			m++;
			stream[m] = (Bit16s)(outbufr[i] * 32767.0f);
			m++;
		}
		for (unsigned int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
			if (!partialManager->shouldReverb(i)) {
				if (partialManager->produceOutput(i, &tmpBuffer[0], len)) {
					ProduceOutput1(&tmpBuffer[0], stream, len, masterVolume);
				}
			}
		}
	} else {
		for (unsigned int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
			if (partialManager->produceOutput(i, &tmpBuffer[0], len))
				ProduceOutput1(&tmpBuffer[0], stream, len, masterVolume);
		}
	}

	partialManager->clearAlreadyOutputed();

#if MT32EMU_MONITOR_PARTIALS == 1
	samplepos += len;
	if (samplepos > myProp.SampleRate * 5) {
		samplepos = 0;
		int partialUsage[9];
		partialManager->GetPerPartPartialUsage(partialUsage);
		printDebug("1:%02d 2:%02d 3:%02d 4:%02d 5:%02d 6:%02d 7:%02d 8:%02d", partialUsage[0], partialUsage[1], partialUsage[2], partialUsage[3], partialUsage[4], partialUsage[5], partialUsage[6], partialUsage[7]);
		printDebug("Rhythm: %02d  TOTAL: %02d", partialUsage[8], MT32EMU_MAX_PARTIALS - partialManager->GetFreePartialCount());
	}
#endif
}

const Partial *Synth::getPartial(unsigned int partialNum) const {
	return partialManager->getPartial(partialNum);
}

const Part *Synth::getPart(unsigned int partNum) const {
	if (partNum > 8)
		return NULL;
	return parts[partNum];
}

}
