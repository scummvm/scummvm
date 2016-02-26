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

#include "adl/adl.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/fs.h"

#include "engines/advancedDetector.h"
#include "engines/metaengine.h"

namespace Adl {

struct AdlGameDescription {
	ADGameDescription desc;
	GameType gameType;
};

uint32 AdlEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

const char *AdlEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}

const char *const directoryGlobs[] = {
	"game",
	"datafiles",
	0
};

static const PlainGameDescriptor adlGames[] = {
	// Games
	{"hires1", "Hi-Res Adventure #1: Mystery House"},
	{0, 0}
};

static const AdlGameDescription gameDescriptions[] = {

	{ // MD5 by waltervn
		{
			"hires1", 0,
			{
				{"ADVENTURE", 0, "22d9e63a11d69fa033ba1738715ad09a", 29952},
				{"AUTO LOAD OBJ", 0, "23bfccfe9fcff9b22cf6c41bde9078ac", 12291},
				{"MYSTERY.HELLO", 0, "2289b7fea300b506e902a4c597968369", 836},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2GS, // FIXME
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeAdl1
	},
	{AD_TABLE_END_MARKER, kGameTypeNone}
};

class AdlMetaEngine : public AdvancedMetaEngine {
public:
	AdlMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(AdlGameDescription), adlGames) { }

	const char *getName() const {
		return "Hi-Res Adventure";
	}

	const char *getOriginalCopyright() const {
		return "Copyright (C) Sierra On-Line";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
};

bool AdlMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd) {
		*engine = AdlEngine::create(((const AdlGameDescription *)gd)->gameType, syst, (const AdlGameDescription *)gd);
	}
	return gd != 0;
}

} // End of namespace Adl

#if PLUGIN_ENABLED_DYNAMIC(ADL)
	REGISTER_PLUGIN_DYNAMIC(ADL, PLUGIN_TYPE_ENGINE, Adl::AdlMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ADL, PLUGIN_TYPE_ENGINE, Adl::AdlMetaEngine);
#endif
