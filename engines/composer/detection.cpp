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
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "composer/composer.h"

namespace Composer {

struct ComposerGameDescription {
	ADGameDescription desc;

	int gameType;
};

int ComposerEngine::getGameType() const {
	return _gameDescription->gameType;
}

const char *ComposerEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}

uint32 ComposerEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language ComposerEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

}

static const PlainGameDescriptor composerGames[] = {
	{"darby", "Darby the Dragon"},
	{"gregory", "Gregory and the Hot Air Balloon"},
	{"demo1", "Magic Tales Demo 1"},
	{0, 0}
};

namespace Composer {

using Common::GUIO_NONE;

static const ComposerGameDescription gameDescriptions[] = {
	{
		{
			"demo1",
			0,
			{{"book.ini", 0, "dbc98c566f4ac61b544443524585dccb", -1},
			 AD_LISTEND},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ComposerV1
	},

	{
		{
			"darby",
			0,
			{{"install.inf", 0, "e83cc20ee18a2e138da1aadfc640dff2", -1},
			 AD_LISTEND},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ComposerV2
	},

	{
		{
			"gregory",
			0,
			{{"install.inf", 0, "b7e9d6f7949d412dad0a183375069844", -1},
			 AD_LISTEND},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ComposerV2
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Composer

using namespace Composer;

class ComposerMetaEngine : public AdvancedMetaEngine {
public:
	ComposerMetaEngine() : AdvancedMetaEngine(Composer::gameDescriptions, sizeof(Composer::ComposerGameDescription), composerGames) {}

	virtual const char *getName() const {
		return "Magic Composer Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Animation Magic";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
};

bool ComposerMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Composer::ComposerGameDescription *gd = (const Composer::ComposerGameDescription *)desc;
	if (gd) {
		*engine = new Composer::ComposerEngine(syst, gd);
	}
	return gd != 0;
}

bool ComposerMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Composer::ComposerEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsRTL);
}

#if PLUGIN_ENABLED_DYNAMIC(COMPOSER)
REGISTER_PLUGIN_DYNAMIC(COMPOSER, PLUGIN_TYPE_ENGINE, ComposerMetaEngine);
#else
REGISTER_PLUGIN_STATIC(COMPOSER, PLUGIN_TYPE_ENGINE, ComposerMetaEngine);
#endif
