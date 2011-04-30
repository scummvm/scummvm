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

#include "m4/m4.h"
#include "m4/resource.h"

namespace M4 {

struct M4GameDescription {
	ADGameDescription desc;

	int gameType;
	uint32 features;
};

int MadsM4Engine::getGameType() const { return _gameDescription->gameType; }
uint32 MadsM4Engine::getFeatures() const { return _gameDescription->features; }
Common::Language MadsM4Engine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform MadsM4Engine::getPlatform() const { return _gameDescription->desc.platform; }

} // End of namespace M4

static const PlainGameDescriptor m4Games[] = {
	{"m4", "MADS/M4 engine game"},
	{"riddle", "Ripley's Believe It or Not!: The Riddle of Master Lu"},
	{"burger", "Orion Burger"},
	{"rex", "Rex Nebular and the Cosmic Gender Bender"},
	{"dragon", "DragonSphere"},
	{"dragoncd", "DragonSphere CD"},
	{"phantom", "Return of the Phantom"},
	{"phantomcd", "Return of the Phantom CD"},
	{0, 0}
};

namespace M4 {

const char *MadsM4Engine::getGameFile(int fileType) {
	for (int i = 0; _gameDescription->desc.filesDescriptions[i].fileName; i++) {
		if (_gameDescription->desc.filesDescriptions[i].fileType == fileType)
			return _gameDescription->desc.filesDescriptions[i].fileName;
	}
	return NULL;
}

using Common::GUIO_NONE;
using Common::GUIO_NOSPEECH;

static const M4GameDescription gameDescriptions[] = {
	{
		{
			"burger",
			"",
			{
				{ "burger.has",	kFileTypeHash, "10c8064e9c771072122f50737ac97245", 730771},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Burger,
		kFeaturesCD
	},
	{
		{
			"burger",
			"",
			{
				{ "burger.has",	kFileTypeHash, "55be8693a4c36e7efcdca0f0c77758ae", 724191},
				{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Burger,
		kFeaturesCD
	},
	{
		{
			"burger",
			"",
			{
				{ "burger.has",	kFileTypeHash, "795c98a74e351ec437a396bb29897daf", 730771},
				{ NULL, 0, NULL, 0}
			},
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Burger,
		kFeaturesCD
	},
	{
		{
			"burger",
			"Demo",
			{
				{ "burger.has",	kFileTypeHash, "fc3f363b6153240a448bd3b7be9318da", 62159},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Burger,
		kFeaturesDemo
	},
	{
		{
			"burger",
			"Demo",
			{
				{ "overview.has",	kFileTypeHash, "57aa43a3ef88a934a43e9b1890ef5e17", 10519},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Burger,
		kFeaturesDemo
	},
	{
		{
			"riddle",
			"",
			{
				{ "ripley.has",	kFileTypeHash, "056d517360c89eb4c297a319f21a7071", 700469},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Riddle,
		kFeaturesCD
	},
	{
		{
			"riddle",
			"",
			{
				{ "ripley.has",	kFileTypeHash, "d073582c9011d44dd0d7e2ede317a86d", 700469},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Riddle,
		kFeaturesCD
	},
	{
		{
			"riddle",
			"",
			{
				{ "ripley.has",	kFileTypeHash, "d9e9f8befec432a047b1047fb2bc7aea", 710997},
				{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Riddle,
		kFeaturesCD
	},
	{
		{
			"riddle",
			"",
			{
				{ "ripley.has",	kFileTypeHash, "3d48c5700785d11e6a5bc832b95be918", 701973},
				{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Riddle,
		kFeaturesCD
	},
	{
		{
			"riddle",
			"",
			{
				{ "ripley.has",	kFileTypeHash, "5ee011cff7178dae3ddf6f9b7d4102ac", 701691},
				{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Riddle,
		kFeaturesCD
	},
	{	// Demo
		{
			"riddle",
			"Demo",
			{
				{ "ripley.has",	kFileTypeHash, "3a90dd7052860b6e246ec7e0aaf202f6", 104527},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Riddle,
		kFeaturesDemo
	},
	{
		{
			"rex",
			"",
			{
				{ "global.hag",	kFileTypeHAG, "0530cbeee109fc79cc24421128dea1ce", 2083078},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NOSPEECH
		},
		GType_RexNebular,
		kFeaturesNone
	},
	{	// Demo
		{
			"rex",
			"Demo",
			{
				{ "global.hag",	kFileTypeHAG, "d5a481d14bc1bda66e46965a39badcc7", 220429},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_RexNebular,
		kFeaturesDemo
	},
	{
		{
			"dragon",
			"",
			{
				{ "global.hag",	kFileTypeHAG, "c3a6877665e7f21bf3d2b1e667155562", 2320567},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NOSPEECH
		},
		GType_DragonSphere,
		kFeaturesNone
	},
	{	// CD version
		{
			"dragoncd",
			"",
			{
				{ "global.hag",	kFileTypeHAG, "c3a6877665e7f21bf3d2b1e667155562", 2320567},
				{ "speech.hag",	kFileTypeHAG, NULL, -1},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_DragonSphere,
		kFeaturesCD
	},
	{	// Demo
		{
			"dragon",
			"Demo",
			{
				{ "global.hag",	kFileTypeHAG, "541e12d8e9aad0c65d65f150de47582e", 212718},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_DragonSphere,
		kFeaturesDemo
	},
	{
		{
			"phantom",
			"",
			{
				{ "global.hag",	kFileTypeHAG, "bdce9ca93a015f0883d1bc0fabd0cdfa", 812150},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NOSPEECH
		},
		GType_Phantom,
		kFeaturesNone
	},
	{	// CD version
		{
			"phantomcd",
			"",
			{
				{ "global.hag",	kFileTypeHAG, "8a51c984eb4c64e8b30a7e6670f6bddb", 101154000},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Phantom,
		kFeaturesCD
	},
	{	// CD version
		{
			"phantom",
			"Demo",
			{
				{ "global.hag",	kFileTypeHAG, "e810adbc6fac77ac2fec841a9ec5e20e", 115266},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO_NONE
		},
		GType_Phantom,
		kFeaturesDemo
	},
	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace M4

static const char *directoryGlobs[] = {
	"option1",
	0
};

class M4MetaEngine : public AdvancedMetaEngine {
public:
	M4MetaEngine() : AdvancedMetaEngine(M4::gameDescriptions, sizeof(M4::M4GameDescription), m4Games) {
		_singleid = "m4";
		_guioptions = Common::GUIO_NOMIDI;
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getName() const {
		return "MADS/M4";
	}

	virtual const char *getOriginalCopyright() const {
		return "Riddle of Master Lu & Orion Burger (C) Sanctuary Woods";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool M4MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const M4::M4GameDescription *gd = (const M4::M4GameDescription *)desc;
	if (gd) {
		if ((gd->gameType == M4::GType_Burger) || (gd->gameType == M4::GType_Riddle))
			*engine = new M4::M4Engine(syst, gd);
		else
			*engine = new M4::MadsEngine(syst, gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(M4)
	REGISTER_PLUGIN_DYNAMIC(M4, PLUGIN_TYPE_ENGINE, M4MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(M4, PLUGIN_TYPE_ENGINE, M4MetaEngine);
#endif
