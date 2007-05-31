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
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/engines/cruise/detection.cpp $
 * $Id:detection.cpp 26949 2007-05-26 20:23:24Z david_corrales $
 *
 */

#include "common/stdafx.h"

#include "base/plugins.h"

#include "common/advancedDetector.h"

#include "cruise/cruise.h"

namespace Cruise {

struct CRUISEGameDescription {
	Common::ADGameDescription desc;

	int gameType;
	uint32 features;
};

int CruiseEngine::getGameType() const { return _gameDescription->gameType; }
uint32 CruiseEngine::getFeatures() const { return _gameDescription->features; }
Common::Language CruiseEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform CruiseEngine::getPlatform() const { return _gameDescription->desc.platform; }

}

static const PlainGameDescriptor cruiseGames[] = {
	{"cruise", "Cinematique evo.2 engine game"},
	{"cruise", "Cruise for a corpse"},
	{0, 0}
};

static const Common::ADObsoleteGameID obsoleteGameIDsTable[] = {
	{"cruise", "cruise", Common::kPlatformUnknown},
	{0, 0, Common::kPlatformUnknown}
};

namespace Cruise {

static const CRUISEGameDescription gameDescriptions[] = {
	{
	    {
			"cruise",
			"",
			AD_ENTRY1("D1", "41a7a4d426dbd048eb369cfee4bb2717"),
			Common::FR_FRA,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_CRUISE,
		0,
    },
	{
	    {
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "a90d2b9ead6b4d812cd14268672cf178"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_CRUISE,
		0,
	},
	{AD_TABLE_END_MARKER, 0, 0}
};

}

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Cruise::gameDescriptions,
	// Size of that superset structure
	sizeof(Cruise::CRUISEGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	cruiseGames,
	// Structure for autoupgrading obsolete targets
	obsoleteGameIDsTable,
	// Name of single gameid (optional)
	"cruise",
	// List of files for file-based fallback detection (optional)
	0,
	// Fallback callback
	0,
	// Flags
	Common::kADFlagAugmentPreferredTarget
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(CRUISE, Cruise::CruiseEngine, detectionParams);

REGISTER_PLUGIN(CRUISE, "Cinematique evo 2 engine", "Cruise for a Corpse (C) Delphine Software");

namespace Cruise {

bool CruiseEngine::initGame() {
	_gameDescription = (const CRUISEGameDescription *)Common::AdvancedDetector::detectBestMatchingGame(detectionParams);

	return (_gameDescription != 0);
}

} // End of namespace Cruise
