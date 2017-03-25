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
#include "common/file.h"

#include "mario/mario.h"


namespace Mario {

const char *MarioGame::getGameId() const { return _gameDescription->gameId; }
Common::Platform MarioGame::getPlatform() const { return _gameDescription->platform; }

}

static const PlainGameDescriptor marioGames[] = {
	{"mario", "Mario and a wet job"},
	{0, 0}
};

namespace Mario {

static const ADGameDescription gameDescriptions[] = {

	// Mario PC version
	{
		"mario",
		0,
		AD_ENTRY1s("GAME.BIN", 0, 41622),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	/*
	// Mario 3DO version
	{
		"mario",
		0,
		AD_ENTRY1s("launchme", 0, 143300),
		Common::EN_ANY,
		Common::kPlatform3DO,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	*/

	AD_TABLE_END_MARKER
};

} // End of namespace Mario

class MarioMetaEngine : public AdvancedMetaEngine {
public:
	MarioMetaEngine() : AdvancedMetaEngine(Mario::gameDescriptions, sizeof(ADGameDescription), marioGames) {
		_singleId = "mario";
	}

	virtual const char *getName() const {
		return "Mario Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Mario Engine (C) Deep Sewers Production";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool MarioMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool MarioMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Mario::MarioGame(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(MARIO)
REGISTER_PLUGIN_DYNAMIC(MARIO, PLUGIN_TYPE_ENGINE, MarioMetaEngine);
#else
REGISTER_PLUGIN_STATIC(MARIO, PLUGIN_TYPE_ENGINE, MarioMetaEngine);
#endif
