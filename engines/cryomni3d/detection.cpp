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
#include "common/md5.h"
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

/**
 * The fallback game descriptor used by the meta engine's fallbackDetector.
 * Contents of this struct are overwritten by the fallbackDetector.
 */
static CryOmni3DGameDescription s_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO0()
	},
	0,
	0
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
		        sizeof(CryOmni3DGameDescription), cryomni3DGames, optionsList) {
		//_singleId = "cryomni3d";
		_maxScanDepth = 1;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles,
	                              const Common::FSList &fslist) const override {

		ADDetectedGame game;

		SearchMan.addDirectory("CryOmni3DMetaEngine::fallbackDetect", fslist.begin()->getParent());
		debug("Adding to SearchMan: %s", fslist.begin()->getParent().getPath().c_str());

		// Detect Versailles
		game = fallbackDetectVersailles(fslist.begin()->getParent());
		if (game.desc) {
			SearchMan.remove("CryOmni3DMetaEngine::fallbackDetect");
			return game;
		}

		SearchMan.remove("CryOmni3DMetaEngine::fallbackDetect");

		// Fallback to standard fallback detection
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

	bool addUnknownFile(const Common::FSNode &node, ADDetectedGame &game) const;

	ADDetectedGame fallbackDetectVersailles(const Common::FSNode &root) const;
};

bool CryOmni3DMetaEngine::addUnknownFile(const Common::FSNode &node, ADDetectedGame &game) const {
	Common::File testFile;
	FileProperties fileProps;

	if (!testFile.open(node)) {
		return false;
	}

	fileProps.size = (int32)testFile.size();
	fileProps.md5 = Common::computeStreamMD5AsString(testFile, _md5Bytes);

	game.hasUnknownFiles = true;
	game.matchedFiles[node.getName()] = fileProps;

	return true;
}

ADDetectedGame CryOmni3DMetaEngine::fallbackDetectVersailles(const Common::FSNode &root) const {
	debug("Checking for OBJETS/VS1.HLZ");
	if (!root.getChild("OBJETS").getChild("VS1.HLZ").exists()) {
		debug("not found");
		return ADDetectedGame();
	}
	debug("found !");

	Common::FSNode node;
	const ADGameDescription *gameDesc = &s_fallbackDesc.desc;
	ADDetectedGame game(gameDesc);

	s_fallbackDesc.desc.gameId = "versailles";
	s_fallbackDesc.desc.extra = "fallback";
	s_fallbackDesc.desc.language = Common::UNK_LANG;
	s_fallbackDesc.desc.flags = ADGF_UNSTABLE;
	s_fallbackDesc.desc.platform = Common::kPlatformUnknown;
	s_fallbackDesc.desc.guiOptions = GUI_OPTIONS_VERSAILLES;

	s_fallbackDesc.gameType = GType_VERSAILLES;

	// Sounds good, determine platform
	node = root.getChild("VERSAILL.PGM");
	if (node.exists()) {
		addUnknownFile(node, game);

		s_fallbackDesc.desc.platform = Common::kPlatformDOS;
	}
	node = root.getChild("VERSAILL.EXE");
	if (node.exists()) {
		addUnknownFile(node, game);

		s_fallbackDesc.desc.platform = Common::kPlatformWindows;
	}
	node = root.getChild("PROGRAM.Z");
	if (node.exists()) {
		addUnknownFile(node, game);

		s_fallbackDesc.desc.platform = Common::kPlatformWindows;
	}
	node = root.getChild("Versailles");
	if (node.exists()) {
		addUnknownFile(node, game);

		s_fallbackDesc.desc.platform = Common::kPlatformMacintosh;
	}

	// Determine language
	node = root.getChild("GTO").getChild("DIALOG1.GTO");
	if (node.getChild("DIALOG1.GTO").exists()) {
		s_fallbackDesc.desc.language = Common::FR_FRA;
	} else if (node.getChild("DIALOG1.ALM").exists()) {
		s_fallbackDesc.desc.language = Common::DE_DEU;
	} else if (node.getChild("DIALOG1.GB").exists()) {
		s_fallbackDesc.desc.language = Common::EN_ANY;
	} else if (node.getChild("DIALOG1.SP").exists()) {
		s_fallbackDesc.desc.language = Common::ES_ESP;
	} else if (node.getChild("DIALOG1.ITA").exists()) {
		s_fallbackDesc.desc.language = Common::IT_ITA;
	}

	// Determine game flags
	s_fallbackDesc.features = 0;
	node = root.getChild("FONTS").getChild("FONT01.CRF");
	if (node.exists()) {
		// Add file to report to let developers set appropriate game flags
		addUnknownFile(node, game);

		s_fallbackDesc.features |= GF_VERSAILLES_NUMERICFONTS;
	}

	node = root.getChild("DIAL").getChild("VOIX").getChild("ALI001__.WAV");
	if (node.exists()) {
		// Add file to report to let developers set appropriate game flags
		addUnknownFile(node, game);

		s_fallbackDesc.features |= GF_VERSAILLES_AUDIOPADDING;
	}

	return game;
}

bool CryOmni3DMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves)
	    || (f == kSupportsLoadingDuringStartup)
	    || (f == kSupportsDeleteSave)
	    || (f == kSimpleSavesNames);
}

SaveStateList CryOmni3DMetaEngine::listSaves(const char *target) const {
	// Replicate constant here to shorten lines
	static const uint kSaveDescriptionLen = CryOmni3DEngine::kSaveDescriptionLen;
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
	const CryOmni3DGameDescription *gd = (const CryOmni3DGameDescription *)desc;

	if (gd) {
		switch (gd->gameType) {
		case GType_VERSAILLES:
#ifdef ENABLE_VERSAILLES
			*engine = new Versailles::CryOmni3DEngine_Versailles(syst, gd);
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

} // End of Namespace CryOmni3D

#if PLUGIN_ENABLED_DYNAMIC(CRYOMNI3D)
REGISTER_PLUGIN_DYNAMIC(CRYOMNI3D, PLUGIN_TYPE_ENGINE, CryOmni3D::CryOmni3DMetaEngine);
#else
REGISTER_PLUGIN_STATIC(CRYOMNI3D, PLUGIN_TYPE_ENGINE, CryOmni3D::CryOmni3DMetaEngine);
#endif
