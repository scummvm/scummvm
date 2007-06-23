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
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/engines/agos/detection.cpp $
 * $Id:detection.cpp 26949 2007-05-26 20:23:24Z david_corrales $
 *
 */

#include "common/stdafx.h"

#include "base/plugins.h"

#include "common/advancedDetector.h"
#include "common/config-manager.h"

#include "agos/agos.h"

namespace AGOS {

struct AGOSGameDescription {
	Common::ADGameDescription desc;

	int gameType;
	int gameId;
	uint32 features;
};

}

/**
 * Conversion table mapping old obsolete target names to the
 * corresponding new target and platform combination.
 *
 */
static const Common::ADObsoleteGameID obsoleteGameIDsTable[] = {
	{"simon1acorn", "simon1", Common::kPlatformAcorn},
	{"simon1amiga", "simon1", Common::kPlatformAmiga},
	{"simon1cd32", "simon1", Common::kPlatformAmiga},
	{"simon1demo", "simon1", Common::kPlatformPC},
	{"simon1dos", "simon1", Common::kPlatformPC},
	{"simon1talkie", "simon1", Common::kPlatformPC},
	{"simon1win", "simon1", Common::kPlatformWindows},
	{"simon2dos", "simon2",  Common::kPlatformPC},
	{"simon2talkie", "simon2", Common::kPlatformPC},
	{"simon2mac", "simon2", Common::kPlatformMacintosh},
	{"simon2win", "simon2",  Common::kPlatformWindows},
	{0, 0, Common::kPlatformUnknown}
};

static const PlainGameDescriptor simonGames[] = {
	{"elvira1", "Elvira - Mistress of the Dark"},
	{"elvira2", "Elvira II - The Jaws of Cerberus"},
	{"waxworks", "Waxworks"},
	{"simon1", "Simon the Sorcerer 1"},
	{"simon2", "Simon the Sorcerer 2"},
	{"feeble", "The Feeble Files"},
	{"dimp", "Demon in my Pocket"},
	{"jumble", "Jumble"},
	{"puzzle", "NoPatience"},
	{"swampy", "Swampy Adventures"},
	{0, 0}
};

#include "agos/detection_tables.h"

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)AGOS::gameDescriptions,
	// Size of that superset structure
	sizeof(AGOS::AGOSGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	simonGames,
	// Structure for autoupgrading obsolete targets
	obsoleteGameIDsTable,
	// Name of single gameid (optional)
	0,
	// List of files for file-based fallback detection (optional)
	0,
	// Fallback callback
	0,
	// Flags
	Common::kADFlagAugmentPreferredTarget
};

GameList Engine_AGOS_gameIDList() {
	return GameList(simonGames);
}
 
GameDescriptor Engine_AGOS_findGameID(const char *gameid) {
	return Common::AdvancedDetector::findGameID(gameid, simonGames, obsoleteGameIDsTable);
}

GameList Engine_AGOS_detectGames(const FSList &fslist) {
	return Common::AdvancedDetector::detectAllGames(fslist, detectionParams);
}

PluginError Engine_AGOS_create(OSystem *syst, Engine **engine) {
	assert(engine);
	const char *gameid = ConfMan.get("gameid").c_str();
	
	//Common::EncapsulatedADGameDesc encapsulatedDesc = Common::AdvancedDetector::detectBestMatchingGame(detectionParams);
	//const AGOSGameDescription *gd = (const AGOSGameDescription *)(encapsulatedDesc.realDesc);
	//if (gd == 0) {
	//	return kNoGameDataFoundError;
	//}

	if (!scumm_stricmp("elvira1", gameid)) {
		*engine = new AGOS::AGOSEngine_Elvira1(syst);
	} else if (!scumm_stricmp("elvira2", gameid)) {
		*engine = new AGOS::AGOSEngine_Elvira2(syst);
	} else if (!scumm_stricmp("waxworks", gameid)) {
		*engine = new AGOS::AGOSEngine_Waxworks(syst);
	} else if (!scumm_stricmp("simon1", gameid)) {
		*engine = new AGOS::AGOSEngine_Simon1(syst);
	} else if (!scumm_stricmp("simon2", gameid)) {
		*engine = new AGOS::AGOSEngine_Simon2(syst);
	} else if (!scumm_stricmp("feeble", gameid)) {
		*engine = new AGOS::AGOSEngine_Feeble(syst);
	} else if (!scumm_stricmp("dimp", gameid)) {
		*engine = new AGOS::AGOSEngine_PuzzlePack(syst);
	} else if (!scumm_stricmp("jumble", gameid)) {
		*engine = new AGOS::AGOSEngine_PuzzlePack(syst);
	} else if (!scumm_stricmp("puzzle", gameid)) {
		*engine = new AGOS::AGOSEngine_PuzzlePack(syst);
	} else if (!scumm_stricmp("swampy", gameid)) {
		*engine = new AGOS::AGOSEngine_PuzzlePack(syst);
	} else {
		error("AGOS engine created with invalid gameid");
	}

	return kNoError;
}
 
REGISTER_PLUGIN(AGOS, "AGOS", "AGOS (C) Adventure Soft");

namespace AGOS {

bool AGOSEngine::initGame() {
	Common::EncapsulatedADGameDesc encapsulatedDesc = Common::AdvancedDetector::detectBestMatchingGame(detectionParams);
	_gameDescription = (const AGOSGameDescription *)(encapsulatedDesc.realDesc);

	return (_gameDescription != 0);
}


int AGOSEngine::getGameId() const {
	return _gameDescription->gameId;
}

int AGOSEngine::getGameType() const {
	return _gameDescription->gameType;
}

uint32 AGOSEngine::getFeatures() const {
	return _gameDescription->features;
}

const char *AGOSEngine::getExtra() const {
	return _gameDescription->desc.extra;
}

Common::Language AGOSEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform AGOSEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

const char *AGOSEngine::getFileName(int type) const { 
	for (int i = 0; _gameDescription->desc.filesDescriptions[i].fileType; i++) {
		if (_gameDescription->desc.filesDescriptions[i].fileType == type)
			return _gameDescription->desc.filesDescriptions[i].fileName; 
	}
	return NULL;
}

} // End of namespace AGOS
