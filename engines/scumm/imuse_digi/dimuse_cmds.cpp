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

// We have some uintptr arguments as commands 28, 29 and 30 actually require pointer arguments
// Unfortunately this makes function calls for other command a little less pretty...
int IMuseDigital::cmdsHandleCmd(int cmd, uint8 *ptr, int a, int b, int c, int d, int e,
	int f, int g, int h, int i, int j, int k, int l, int m, int n) {

	// Convert the character constant (single quotes '') to string
	char marker[5];
	if (!_isEarlyDiMUSE && (cmd == 17 || cmd == 18 || cmd == 19)) {
		for (int index = 0; index < 4; index++) {
#if defined SCUMM_BIG_ENDIAN
			marker[index] = (b >> (8 * index)) & 0xff;
#elif defined SCUMM_LITTLE_ENDIAN
			marker[3 - index] = (b >> (8 * index)) & 0xff;
#endif
		}
		marker[4] = '\0';
	}

	switch (cmd) {
	case DIMUSE_C_INIT:
		return cmdsInit();
	case DIMUSE_C_PAUSE:
		return cmdsPause();
	case DIMUSE_C_RESUME:
		return cmdsResume();
	case DIMUSE_C_SET_GRP_VOL:
		return _groupsHandler->setGroupVol(a, b);
	case DIMUSE_C_START_SND:
		cmdsStartSound(a, b);
		break;
	case DIMUSE_C_STOP_SND:
		cmdsStopSound(a);
		break;
	case DIMUSE_C_STOP_ALL_SNDS:
		cmdsStopAllSounds();
		break;
	case DIMUSE_C_GET_NEXT_SND:
		return cmdsGetNextSound(a);
	case DIMUSE_C_SET_PARAM:
		cmdsSetParam(a, b, c);
		break;
	case DIMUSE_C_GET_PARAM:
		return cmdsGetParam(a, b);
	case DIMUSE_C_FADE_PARAM:
		return _fadesHandler->fadeParam(a, b, c, d);
	case DIMUSE_C_SET_HOOK:
		return cmdsSetHook(a, b);
	case DIMUSE_C_GET_HOOK:
		return cmdsGetHook(a);
	case DIMUSE_C_SET_TRIGGER:
		return _triggersHandler->setTrigger(a, marker, c, d, e, f, g, h, i, j, k, l, m, n);
	case DIMUSE_C_CHECK_TRIGGER:
		return _triggersHandler->checkTrigger(a, marker, c);
	case DIMUSE_C_CLEAR_TRIGGER:
		return _triggersHandler->clearTrigger(a, marker, c);
	case DIMUSE_C_DEFER_CMD:
		return _triggersHandler->deferCommand(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
	case DIMUSE_C_GET_MARKER_SYNCS:
		_vm->_sound->extractSyncsFromDiMUSEMarker((char *)ptr);
		break;
	case DIMUSE_C_START_STREAM:
		return waveStartStream(a, b, c);
	case DIMUSE_C_SWITCH_STREAM:
		if (_isEarlyDiMUSE)
			return waveSwitchStream(a, b, ptr, d, e);
		else
			return waveSwitchStream(a, b, c, d, e);
	case DIMUSE_C_PROCESS_STREAMS:
		return waveProcessStreams();
	case DIMUSE_C_FEED_STREAM:
		return waveFeedStream(a, ptr, c, d);
	default:
		debug(5, "IMuseDigital::cmdsHandleCmd(): bogus/unused opcode ignored (%d).", cmd);
		return -1;
	}

	return 0;
}

int IMuseDigital::cmdsInit() {
	_cmdsRunning60HzCount = 0;
	_cmdsRunning10HzCount = 0;

	if (_groupsHandler->init() || _fadesHandler->init() ||
		_triggersHandler->init() || waveInit()) {
		return -1;
	}

	_cmdsPauseCount = 0;
	return 48;
}

int IMuseDigital::cmdsDeinit() {
	waveTerminate();
	waveOutDeinit();
	_triggersHandler->deinit();
	_fadesHandler->deinit();
	_cmdsPauseCount = 0;
	_cmdsRunning60HzCount = 0;
	_cmdsRunning10HzCount = 0;

	return 0;
}

int IMuseDigital::cmdsTerminate() {
	return 0;
}

int IMuseDigital::cmdsPause() {
	int result = 0;

	if (_cmdsPauseCount == 0) {
		result = wavePause();
	}

	if (!result) {
		result = _cmdsPauseCount + 1;
	}
	_cmdsPauseCount++;

	return result;
}

int IMuseDigital::cmdsResume() {
	int result = 0;

	if (_cmdsPauseCount == 1) {
		result = waveResume();
	}

	if (_cmdsPauseCount != 0) {
		_cmdsPauseCount--;
	}

	if (!result) {
		result = _cmdsPauseCount;
	}

	return result;
}

void IMuseDigital::cmdsSaveLoad(Common::Serializer &ser) {
	// Serialize in this order:
	// - Open files
	// - Fades
	// - Triggers
	// - Pass the control to waveSaveLoad and then tracksSaveLoad, which will serialize:
	//     - Dispatches
	//     - Tracks (with SYNCs, if the game is COMI)
	// - State and sequence info
	// - Attributes
	// - Full Throttle's music cue ID

	_filesHandler->saveLoad(ser);
	_fadesHandler->saveLoad(ser);
	_triggersHandler->saveLoad(ser);
	waveSaveLoad(ser);
	ser.syncAsSint32LE(_curMusicState, VER(103));
	ser.syncAsSint32LE(_curMusicSeq, VER(103));
	ser.syncAsSint32LE(_nextSeqToPlay, VER(103));
	ser.syncAsByte(_radioChatterSFX, VER(103));
	ser.syncArray(_attributes, 188, Common::Serializer::Sint32LE, VER(103));
	ser.syncAsSint32LE(_curMusicCue, VER(103));
}

int IMuseDigital::cmdsStartSound(int soundId, int priority) {
	uint8 *src = _filesHandler->getSoundAddrData(soundId);

	if (src == nullptr) {
		debug(5, "IMuseDigital::cmdsStartSound(): ERROR: resource address for sound %d is NULL", soundId);
		return -1;
	}

	// Check for the "Creative Voice File" header
	if (_isEarlyDiMUSE && READ_BE_UINT32(src) == MKTAG('C', 'r', 'e', 'a'))
		return waveStartSound(soundId, priority);

	// Check for the "iMUS" header
	if (READ_BE_UINT32(src) == MKTAG('i', 'M', 'U', 'S'))
		return waveStartSound(soundId, priority);

	return -1;
}

int IMuseDigital::cmdsStopSound(int soundId) {
	int result = _filesHandler->getNextSound(soundId);

	if (result != 2)
		return -1;

	return waveStopSound(soundId);
}

int IMuseDigital::cmdsStopAllSounds() {
	return _triggersHandler->clearAllTriggers() | waveStopAllSounds();
}

int IMuseDigital::cmdsGetNextSound(int soundId) {
	return waveGetNextSound(soundId);
}

int IMuseDigital::cmdsSetParam(int soundId, int subCmd, int value) {
	int result = _filesHandler->getNextSound(soundId);

	if (result != 2)
		return -1;

	return waveSetParam(soundId, subCmd, value);
}

int IMuseDigital::cmdsGetParam(int soundId, int subCmd) {
	int result = _filesHandler->getNextSound(soundId);

	if (subCmd != 0) {
		if (subCmd == DIMUSE_P_TRIGS_SNDS) {
			return _triggersHandler->countPendingSounds(soundId);
		}

		if (result == 2) {
			return waveGetParam(soundId, subCmd);
		}

		result = (subCmd == DIMUSE_P_SND_TRACK_NUM) - 1;
	}

	return result;
}

int IMuseDigital::cmdsSetHook(int soundId, int hookId) {
	int result = _filesHandler->getNextSound(soundId);

	if (result != 2)
		return -1;

	return waveSetHook(soundId, hookId);
}

int IMuseDigital::cmdsGetHook(int soundId) {
	int result = _filesHandler->getNextSound(soundId);

	if (result != 2)
		return -1;

	return waveGetHook(soundId);
}

} // End of namespace Scumm
