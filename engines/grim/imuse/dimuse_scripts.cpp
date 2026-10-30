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

#define COMI_STATE_OFFSET 3

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
	diMUSETerminate();

	_curMusicState = 0;
	_curMusicSeq = 0;
	_nextSeqToPlay = 0;
	memset(_attributes, 0, sizeof(_attributes));
	return 0;
}

void IMuseDigital::scriptRefresh() {
	int soundId;
	int nextSound;

	if (_stopSequenceFlag) {
		scriptSetSequence(0);
		_stopSequenceFlag = 0;
	}

	soundId = 0;

	while (1) {
		nextSound = diMUSEGetNextSound(soundId);
		soundId = nextSound;

		if (!nextSound)
			break;

		if (diMUSEGetParam(nextSound, DIMUSE_P_SND_HAS_STREAM) && diMUSEGetParam(soundId, DIMUSE_P_STREAM_BUFID) == DIMUSE_BUFFER_MUSIC) {
			if (soundId)
				return;
			break;
		}
	}

	if (_curMusicSeq)
		scriptSetSequence(0);

	flushTracks();
}

void IMuseDigital::scriptSetState(int soundId) {
	setComiMusicState(soundId);
}

void IMuseDigital::scriptSetSequence(int soundId) {
	setComiMusicSequence(soundId);
}

void IMuseDigital::scriptSetCuePoint(int cueId) {
}

int IMuseDigital::scriptSetAttribute(int attrIndex, int attrVal) {
	return 0;
}

int IMuseDigital::scriptTriggerCallback(char *marker) {
	if (marker[0] != '_') {
		debug(5, "IMuseDigital::scriptTriggerCallback(): got marker != '_end', callback ignored");
		return -1;
	}
	_stopSequenceFlag = 1;
	return 0;
}


void IMuseDigital::setComiMusicState(int stateId) {
	int l, num = -1;

	if (stateId == 0)
		stateId = 1000;

	for (l = 0; _comiStateMusicTable[l].soundId != -1; l++) {
		if ((_comiStateMusicTable[l].soundId == stateId)) {
			debug(5, "IMuseDigital::setComiMusicState(): Set music state: %s, %s", _comiStateMusicTable[l].name, _comiStateMusicTable[l].filename);
			num = l;
			break;
		}
	}

	if (num == -1)
		return;

	if (_curMusicState == num)
		return;

	if (_curMusicSeq == 0) {
		if (num == 0)
			playComiMusic(nullptr, &_comiStateMusicTable[0], num, false);
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
			debug(5, "IMuseDigital::setComiMusicSequence(): Set music sequence: %s, %s", _comiSeqMusicTable[l].name, _comiSeqMusicTable[l].filename);
			num = l;
			break;
		}
	}

	if (num == -1)
		return;

	if (_curMusicSeq == num)
		return;

	if (num != 0) {
		if (_curMusicSeq && ((_comiSeqMusicTable[_curMusicSeq].transitionType == 4)
			|| (_comiSeqMusicTable[_curMusicSeq].transitionType == 6))) {
			_nextSeqToPlay = num;
			return;
		} else {
			playComiMusic(_comiSeqMusicTable[num].name, &_comiSeqMusicTable[num], 0, true);
			_nextSeqToPlay = 0;
		}
	} else {
		if (_nextSeqToPlay != 0) {
			playComiMusic(_comiSeqMusicTable[_nextSeqToPlay].name, &_comiSeqMusicTable[_nextSeqToPlay], 0, true);
			num = _nextSeqToPlay;
			_nextSeqToPlay = 0;
		} else {
			if (_curMusicState != 0) {
				playComiMusic(_comiStateMusicTable[_curMusicState].name, &_comiStateMusicTable[_curMusicState], _curMusicState, true);
			} else {
				playComiMusic(nullptr, &_comiStateMusicTable[0], _curMusicState, true);
			}
			num = 0;
		}
	}

	_curMusicSeq = num;
}

