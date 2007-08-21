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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "sound/audiocd.h"

#include "gob/gob.h"
#include "gob/cdrom.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/game.h"

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

	strncpy0(tmp, fname, 79);

	handle = _vm->_dataIO->openData(tmp);

	if (handle == -1)
		return;

	_vm->_dataIO->closeData(handle);

	_vm->_dataIO->getUnpackedData(tmp);

	handle = _vm->_dataIO->openData(tmp);
	DataStream *stream = _vm->_dataIO->openAsStream(handle, true);

	version = stream->readUint16LE();
	startChunk = stream->readUint16LE();
	_numTracks = stream->readUint16LE();

	if (version != 3)
		error("%s: Unknown version %d", fname, version);

	stream->seek(50);

	for (int i = 0; i < startChunk; i++) {
		pos = stream->readUint16LE();

		if (!pos)
			break;

		stream->skip(pos);
	}

	_LICbuffer = new byte[_numTracks * 22];
	stream->read(_LICbuffer, _numTracks * 22);

	delete stream;
}

void CDROM::freeLICbuffer() {
	delete[] _LICbuffer;
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
		if (!scumm_stricmp(_vm->_game->_curTotFile, tracks[i][0])) {
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

	// Default to "ang?" for other languages (including EN_USA)
	int language = _vm->_global->_language <= 4 ? _vm->_global->_language : 2;
	for (int i = 0; i < ARRAYSIZE(tracks); i++)
		if (!scumm_stricmp(_vm->_game->_curTotFile, tracks[i][0])) {
			startTrack(tracks[i][language + 1]);
			break;
		}
}

void CDROM::startTrack(const char *trackName) {
	if (!_LICbuffer)
		return;

	debugC(1, kDebugMusic, "CDROM::startTrack(%s)", trackName);

	byte *matchPtr = getTrackBuffer(trackName);
	if (!matchPtr) {
		warning("Track \"%s\" not found", trackName);
		return;
	}

	strncpy0(_curTrack, trackName, 15);

	stopPlaying();
	_curTrackBuffer = matchPtr;

	while (getTrackPos() >= 0);

	uint32 start = READ_LE_UINT32(matchPtr + 12);
	uint32 end   = READ_LE_UINT32(matchPtr + 16);

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
	debugC(1, kDebugMusic, "CDROM::play(%d, %d)", from, to);

	AudioCD.play(1, 1, from, to - from + 1);
	_cdPlaying = true;
}

int32 CDROM::getTrackPos(const char *keyTrack) {
	byte *keyBuffer = getTrackBuffer(keyTrack);
	uint32 curPos = (_vm->_util->getTimeKey() - _startTime) * 3 / 40;

	if (_cdPlaying && (_vm->_util->getTimeKey() < _trackStop)) {
		if (keyBuffer && _curTrackBuffer && (keyBuffer != _curTrackBuffer)) {
			uint32 kStart = READ_LE_UINT32(keyBuffer + 12);
			uint32 kEnd = READ_LE_UINT32(keyBuffer + 16);
			uint32 cStart = READ_LE_UINT32(_curTrackBuffer + 12);
			uint32 cEnd = READ_LE_UINT32(_curTrackBuffer + 16);

			if ((kStart >= cStart) && (kEnd <= cEnd)) {
				if ((kStart - cStart) > curPos)
					return -2;
				if ((kEnd - cStart) < curPos)
					return -1;
			}
		}

		return curPos;
	} else
		return -1;
}

const char *CDROM::getCurTrack() {
	return _curTrack;
}

void CDROM::stopPlaying() {
	stop();

	while (getTrackPos() != -1);
}

void CDROM::stop() {
	debugC(1, kDebugMusic, "CDROM::stop()");

	_curTrackBuffer = 0;
	AudioCD.stop();
	_cdPlaying = false;
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

byte *CDROM::getTrackBuffer(const char *trackName) {
	if (!_LICbuffer || !trackName)
		return 0;

	byte *matchPtr = 0;
	byte *curPtr = _LICbuffer;

	for (int i = 0; i < _numTracks; i++) {
		if (!scumm_stricmp((char *) curPtr, trackName)) {
			matchPtr = curPtr;
			break;
		}
		curPtr += 22;
	}

	return matchPtr;
}

} // End of namespace Gob
