/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"

#include "agos/agos.h"

namespace AGOS {
static DetectedGameList GAME_detectGames(const FSList &fslist);
}

using Common::File;

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
	{NULL, NULL, Common::kPlatformUnknown}
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
	{NULL, NULL}
};

ADVANCED_DETECTOR_GAMEID_LIST(AGOS, simonGames);

ADVANCED_DETECTOR_FIND_GAMEID(AGOS, simonGames, obsoleteGameIDsTable);

ADVANCED_DETECTOR_DETECT_GAMES(AGOS, AGOS::GAME_detectGames);

ADVANCED_DETECTOR_ENGINE_CREATE(AGOS, AGOS::AGOSEngine, "AGOSEngine", obsoleteGameIDsTable);

REGISTER_PLUGIN(AGOS, "AGOS", "AGOS (C) Adventure Soft");

namespace AGOS {

using Common::ADGameFileDescription;
using Common::ADGameDescription;

#include "agosgame.cpp"

ADVANCED_DETECTOR_TO_DETECTED_GAME(simonGames);

ADVANCED_DETECTOR_DETECT_INIT_GAME(AGOSEngine::initGame, gameDescriptions, _gameDescription, Common::ADTrue);

ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(GAME_detectGames, gameDescriptions);

} // End of namespace AGOS
