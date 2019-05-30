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
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "cryomni3d/cryomni3d.h"

#ifdef ENABLE_VERSAILLES
#include "cryomni3d/versailles/engine.h"
#endif

namespace CryOmni3D {

struct CryOmni3DGameDescription {
	ADGameDescription desc;

	uint8 gameType;
	uint32 features;
	const char *appName;
};

const char *CryOmni3DEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

uint32 CryOmni3DEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform CryOmni3DEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

const char *CryOmni3DEngine::getAppName() const {
	return _gameDescription->appName;
}

uint8 CryOmni3DEngine::getGameType() const {
	return _gameDescription->gameType;
}

Common::Language CryOmni3DEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool CryOmni3DEngine::hasFeature(EngineFeature f) const {
	return false;
//		(f == kSupportsRTL);
}

/*
#ifdef ENABLE_MYST

bool MohawkEngine_Myst::hasFeature(EngineFeature f) const {
    return
        MohawkEngine::hasFeature(f)
        || (f == kSupportsLoadingDuringRuntime)
        || (f == kSupportsSavingDuringRuntime);
}

#endif
*/

} // End of Namespace CryOmni3D

static const PlainGameDescriptor cryomni3DGames[] = {
	{"versailles", "Versailles 1685"},
	{0, 0}
};

#include "cryomni3d/detection_tables.h"

/*
static const char *directoryGlobs[] = {
    "all",
    "assets1",
    "data",
    "program",
    "95instal",
    "Rugrats Adventure Game",
    0
};
*/

static const ADExtraGuiOptionsMap optionsList[] = {
	/*{
	        GAMEOPTION_PLAY_MYST_FLYBY,
	        {
	                _s("Play the Myst fly by movie"),
	                _s("The Myst fly by movie was not played by the original engine."),
	                "playmystflyby",
	                false
	        }
	},*/

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class CryOmni3DMetaEngine : public AdvancedMetaEngine {
public:
	CryOmni3DMetaEngine() : AdvancedMetaEngine(CryOmni3D::gameDescriptions,
		        sizeof(CryOmni3D::CryOmni3DGameDescription), cryomni3DGames, optionsList) {
		//_singleId = "cryomni3d";
		_maxScanDepth = 2;
		//_directoryGlobs = directoryGlobs;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles,
	                              const Common::FSList &fslist) const override {
		return detectGameFilebased(allFiles, fslist, CryOmni3D::fileBased);
	}

	virtual const char *getName() const {
		return "Cryo Omni3D";
	}

	virtual const char *getOriginalCopyright() const {
		return "Cryo game Engine (C) 1997-2002 Cryo Interactive";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	SaveStateList listSavesForPrefix(const char *prefix, const char *extension) const;
	virtual int getMaximumSaveSlot() const { return 999; }
	virtual void removeSaveState(const char *target, int slot) const;
	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool CryOmni3DMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves)
	    || (f == kSupportsLoadingDuringStartup)
	    || (f == kSupportsDeleteSave)
	    || (f == kSavesSupportMetaInfo)
	    || (f == kSavesSupportThumbnail)
	    || (f == kSavesSupportCreationDate)
	    || (f == kSavesSupportPlayTime);
}

SaveStateList CryOmni3DMetaEngine::listSavesForPrefix(const char *prefix,
        const char *extension) const {
	Common::String pattern = Common::String::format("%s-###.%s", prefix, extension);
	Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
	size_t prefixLen = strlen(prefix);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end();
	        ++filename) {
		// Extract the slot number from the filename
		char slot[4];
		slot[0] = (*filename)[prefixLen + 1];
		slot[1] = (*filename)[prefixLen + 2];
		slot[2] = (*filename)[prefixLen + 3];
		slot[3] = '\0';

		int slotNum = atoi(slot);

		saveList.push_back(SaveStateDescriptor(slotNum, ""));
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());

	return saveList;
}

SaveStateList CryOmni3DMetaEngine::listSaves(const char *target) const {
	SaveStateList saveList;

	/*
	// Loading games is only supported in Myst/Riven currently.
	saveList = listSavesForPrefix("myst", "mys");

	for (SaveStateList::iterator save = saveList.begin(); save != saveList.end(); ++save) {
	    // Read the description from the save
	    int slot = save->getSaveSlot();
	    Common::String description = Mohawk::MystGameState::querySaveDescription(slot);
	    save->setDescription(description);
	}
	*/

	return saveList;
}

void CryOmni3DMetaEngine::removeSaveState(const char *target, int slot) const {

	/*
	// Removing saved games is only supported in Myst/Riven currently.
	if (strstr(target, "myst")) {
	    Mohawk::MystGameState::deleteSave(slot);
	}
	*/
}

SaveStateDescriptor CryOmni3DMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	/*
	if (strstr(target, "myst")) {
	    return Mohawk::MystGameState::querySaveMetaInfos(slot);
	}
	*/
	return SaveStateDescriptor();
}

bool CryOmni3DMetaEngine::createInstance(OSystem *syst, Engine **engine,
        const ADGameDescription *desc) const {
	const CryOmni3D::CryOmni3DGameDescription *gd = (const CryOmni3D::CryOmni3DGameDescription *)desc;

	if (gd) {
		switch (gd->gameType) {
		case CryOmni3D::GType_VERSAILLES:
#ifdef ENABLE_VERSAILLES
			*engine = new CryOmni3D::Versailles::CryOmni3DEngine_Versailles(syst, gd);
			break;
#else
			warning("Versailles support not compiled in");
			return false;
#endif
		default:
			error("Unknown Cryo Omni3D Engine");
		}
	}

	return (gd != 0);
}

#if PLUGIN_ENABLED_DYNAMIC(CRYOMNI3D)
REGISTER_PLUGIN_DYNAMIC(CRYOMNI3D, PLUGIN_TYPE_ENGINE, CryOmni3DMetaEngine);
#else
REGISTER_PLUGIN_STATIC(CRYOMNI3D, PLUGIN_TYPE_ENGINE, CryOmni3DMetaEngine);
#endif
