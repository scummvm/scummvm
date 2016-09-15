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

#include "common/savefile.h"
#include "common/system.h"
#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "chewy/chewy.h"


namespace Chewy {

struct ChewyGameDescription {
	ADGameDescription desc;
};

uint32 ChewyEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language ChewyEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

}

static const PlainGameDescriptor chewyGames[] = {
	{"chewy", "Chewy: Esc from F5"},
	{0, 0}
};

static const char *directoryGlobs[] = {
	"txt",
	0
};

namespace Chewy {

static const ChewyGameDescription gameDescriptions[] = {

	{
		{
			"chewy",
			0,
			AD_ENTRY1s("atds.tap", "e6050c144dd4f23d79ea4f89a8ef306e", 218857),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// Chewy - ESC von F5 - German
		// Master version 1.1 (CHEWY.EXE - offset 0x8AB28)
		// The source CD-ROM has the Matrix code SONOPRESS R-7885 B
		// The disc contains several demos and files from 1996
		// Provided by lotharsm
		{
			"chewy",
			0,
			AD_ENTRY1s("atds.tap", "c117e884cc5b4bbe50ae1217d13916c4", 231071),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// Chewy - ESC von F5 - German
		// Master version 1.0 (CHEWY.EXE - offset 0x8AB10)
		// The source CD-ROM has the Matrix code SONOPRESS M-2742 A
		// CD-ROM has the label "CHEWY_V1_0"
		// Provided by lotharsm
		{
			"chewy",
			0,
			AD_ENTRY1s("atds.tap", "e22f97761c0e7772ec99660f2277b1a4", 231001),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Chewy

class ChewyMetaEngine : public AdvancedMetaEngine {
public:
	ChewyMetaEngine() : AdvancedMetaEngine(Chewy::gameDescriptions, sizeof(Chewy::ChewyGameDescription), chewyGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
		_singleId = "chewy";
	}

	virtual const char *getName() const {
		return "Chewy: Esc from F5";
	}

	virtual const char *getOriginalCopyright() const {
		return "Chewy: Esc from F5 (C) 1995 New Generation Software";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool ChewyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

bool Chewy::ChewyEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool ChewyMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Chewy::ChewyGameDescription *gd = (const Chewy::ChewyGameDescription *)desc;
	if (gd) {
		*engine = new Chewy::ChewyEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList ChewyMetaEngine::listSaves(const char *target) const {
	SaveStateList saveList;

	return saveList;
}

int ChewyMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void ChewyMetaEngine::removeSaveState(const char *target, int slot) const {
}

SaveStateDescriptor ChewyMetaEngine::querySaveMetaInfos(const char *target, int slot) const {

	return SaveStateDescriptor();
} // End of namespace Chewy

#if PLUGIN_ENABLED_DYNAMIC(CHEWY)
	REGISTER_PLUGIN_DYNAMIC(CHEWY, PLUGIN_TYPE_ENGINE, ChewyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CHEWY, PLUGIN_TYPE_ENGINE, ChewyMetaEngine);
#endif
