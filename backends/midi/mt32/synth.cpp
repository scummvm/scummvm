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

#define BENCHMARK 0

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#if BENCHMARK > 0
#include <time.h>
#endif

#include "mt32emu.h"

// Debugging stuff
// Used to dump drum patches to syx file for viewing
#define DUMPDRUMS 0

#define SYSEX_SIZE 512

namespace MT32Emu {

iir_filter_type usefilter;

static const Bit8u InitPatches[8] = {
	68, 48, 95, 78, 41, 3, 110, 122
};

// Maps MIDI channel numbers to MT-32 parts (not to be confused with "partials")
// This is the default (FIXME: the mapping from 11->9 is undocumented, is this correct?):
static const Bit8s InitChanTable[16] = {
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 8, -1, -1, -1, -1, -1
};
// This alternative configuration can be selected by holding "Master Volume"
// and pressing "PART button 1" on the real MT-32's frontpanel.
//static const Bit8s InitChanTable[16] = {
//	0, 1, 2, 3, 4, 5, 6, 7, -1, 8, -1, -1, -1, -1, -1, -1
//};

static int axtoi(char *str) {
	int result = 0;
	while (*str) {
		char ch = *str++;
		if (ch >= '0' && ch <= '9')
			ch -= '0';
		else if (ch >= 'a' && ch <= 'f')
			ch = ch + 10 - 'a';
		else if (ch >= 'A' && ch <= 'F')
			ch = ch + 10 - 'A';
		else
			break;
		result = (result << 4) | ch;
	}
	return result;
}

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

Synth::Synth() {
	isOpen = false;
	reverbModel = NULL;
	partialManager = NULL;
	memset(waveforms, 0, sizeof(waveforms));
	memset(parts, 0, sizeof(parts));
}

Synth::~Synth() {
	close(); // Make sure we're closed and everything is freed
}

void Synth::report(ReportType type, void *data) {
	if (myProp.report != NULL) {
		myProp.report(myProp.userData, type, data);
	}
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

void Synth::initReverb(char newRevMode, char newRevTime) {
	// FIXME:KG: I don't think it's necessary to recreate the reverbModel... Just set the parameters
	if (reverbModel != NULL)
		delete reverbModel;
	reverbModel = new revmodel();

	curRevTime = newRevTime;
	curRevMode = newRevMode;

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
	reverbModel->setwet((float)mt32ram.params.system.reverbLevel / 8.0f);
	reverbModel->setwidth((float)curRevTime / 8.0f);
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

bool Synth::loadPreset(const char *filename) {
	File *file = openFile(filename, File::OpenMode_read);
	if (file == NULL) {
		printDebug("*** Error: Failed to load preset %s", filename);
		return false;
	}
	bool inSys = false;
	Bit8u sysexBuf[SYSEX_SIZE];
	Bit16u syslen = 0;
	int filePos = 0;
	bool rc = true;
	for (;;) {
		int fc = file->readByte();
		if (fc == -1) {
			if (!file->isEOF()) {
				rc = false;
			}
			break;
		}
		Bit8u c = (Bit8u)fc;
		sysexBuf[syslen] = c;
		syslen++;
		filePos++;
		if (c==0xf0)
			inSys = true;
		if ((c==0xf7) && (inSys)) {
			playSysex(&sysexBuf[0],syslen);
			inSys = false;
			syslen = 0;
		}
	}
	closeFile(file);
	return rc;
}

unsigned char calcChecksum(unsigned char *data, unsigned int len, unsigned char checksum) {
	for (unsigned int i = 0; i < len; i++) {
		checksum = checksum + data[i];
	}
	checksum = checksum & 0x7f;
	if (checksum)
		checksum = 0x80 - checksum;
	return checksum;
}

bool Synth::loadDrums(const char *filename) {
	File *file = openFile(filename, File::OpenMode_read);
	if (file == NULL) {
		return false;
	}
	int drumnum = 0;
	for (;;) {
		//Read common area
		TimbreParam *timbre = &mt32ram.params.timbres[drumnum + 192].timbre;
		if (file->read(&timbre->common, 14) != 14)
			break;
		char drumname[11];
		strncpy(drumname, timbre->common.name, 10);
		drumname[10] = 0;
		bool breakout = false;
		for (int t=0;t<4;t++) {
			if (((timbre->common.pmute >> t) & 0x1) == 0x1) {
				if (file->read(&timbre->partial[t], 58) != 58) {
					breakout = true;
					break;
				}
				//printDebug("Loaded drum #%d (%s) - t %d", drumnum, drumname, t);
			}
		}
		if (breakout) {
			break;
		}
		//printDebug("Loaded drum #%d (%s)", drumnum, drumname);
		drumnum++;
	}
	closeFile(file);
	return true;
}

void Synth::dumpDrums(const char *filename) {
	File *file = openFile(filename, File::OpenMode_write);
	if (file == NULL)
		return;
	char dumbtext[10];
	memset(dumbtext,0,10);
	for (int drumnum=0;drumnum<30;drumnum++) {
		// Sysex header
		if (!file->writeByte(0xf0))
			break;
		if (!file->writeByte(0x41))
			break;
		if (!file->writeByte(0x10))
			break;
		if (!file->writeByte(0x16))
			break;
		if (!file->writeByte(0x12))
			break;

		int useaddr = drumnum * 256;
		char lsb = (char)(useaddr & 0x7f);
		char isb = (char)((useaddr >> 7) & 0x7f);
		char msb = (char)(((useaddr >> 14) & 0x7f) | 0x08);
		//Address
		if (!file->writeByte(msb))
			break;
		if (!file->writeByte(isb))
			break;
		if (!file->writeByte(lsb))
			break;

		TimbreParam *timbre = &mt32ram.params.timbres[192 + drumnum].timbre;
		//Data
		if (file->write(&timbre->common,0xE) != 0xE)
			break;
		if (file->write(&timbre->partial[0],0x3A) != 0x3A)
			break;
		if (file->write(&timbre->partial[1],0x3A) != 0x3A)
			break;
		if (file->write(&timbre->partial[2],0x3A) != 0x3A)
			break;
		if (file->write(&timbre->partial[3],0x3A) != 0x3A)
			break;
		//Checksum
		unsigned char *dat = (unsigned char *)timbre;
		unsigned char checksum = calcChecksum(dat, 246, msb + isb + lsb);
		if (!file->writeByte(checksum))
			break;

		//End of sysex
		if (!file->writeByte(0xf7))
			break;
	}
	closeFile(file);
}

bool Synth::loadPCMToROMMap(const char *filename) {
	File *file = openFile(filename, File::OpenMode_read); // Original specified text mode

	if (file == NULL) {
		return false;
	}

	for (int i=0;i<54;i++) {
		PCMReassign[i] = i;
		PCM[i].tune = 220.0f;
		PCM[i].ampval = 256;
	}
	//PCM[53].ampval = 128;

	char tbuf[512];
	char *cp;
	if (!file->readLine(tbuf,sizeof(tbuf))) {
		return false;
	}
	Bit32u patchstart = 0; //axtoi(tbuf);
	Bit32u patchend = 0;
	Bit32u patchcount = 0;
	bool rc = true;
	for (;;) {
		if (!file->readLine(tbuf,sizeof(tbuf))) {
			if (!file->isEOF()) {
				rc = false;
			}
			break;
		}
		cp = strtok(tbuf," \n\r");
		PCM[patchcount].loop = false;
		if (cp != NULL) {
			patchend = axtoi(cp);
			cp = strtok(NULL," \n\r");
			if (cp != NULL) {
				cp = strtok(NULL," \n\r");
				if (cp != NULL) {
					cp = strtok(NULL," \n\r");
					if (cp !=NULL) {
						int newpcm = atoi(cp);
						PCMReassign[newpcm] = patchcount;
						cp = strtok(NULL," \n\r");
						if (cp != NULL) {
							if (atoi(cp)==1)
								PCM[patchcount].loop = true;
							cp = strtok(NULL," \n\r");
							if (cp != NULL) {
								PCM[patchcount].tune = (float)atoi(cp) / 100.0f;
								//printDebug("PCM %d tuning at %f", patchcount, PCM[patchcount].tune);
							}
						}
					}
				}
			}
		}
		if (patchend==0)
			break;

		PCM[patchcount].addr =  patchstart;
		PCM[patchcount].len = patchend - patchstart;
		patchcount++;
		//printf("Patch %d %d %d %d", patchcount, patchstart, patchend, mt32ram.PCM[patchcount].len);
		patchstart = patchend;
	}
	closeFile(file);
	if (!rc)
		return rc;

	PCM[53].len = 1950;

	// Generate official PCM list

	// Normal sounds
	int pat = 0;
	for (int p = 0; p < 54; p++) {
		PCMList[pat].addr = PCM[PCMReassign[p]].addr;
		PCMList[pat].len = PCM[PCMReassign[p]].len;
		PCMList[pat].loop = PCM[PCMReassign[p]].loop;
		PCMList[pat].aggSound = -1;
		PCMList[pat].pcmnum = PCMReassign[p];
		pat++;
	}

	// Drum specific sounds.  Not exactly sure yet how these are different
	for (int p = 0; p < 20; p++) {
		PCMList[pat] = PCMList[p];
		pat++;
	}

	// Looped PCM sounds.  The last remaining 9 are aggregate sounds;
	// FIXME:KG: I hope this is correct; the original was heavily broken,
	// and it was hard to determine the author's intention.
	for (int p = 0; p < 54; p++) {
		if (p < 45) {
			int pcmNum = p > 7 ? p - 1 : p;
			PCMList[pat].addr = PCM[PCMReassign[pcmNum]].addr;
			PCMList[pat].len = PCM[PCMReassign[pcmNum]].len;
			PCMList[pat].pcmnum = PCMReassign[pcmNum];
			PCMList[pat].loop = true;
			PCMList[pat].aggSound = -1;
		} else {
			//Calculate aggregate length
			int aggsnd = p - 45;
			int tmplen = 0;
			int sndpos = 0;
			while (LoopPatterns[aggsnd][sndpos] != -1) {
				tmplen += PCM[LoopPatterns[aggsnd][sndpos]].len;
				sndpos++;
			}
			PCMList[pat].addr = 0;
			PCMList[pat].len = tmplen;
			PCMList[pat].loop = true;
			PCMList[pat].aggSound = aggsnd;
		}
		pat++;
	}

	//for (p=0;p<128;p++)
	//	printDebug("PCM #%d addr 0x%x len %d loop %d aggSound %d pcmnum %d", p, PCMList[p].addr, PCMList[p].len, PCMList[p].loop, PCMList[p].aggSound, PCMList[p].pcmnum);
	return true;
}

bool Synth::loadROM(const char *filename) {
	File *file = openFile(filename, File::OpenMode_read); // ROM File
	if (file == NULL) {
		return false;
	}
#ifdef MT32OUT
	File *outFile = openFile("mt32out.raw", File::OpenMode_write);
	File *outFileB = openFile("mt32out2.raw", File::OpenMode_write);
#endif
	bool rc = true;
	for (int i = 0; ; i++) {
		int fc = file->readByte();
		if (fc == -1) {
			if (!file->isEOF()) {
				rc = false;
			}
			break;
		}
		Bit16s s = (Bit16s)fc;
		fc = file->readByte();
		if (fc == -1) {
			if (!file->isEOF()) {
				rc = false;
			} else {
				printDebug("ROM file has an odd number of bytes! Ignoring last");
			}
			break;
		}
		Bit16s c = (Bit16s)fc;

		short e;
		int bit;
		int u;

		int order[16] = {0, 9, 1 ,2, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15, 8};

		e=0;
		for (u=0;u<15;u++) {
			if (order[u]<8)
				bit = (s >> (7-order[u])) & 0x1;
			else
				bit = (c >> (7-(order[u]-8))) & 0x1;
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

#ifdef MT32OUT
		outFile->writeByte(e & 0xff);
		outFile->writeByte(((e >> 8) & 0x7f));
#endif
		// File is encoded in dB, convert to PCM
		// MINDB = -96
		// MAXDB = -15
		float testval;
		testval = (float)((~e) & 0x7fff);
		testval = -(testval / 400.00f);
		//testval = -(testval / 341.32291666666666666666666666667);
		float vol = powf(8,(testval / 20)) * 32767.0f;

		if (e>0)
			vol = -vol;

		romfile[i] = (Bit16s)vol;

#ifdef MT32OUT
		outFileB->writeByte(romfile[i] & 0xff);
		outFileB->writeByte(romfile[i] >> 8);
#endif
	}
#ifdef MT32OUT
	closeFile(outFileB);
	closeFile(outFile);
#endif
	closeFile(file);
	return rc;
}

bool Synth::open(SynthProperties &useProp) {
	if (isOpen)
		return false;

	// Initalise patch information

	myProp = useProp;

	usefilter = &iir_filter_normal;

	partialManager = new PartialManager(this);

	// This is so that names won't be garbage during early setup debug output, but we can detect bugs
	memset(&mt32ram, '?', sizeof(mt32ram));

	printDebug("Initialising patch banks");
	initmode = 0;
	if (!loadPreset("Preset1.syx")) {
		report(ReportType_errorPreset1, &errno);
		return false;
	}
	initmode = 1;
	if (!loadPreset("Preset2.syx")) {
		report(ReportType_errorPreset2, &errno);
		return false;
	}
	initmode = 2;

	printDebug("Initialising Drums");
	if (!loadDrums("drumpat.rom")) {
		report(ReportType_errorDrumpat, &errno);
		return false;
	}

#if DUMPDRUMS == 1
	strcpy(&pathBuf[0], baseDir);
	dumpDrums(strcat(&pathBuf[0],"drumsys.syx"));
#endif

	printDebug("Initialising PCM-to-ROM map");
	if (!loadPCMToROMMap("patchlog.cfg")) {
		printDebug("Init Error - Missing patchlog.cfg");
		report(ReportType_errorPatchlog, &errno);
		return false;
	}

	printDebug("Initialising ROM");
	if (!loadROM("MT32_PCM.ROM")) {
		printDebug("Init Error - Missing MT32_PCM.ROM");
		report(ReportType_errorMT32ROM, &errno);
		return false;
	}
	memcpy(chantable, InitChanTable, sizeof (chantable));
	for (unsigned char i = 0; i < 128; i++) {
		mt32ram.params.patches[i].timbreGroup = i >> 6;
		mt32ram.params.patches[i].timbreNum = i & 63;
	}

	TableInitialiser tableInitialiser;
	tableInitialiser.initMT32Tables(this, PCM, (float)myProp.SampleRate);
	if (myProp.UseDefault)
		initReverb(0,5);
	else
		initReverb(myProp.RevType, myProp.RevTime);

	for (int i = 0; i < 9; i++) {
		parts[i] = new Part(this, i);

		if (i<8) {
			// The patch is already set by the presets, now set the timbre it wants
			parts[i]->setTimbre(&mt32ram.params.timbres[parts[i]->getAbsTimbreNum()].timbre);
			// And refresh the part's cache
			parts[i]->RefreshPatch();
		}
	}

	// For resetting mt32 mid-execution
	mt32default = mt32ram;

#ifdef HAVE_X86
	bool availableSSE = DetectSIMD();
	bool available3DNow = Detect3DNow();

	if (availableSSE)
		report(ReportType_availableSSE, NULL);
	if (available3DNow)
		report(ReportType_available3DNow, NULL);

	if (available3DNow) {
		printDebug("Detected and using SIMD (AMD 3DNow) extensions");
		usefilter = &iir_filter_3dnow;
	} else if (availableSSE) {
		printDebug("Detected and using SIMD (Intel SSE) extensions");
		usefilter = &iir_filter_sse;
	}
#endif

#if BENCHMARK > 1
	// Benchmark 3DNow, Floating point, and SSE filters
	clock_t start, end;
	float histval[50];

	for (int bench = 0; bench < 3; bench++) {
		start = clock();
		for (int benchcnt=0;benchcnt<2000000;benchcnt++) {
			switch (bench) {
				case 0:
					iir_filter_normal(0,&histval[0],filtcoeff[0][0],0);
					break;
				case 1:
					if (!availableSSE) {
						printDebug("Skipping SSE benchmark, SSE not available");
						continue;
					}
			        iir_filter_sse(0,&histval[0],filtcoeff[0][0],0);
					break;
				case 2:
					if (!available3DNow) {
						printDebug("Skipping 3DNow benchmark, 3DNow not available");
						continue;
					}
			        iir_filter_3dnow(0,&histval[0],filtcoeff[0][0],0);
					break;
			}
		}
		end = clock();
		printDebug("Bench %ld completed in %ld milliseconds", bench, (end - start) * 1000 / CLOCKS_PER_SEC);
	}
#endif

	isOpen=true;
	isEnabled=false;

	printDebug("**************** Initialisation complete ****************");
	return true;
};

void Synth::close(void) {
	if (!isOpen)
		return;

	for (int t = 0; t < 4; t++) {
		for (int m = 0; m < 128; m++) {
			if (waveforms[t][m]!=NULL) {
				delete[] waveforms[t][m];
				waveforms[t][m] = NULL;
			}
		}
	}
	if (partialManager != NULL) {
		partialManager = NULL;
		delete partialManager;
	}

	if (reverbModel != NULL) {
		reverbModel = NULL;
		delete reverbModel;
	}

	for (int i = 0; i < 9; i++) {
		if (parts[i] != NULL) {
			delete parts[i];
			parts[i] = NULL;
		}
	}

	isOpen=false;
};

void Synth::playMsg(Bit32u msg) {
	unsigned char code = (unsigned char)((msg & 0xf0) >> 4);
	unsigned char chan = (unsigned char)(msg & 0xf);
	unsigned char note = (unsigned char)((msg & 0xff00) >> 8);
	unsigned char velocity = (unsigned char)((msg & 0xff0000) >> 16);
	isEnabled = true;

	//if (code!=0xf) printDebug("Playing chan %d, code 0x%01x note: 0x%02x", chan, code, note);

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
		parts[part]->StopNote(note);
		break;
	case 0x9:
		//printDebug("Note ON - Part %d, Note %d Vel %d", part, note, velocity);
		if (velocity == 0) {
			// MIDI defines note-on with velocity 0 as being the same as note-off with velocity 40
			parts[part]->StopNote(note);
		} else {
			parts[part]->PlayNote(partialManager, note, velocity);
		}
		break;
	case 0xB: // Control change
		switch (note) {
		case 0x01:  // Modulation
			//printDebug("Modulation: %d", velocity);
			parts[part]->SetModulation(velocity);
			break;
		case 0x0B:
			//printDebug("Expression set: %d", velocity);
			parts[part]->SetVolume(velocity);
			break;
		case 0x07:  // Set volume
			//if (part!=3) return;
			//printDebug("Volume set: %d", velocity);
			parts[part]->SetVolume(velocity);
			break;
		case 0x0A:  // Pan
			//printDebug("Pan set: %d", velocity);
			parts[part]->SetPan(velocity);
			break;
		case 0x40: // Hold pedal
			//printDebug("Hold pedal set: %d", velocity);
			parts[part]->SetHoldPedal(velocity>=64);
			break;

		case 0x7B: // All notes off
			//printDebug("All notes off");
			parts[part]->AllStop();
			break;
		case 0x79: // Reset all controllers
			printDebug("Reset all controllers (NYI)");
			break;

		default:
			printDebug("Unknown MIDI Control code: 0x%02x - vel %02x",note, velocity);
			break;
		}

		break;
	case 0xC: // Program change
		//printDebug("Program change %01x", note);
		if (part < 8) {
			parts[part]->SetPatch(note);
		} else {
			printDebug("Program change attempted on rhythm part");
		}
		break;
	case 0xE: // Pitch bender
		bend = (velocity << 7) | (note);
		//printDebug("Pitch bender %02x", bend);
		parts[part]->SetBend(bend);
		break;
	default:
		printDebug("Unknown Midi code: 0x%01x - %02x - %02x", code, note, velocity);
		break;
	}

	//midiOutShortMsg(m_out, msg);
};

void Synth::playSysex(Bit8u * sysex,Bit32u len) {
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

void Synth::playSysexWithoutFraming(Bit8u * sysex, Bit32u len) {
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

void Synth::playSysexWithoutHeader(unsigned char device, Bit8u *sysex, Bit32u len) {
	if (device > 0x10) {
		// We have device ID 0x10 (default, but changeable, on real MT-32), < 0x10 is for channels
		printDebug("playSysexWithoutHeader: Message is not intended for this device ID (provided: %02x, expected: 0x10 or channel)", (int)device);
		return;
	}
	if (len < 4) {
		printDebug("playSysexWithoutHeader: Message is too short (%d bytes)!", len);
		return;
	}
	unsigned char checksum = calcChecksum(sysex, len - 1, 0);
	if (checksum != sysex[len - 1]) {
		printDebug("playSysexWithoutHeader: Message checksum is incorrect (provided: %02x, expected: %02x)!", sysex[len - 1], checksum);
		return;
	}
	len -= 1; // Exclude checksum
	Bit32u addr = (sysex[0] << 16) | (sysex[1] << 8) | (sysex[2]);
	addr = MEMADDR(addr);
	sysex += 3;
	len -= 3;
	printDebug("Sysex addr: 0x%06x", SYSEXMEMADDR(addr));
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
		if (off + len > sizeof(mt32ram.banks.pTemp)) {
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		int firstPart = off / sizeof(MemParams::PatchTemp);
		off %= sizeof(MemParams::PatchTemp);
		for (unsigned int m = 0; m < len; m++)
			mt32ram.banks.pTemp[firstPart][off + m] = sysex[m];
		//printDebug("Patch temp: Patch %d, offset %x, len %d", off/16, off % 16, len);

		int lastPart = firstPart + NUMTOUCHED(off + len, MemParams::PatchTemp) - 1;
		for (int i = firstPart; i <= lastPart; i++) {
			int absTimbreNum = mt32ram.params.patchSettings[i].patch.timbreGroup * 64 + mt32ram.params.patchSettings[i].patch.timbreNum;
			char timbreName[11];
			memcpy(timbreName, mt32ram.params.timbres[absTimbreNum].timbre.common.name, 10);
			timbreName[10] = 0;
			printDebug("WRITE-PARTPATCH (%d-%d@%d..%d): %d; timbre=%d (%s)", firstPart, lastPart, off, off + len, i, absTimbreNum, timbreName);
			if (parts[i] != NULL) {
				parts[i]->setTimbre(&mt32ram.params.timbres[parts[i]->getAbsTimbreNum()].timbre);
				parts[i]->RefreshPatch();
			}
		}
	} else if (addr >= MEMADDR(0x030110) && addr < MEMADDR(0x040000)) {
		int off = addr - MEMADDR(0x030110);
		if (off + len > sizeof(mt32ram.banks.rTemp)) {
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		int firstDrum = off / sizeof(MemParams::RhythmTemp);
		off %= sizeof(MemParams::RhythmTemp);
		for (unsigned int m = 0; m < len; m++)
			mt32ram.banks.rTemp[firstDrum][off + m] = sysex[m];
		int lastDrum = firstDrum + NUMTOUCHED(off + len, MemParams::RhythmTemp) - 1;
		for (int i = firstDrum; i <= lastDrum; i++) {
			int timbreNum = mt32ram.params.rhythmSettings[i].timbre;
			char timbreName[11];
			if (timbreNum < 94) {
				memcpy(timbreName, mt32ram.params.timbres[128 + timbreNum].timbre.common.name, 10);
				timbreName[10] = 0;
			} else {
				strcpy(timbreName, "[None]");
			}
			printDebug("WRITE-RHYTHM (%d-%d@%d..%d): %d; level=%02x, panpot=%02x, reverb=%02x, timbre=%d (%s)", firstDrum, lastDrum, off, off + len, i, mt32ram.params.rhythmSettings[i].outlevel, mt32ram.params.rhythmSettings[i].panpot, mt32ram.params.rhythmSettings[i].reverbSwitch, mt32ram.params.rhythmSettings[i].timbre, timbreName);
		}
		if (parts[8] != NULL) {
			parts[8]->RefreshDrumCache();
		}
	} else if (addr >= MEMADDR(0x040000) && addr < MEMADDR(0x050000)) {
		int off = addr - MEMADDR(0x040000);
		if (off + len > sizeof(mt32ram.banks.tTemp)) {
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		int firstPart = off / sizeof(TimbreParam);
		off %= sizeof(TimbreParam);
		for (unsigned int m = 0; m < len; m++)
			mt32ram.banks.tTemp[firstPart][off + m] = sysex[m];
		int lastPart = firstPart + NUMTOUCHED(off + len, TimbreParam) - 1;
		for (int i = firstPart; i <= lastPart; i++) {
			char instrumentName[11];
			memcpy(instrumentName, mt32ram.params.timbreSettings[i].common.name, 10);
			instrumentName[10] = 0;
			printDebug("WRITE-PARTTIMBRE (%d-%d@%d..%d): timbre=%d (%s)", firstPart, lastPart, off, off + len, i, instrumentName);
			if (parts[i] != NULL) {
				parts[i]->RefreshPatch();
			}
		}
	}
	else if (addr >= MEMADDR(0x050000) && addr < MEMADDR(0x060000)) {
		int off = addr - MEMADDR(0x050000);
		if (off + len > sizeof(mt32ram.banks.patchBank)) {
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		int firstPatch = off / sizeof(PatchParam);
		off %= sizeof(PatchParam);
		for (unsigned int m = 0; m < len; m++)
			mt32ram.banks.patchBank[firstPatch][off + m] = sysex[m];
		int lastPatch = firstPatch + NUMTOUCHED(off + len, PatchParam) - 1;
		for (int i = firstPatch; i <= lastPatch; i++) {
			PatchParam *patch = &mt32ram.params.patches[i];
			int patchAbsTimbreNum = patch->timbreGroup * 64 + patch->timbreNum;
			char instrumentName[11];
			memcpy(instrumentName, mt32ram.params.timbres[patchAbsTimbreNum].timbre.common.name, 10);
			instrumentName[10] = 0;
			printDebug("WRITE-PATCH (%d-%d@%d..%d): %d; timbre=%d (%s)", firstPatch, lastPatch, off, off + len, i, patchAbsTimbreNum, instrumentName);
			// FIXME:KG: The below is definitely dodgy. We just guess that this is the patch that the part was using
			// based on a timbre match (but many patches could have the same timbre!)
			// If this refresh is really correct, we should store the patch number in use by each part.
			/*
			for (int part = 0; part < 8; part++) {
				if (parts[part] != NULL) {
					int partPatchAbsTimbreNum = mt32ram.params.patchSettings[part].patch.timbreGroup * 64 + mt32ram.params.patchSettings[part].patch.timbreNum;
					if (partPatchAbsTimbreNum == patchAbsTimbreNum) {
						parts[part]->setPatch(patch);
						parts[part]->RefreshPatch();
					}
				}
			}
			*/
		}
	} else if (addr >= MEMADDR(0x080000) && addr < MEMADDR(0x090000)) {
		// Timbre patches
		int off = addr - MEMADDR(0x080000);
		if (off + len > sizeof(MemParams::PaddedTimbre) * 64) {
			// You can only write to one group at a time
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		unsigned int firstTimbre = off / sizeof (MemParams::PaddedTimbre);
		off %= sizeof (MemParams::PaddedTimbre);
		switch (initmode) {
			case 0:
				// Write into first built-in timbre group
				break;
			case 1:
				// Write into second built-in timbre group
				firstTimbre += 64;
				break;
			default:
				firstTimbre += 128;
				// Write into user timbre group
		}
		for (unsigned int m = 0; m < len; m++)
			mt32ram.banks.timbreBank[firstTimbre][off + m] = sysex[m];
		unsigned int lastTimbre = firstTimbre + NUMTOUCHED(len + off, MemParams::PaddedTimbre) - 1;
		for (unsigned int i = firstTimbre; i <= lastTimbre; i++) {
			char instrumentName[11];
			memcpy(instrumentName, mt32ram.params.timbres[i].timbre.common.name, 10);
			instrumentName[10] = 0;
			printDebug("WRITE-TIMBRE (%d-%d@%d..%d): %d; name=\"%s\"", firstTimbre, lastTimbre, off, off + len, i, instrumentName);
			// FIXME:KG: Not sure if the stuff below should be done (for rhythm and/or parts)...
			// Does the real MT-32 automatically do this?
			if (i >= 128 && parts[8] != NULL) {
				// FIXME:KG: Only bother to re-cache when this timbre's actually in the rhythm map
				parts[8]->SetPatch(i); // Re-cache this timbre
			}
			for (unsigned int part = 0; part < 8; part++) {
				if (parts[part] != NULL) {
					if (parts[part]->getAbsTimbreNum() == i) {
						parts[part]->RefreshPatch();
					}
				}
			}
		}
	} else if (addr >= MEMADDR(0x100000) && addr < MEMADDR(0x200000)) {
		int off = addr - MEMADDR(0x100000);
		if (off + len > sizeof(mt32ram.banks.systemBank)) {
			printDebug("playSysexWithoutHeader: Message goes beyond bounds of memory region (addr=0x%06x, len=%d)!", SYSEXMEMADDR(addr), len);
			return;
		}
		for (unsigned int m = 0; m < len; m++)
			mt32ram.banks.systemBank[m + off] = sysex[m];

		report(ReportType_devReconfig, NULL);

		printDebug("System Reconfiguration:");
		memset(chantable,-1,sizeof(chantable));

		for (unsigned int i = 0; i < 9; i++) {
			//LOG(LOG_MISC|LOG_ERROR,"Part %d set to MIDI channel %d",i,mt32ram.params.system.chanAssign[i]);
			if (mt32ram.params.system.chanAssign[i] == 16) {
				parts[i]->AllStop();
			} else {
				chantable[(int)mt32ram.params.system.chanAssign[i]] = (char)i;
			}
		}

		printDebug(" Master Tune: %f", ((float)mt32ram.params.system.masterTune)*0.2+432.1);
		printDebug(" Reverb: mode=%d, time=%d, level=%d", mt32ram.params.system.reverbMode, mt32ram.params.system.reverbTime, mt32ram.params.system.reverbLevel);
		report(ReportType_newReverbMode,  &mt32ram.params.system.reverbMode);
		report(ReportType_newReverbTime,  &mt32ram.params.system.reverbTime);
		report(ReportType_newReverbLevel, &mt32ram.params.system.reverbLevel);

		if ((mt32ram.params.system.reverbMode != curRevMode) || (mt32ram.params.system.reverbTime != curRevTime)) {
			if (myProp.UseDefault) {
				initReverb(mt32ram.params.system.reverbMode, mt32ram.params.system.reverbTime);
				curRevLevel = mt32ram.params.system.reverbLevel;
			} else {
				initReverb(myProp.RevType, myProp.RevTime);
				curRevLevel = myProp.RevLevel;
			}
		}

		char *rset = mt32ram.params.system.reserveSettings;
		printDebug(" Partial reserve: 1=%02d 2=%02d 3=%02d 4=%02d 5=%02d 6=%02d 7=%02d 8=%02d Rhythm=%02d", rset[0], rset[1], rset[2], rset[3], rset[4], rset[5], rset[6], rset[7], rset[8]);
		int pr = partialManager->SetReserve(rset);
		if (pr != 32)
			printDebug(" (Partial Reserve Table with less than 32 partials reserved!)");
		rset = mt32ram.params.system.chanAssign;
		printDebug(" Part assign:     1=%02d 2=%02d 3=%02d 4=%02d 5=%02d 6=%02d 7=%02d 8=%02d Rhythm=%02d", rset[0], rset[1], rset[2], rset[3], rset[4], rset[5], rset[6], rset[7], rset[8]);
		printDebug(" Master volume: %d", mt32ram.params.system.masterVol);
		mastervolume = (Bit16s)((float)mt32ram.params.system.masterVol * 327.0);
	} else if (addr == MEMADDR(0x200000)) {
		char buf[SYSEX_SIZE];
		memset(&buf, 0, SYSEX_SIZE);
		memcpy(&buf, &sysex[0], len);
		printDebug("LCD Display: %s", buf);
		report(ReportType_lcdMessage, buf);
	} else if (addr >= MEMADDR(0x7f0000)) {
		printDebug("Reset");
		report(ReportType_devReset, NULL);
		partialManager->DeactivateAll();
		mt32ram = mt32default;
		for (int i = 0; i < 8; i++) {
			parts[i]->RefreshPatch();
		}
		parts[8]->RefreshDrumCache();
		isEnabled = false;
	} else {
		printDebug("Sysex write to unrecognised address %06x", SYSEXMEMADDR(addr));
	}
};

int Synth::dumpSysex(char *filename) {
	File *file = openFile(filename, File::OpenMode_write);
	if (file == NULL)
		return -1;

	int patchnum;
	for (patchnum=0;patchnum<64;patchnum++) {
		// Sysex header
		if (!file->writeByte(0xF0))
			break;
		if (!file->writeByte(0x41))
			break;
		if (!file->writeByte(0x10))
			break;
		if (!file->writeByte(0x16))
			break;
		if (!file->writeByte(0x12))
			break;

		int useaddr = patchnum * 256;
		char lsb = (char)(useaddr & 0x7f);
		char isb = (char)((useaddr >> 7) & 0x7f);
		char msb = (char)(((useaddr >> 14) & 0x7f) | 0x08);
		//Address
		if (!file->writeByte(msb))
			break;
		if (!file->writeByte(isb))
			break;
		if (!file->writeByte(lsb))
			break;

		//Data
		if (file->write(&mt32ram.params.timbres[patchnum + 128].timbre.common,0xE) != 0xE)
			break;
		if (file->write(&mt32ram.params.timbres[patchnum + 128].timbre.partial[0],0x3A) != 0x3A)
			break;
		if (file->write(&mt32ram.params.timbres[patchnum + 128].timbre.partial[1],0x3A) != 0x3A)
			break;
		if (file->write(&mt32ram.params.timbres[patchnum + 128].timbre.partial[2],0x3A) != 0x3A)
			break;
		if (file->write(&mt32ram.params.timbres[patchnum + 128].timbre.partial[3],0x3A) != 0x3A)
			break;
		//Checksum
		unsigned char *dat = (unsigned char *)&mt32ram.params.timbres[patchnum + 128].timbre;
		unsigned char checksum = calcChecksum(dat, 246, msb + isb + lsb);

		if (!file->writeByte(checksum))
			break;

		//End of sysex
		if (!file->writeByte(0xF7))
			break;
	}
	closeFile(file);
	printDebug("Wrote temp patches to %s", filename);
	return 0;
}

void ProduceOutput1(Bit16s *useBuf, Bit16s *stream, Bit32u len, Bit16s volume) {
#if USE_MMX > 2
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

	partialManager->AgeAll();

	if (myProp.UseReverb) {
		bool hasOutput = false;
		for (unsigned int i = 0; i < MAXPARTIALS; i++) {
			if (partialManager->shouldReverb(i)) {
				if (partialManager->ProduceOutput(i, &tmpBuffer[0], len)) {
					ProduceOutput1(&tmpBuffer[0], stream, len, mastervolume);
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
		for (unsigned int i = 0; i < MAXPARTIALS; i++) {
			if (!partialManager->shouldReverb(i)) {
				if (partialManager->ProduceOutput(i, &tmpBuffer[0], len)) {
					ProduceOutput1(&tmpBuffer[0], stream, len, mastervolume);
				}
			}
		}
	} else {
		for (unsigned int i = 0; i < MAXPARTIALS; i++) {
			if (partialManager->ProduceOutput(i, &tmpBuffer[0], len))
				ProduceOutput1(&tmpBuffer[0], stream, len, mastervolume);
		}
	}

	partialManager->ClearAlreadyOutputed();

#if MONITORPARTIALS == 1
	samplepos += len;
	if (samplepos > myProp.SampleRate * 5) {
		samplepos = 0;
		int partialUsage[9];
		partialManager->GetPerPartPartialUsage(partialUsage);
		printDebug("1:%02d 2:%02d 3:%02d 4:%02d 5:%02d 6:%02d 7:%02d 8:%02d", partialUsage[0], partialUsage[1], partialUsage[2], partialUsage[3], partialUsage[4], partialUsage[5], partialUsage[6], partialUsage[7]);
		printDebug("Rhythm: %02d  TOTAL: %02d", partialUsage[8], MAXPARTIALS - partialManager->GetFreePartialCount());
	}
#endif
}

}
