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

void IMuseDigital::setDigMusicState(int stateId) {
	int l, num = -1;

	for (l = 0; _digStateMusicTable[l].soundId != -1; l++) {
		if ((_digStateMusicTable[l].soundId == stateId)) {
			debug(5, "Set music state: %s, %s, %s", _digStateMusicTable[l].name, _digStateMusicTable[l].title, _digStateMusicTable[l].filename);
			num = l;
			break;
		}
	}

	if (num == -1) {
		for (l = 0; _digStateMusicMap[l].roomId != -1; l++) {
			if ((_digStateMusicMap[l].roomId == stateId)) {
				break;
			}
		}
		num = l;

		int val2 = _attributesTable[_digStateMusicMap[num].unk1];
		if (val2 == 0) {
			if (_attributesTable[_digStateMusicMap[num].unk3] != 0) {
				num = _digStateMusicMap[num].unk4;
			} else {
				num = _digStateMusicMap[num].musicTableIndex;
			}
		} else {
			int val = _attributesTable[_digStateMusicMap[num].unk2];
			if (val == 0) {
				num = _digStateMusicMap[num].musicTableIndex + val2;
			} else {
				num = val;
			}
		}
	}

	debug(5, "Set music state: %s, %s, %s", _digStateMusicTable[num].name, _digStateMusicTable[num].title, _digStateMusicTable[num].filename);

	if (_curMusicState == num)
		return;

	if (_curMusicSeq == 0) {
		if (num == 0)
			playDigMusic(NULL, &_digStateMusicTable[num], num, false);
		else
			playDigMusic(_digStateMusicTable[num].name, &_digStateMusicTable[num], num, false);
	}

	_curMusicState = num;
}

void IMuseDigital::setDigMusicSequence(int seqId) {
	int l, num = -1;

	if (seqId == 0)
		seqId = 2000;

	for (l = 0; _digSeqMusicTable[l].soundId != -1; l++) {
		if ((_digSeqMusicTable[l].soundId == seqId)) {
			debug(5, "Set music sequence: %s, %s, %s", _digSeqMusicTable[l].name, _digSeqMusicTable[l].title, _digSeqMusicTable[l].filename);
			num = l;
			break;
		}
	}
	assert(num != -1);

	if (_curMusicSeq == num)
		return;

	if (num != 0) {
		if (_curMusicSeq == 0) {
			playDigMusic(_digSeqMusicTable[num].name, &_digSeqMusicTable[num], 0, true);
			_curSeqAtribPos = 0;
			_attributesSeq[num] = 1;
		} else {
			if ((_digSeqMusicTable[_curMusicSeq].opcode == 4) && (_digSeqMusicTable[_curMusicSeq].opcode == 6)) {
				_curSeqAtribPos = num;
				return;
			} else if (_digSeqMusicTable[_curMusicSeq].opcode == 6) {
				playDigMusic(_digSeqMusicTable[num].name, &_digSeqMusicTable[num], 0, true);
				_curSeqAtribPos = 0;
				_attributesSeq[num] = 1;
			}
		}
	} else {
		if (_curSeqAtribPos != 0) {
			playDigMusic(_digSeqMusicTable[_curSeqAtribPos].name, &_digSeqMusicTable[num], 0, true);
			_attributesSeq[_curSeqAtribPos] = 1;
			_curSeqAtribPos = 0;
			num = _curSeqAtribPos;
		} else {
			if (_curMusicState != 0)
				playDigMusic(_digStateMusicTable[_curMusicState].name, &_digSeqMusicTable[num], _curMusicState, false);
			else
				playDigMusic(NULL, &_digStateMusicTable[num], _curMusicState, false);
			num = 0;
		}
	}

	_curMusicSeq = num;
}

