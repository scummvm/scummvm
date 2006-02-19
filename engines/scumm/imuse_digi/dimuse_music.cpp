/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "scumm/scumm.h"
#include "scumm/imuse_digi/dimuse.h"

namespace Scumm {

#define DIG_STATE_OFFSET 11
#define DIG_SEQ_OFFSET (DIG_STATE_OFFSET + 65)
#define COMI_STATE_OFFSET 3

void IMuseDigital::setDigMusicState(int stateId) {
	int l, num = -1;

	for (l = 0; _digStateMusicTable[l].soundId != -1; l++) {
		if ((_digStateMusicTable[l].soundId == stateId)) {
			debug(5, "Set music state: %s, %s", _digStateMusicTable[l].name, _digStateMusicTable[l].filename);
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

		int offset = _attributes[_digStateMusicMap[num].offset];
		if (offset == 0) {
			if (_attributes[_digStateMusicMap[num].atribPos] != 0) {
				num = _digStateMusicMap[num].stateIndex3;
			} else {
				num = _digStateMusicMap[num].stateIndex1;
			}
		} else {
			int stateIndex2 = _digStateMusicMap[num].stateIndex2;
			if (stateIndex2 == 0) {
				num = _digStateMusicMap[num].stateIndex1 + offset;
			} else {
				num = stateIndex2;
			}
		}
	}

	debug(5, "Set music state: %s, %s", _digStateMusicTable[num].name, _digStateMusicTable[num].filename);

	if (_curMusicState == num)
		return;

	if (_curMusicSeq == 0) {
		if (num == 0)
			playDigMusic(NULL, &_digStateMusicTable[0], num, false);
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
			debug(5, "Set music sequence: %s, %s", _digSeqMusicTable[l].name, _digSeqMusicTable[l].filename);
			num = l;
			break;
		}
	}

	if (num == -1)
		return;

	if (_curMusicSeq == num)
		return;

	if (num != 0) {
		if (_curMusicSeq == 0) {
			playDigMusic(_digSeqMusicTable[num].name, &_digSeqMusicTable[num], 0, true);
			_nextSeqToPlay = 0;
			_attributes[DIG_SEQ_OFFSET + num] = 1;
		} else {
			if ((_digSeqMusicTable[_curMusicSeq].opcode == 4) || (_digSeqMusicTable[_curMusicSeq].opcode == 6)) {
				_nextSeqToPlay = num;
				return;
			} else {
				playDigMusic(_digSeqMusicTable[num].name, &_digSeqMusicTable[num], 0, true);
				_nextSeqToPlay = 0;
				_attributes[DIG_SEQ_OFFSET + num] = 1;
			}
		}
	} else {
		if (_nextSeqToPlay != 0) {
			playDigMusic(_digSeqMusicTable[_nextSeqToPlay].name, &_digSeqMusicTable[_nextSeqToPlay], 0, true);
			_attributes[DIG_SEQ_OFFSET + _nextSeqToPlay] = 1;
			num = _nextSeqToPlay;
			_nextSeqToPlay = 0;
		} else {
			if (_curMusicState != 0) {
				playDigMusic(_digStateMusicTable[_curMusicState].name, &_digStateMusicTable[_curMusicState], _curMusicState, true);
			} else
				playDigMusic(NULL, &_digStateMusicTable[0], _curMusicState, true);
			num = 0;
		}
	}

	_curMusicSeq = num;
}

void IMuseDigital::playDigMusic(const char *songName, const imuseDigTable *table, int atribPos, bool sequence) {
	int hookId = 0;

	if (songName != NULL) {
		if ((_attributes[DIG_SEQ_OFFSET + 38] != 0) && (_attributes[DIG_SEQ_OFFSET + 41] == _attributes[DIG_SEQ_OFFSET + 38])) {
			if ((atribPos == 43) || (atribPos == 44))
				hookId = 3;
		}

		if ((_attributes[DIG_SEQ_OFFSET + 46] != 0) && (_attributes[DIG_SEQ_OFFSET + 48] == 0)) {
			if ((atribPos == 38) || (atribPos == 39))
				hookId = 3;
		}

		if ((_attributes[DIG_SEQ_OFFSET + 53] != 0)) {
			if ((atribPos == 50) || (atribPos == 51))
				hookId = 3;
		}

		if ((atribPos != 0) && (hookId == 0)) {
			if (table->atribPos != 0)
				atribPos = table->atribPos;
			hookId = _attributes[DIG_STATE_OFFSET + atribPos];
			if (table->hookId != 0) {
				if ((hookId != 0) && (table->hookId > 1)) {
					_attributes[DIG_STATE_OFFSET + atribPos] = 2;
				} else {
					_attributes[DIG_STATE_OFFSET + atribPos] = hookId + 1;
					if (table->hookId < hookId + 1)
						_attributes[DIG_STATE_OFFSET + atribPos] = 1;
				}
			}
		}
	}

	fadeOutMusic(120);

	switch(table->opcode) {
		case 0:
		case 5:
		case 6:
			break;
		case 3:
		case 4:
			if (table->filename[0] == 0) {
				return;
			}
			if ((!sequence) && (table->atribPos != 0) &&
					(table->atribPos == _digStateMusicTable[_curMusicState].atribPos)) {
				startMusic(table->filename, table->soundId, 0, 127);
				return;
			}
			startMusic(table->filename, table->soundId, hookId, 127);
			break;
	}
}

void IMuseDigital::setComiMusicState(int stateId) {
	int l, num = -1;

	// This happens at the beginning of Part II, but should apparently not
	// do anything since the correct music is already playing. A left-over
	// of some kind?

	if (stateId == 4)
		return;

	if (stateId == 0)
		stateId = 1000;

	for (l = 0; _comiStateMusicTable[l].soundId != -1; l++) {
		if ((_comiStateMusicTable[l].soundId == stateId)) {
			debug(5, "Set music state: %s, %s", _comiStateMusicTable[l].name, _comiStateMusicTable[l].filename);
			num = l;
			break;
		}
	}
	assert(num != -1);

	if (_curMusicState == num)
		return;

	if (_curMusicSeq == 0) {
		if (num == 0)
			playComiMusic(NULL, &_comiStateMusicTable[0], num, false);
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
			debug(5, "Set music sequence: %s, %s", _comiSeqMusicTable[l].name, _comiSeqMusicTable[l].filename);
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
			_nextSeqToPlay = 0;
		} else {
			if ((_comiSeqMusicTable[_curMusicSeq].opcode == 4) || (_comiSeqMusicTable[_curMusicSeq].opcode == 6)) {
				_nextSeqToPlay = num;
				return;
			} else {
				playComiMusic(_comiSeqMusicTable[num].name, &_comiSeqMusicTable[num], 0, true);
				_nextSeqToPlay = 0;
			}
		}
	} else {
		if (_nextSeqToPlay != 0) {
			playComiMusic(_comiSeqMusicTable[_nextSeqToPlay].name, &_comiSeqMusicTable[_nextSeqToPlay], 0, true);
			num = _nextSeqToPlay;
			_nextSeqToPlay = 0;
		} else {
			if (_curMusicState != 0) {
				playComiMusic(_comiStateMusicTable[_curMusicState].name, &_comiStateMusicTable[_curMusicState], _curMusicState, true);
			} else
				playComiMusic(NULL, &_comiStateMusicTable[0], _curMusicState, true);
			num = 0;
		}
	}

