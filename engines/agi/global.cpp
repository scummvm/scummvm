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
 */

#include "agi/agi.h"

namespace Agi {

bool AgiBase::getFlag(int16 flagNr) {
	uint8 *flagPtr = _game.flags;

	flagPtr += flagNr >> 3;
	return (*flagPtr & (1 << (flagNr & 0x07))) != 0;
}

void AgiBase::setFlag(int16 flagNr, bool newState) {
	uint8 *flagPtr = _game.flags;

	flagPtr += flagNr >> 3;
	if (newState)
		*flagPtr |= 1 << (flagNr & 0x07);	// set bit
	else
		*flagPtr &= ~(1 << (flagNr & 0x07));	// clear bit
}

void AgiBase::flipFlag(int16 flagNr) {
	uint8 *flagPtr = _game.flags;

	flagPtr += flagNr >> 3;
	*flagPtr ^= 1 << (flagNr & 0x07);	// flip bit
}

void AgiEngine::setVar(int16 varNr, byte newValue) {
	_game.vars[varNr] = newValue;

	if (varNr == VM_VAR_VOLUME) {
		_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, newValue * 17);
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, newValue * 17);
	}
}

byte AgiEngine::getVar(int16 varNr) {
	switch (varNr) {
	case VM_VAR_SECONDS:
	case VM_VAR_MINUTES:
	case VM_VAR_HOURS:
	case VM_VAR_DAYS:
		// Timer Update is necessary in here, because of at least Manhunter 1 script 153
		// Sierra AGI updated the timer via a timer procedure
		inGameTimerUpdate();
		break;
	default:
		break;
	}
	return _game.vars[varNr];
}

// In-Game timer, used for timer VM Variables
void AgiEngine::inGameTimerReset(uint32 newPlayTime) {
	_passedPlayTimeCycles = 0;
	_lastUsedPlayTimeInCycles = newPlayTime / 50;
	_lastUsedPlayTimeInSeconds = newPlayTime / 1000;
	setTotalPlayTime(newPlayTime);
}
void AgiEngine::inGameTimerPause() {
	pauseEngine(true);
}
void AgiEngine::inGameTimerResume() {
	pauseEngine(false);
}
uint32 AgiEngine::inGameTimerGet() {
	return getTotalPlayTime();
}

// This is called, when one of the timer variables is read
// We calculate the latest variables, according to current official playtime
// This is also called in the main loop, because the game needs to be sync'd to 20 cycles per second
void AgiEngine::inGameTimerUpdate() {
	uint32 curPlayTimeMilliseconds = inGameTimerGet();
	uint32 curPlayTimeCycles = curPlayTimeMilliseconds / 50;

	if (curPlayTimeCycles == _lastUsedPlayTimeInCycles) {
		// No difference, skip updating
		return;
	}

	// Increase passed cycles accordingly
	int32 playTimeCycleDelta = curPlayTimeCycles - _lastUsedPlayTimeInCycles;
	if (playTimeCycleDelta > 0) {
		_passedPlayTimeCycles += playTimeCycleDelta;
	}
	_lastUsedPlayTimeInCycles = curPlayTimeCycles;

	// Now calculate current play time in seconds
	uint32 curPlayTimeSeconds = curPlayTimeMilliseconds / 1000;

	if (curPlayTimeSeconds == _lastUsedPlayTimeInSeconds) {
		// No difference, skip updating
		return;
	}

	uint32 secondsLeft = 0;
	byte   curDays = 0;
	byte   curHours = 0;
	byte   curMinutes = 0;
	byte   curSeconds = 0;

	curDays = curPlayTimeSeconds / 86400;
	secondsLeft = curPlayTimeSeconds % 86400;

	curHours = secondsLeft / 3600;
	secondsLeft = secondsLeft % 3600;

	curMinutes = secondsLeft / 60;
	curSeconds = secondsLeft % 60;

	// directly set them, otherwise we would go into an endless loop
	_game.vars[VM_VAR_SECONDS] = curSeconds;
	_game.vars[VM_VAR_MINUTES] = curMinutes;
	_game.vars[VM_VAR_HOURS] = curHours;
	_game.vars[VM_VAR_DAYS] = curDays;

	_lastUsedPlayTimeInSeconds = curPlayTimeSeconds;
}

void AgiEngine::decrypt(uint8 *mem, int len) {
	const uint8 *key;
	int i;

	key = (getFeatures() & GF_AGDS) ? (const uint8 *)CRYPT_KEY_AGDS
	                   : (const uint8 *)CRYPT_KEY_SIERRA;

	for (i = 0; i < len; i++)
		*(mem + i) ^= *(key + (i % 11));
}

} // End of namespace Agi
