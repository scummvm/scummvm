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

#include "common/stream.h"
#include "common/textconsole.h"
#include "scumm/insane/rebel1/releases.h"

namespace Scumm {

static const Rebel1WalkerData kRetailWalker = {
	{ { 2588, 2323, 877 }, { 1709, 1444, -2 }, { 262, -2, -2 } },
	2, true
};

static const Rebel1WalkerData kDemo940413Walker = {
	{ { 2591, 2324, 878 }, { 1712, 1445, -1 }, { 265, -1, -1 } },
	1, false
};

static const byte kDemo940413RestoredLevels[Rebel1Release::kNumLevels] = { 3, 6, 8, 10 };

// Variants match detection_tables.h. Keep release-specific resources and chapter
// order here so menus, progression and saves use the same content description.
static const Rebel1Release kReleases[] = {
	{
		"", "ASSAULT.EXE", "OPEN/O1LOGO.ANM", nullptr, "OPEN/O1OPEN.ANM", "FIN/FNFINAL.ANM", true, true,
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }, nullptr,
		0x7fff, false, { 0, 0, 0 }, &kRetailWalker
	},
	{
		// v0.67: the original at 0x28ac plays the logo once, then loops the
		// preview. Neither the menu movie nor any chapter assets are supplied.
		"Demo v0.67", "REBEL.EXE", "OPEN/O1LOGO.ANM", nullptr, "OPEN/O1OPEN.ANM", nullptr, false, false,
		{ 0 }, nullptr,
		0, false, { 0, 0, 0 }, &kRetailWalker
	},
	{
		// CD-ROM Demo v1.5: the original dispatcher at 0x1478d redirects
		// chapter 1 to 2, chapter 3 to 10, and returns to the menu after 10.
		"Demo v1.5", "ASSAULT.EXE", "OPEN/O1LOGO.ANM", "OPEN/O1DEMO.ANM", "OPEN/O1OPEN.ANM", nullptr, false, true,
		{ 2, 10 }, nullptr,
		0x7fff, false, { 0, 0, 0 }, &kRetailWalker
	},
	{
		// CD-ROM Demo v1.51: the original dispatcher skips chapters 3-9 and
		// 11-15, their transitions, and the retail ending.
		"Demo v1.51", "ASSAULT.EXE", "OPEN/O1LOGO.ANM", nullptr, "OPEN/O1OPEN.ANM", nullptr, false, true,
		{ 1, 2, 10 }, nullptr,
		0x7fff, false, { 0, 0, 0 }, &kRetailWalker
	},
	{
		// PC Media 1 preview: the dispatcher at 0x1496e redirects every
		// chapter and transition to the intro/menu at 0x151f2. No chapter
		// assets are supplied; the demo notice precedes the preview movie.
		"Demo v1.7", "ASSAULT.EXE", "OPEN/O1LOGO.ANM", "OPEN/O1DEMO.ANM", "OPEN/O1OPEN.ANM", nullptr, false, false,
		{ 0 }, nullptr,
		0, false, { 0, 0, 0 }, &kRetailWalker
	},
	{
		// The dispatcher at 0x3466 starts at chapter 8, skips 9, and returns
		// to the menu after 10. Chapters 3 and 6 have complete resources and
		// handlers, but are only reachable through the original debug keys.
		"Demo 1994-04-13", "REBEL.EXE", "OPEN/O1LOGO.ANM", "OPEN/O1DEMO.ANM", "OPEN/O1OPEN.ANM", nullptr, false, false,
		{ 8, 10 }, kDemo940413RestoredLevels,
		1 << (6 - 1), true, { 0x289dc, 0x1a, 0x222 }, &kDemo940413Walker
	},
	{
		// Macintosh v1.02c: CODE 3's dispatcher redirects chapters 1-5 to 6,
		// 7-8 to 9, and 10-15 to the menu. The included ending is not played.
		// Chapter assets and the tuning rows for 6/9 match the retail defaults.
		"Demo v1.02c", nullptr, "OPEN/O1LOGO.ANM", nullptr, "OPEN/O1OPEN.ANM", nullptr, false, false,
		{ 6, 9 }, nullptr,
		0x7fff, false, { 0, 0, 0 }, &kRetailWalker
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
	if (!passcodes || level < 1 || level > kNumLevels + 1)
		return 0;
	for (; level <= kNumLevels; ++level) {
		if (findLevel(level) >= 0)
			return level;
	}
	return ending ? kNumLevels + 1 : 0;
}

bool Rebel1Release::readTuningData(Common::SeekableReadStream &in, Rebel1TuningTable &table) const {
	const int64 requiredSize = (int64)tuning.offset +
		(ARRAYSIZE(table) - 1) * tuning.levelStride +
		(ARRAYSIZE(table[0]) - 1) * tuning.difficultyStride + sizeof(table[0][0]);
	if (!tuning.offset || in.size() < requiredSize)
		return false;

	for (uint level = 0; level < ARRAYSIZE(table); ++level) {
		for (uint difficulty = 0; difficulty < ARRAYSIZE(table[level]); ++difficulty) {
			const int64 offset = (int64)tuning.offset +
				level * tuning.levelStride + difficulty * tuning.difficultyStride;
			if (!in.seek(offset))
				return false;
			for (uint field = 0; field < ARRAYSIZE(table[level][difficulty]); ++field)
				table[level][difficulty][field] = in.readSint16LE();
		}
	}
	return !in.err() && !in.eos();
}

Rebel1Release getRebel1Release(const char *variant, bool restoredContent) {
	if (!variant)
		variant = "";
	for (const Rebel1Release &release : kReleases) {
		if (!strcmp(variant, release.variant)) {
			Rebel1Release result = release;
			if (restoredContent && release.restoredLevels)
				memcpy(result.levels, release.restoredLevels, sizeof(result.levels));
			return result;
		}
	}
	error("Unknown Rebel Assault release '%s'", variant);
}

} // End of namespace Scumm
