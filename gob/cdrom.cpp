/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/cdrom.h"
#include "gob/dataio.h"
#include "gob/game.h"
#include "gob/global.h"
#include "gob/util.h"
#include "sound/audiocd.h"

namespace Gob {

byte *cd_LICbuffer;
char cd_curTrack[16];
uint16 cd_numTracks;
bool cd_globFlag;
uint32 cd_trackStop;
uint32 cd_startTime;

void cd_readLIC(const char *fname) {
	char tmp[80];
	int handle;
	uint16 version, startChunk, pos;

	cd_freeLICbuffer();

	*cd_curTrack = 0;

	strcpy(tmp, fname);

	handle = data_openData(tmp);

	if (handle == -1)
		return;

	data_closeData(handle);

	data_getUnpackedData(tmp);

	handle = data_openData(tmp);

	data_readData(handle, (char *)&version, 2);
	version = READ_LE_UINT16(&version);

	data_readData(handle, (char *)&startChunk, 2);
	startChunk = READ_LE_UINT16(&startChunk);

	data_readData(handle, (char *)&cd_numTracks, 2);
	cd_numTracks = READ_LE_UINT16(&cd_numTracks);

	if (version != 3) {
		error("Wrong file %s (%d)", fname, version);
		return;
	}

	data_seekData(handle, 50, SEEK_SET);

	for (int i = 0; i < startChunk; i++) {
		data_readData(handle, (char *)&pos, 2);
		pos = READ_LE_UINT16(&pos);

		if (!pos)
			break;

		data_seekData(handle, pos, SEEK_CUR);
	}

	cd_LICbuffer = (byte *)malloc(cd_numTracks * 22);
	data_readData(handle, (char *)cd_LICbuffer, cd_numTracks * 22);

	data_closeData(handle);
}

void cd_freeLICbuffer(void) {
	free(cd_LICbuffer);
	cd_LICbuffer = 0;
}

void cd_playBgMusic() {
	static const char *tracks[][2] = {
		{"avt00.tot",  "mine"},
		{"avt001.tot", "nuit"},
		{"avt002.tot", "campagne"},
		{"avt003.tot", "extsor1"},
		{"avt004.tot", "interieure"},
		{"avt005.tot", "zombie"},
		{"avt006.tot", "zombie"},
		{"avt007.tot", "campagne"},
		{"avt008.tot", "campagne"},
		{"avt009.tot", "extsor1"},
		{"avt010.tot", "extsor1"},
		{"avt011.tot", "interieure"},
		{"avt012.tot", "zombie"},
		{"avt014.tot", "nuit"},
		{"avt015.tot", "interieure"},
		{"avt016.tot", "statue"},
		{"avt017.tot", "zombie"},
		{"avt018.tot", "statue"},
		{"avt019.tot", "mine"},
		{"avt020.tot", "statue"},
		{"avt021.tot", "mine"},
		{"avt022.tot", "zombie"}
	};

	for (int i = 0; i < ARRAYSIZE(tracks); i++)
		if (!scumm_stricmp(game_curTotFile, tracks[i][0])) {
			cd_startTrack(tracks[i][1]);
			break;
		}
}

void cd_playMultMusic() {
	static const char *tracks[][6] = {
		{"avt005.tot", "fra1", "all1", "ang1", "esp1", "ita1"},
		{"avt006.tot", "fra2", "all2", "ang2", "esp2", "ita2"},
		{"avt012.tot", "fra3", "all3", "ang3", "esp3", "ita3"},
		{"avt016.tot", "fra4", "all4", "ang4", "esp4", "ita4"},
		{"avt019.tot", "fra5", "all5", "ang5", "esp5", "ita5"},
		{"avt022.tot", "fra6", "all6", "ang6", "esp6", "ita6"}
	};

	for (int i = 0; i < ARRAYSIZE(tracks); i++)
		if (!scumm_stricmp(game_curTotFile, tracks[i][0])) {
			cd_globFlag = true;
			cd_startTrack(tracks[i][language + 1]);
			break;
		}
}

void cd_startTrack(const char *trackname) {
	byte *curPtr, *matchPtr;

	if (!cd_LICbuffer)
		return;

	debug(3, "cd_startTrack(%s)", trackname);

	matchPtr = 0;
	curPtr = cd_LICbuffer;

	for (int i = 0; i < cd_numTracks; i++) {
		if (!scumm_stricmp((char *)curPtr, trackname)) {
			matchPtr = curPtr;
			break;
		}
		curPtr += 22;
	}

	if (!matchPtr) {
		error("Track %s not found", trackname);
		return;
	}

	strcpy(cd_curTrack, trackname);

	cd_stopPlaying();

	while (cd_getTrackPos() != -1);

	uint32 start, end;

	start = READ_LE_UINT32(matchPtr + 12);
	end   = READ_LE_UINT32(matchPtr + 16);

	cd_play(start, end);

	cd_startTime = util_getTimeKey();
	cd_trackStop = cd_startTime + (end - start + 1 + 150) * 40 / 3;
}

void cd_play(uint32 from, uint32 to) {
	// play from sector [from] to sector [to]
	//
	// format is HSG:
	// HSG encodes frame information into a double word:
	// minute multiplied by 4500, plus second multiplied by 75,
	// plus frame, minus 150
	debug(3, "cd_play(%d, %d)", from, to);

	AudioCD.play(1, 0, from, to - from + 1);
}

int32 cd_getTrackPos(void) {
	uint32 curPos = util_getTimeKey() - cd_startTime;

	if (AudioCD.isPlaying() && (util_getTimeKey() < cd_trackStop))
		return curPos * 3 / 40;
	else
		return -1;
}

void cd_stopPlaying(void) {
	cd_stop();

	while (cd_getTrackPos() != -1);
}

void cd_stop(void) {
	debug(3, "cd_stop()");

	AudioCD.stop();
}

void cd_testCD(int trySubst, const char *label) {
	if (!trySubst) {
		error("CDROM track substitution is not supported");
		return;
	}

	cd_LICbuffer = 0;
	cd_globFlag = false;

	// Original checked CD label here
	// but will skip it as it will require OSystem extensions of direct
	// CD secor reading
}

} // End of namespace Gob
