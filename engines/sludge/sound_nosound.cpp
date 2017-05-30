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
#include "allfiles.h"
#include "newfatal.h"
#include "sound.h"
#include "moreio.h"
#include "fileset.h"

namespace Sludge {

bool soundOK = false;

int defVol = 128;
int defSoundVol = 255;

#if 0
char *loadEntireFileToMemory(FILE *inputFile, uint32_t size) {
	char *allData = new char[size];
	if (! allData) return NULL;
	fread(allData, size, 1, inputFile);
	finishAccess();

	return allData;
}
#endif

int findInSoundCache(int a) {
//#pragma unused(a)
	return -1;
}

void stopMOD(int i) {
//#pragma unused(i)
}

void huntKillSound(int filenum) {
//#pragma unused(filenum)
}

void huntKillFreeSound(int filenum) {
//#pragma unused(filenum)
}

bool initSoundStuff(HWND hwnd) {
//	#pragma unused(hwnd)
	return false;
}

void killSoundStuff() {
}

bool playMOD(int f, int a, int fromTrack) {
//#pragma unused (f,a,fromTrack)
	return true;
}

void setMusicVolume(int a, int v) {
//#pragma unused (a,v)
}

void setDefaultMusicVolume(int v) {
	defVol = v;
}

void setSoundVolume(int a, int v) {
//#pragma unused (a,v)
}

bool stillPlayingSound(int ch) {
//#pragma unused (ch)
	return false;
}

void setSoundLoop(int a, int s, int e) {
//#pragma unused (a,s,e)
}

void setDefaultSoundVolume(int v) {
	defSoundVol = v;
}

bool forceRemoveSound() {
	return 0;
}

int cacheSound(int f) {
//#pragma unused (f)
	return 0;
}

bool startSound(int f, bool loopy) {
//#pragma unused (f,loopy)
	return true;
}

void saveSounds(Common::WriteStream *stream) {
	stream->writeByte(0);
	stream->writeUint16BE(defSoundVol);
	stream->writeUint16BE(defVol);
}

void loadSounds(Common::SeekableReadStream *stream) {
	while (stream->readByte()) {
		stream->readUint16BE();
		stream->readUint16BE();
	}

	defSoundVol = stream->readUint16BE();
	defVol = stream->readUint16BE();
}

bool getSoundCacheStack(stackHandler *sH) {
//#pragma unused (sH)
	return true;
}

} // End of namespace Sludge