void IMuseDigital::playDigMusic(const char *songName, const imuseDigTable *table, int atribPos, bool sequence) {
	int hookId = 0;

	if (songName != NULL) {
		if ((_attributesSeq[38] != 0) && (_attributesSeq[41] == _attributesSeq[38])) {
			if ((atribPos == 43) || (atribPos == 44))
				hookId = 3;
		}

		if ((_attributesSeq[46] != 0) && (_attributesSeq[48] == 0)) {
			if ((atribPos == 38) || (atribPos == 39))
				hookId = 3;
		}

		if ((_attributesSeq[53] != 0)) {
			if ((atribPos == 50) || (atribPos == 51))
				hookId = 3;
		}

		if ((atribPos != 0) && (hookId == 0)) {
			if (table->param != 0)
				atribPos = table->param;
			hookId = _attributesSeq[atribPos];
		}
	}

	if (table->filename[0] == 0) {
		fadeOutMusic(120);
		return;
	}

	fadeOutMusic(120);

	switch(table->opcode) {
		case 0:
		case 5:
		case 6:
			break;
		case 3:
			if ((!sequence) && (table->param != 0)) {
				if (table->param == _digStateMusicTable[_curMusicState].param) {
					startMusic(table->filename, table->soundId, sequence, 0, 127);
				} 
			} else {
				startMusic(table->filename, table->soundId, sequence, hookId, 127);
			}
		case 4:
			startMusic(table->filename, table->soundId, sequence, 0, 127);
			break;
	}
}

void IMuseDigital::setComiMusicState(int stateId) {
	int l, num = -1;

	if ((stateId == 0) || (stateId == 4))
		stateId = 1000;

	for (l = 0; _comiStateMusicTable[l].soundId != -1; l++) {
		if ((_comiStateMusicTable[l].soundId == stateId)) {
			debug(5, "Set music state: %s, %s, %s", _comiStateMusicTable[l].name, _comiStateMusicTable[l].title, _comiStateMusicTable[l].filename);
			num = l;
			break;
		}
	}
	assert(num != -1);

	if (_curMusicState == num)
		return;

	if (_curMusicSeq == 0) {
		if (num == 0)
			playComiMusic(NULL, &_comiStateMusicTable[num], num, false);
		else
			playComiMusic(_comiStateMusicTable[num].name, &_comiStateMusicTable[num], num, false);
	}

	_curMusicState = num;
}

void IMuseDigital::setComiMusicSequence(int seqId) {
	int l, num = -1;

	if (seqId == 0)
		seqId = 2000;

	for (l = 0; _comiSeqMusicTable[l].soundId != -1; l++) {
		if ((_comiSeqMusicTable[l].soundId == seqId)) {
			debug(5, "Set music sequence: %s, %s, %s", _comiSeqMusicTable[l].name, _comiSeqMusicTable[l].title, _comiSeqMusicTable[l].filename);
			num = l;
			break;
		}
	}
	assert(num != -1);

	if (_curMusicSeq == num)
		return;

	if (num != 0) {
		if (_curMusicSeq == 0) {
			playComiMusic(_comiSeqMusicTable[num].name, &_comiSeqMusicTable[num], 0, true);
			_curSeqAtribPos = 0;
			_attributesSeq[num] = 1;
		} else {
			if ((_comiSeqMusicTable[_curMusicSeq].opcode == 4) && (_comiSeqMusicTable[_curMusicSeq].opcode == 6)) {
				_curSeqAtribPos = num;
				return;
			} else if (_comiSeqMusicTable[_curMusicSeq].opcode == 6) {
				playComiMusic(_comiSeqMusicTable[num].name, &_comiSeqMusicTable[num], 0, true);
				_curSeqAtribPos = 0;
				_attributesSeq[num] = 1;
			}
		}
	} else {
		if (_curSeqAtribPos != 0) {
			playComiMusic(_comiSeqMusicTable[_curSeqAtribPos].name, &_comiSeqMusicTable[num], 0, true);
			_attributesSeq[_curSeqAtribPos] = 1;
			_curSeqAtribPos = 0;
			num = _curSeqAtribPos;
		} else {
			if (_curMusicState != 0)
				playComiMusic(_comiStateMusicTable[_curMusicState].name, &_comiSeqMusicTable[num], _curMusicState, false);
			else
				playComiMusic(NULL, &_comiStateMusicTable[num], _curMusicState, false);
			num = 0;
		}
	}

	_curMusicSeq = num;
}

