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

#define GAMEOPTION_ALT_INTRO  GUIO_GAMEOPTIONS1

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ALT_INTRO,
		{
			_s("Alternative intro"),
			_s("Use an alternative game intro (CD version only)"),
			"alt_intro",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

namespace Queen {

static const QueenGameDescription gameDescriptions[] = {
	// Amiga Demo - English
	{
		{
			"queen",
			"Demo",
			AD_ENTRY1s("queen.1", "f7a1a37ac93bf763b1569231237cb4d8", 563335),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
	},

	// Amiga Interview Demo - English
	{
		{
			"queen",
			"Interview",
			AD_ENTRY1s("queen.1", "f5d42a18d8f5689480413871410663d7", 597032),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
	},

	// DOS Demo - English
	{
		{
			"queen",
			"Demo",
			AD_ENTRY1s("queen.1", "f39334d8133840aa3bcbd733c12937cf", 3732177),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
	},

	// DOS Interview Demo - English
	{
		{
			"queen",
			"Interview",
			AD_ENTRY1s("queen.1", "30b3291f37665bf24d9482b183cb2f67", 1915913),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
	},

	// PCGAMES DOS Demo - English
	{
		{
			"queen",
			"Demo",
			AD_ENTRY1s("queen.1", "f39334d8133840aa3bcbd733c12937cf", 3724538),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
	},

#if 0
	// Amiga Floppy - English
	{
		{
			"queen",
			"Floppy",
			AD_ENTRY1s("queen.1", NULL, 351775), // TODO: Fill in correct MD5
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
	},
#endif

	// DOS Floppy - English
	{
		{
			"queen",
			"Floppy",
			AD_ENTRY1s("queen.1", "f5e827645d3c887be3bdf4729d847756", 22677657),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
	},

	// DOS CD - English
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1", "b6302bccf70463de3d5faf0f0628f742", 190787021),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},

	// DOS Floppy - French
	{
		{
			"queen",
			"Floppy",
			AD_ENTRY1s("queen.1", "f5e827645d3c887be3bdf4729d847756", 22157304),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
	},

	// DOS CD - French
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1", "6fd5486a0db75bae2e023b575c3d6a5d", 186689095),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},

#if 0
	// DOS Floppy - German
	{
		{
			"queen",
			"Floppy",
			AD_ENTRY1s("queen.1", NULL, 22240013), // TODO: Fill in correct MD5
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
	},
#endif

#if 0
	// DOS CD - German
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1", NULL, 217648975), // TODO: Fill in correct MD5
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},
#endif

#if 0
	// DOS CD - Hebrew
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1", NULL, 190705558), // TODO: Fill in correct MD5
			Common::HE_ISR,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},
#endif

#if 0
	// DOS Floppy - Italian
	{
		{
			"queen",
			"Floppy",
			AD_ENTRY1s("queen.1", NULL, 22461366), // TODO: Fill in correct MD5
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
	},
#endif

	// DOS CD - Italian
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1", "b6302bccf70463de3d5faf0f0628f742", 190795582),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},

#if 0
	// DOS CD - Spanish
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1", NULL, 190730602), // TODO: Fill in correct MD5
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},
#endif

	// DOS CD - English (Compressed Freeware Release v1.0)
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1c", "a0749bb8b72e537ead1a63a3dde1443d", 54108887),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},

	// DOS CD - English (Compressed Freeware Release v1.1)
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1c", "21fd690b372f8a6289f6f33bc986276c", 51222412),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},

	// DOS CD - French (Compressed Freeware Release v1.0)
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1c", "67e3020f8a35e1df7b1c753b5aaa71e1", 97382620),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},

	// DOS CD - German (Compressed Freeware Release v1.0)
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1c", "28f78dbec7e20f603a10c2f8ea889a5c", 108738717),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},

	// DOS CD - Hebrew (Compressed Freeware Release v1.0)
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1c", "4d52d8780613ef27a2b779caecb20a21", 99391805),
			Common::HE_ISR,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},

	// DOS CD - Italian (Compressed Freeware Release v1.0)
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1c", "2f72b715ed753cf905a37cdcc7ea611e", 98327801),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},

	// TODO: Freeware Release for Spanish DOS CD is missing.
#if 0
	// DOS CD - Spanish (Compressed Freeware Release v1.0)
	{
		{
			"queen",
			"Talkie",
			AD_ENTRY1s("queen.1c", NULL, ?),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_ALT_INTRO)
		},
	},
#endif

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Queen

class QueenMetaEngine : public AdvancedMetaEngine {
public:
	QueenMetaEngine() : AdvancedMetaEngine(Queen::gameDescriptions, sizeof(Queen::QueenGameDescription), queenGames, optionsList) {
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
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const { return 99; }
	virtual void removeSaveState(const char *target, int slot) const;

	const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const;
};

bool QueenMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

const ADGameDescription *QueenMetaEngine::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	static ADGameDescription desc;

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
				desc.gameid = "queen";
				desc.language = version.language;
				desc.platform = version.platform;
				desc.flags = ADGF_NO_FLAGS;
				desc.guioptions = GUIO0();
				if (version.features & Queen::GF_DEMO) {
					desc.extra = "Demo";
					desc.flags = ADGF_DEMO;
					desc.guioptions = GUIO_NOSPEECH;
				} else if (version.features & Queen::GF_INTERVIEW) {
					desc.extra = "Interview";
					desc.flags = ADGF_DEMO;
					desc.guioptions = GUIO_NOSPEECH;
				} else if (version.features & Queen::GF_FLOPPY) {
					desc.extra = "Floppy";
					desc.guioptions = GUIO_NOSPEECH;
				} else if (version.features & Queen::GF_TALKIE) {
					desc.extra = "Talkie";
					desc.guioptions = GAMEOPTION_ALT_INTRO;
				}
				return (const ADGameDescription *)&desc;
			}
		}
	}
	return 0;
}

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
	Common::String filename = Common::String::format("queen.s%02d", slot);

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
