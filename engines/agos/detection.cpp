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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "engines/obsolete.h"

#include "common/system.h"
#include "common/textconsole.h"
#include "common/installshield_cab.h"

#include "agos/detection.h"
#include "agos/intern_detection.h"
#include "agos/obsolete.h" // Obsolete ID table.

/**
 * Conversion table mapping old obsolete target names to the
 * corresponding new target and platform combination.
 *
 */

static const PlainGameDescriptor agosGames[] = {
	{"pn", "Personal Nightmare"},
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

static const char *const directoryGlobs[] = {
	"execute", // Used by Simon1 Acorn CD
	0
};

using namespace AGOS;

class AgosMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	AgosMetaEngineDetection() : AdvancedMetaEngineDetection(AGOS::gameDescriptions, sizeof(AGOS::AGOSGameDescription), agosGames) {
		_guiOptions = GUIO1(GUIO_NOLAUNCHLOAD);
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	PlainGameDescriptor findGame(const char *gameId) const override {
		return Engines::findGameID(gameId, _gameIds, obsoleteGameIDsTable);
	}

	const char *getEngineId() const override {
		return "agos";
	}

	const char *getName() const override {
		return "AGOS";
	}

	const char *getOriginalCopyright() const override {
		return "AGOS (C) Adventure Soft";
	}
};

REGISTER_PLUGIN_STATIC(AGOS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AgosMetaEngineDetection);