void IMuseDigital::playComiMusic(const char *songName, const imuseComiTable *table, int atribPos, bool sequence) {
	int hookId = 0;

	if ((songName != NULL) && (atribPos != 0)) {
		hookId = _attributesSeq[atribPos];
		if (table->hookId != 0) {
			if ((hookId != 0) && (table->hookId <= 1)) {
				_attributesSeq[atribPos] = hookId + 1;
				if (table->hookId < hookId + 1)
					_attributesSeq[atribPos] = 1;
			} else {
				_attributesSeq[atribPos] = 2;
			}
		}
	}

	if (table->filename[0] == 0) {
		fadeOutMusic(120);
		return;
	}

	switch(table->opcode) {
		case 0:
		case 8:
		case 9:
			fadeOutMusic(120);
			break;
		case 1:
			fadeOutMusic(120);
			startMusic(table->filename, table->soundId, sequence, 0, 1);
			setFade(table->soundId, 127, 120);
			break;
		case 2:
			{
				int fadeDelay = table->fadeDelay;
				if (fadeDelay == 0)
					fadeDelay = 1000;
				fadeOutMusic(fadeDelay);
				startMusic(table->filename, table->soundId, sequence, table->hookId, 127);
			}
			break;
		case 3:
			if ((!sequence) && (table->param != 0)) {
				if (table->param == _comiStateMusicTable[_curMusicState].param) {
					int fadeDelay = table->fadeDelay;
					if (fadeDelay == 0)
						fadeDelay = 1000;
					fadeOutMusic(fadeDelay);
					startMusic(table->filename, table->soundId, sequence, 0, 127);
				} 
			} else {
				int fadeDelay = table->fadeDelay;
				if (fadeDelay == 0)
					fadeDelay = 1000;
				fadeOutMusic(fadeDelay);
				startMusic(table->filename, table->soundId, sequence, table->hookId, 127);
			}
		case 4:
			fadeOutMusic(120);
			startMusic(table->filename, table->soundId, sequence, 0, 127);
			break;
		case 12:
			{
				int fadeDelay = table->fadeDelay;
				if (fadeDelay == 0)
					fadeDelay = 1000;
				fadeOutMusic(fadeDelay);
				startMusic(table->filename, table->soundId, sequence, table->hookId, 127);
			}
			break;
	}
}

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
		playFtMusic(NULL, 0, 0, false);
	else
		playFtMusic(_ftStateMusicTable[stateId].audioName, _ftStateMusicTable[stateId].opcode, _ftStateMusicTable[stateId].volume, false);

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
			playFtMusic(NULL, 0, 0, true);
		else
			playFtMusic(_ftStateMusicTable[seqId].audioName, _ftStateMusicTable[seqId].opcode, _ftStateMusicTable[seqId].volume, true);
	} else {
		int seq = (seqId - 1) * 4;
		playFtMusic(_ftSeqMusicTable[seq].audioName, _ftSeqMusicTable[seq].opcode, _ftSeqMusicTable[seq].volume, true);
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
		playFtMusic(NULL, 0, 0, true);
	else {
		int seq = ((_curMusicSeq - 1) + cueId) * 4;
		playFtMusic(_ftSeqMusicTable[seq].audioName, _ftSeqMusicTable[seq].opcode, _ftSeqMusicTable[seq].volume, true);
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

void IMuseDigital::playFtMusic(const char *songName, int opcode, int volume, bool sequence) {
	fadeOutMusic(200);

	switch(opcode) {
		case 0:
		case 4:
			break;
		case 1:
		case 2:
		case 3:
			{
				int soundId = getSoundIdByName(songName);
				if (soundId != -1)
					startMusic(soundId, sequence, volume);
			}
			break;
	}
}


} // End of namespace Scumm
