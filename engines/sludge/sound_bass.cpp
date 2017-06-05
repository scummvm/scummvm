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

#include "sludge/allfiles.h"
#include "sludge/newfatal.h"
#include "sludge/sound.h"
#include "sludge/moreio.h"
#include "sludge/fileset.h"

#define MAX_SAMPLES 8
#define MAX_MODS 3
#define EFFECT_CHANNELS 8
#define TOTAL_CHANNELS 32

namespace Sludge {

bool soundOK = false;

struct soundThing {
	HSAMPLE sample;
	int fileLoaded, vol;
	int mostRecentChannel;
	bool looping;
};

DWORD mod[MAX_MODS];

soundThing soundCache[MAX_SAMPLES];

int defVol = 128;
int defSoundVol = 255;

char *loadEntireFileToMemory(Common::SeekableReadStream *inputFile, uint32_t size) {
	char *allData = new char[size];
	if (!allData)
		return NULL;
	inputFile->read(allData, size);
	finishAccess();

	return allData;
}

void stopMOD(int i) {
	if (mod[i]) {
		BASS_ChannelStop(mod[i]);
		BASS_MusicFree(mod[i]);
		mod[i] = NULL;
	}
}

int findInSoundCache(int a) {
	int i;
	for (i = 0; i < MAX_SAMPLES; i++) {
		if (soundCache[i].fileLoaded == a)
			return i;
	}
	return -1;
}

void huntKillSound(int filenum) {
	int gotSlot = findInSoundCache(filenum);
	if (gotSlot == -1)
		return;
	soundCache[gotSlot].looping = false;
	BASS_SampleStop(soundCache[gotSlot].sample);
}

void freeSound(int a) {
	BASS_SampleFree(soundCache[a].sample);
	soundCache[a].sample = NULL;
	soundCache[a].fileLoaded = -1;
	soundCache[a].looping = false;
}

void huntKillFreeSound(int filenum) {
	int gotSlot = findInSoundCache(filenum);
	if (gotSlot != -1)
		freeSound(gotSlot);
}

bool initSoundStuff(HWND hwnd) {
	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		warning(WARNING_BASS_WRONG_VERSION);
		return false;
	}

	if (!BASS_Init(-1, 44100, 0, hwnd, NULL)) {
		warning(WARNING_BASS_FAIL);
		return false;
	}

	int a;
	for (a = 0; a < MAX_SAMPLES; a ++) {
		soundCache[a].sample = NULL;
		soundCache[a].fileLoaded = -1;
	}

	BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, 10000);
	BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, 10000);
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 10000);
	return soundOK = true;
}

void killSoundStuff() {
	if (soundOK) {
		int a;
		for (a = 0; a < MAX_MODS; a++)
			stopMOD(a);
		for (a = 0; a < MAX_SAMPLES; a++)
			freeSound(a);
		BASS_Free();
	}
}

bool playMOD(int f, int a, int fromTrack) {
	if (soundOK) {

		stopMOD(a);

		setResourceForFatal(f);
		uint32_t length = openFileFromNum(f);
		if (length == 0)
			return NULL;

		char *memImage;
		memImage = loadEntireFileToMemory(bigDataFile, length);
		if (!memImage)
			return fatal(ERROR_MUSIC_MEMORY_LOW);

		mod[a] = BASS_MusicLoad(true, memImage, 0, length, BASS_MUSIC_LOOP | BASS_MUSIC_RAMP/*|BASS_MUSIC_PRESCAN needed too if we're going to set the position in bytes*/, 0);
		delete memImage;

		if (!mod[a]) {

		} else {
			setMusicVolume(a, defVol);

			if (!BASS_ChannelPlay(mod[a], true))
				debugOut("playMOD: Error %d!\n", BASS_ErrorGetCode());

			BASS_ChannelSetPosition(mod[a], MAKELONG(fromTrack, 0), BASS_POS_MUSIC_ORDER);
			BASS_ChannelFlags(mod[a], BASS_SAMPLE_LOOP | BASS_MUSIC_RAMP, BASS_SAMPLE_LOOP | BASS_MUSIC_RAMP);
		}
		setResourceForFatal(-1);
	}
	return true;
}

void setMusicVolume(int a, int v) {
	int ret;
	if (soundOK && mod[a]) {
		ret = BASS_ChannelSetAttribute(mod[a], BASS_ATTRIB_VOL, (float)v / 256);
		if (!ret) {
			debugOut("setMusicVolume: Error %d\n", BASS_ErrorGetCode());
		}
	}
}

void setDefaultMusicVolume(int v) {
	defVol = v;
}

void setSoundVolume(int a, int v) {
	if (soundOK) {
		int ch = findInSoundCache(a);
		if (ch != -1) {
			if (BASS_ChannelIsActive(soundCache[ch].mostRecentChannel)) {
				BASS_ChannelSetAttribute(soundCache[ch].mostRecentChannel, BASS_ATTRIB_VOL, (float)v / 256);
			}
		}
	}
}

bool stillPlayingSound(int ch) {
	if (soundOK)
		if (ch != -1)
			if (soundCache[ch].fileLoaded != -1)
				if (BASS_ChannelIsActive(soundCache[ch].mostRecentChannel) != BASS_ACTIVE_STOPPED)
					return true;
	return false;
}

void setSoundLoop(int a, int s, int e) {
//	if (soundOK) {
//		int ch = findInSoundCache (a);
//		if (ch != -1) {
//			int en = FSOUND_Sample_GetLength (soundCache[ch].sample);
//			if (e < 1 || e >= en) e = en - 1;
//			if (s < 0 || s >= e) s = 0;
//
//			FSOUND_Sample_SetLoopPoints (soundCache[ch].sample, s, e);
//		}
//	}
}

