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

#ifndef SCUMM_INSANE_REBEL2_RELEASES_H
#define SCUMM_INSANE_REBEL2_RELEASES_H

#include "common/scummsys.h"

namespace Scumm {

struct Rebel2DifficultyParams {
	int16 laserDelay;
	int16 snapDistance;
	int16 missDamage;
	int16 dodgeDamage;
	int16 shotDamage;
	int16 specialDamage;
	int16 shotAccuracy;
	int16 hitPoints;
	int16 dodgePoints;
	int16 timePoints;
	int16 levelPoints;
	int16 specialPoints;
	int16 flags;
	int16 rollRate;
	int16 liftRate;
	int16 slideRate;
	int16 driftRate;
};

struct Rebel2DifficultyOverride {
	byte levelType;
	Rebel2DifficultyParams difficulty[6];
};

struct Rebel2Release {
	enum { kNumLevels = 15, kFinale = 16 };

	const char *variant;
	const char *strings;
	const char *nonInteractiveVideo;
	// Chapter numbers in playback order; unused entries are zero.
	byte levels[kNumLevels];
	bool ending;
	bool unlockAvailableLevels;
	// The DA1.06 demo starts chapter 6 at the attack on the reactor.
	bool skipMiningFacilityAttack;
	const Rebel2DifficultyOverride *difficultyOverrides;
	uint difficultyOverrideCount;

	bool isChapterAvailable(int chapter) const;
	int getNextChapter(int chapter) const;
	const Rebel2DifficultyParams *getDifficultyOverride(int difficulty, int levelType) const;
};

Rebel2Release getRebel2Release(const char *variant, bool restoredContent = false);

} // End of namespace Scumm

#endif
