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

#include "engines/grim/imuse/imuse_engine.h"

namespace Grim {

#define GRIM_STATE_OFFSET   3
#define GRIM_SEQ_OFFSET     185
#define GRIM_STOP_SEQ_ATTR  227

#define GRIM_DEMO_NUM_ATTRIBUTES  31
#define GRIM_DEMO_SEQ_OFFSET      27
#define GRIM_DEMO_STOP_SEQ_ATTR   30

int IMuse::scriptParse(int cmd, int a, int b) {
	if (_scriptInitializedFlag || !cmd) {
		switch (cmd) {
		case IMUSE_C_SCRIPT_INIT:
			if (_scriptInitializedFlag) {
				debug(5, "IMuse::scriptParse(): script module already initialized");
				return -1;
			} else {
				_scriptInitializedFlag = true;
				return scriptInit();
			}
		case IMUSE_C_SCRIPT_TERMINATE:
			_scriptInitializedFlag = false;
			return scriptTerminate();
		case IMUSE_C_SCRIPT_SAVE:
		case IMUSE_C_SCRIPT_RESTORE:
			break;
		case IMUSE_C_SCRIPT_REFRESH:
			scriptRefresh();
			return 0;
		case IMUSE_C_SCRIPT_SET_STATE:
			if (_demo)
				return scriptSetState(a);

			scriptSetState(a);
			return 0;
		case IMUSE_C_SCRIPT_SET_SEQUENCE:
			if (_demo)
				return scriptSetSequence(a);

			scriptSetSequence(a);
			return 0;
		case IMUSE_C_SCRIPT_CUE_POINT:
			scriptSetCuePoint(a);
			return 0;
		case IMUSE_C_SCRIPT_SET_ATTRIBUTE:
			return scriptSetAttribute(a, b);
		default:
			debug(5, "IMuse::scriptParse(): unrecognized opcode (%d)", cmd);
			return -1;
		}
	} else {
		debug(5, "IMuse::scriptParse(): script module not initialized");
		return -1;
	}

	return -1;
}

int IMuse::scriptInit() {
	imInitFilelist();

	_curMusicState = 0;
	_curMusicSeq = 0;
	_nextSeqToPlay = 0;
	memset(_attributes, 0, _demo ? GRIM_DEMO_NUM_ATTRIBUTES * sizeof(int32) : sizeof(_attributes));
	return 0;
}

int IMuse::scriptTerminate() {
	scriptDeinit();

	_curMusicState = 0;
	_curMusicSeq = 0;
	_nextSeqToPlay = 0;
	memset(_attributes, 0, _demo ? GRIM_DEMO_NUM_ATTRIBUTES * sizeof(int32) : sizeof(_attributes));
	return 0;
}

void IMuse::scriptRefresh() {
	int soundId;
	int stopSeqAttr = _demo ? GRIM_DEMO_STOP_SEQ_ATTR : GRIM_STOP_SEQ_ATTR;

	if (_attributes[stopSeqAttr]) {
		scriptSetSequence(0);
		_attributes[stopSeqAttr] = 0;
	}

	soundId = imGetNextSound(0);
	while (soundId) {
		if (imGetParam(soundId, IMUSE_P_SND_HAS_STREAM) && imGetParam(soundId, IMUSE_P_STREAM_BUFID) == IMUSE_BUFFER_MUSIC)
			break;

		soundId = imGetNextSound(soundId);
	}

	if (!soundId) {
		if (_curMusicSeq)
			scriptSetSequence(0);
	}

	imFlushSounds();
}

// The demo returns the current state/sequence/attribute value from these functions
int IMuse::scriptSetState(int stateId) {
	const IMuseStateTable *stateTable = _demo ? _grimDemoStateMusicTable : _grimStateMusicTable;
	int stateTableSize = _demo ? ARRAYSIZE(_grimDemoStateMusicTable) : ARRAYSIZE(_grimStateMusicTable);
	int num;

	if (stateId == -1)
		return stateTable[_curMusicState].soundId;

	for (num = 0; num < stateTableSize; num++) {
		if (stateTable[num].soundId == stateId)
			break;
	}

	// (The demo returns the address past the end of the table here)
	if (num == stateTableSize)
		return 0;

	debug(5, "IMuse::scriptSetState(): Set music state: %s", stateTable[num].internalName);

	if (num != _curMusicState) {
		if (!_curMusicSeq)
			scriptPlayMusic(num ? &stateTable[num] : nullptr, num, false);

		_curMusicState = num;
	}

	return stateTable[_curMusicState].soundId;
}

int IMuse::scriptSetSequence(int seqId) {
	const IMuseStateTable *seqTable = _demo ? _grimDemoSeqMusicTable : _grimSeqMusicTable;
	const IMuseStateTable *stateTable = _demo ? _grimDemoStateMusicTable : _grimStateMusicTable;
	int seqTableSize = _demo ? ARRAYSIZE(_grimDemoSeqMusicTable) : ARRAYSIZE(_grimSeqMusicTable);
	int seqOffset = _demo ? GRIM_DEMO_SEQ_OFFSET : GRIM_SEQ_OFFSET;
	int num;

	if (seqId == -1)
		return seqTable[_curMusicSeq].soundId;

	if (seqId == 0)
		seqId = 2000;

	for (num = 0; num < seqTableSize; num++) {
		if (seqTable[num].soundId == seqId)
			break;
	}

	if (num == seqTableSize) {
		debug(5, "IMuse::scriptSetSequence(): bogus sequence");
		return -1;
	}

	debug(5, "IMuse::scriptSetSequence(): Set music sequence: %s", seqTable[num].internalName);

	if (num == _curMusicSeq)
		return seqTable[_curMusicSeq].soundId;

	if (num) {
		if (_curMusicSeq && (seqTable[_curMusicSeq].transitionId == 4 || seqTable[_curMusicSeq].transitionId == 6)) {
			_nextSeqToPlay = num;
			return seqTable[_curMusicSeq].soundId;
		}

		scriptPlayMusic(&seqTable[num], 0, true);
		_nextSeqToPlay = 0;
		_attributes[seqOffset + num] = 1;
		_curMusicSeq = num;
	} else {
		if (_nextSeqToPlay) {
			scriptPlayMusic(&seqTable[_nextSeqToPlay], 0, true);
			_curMusicSeq = _nextSeqToPlay;
			_nextSeqToPlay = 0;
			_attributes[seqOffset + _curMusicSeq] = 1;
			return seqTable[_curMusicSeq].soundId;
		}

		scriptPlayMusic(_curMusicState ? &stateTable[_curMusicState] : nullptr, _curMusicState, true);
		_curMusicSeq = 0;
	}

	return seqTable[_curMusicSeq].soundId;
}

void IMuse::scriptSetCuePoint(int cueId) {
}

int IMuse::scriptSetAttribute(int attrIndex, int attrVal) {
	const IMuseStateTable *stateTable = _demo ? _grimDemoStateMusicTable : _grimStateMusicTable;
	const IMuseStateTable *seqTable = _demo ? _grimDemoSeqMusicTable : _grimSeqMusicTable;
	int stateTableSize = _demo ? ARRAYSIZE(_grimDemoStateMusicTable) : ARRAYSIZE(_grimStateMusicTable);
	int seqTableSize = _demo ? ARRAYSIZE(_grimDemoSeqMusicTable) : ARRAYSIZE(_grimSeqMusicTable);
	int numAttributes = _demo ? GRIM_DEMO_NUM_ATTRIBUTES : ARRAYSIZE(_attributes);

	if (attrIndex == -1) {
		if (attrVal == 0) {
			imPause();
		} else if (attrVal == 1) {
			imResume();
		} else if (attrVal == -1) {
			memset(_attributes, 0, numAttributes * sizeof(int32));
		}
	} else if (attrIndex == 10000) {
		char *dest = (char *)(intptr)attrVal;

		for (int i = 0; i < stateTableSize; i++) {
			memcpy(dest, stateTable[i].name, strlen(stateTable[i].name) + 1);
			dest += strlen(stateTable[i].name) + 1;
		}

		for (int i = 0; i < seqTableSize; i++) {
			memcpy(dest, seqTable[i].name, strlen(seqTable[i].name) + 1);
			dest += strlen(seqTable[i].name) + 1;
		}

		if (_demo)
			return 25;
	} else if ((uint32)attrIndex >= (uint32)numAttributes) {
		debug(5, "IMuse::scriptSetAttribute(): bogus attribute");
	} else {
		if (attrVal != -1) {
			debug(5, "IMuse::scriptSetAttribute(): Set attr %u to %u", attrIndex, attrVal);
			_attributes[attrIndex] = attrVal;
		}

		if (_demo)
			return _attributes[attrIndex];
	}

	return 0;
}

void IMuse::scriptSaveLoad(Common::Serializer &ser) {
	if (ser.isLoading()) {
		imUnloadAll();
		imCloseAll();
	}

	ser.syncAsSint32LE(_curMusicState, VER(30));
	ser.syncAsSint32LE(_curMusicSeq, VER(30));
	ser.syncAsSint32LE(_nextSeqToPlay, VER(30));
	ser.syncArray(_attributes, ARRAYSIZE(_attributes), Common::Serializer::Sint32LE, VER(30));
	imSaveLoadFilelist(ser);
}

void IMuse::scriptDeinit() {
	imStopAllSounds();
	imUnloadAll();
	imCloseAll();
}

int IMuse::scriptTriggerCallback(char *marker) {
	if (marker[0] != '_') {
		debug(5, "IMuse::scriptTriggerCallback(): got marker != '_end', callback ignored");
		return -1;
	}
	_attributes[_demo ? GRIM_DEMO_STOP_SEQ_ATTR : GRIM_STOP_SEQ_ATTR] = 1;
	return 0;
}

void IMuse::scriptPlayMusic(const IMuseStateTable *tableEntry, int curSongEntryId, int isSequence) {
	int hookId = 0;
	int attribPos, oldSoundId, newSoundId, soundId;
	int fadeTime, fadeDelay, fadeLength;
	char fileName[64];

	Common::StackLock lock(*_mutex);

	if (_demo) {
		scriptPlayMusicDemo(tableEntry, curSongEntryId, isSequence);
		return;
	}

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
	for (soundId = imGetNextSound(0); soundId; soundId = imGetNextSound(soundId)) {
		if (imGetParam(soundId, IMUSE_P_GROUP) == IMUSE_GROUP_MUSICEFF && !imGetParam(soundId, IMUSE_P_SND_HAS_STREAM))
			imFadeParam(soundId, IMUSE_P_VOLUME, 0, 120);
	}

	oldSoundId = 0;
	for (soundId = imGetNextSound(0); soundId; soundId = imGetNextSound(soundId)) {
		if (imGetParam(soundId, IMUSE_P_SND_HAS_STREAM) && imGetParam(soundId, IMUSE_P_STREAM_BUFID) == IMUSE_BUFFER_MUSIC) {
			oldSoundId = soundId;
			break;
		}
	}

	if (!tableEntry) {
		if (oldSoundId)
			imFadeParam(oldSoundId, IMUSE_P_VOLUME, 0, 120);
		return;
	}

	fadeTime = tableEntry->fadeTime;
	if (!fadeTime)
		fadeTime = 1000;

	fadeDelay = fadeTime;
	fadeLength = (3 * fadeTime) / 50;

	switch (tableEntry->transitionId) {
	case 0:
		debug(5, "IMuse::scriptPlayMusic(): NULL transition, ignored");
		break;
	case 2:
	case 3:
	case 4:
	case 10:
	case 11:
	case 12:
		// The original remaps the name through a table which always appends ".IMC"
		Common::strlcpy(fileName, tableEntry->name, sizeof(fileName));
		Common::strlcat(fileName, ".IMC", sizeof(fileName));

		debug(5, "IMuse::scriptPlayMusic(): opening %s", fileName);
		newSoundId = imOpenSound(fileName, 0);

		if (!newSoundId) {
			debug(5, "IMuse::scriptPlayMusic(): unable to open %s", fileName);
			if (oldSoundId)
				imFadeParam(oldSoundId, IMUSE_P_VOLUME, 0, 60);
			return;
		}

		if (tableEntry->transitionId == 4) {
			_attributes[GRIM_STOP_SEQ_ATTR] = 0;
			imSetTrigger(newSoundId, "_end", IMUSE_C_SCRIPT_CALLBACK);
		}

		debug(5, "IMuse::scriptPlayMusic(): HOOK=%d", hookId);

		if (!oldSoundId) {
			debug(5, "IMuse::scriptPlayMusic(): no stream sound");
			if (imStartStream(newSoundId, 126, IMUSE_BUFFER_MUSIC))
				debug(5, "IMuse::scriptPlayMusic(): failed to start the stream for sound %d", newSoundId);

			imSetParam(newSoundId, IMUSE_P_VOLUME, 0);
			imFadeParam(newSoundId, IMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			imSetParam(newSoundId, IMUSE_P_PAN, tableEntry->pan);
			imSetParam(newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			imSetHook(newSoundId, hookId);
		} else if (tableEntry->transitionId == 2) {
			imSwitchStream(oldSoundId, newSoundId, fadeDelay, 0, 0);
			debug(5, "IMuse::scriptPlayMusic(): FADE=%d, vol=%d", fadeDelay, tableEntry->volume);
			imFadeParam(newSoundId, IMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			imFadeParam(newSoundId, IMUSE_P_PAN, tableEntry->pan, fadeLength);
			imSetParam(newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			imSetHook(newSoundId, hookId);
			imProcessStreams();
		} else if (oldSoundId == newSoundId) {
			imFadeParam(newSoundId, IMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			debug(5, "IMuse::scriptPlayMusic(): fading VOL=%d", tableEntry->volume);
			if (tableEntry->pan) {
				imFadeParam(newSoundId, IMUSE_P_PAN, tableEntry->pan, fadeLength);
				debug(5, "IMuse::scriptPlayMusic(): and PAN=%d", tableEntry->pan);
			}
		} else if (!isSequence && tableEntry->attribPos && tableEntry->attribPos == _grimStateMusicTable[_curMusicState].attribPos) {
			imSwitchStream(oldSoundId, newSoundId, fadeDelay, 0, 1);
			imFadeParam(newSoundId, IMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			imFadeParam(newSoundId, IMUSE_P_PAN, tableEntry->pan, fadeLength);
			imSetParam(newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			if (hookId == 100)
				imSetHook(newSoundId, 100);
			imProcessStreams();
		} else if (tableEntry->transitionId == 10 || tableEntry->transitionId == 12) {
			if (tableEntry->transitionId == 12) {
				imSetHook(oldSoundId, 99);
				imSetTrigger(oldSoundId, "clear", IMUSE_C_SET_HOOK, oldSoundId, 128);
			}

			imSetTrigger(oldSoundId, "exit", IMUSE_C_SWITCH_STREAM, oldSoundId, newSoundId, fadeDelay, 1, 0);
			imSetTrigger(oldSoundId, "exit", IMUSE_C_FADE_PARAM, newSoundId, IMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			imSetTrigger(oldSoundId, "exit", IMUSE_C_FADE_PARAM, newSoundId, IMUSE_P_PAN, tableEntry->pan, fadeLength);
			imSetTrigger(oldSoundId, "exit", IMUSE_C_SET_PARAM, newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			imSetTrigger(oldSoundId, "exit", IMUSE_C_SET_HOOK, newSoundId, hookId);
			imProcessStreams();
		} else if (tableEntry->transitionId == 11) {
			imSetTrigger(oldSoundId, "exit2", IMUSE_C_SWITCH_STREAM, oldSoundId, newSoundId, fadeDelay, 1, 0);
			imSetTrigger(oldSoundId, "exit2", IMUSE_C_FADE_PARAM, newSoundId, IMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			imSetTrigger(oldSoundId, "exit2", IMUSE_C_FADE_PARAM, newSoundId, IMUSE_P_PAN, tableEntry->pan, fadeLength);
			imSetTrigger(oldSoundId, "exit2", IMUSE_C_SET_PARAM, newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			imSetTrigger(oldSoundId, "exit2", IMUSE_C_SET_HOOK, newSoundId, hookId);
			imProcessStreams();
		} else {
			imSwitchStream(oldSoundId, newSoundId, fadeDelay, 0, 0);
			debug(5, "IMuse::scriptPlayMusic(): FADE=%d, vol=%d", fadeDelay, tableEntry->volume);
			imFadeParam(newSoundId, IMUSE_P_VOLUME, tableEntry->volume, fadeLength);
			imFadeParam(newSoundId, IMUSE_P_PAN, tableEntry->pan, fadeLength);
			debug(5, "IMuse::scriptPlayMusic(): set pan=%d", tableEntry->pan);
			imSetParam(newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			imSetHook(newSoundId, hookId);
			imProcessStreams();
		}

		imCloseSound(newSoundId);
		imSetParam(newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
		break;
	case 5:
		break;
	case 6:
		_attributes[GRIM_STOP_SEQ_ATTR] = 0;
		imSetTrigger(IMUSE_SMUSH_SOUNDID + IMUSE_BUFFER_MUSIC, "_end", IMUSE_C_SCRIPT_CALLBACK);
		break;
	case 7:
		if (oldSoundId)
			imFadeParam(oldSoundId, IMUSE_P_VOLUME, 0, 60);
		break;
	case 8:
		if (oldSoundId)
			imSetHook(oldSoundId, 100);
		break;
	case 9:
		if (oldSoundId)
			imSetHook(oldSoundId, 99);

		_attributes[GRIM_STOP_SEQ_ATTR] = 0;
		imSetTrigger(oldSoundId, "_end", IMUSE_C_SCRIPT_CALLBACK);
		break;
	default:
		debug(5, "IMuse::scriptPlayMusic(): bogus transition type, ignored");
		break;
	}
}

void IMuse::scriptPlayMusicDemo(const IMuseStateTable *tableEntry, int curSongEntryId, int isSequence) {
	int hookId = 0;
	int oldSoundId, newSoundId, soundId;
	int fadeTime, fadeDelay;
	char fileName[64];

	fileName[0] = '\0';
	if (tableEntry) {
		Common::strlcpy(fileName, tableEntry->name, sizeof(fileName));
		for (int i = 0; i < ARRAYSIZE(_grimDemoMusicFileTable); i++) {
			if (!strcmp(fileName, _grimDemoMusicFileTable[i].name)) {
				Common::strlcpy(fileName, _grimDemoMusicFileTable[i].fileName, sizeof(fileName));
				break;
			}
		}
	}

	if (tableEntry && curSongEntryId) {
		int attribPos = curSongEntryId;
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

	// Fade out any music track which is not streamed
	for (soundId = imGetNextSound(0); soundId; soundId = imGetNextSound(soundId)) {
		if (imGetParam(soundId, IMUSE_P_GROUP) == IMUSE_GROUP_MUSICEFF && !imGetParam(soundId, IMUSE_P_SND_HAS_STREAM))
			imFadeParam(soundId, IMUSE_P_VOLUME, 0, 120);
	}

	oldSoundId = 0;
	for (soundId = imGetNextSound(0); soundId; soundId = imGetNextSound(soundId)) {
		if (imGetParam(soundId, IMUSE_P_SND_HAS_STREAM) && imGetParam(soundId, IMUSE_P_STREAM_BUFID) == IMUSE_BUFFER_MUSIC) {
			oldSoundId = soundId;
			break;
		}
	}

	if (!tableEntry) {
		if (oldSoundId)
			imFadeParam(oldSoundId, IMUSE_P_VOLUME, 0, 120);
		return;
	}

	switch (tableEntry->transitionId) {
	case 0:
		debug(5, "IMuse::scriptPlayMusicDemo(): NULL transition, ignored");
		break;
	case 1:
		debug(5, "IMuse::scriptPlayMusicDemo(): loading %s", fileName);
		newSoundId = imLoadSound(fileName, 0);
		if (!newSoundId) {
			debug(5, "IMuse::scriptPlayMusicDemo(): unable to load %s", fileName);
			return;
		}

		if (imStartSound(newSoundId, 126))
			debug(5, "IMuse::scriptPlayMusicDemo(): failed to start sound %d", newSoundId);

		imSetParam(newSoundId, IMUSE_P_VOLUME, 1);
		imFadeParam(newSoundId, IMUSE_P_VOLUME, 127, 120);
		imUnloadSound(newSoundId);
		imSetParam(newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
		break;
	case 2:
	case 3:
	case 4:
	case 10:
	case 11:
	case 12:
		debug(5, "IMuse::scriptPlayMusicDemo(): opening %s", fileName);
		newSoundId = imOpenSound(fileName, 0);

		if (!newSoundId) {
			debug(5, "IMuse::scriptPlayMusicDemo(): unable to open %s", fileName);
			if (oldSoundId)
				imFadeParam(oldSoundId, IMUSE_P_VOLUME, 0, 60);
			return;
		}

		if (tableEntry->transitionId == 4) {
			_attributes[GRIM_DEMO_STOP_SEQ_ATTR] = 0;
			imSetTrigger(newSoundId, "_end", IMUSE_C_SCRIPT_CALLBACK);
		}

		fadeTime = tableEntry->fadeTime;
		fadeDelay = fadeTime ? fadeTime : 1000;

		if (!oldSoundId) {
			if (imStartStream(newSoundId, 126, IMUSE_BUFFER_MUSIC))
				debug(5, "IMuse::scriptPlayMusicDemo(): failed to start the stream for sound %d", newSoundId);

			imSetParam(newSoundId, IMUSE_P_VOLUME, tableEntry->volume);
			imSetParam(newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			imSetHook(newSoundId, hookId);
		} else if (tableEntry->transitionId == 2) {
			imSwitchStream(oldSoundId, newSoundId, fadeDelay, 0, 0);
			imSetParam(newSoundId, IMUSE_P_VOLUME, tableEntry->volume);
			imSetParam(newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			imSetHook(newSoundId, tableEntry->hookId);
			imProcessStreams();
		} else if (oldSoundId == newSoundId) {
			imFadeParam(newSoundId, IMUSE_P_VOLUME, tableEntry->volume, fadeTime);
			debug(5, "IMuse::scriptPlayMusicDemo(): now VOL=%d", tableEntry->volume);
		} else if (!isSequence && tableEntry->attribPos && tableEntry->attribPos == _grimDemoStateMusicTable[_curMusicState].attribPos) {
			imSwitchStream(oldSoundId, newSoundId, fadeDelay, 0, 1);
			imSetParam(newSoundId, IMUSE_P_VOLUME, tableEntry->volume);
			imSetParam(newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			imProcessStreams();
		} else if (tableEntry->transitionId == 10 || tableEntry->transitionId == 12) {
			if (tableEntry->transitionId == 12)
				imSetHook(oldSoundId, tableEntry->hookId);

			imSetTrigger(oldSoundId, "exit", IMUSE_C_SWITCH_STREAM, oldSoundId, newSoundId, fadeDelay, 1, 0);
			imSetTrigger(oldSoundId, "exit", IMUSE_C_SET_PARAM, newSoundId, IMUSE_P_VOLUME, tableEntry->volume);
			imSetTrigger(oldSoundId, "exit", IMUSE_C_SET_PARAM, newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			imSetTrigger(oldSoundId, "exit", IMUSE_C_SET_HOOK, newSoundId, hookId);
			imProcessStreams();
		} else if (tableEntry->transitionId == 11) {
			imSetTrigger(oldSoundId, "exit2", IMUSE_C_SWITCH_STREAM, oldSoundId, newSoundId, fadeDelay, 1, 0);
			imSetTrigger(oldSoundId, "exit2", IMUSE_C_SET_PARAM, newSoundId, IMUSE_P_VOLUME, tableEntry->volume);
			imSetTrigger(oldSoundId, "exit2", IMUSE_C_SET_PARAM, newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			imSetTrigger(oldSoundId, "exit2", IMUSE_C_SET_HOOK, newSoundId, hookId);
			imProcessStreams();
		} else {
			imSwitchStream(oldSoundId, newSoundId, fadeDelay, 0, 0);
			imSetParam(newSoundId, IMUSE_P_VOLUME, tableEntry->volume);
			imSetParam(newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
			imSetHook(newSoundId, 9);
			imProcessStreams();
		}

		imCloseSound(newSoundId);
		imSetParam(newSoundId, IMUSE_P_GROUP, IMUSE_GROUP_MUSICEFF);
		break;
	case 5:
		break;
	case 6:
		_attributes[GRIM_DEMO_STOP_SEQ_ATTR] = 0;
		imSetTrigger(IMUSE_SMUSH_SOUNDID + IMUSE_BUFFER_MUSIC, "_end", IMUSE_C_SCRIPT_CALLBACK);
		break;
	case 7:
		if (oldSoundId)
			imFadeParam(oldSoundId, IMUSE_P_VOLUME, 0, 60);
		break;
	case 8:
		if (oldSoundId)
			imSetHook(oldSoundId, tableEntry->hookId);
		break;
	case 9:
		if (oldSoundId)
			imSetHook(oldSoundId, tableEntry->hookId);

		_attributes[GRIM_DEMO_STOP_SEQ_ATTR] = 0;
		imSetTrigger(oldSoundId, "_end", IMUSE_C_SCRIPT_CALLBACK);
		break;
	default:
		debug(5, "IMuse::scriptPlayMusicDemo(): bogus transition type, ignored");
		break;
	}
}

} // End of namespace Grim
