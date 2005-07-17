// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "bits.h"
#include "debug.h"

#include "imuse/imuse.h"

void Imuse::setMusicState(int stateId) {
	int l, num = -1;

	if (stateId == 0)
		stateId = 1000;

	for (l = 0; _stateMusicTable[l].soundId != -1; l++) {
		if (_stateMusicTable[l].soundId == stateId) {
			num = l;
			break;
		}
	}
	assert(num != -1);

	if (_curMusicState == num)
		return;

	if (_curMusicSeq == 0) {
		playMusic(&_stateMusicTable[num], num, false);
	}

	_curMusicState = num;
}

int Imuse::setMusicSequence(int seqId) {
	int l, num = -1;

	if (seqId == -1)
		return _seqMusicTable[_curMusicSeq].soundId;

	if (seqId == 0)
		seqId = 2000;

	for (l = 0; _seqMusicTable[l].soundId != -1; l++) {
		if (_seqMusicTable[l].soundId == seqId) {
			num = l;
			break;
		}
	}

	assert(num != -1);

	if (_curMusicSeq == num)
		return _seqMusicTable[_curMusicSeq].soundId;

	if (num != 0) {
		playMusic(&_seqMusicTable[num], 0, true);
	} else {
		playMusic(&_stateMusicTable[_curMusicState], _curMusicState, true);
		num = 0;
	}

	_curMusicSeq = num;
	return _seqMusicTable[_curMusicSeq].soundId;
}

void Imuse::playMusic(const ImuseTable *table, int atribPos, bool sequence) {
	int hookId = 0;

	if (atribPos != 0) {
		if (table->atribPos != 0)
			atribPos = table->atribPos;
		hookId = _attributes[atribPos];
		if (table->hookId != 0) {
			if ((hookId == 0) && (table->hookId > 1)) {
				_attributes[atribPos] = 2;
			} else {
				_attributes[atribPos] = hookId + 1;
				if (table->hookId < hookId + 1)
					_attributes[atribPos] = 1;
			}
		}
	}
	if (hookId == 0)
		hookId = 100;

	if (table->opcode == 0) {
		fadeOutMusic(120);
	} else if ((table->opcode == 2) || (table->opcode == 3)) {
		if (table->filename[0] == 0) {
			fadeOutMusic(60);
		} else {
			char *soundName = getCurMusicSoundName();
			int pan;
			
			if (table->pan == 0)
				pan = 64;
			else
				pan = table->pan;
			if (soundName != NULL && (table->opcode == 3) && (!sequence)
			 && (strcmp(soundName, table->filename) == 0) && (table->atribPos != 0)
			 && table->atribPos == _stateMusicTable[_curMusicState].atribPos) {
				setFadeVolume(soundName, table->volume, table->fadeOut60TicksDelay);
				setFadePan(soundName, pan, table->fadeOut60TicksDelay);
				setHookId(soundName, hookId);
			} else {
				fadeOutMusic(table->fadeOut60TicksDelay);
				startMusic(table->filename, hookId, table->volume, pan);
			}
		}
	}
}
