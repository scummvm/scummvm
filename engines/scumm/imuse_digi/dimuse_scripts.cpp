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

#include "scumm/imuse_digi/dimuse_engine.h"

namespace Scumm {

#define DIG_STATE_OFFSET 11
#define DIG_SEQ_OFFSET (DIG_STATE_OFFSET + 65)
#define COMI_STATE_OFFSET 3

int IMuseDigital::scriptParse(int cmd, int a, int b) {
	if (_scriptInitializedFlag || !cmd) {
		switch (cmd) {
		case 0:
			if (_scriptInitializedFlag) {
				debug(5, "IMuseDigital::scriptParse(): script module already initialized");
				return -1;
			} else {
				_scriptInitializedFlag = 1;
				return scriptInit();
			}
		case 1:
			_scriptInitializedFlag = 0;
			return scriptTerminate();
		case 2: // script_save(a, b);
		case 3: // script_restore(a);
			break;
		case 4:
			scriptRefresh();
			return 0;
		case 5:
			scriptSetState(a);
			return 0;
		case 6:
			scriptSetSequence(a);
			return 0;
		case 7:
			scriptSetCuePoint(a);
			return 0;
		case 8:
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
	_curMusicCue = 0;
	memset(_attributes, 0, sizeof(_attributes));
	return 0;
}

int IMuseDigital::scriptTerminate() {
	diMUSETerminate();

	_curMusicState = 0;
	_curMusicSeq = 0;
	_nextSeqToPlay = 0;
	_curMusicCue = 0;
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
	if (_vm->_game.id == GID_DIG && !_isEarlyDiMUSE) {
		setDigMusicState(soundId);
	} else if (_vm->_game.id == GID_CMI) {
		setComiMusicState(soundId);
	} else {
		setFtMusicState(soundId);
	}
}

void IMuseDigital::scriptSetSequence(int soundId) {
	if (_vm->_game.id == GID_DIG && !_isEarlyDiMUSE) {
		setDigMusicSequence(soundId);
	} else if (_vm->_game.id == GID_CMI) {
		setComiMusicSequence(soundId);
	} else {
		setFtMusicSequence(soundId);
	}
}

void IMuseDigital::scriptSetCuePoint(int cueId) {
	if (!_isEarlyDiMUSE)
		return;

	if (cueId > 3)
		return;

	debug(5, "IMuseDigital::scriptSetCuePoint(): Cue point sequence: %d", cueId);

	if (_curMusicSeq && _curMusicCue != cueId) {
		if (cueId == 0)
			playFtMusic(nullptr, 0, 0);
		else {
			int seq = ((_curMusicSeq - 1) * 4) + cueId;
			playFtMusic(_ftSeqMusicTable[seq].audioName, _ftSeqMusicTable[seq].transitionType, _ftSeqMusicTable[seq].volume);
		}
	}

	_curMusicCue = cueId;
}

int IMuseDigital::scriptSetAttribute(int attrIndex, int attrVal) {
	// FT appears to set a single attribute to 1 at start-up and
	// never use it again, so we currently ignore that behavior
	if (_vm->_game.id == GID_DIG) {
		_attributes[attrIndex] = attrVal;
	}
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

void Scumm::IMuseDigital::setFtMusicState(int stateId) {
	if (stateId > 48)
		return;

	debug(5, "IMuseDigital::setFtMusicState(): State music: %s, %s", _ftStateMusicTable[stateId].name, _ftStateMusicTable[stateId].audioName);

	if (_curMusicState == stateId)
		return;

	if (_curMusicSeq == 0) {
		if (stateId == 0) {
			playFtMusic(nullptr, 0, 0);
		} else {
			playFtMusic(_ftStateMusicTable[stateId].audioName, _ftStateMusicTable[stateId].transitionType, _ftStateMusicTable[stateId].volume);
		}
	}

	_curMusicState = stateId;
}

void IMuseDigital::setFtMusicSequence(int seqId) {
	if (seqId > 52)
		return;

	debug(5, "IMuseDigital::setFtMusicSequence(): Sequence music: %s", _ftSeqNames[seqId].name);

	if (_curMusicSeq != seqId) {
		if (seqId == 0) {
			if (_curMusicState == 0) {
				playFtMusic(nullptr, 0, 0);
			} else {
				playFtMusic(_ftStateMusicTable[_curMusicState].audioName, _ftStateMusicTable[_curMusicState].transitionType, _ftStateMusicTable[_curMusicState].volume);
			}
		} else {
			int seq = (seqId - 1) * 4;
			playFtMusic(_ftSeqMusicTable[seq].audioName, _ftSeqMusicTable[seq].transitionType, _ftSeqMusicTable[seq].volume);
		}
	}

	_curMusicSeq = seqId;
	_curMusicCue = 0;
}


void IMuseDigital::setDigMusicState(int stateId) {
	int l, num = -1;

	for (l = 0; _digStateMusicTable[l].soundId != -1; l++) {
		if ((_digStateMusicTable[l].soundId == stateId)) {
			debug(5, "IMuseDigital::setDigMusicState(): Set music state: %s, %s", _digStateMusicTable[l].name, _digStateMusicTable[l].filename);
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
			if (_attributes[_digStateMusicMap[num].attribPos] != 0) {
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

	debug(5, "IMuseDigital::setDigMusicState(): Set music state: %s, %s", _digStateMusicTable[num].name, _digStateMusicTable[num].filename);

	if (_curMusicState == num)
		return;

	if (_curMusicSeq == 0) {
		if (num == 0)
			playDigMusic(nullptr, &_digStateMusicTable[0], num, false);
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
			debug(5, "IMuseDigital::setDigMusicSequence(): Set music sequence: %s, %s", _digSeqMusicTable[l].name, _digSeqMusicTable[l].filename);
			num = l;
			break;
		}
	}

	if (num == -1)
		return;

	if (_curMusicSeq == num)
		return;

	if (num != 0) {
		if (_curMusicSeq && ((_digSeqMusicTable[_curMusicSeq].transitionType == 4)
			|| (_digSeqMusicTable[_curMusicSeq].transitionType == 6))) {
			_nextSeqToPlay = num;
			return;
		} else {
			playDigMusic(_digSeqMusicTable[num].name, &_digSeqMusicTable[num], 0, true);
			_nextSeqToPlay = 0;
			_attributes[DIG_SEQ_OFFSET + num] = 1;
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
			} else {
				playDigMusic(nullptr, &_digStateMusicTable[0], _curMusicState, true);
			}

			num = 0;
		}
	}

	_curMusicSeq = num;
}

void IMuseDigital::setComiMusicState(int stateId) {
	int l, num = -1;

	if (stateId == 0)
		stateId = 1000;

	if ((_vm->_game.features & GF_DEMO) && stateId == 1000)
		stateId = 0;

	if (!(_vm->_game.features & GF_DEMO)) {
		for (l = 0; _comiStateMusicTable[l].soundId != -1; l++) {
			if ((_comiStateMusicTable[l].soundId == stateId)) {
				debug(5, "IMuseDigital::setComiMusicState(): Set music state: %s, %s", _comiStateMusicTable[l].name, _comiStateMusicTable[l].filename);
				num = l;
				break;
			}
		}
	}

	if (num == -1 && !(_vm->_game.features & GF_DEMO))
		return;

	if (!(_vm->_game.features & GF_DEMO) && _curMusicState == num) {
		return;
	} else if ((_vm->_game.features & GF_DEMO) && _curMusicState == stateId) {
		return;
	}

	if (_curMusicSeq == 0) {
		if (_vm->_game.features & GF_DEMO) {
			if (_curMusicSeq == 0) {
				if (stateId == 0)
					playComiDemoMusic(nullptr, &_comiDemoStateMusicTable[0], stateId, false);
				else
					playComiDemoMusic(_comiDemoStateMusicTable[stateId].name, &_comiDemoStateMusicTable[stateId], stateId, false);
			}
		} else {
			if (num == 0)
				playComiMusic(nullptr, &_comiStateMusicTable[0], num, false);
			else
				playComiMusic(_comiStateMusicTable[num].name, &_comiStateMusicTable[num], num, false);
		}
	}

	if (!(_vm->_game.features & GF_DEMO)) {
		_curMusicState = num;
	} else {
		_curMusicState = stateId;
	}
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

void IMuseDigital::playFtMusic(const char *songName, int transitionType, int volume) {
	int oldSoundId = 0;
	int soundId;

	if (!_spooledMusicEnabled)
		return;

	// Check for any music piece which is played as a SFX (without an associated stream)
	// and fade it out
	for (int i = diMUSEGetNextSound(0); i; i = diMUSEGetNextSound(i)) {
		if (diMUSEGetParam(i, DIMUSE_P_GROUP) == DIMUSE_GROUP_MUSICEFF && !diMUSEGetParam(i, DIMUSE_P_SND_HAS_STREAM))
			diMUSEFadeParam(i, DIMUSE_P_VOLUME, 0, 200);
	}

	// Now grab the current standard music soundId: it will either be crossfaded by SwitchStream,
	// or faded out
	for (int j = diMUSEGetNextSound(0); j; j = diMUSEGetNextSound(j)) {
		if (diMUSEGetParam(j, DIMUSE_P_GROUP) == DIMUSE_GROUP_MUSICEFF && diMUSEGetParam(j, DIMUSE_P_SND_HAS_STREAM))
			oldSoundId = j;
	}

	if (songName) {
		switch (transitionType) {
		case 0:
			debug(5, "IMuseDigital::playFtMusic(): NULL transition, ignored");
			return;
		case 1:
			soundId = getSoundIdByName(songName);

			if (_filesHandler->openSound(soundId))
				return;

			if (!soundId) {
				debug(5, "IMuseDigital::playFtMusic(): failed to retrieve soundId for sound \"%s\"", songName);
				break;
			}

			if (diMUSEStartSound(soundId, 126))
				debug(5, "IMuseDigital::playFtMusic(): transition 1, failed to start sound \"%s\"(%d)", songName, soundId);

			_filesHandler->closeSound(soundId);
			diMUSESetParam(soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
			diMUSESetParam(soundId, DIMUSE_P_VOLUME, volume);
			break;
		case 2:
		case 3:
			soundId = getSoundIdByName(songName);
			if (soundId)
				if (_filesHandler->openSound(soundId))
					return;

			if (soundId) {
				if (oldSoundId) {
					if (oldSoundId != soundId || transitionType == 2) {
						diMUSESwitchStream(oldSoundId, soundId, _ftCrossfadeBuffer, sizeof(_ftCrossfadeBuffer), 0);
					}

					// WORKAROUND for bug in the original: at the beginning of the game, going in
					// and out of the bar a couple of times breaks and temporarily stops the music
					// Here, we override the fade out, just like the remastered does
					if (oldSoundId == soundId && soundId == 622) {
						diMUSEFadeParam(soundId, DIMUSE_P_VOLUME, volume, 200);
					}
				} else if (diMUSEStartStream(soundId, 126, DIMUSE_BUFFER_MUSIC)) {
					debug(5, "IMuseDigital::playFtMusic(): failed to start the stream for \"%s\" (%d)", songName, soundId);
				}

				_filesHandler->closeSound(soundId);
				diMUSESetParam(soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
				diMUSESetParam(soundId, DIMUSE_P_VOLUME, volume);
			} else {
				debug(5, "IMuseDigital::playFtMusic(): failed to retrieve soundId for sound \"%s\" (%d)", songName, soundId);
			}
			break;
		case 4:
			if (oldSoundId)
				diMUSEFadeParam(oldSoundId, DIMUSE_P_VOLUME, 0, 200);
			return;
		default:
			debug(5, "IMuseDigital::playFtMusic(): bogus transition type, ignored");
			return;
		}
	} else {
		if (oldSoundId)
			diMUSEFadeParam(oldSoundId, DIMUSE_P_VOLUME, 0, 200);
	}
}

void IMuseDigital::playDigMusic(const char *songName, const imuseDigTable *table, int attribPos, bool sequence) {
	int hookId = 0;

	if (songName != nullptr) {
		if ((_attributes[DIG_SEQ_OFFSET + 38]) && (!_attributes[DIG_SEQ_OFFSET + 41])) {
			if ((attribPos == 43) || (attribPos == 44))
				hookId = 3;
		}

		if ((_attributes[DIG_SEQ_OFFSET + 46] != 0) && (_attributes[DIG_SEQ_OFFSET + 48] == 0)) {
			if ((attribPos == 38) || (attribPos == 39))
				hookId = 3;
		}

		if ((_attributes[DIG_SEQ_OFFSET + 53] != 0)) {
			if ((attribPos == 50) || (attribPos == 51))
				hookId = 3;
		}

		if ((attribPos != 0) && (hookId == 0)) {
			if (table->attribPos != 0)
				attribPos = table->attribPos;
			hookId = _attributes[DIG_STATE_OFFSET + attribPos];
			if (table->hookId != 0) {
				if ((hookId != 0) && (table->hookId > 1)) {
					_attributes[DIG_STATE_OFFSET + attribPos] = 2;
				} else {
					_attributes[DIG_STATE_OFFSET + attribPos] = hookId + 1;
					if (table->hookId < hookId + 1)
						_attributes[DIG_STATE_OFFSET + attribPos] = 1;
				}
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
		debug(5, "IMuseDigital::playDigMusic(): NULL transition, ignored");
		break;
	case 1:
		if (_filesHandler->openSound(table->soundId))
			return;

		if (table->soundId) {
			if (diMUSEStartSound(table->soundId, 126))
				debug(5, "IMuseDigital::playDigMusic(): transition 1, failed to start the sound (%d)", table->soundId);
			diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 1);
			diMUSEFadeParam(table->soundId, DIMUSE_P_VOLUME, 127, 120);
			_filesHandler->closeSound(table->soundId);
			diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
		} else {
			debug(5, "IMuseDigital::playDigMusic(): transition 1, empty soundId, ignored");
		}

		break;
	case 2:
	case 3:
	case 4:
		if (_filesHandler->openSound(table->soundId))
			return;

		if (table->filename[0] == 0 || table->soundId == 0) {
			if (oldSoundId)
				diMUSEFadeParam(oldSoundId, DIMUSE_P_VOLUME, 0, 60);
			return;
		}

		if (table->transitionType == 4) {
			_stopSequenceFlag = 0;
			diMUSESetTrigger(table->soundId, MKTAG('_', 'e', 'n', 'd'), 0);
		}

		if (oldSoundId) {
			if (table->transitionType == 2) {
				diMUSESwitchStream(oldSoundId, table->soundId, 1800, 0, 0);
				diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 127);
				diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
				diMUSESetHook(table->soundId, hookId);
				diMUSEProcessStreams();
				diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF); // Repeated intentionally
				return;
			}

			if (oldSoundId != table->soundId) {
				if ((!sequence) && (table->attribPos != 0) &&
					(table->attribPos == _digStateMusicTable[_curMusicState].attribPos)) {
					diMUSESwitchStream(oldSoundId, table->soundId, 1800, 0, 1);
					diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 127);
					diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
					diMUSEProcessStreams();
				} else {
					diMUSESwitchStream(oldSoundId, table->soundId, 1800, 0, 0);
					diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 127);
					diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
					diMUSESetHook(table->soundId, hookId);
					diMUSEProcessStreams();
					_filesHandler->closeSound(table->soundId);
					diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF); // Repeated intentionally
				}
			}
		} else {
			if (diMUSEStartStream(table->soundId, 126, DIMUSE_BUFFER_MUSIC))
				debug(5, "IMuseDigital::playDigMusic(): failed to start the stream for sound %d", table->soundId);
			diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 127);
			diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
			diMUSESetHook(table->soundId, hookId);
		}
		_filesHandler->closeSound(table->soundId);
		diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF); // Repeated intentionally
		break;
	case 5:
		debug(5, "IMuseDigital::playDigMusic(): no-op transition type (5), ignored");
		break;
	case 6:
		_stopSequenceFlag = 0;
		diMUSESetTrigger(DIMUSE_SMUSH_SOUNDID + DIMUSE_BUFFER_MUSIC, MKTAG('_', 'e', 'n', 'd'), 0);
		break;
	case 7:
		if (oldSoundId)
			diMUSEFadeParam(oldSoundId, DIMUSE_P_VOLUME, 0, 60);
		break;
	default:
		debug(5, "IMuseDigital::playDigMusic(): bogus transition type, ignored");
		break;
	}
}

void IMuseDigital::playComiDemoMusic(const char *songName, const imuseComiTable *table, int attribPos, bool sequence) {
	// This is a stripped down version of playDigMusic
	int hookId = 0;

	if (songName != nullptr) {
		if (attribPos != 0) {
			if (table->attribPos != 0)
				attribPos = table->attribPos;
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
	case 3:
		if (_filesHandler->openSound(table->soundId))
			return;

		if (table->filename[0] == 0 || table->soundId == 0) {
			if (oldSoundId)
				diMUSEFadeParam(oldSoundId, DIMUSE_P_VOLUME, 0, 60);
			return;
		}

		if (oldSoundId) {
			if (oldSoundId != table->soundId) {
				if ((!sequence) && (table->attribPos != 0) &&
					(table->attribPos == _comiDemoStateMusicTable[_curMusicState].attribPos)) {
					diMUSESwitchStream(oldSoundId, table->soundId, 1800, 0, 1);
					diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 127);
					diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
					diMUSEProcessStreams();
				} else {
					diMUSESwitchStream(oldSoundId, table->soundId, 1800, 0, 0);
					diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 127);
					diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
					diMUSESetHook(table->soundId, hookId);
					diMUSEProcessStreams();
					_filesHandler->closeSound(table->soundId);
					diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF); // Repeated intentionally
				}
			}
		} else {
			if (diMUSEStartStream(table->soundId, 126, DIMUSE_BUFFER_MUSIC))
				debug(5, "IMuseDigital::playComiDemoMusic(): failed to start the stream for sound %d", table->soundId);
			diMUSESetParam(table->soundId, DIMUSE_P_VOLUME, 127);
			diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF);
			diMUSESetHook(table->soundId, hookId);
		}
		_filesHandler->closeSound(table->soundId);
		diMUSESetParam(table->soundId, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSICEFF); // Repeated intentionally
		break;
	default:
		debug(5, "IMuseDigital::playDigMusic(): bogus or unused transition type, ignored");
		break;
	}
}

