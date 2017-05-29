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
#include "common/debug.h"

#include "engines/advancedDetector.h"

#include "sludge/sludge.h"

namespace Sludge {

struct SludgeGameDescription {
	ADGameDescription desc;

	int gameType;
};

int SludgeEngine::getGameType() const { return _gameDescription->gameType; }
const char *SludgeEngine::getGameId() const { return _gameDescription->desc.gameId;}
uint32 SludgeEngine::getFeatures() const { return _gameDescription->desc.flags; }
Common::Language SludgeEngine::getLanguage() const { return _gameDescription->desc.language; }

} // End of namespace Sludge

static const PlainGameDescriptor sludgeGames[] = {
	{ "sludge", "Sludge Game" },
	{ "welcome", "Welcome Example" },
	{ "welcome2", "Welcome Example 2" },
	{ "welcomePng", "Welcome For PNG Test" },
	{ "verbcoin", "Verb Coin" },
	{ 0, 0 }
};
 
namespace Sludge {
static const SludgeGameDescription gameDescriptions[] = {
	{
		{
			"welcome",
			"",
			AD_ENTRY1("Welcome.slg", "50445503761cf6684fe3270d0860a4c3"),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"welcome2",
			"",
			AD_ENTRY1("Welcome.slg", "cb1f307c05b8ae4107bcc7f86a3d2f99"),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"welcomePng",
			"",
			AD_ENTRY1("Welcome.slg", "ff0d7ef0ce73b5503ef6b965928af6d6"),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"verbcoin",
			"",
			AD_ENTRY1("Welcome.slg", "e39ec315dcbf3a1137481f0a5fe1617d"),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Sludge

class SludgeMetaEngine : public AdvancedMetaEngine {
public:
	SludgeMetaEngine() : AdvancedMetaEngine(Sludge::gameDescriptions, sizeof(Sludge::SludgeGameDescription), sludgeGames) {
		_singleId = "sludge";
		_maxScanDepth = 1;
	}

	virtual const char *getName() const {
		return "Sludge Engine";
	}
 
	virtual const char *getOriginalCopyright() const {
		return "Copyright (C) 2000-2014 Hungry Software and contributors";
	}
 
 
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		const Sludge::SludgeGameDescription *gd = (const Sludge::SludgeGameDescription *)desc;
			if (gd) {
				*engine = new Sludge::SludgeEngine(syst, gd);
			}
			return gd != 0;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(SLUDGE)
	REGISTER_PLUGIN_DYNAMIC(SLUDGE, PLUGIN_TYPE_ENGINE, SludgeMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SLUDGE, PLUGIN_TYPE_ENGINE, SludgeMetaEngine);
#endif
