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

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "mt32emu.h"

namespace MT32Emu {

const int MAX_SYSEX_SIZE = 512;

float iir_filter_normal(float input,float *hist1_ptr, float *coef_ptr, int revLevel) {
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

	output *= ResonInv[revLevel];

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
	memset(noteLookups, 0, sizeof(noteLookups));
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
	if (reverbModel != NULL)
		delete reverbModel;
	reverbModel = new revmodel();

	switch(newRevMode) {
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
	if (myProp.openFile != NULL) {
		return myProp.openFile(myProp.userData, filename, mode);
	}
	char pathBuf[2048];
	if (myProp.baseDir != NULL) {
		strcpy(&pathBuf[0], myProp.baseDir);
		strcat(&pathBuf[0], filename);
		filename = pathBuf;
	}
	ANSIFile *file = new ANSIFile();
	if (!file->open(filename, mode)) {
		delete file;
		return NULL;
	}
	return file;
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
	bool rc = (file->read(controlROMData, sizeof(controlROMData)) == sizeof(controlROMData));

	closeFile(file);
	return rc;
}

bool Synth::loadPCMROM(const char *filename) {
	File *file = openFile(filename, File::OpenMode_read); // ROM File
	if (file == NULL) {
		return false;
	}
	bool rc = true;
	for (int i = 0; ; i++) {
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
				printDebug("ROM file has an odd number of bytes! Ignoring last");
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

		// File is encoded in dB, convert to PCM
		// MINDB = -96
		// MAXDB = -15
		float testval;
		testval = (float)((~e) & 0x7fff);
		testval = -(testval / 400.00f);
		//testval = -(testval / 341.32291666666666666666666666667);
		float vol = powf(8, testval / 20) * 32767.0f;

		if (e > 0)
			vol = -vol;

		romfile[i] = (Bit16s)vol;

	}
	closeFile(file);
	return rc;
}

struct ControlROMPCMStruct
{
	Bit8u pos;
	Bit8u len;
	Bit8u pitchLSB;
	Bit8u pitchMSB;
};

void Synth::initPCMList() {
	ControlROMPCMStruct *tps = (ControlROMPCMStruct *)&controlROMData[0x3000];
	for (int i = 0; i < 128; i++) {
		int rAddr = tps[i].pos * 0x800;
		int rLenExp = (tps[i].len & 0x70) >> 4;
		int rLen = 0x800 << rLenExp;
		bool rLoop = (tps[i].len & 0x80) != 0;
		//Bit8u rFlag = tps[i].len & 0x0F;
		Bit16u rTuneOffset = (tps[i].pitchMSB << 8) | tps[i].pitchLSB;
		//FIXME:KG: Pick a number, any number. The one below sounded best to me in listening tests, but needs to be confirmed.
		double STANDARDFREQ = 432.1;
		float rTune = (float)(STANDARDFREQ * pow(2.0, (0x5000 - rTuneOffset) / 4096.0 - 9.0 / 12.0));
		//printDebug("%f,%d,%d", pTune, tps[i].pitchCoarse, tps[i].pitchFine);
		PCMList[i].addr = rAddr;
		PCMList[i].len = rLen;
		PCMList[i].loop = rLoop;
		PCMList[i].tune = rTune;
	}
}

void Synth::initRhythmTimbre(int timbreNum, const Bit8u *mem) {
	TimbreParam *timbre = &mt32ram.timbres[timbreNum].timbre;
	memcpy(&timbre->common, mem, 14);
	mem += 14;
	char drumname[11];
	strncpy(drumname, timbre->common.name, 10);
	drumname[10] = 0;
	for (int t = 0; t < 4; t++) {
		if (((timbre->common.pmute >> t) & 0x1) == 0x1) {
			memcpy(&timbre->partial[t], mem, 58);
			mem += 58;
		}
	}
}

void Synth::initRhythmTimbres() {
	const Bit8u *drumMap = &controlROMData[0x3200];
	int timbreNum = 192;
	for (Bit16u i = 0; i < 60; i += 2) {
		Bit16u address = (drumMap[i + 1] << 8) | drumMap[i];
		initRhythmTimbre(timbreNum++, &controlROMData[address]);
	}
}

void Synth::initTimbres(Bit16u mapAddress, int startTimbre) {
	for (Bit16u i = mapAddress; i < mapAddress + 0x80; i += 2) {
		Bit16u address = (controlROMData[i + 1] << 8) | controlROMData[i];
		address = address + (mapAddress - 0x8000);
		TimbreParam *timbre = &mt32ram.timbres[startTimbre++].timbre;
		memcpy(timbre, &controlROMData[address], sizeof(TimbreParam));
	}
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
	for (int i = 128; i < 192; i++) {
		// If something sets a patch to point to an uninitialised memory timbre, don't play anything
		mt32ram.timbres[i].timbre.common.pmute = 0;
	}

	printDebug("Loading Control ROM");
	if (!loadControlROM("MT32_CONTROL.ROM")) {
		printDebug("Init Error - Missing or invalid MT32_CONTROL.ROM");
		report(ReportType_errorControlROM, &errno);
		return false;
	}

	printDebug("Loading PCM ROM");
	if (!loadPCMROM("MT32_PCM.ROM")) {
		printDebug("Init Error - Missing MT32_PCM.ROM");
		report(ReportType_errorPCMROM, &errno);
		return false;
	}

	partialManager = new PartialManager(this);

	printDebug("Initialising PCM List");
	initPCMList();

	printDebug("Initialising Timbre Bank A");
	initTimbres(0x8000, 0);

	printDebug("Initialising Timbre Bank B");
	initTimbres(0xC000, 64);

	printDebug("Initialising Timbre Bank R");
	initRhythmTimbres();

	printDebug("Initialising Rhythm Temp");
	memcpy(mt32ram.rhythmSettings, &controlROMData[0x741C], 344);

	printDebug("Initialising Patches");
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

	printDebug("Initialising System");
	//FIXME: Confirm that these are all correct
	// The MT-32 manual claims that "Standard pitch" is 442Hz.
	// I assume they mean this is the MT-32 default pitch, and not concert pitch,
	// since the latter has been internationally defined as 440Hz for decades.
	// Regardless, I'm setting the default masterTune to 440Hz
	mt32ram.system.masterTune = 0x40;
	mt32ram.system.reverbMode = 0;
	mt32ram.system.reverbTime = 5;
	mt32ram.system.reverbLevel = 3;
	memcpy(mt32ram.system.reserveSettings, &controlROMData[0x57E5], 9);
	for (Bit8u i = 0; i < 9; i++) {
		// This is the default: {1, 2, 3, 4, 5, 6, 7, 8, 9}
		// An alternative configuration can be selected by holding "Master Volume"
		// and pressing "PART button 1" on the real MT-32's frontpanel.
		// The channel assignment is then {0, 1, 2, 3, 4, 5, 6, 7, 9}
		mt32ram.system.chanAssign[i] = i + 1;
	}
	mt32ram.system.masterVol = 100;
	if (!refreshSystem())
		return false;

	for (int i = 0; i < 8; i++) {
		mt32ram.patchSettings[i].outlevel = 80;
		mt32ram.patchSettings[i].panpot = controlROMData[0x5800 + i];
		memset(mt32ram.patchSettings[i].dummyv, 0, sizeof(mt32ram.patchSettings[i].dummyv));
		parts[i] = new Part(this, i);
		parts[i]->setProgram(controlROMData[0x57EE + i]);
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

	TableInitialiser::freeNotes();
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
	isOpen = false;
}

void Synth::playMsg(Bit32u msg) {
	unsigned char code = (unsigned char)((msg & 0xf0) >> 4);
	unsigned char chan = (unsigned char)(msg & 0xf);
	unsigned char note = (unsigned char)((msg & 0xff00) >> 8);
	unsigned char velocity = (unsigned char)((msg & 0xff0000) >> 16);
	isEnabled = true;

	//printDebug("Playing chan %d, code 0x%01x note: 0x%02x", chan, code, note);

	char part = chantable[chan];
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
			//if (part!=3) return;
			//printDebug("Volume set: %d", velocity);
			parts[part]->setVolume(velocity);
			break;
		case 0x0A:  // Pan
			//printDebug("Pan set: %d", velocity);
			parts[part]->setPan(velocity);
			break;
		case 0x0B:
			//printDebug("Expression set: %d", velocity);
			parts[part]->setVolume(velocity);
			break;
		case 0x40: // Hold pedal
			//printDebug("Hold pedal set: %d", velocity);
			parts[part]->setHoldPedal(velocity>=64);
			break;

		case 0x79: // Reset all controllers
			printDebug("Reset all controllers (NYI)");
			break;

		case 0x7B: // All notes off
			//printDebug("All notes off");
			parts[part]->allStop();
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

void Synth::playSysex(const Bit8u * sysex,Bit32u len) {
	if (len < 3) {
		printDebug("playSysex: Message is too short for sysex (%d bytes)", len);
	}
	if (sysex[0] != 0xf0) {
		printDebug("playSysex: Message lacks start-of-sysex (0xf0)");
		return;
	}
	if (sysex[len - 1] != 0xf7) {
		printDebug("playSysex: Message lacks end-of-sysex (0xf7)");
		return;
	}
	playSysexWithoutFraming(sysex + 1, len - 2);
}

void Synth::playSysexWithoutFraming(const Bit8u * sysex, Bit32u len) {
	if (len < 4) {
		printDebug("playSysexWithoutFraming: Message is too short (%d bytes)!", len);
		return;
	}
	if (sysex[0] != 0x41) {
		printDebug("playSysexWithoutFraming: Header not intended for this device manufacturer: %02x %02x %02x %02x", (int)sysex[0], (int)sysex[1], (int)sysex[2], (int)sysex[3]);
		return;
	}
	if (sysex[2] == 0x14) {
		printDebug("playSysexWithoutFraming: Header is intended for Roland D-50 (not yet supported): %02x %02x %02x %02x", (int)sysex[0], (int)sysex[1], (int)sysex[2], (int)sysex[3]);
		return;
	}
	else if (sysex[2] != 0x16) {
		printDebug("playSysexWithoutFraming: Header not intended for MT-32: %02x %02x %02x %02x", (int)sysex[0], (int)sysex[1], (int)sysex[2], (int)sysex[3]);
		return;
	}
	if (sysex[3] != 0x12) {
		printDebug("playSysexWithoutFraming: Unsupported command %02x", sysex[3]);
		return;
	}
	playSysexWithoutHeader(sysex[1], sysex + 4, len - 4);
}

// MEMADDR() converts from sysex-padded, SYSEXMEMADDR converts to it
// Roland provides documentation using the sysex-padded addresses, so we tend to use that int code and output
#define MEMADDR(x) ((((x) & 0x7f0000) >> 2) | (((x) & 0x7f00) >> 1) | ((x) & 0x7f))
#define SYSEXMEMADDR(x) ((((x) & 0x1FC000) << 2) | (((x) & 0x3F80) << 1) | ((x) & 0x7f))

#define NUMTOUCHED(x,y) (((x) + sizeof(y) - 1) / sizeof(y))

void Synth::playSysexWithoutHeader(unsigned char device, const Bit8u *sysex, Bit32u len) {
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
	Bit32u addr = (sysex[0] << 16) | (sysex[1] << 8) | (sysex[2]);
	addr = MEMADDR(addr);
	sysex += 3;
	len -= 3;
	//printDebug("Sysex addr: 0x%06x", SYSEXMEMADDR(addr));
	// NOTE: Please keep both lower and upper bounds in each check, for ease of reading
	if (device < 0x10) {
		printDebug("WRITE-CHANNEL: Channel %d temp area 0x%06x", device, SYSEXMEMADDR(addr));
		if (/*addr >= MEMADDR(0x000000) && */addr < MEMADDR(0x010000)) {
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
			addr += MEMADDR(0x030000) + offset;
		} else if (/*addr >= 0x010000 && */ addr < MEMADDR(0x020000)) {
			addr += MEMADDR(0x030110) - MEMADDR(0x010000);
		} else if (/*addr >= 0x020000 && */ addr < MEMADDR(0x030000)) {
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
			addr += MEMADDR(0x040000) - MEMADDR(0x020000) + offset;
		} else {
			printDebug("PlaySysexWithoutHeader: Invalid channel %d address 0x%06x", device, SYSEXMEMADDR(addr));
			return;
		}
	}
	if (addr >= MEMADDR(0x030000) && addr < MEMADDR(0x030110)) {
		int off = addr - MEMADDR(0x030000);
		if (off + len > sizeof(mt32ram.patchSettings)) {
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		int firstPart = off / sizeof(MemParams::PatchTemp);
		off %= sizeof(MemParams::PatchTemp);
		for (unsigned int m = 0; m < len; m++)
			((Bit8u *)&mt32ram.patchSettings[firstPart])[off + m] = sysex[m];
		//printDebug("Patch temp: Patch %d, offset %x, len %d", off/16, off % 16, len);

		int lastPart = firstPart + NUMTOUCHED(off + len, MemParams::PatchTemp) - 1;
		for (int i = firstPart; i <= lastPart; i++) {
			int absTimbreNum = mt32ram.patchSettings[i].patch.timbreGroup * 64 + mt32ram.patchSettings[i].patch.timbreNum;
			char timbreName[11];
			memcpy(timbreName, mt32ram.timbres[absTimbreNum].timbre.common.name, 10);
			timbreName[10] = 0;
			printDebug("WRITE-PARTPATCH (%d-%d@%d..%d): %d; timbre=%d (%s)", firstPart, lastPart, off, off + len, i, absTimbreNum, timbreName);
			if (parts[i] != NULL) {
				if (i == firstPart && off > 2) {
					printDebug(" (Not updating timbre, since those values weren't touched)");
				} else {
					// Not sure whether we should do this at all, really.
					parts[i]->setTimbre(&mt32ram.timbres[parts[i]->getAbsTimbreNum()].timbre);
				}
				parts[i]->refresh();
			}
		}
	} else if (addr >= MEMADDR(0x030110) && addr < MEMADDR(0x040000)) {
		int off = addr - MEMADDR(0x030110);
		if (off + len > sizeof(mt32ram.rhythmSettings)) {
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		int firstDrum = off / sizeof(MemParams::RhythmTemp);
		off %= sizeof(MemParams::RhythmTemp);
		for (unsigned int m = 0; m < len; m++)
			((Bit8u *)&mt32ram.rhythmSettings[firstDrum])[off + m] = sysex[m];
		int lastDrum = firstDrum + NUMTOUCHED(off + len, MemParams::RhythmTemp) - 1;
		for (int i = firstDrum; i <= lastDrum; i++) {
			int timbreNum = mt32ram.rhythmSettings[i].timbre;
			char timbreName[11];
			if (timbreNum < 94) {
				memcpy(timbreName, mt32ram.timbres[128 + timbreNum].timbre.common.name, 10);
				timbreName[10] = 0;
			} else {
				strcpy(timbreName, "[None]");
			}
			printDebug("WRITE-RHYTHM (%d-%d@%d..%d): %d; level=%02x, panpot=%02x, reverb=%02x, timbre=%d (%s)", firstDrum, lastDrum, off, off + len, i, mt32ram.rhythmSettings[i].outlevel, mt32ram.rhythmSettings[i].panpot, mt32ram.rhythmSettings[i].reverbSwitch, mt32ram.rhythmSettings[i].timbre, timbreName);
		}
		if (parts[8] != NULL) {
			parts[8]->refresh();
		}
	} else if (addr >= MEMADDR(0x040000) && addr < MEMADDR(0x050000)) {
		int off = addr - MEMADDR(0x040000);
		if (off + len > sizeof(mt32ram.timbreSettings)) {
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		int firstPart = off / sizeof(TimbreParam);
		off %= sizeof(TimbreParam);
		for (unsigned int m = 0; m < len; m++)
			((Bit8u *)&mt32ram.timbreSettings[firstPart])[off + m] = sysex[m];
		int lastPart = firstPart + NUMTOUCHED(off + len, TimbreParam) - 1;
		for (int i = firstPart; i <= lastPart; i++) {
			char instrumentName[11];
			memcpy(instrumentName, mt32ram.timbreSettings[i].common.name, 10);
			instrumentName[10] = 0;
			printDebug("WRITE-PARTTIMBRE (%d-%d@%d..%d): timbre=%d (%s)", firstPart, lastPart, off, off + len, i, instrumentName);
			if (parts[i] != NULL) {
				parts[i]->refresh();
			}
		}
	}
	else if (addr >= MEMADDR(0x050000) && addr < MEMADDR(0x060000)) {
		int off = addr - MEMADDR(0x050000);
		if (off + len > sizeof(mt32ram.patches)) {
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		int firstPatch = off / sizeof(PatchParam);
		off %= sizeof(PatchParam);
		for (unsigned int m = 0; m < len; m++)
			((Bit8u *)&mt32ram.patches[firstPatch])[off + m] = sysex[m];
		int lastPatch = firstPatch + NUMTOUCHED(off + len, PatchParam) - 1;
		for (int i = firstPatch; i <= lastPatch; i++) {
			PatchParam *patch = &mt32ram.patches[i];
			int patchAbsTimbreNum = patch->timbreGroup * 64 + patch->timbreNum;
			char instrumentName[11];
			memcpy(instrumentName, mt32ram.timbres[patchAbsTimbreNum].timbre.common.name, 10);
			instrumentName[10] = 0;
			Bit8u *n = (Bit8u *)patch;
			printDebug("WRITE-PATCH (%d-%d@%d..%d): %d; timbre=%d (%s) %02X%02X%02X%02X%02X%02X%02X%02X", firstPatch, lastPatch, off, off + len, i, patchAbsTimbreNum, instrumentName, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7]);
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
	} else if (addr >= MEMADDR(0x080000) && addr < MEMADDR(0x090000)) {
		// Timbres
		int off = addr - MEMADDR(0x080000);
		if (off + len > sizeof(MemParams::PaddedTimbre) * 64) {
			// You can only write to one group at a time
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		unsigned int firstTimbre = off / sizeof (MemParams::PaddedTimbre);
		off %= sizeof (MemParams::PaddedTimbre);
		firstTimbre += 128;
		for (unsigned int m = 0; m < len; m++)
			((Bit8u *)&mt32ram.timbres[firstTimbre])[off + m] = sysex[m];
		unsigned int lastTimbre = firstTimbre + NUMTOUCHED(len + off, MemParams::PaddedTimbre) - 1;
		for (unsigned int i = firstTimbre; i <= lastTimbre; i++) {
			char instrumentName[11];
			memcpy(instrumentName, mt32ram.timbres[i].timbre.common.name, 10);
			instrumentName[10] = 0;
			printDebug("WRITE-TIMBRE (%d-%d@%d..%d): %d; name=\"%s\"", firstTimbre, lastTimbre, off, off + len, i, instrumentName);
			// FIXME:KG: Not sure if the stuff below should be done (for rhythm and/or parts)...
			// Does the real MT-32 automatically do this?
			for (unsigned int part = 0; part < 9; part++) {
				if (parts[part] != NULL) {
					parts[part]->refreshTimbre(i);
				}
			}
		}
	} else if (addr >= MEMADDR(0x100000) && addr < MEMADDR(0x200000)) {
		int off = addr - MEMADDR(0x100000);
		if (off + len > sizeof(mt32ram.system)) {
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		for (unsigned int m = 0; m < len; m++)
			((Bit8u *)&mt32ram.system)[m + off] = sysex[m];

		report(ReportType_devReconfig, NULL);

		printDebug("WRITE-SYSTEM:");
		refreshSystem();
	} else if (addr == MEMADDR(0x200000)) {
		char buf[MAX_SYSEX_SIZE];
		if (len > MAX_SYSEX_SIZE - 1) {
			printDebug("WRITE-LCD sysex length (%d) exceeded MAX_SYSEX_SIZE (%d) - 1; truncating", len, MAX_SYSEX_SIZE);
			len = MAX_SYSEX_SIZE - 1;
		}
		memcpy(&buf, &sysex[0], len);
		buf[len] = 0;
		printDebug("WRITE-LCD: %s", buf);
		report(ReportType_lcdMessage, buf);
	} else if (addr >= MEMADDR(0x7f0000)) {
		printDebug("Reset");
		report(ReportType_devReset, NULL);
		partialManager->deactivateAll();
		mt32ram = mt32default;
		for (int i = 0; i < 9; i++) {
			parts[i]->refresh();
		}
		isEnabled = false;
	} else {
		printDebug("Sysex write to unrecognised address %06x", SYSEXMEMADDR(addr));
	}
}

bool Synth::refreshSystem() {
	memset(chantable,-1,sizeof(chantable));

	for (unsigned int i = 0; i < 9; i++) {
		//LOG(LOG_MISC|LOG_ERROR,"Part %d set to MIDI channel %d",i,mt32ram.system.chanAssign[i]);
		if (mt32ram.system.chanAssign[i] == 16 && parts[i] != NULL) {
			parts[i]->allStop();
		} else {
			chantable[(int)mt32ram.system.chanAssign[i]] = (char)i;
		}
	}
	//FIXME:KG: This is just an educated guess.
	// The LAPC-I documentation claims a range of 427.5Hz-452.6Hz (similar to what we have here)
	// The MT-32 documentation claims a range of 432.1Hz-457.6Hz
	masterTune = 440.0f * powf(2.0f, (mt32ram.system.masterTune - 64.0f) / (128.0f * 12.0f));
	printDebug(" Master Tune: %f", masterTune);
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
	masterVolume = (Bit16u)(mt32ram.system.masterVol * 327);
	if (!TableInitialiser::initMT32Tables(this, PCMList, (float)myProp.sampleRate, masterTune)) {
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

void Synth::doRender(Bit16s * stream,Bit32u len) {
	Bit32u m;

	partialManager->ageAll();

	if (myProp.useReverb) {
		bool hasOutput = false;
		for (unsigned int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
			if (partialManager->shouldReverb(i)) {
				if (partialManager->produceOutput(i, &tmpBuffer[0], len)) {
					ProduceOutput1(&tmpBuffer[0], stream, len, masterVolume);
					hasOutput = true;
				}
			}
		}
		// No point in doing reverb on a mute buffer...
		if (hasOutput) {
			m=0;
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

}
