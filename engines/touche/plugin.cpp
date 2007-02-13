/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/config-manager.h"
#include "common/advancedDetector.h"

#include "base/plugins.h"

#include "touche/touche.h"

namespace Touche {
static GameList GAME_detectGames(const FSList &fslist);
}


static const PlainGameDescriptor toucheGames[] = {
	{"touche", "Touche: The Adventures of the Fifth Musketeer"},
	{"touche-fr", "Touche: Les Aventures du Cinquieme Mousquetaire"},
	{"touche-de", "Touche: Die Abenteuer des funften Musketiers"},
	{"touche-it", "Touche: The Adventures of the Fifth Musketeer"},
	{0, 0}
};

namespace Touche {

static const Common::ADGameDescription gameDescriptions[] = {
	{ // retail version
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "2af0177f8887e3430f345e6b4d8b1414", 26350211),
		Common::EN_ANY,
		Common::kPlatformPC,
		Common::ADGF_NO_FLAGS
	},
	{ // retail version - tracker item #1601818
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "95967f0b51d2e813e99ca00325098340", 26350190),
		Common::EN_ANY,
		Common::kPlatformPC,
		Common::ADGF_NO_FLAGS
	},
	{ // retail version
		"touche-fr",
		"",
		AD_ENTRY1s("touche.dat", "1caa20bb4d4fc2ce8eb867b6610082b3", 26558232),
		Common::FR_FRA,
		Common::kPlatformPC,
		Common::ADGF_NO_FLAGS
	},
	{ // retail version - tracker item #1598643
		"touche-de",
		"",
		AD_ENTRY1s("touche.dat", "be2ae6454b3325e410946f2322547cd4", 26625537),
		Common::DE_DEU,
		Common::kPlatformPC,
		Common::ADGF_NO_FLAGS
	},
	{ // fan-made translation (http://www.iagtg.net/) - tracker item #1602360
		"touche-it",
		"",
		AD_ENTRY1s("touche.dat", "1f442331d4b327c3488a9f6ffe9bdd25", 26367792),
		Common::IT_ITA,
		Common::kPlatformPC,
		Common::ADGF_NO_FLAGS
	},
	{ // demo version
		"touche",
		"Demo",
		AD_ENTRY1s("touche.dat", "ddaed436445b2e77294ed19e8ae4aa2c", 8720683),
		Common::EN_ANY,
		Common::kPlatformPC,
		Common::ADGF_DEMO
	},
	AD_TABLE_END_MARKER
};

}

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Touche::gameDescriptions,
	// Size of that superset structure
	sizeof(Common::ADGameDescription),
	// Number of bytes to compute MD5 sum for
	4096,
	// List of all engine targets
	toucheGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"touche",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(TOUCHE, Touche::ToucheEngine, Touche::GAME_detectGames, detectionParams);

REGISTER_PLUGIN(TOUCHE, "Touche Engine", "Touche: The Adventures of the 5th Musketeer (C) Clipper Software");

namespace Touche {

bool ToucheEngine::detectGame() {
	int i = Common::AdvancedDetector::detectBestMatchingGame(detectionParams);
	if (i < 0)
		return false;

	_language = gameDescriptions[i].language;
	return true;
}

GameList GAME_detectGames(const FSList &fslist) {
	return Common::AdvancedDetector::detectAllGames(fslist, detectionParams);
}

} // End of namespace Touche
