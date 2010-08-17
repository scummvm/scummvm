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

#include "engines/advancedDetector.h"

#include "hugo/hugo.h"
#include "hugo/intro.h"

namespace Hugo {

struct HugoGameDescription {
	ADGameDescription desc;
	GameType gameType;
};

uint32 HugoEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

static const PlainGameDescriptor hugoGames[] = {
	// Games
	{"hugo1", "Hugo 1: Hugo's House of Horrors"},
	{"hugo2", "Hugo 2: Hugo's Mystery Adventure"},
	{"hugo3", "Hugo 3: Hugo's Amazon Adventure"},

	{0, 0}
};

static const HugoGameDescription gameDescriptions[] = {

	// Hugo1 DOS
	{
		{
			"hugo1", 0,
			AD_ENTRY1s("house.art", "c9403b2fe539185c9fd569b6cc4ff5ca", 14811),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		kGameTypeHugo1
	},
	// Hugo1 Windows
	{
		{
			"hugo1", 0,
			AD_ENTRY1s("objects.dat", "3ba0f108f7690a05a34c56a02fbe644a", 126488),
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_PACKED,
			Common::GUIO_NONE
		},
		kGameTypeHugo1
	},
	// Hugo2 DOS
	{
		{
			"hugo2", 0,
			AD_ENTRY1s("objects.dat", "88a718cc0ff2b3b25d49aaaa69d6d52c", 155240),
			Common::EN_ANY,
			Common::kPlatformPC,
			GF_PACKED,
			Common::GUIO_NONE
		},
		kGameTypeHugo2
	},
	// Hugo2 Windows
	{
		{
			"hugo2", 0,
			AD_ENTRY1s("objects.dat", "5df4ffc851e66a544c0e95e4e084a806", 158480),
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_PACKED,
			Common::GUIO_NONE
		},
		kGameTypeHugo2
	},
	// Hugo3 DOS
	{
		{
			"hugo3", 0,
			AD_ENTRY1s("objects.dat", "bb1b061538a445f2eb99b682c0f506cc", 136419),
			Common::EN_ANY,
			Common::kPlatformPC,
			GF_PACKED,
			Common::GUIO_NONE
		},
		kGameTypeHugo3
	},
	// Hugo3 Windows
	{
		{
			"hugo3", 0,
			AD_ENTRY1s("objects.dat", "c9a8af7aa14cc907434eecee3ddd06d3", 136638),
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_PACKED,
			Common::GUIO_NONE
		},
		kGameTypeHugo3
	},

	{AD_TABLE_END_MARKER, kGameTypeNone}
};

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)gameDescriptions,
	// Size of that superset structure
	sizeof(HugoGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	hugoGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	0,
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NONE,
	// Maximum directory depth
	1,
	// List of directory globs
	0
};

class HugoMetaEngine : public AdvancedMetaEngine {
public:
	HugoMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	const char *getName() const {
		return "Hugo Engine";
	}

	const char *getOriginalCopyright() const {
		return "Hugo Engine (C) 1989-1997 David P. Gray";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;

	bool hasFeature(MetaEngineFeature f) const;
};

bool HugoMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd) {
		*engine = new HugoEngine(syst, (const HugoGameDescription *)gd);
		((HugoEngine *)*engine)->initGame((const HugoGameDescription *)gd);
		((HugoEngine *)*engine)->initGamePart((const HugoGameDescription *)gd);
	}
	return gd != 0;
}

bool HugoMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

} // End of namespace Hugo

#if PLUGIN_ENABLED_DYNAMIC(HUGO)
REGISTER_PLUGIN_DYNAMIC(HUGO, PLUGIN_TYPE_ENGINE, Hugo::HugoMetaEngine);
#else
REGISTER_PLUGIN_STATIC(HUGO, PLUGIN_TYPE_ENGINE, Hugo::HugoMetaEngine);
#endif

namespace Hugo {

void HugoEngine::initGame(const HugoGameDescription *gd) {
	_gameType = gd->gameType;
	_platform = gd->desc.platform;
	_packedFl = (getFeatures() & GF_PACKED);
}

void HugoEngine::initGamePart(const HugoGameDescription *gd) {
	char tmpStr[8];
	_gameVariant = _gameType - 1 + (_platform == Common::kPlatformWindows ? 0 : 3);

//Generate filenames
	if (gd->desc.platform == Common::kPlatformWindows)
		sprintf(tmpStr, "%s%c", gd->desc.gameid, 'w');
	else
		sprintf(tmpStr, "%s%c", gd->desc.gameid, 'd');

	sprintf(_initFilename, "%s-00.SAV", tmpStr);
	sprintf(_saveFilename, "%s-%s.SAV", tmpStr, "%d");

	switch (_gameVariant) {
	case 0:
		_introHandler = new intro_1w(*this);
		break;
	case 1:
		_introHandler = new intro_2w(*this);
		break;
	case 2:
		_introHandler = new intro_3w(*this);
		break;
	case 3:
		_introHandler = new intro_1d(*this);
		break;
	case 4:
		_introHandler = new intro_2d(*this);
		break;
	case 5:
		_introHandler = new intro_3d(*this);
		break;
	}
}
} // End of namespace Hugo
