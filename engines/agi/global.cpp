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

#include "common/config-manager.h"
#include "audio/mixer.h"

#include "agi/agi.h"
#include "agi/graphics.h"

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
		*flagPtr |= 1 << (flagNr & 0x07);    // set bit
	else
		*flagPtr &= ~(1 << (flagNr & 0x07)); // clear bit
}

void AgiBase::flipFlag(int16 flagNr) {
	uint8 *flagPtr = _game.flags;

	flagPtr += flagNr >> 3;
	*flagPtr ^= 1 << (flagNr & 0x07);        // flip bit
}

void AgiEngine::setVar(int16 varNr, byte newValue) {
	_game.vars[varNr] = newValue;

	switch (varNr) {
	case VM_VAR_SECONDS:
		setVarSecondsTrigger(newValue);
		break;
	case VM_VAR_VOLUME:
		setVolumeViaScripts(newValue);
		break;
	default:
		break;
	}
}

byte AgiEngine::getVar(int16 varNr) {
	switch (varNr) {
	case VM_VAR_SECONDS:
		getVarSecondsHeuristicTrigger();
		// fall through
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

// sets volume based on script value
// 0 - maximum volume
// 15 - mute
void AgiEngine::setVolumeViaScripts(byte newVolume) {
	newVolume = CLIP<byte>(newVolume, 0, 15);

	if (_veryFirstInitialCycle) {
		// WORKAROUND:
		// The very first cycle is currently running and volume got changed
		// This is surely the initial value. For plenty of fan games, a default of 15 is set
		// Which actually means "mute" in AGI, but AGI on PC used PC speaker, which did not use
		// volume setting. We do. So we detect such a situation and set a flag, so that the
		// volume will get interpreted "correctly" for those fan games.
		// Note: not all fan games are broken in that regard!
		// See bug #7035
		if (getFeatures() & GF_FANMADE) {
			// We only check for fan games, Sierra always did it properly of course
			if (newVolume == 15) {
				// Volume gets set to mute at the start?
				// Probably broken fan game detected, set flag
				debug("Broken volume in fan game detected, enabling workaround");
				_setVolumeBrokenFangame = true;
			}
		}
	}

	if (!_setVolumeBrokenFangame) {
		// In AGI 15 is mute, 0 is loudest
		// Some fan games set this incorrectly as 15 for loudest, 0 for mute
		newVolume = 15 - newVolume; // turn volume around
	}

	int scummVMVolume = newVolume * Audio::Mixer::kMaxMixerVolume / 15;
	bool scummVMMute = false;

	// Set ScummVM setting
	// We do not set "mute". In case "mute" is set, we will not apply the scripts wishes
	ConfMan.setInt("music_volume", scummVMVolume);
	ConfMan.setInt("sfx_volume", scummVMVolume);

	if (ConfMan.hasKey("mute"))
		scummVMMute = ConfMan.getBool("mute");

	if (!scummVMMute) {
		// Also change volume directly
		_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, scummVMVolume);
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, scummVMVolume);
	}
}

void AgiEngine::setVolumeViaSystemSetting() {
	int scummVMVolumeMusic = ConfMan.getInt("music_volume");
	int scummVMVolumeSfx = ConfMan.getInt("sfx_volume");
	bool scummVMMute = false;
	int internalVolume = 0;

	if (ConfMan.hasKey("mute"))
		scummVMMute = ConfMan.getBool("mute");

	// Clip user system setting
	scummVMVolumeMusic = CLIP<int>(scummVMVolumeMusic, 0, Audio::Mixer::kMaxMixerVolume);
	scummVMVolumeSfx = CLIP<int>(scummVMVolumeSfx, 0, Audio::Mixer::kMaxMixerVolume);

	if (scummVMMute) {
		scummVMVolumeMusic = 0;
		scummVMVolumeSfx = 0;
	}

	// Now actually set it
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, scummVMVolumeMusic);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, scummVMVolumeSfx);

	// Take lowest volume to the scripts
	if (scummVMVolumeMusic < scummVMVolumeSfx) {
		internalVolume = scummVMVolumeMusic;
	} else {
		internalVolume = scummVMVolumeSfx;
	}
	// Change it to 0-15 range
	internalVolume = (internalVolume + 1) * 15 / Audio::Mixer::kMaxMixerVolume;
	// Reverse it
	internalVolume = 15 - internalVolume;
	// Put it into the VM variable. Directly set it, otherwise it would call a volume set call
	_game.vars[VM_VAR_VOLUME] = internalVolume;
}

void AgiEngine::resetGetVarSecondsHeuristic() {
	_getVarSecondsHeuristicLastInstructionCounter = 0;
	_getVarSecondsHeuristicCounter = 0;
}

