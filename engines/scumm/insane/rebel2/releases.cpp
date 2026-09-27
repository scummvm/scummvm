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
#include "common/util.h"
#include "scumm/insane/rebel2/releases.h"

namespace Scumm {

// DA1.06's difficulty table starts at offset 0x4798 in LE object 3.
// Rows have 17 words; each difficulty occupies 0x242 bytes. Types 5 and 6
// cover the shield attack and reactor attack respectively.
// Mac v1.0 has the same combat values at 0x5a70 in its unpacked PEF data
// section. Its unused flight columns are -1; neither section uses flight controls.
static const Rebel2DifficultyOverride kChapter6DemoDifficulty[] = {
	{ 5, {
		{ 5, 5, -1, 19, 2, 15, 75, 25, 50, 2, 500, -1, 8, -1, -1, -1, -1 },
		{ 5, 3, -1, 25, 4, 15, 75, 50, 100, 4, 1000, -1, 16, -1, -1, -1, -1 },
		{ 5, 1, -1, 30, 6, 15, 75, 75, 150, 6, 1500, -1, 0, -1, -1, -1, -1 },
		{ 5, 0, 220, 50, 15, 15, 79, 100, 200, 8, 2000, -1, 4, -1, -1, -1, -1 },
		{ 5, 1, -1, 30, 6, 15, 75, 75, 150, 6, 1500, -1, 0, -1, -1, -1, -1 },
		{ 5, 1, -1, 30, 6, 15, 75, 75, 150, 6, 1500, -1, 0, -1, -1, -1, -1 }
	} },
	{ 6, {
		{ 5, 5, 180, 27, 3, -1, 75, 25, 50, 2, 500, 250, 8, 120, 120, 120, 75 },
		{ 5, 3, 190, 35, 4, -1, 75, 50, 100, 4, 1000, 500, 16, 140, 140, 140, 90 },
		{ 5, 1, 200, 50, 4, -1, 75, 75, 150, 6, 1500, 750, 0, 160, 160, 160, 105 },
		{ 5, 0, 220, 90, 15, -1, 90, 100, 200, 8, 2000, 1000, 4, 180, 180, 180, 140 },
		{ 5, 1, 200, 50, 4, -1, 75, 75, 150, 6, 1500, 750, 0, 160, 160, 160, 105 },
		{ 5, 1, 200, 50, 4, -1, 75, 75, 150, 6, 1500, 750, 0, 160, 160, 160, 105 }
	} }
};

static const Rebel2DemoVideo kDemoVideos[] = {
	{ "OPEN/O_DEMO.SAN", 0 },
	{ nullptr, 0 }
};

static const Rebel2DemoVideo kWindowsDemoVideos[] = {
	{ "RA2VID/O_OPEN_A.SAN", 0x28 },
	{ "RA2VID/O_OPEN_B.SAN", 0x28 },
	{ nullptr, 0 }
};

static const char *const kWindowsDemoFonts[] = {
	"RA2VID/TALKFONT.NUT", "RA2VID/SMALFONT.NUT", "RA2VID/TITLFONT.NUT", nullptr
};

// Variants match detection_tables.h. Availability also limits passwords,
// saved pilot progress and the unlock-all option.
static const Rebel2Release kReleases[] = {
	{
		"", nullptr, "SYSTM/GAME.TRS", nullptr, nullptr,
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
		true, false, false, false, nullptr, 0, nullptr
	},
	{
		"Demo", nullptr, "SYSTM/GAME.TRS", kDemoVideos, nullptr, { 0 },
		false, false, false, false, nullptr, 0, nullptr
	},
	{
		// LUCASDMO.EXE (0x401340/0x401610) plays both intro movies, using
		// three fonts and a TRS containing only credits and the demo notice.
		"Demo Windows 95", nullptr, "RA2VID/REBEL2.TRS", kWindowsDemoVideos, nullptr, { 0 },
		false, false, false, false, nullptr, 0, kWindowsDemoFonts
	},
	{
		// RBL2DEMO.EXE selects GAME_E.TRS. The original chapter-6 handler
		// (LE object 1, 0x10d10) skips the shield attack and uses 06END_B.
		"Demo DA1.06", nullptr, "SYSTM/GAME_E.TRS", nullptr, nullptr, { 6 },
		false, true, true, false, kChapter6DemoDifficulty, ARRAYSIZE(kChapter6DemoDifficulty), nullptr
	},
	{
		// GAME.TRS selects chapters 1, 2 and 4 in playable-demo mode.
		// LE object 1, 0xf860 advances passwords past missing chapters;
		// 0x12eb0 plays O_PLAYDE instead of the retail finale.
		"Demo DG1.15", nullptr, "SYSTM/GAME.TRS", nullptr, "OPEN/O_PLAYDE.SAN", { 1, 2, 4 },
		false, false, false, true, nullptr, 0, nullptr
	},
	{
		// GAME.TRS selects chapters 1, 2 and 3 in playable-demo mode.
		// LE object 1, 0x12e70 plays O_PLAYDE after the last chapter;
		// 0xf840 retains retail completion passwords, unlike DG1.15.
		"Demo Special Edition", nullptr, "SYSTM/GAME.TRS", nullptr, "OPEN/O_PLAYDE.SAN", { 1, 2, 3 },
		false, false, false, false, nullptr, 0, nullptr
	},
	{
		// Macintosh v1.0's chapter-6 handler (PEF code section, 0x17fe8)
		// also skips the shield attack in demo mode and uses 06END_B.
		"Demo v1.0", "Rebel Assault II Demo Data", "SYSTM/GAME_E.TRS", nullptr, nullptr, { 6 },
		false, true, true, false, kChapter6DemoDifficulty, ARRAYSIZE(kChapter6DemoDifficulty), nullptr
	}
};

int Rebel2Release::getFontCount() const {
	if (!fontFiles)
		return 4;
	int count = 0;
	while (fontFiles[count])
		++count;
	return count;
}

const char *Rebel2Release::getFontFile(int font, bool highRes) const {
	static const char *const kFontsLo[] = {
		"SYSTM/TALKFONT.NUT", "SYSTM/SMALFONT.NUT", "SYSTM/TITLFONT.NUT", "SYSTM/POVFONT.NUT"
	};
	static const char *const kFontsHi[] = {
		"SYSTM/TKHIFONT.NUT", "SYSTM/SMHIFONT.NUT", "SYSTM/TIHIFONT.NUT", "SYSTM/POHIFONT.NUT"
	};
	if (font < 0 || font >= getFontCount())
		font = 0;
	return fontFiles ? fontFiles[font] : (highRes ? kFontsHi[font] : kFontsLo[font]);
}

bool Rebel2Release::isChapterAvailable(int chapter) const {
	if (chapter == kFinale)
		return ending;
	if (chapter < 1 || chapter > kNumLevels)
		return false;
	for (int i = 0; i < kNumLevels && levels[i]; ++i) {
		if (levels[i] == chapter)
			return true;
	}
	return false;
}

int Rebel2Release::getNextChapter(int chapter) const {
	for (int i = 0; i < kNumLevels && levels[i]; ++i) {
		if (levels[i] == chapter) {
			if (i + 1 < kNumLevels && levels[i + 1])
				return levels[i + 1];
			return ending ? kFinale : 0;
		}
	}
	return 0;
}

int Rebel2Release::getCompletionPasswordChapter(int chapter) const {
	if (!advanceCompletionPasswords || !isChapterAvailable(chapter))
		return chapter;

	// Passwords unlock the chapter following their retail chapter number.
	// DG1.15 shows the finale password after its last included chapter.
	const int nextChapter = getNextChapter(chapter);
	return nextChapter ? nextChapter - 1 : kNumLevels;
}

const Rebel2DifficultyParams *Rebel2Release::getDifficultyOverride(int difficulty, int levelType) const {
	if (difficulty < 0 || difficulty >= 6)
		return nullptr;
	for (uint i = 0; i < difficultyOverrideCount; ++i) {
		if (difficultyOverrides[i].levelType == levelType)
			return &difficultyOverrides[i].difficulty[difficulty];
	}
	return nullptr;
}

Rebel2Release getRebel2Release(const char *variant, bool restoredContent) {
	if (!variant)
		variant = "";
	for (const Rebel2Release &release : kReleases) {
		if (!strcmp(variant, release.variant)) {
			Rebel2Release result = release;
			if (restoredContent)
				result.skipMiningFacilityAttack = false;
			return result;
		}
	}
	error("Unknown Rebel Assault II release '%s'", variant);
}

} // End of namespace Scumm
