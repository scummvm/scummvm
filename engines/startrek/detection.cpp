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
 * $URL: https://scummvm-startrek.googlecode.com/svn/trunk/detection.cpp $
 * $Id: detection.cpp 15 2010-06-27 06:13:42Z clone2727 $
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/config-manager.h"
#include "common/file.h"

#include "startrek/startrek.h"

namespace StarTrek {

struct StarTrekGameDescription {
	ADGameDescription desc;

	uint8 gameType;
	uint32 features;
	uint16 version;
};

uint32 StarTrekEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform StarTrekEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 StarTrekEngine::getVersion() const {
	return _gameDescription->version;
}

uint8 StarTrekEngine::getGameType() {
	return _gameDescription->gameType;
}

Common::Language StarTrekEngine::getLanguage() {
	return _gameDescription->desc.language;
}

} // End of Namespace StarTrek

static const PlainGameDescriptor starTrekGames[] = {
	{"startrek", "Star Trek game"},
	{"st25", "Star Trek: 25th Anniversary"},
	{"stjr", "Star Trek: Judgment Rites"},
	{0, 0}
};


namespace StarTrek {

static const StarTrekGameDescription gameDescriptions[] = {
	{
		{
			"st25",
			"",
			AD_ENTRY1("data.001", "57040928a0f374281aa86ba4e7db8444"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_ST25,
		0,
		0,
	},
	
	{
		{
			"st25",
			"",
			AD_ENTRY1("data.000", "f0918b6d096455ce2ae6dd5ef973292e"),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_ST25,
		0,
		0,
	},
	
	{
		{
			"st25",
			"",
			AD_ENTRY1("data.000", "70d0e374d5fa973e536dba0f42310672"),
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_ST25,
		0,
		0,
	},
	
	{
		{
			"st25",
			"",
			AD_ENTRY1("data.000", "d0299af1385edd7c7612ed453e417dd8"),
			Common::FR_FRA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_ST25,
		0,
		0,
	},
	
	{
		{
			"st25",
			"",
			AD_ENTRY1("Star Trek Data", "871fa51c7680c0a43df9622128f1569f"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO0()
		},
		GType_ST25,
		0,
		0,
	},
	
	{
		{
			"st25",
			"Demo",
			AD_ENTRY1("data.001", "f68126e7e36ce6286c4c8575f8b594f5"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO0()
		},
		GType_ST25,
		GF_DEMO,
		0,
	},

	{
		{
			"st25",
			"Demo",
			AD_ENTRY1("Star Trek Data", "88f88b81d389a3217fc7efd6ee631c42"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK | ADGF_DEMO,
			GUIO0()
		},
		GType_ST25,
		GF_DEMO,
		0,
	},
	
	{
		{
			"stjr",
			"",
			AD_ENTRY1("data.001", "1c8de3c02f69c07c582d59d3c29e4dd9"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_STJR,
		0,
		0,
	},	
	
	{ AD_TABLE_END_MARKER, 0, 0, 0 }
};

} // End of namespace StarTrek

class StarTrekMetaEngine : public AdvancedMetaEngine {
public:
	StarTrekMetaEngine() : AdvancedMetaEngine(StarTrek::gameDescriptions, sizeof(StarTrek::StarTrekGameDescription), starTrekGames) {
		_singleId = "startrek";
	}

	virtual const char *getName() const {
		return "Star Trek Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Star Trek: 25th Anniversary, Star Trek: Judgment Rites (C) Interplay";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool StarTrekMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const StarTrek::StarTrekGameDescription *gd = (const StarTrek::StarTrekGameDescription *)desc;
	
	*engine = new StarTrek::StarTrekEngine(syst, gd);
	
	return (gd != 0);
}

#if PLUGIN_ENABLED_DYNAMIC(STARTREK)
	REGISTER_PLUGIN_DYNAMIC(STARTREK, PLUGIN_TYPE_ENGINE, StarTrekMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(STARTREK, PLUGIN_TYPE_ENGINE, StarTrekMetaEngine);
#endif

