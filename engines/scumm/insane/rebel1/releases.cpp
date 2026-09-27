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

#include "common/textconsole.h"
#include "scumm/insane/rebel1/releases.h"

namespace Scumm {

// Variants match detection_tables.h. Keep release-specific resources and chapter
// order here so menus, progression and saves use the same content description.
static const Rebel1Release kReleases[] = {
	{
		"", "ASSAULT.EXE", "OPEN/O1LOGO.ANM", "OPEN/O1OPEN.ANM", "FIN/FNFINAL.ANM", true,
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
	},
	{
		// CD-ROM Demo v1.51: the original dispatcher skips chapters 3-9 and
		// 11-15, their transitions, and the retail ending.
		"Demo v1.51", "ASSAULT.EXE", "OPEN/O1LOGO.ANM", "OPEN/O1OPEN.ANM", nullptr, false,
		{ 1, 2, 10 }
	}
};

int Rebel1Release::getLevelCount() const {
	int count = 0;
	while (count < kNumLevels && levels[count])
		++count;
	return count;
}

int Rebel1Release::findLevel(int level) const {
	for (int i = 0; i < getLevelCount(); ++i) {
		if (levels[i] == level)
			return i;
	}
	return -1;
}

int Rebel1Release::resolvePasscodeLevel(int level) const {
	// The original sampler advances passcodes past omitted chapters. A code
	// beyond its last chapter returns to the menu instead of playing the ending.
	if (level < 1 || level > kNumLevels + 1)
		return 0;
	for (; level <= kNumLevels; ++level) {
		if (findLevel(level) >= 0)
			return level;
	}
	return ending ? kNumLevels + 1 : 0;
}

const Rebel1Release &getRebel1Release(const char *variant) {
	if (!variant)
		variant = "";
	for (const Rebel1Release &release : kReleases) {
		if (!strcmp(variant, release.variant))
			return release;
	}
	error("Unknown Rebel Assault release '%s'", variant);
}

} // End of namespace Scumm
