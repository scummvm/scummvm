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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/detection.h"
#include "queen/input.h"


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

	void languageInput(Common::Keymap *perLanguage,const char *_commandKeys);
	Common::KeymapArray initKeymaps(const char *target) const override;

};

const char *const _commandKeys[] = {
	"ocmglptu", // English
	"osbgpnre", // German
	"ofdnepau", // French
	"acsdgpqu", // Italian
	"ocmglptu", // Hebrew
	"acodmthu"  // Spanish
};

void QueenMetaEngine::languageInput(Common::Keymap *perLanguage, const char* _commandKeys) {

	using namespace Common;
	using namespace Queen;
	Action *act;

	act = new Action("OPEN", _("OPEN"));
	act->setCustomEngineActionEvent(VERB_OPEN);
	act->addDefaultInputMapping(String(_commandKeys[0]));
    perLanguage->addAction(act);

	act = new Action("CLOSE", _("CLOSE"));
	act->setCustomEngineActionEvent(VERB_CLOSE);
	act->addDefaultInputMapping(String(_commandKeys[1]));
	perLanguage->addAction(act);

	act = new Action("MOVE", _("MOVE"));
	act->setCustomEngineActionEvent(VERB_MOVE);
	act->addDefaultInputMapping(String(_commandKeys[2]));
	perLanguage->addAction(act);

	act = new Action("GIVE", _("GIVE"));
	act->setCustomEngineActionEvent(VERB_GIVE);
	act->addDefaultInputMapping(String(_commandKeys[3]));
	perLanguage->addAction(act);

	act = new Action("LOOKAT", _("LOOK AT"));
	act->setCustomEngineActionEvent(VERB_LOOK_AT);
	act->addDefaultInputMapping(String(_commandKeys[4]));
	perLanguage->addAction(act);

	act = new Action("PICKUP", _("PICK UP"));
	act->setCustomEngineActionEvent(VERB_PICK_UP);
	act->addDefaultInputMapping(String(_commandKeys[5]));
	perLanguage->addAction(act);

	act = new Action("TALKTO", _("TALK TO"));
	act->setCustomEngineActionEvent(VERB_TALK_TO);
	act->addDefaultInputMapping(String(_commandKeys[6]));
	perLanguage->addAction(act);

	act = new Action("USE", _("USE"));
	act->setCustomEngineActionEvent(VERB_USE);
	act->addDefaultInputMapping(String(_commandKeys[7]));
	perLanguage->addAction(act);
}

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

Common::Language language;

Common::KeymapArray QueenMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Queen;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "queen-default", _("Default keymappings"));
	Keymap *engineKeyEnglish = new Keymap(Keymap::kKeymapTypeGame, "English", _("English"));
	Keymap *engineKeyGerman = new Keymap(Keymap::kKeymapTypeGame, "German", _("German"));
	Keymap *engineKeyFrench = new Keymap(Keymap::kKeymapTypeGame, "French", _("French"));
	Keymap *engineKeyItalian = new Keymap(Keymap::kKeymapTypeGame, "Italian", _("Italian"));
	Keymap *engineKeyHebrew = new Keymap(Keymap::kKeymapTypeGame, "Hebrew", _("Hebrew"));
	Keymap *engineKeySpanish = new Keymap(Keymap::kKeymapTypeGame, "Spanish", _("Spanish"));

	KeymapArray keymaps(7);
	keymaps[0] = engineKeyMap;
	keymaps[1] = engineKeyEnglish;
	keymaps[2] = engineKeyGerman;
	keymaps[3] = engineKeyFrench;
	keymaps[4] = engineKeyItalian;
	keymaps[5] = engineKeyHebrew;
	keymaps[6] = engineKeySpanish;

	Action *act;

	act = new Action("USEJOURNAL", _("USE JOURNAL"));
	act->setCustomEngineActionEvent(VERB_USE_JOURNAL);
	act->addDefaultInputMapping("F1");
	engineKeyMap->addAction(act);

	act = new Action("SKIPTEXT", _("SKIP TEXT"));
	act->setCustomEngineActionEvent(VERB_SKIP_TEXT);
	act->addDefaultInputMapping("SPACE");
	engineKeyMap->addAction(act);

	act = new Action("USEITEM1", _("USE ITEM 1"));
	act->setCustomEngineActionEvent(VERB_DIGIT_1);
	act->addDefaultInputMapping("1");
	engineKeyMap->addAction(act);

	act = new Action("USEITEM2", _("USE ITEM 2"));
	act->setCustomEngineActionEvent(VERB_DIGIT_2);
	act->addDefaultInputMapping("2");
	engineKeyMap->addAction(act);

	act = new Action("USEITEM3", _("USE ITEM 3"));
	act->setCustomEngineActionEvent(VERB_DIGIT_3);
	act->addDefaultInputMapping("3");
	engineKeyMap->addAction(act);

	act = new Action("USEITEM4", _("USE ITEM 4"));
	act->setCustomEngineActionEvent(VERB_DIGIT_4);
	act->addDefaultInputMapping("4");
	engineKeyMap->addAction(act);

	const_cast<QueenMetaEngine *>(this)->languageInput(engineKeyEnglish, _commandKeys[0]);
	const_cast<QueenMetaEngine *>(this)->languageInput(engineKeyGerman, _commandKeys[1]);
	const_cast<QueenMetaEngine *>(this)->languageInput(engineKeyFrench, _commandKeys[2]);
	const_cast<QueenMetaEngine *>(this)->languageInput(engineKeyItalian, _commandKeys[3]);
	const_cast<QueenMetaEngine *>(this)->languageInput(engineKeyHebrew, _commandKeys[4]);
	const_cast<QueenMetaEngine *>(this)->languageInput(engineKeySpanish, _commandKeys[5]);

	engineKeySpanish->setEnabled(false);
	engineKeyEnglish->setEnabled(false);
	engineKeyFrench->setEnabled(false);
	engineKeyGerman->setEnabled(false);
	engineKeyItalian->setEnabled(false);
	engineKeyHebrew->setEnabled(false);


	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(QUEEN)
	REGISTER_PLUGIN_DYNAMIC(QUEEN, PLUGIN_TYPE_ENGINE, QueenMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(QUEEN, PLUGIN_TYPE_ENGINE, QueenMetaEngine);
#endif

