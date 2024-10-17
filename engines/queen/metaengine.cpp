/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/advancedDetector.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/detection.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ALT_INTRO,
		{
			_s("Alternative intro"),
			_s("Use an alternative game intro (CD version only)"),
			"alt_intro",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_ALT_FONT,
		{
			_s("Improved font"),
			_s("Use an easier to read custom font"),
			"alt_font",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class QueenMetaEngine : public AdvancedMetaEngine<Queen::QueenGameDescription> {
public:
	const char *getName() const override {
		return "queen";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const Queen::QueenGameDescription *desc) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override { return 99; }
	bool removeSaveState(const char *target, int slot) const override;
	int getAutosaveSlot() const override { return 99; }
};

bool QueenMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

SaveStateList QueenMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	char saveDesc[32];
	Common::String pattern("queen.s##");

	filenames = saveFileMan->listSavefiles(pattern);

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
				saveList.push_back(SaveStateDescriptor(this, slotNum, saveDesc));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

bool QueenMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("queen.s%02d", slot);

	return g_system->getSavefileManager()->removeSavefile(filename);
}

Common::Error QueenMetaEngine::createInstance(OSystem *syst, Engine **engine, const Queen::QueenGameDescription *desc) const {
	*engine = new Queen::QueenEngine(syst); //FIXME ,desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(QUEEN)
	REGISTER_PLUGIN_DYNAMIC(QUEEN, PLUGIN_TYPE_ENGINE, QueenMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(QUEEN, PLUGIN_TYPE_ENGINE, QueenMetaEngine);
#endif

