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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_AMBIENT_AUDIO_H
#define HOLLYWOOD_SCENES_PLAYABLE_AMBIENT_AUDIO_H

#include "common/types.h"

namespace Hollywood {

// Scene ambient sound scheduling mode.
enum AmbientSoundMode {
	kAmbientSoundNone,
	kAmbientSoundLoop,
	kAmbientSoundRandomRange
};

// Scene ambient music scheduling mode.
enum AmbientMusicMode {
	kAmbientMusicNone,
	kAmbientMusicLoopRotation,
	kAmbientMusicRandomRange
};

// Declarative ambient cue setup used by PlayableScene each timer tick.
struct AmbientAudioProfile {
	AmbientAudioProfile() :
		checkMillis(0),
		soundMode(kAmbientSoundNone),
		soundCueId(0),
		soundFirstCueId(0),
		soundCueCount(0),
		soundProbabilityModulus(0),
		soundVolumePercent(0),
		musicMode(kAmbientMusicNone),
		musicStillCueId(0),
		musicFirstCueId(0),
		musicCueCount(0),
		musicProbabilityModulus(0),
		musicVolumePercent(0) {
	}

	uint32 checkMillis;
	AmbientSoundMode soundMode;
	byte soundCueId;
	byte soundFirstCueId;
	byte soundCueCount;
	byte soundProbabilityModulus;
	byte soundVolumePercent;
	AmbientMusicMode musicMode;
	byte musicStillCueId;
	byte musicFirstCueId;
	byte musicCueCount;
	byte musicProbabilityModulus;
	byte musicVolumePercent;
};

} // End of namespace Hollywood

#endif
