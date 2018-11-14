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

#include "glk/glk.h"

#include "base/plugins.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"
#include "engines/advancedDetector.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"

#define MAX_SAVES 99

namespace Glk {

struct GlkGameDescription {
	ADGameDescription _desc;
	Common::String _filename;
	InterpreterType _interpType;
	Common::String _md5;
};

const Common::String &GlkEngine::getFilename() const {
	return _gameDescription->_filename;
}
uint32 GlkEngine::getFeatures() const {
	return _gameDescription->_desc.flags;
}

bool GlkEngine::isDemo() const {
	return (bool)(_gameDescription->_desc.flags & ADGF_DEMO);
}

Common::Language GlkEngine::getLanguage() const {
	return _gameDescription->_desc.language;
}

InterpreterType GlkEngine::getInterpreterType() const {
	return _gameDescription->_interpType;
}

const Common::String &GlkEngine::getGameMD5() const {
	return _gameDescription->_md5;
}

} // End of namespace Glk

#include "glk/frotz/detection_tables.h"
#define ZCODE(ID, NAME) { ID, Glk::Frotz::NAME##_DESC }

static const PlainGameDescriptor glkGames[] = {
	{"zcode", "Zcode Games" },
	{"scottadams", "Scott Adams Games"},

	// Infocom/Z-code games
	ZCODE("amfv", AMFV),
	ZCODE("arthur", ARTHUR),
	ZCODE("ballyhoo", BALLYHOO),
	ZCODE("beyondzork", BEYONDZORK),
	ZCODE("borderzone", BORDERZONE),
	ZCODE("bureaucracy", BUREAUCRACY),
	ZCODE("cutthroats", CUTTHROATS),
	ZCODE("deadline", DEADLINE),
	ZCODE("enchanter", ENCHANTER),
	ZCODE("hhgttg", HHGTTG),
	ZCODE("hijinx", HIJINX),
	ZCODE("infidel", INFIDEL),
	ZCODE("journey", JOURNEY),
	ZCODE("lgop", LGOP),
	ZCODE("lgop2", LGOP2),
	ZCODE("lurking", LURKING),
	ZCODE("minizork1", MINIZORK1),
	ZCODE("moonmist", MOONMIST),
	ZCODE("nordbert", NORDBERT),
	ZCODE("planetfall", PLANETFALL),
	ZCODE("plundered", PLUNDERED),
	ZCODE("sampler1", SAMPLER1),
	ZCODE("sampler2", SAMPLER2),
	ZCODE("seastalker", SEASTALKER),
	ZCODE("sherlockriddle", SHERLOCKRIDDLE),
	ZCODE("shogun", SHOGUN),
	ZCODE("sorcerer", SORCERER),
	ZCODE("spellbreaker", SPELLBREAKER),
	ZCODE("starcross", STARCROSS),
	ZCODE("stationfall", STATIONFALL),
	ZCODE("suspect", SUSPECT),
	ZCODE("suspended", SUSPENDED),
	ZCODE("trinity", TRINITY),
	ZCODE("wishbringer", WISHBRINGER),
	ZCODE("witness", WITNESS),
	ZCODE("zork0", ZORK0),
	ZCODE("zork1", ZORK1),
	ZCODE("zork2", ZORK2),
	ZCODE("zork3", ZORK3),
	ZCODE("ztuu", ZTUU),

	// Scott Adams games
	{ "adventureland", "Adventureland" },
	{ "pirateadventure", "Pirate Adventure" },
	{ "missionimpossible", "Mission Impossible" },
	{ "voodoocastle", "Voodoo Castle" },
	{ "thecount", "The Count" },
	{ "strangeodyssey", "Strange Odyssey" },
	{ "mysteryfunhouse", "Mystery Fun House" },
	{ "pyramidofdoom", "Pyramid Of Doom" },
	{ "ghosttown", "Ghost Town" },
	{ "savageisland1", "Savage Island, Part 1" },
	{ "savageisland2", "Savage Island, Part 2" },
	{ "goldenvoyage", "The Golden Voyage" },
	{ "adventure13", "Adventure 13" },
	{ "adventure14", "Adventure 14" },
	{ "buckaroobonzai", "Buckaroo Banzai" },
	{ "marveladventure", "Marvel Adventure #1" },
	{ "scottsampler", "Adventure International's Mini-Adventure Sampler" },
	{0, 0}
};

#include "common/config-manager.h"
#include "common/file.h"
#include "glk/detection_tables.h"
#include "glk/frotz/detection.h"
#include "glk/frotz/frotz.h"
#include "glk/scott/detection.h"
#include "glk/scott/scott.h"

