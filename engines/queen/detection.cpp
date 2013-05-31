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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/gui_options.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "queen/queen.h"
#include "queen/resource.h"

namespace Queen {

struct QueenGameDescription {
	ADGameDescription desc;
};

} // End of namespace Queen

static const PlainGameDescriptor queenGames[] = {
	{"queen", "Flight of the Amazon Queen"},
	{0, 0}
};

static const ExtraGuiOption queenExtraGuiOption = {
	_s("Alternative intro"),
	_s("Use an alternative game intro (CD version only)"),
	"alt_intro",
	false
};

namespace Queen {

static const QueenGameDescription gameDescriptions[] = {
	{
		{
			"queen",
			"",
			AD_ENTRY1s("FIXME", "FIXME", 0),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Queen

class QueenMetaEngine : public AdvancedMetaEngine {
public:
	QueenMetaEngine() : AdvancedMetaEngine(Queen::gameDescriptions, sizeof(Queen::QueenGameDescription), queenGames) {
		_singleid = "queen";
	}

	virtual const char *getName() const {
		return "Queen";
	}

	virtual const char *getOriginalCopyright() const {
		return "Flight of the Amazon Queen (C) John Passfield and Steve Stamatiadis";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const { return 99; }
	virtual void removeSaveState(const char *target, int slot) const;

	//const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const;
};

bool QueenMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

const ExtraGuiOptions QueenMetaEngine::getExtraGuiOptions(const Common::String &target) const {
	Common::String guiOptions;
	ExtraGuiOptions options;

	if (target.empty()) {
		options.push_back(queenExtraGuiOption);
		return options;
	}

	if (ConfMan.hasKey("guioptions", target)) {
		guiOptions = ConfMan.get("guioptions", target);
		guiOptions = parseGameGUIOptions(guiOptions);
	}

	if (!guiOptions.contains(GUIO_NOSPEECH))
		options.push_back(queenExtraGuiOption);
	return options;
}

/* FIXME - Migrate this code (Use as falllback):
GameList QueenMetaEngine::detectGames(const Common::FSList &fslist) const {
	GameList detectedGames;

	// Iterate over all files in the given directory
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory()) {
			continue;
		}
		if (file->getName().equalsIgnoreCase("queen.1") || file->getName().equalsIgnoreCase("queen.1c")) {
			Common::File dataFile;
			if (!dataFile.open(*file)) {
				continue;
			}
			Queen::DetectedGameVersion version;
			if (Queen::Resource::detectVersion(&version, &dataFile)) {
				GameDescriptor dg(queenGameDescriptor.gameid, queenGameDescriptor.description, version.language, version.platform);
				if (version.features & Queen::GF_DEMO) {
					dg.updateDesc("Demo");
					dg.setGUIOptions(GUIO_NOSPEECH);
				} else if (version.features & Queen::GF_INTERVIEW) {
					dg.updateDesc("Interview");
					dg.setGUIOptions(GUIO_NOSPEECH);
				} else if (version.features & Queen::GF_FLOPPY) {
					dg.updateDesc("Floppy");
					dg.setGUIOptions(GUIO_NOSPEECH);
				} else if (version.features & Queen::GF_TALKIE) {
					dg.updateDesc("Talkie");
				}
				detectedGames.push_back(dg);
				break;
			}
		}
	}
	return detectedGames;
}
*/

SaveStateList QueenMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	char saveDesc[32];
	Common::String pattern("queen.s??");

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				for (int i = 0; i < 4; i++)
					in->readUint32BE();
				in->read(saveDesc, 32);
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	return saveList;
}

void QueenMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = target;
	filename += Common::String::format(".s%02d", slot);

	g_system->getSavefileManager()->removeSavefile(filename);
}

bool QueenMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Queen::QueenGameDescription *gd = (const Queen::QueenGameDescription *)desc;

	if (gd)
		*engine = new Queen::QueenEngine(syst); //FIXME , gd);

	return (gd != 0);
}

#if PLUGIN_ENABLED_DYNAMIC(QUEEN)
	REGISTER_PLUGIN_DYNAMIC(QUEEN, PLUGIN_TYPE_ENGINE, QueenMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(QUEEN, PLUGIN_TYPE_ENGINE, QueenMetaEngine);
#endif
