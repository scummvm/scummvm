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

CDROM::CDROM(GobEngine *vm) : _vm(vm) {
	_cdPlaying = false;
	_LICbuffer = 0;
	for (int i = 0; i < 16; i++)
		_curTrack[i] = 0;
	_numTracks = 0;
	_trackStop = 0;
	_startTime = 0;
}

void CDROM::readLIC(const char *fname) {
	char tmp[80];
	int handle;
	uint16 version, startChunk, pos;

	freeLICbuffer();

	*_curTrack = 0;

	strcpy(tmp, fname);

	handle = _vm->_dataio->openData(tmp);

	if (handle == -1)
		return;

	_vm->_dataio->closeData(handle);

	_vm->_dataio->getUnpackedData(tmp);

	handle = _vm->_dataio->openData(tmp);

	_vm->_dataio->readData(handle, (char *)&version, 2);
	version = READ_LE_UINT16(&version);

	_vm->_dataio->readData(handle, (char *)&startChunk, 2);
	startChunk = READ_LE_UINT16(&startChunk);

	_vm->_dataio->readData(handle, (char *)&_numTracks, 2);
	_numTracks = READ_LE_UINT16(&_numTracks);

	if (version != 3) {
		error("Wrong file %s (%d)", fname, version);
		return;
	}

	_vm->_dataio->seekData(handle, 50, SEEK_SET);

	for (int i = 0; i < startChunk; i++) {
		_vm->_dataio->readData(handle, (char *)&pos, 2);
		pos = READ_LE_UINT16(&pos);

		if (!pos)
			break;

		_vm->_dataio->seekData(handle, pos, SEEK_CUR);
	}

	_LICbuffer = (byte *)malloc(_numTracks * 22);
	_vm->_dataio->readData(handle, (char *)_LICbuffer, _numTracks * 22);

	_vm->_dataio->closeData(handle);
}

void CDROM::freeLICbuffer(void) {
	free(_LICbuffer);
	_LICbuffer = 0;
}

void CDROM::playBgMusic() {
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
		if (!scumm_stricmp(_vm->_game->curTotFile, tracks[i][0])) {
			startTrack(tracks[i][1]);
			break;
		}
}

void CDROM::playMultMusic() {
	static const char *tracks[][6] = {
		{"avt005.tot", "fra1", "all1", "ang1", "esp1", "ita1"},
		{"avt006.tot", "fra2", "all2", "ang2", "esp2", "ita2"},
		{"avt012.tot", "fra3", "all3", "ang3", "esp3", "ita3"},
		{"avt016.tot", "fra4", "all4", "ang4", "esp4", "ita4"},
		{"avt019.tot", "fra5", "all5", "ang5", "esp5", "ita5"},
		{"avt022.tot", "fra6", "all6", "ang6", "esp6", "ita6"}
	};

	for (int i = 0; i < ARRAYSIZE(tracks); i++)
		if (!scumm_stricmp(_vm->_game->curTotFile, tracks[i][0])) {
			_cdPlaying = true;
			startTrack(tracks[i][_vm->_global->_language + 1]);
			break;
		}
}

void CDROM::startTrack(const char *trackname) {
	byte *curPtr, *matchPtr;

	if (!_LICbuffer)
		return;

	debug(3, "startTrack(%s)", trackname);

	matchPtr = 0;
	curPtr = _LICbuffer;

	for (int i = 0; i < _numTracks; i++) {
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

	strcpy(_curTrack, trackname);

	stopPlaying();

	while (getTrackPos() != -1);

	uint32 start, end;

	start = READ_LE_UINT32(matchPtr + 12);
	end   = READ_LE_UINT32(matchPtr + 16);

	play(start, end);

	_startTime = _vm->_util->getTimeKey();
	_trackStop = _startTime + (end - start + 1 + 150) * 40 / 3;
}

void CDROM::play(uint32 from, uint32 to) {
	// play from sector [from] to sector [to]
	//
	// format is HSG:
	// HSG encodes frame information into a double word:
	// minute multiplied by 4500, plus second multiplied by 75,
	// plus frame, minus 150
	debug(3, "play(%d, %d)", from, to);

	AudioCD.play(1, 0, from, to - from + 1);
}

int32 CDROM::getTrackPos(void) {
	uint32 curPos = _vm->_util->getTimeKey() - _startTime;

	if (AudioCD.isPlaying() && (_vm->_util->getTimeKey() < _trackStop))
		return curPos * 3 / 40;
	else
		return -1;
}

void CDROM::stopPlaying(void) {
	stop();

	while (getTrackPos() != -1);
}

void CDROM::stop(void) {
	debug(3, "stop()");

	AudioCD.stop();
}

void CDROM::testCD(int trySubst, const char *label) {
	if (!trySubst) {
		error("CDROM track substitution is not supported");
		return;
	}

	_LICbuffer = 0;
	_cdPlaying = false;

	// Original checked CD label here
	// but will skip it as it will require OSystem extensions of direct
	// CD secor reading
}

} // End of namespace Gob