	_curMusicSeq = num;
}

void IMuseDigital::playComiMusic(const char *songName, const imuseComiTable *table, int atribPos, bool sequence) {
	int hookId = 0;

	if ((songName != NULL) && (atribPos != 0)) {
		if (table->atribPos != 0)
			atribPos = table->atribPos;
		hookId = _attributes[COMI_STATE_OFFSET + atribPos];
		if (table->hookId != 0) {
			if ((hookId != 0) && (table->hookId > 1)) {
				_attributes[COMI_STATE_OFFSET + atribPos] = 2;
			} else {
				_attributes[COMI_STATE_OFFSET + atribPos] = hookId + 1;
				if (table->hookId < hookId + 1)
					_attributes[COMI_STATE_OFFSET + atribPos] = 1;
			}
		}
	}

	switch (table->opcode) {
		case 0:
			fadeOutMusic(120);
			break;
		case 8:
		case 9:
			setHookId(table->soundId, table->soundId);
			break;
		case 1:
			if (table->filename[0] == 0) {
				fadeOutMusic(120);
				return;
			}
			fadeOutMusic(120);
			startMusic(table->filename, table->soundId, 0, 1);
			setFade(table->soundId, 127, 120);
			break;
		case 2:
			if (table->filename[0] == 0) {
				fadeOutMusic(60);
				return;
			}
			fadeOutMusic(table->fadeOut60TicksDelay);
			startMusic(table->filename, table->soundId, table->hookId, 127);
			break;
		case 3:
		case 4:
		case 12:
			if (table->filename[0] == 0) {
				fadeOutMusic(60);
				return;
			}
			fadeOutMusic(table->fadeOut60TicksDelay);
			if ((!sequence) && (table->atribPos != 0) &&
					(table->atribPos == _comiStateMusicTable[_curMusicState].atribPos)) {
				startMusic(table->filename, table->soundId, 0, 127);
				return;
			}
			if (table->opcode == 12) {
				startMusic(table->filename, table->soundId, table->hookId, 127);
			} else {
				startMusic(table->filename, table->soundId, hookId, 127);
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

	if (_curMusicSeq == 0) {
		if (stateId == 0)
			playFtMusic(NULL, 0, 0);
		else
			playFtMusic(_ftStateMusicTable[stateId].audioName, _ftStateMusicTable[stateId].opcode, _ftStateMusicTable[stateId].volume);
	}

	_curMusicState = stateId;
}

void IMuseDigital::setFtMusicSequence(int seqId) {
	if (seqId > 52)
		return;

	debug(5, "Sequence music: %s", _ftSeqNames[seqId].name);

	if (_curMusicSeq == seqId)
		return;

	if (seqId == 0) {
		if (_curMusicState == 0)
			playFtMusic(NULL, 0, 0);
		else {
			playFtMusic(_ftStateMusicTable[_curMusicState].audioName, _ftStateMusicTable[_curMusicState].opcode, _ftStateMusicTable[_curMusicState].volume);
		}
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
		int seq = ((_curMusicSeq - 1) * 4) + cueId;
		playFtMusic(_ftSeqMusicTable[seq].audioName, _ftSeqMusicTable[seq].opcode, _ftSeqMusicTable[seq].volume);
	}

	_curMusicCue = cueId;
}

void IMuseDigital::setAudioNames(int32 num, char *names) {
	free(_audioNames);
	_numAudioNames = num;
	_audioNames = names;
}

int IMuseDigital::getSoundIdByName(const char *soundName) {
	if (soundName && soundName[0] != 0) {
		for (int r = 0; r < _numAudioNames; r++) {
			if (strcmp(soundName, &_audioNames[r * 9]) == 0) {
				return r;
			}
		}
	}

	return -1;
}

void IMuseDigital::playFtMusic(const char *songName, int opcode, int volume) {
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
				if (soundId != -1) {
					startMusic(soundId, volume);
				}
			}
			break;
	}
}


} // End of namespace Scumm