void IMuseDigital::playComiMusic(const char *songName, const imuseComiTable *table, int attribPos, bool sequence) {
	int hookId = 0;
	int fadeDelay = 0;

	// This flag is not set in the original,
	// but since we're not using DirectSound and this is pretty much timing related
	// please see IMuseDigital::waveOutWrite() for more context...
	Common::StackLock lock(*_mutex);
	_waveOutXorTrigger = 1;

	if ((songName != nullptr) && (attribPos != 0)) {
		if (table->attribPos != 0)
			attribPos = table->attribPos;
		hookId = _attributes[COMI_STATE_OFFSET + attribPos];

		if (table->hookId != 0) {
			if ((hookId != 0) && (table->hookId > 1)) {
				_attributes[COMI_STATE_OFFSET + attribPos] = 2;
			} else {
				_attributes[COMI_STATE_OFFSET + attribPos] = hookId + 1;
				if (table->hookId < hookId + 1)
					_attributes[COMI_STATE_OFFSET + attribPos] = 1;
			}
		}
	}

	int nextSoundId = 0;
	while (1) {
		nextSoundId = diMUSEGetNextSound(nextSoundId);
		if (!nextSoundId)
			break;

		// If a sound is found (and its stream is active), fade it out if it's a music track
		if (diMUSEGetParam(nextSoundId, DIMUSE_P_GROUP) == DIMUSE_GROUP_MUSICEFF && !diMUSEGetParam(nextSoundId, DIMUSE_P_SND_HAS_STREAM))
			diMUSEFadeParam(nextSoundId, DIMUSE_P_VOLUME, 0, 120);
	}

	int oldSoundId = 0;
	nextSoundId = 0;
	while (1) {
		nextSoundId = diMUSEGetNextSound(nextSoundId);
		if (!nextSoundId)
			break;

		if (diMUSEGetParam(nextSoundId, DIMUSE_P_SND_HAS_STREAM) && (diMUSEGetParam(nextSoundId, DIMUSE_P_STREAM_BUFID) == DIMUSE_BUFFER_MUSIC)) {
			oldSoundId = nextSoundId;
			break;
		}
	}

	if (!songName) {
		if (oldSoundId)
			diMUSEFadeParam(oldSoundId, DIMUSE_P_VOLUME, 0, 120);
		return;
	}

	switch (table->transitionType) {
	case 0:
		debug(5, "IMuseDigital::playComiMusic(): NULL transition, ignored");
		break;
	case 1:
		if (_filesHandler->openSound(table->soundId))
			return;

		if (table->soundId) {
			if (diMUSEStartSound(table->soundId, 126))
				debug(5, "IMuseDigital::playComiMusic(): transition 1, failed to start the sound (%d)", table->soundId);
			diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 1);
			diMUSEFadeParam(table->soundId, DIMUSE_P_VOLUME, 127, 120);
			_filesHandler->closeSound(table->soundId);
			diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
		} else {
			debug(5, "IMuseDigital::playComiMusic(): transition 1, empty soundId, ignored");
		}
		break;
	case 2:
	case 3:
	case 4:
	case 12:
		if (_filesHandler->openSound(table->soundId))
			return;

		if (table->filename[0] == 0 || table->soundId == 0) {
			if (oldSoundId)
				diMUSEFadeParam(oldSoundId, DIMUSE_P_VOLUME, 0, 60);
			break;
		}

		if (table->transitionType == 4) {
			_stopSequenceFlag = 0;
			diMUSESetTrigger(table->soundId, MKTAG('_', 'e', 'n', 'd'), DIMUSE_C_SCRIPT_CALLBACK);
		}

		if (oldSoundId) {
			fadeDelay = table->fadeOutDelay;
			if (!fadeDelay)
				fadeDelay = 1000;
			else
				fadeDelay = (fadeDelay * 100) / 6; // Set dimuse_table fade out time to millisecond scale

			if (table->transitionType == 2) {
				diMUSESwitchStream(oldSoundId, table->soundId, fadeDelay, 0, 0);
				diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 127);
				diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
				diMUSESetHook(table->soundId, table->hookId);
				diMUSEProcessStreams();
			} else if (oldSoundId != table->soundId) {
				if ((!sequence) && (table->attribPos != 0) &&
					(table->attribPos == _comiStateMusicTable[_curMusicState].attribPos)) {

					debug(5, "IMuseDigital::playComiMusic(): Starting new sound (%s) with same attribute as old sound (%s)",
						table->name, _comiStateMusicTable[_curMusicState].name);

					diMUSESwitchStream(oldSoundId, table->soundId, fadeDelay, 0, 1);
					diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 127);
					diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
					diMUSEProcessStreams();
				} else {
					switch (table->transitionType) {
					case 12:
						diMUSESetHook(oldSoundId, table->hookId);
						diMUSESetTrigger(oldSoundId, MKTAG('e', 'x', 'i', 't'), DIMUSE_C_SWITCH_STREAM, oldSoundId, table->soundId, fadeDelay, 1, 0);
						diMUSESetTrigger(oldSoundId, MKTAG('e', 'x', 'i', 't'), DIMUSE_C_SET_PARAM, table->soundId, DIMUSE_P_VOLUME, 127);
						diMUSESetTrigger(oldSoundId, MKTAG('e', 'x', 'i', 't'), DIMUSE_C_SET_PARAM, table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
						diMUSESetTrigger(oldSoundId, MKTAG('e', 'x', 'i', 't'), DIMUSE_C_SET_HOOK, table->soundId, hookId);
						diMUSEProcessStreams();
						break;
					default:
						diMUSESwitchStream(oldSoundId, table->soundId, fadeDelay, 0, 0);
						diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 127);
						diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
						diMUSESetHook(table->soundId, hookId);
						diMUSEProcessStreams();
						break;
					}
				}
			}
		} else {
			if (diMUSEStartStream(table->soundId, 126, DIMUSE_BUFFER_MUSIC))
				debug(5, "IMuseDigital::playComiMusic(): failed to start the stream for sound %d", table->soundId);
			diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 127);
			diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
			diMUSESetHook(table->soundId, hookId);
		}
		_filesHandler->closeSound(table->soundId);
		diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
		break;
	case 5:
		debug(5, "IMuseDigital::playComiMusic(): no-op transition type (5), ignored");
		break;
	case 6:
		_stopSequenceFlag = 0;
		diMUSESetTrigger(DIMUSE_SMUSH_SOUNDID + DIMUSE_BUFFER_MUSIC, MKTAG('_', 'e', 'n', 'd'), DIMUSE_C_SCRIPT_CALLBACK);
		break;
	case 7:
		if (oldSoundId)
			diMUSEFadeParam(oldSoundId, DIMUSE_P_VOLUME, 0, 60);
		break;
	case 8:
		if (oldSoundId)
			diMUSESetHook(oldSoundId, table->hookId);
		break;
	case 9:
		if (oldSoundId)
			diMUSESetHook(oldSoundId, table->hookId);
		_stopSequenceFlag = 0;
		diMUSESetTrigger(oldSoundId, MKTAG('_', 'e', 'n', 'd'), DIMUSE_C_SCRIPT_CALLBACK);
		break;
	default:
		debug(5, "IMuseDigital::playComiMusic(): bogus transition type, ignored");
		break;
	}
}

} // End of namespace Grim
