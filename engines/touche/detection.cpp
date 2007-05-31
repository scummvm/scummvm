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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/engines/touche/detection.cpp $
 * $Id:detection.cpp 26949 2007-05-26 20:23:24Z david_corrales $
 *
 */

#include "common/stdafx.h"
#include "common/config-manager.h"
#include "common/advancedDetector.h"

#include "base/plugins.h"

#include "touche/touche.h"

static const PlainGameDescriptor toucheGames[] = {
	{ "touche", "Touche: The Adventures of the Fifth Musketeer" },
	{ 0, 0 }
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
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "1caa20bb4d4fc2ce8eb867b6610082b3", 26558232),
		Common::FR_FRA,
		Common::kPlatformPC,
		Common::ADGF_NO_FLAGS
	},
	{ // retail version - tracker item #1598643
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "be2ae6454b3325e410946f2322547cd4", 26625537),
		Common::DE_DEU,
		Common::kPlatformPC,
		Common::ADGF_NO_FLAGS
	},
	{ // retail version - tracker item #1681643
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "64e95ba1decf5a5a60f8fa1840f40c62", 26529523),
		Common::ES_ESP,
		Common::kPlatformPC,
		Common::ADGF_NO_FLAGS
	},
	{ // fan-made translation (http://www.iagtg.net/) - tracker item #1602360
		"touche",
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

static const Common::ADFileBasedFallback fileBasedFallback[] = {
	{ &gameDescriptions[0], { "touche.dat", 0 } }, // default to english version
	{ 0, { 0 } }
};

}

static const Common::ADParams detectionParams = {
	(const byte *)Touche::gameDescriptions,
	sizeof(Common::ADGameDescription),
	4096, // number of md5 bytes
	toucheGames,
	0, // no obsolete targets data
	"touche",
	Touche::fileBasedFallback, // file-based detection data to enable not yet known versions to start
	0, // no fallback callback
	Common::kADFlagAugmentPreferredTarget | Common::kADFlagPrintWarningOnFileBasedFallback
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(TOUCHE, Touche::ToucheEngine, detectionParams);

REGISTER_PLUGIN(TOUCHE, "Touche Engine", "Touche: The Adventures of the 5th Musketeer (C) Clipper Software");

namespace Touche {

bool ToucheEngine::detectGame() {
	const Common::ADGameDescription *gd = Common::AdvancedDetector::detectBestMatchingGame(detectionParams);
	if (gd == 0)
		return false;

	_language = gd->language;
	return true;
}

} // End of namespace Touche