void IMuseDigital::playComiMusic(const char *songName, const imuseComiTable *table, int attribPos, bool sequence) {
	int hookId = 0;
	int fadeDelay = 0;

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
			diMUSESetTrigger(table->soundId, MKTAG('_', 'e', 'n', 'd'), 0);
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
						diMUSESetTrigger(oldSoundId, MKTAG('e', 'x', 'i', 't'), 26, oldSoundId, table->soundId, fadeDelay, 1, 0);
						diMUSESetTrigger(oldSoundId, MKTAG('e', 'x', 'i', 't'), 12, table->soundId, DIMUSE_P_VOLUME, 127);
						diMUSESetTrigger(oldSoundId, MKTAG('e', 'x', 'i', 't'), 12, table->soundId, DIMUSE_P_GROUP, 4);
						diMUSESetTrigger(oldSoundId, MKTAG('e', 'x', 'i', 't'), 15, table->soundId, hookId);
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
		diMUSESetTrigger(DIMUSE_SMUSH_SOUNDID + DIMUSE_BUFFER_MUSIC, MKTAG('_', 'e', 'n', 'd'), 0);
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
		diMUSESetTrigger(oldSoundId, MKTAG('_', 'e', 'n', 'd'), 0);
		break;
	default:
		debug(5, "IMuseDigital::playComiMusic(): bogus transition type, ignored");
		break;
	}
}

} // End of namespace Scumm
