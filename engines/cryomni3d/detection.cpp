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

uint8 CryOmni3DEngine::getGameType() const {
	return _gameDescription->gameType;
}

Common::Language CryOmni3DEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool CryOmni3DEngine::hasFeature(EngineFeature f) const {
	return
	    (f == kSupportsRTL)
	    || (f == kSupportsSubtitleOptions);
}

} // End of Namespace CryOmni3D

static const PlainGameDescriptor cryomni3DGames[] = {
	{"versailles", "Versailles 1685"},
	{0, 0}
};

#include "cryomni3d/detection_tables.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class CryOmni3DMetaEngine : public AdvancedMetaEngine {
public:
	CryOmni3DMetaEngine() : AdvancedMetaEngine(CryOmni3D::gameDescriptions,
		        sizeof(CryOmni3D::CryOmni3DGameDescription), cryomni3DGames, optionsList) {
		//_singleId = "cryomni3d";
		_maxScanDepth = 2;
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
	virtual int getMaximumSaveSlot() const { return 999; }
	virtual void removeSaveState(const char *target, int slot) const;
};

bool CryOmni3DMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves)
	    || (f == kSupportsLoadingDuringStartup)
	    || (f == kSupportsDeleteSave)
	    || (f == kSimpleSavesNames);
}

SaveStateList CryOmni3DMetaEngine::listSaves(const char *target) const {
	// Replicate constant here to shorten lines
	static const uint kSaveDescriptionLen = CryOmni3D::CryOmni3DEngine::kSaveDescriptionLen;
	SaveStateList saveList;

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();

	char saveName[kSaveDescriptionLen + 1];
	saveName[kSaveDescriptionLen] = '\0';
	Common::String pattern = Common::String::format("%s.????", target);
	Common::StringArray filenames = saveMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	int slotNum;

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end();
	        ++file) {
		// Obtain the last 4 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 4);

		if (slotNum >= 1 && slotNum <= 99) {
			Common::InSaveFile *in = saveMan->openForLoading(*file);
			if (in) {
				if (in->read(saveName, kSaveDescriptionLen) == kSaveDescriptionLen) {
					saveList.push_back(SaveStateDescriptor(slotNum - 1, saveName));
				}
				delete in;
			}
		}
	}

	return saveList;
}

void CryOmni3DMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%04d", target, slot + 1);

	g_system->getSavefileManager()->removeSavefile(filename);
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
