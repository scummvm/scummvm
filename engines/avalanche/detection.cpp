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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/savefile.h"
#include "graphics/thumbnail.h"

#include "avalanche/avalanche.h"

namespace Avalanche {

struct AvalancheGameDescription {
	ADGameDescription desc;
};

uint32 AvalancheEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

const char *AvalancheEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}

static const PlainGameDescriptor avalancheGames[] = {
	{"avalot", "Lord Avalot d'Argent"},
	{0, 0}
};

static const ADGameDescription gameDescriptions[] = {
	{
		"Avalanche", 0,
		{
			{"avalot.sez", 0, "de10eb353228013da3d3297784f81ff9", 48763},
			{"mainmenu.avd", 0, "89f31211af579a872045b175cc264298", 18880},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

class AvalancheMetaEngine : public AdvancedMetaEngine {
public:
	AvalancheMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(AvalancheGameDescription), avalancheGames) {
	}

	const char *getName() const {
		return "Avalanche";
	}

	const char *getOriginalCopyright() const {
		return "Avalanche Engine Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
	bool hasFeature(MetaEngineFeature f) const;
};

bool AvalancheMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd)
		*engine = new AvalancheEngine(syst, (const AvalancheGameDescription *)gd);
	return gd != 0;
}

bool AvalancheMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

} // End of namespace Avalanche

#if PLUGIN_ENABLED_DYNAMIC(AVALANCHE)
REGISTER_PLUGIN_DYNAMIC(AVALANCHE, PLUGIN_TYPE_ENGINE, Avalanche::AvalancheMetaEngine);
#else
REGISTER_PLUGIN_STATIC(AVALANCHE, PLUGIN_TYPE_ENGINE, Avalanche::AvalancheMetaEngine);
#endif
