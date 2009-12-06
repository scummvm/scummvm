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

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"

#include "base/plugins.h"

#include "asylum/asylum.h"

static const PlainGameDescriptor asylumGames[] = {
	{ "asylum", "Sanitarium" },
	{ 0, 0 }
};

namespace Asylum {

static const ADGameDescription gameDescriptions[] = {
	{
		"asylum",
		"Unpatched",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "4d13f1412f9e7dd3eaf0a58f0e00e662", 272057},
			{"SCN.006",   0, "71a5f49cbda597a11e877589075e27b5", 2918330},
			{0,0,0,0}
		},
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},
	{
		"asylum",
		"Patched", // Level 2 Lockout Bug patch was applied
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "4d13f1412f9e7dd3eaf0a58f0e00e662", 272057},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			{0,0,0,0}
		},
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},
	{
		"asylum",
		"French Version",
		{
			{"SNTRM.DAT", 0, "e09a12543c6ede1727c8ecffb74e7fd2", 8930},
			{"RES.000",   0, "2a4677ee3fd7bfa3161e16cd59371238", 277931},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			{0,0,0,0}
		},
		Common::FR_FRA,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},
	AD_TABLE_END_MARKER
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{ &gameDescriptions[0], { "SNTRM.DAT", 0 } }, // default to english version
	{ 0, { 0 } }
};

} // End of namespace Asylum

static const ADParams detectionParams = {
	(const byte *)Asylum::gameDescriptions,
	sizeof(ADGameDescription),
	0, // number of md5 bytes
	asylumGames,
	0, // no obsolete targets data
	"asylum",
	Asylum::fileBasedFallback, // file-based detection data to enable not yet known versions to start
	kADFlagPrintWarningOnFileBasedFallback,
	Common::GUIO_NONE
};

class AsylumMetaEngine : public AdvancedMetaEngine {
public:
	AsylumMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Asylum Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Sanitarium (c) ASC Games";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool AsylumMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave);
}

bool Asylum::AsylumEngine::hasFeature(EngineFeature f) const {
	return
	    (f == kSupportsRTL) ||
	    (f == kSupportsLoadingDuringRuntime) ||
	    (f == kSupportsSavingDuringRuntime) ||
	    (f == kSupportsSubtitleOptions);
}

bool AsylumMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Asylum::AsylumEngine(syst, desc->language);
	}
	return desc != 0;
}

SaveStateList AsylumMetaEngine::listSaves(const char *target) const {
	SaveStateList saveList;

	return saveList;
}

int AsylumMetaEngine::getMaximumSaveSlot() const {
	return 0;
}

void AsylumMetaEngine::removeSaveState(const char *target, int slot) const {
	// TODO
}

#if PLUGIN_ENABLED_DYNAMIC(ASYLUM)
REGISTER_PLUGIN_DYNAMIC(ASYLUM, PLUGIN_TYPE_ENGINE, AsylumMetaEngine);
#else
REGISTER_PLUGIN_STATIC(ASYLUM, PLUGIN_TYPE_ENGINE, AsylumMetaEngine);
#endif
