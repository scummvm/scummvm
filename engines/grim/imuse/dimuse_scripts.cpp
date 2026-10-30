/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/grim/imuse/dimuse_engine.h"

namespace Grim {

#define GRIM_STATE_OFFSET   3
#define GRIM_SEQ_OFFSET     185
#define GRIM_STOP_SEQ_ATTR  227

int IMuseDigital::scriptParse(int cmd, int a, int b) {
	if (_scriptInitializedFlag || !cmd) {
		switch (cmd) {
		case DIMUSE_C_SCRIPT_INIT:
			if (_scriptInitializedFlag) {
				debug(5, "IMuseDigital::scriptParse(): script module already initialized");
				return -1;
			} else {
				_scriptInitializedFlag = 1;
				return scriptInit();
			}
		case DIMUSE_C_SCRIPT_TERMINATE:
			_scriptInitializedFlag = 0;
			return scriptTerminate();
		case DIMUSE_C_SCRIPT_SAVE:
		case DIMUSE_C_SCRIPT_RESTORE:
			break;
		case DIMUSE_C_SCRIPT_REFRESH:
			scriptRefresh();
			return 0;
		case DIMUSE_C_SCRIPT_SET_STATE:
			scriptSetState(a);
			return 0;
		case DIMUSE_C_SCRIPT_SET_SEQUENCE:
			scriptSetSequence(a);
			return 0;
		case DIMUSE_C_SCRIPT_CUE_POINT:
			scriptSetCuePoint(a);
			return 0;
		case DIMUSE_C_SCRIPT_SET_ATTRIBUTE:
			return scriptSetAttribute(a, b);
		default:
			debug(5, "IMuseDigital::scriptParse(): unrecognized opcode (%d)", cmd);
			return -1;
		}
	} else {
		debug(5, "IMuseDigital::scriptParse(): script module not initialized");
		return -1;
	}

	return -1;
}

int IMuseDigital::scriptInit() {
	_curMusicState = 0;
	_curMusicSeq = 0;
	_nextSeqToPlay = 0;
	memset(_attributes, 0, sizeof(_attributes));
	return 0;
}

int IMuseDigital::scriptTerminate() {
	scriptDeinit();

	_curMusicState = 0;
	_curMusicSeq = 0;
	_nextSeqToPlay = 0;
	memset(_attributes, 0, sizeof(_attributes));
	return 0;
}

void IMuseDigital::scriptRefresh() {
	int soundId;

	if (_attributes[GRIM_STOP_SEQ_ATTR]) {
		scriptSetSequence(0);
		_attributes[GRIM_STOP_SEQ_ATTR] = 0;
	}

	soundId = diMUSEGetNextSound(0);
	while (soundId) {
		if (diMUSEGetParam(soundId, DIMUSE_P_SND_HAS_STREAM) && diMUSEGetParam(soundId, DIMUSE_P_STREAM_BUFID) == DIMUSE_BUFFER_MUSIC)
			break;

		soundId = diMUSEGetNextSound(soundId);
	}

	if (!soundId) {
		if (_curMusicSeq)
			scriptSetSequence(0);
	}

	flushTracks();
}

void IMuseDigital::scriptSetState(int stateId) {
	int num;

	if (stateId == -1)
		return;

	for (num = 0; num < ARRAYSIZE(_grimStateMusicTable); num++) {
		if (_grimStateMusicTable[num].soundId == stateId)
			break;
	}

	if (num == ARRAYSIZE(_grimStateMusicTable))
		return;

	debug(5, "IMuseDigital::scriptSetState(): Set music state: %s", _grimStateMusicTable[num].internalName);

	if (num != _curMusicState) {
		if (!_curMusicSeq)
			scriptPlayMusic(num ? &_grimStateMusicTable[num] : nullptr, num, false);

		_curMusicState = num;
	}
}

void IMuseDigital::scriptSetSequence(int seqId) {
	int num;

	if (seqId == -1)
		return;

	if (seqId == 0)
		seqId = 2000;

	for (num = 0; num < ARRAYSIZE(_grimSeqMusicTable); num++) {
		if (_grimSeqMusicTable[num].soundId == seqId)
			break;
	}

	if (num == ARRAYSIZE(_grimSeqMusicTable)) {
		debug(5, "IMuseDigital::scriptSetSequence(): bogus sequence");
		return;
	}

	debug(5, "IMuseDigital::scriptSetSequence(): Set music sequence: %s", _grimSeqMusicTable[num].internalName);

	if (num == _curMusicSeq)
		return;

	if (num) {
		if (_curMusicSeq && (_grimSeqMusicTable[_curMusicSeq].transitionId == 4 || _grimSeqMusicTable[_curMusicSeq].transitionId == 6)) {
			_nextSeqToPlay = num;
			return;
		}

		scriptPlayMusic(&_grimSeqMusicTable[num], 0, true);
		_nextSeqToPlay = 0;
		_attributes[GRIM_SEQ_OFFSET + num] = 1;
		_curMusicSeq = num;
	} else {
		if (_nextSeqToPlay) {
			scriptPlayMusic(&_grimSeqMusicTable[_nextSeqToPlay], 0, true);
			_curMusicSeq = _nextSeqToPlay;
			_nextSeqToPlay = 0;
			_attributes[GRIM_SEQ_OFFSET + _curMusicSeq] = 1;
			return;
		}

		scriptPlayMusic(_curMusicState ? &_grimStateMusicTable[_curMusicState] : nullptr, _curMusicState, true);
		_curMusicSeq = 0;
	}
}

void IMuseDigital::scriptSetCuePoint(int cueId) {
}

int IMuseDigital::scriptSetAttribute(int attrIndex, int attrVal) {
	if (attrIndex == -1) {
		if (attrVal == 0) {
			diMUSEPause();
		} else if (attrVal == 1) {
			diMUSEResume();
		} else if (attrVal == -1) {
			memset(_attributes, 0, sizeof(_attributes));
		}
	} else if (attrIndex == 10000) {
		char *dest = (char *)(intptr)attrVal;

		for (int i = 0; i < ARRAYSIZE(_grimStateMusicTable); i++) {
			memcpy(dest, _grimStateMusicTable[i].name, strlen(_grimStateMusicTable[i].name) + 1);
			dest += strlen(_grimStateMusicTable[i].name) + 1;
		}

		for (int i = 0; i < ARRAYSIZE(_grimSeqMusicTable); i++) {
			memcpy(dest, _grimSeqMusicTable[i].name, strlen(_grimSeqMusicTable[i].name) + 1);
			dest += strlen(_grimSeqMusicTable[i].name) + 1;
		}
	} else if ((uint32)attrIndex >= ARRAYSIZE(_attributes)) {
		debug(5, "IMuseDigital::scriptSetAttribute(): bogus attribute");
	} else if (attrVal != -1) {
		debug(5, "IMuseDigital::scriptSetAttribute(): Set attr %u to %u", attrIndex, attrVal);
		_attributes[attrIndex] = attrVal;
	}

	return 0;
}

void IMuseDigital::scriptSaveLoad(Common::Serializer &ser) {
	if (ser.isLoading())
		_filesHandler->closeAllSounds();

	ser.syncAsSint32LE(_curMusicState, VER(103));
	ser.syncAsSint32LE(_curMusicSeq, VER(103));
	ser.syncAsSint32LE(_nextSeqToPlay, VER(103));
	ser.syncArray(_attributes, ARRAYSIZE(_attributes), Common::Serializer::Sint32LE, VER(103));
	_filesHandler->saveLoad(ser);
}

void IMuseDigital::scriptDeinit() {
	diMUSEStopAllSounds();
	_filesHandler->closeAllSounds();
}

int IMuseDigital::scriptTriggerCallback(char *marker) {
	if (marker[0] != '_') {
		debug(5, "IMuseDigital::scriptTriggerCallback(): got marker != '_end', callback ignored");
		return -1;
	}
	_attributes[GRIM_STOP_SEQ_ATTR] = 1;
	return 0;
}

void IMuseDigital::scriptPlayMusic(const IMuseDigiStateTable *tableEntry, int curSongEntryId, int isSequence) {
	int hookId = 0;
	int attribPos, oldSoundId, newSoundId, soundId;
	int fadeTime, fadeDelay, fadeLength;

	Common::StackLock lock(*_mutex);

	if (tableEntry) {
		attribPos = curSongEntryId;
		if (curSongEntryId) {
			if (tableEntry->attribPos)
				attribPos = tableEntry->attribPos;

			hookId = _attributes[GRIM_STATE_OFFSET + attribPos];

			if (tableEntry->hookId) {
				if (hookId || tableEntry->hookId <= 1) {
					_attributes[GRIM_STATE_OFFSET + attribPos] = hookId + 1;
					if (hookId + 1 > tableEntry->hookId)
						_attributes[GRIM_STATE_OFFSET + attribPos] = 1;
				} else {
					_attributes[GRIM_STATE_OFFSET + attribPos] = 2;
				}
			}
		}
	}

	if (!hookId)
		hookId = 100;

	// Fade out any music track which is not streamed
	for (soundId = diMUSEGetNextSound(0); soundId; soundId = diMUSEGetNextSound(soundId)) {
		if (diMUSEGetParam(soundId, DIMUSE_P_GROUP) == DIMUSE_GROUP_MUSICEFF && !diMUSEGetParam(soundId, DIMUSE_P_SND_HAS_STREAM))
			diMUSEFadeParam(soundId, DIMUSE_P_VOLUME, 0, 120);
	}

	oldSoundId = 0;
	for (soundId = diMUSEGetNextSound(0); soundId; soundId = diMUSEGetNextSound(soundId)) {
		if (diMUSEGetParam(soundId, DIMUSE_P_SND_HAS_STREAM) && diMUSEGetParam(soundId, DIMUSE_P_STREAM_BUFID) == DIMUSE_BUFFER_MUSIC) {
			oldSoundId = soundId;
			break;
		}
	}

	if (!tableEntry) {
		if (oldSoundId)
			diMUSEFadeParam(oldSoundId, DIMUSE_P_VOLUME, 0, 120);
		return;
	}

	fadeTime = tableEntry->fadeTime;
	if (!fadeTime)
		fadeTime = 1000;

	fadeDelay = fadeTime;
	fadeLength = (3 * fadeTime) / 50;

	switch (tableEntry->transitionId) {
	case 0:
		debug(5, "IMuseDigital::scriptPlayMusic(): NULL transition, ignored");
		break;
	case 2:
	case 3:
	case 4:
	case 10:
	case 11:
	case 12:
		// TODO
		// newSoundId = diMUSEOpenSound(fileName, 0);
		newSoundId = _filesHandler->openSound(tableEntry->soundId) ? 0 : tableEntry->soundId;

		if (!newSoundId) {
			debug(5, "IMuseDigital::scriptPlayMusic(): unable to open sound %d", tableEntry->soundId);
			if (oldSoundId)
				diMUSEFadeParam(oldSoundId, DIMUSE_P_VOLUME, 0, 60);
			return;
		}

		if (tableEntry->transitionId == 4) {
			_attributes[GRIM_STOP_SEQ_ATTR] = 0;
			diMUSESetTrigger(newSoundId, "_end", DIMUSE_C_SCRIPT_CALLBACK);
		}

		debug(5, "IMuseDigital::scriptPlayMusic(): HOOK=%d", hookId);

		if (!oldSoundId) {
			debug(5, "IMuseDigital::scriptPlayMusic(): no stream sound");
			if (diMUSEStartStream(newSoundId, 126, DIMUSE_BUFFER_MUSIC))
				debug(5, "IMuseDigital::scriptPlayMusic(): failed to start the stream for sound %d", newSoundId);

			diMUSESetParam(newSoundId, DIMUSE_P_VOLUME, 0);
			diMUSEFadeParam(newSoundId, DIMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			diMUSESetParam(newSoundId, DIMUSE_P_PAN, tableEntry->pan);
			diMUSESetParam(newSoundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
			diMUSESetHook(newSoundId, hookId);
		} else if (tableEntry->transitionId == 2) {
			diMUSESwitchStream(oldSoundId, newSoundId, fadeDelay, 0, 0);
			debug(5, "IMuseDigital::scriptPlayMusic(): FADE=%d, vol=%d", fadeDelay, tableEntry->volume);
			diMUSEFadeParam(newSoundId, DIMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			diMUSEFadeParam(newSoundId, DIMUSE_P_PAN, tableEntry->pan, fadeLength);
			diMUSESetParam(newSoundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
			diMUSESetHook(newSoundId, hookId);
			diMUSEProcessStreams();
		} else if (oldSoundId == newSoundId) {
			diMUSEFadeParam(newSoundId, DIMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			debug(5, "IMuseDigital::scriptPlayMusic(): fading VOL=%d", tableEntry->volume);
			if (tableEntry->pan) {
				diMUSEFadeParam(newSoundId, DIMUSE_P_PAN, tableEntry->pan, fadeLength);
				debug(5, "IMuseDigital::scriptPlayMusic(): and PAN=%d", tableEntry->pan);
			}
		} else if (!isSequence && tableEntry->attribPos && tableEntry->attribPos == _grimStateMusicTable[_curMusicState].attribPos) {
			diMUSESwitchStream(oldSoundId, newSoundId, fadeDelay, 0, 1);
			diMUSEFadeParam(newSoundId, DIMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			diMUSEFadeParam(newSoundId, DIMUSE_P_PAN, tableEntry->pan, fadeLength);
			diMUSESetParam(newSoundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
			if (hookId == 100)
				diMUSESetHook(newSoundId, 100);
			diMUSEProcessStreams();
		} else if (tableEntry->transitionId == 10 || tableEntry->transitionId == 12) {
			if (tableEntry->transitionId == 12) {
				diMUSESetHook(oldSoundId, 99);
				diMUSESetTrigger(oldSoundId, "clear", DIMUSE_C_SET_HOOK, oldSoundId, 128);
			}

			diMUSESetTrigger(oldSoundId, "exit", DIMUSE_C_SWITCH_STREAM, oldSoundId, newSoundId, fadeDelay, 1, 0);
			diMUSESetTrigger(oldSoundId, "exit", DIMUSE_C_FADE_PARAM, newSoundId, DIMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			diMUSESetTrigger(oldSoundId, "exit", DIMUSE_C_FADE_PARAM, newSoundId, DIMUSE_P_PAN, tableEntry->pan, fadeLength);
			diMUSESetTrigger(oldSoundId, "exit", DIMUSE_C_SET_PARAM, newSoundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
			diMUSESetTrigger(oldSoundId, "exit", DIMUSE_C_SET_HOOK, newSoundId, hookId);
			diMUSEProcessStreams();
		} else if (tableEntry->transitionId == 11) {
			diMUSESetTrigger(oldSoundId, "exit2", DIMUSE_C_SWITCH_STREAM, oldSoundId, newSoundId, fadeDelay, 1, 0);
			diMUSESetTrigger(oldSoundId, "exit2", DIMUSE_C_FADE_PARAM, newSoundId, DIMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			diMUSESetTrigger(oldSoundId, "exit2", DIMUSE_C_FADE_PARAM, newSoundId, DIMUSE_P_PAN, tableEntry->pan, fadeLength);
			diMUSESetTrigger(oldSoundId, "exit2", DIMUSE_C_SET_PARAM, newSoundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
			diMUSESetTrigger(oldSoundId, "exit2", DIMUSE_C_SET_HOOK, newSoundId, hookId);
			diMUSEProcessStreams();
		} else {
			diMUSESwitchStream(oldSoundId, newSoundId, fadeDelay, 0, 0);
			debug(5, "IMuseDigital::scriptPlayMusic(): FADE=%d, vol=%d", fadeDelay, tableEntry->volume);
			diMUSEFadeParam(newSoundId, DIMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			diMUSEFadeParam(newSoundId, DIMUSE_P_PAN, tableEntry->pan, fadeLength);
			debug(5, "IMuseDigital::scriptPlayMusic(): set pan=%d", tableEntry->pan);
			diMUSESetParam(newSoundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
			diMUSESetHook(newSoundId, hookId);
			diMUSEProcessStreams();
		}

		_filesHandler->closeSound(newSoundId);
		diMUSESetParam(newSoundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
		break;
	case 5:
		break;
	case 6:
		_attributes[GRIM_STOP_SEQ_ATTR] = 0;
		diMUSESetTrigger(DIMUSE_SMUSH_SOUNDID + DIMUSE_BUFFER_MUSIC, "_end", DIMUSE_C_SCRIPT_CALLBACK);
		break;
	case 7:
		if (oldSoundId)
			diMUSEFadeParam(oldSoundId, DIMUSE_P_VOLUME, 0, 60);
		break;
	case 8:
		if (oldSoundId)
			diMUSESetHook(oldSoundId, 100);
		break;
	case 9:
		if (oldSoundId)
			diMUSESetHook(oldSoundId, 99);

		_attributes[GRIM_STOP_SEQ_ATTR] = 0;
		diMUSESetTrigger(oldSoundId, "_end", DIMUSE_C_SCRIPT_CALLBACK);
		break;
	default:
		debug(5, "IMuseDigital::scriptPlayMusic(): bogus transition type, ignored");
		break;
	}
}

} // End of namespace Grim