class GlkMetaEngine : public AdvancedMetaEngine {
public:
	GlkMetaEngine() : AdvancedMetaEngine(Glk::gameDescriptions, sizeof(Glk::GlkGameDescription), glkGames) {
		_maxScanDepth = 3;
	}

	virtual const char *getName() const {
		return "ScummGlk Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "ScummGlk Engine (c) 2018";
	}

	virtual bool hasFeature(MetaEngineFeature f) const override;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;

	virtual DetectedGames detectGames(const Common::FSList &fslist) const override;

	virtual ADDetectedGames detectGame(const Common::FSNode &parent, const FileMap &allFiles, Common::Language language, Common::Platform platform, const Common::String &extra) const override;
};

bool GlkMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportCreationDate) ||
	    (f == kSavesSupportPlayTime) ||
	    (f == kSimpleSavesNames);
}

bool Glk::GlkEngine::hasFeature(EngineFeature f) const {
	return
	    (f == kSupportsRTL) ||
	    (f == kSupportsLoadingDuringRuntime) ||
	    (f == kSupportsSavingDuringRuntime);
}

bool GlkMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Glk::GlkGameDescription *gd = (const Glk::GlkGameDescription *)desc;

	switch (gd->_interpType) {
	case Glk::INTERPRETER_FROTZ:
		*engine = new Glk::Frotz::Frotz(syst, gd);
		break;
	case Glk::INTERPRETER_SCOTT:
		*engine = new Glk::Scott::Scott(syst, gd);
		break;
	default:
		error("Unknown interpreter");
	}

	return gd != 0;
}

SaveStateList GlkMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);
	Glk::SavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot <= MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (Glk::FileStream::readSavegameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(slot, header._saveName));

				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int GlkMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void GlkMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor GlkMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename);

	if (in) {
		Glk::SavegameHeader header;
		if (Glk::FileStream::readSavegameHeader(in, header)) {
			// Create the return descriptor
			SaveStateDescriptor desc(slot, header._saveName);
			desc.setSaveDate(header._year, header._month, header._day);
			desc.setSaveTime(header._hour, header._minute);
			desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);

			delete in;
			return desc;
		}
	}

	return SaveStateDescriptor();
}

DetectedGames GlkMetaEngine::detectGames(const Common::FSList &fslist) const {
	DetectedGames detectedGames;
	Glk::Frotz::FrotzMetaEngine::detectGames(fslist, detectedGames);
	Glk::Scott::ScottMetaEngine::detectGames(fslist, detectedGames);

	return detectedGames;
}

static Glk::GlkGameDescription gameDescription;

ADDetectedGames GlkMetaEngine::detectGame(const Common::FSNode &parent, const FileMap &allFiles, Common::Language language, Common::Platform platform, const Common::String &extra) const {
	static char gameId[100];
	strcpy(gameId, ConfMan.get("gameid").c_str());
	Common::String filename = ConfMan.get("filename");

	Common::FSList fslist;
	DetectedGames detectedGames;
	fslist.push_back(parent.getChild(filename));
	ADDetectedGames results;
	Common::File f;

	// Check each sub-engine for any detected games
	if (Glk::Frotz::FrotzMetaEngine::detectGames(fslist, detectedGames))
		gameDescription._interpType = Glk::INTERPRETER_FROTZ;
	else if (Glk::Scott::ScottMetaEngine::detectGames(fslist, detectedGames))
		gameDescription._interpType = Glk::INTERPRETER_SCOTT;
	else
		// No match found, so return no results
		return results;

	// Set up the game description and return it
	if (f.open(parent.getChild(filename))) {
		DetectedGame gd = detectedGames.front();

		gameDescription._desc.gameId = gameId;
		gameDescription._desc.language = gd.language;
		gameDescription._desc.platform = gd.platform;
		gameDescription._desc.guiOptions = GUIO4(GUIO_NOSPEECH, GUIO_NOSFX, GUIO_NOMUSIC, GUIO_NOSUBTITLES);
		gameDescription._filename = filename;
		gameDescription._md5 = Common::computeStreamMD5AsString(f, 5000);

		ADDetectedGame dg((ADGameDescription *)&gameDescription);
		results.push_back(dg);
	}

	return results;
}

#if PLUGIN_ENABLED_DYNAMIC(GLK)
REGISTER_PLUGIN_DYNAMIC(GLK, PLUGIN_TYPE_ENGINE, GlkMetaEngine);
#else
REGISTER_PLUGIN_STATIC(GLK, PLUGIN_TYPE_ENGINE, GlkMetaEngine);
#endif
