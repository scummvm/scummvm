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

#ifndef SCUMM_INSANE_REBEL1_RELEASES_H
#define SCUMM_INSANE_REBEL1_RELEASES_H

#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

namespace Scumm {

typedef int16 Rebel1TuningTable[21][3][13];

struct Rebel1TuningLayout {
	uint32 offset; // File offset in the executable; zero uses the retail defaults.
	uint16 levelStride;
	uint16 difficultyStride;
};

struct Rebel1WalkerData {
	// Window ends indexed by [window][route], in ANM-local frames. Negative entries are disabled.
	int16 attackWindows[3][3];
	// Destination frame after the extra source frame at the splice.
	int16 routeStartFrame;
	bool earlyShot;
};

struct Rebel1Release {
	enum { kNumLevels = 15 };

	const char *variant;
	// DOS executable supplying UI strings/tuning; null uses built-in defaults.
	const char *executable;
	const char *logo;
	const char *introNotice;
	const char *intro;
	const char *ending;
	bool chapterTransitions;
	bool passcodes;
	// Chapter numbers in playback order. Unused entries are zero.
	byte levels[kNumLevels];
	// Optional replacement chapter order when restored content is enabled.
	const byte *restoredLevels;
	// Bit N enables the target/path bonus for chapter N + 1.
	uint16 chapterBonusMask;
	// Early chapter 6 awards one or three tuning bonuses at 20/30 kills.
	bool tieredAsteroidChaseBonus;
	Rebel1TuningLayout tuning;
	const Rebel1WalkerData *walker;

	int getLevelCount() const;
	int findLevel(int level) const;
	int resolvePasscodeLevel(int level) const;
	bool readTuningData(Common::SeekableReadStream &in, Rebel1TuningTable &table) const;
};

Rebel1Release getRebel1Release(const char *variant, bool restoredContent = false);

} // End of namespace Scumm

#endif
