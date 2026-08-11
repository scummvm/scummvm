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

#include "queen/input.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/detection.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

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
	Common::KeymapArray initKeymaps(const char *target) const override;
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

static const char *const CommandKeys[] = {
	"ocmglptu", // English
	"osbgpnre", // German
	"ofdnepau", // French
	"acsdgpqu", // Italian
	"ocmglptu", // Hebrew
	"acodmthu"  // Spanish
};

const char *CurrentCommandKeys;

Common::KeymapArray QueenMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Queen;

	Keymap *engineKeymap = new Keymap(Keymap::kKeymapTypeGame, "queen-default", _("Default keymappings"));
	Keymap *gameKeymap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *journalKeymap = new Keymap(Keymap::kKeymapTypeGame, "journal", _("Journal keymappings"));

	Action *act;

	Language language = parseLanguage(ConfMan.get("language"));

	switch (language) {
	case EN_ANY:
	case EL_GRC:
	case RU_RUS:
		CurrentCommandKeys = CommandKeys[0];
		break;
	case DE_DEU:
		CurrentCommandKeys = CommandKeys[1];
		break;
	case FR_FRA:
		CurrentCommandKeys = CommandKeys[2];
		break;
	case IT_ITA:
		CurrentCommandKeys = CommandKeys[3];
		break;
	case HE_ISR:
		CurrentCommandKeys = CommandKeys[4];
		break;
	case ES_ESP:
		CurrentCommandKeys = CommandKeys[5];
		break;
	default:
		error("Unknown language");
		break;
	}

	act = new Action(kStandardActionLeftClick, _("Move / Interact"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeymap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Move / Skip / Use default action with inventory item"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeymap->addAction(act);

	act = new Action("FASTMODE", _("Toggle fast mode"));
	act->setCustomEngineActionEvent(kActionFastMode);
	act->addDefaultInputMapping("C+f");
	act->addDefaultInputMapping("JOY_Y");
	gameKeymap->addAction(act);

	act = new Action("SKIPTEXT", _("Skip text"));
	act->setCustomEngineActionEvent(kActionSkipText);
	act->addDefaultInputMapping("SPACE");
	gameKeymap->addAction(act);

	act = new Action("INVSCROLLUP", _("Scroll inventory up"));
	act->setCustomEngineActionEvent(kActionScrollUp);
	act->addDefaultInputMapping("COMMA");
	act->addDefaultInputMapping("JOY_UP");
	gameKeymap->addAction(act);

	act = new Action("INVSCROLLDOWN", _("Scroll inventory down"));
	act->setCustomEngineActionEvent(kActionScrollDown);
	act->addDefaultInputMapping("PERIOD");
	act->addDefaultInputMapping("JOY_DOWN");
	gameKeymap->addAction(act);

	act = new Action("INVITEM1", _("Inventory item 1"));
	act->setCustomEngineActionEvent(kActionInvSlot1);
	act->addDefaultInputMapping("1");
	gameKeymap->addAction(act);

	act = new Action("INVITEM2", _("Inventory item 2"));
	act->setCustomEngineActionEvent(kActionInvSlot2);
	act->addDefaultInputMapping("2");
	gameKeymap->addAction(act);

	act = new Action("INVITEM3", _("Inventory item 3"));
	act->setCustomEngineActionEvent(kActionInvSlot3);
	act->addDefaultInputMapping("3");
	gameKeymap->addAction(act);

	act = new Action("INVITEM4", _("Inventory item 4"));
	act->setCustomEngineActionEvent(kActionInvSlot4);
	act->addDefaultInputMapping("4");
	gameKeymap->addAction(act);

	act = new Action("SKIPCUTAWAY", _("Skip cutaway / dialog"));
	act->setCustomEngineActionEvent(kActionSkipCutaway);
	act->addDefaultInputMapping("ESCAPE");
	gameKeymap->addAction(act);

	act = new Action("USEJOURNAL", _("Use journal"));
	act->setCustomEngineActionEvent(kActionJournal);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_X");
	gameKeymap->addAction(act);

	act = new Action("QUICKSAVE", _("Quick save"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F11");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	gameKeymap->addAction(act);

	act = new Action("QUICKLOAD", _("Quick load"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("F12");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	gameKeymap->addAction(act);

	act = new Action("OPEN", _("Open"));
	act->setCustomEngineActionEvent(kActionOpen);
	act->addDefaultInputMapping(String(CurrentCommandKeys[0]));
	gameKeymap->addAction(act);

	act = new Action("CLOSE", _("Close"));
	act->setCustomEngineActionEvent(kActionClose);
	act->addDefaultInputMapping(String(CurrentCommandKeys[1]));
	gameKeymap->addAction(act);

	act = new Action("MOVE", _("Move"));
	act->setCustomEngineActionEvent(kActionMove);
	act->addDefaultInputMapping(String(CurrentCommandKeys[2]));
	gameKeymap->addAction(act);

	act = new Action("GIVE", _("Give"));
	act->setCustomEngineActionEvent(kActionGive);
	act->addDefaultInputMapping(String(CurrentCommandKeys[3]));
	gameKeymap->addAction(act);

	act = new Action("LOOK", _("Look at"));
	act->setCustomEngineActionEvent(kActionLook);
	act->addDefaultInputMapping(String(CurrentCommandKeys[4]));
	gameKeymap->addAction(act);

	act = new Action("PICKUP", _("Pick up"));
	act->setCustomEngineActionEvent(kActionPickUp);
	act->addDefaultInputMapping(String(CurrentCommandKeys[5]));
	gameKeymap->addAction(act);

	act = new Action("TALK", _("Talk to"));
	act->setCustomEngineActionEvent(kActionTalk);
	act->addDefaultInputMapping(String(CurrentCommandKeys[6]));
	gameKeymap->addAction(act);

	act = new Action("USE", _("Use"));
	act->setCustomEngineActionEvent(kActionUse);
	act->addDefaultInputMapping(String(CurrentCommandKeys[7]));
	gameKeymap->addAction(act);

	act = new Action("CLOSE", _("Close journal"));
	act->setCustomEngineActionEvent(kActionCloseJournal);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_X");
	journalKeymap->addAction(act);

	KeymapArray keymaps(3);

	keymaps[0] = engineKeymap;
	keymaps[1] = gameKeymap;
	keymaps[2] = journalKeymap;

	journalKeymap->setEnabled(false);

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(QUEEN)
	REGISTER_PLUGIN_DYNAMIC(QUEEN, PLUGIN_TYPE_ENGINE, QueenMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(QUEEN, PLUGIN_TYPE_ENGINE, QueenMetaEngine);
#endif