// Called, when the scripts read VM_VAR_SECONDS
void AgiEngine::getVarSecondsHeuristicTrigger() {
	uint32 counterDifference = _instructionCounter - _getVarSecondsHeuristicLastInstructionCounter;

	if (counterDifference <= 3) {
		// Seconds were read within 3 instructions
		_getVarSecondsHeuristicCounter++;
		if (_getVarSecondsHeuristicCounter > 20) {
			// More than 20 times in a row? This really seems to be an inner loop waiting for seconds to change
			// This happens in at least:
			// Police Quest 1 - Poker game (room 75, responsible script 81)

			// Wait a few milliseconds, get events and update screen
			// We MUST NOT process AGI events in here
			wait(10);
			processScummVMEvents();
			_gfx->updateScreen();

			_getVarSecondsHeuristicCounter = 0;
		}
	} else {
		_getVarSecondsHeuristicCounter = 0;
	}
	_getVarSecondsHeuristicLastInstructionCounter = _instructionCounter;
}

// In-Game timer, used for timer VM Variables
void AgiEngine::inGameTimerReset(uint32 newPlayTime) {
	_lastUsedPlayTimeInCycles = newPlayTime / 50;
	_lastUsedPlayTimeInSeconds = newPlayTime / 1000;
	_playTimeInSecondsAdjust = 0; // no adjust for now
	setTotalPlayTime(newPlayTime);
	inGameTimerResetPassedCycles();
}
void AgiEngine::inGameTimerResetPassedCycles() {
	_passedPlayTimeCycles = 0;
}
uint32 AgiEngine::inGameTimerGet() {
	return getTotalPlayTime();
}
uint32 AgiEngine::inGameTimerGetPassedCycles() {
	return _passedPlayTimeCycles;
}

// Seconds got set by the game
// This happens in Mixed Up Mother Goose. The game syncs the songs to VM_VAR_SECONDS, but instead
// of only reading them, it sets it to 0 and then checks if it reached a certain second.
// The original interpreter didn't reset the internal cycles counter. Which means the timing was never accurate,
// because the cycles counter may just overflow right after setting the seconds, which means a second
// increase almost immediately happened. We even fix this issue by adjusting for it.
void AgiEngine::setVarSecondsTrigger(byte newSeconds) {
	// Adjust in game timer, so that VM timer variables are accurate
	inGameTimerUpdate();

	// Adjust VM seconds again
	_game.vars[VM_VAR_SECONDS] = newSeconds;

	// Calculate milliseconds adjust (see comment above)
	uint32 curPlayTimeMilliseconds = inGameTimerGet();
	_playTimeInSecondsAdjust = curPlayTimeMilliseconds % 1000;
}

// This is called, when one of the timer variables is read
// We calculate the latest variables, according to current official playtime
// This is also called in the main loop, because the game needs to be sync'd to 20 cycles per second
void AgiEngine::inGameTimerUpdate() {
	uint32 curPlayTimeMilliseconds = inGameTimerGet();
	uint32 curPlayTimeCycles = curPlayTimeMilliseconds / 25;

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
	if (_playTimeInSecondsAdjust) {
		// Apply adjust from setVarSecondsTrigger()
		if (curPlayTimeMilliseconds >= _playTimeInSecondsAdjust) {
			curPlayTimeMilliseconds -= _playTimeInSecondsAdjust;
		} else {
			curPlayTimeMilliseconds = 0;
		}
	}
	uint32 curPlayTimeSeconds = curPlayTimeMilliseconds / 1000;

	if (curPlayTimeSeconds == _lastUsedPlayTimeInSeconds) {
		// No difference, skip updating
		return;
	}

	int32 playTimeSecondsDelta = curPlayTimeSeconds - _lastUsedPlayTimeInSeconds;

	if (playTimeSecondsDelta > 0) {
		// Read and write to VM vars directly to avoid endless loop
		uint32 secondsLeft = playTimeSecondsDelta;
		byte   curSeconds = _game.vars[VM_VAR_SECONDS];
		byte   curMinutes = _game.vars[VM_VAR_MINUTES];
		byte   curHours = _game.vars[VM_VAR_HOURS];
		byte   curDays = _game.vars[VM_VAR_DAYS];

		// Add delta to VM variables
		if (secondsLeft >= 86400) {
			curDays += secondsLeft / 86400;
			secondsLeft = secondsLeft % 86400;
		}
		if (secondsLeft >= 3600) {
			curHours += secondsLeft / 3600;
			secondsLeft = secondsLeft % 3600;
		}
		if (secondsLeft >= 60) {
			curMinutes += secondsLeft / 60;
			secondsLeft = secondsLeft % 60;
		}
		curSeconds += secondsLeft;

		while (curSeconds > 59) {
			curSeconds -= 60;
			curMinutes++;
		}
		while (curMinutes > 59) {
			curMinutes -= 60;
			curHours++;
		}
		while (curHours > 23) {
			curHours -= 24;
			curDays++;
		}

		// directly set them
		_game.vars[VM_VAR_SECONDS] = curSeconds;
		_game.vars[VM_VAR_MINUTES] = curMinutes;
		_game.vars[VM_VAR_HOURS] = curHours;
		_game.vars[VM_VAR_DAYS] = curDays;
	}

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
