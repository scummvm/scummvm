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
	{"composer", "Composer Game"},
	{"darby", "Darby the Dragon"},
	{"gregory", "Gregory and the Hot Air Balloon"},
	{"liam", "Magic Tales: Liam Finds a Story"},
	{"princess", "The Princess and the Crab"},
	{"sleepingcub", "Sleeping Cub's Test of Courage"},
	{0, 0}
};

namespace Composer {

using Common::GUIO_NONE;

static const ComposerGameDescription gameDescriptions[] = {
	// from Liam Finds a Story CD
	{
		{
			"magictales",
			"Magic Tales Demo: Baby Yaga, Samurai, Imo",
			AD_ENTRY1("book.ini", "dbc98c566f4ac61b544443524585dccb"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_DEMO,
			Common::GUIO_NONE
		},
		GType_ComposerV1
	},

	{
		{
			"liam",
			0,
			AD_ENTRY1s("book.ini", "fc9d9b9e72e7301d011b808606eaa15b", 834),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ComposerV2
	},

	// from Liam Finds a Story CD
	{
		{
			"magictales",
			"Magic Tales Demo: Sleeping Cub, Princess & Crab",
			AD_ENTRY1("book.ini", "3dede2522bb0886c95667b082987a87f"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_DEMO,
			Common::GUIO_NONE
		},
		GType_ComposerV2
	},

	{
		{
			"darby",
			0,
			{
				{"book.ini", 0, "7e3404c559d058521fff2aebe5c427a8", 2545},
				{"page99.rsc", 0, "49cc6b16caa1c5ec7d94a3c47eed9a02", 1286480},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ComposerV2
	},

	{
		{
			"darby",
			0,
			AD_ENTRY1("Darby the Dragon.ini", "d81f9214936fa70d42fc578908d4bb3d"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ComposerV2
	},

	{ // Provided by Strangerke, "CD-Rom 100% Malin" Pack
		{
			"darby",
			0,
			AD_ENTRY1("book.ini", "285308372f7dddff2ca5a25c9192cf5c"),
			Common::FR_FRA,
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
			{
				{"book.ini", 0, "14a562dcf361773445255af9f3e94790", 2234},
				{"page99.rsc", 0, "01f9381162467e052dfd4c704169ef3e", 388644},
				AD_LISTEND
			},
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
			AD_ENTRY1("Gregory.ini", "fa82f14731f28c7379c5a106df07a0d6"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ComposerV2
	},

	{ // Provided by Strangerke, "CD-Rom 100% Malin" Pack
		{
			"gregory",
			0,
			AD_ENTRY1("book.ini", "e54fc5c00de5f94e908a969e445af5d0"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ComposerV2
	},

	{ // Provided by sev
		{
			"princess",
			0,
			{
				{"book.ini", 0, "fb32572577b9a41ba299825ef1e3181e", 966},
				{"page99.rsc", 0, "fd5ebd3b5e36c4651c50241619525355", 45418},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ComposerV2
	},

	{ // Provided by sev
		{
			"sleepingcub",
			0,
			{
				{"book.ini", 0, "0d329e592387009c6387a733a3ea2235", 964},
				{"page99.rsc", 0, "219fbd9bd2ff87c7023814405d753145", 46916},
				AD_LISTEND
			},
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

// we match from data too, to stop detection from a non-top-level directory
const static char *directoryGlobs[] = {
	"data",
	"programs",
	"princess",
	"sleepcub",
	0
};

class ComposerMetaEngine : public AdvancedMetaEngine {
public:
	ComposerMetaEngine() : AdvancedMetaEngine(Composer::gameDescriptions, sizeof(Composer::ComposerGameDescription), composerGames) {
		_singleid = "composer";
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getName() const {
		return "Magic Composer Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Copyright (C) 1995-1999 Animation Magic";
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
