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

#include "cryo/cryo.h"


namespace Cryo {

const char *CryoEngine::getGameId() const { return _gameDescription->gameId; }
bool CryoEngine::isDemo() const { return _gameDescription->flags & ADGF_DEMO; }
Common::Platform CryoEngine::getPlatform() const { return _gameDescription->platform; }

}

static const PlainGameDescriptor cryoGames[] = {
	{"losteden", "Lost Eden"},
	{0, 0}
};

namespace Cryo {

static const ADGameDescription gameDescriptions[] = {

	// Lost Eden PC non-interactive demo version
	// Probably not worth it
	{
		"losteden",
		0,
		AD_ENTRY1s("EDEN6.HSQ", 0, 17093),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},

	// Lost Eden PC interactive demo version
	{
		"losteden",
		0,
		AD_ENTRY1s("EDEN.DAT", 0, 205473728),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},

	// Lost Eden PC version
	{
		"losteden",
		0,
		AD_ENTRY1s("EDEN.DAT", 0, 449853776),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Lost Eden EN PC version
	// Added by Strangerke
	{
		"losteden",
		0,
		AD_ENTRY1s("EDEN.DAT", "2126f14fe38b47c7a132f7937c79a2f0", 451205552),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Lost Eden FR PC version
	// Added by Strangerke
	{
		"losteden",
		0,
		AD_ENTRY1s("EDEN.DAT", "378b1260ac400ecf35f8843357adcca6", 448040496),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Lost Eden DE PC version
	{
		"losteden",
		0,
		AD_ENTRY1s("EDEN.DAT", 0, 457719104),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Lost Eden Mac version
	{
		"losteden",
		0,
		AD_ENTRY1s("EDEN.DAT", 0, 489739536),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Cryo

class CryoMetaEngine : public AdvancedMetaEngine {
public:
	CryoMetaEngine() : AdvancedMetaEngine(Cryo::gameDescriptions, sizeof(ADGameDescription), cryoGames) {
		_singleId = "losteden";
	}

	virtual const char *getName() const {
		return "Cryo";
	}

	virtual const char *getOriginalCopyright() const {
		return "Cryo (C) Cryo Interactive";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool CryoMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool CryoMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Cryo::CryoEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(CRYO)
REGISTER_PLUGIN_DYNAMIC(CRYO, PLUGIN_TYPE_ENGINE, CryoMetaEngine);
#else
REGISTER_PLUGIN_STATIC(CRYO, PLUGIN_TYPE_ENGINE, CryoMetaEngine);
#endif
