/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "common/scummsys.h"
#include "scumm/scumm.h"
#include "scumm/imuse_digi/dimuse.h"

namespace Scumm {

void IMuseDigital::setFtMusicState(int stateId) {
	if (stateId > 48)
		return;

	debug(5, "State music: %s, %s", _ftStateMusicTable[stateId].name, _ftStateMusicTable[stateId].audioName);

	if (_curMusicState == stateId)
		return;

	if (_curMusicSeq != 0) {
		_curMusicState = stateId;
		return;
	}

	if (stateId == 0)
		playFtMusic(NULL, 0, 0);
	else
		playFtMusic(_ftStateMusicTable[stateId].audioName, _ftStateMusicTable[stateId].opcode, _ftStateMusicTable[stateId].volume);

	_curMusicState = stateId;
}

void IMuseDigital::setFtMusicSequence(int seqId) {
	if (seqId > 52)
		return;

	debug(5, "Sequence music: %s, %s", _ftSeqNames[seqId].name);

	if (_curMusicSeq == seqId)
		return;

	if (seqId == 0) {
		if (_curMusicState == 0)
			playFtMusic(NULL, 0, 0);
		else
			playFtMusic(_ftStateMusicTable[seqId].audioName, _ftStateMusicTable[seqId].opcode, _ftStateMusicTable[seqId].volume);
	} else {
		int seq = (seqId - 1) * 4;
		playFtMusic(_ftSeqMusicTable[seq].audioName, _ftSeqMusicTable[seq].opcode, _ftSeqMusicTable[seq].volume);
	}

	_curMusicSeq = seqId;
	_curMusicCue = 0;
}

void IMuseDigital::setFtMusicCuePoint(int cueId) {
	if (cueId > 3)
		return;

	debug(5, "Cue point sequence: %d", cueId);

	if (_curMusicSeq == 0)
		return;

	if (_curMusicCue == cueId)
		return;

	if (cueId == 0)
		playFtMusic(NULL, 0, 0);
	else {
		int seq = ((_curMusicSeq - 1) + cueId) * 4;
		playFtMusic(_ftSeqMusicTable[seq].audioName, _ftSeqMusicTable[seq].opcode, _ftSeqMusicTable[seq].volume);
	}

	_curMusicCue = cueId;
}

int IMuseDigital::getSoundIdByName(const char *soundName) {
	if (soundName && soundName[0] != 0) {
		for (int r = 0; r < _vm->_numAudioNames; r++) {
			if (strcmp(soundName, &_vm->_audioNames[r * 9]) == 0) {
				return r;
			}
		}
	}

	return -1;
}

void IMuseDigital::fadeOutMusic() {
	debug(5, "IMuseDigital::fadeOutMusic");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		_track[l].locked = true;
		if (_track[l].used) {
			parseScriptCmds(14, _track[l].soundId, 0x600, 0, 200, 0, 0, 0);
		}
		_track[l].locked = false;
	}
}

void IMuseDigital::playFtMusic(const char *songName, int opcode, int volume) {
	fadeOutMusic();

	switch(opcode) {
		case 0:
		case 4:
			break;
		case 1:
		case 2:
		case 3:
			{
			int soundId = getSoundIdByName(songName);
			if (soundId != -1) {
				startMusic(soundId);
				parseScriptCmds(12, soundId, 0x600, volume, 0, 0, 0, 0);
			}
			}
			break;
	}
}


} // End of namespace Scumm