void setDefaultSoundVolume(int v) {
	defSoundVol = v;
}

int emptySoundSlot = 0;

int findEmptySoundSlot() {
	int t;
	for (t = 0; t < MAX_SAMPLES; t++) {
		emptySoundSlot++;
		emptySoundSlot %= MAX_SAMPLES;
		if (!soundCache[emptySoundSlot].sample)
			return emptySoundSlot;
	}

	// Argh!They're all playing!Let's trash the oldest that's not looping...

	for (t = 0; t < MAX_SAMPLES; t++) {
		emptySoundSlot++;
		emptySoundSlot %= MAX_SAMPLES;
		if (!soundCache[emptySoundSlot].looping)
			return emptySoundSlot;
	}

	// Holy crap, they're all looping!What's this twat playing at?

	emptySoundSlot++;
	emptySoundSlot %= MAX_SAMPLES;
	return emptySoundSlot;
}

int guessSoundFree = 0;

/*
 void soundWarning (char * t, int i) {
 FILE * u = fopen ("soundlog.txt", "at");
 fprintf (u, "%s: %i\n", t, i);
 fclose (u);
 }
 */

bool forceRemoveSound() {
	for (int a = 0; a < 8; a++) {
		if (soundCache[a].fileLoaded != -1 && !stillPlayingSound(a)) {
//			soundWarning ("Deleting silent sound", a);
			freeSound(a);
			return 1;
		}
	}

	for (int a = 0; a < 8; a++) {
		if (soundCache[a].fileLoaded != -1) {
//			soundWarning ("Deleting playing sound", a);
			freeSound(a);
			return 1;
		}
	}
//	soundWarning ("Cache is empty!", 0);
	return 0;
}

int cacheSound(int f) {
	setResourceForFatal(f);

	if (!soundOK)
		return 0;

	int a = findInSoundCache(f);
	if (a != -1)
		return a;
	if (f == -2)
		return -1;
	a = findEmptySoundSlot();
	freeSound(a);

	uint32_t length = openFileFromNum(f);
	if (!length)
		return -1;

	char *memImage;

	bool tryAgain = true;

	while (tryAgain) {
		memImage = loadEntireFileToMemory(bigDataFile, length);
		tryAgain = memImage == NULL;
		if (tryAgain) {
			if (!forceRemoveSound()) {
				fatal(ERROR_SOUND_MEMORY_LOW);
				return -1;
			}
		}
	}

	for (;;) {
//		soundWarning ("  Trying to load sound into slot", a);
		soundCache[a].sample = BASS_SampleLoad(true, memImage, 0, length, 65535, 0);

		if (soundCache[a].sample) {
			soundCache[a].fileLoaded = f;
			delete memImage;
			setResourceForFatal(-1);
			return a;
		}

		warning(ERROR_SOUND_ODDNESS);
		soundCache[a].sample = NULL;
		soundCache[a].fileLoaded = -1;
		soundCache[a].looping = false;
		return -1;
	}
}

bool startSound(int f, bool loopy) {
	if (soundOK) {
		int a = cacheSound(f);
		if (a == -1)
			return false;

		soundCache[a].looping = loopy;
		soundCache[a].vol = defSoundVol;

		soundCache[a].mostRecentChannel = BASS_SampleGetChannel(soundCache[a].sample, false);
		if (soundCache[a].mostRecentChannel) {
			BASS_ChannelPlay(soundCache[a].mostRecentChannel, true);
			BASS_ChannelSetAttribute(soundCache[a].mostRecentChannel, BASS_ATTRIB_VOL, defSoundVol);
			if (loopy) {
				BASS_ChannelFlags(soundCache[a].mostRecentChannel, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP); // set LOOP flag
			}
		}

	}
	return true;
}

/*
 void debugSounds () {
 FILE * fp = fopen ("newdebug.txt", "at");
 if (fp) {
 for (int aa = 0; aa < 32; aa ++) {
 if (aa == EFFECT_CHANNELS) fprintf (fp, "|");
 fprintf (fp, FSOUND_IsPlaying (aa) ? "#" : ".");
 }
 fprintf (fp, "\n");
 fclose (fp);
 }
 }
 // */

void saveSounds(Common::WriteStream *stream) {
	if (soundOK) {
		for (int i = 0; i < MAX_SAMPLES; i++) {
			if (soundCache[i].looping) {
				stream->writeByte(1);
				stream->writeUint16BE(soundCache[i].fileLoaded);
				stream->writeUint16BE(soundCache[i].vol);
			}
		}
	}
	stream->writeByte(0);
	stream->writeUint16BE(defSoundVol);
	stream->writeUint16BE(defVol);
}

void loadSounds(Common::SeekableReadStream *stream) {
	for (int i = 0; i < MAX_SAMPLES; i++)
		freeSound(i);

	while (stream->readByte()) {
		int fileLoaded = stream->readUint16BE();
		defSoundVol = stream->readUint16BE();
		startSound(fileLoaded, 1);
	}

	defSoundVol = stream->readUint16BE();
	defVol = stream->readUint16BE();
}

bool getSoundCacheStack(stackHandler *sH) {
	variable newFileHandle;
	newFileHandle.varType = SVT_NULL;

	for (int a = 0; a < MAX_SAMPLES; a++) {
		if (soundCache[a].fileLoaded != -1) {
			setVariable(newFileHandle, SVT_FILE, soundCache[a].fileLoaded);
			if (!addVarToStackQuick(newFileHandle, sH->first))
				return false;
			if (sH->last == NULL)
				sH->last = sH->first;
		}
	}
	return true;
}

} // End of namespace Sludge
