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
#include "engines/stark/savemetadata.h"
#include "engines/stark/stark.h"
#include "engines/stark/services/stateprovider.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

namespace Stark {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ASSETS_MOD,
		{
			_s("Load modded assets"),
			_s("Enable loading of external replacement assets."),
			"enable_assets_mod",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_LINEAR_FILTERING,
		{
			_s("Enable linear filtering of the backgrounds images"),
			_s("When linear filtering is enabled the background graphics are smoother in full screen mode, at the cost of some details."),
			"use_linear_filtering",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_FONT_ANTIALIASING,
		{
			_s("Enable font anti-aliasing"),
			_s("When font anti-aliasing is enabled, the text is smoother."),
			"enable_font_antialiasing",
			true,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class StarkMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "stark";
	}

	Common::KeymapArray initKeymaps(const char *target) const override;

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return
			(f == kSupportsListSaves) ||
			(f == kSupportsLoadingDuringStartup) ||
			(f == kSupportsDeleteSave) ||
			(f == kSavesSupportThumbnail) ||
			(f == kSavesSupportMetaInfo) ||
			(f == kSavesSupportPlayTime) ||
			(f == kSavesSupportCreationDate);
	}

	int getMaximumSaveSlot() const override {
		return 999;
	}

	SaveStateList listSaves(const char *target) const override {
		Common::StringArray filenames = StarkEngine::listSaveNames(target);

		SaveStateList saveList;
		for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
			int slot = StarkEngine::getSaveNameSlot(target, *filename);

			// Read the description from the save
			Common::String description;
			Common::InSaveFile *save = g_system->getSavefileManager()->openForLoading(*filename);
			if (save) {
				StateReadStream stream(save);
				description = stream.readString();
			}

			saveList.push_back(SaveStateDescriptor(this, slot, description));
		}

		Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
		return saveList;
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Common::String filename = StarkEngine::formatSaveName(target, slot);
		Common::InSaveFile *save = g_system->getSavefileManager()->openForLoading(filename);
		if (!save) {
			return SaveStateDescriptor();
		}

		SaveStateDescriptor descriptor;
		descriptor.setSaveSlot(slot);

		SaveMetadata metadata;
		Common::ErrorCode readError = metadata.read(save, filename);
		if (readError != Common::kNoError) {
			delete save;
			return descriptor;
		}

		descriptor.setDescription(metadata.description);

		if (metadata.version >= 9) {
			Graphics::Surface *thumb = metadata.readGameScreenThumbnail(save);
			descriptor.setThumbnail(thumb);
			descriptor.setPlayTime(metadata.totalPlayTime);
			descriptor.setSaveDate(metadata.saveYear, metadata.saveMonth, metadata.saveDay);
			descriptor.setSaveTime(metadata.saveHour, metadata.saveMinute);
		}

		if (metadata.version >= 13) {
			descriptor.setAutosave(metadata.isAutoSave);
		}

		delete save;

		return descriptor;
	}

	bool removeSaveState(const char *target, int slot) const override {
		Common::String filename = StarkEngine::formatSaveName(target, slot);
		return g_system->getSavefileManager()->removeSavefile(filename);
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new StarkEngine(syst, desc);
		return Common::kNoError;
	}

	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (!target)
			target = getName();
		if (saveGameIdx == kSavegameFilePattern)
			return Common::String::format("%s-###.tlj", target);
		else
			return StarkEngine::formatSaveName(target, saveGameIdx);
	}
};

Common::KeymapArray StarkMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Stark;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "stark-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	// I18N: Opens in-game Diary
	act = new Action("DIARYMENU", _("Diary menu"));
	act->setCustomEngineActionEvent(kActionDiaryMenu);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	act = new Action("SAVEGAME", _("Save game"));
	act->setCustomEngineActionEvent(kActionSaveGame);
	act->addDefaultInputMapping("F2");
	gameKeyMap->addAction(act);

	act = new Action("LOADGAME", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoadGame);
	act->addDefaultInputMapping("F3");
	gameKeyMap->addAction(act);

	// I18N: Opens in-game conversation log
	act = new Action("CONVOLOG", _("Conversation log"));
	act->setCustomEngineActionEvent(kActionConversationLog);
	act->addDefaultInputMapping("F4");
	gameKeyMap->addAction(act);

	// I18N: Opens in-game Diary. April is the female protagonist name
	act = new Action("APRILSDIARY", _("April's diary (initially disabled)"));
	act->setCustomEngineActionEvent(kActionAprilsDiary);
	act->addDefaultInputMapping("F5");
	gameKeyMap->addAction(act);

	act = new Action("VIDREPLAY", _("Video replay"));
	act->setCustomEngineActionEvent(kActionVideoReplay);
	act->addDefaultInputMapping("F6");
	gameKeyMap->addAction(act);

	act = new Action("GAMESETTINGS", _("Game settings"));
	act->setCustomEngineActionEvent(kActionGameSettings);
	act->addDefaultInputMapping("F7");
	gameKeyMap->addAction(act);

	act = new Action("SAVESCRNSHOT", _("Save screenshot"));
	act->setCustomEngineActionEvent(kActionSaveScreenshot);
	act->addDefaultInputMapping("F8");
	gameKeyMap->addAction(act);

	act = new Action("TOGGLESUBS", _("Toggle subtitles"));
	act->setCustomEngineActionEvent(kActionToggleSubtitles);
	act->addDefaultInputMapping("F9");
	gameKeyMap->addAction(act);

	act = new Action("QUITTOMENU", _("Quit to menu"));
	act->setCustomEngineActionEvent(kActionQuitToMenu);
	act->addDefaultInputMapping("F10");
	gameKeyMap->addAction(act);

	act = new Action("CYCLEBACK", _("Cycle back through inventory cursor items"));
	act->setCustomEngineActionEvent(kActionCycleForwardInventory);
	act->addDefaultInputMapping("a");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	gameKeyMap->addAction(act);

	act = new Action("CYCLEFORWARD", _("Cycle forward through inventory cursor items"));
	act->setCustomEngineActionEvent(kActionCycleBackInventory);
	act->addDefaultInputMapping("s");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	gameKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(kActionInventory);
	act->addDefaultInputMapping("i");
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);

	// I18N: A popup on screen shows shows the exits
	act = new Action("DISPLAYEXITS", _("Display all exits on current location"));
	act->setCustomEngineActionEvent(kActionDisplayExits);
	act->addDefaultInputMapping("x");
	act->addDefaultInputMapping("JOY_RIGHT_STICK");
	gameKeyMap->addAction(act);

	act = new Action("EXITGAME", _("Exit game"));
	act->setCustomEngineActionEvent(kActionExitGame);
	act->addDefaultInputMapping("A+x");
	act->addDefaultInputMapping("A+q");
	gameKeyMap->addAction(act);

	act = new Action("PAUSE", _("Pause game"));
	act->setCustomEngineActionEvent(kActionPause);
	act->addDefaultInputMapping("p");
	act->addDefaultInputMapping("JOY_LEFT_STICK");
	gameKeyMap->addAction(act);

	act = new Action("SCROLLUPINV", _("Scroll up in inventory"));
	act->setCustomEngineActionEvent(kActionInventoryScrollUp);
	act->addDefaultInputMapping("PAGEUP");
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("SCROLLDOWNINV", _("Scroll down in inventory"));
	act->setCustomEngineActionEvent(kActionInventoryScrollDown);
	act->addDefaultInputMapping("PAGEDOWN");
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("SCROLLUPDILOG", _("Scroll up in your dialogues"));
	act->setCustomEngineActionEvent(kActionDialogueScrollUp);
	act->addDefaultInputMapping("PAGEUP");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("SCROLLDOWNDILOG", _("Scroll down in your dialogues"));
	act->setCustomEngineActionEvent(kActionDialogueScrollDown);
	act->addDefaultInputMapping("PAGEDOWN");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("SCROLLUPINVPREVDILOG", _("Go to next dialogue"));
	act->setCustomEngineActionEvent(kActionNextDialogue);
	act->addDefaultInputMapping("DOWN");
	gameKeyMap->addAction(act);

	act = new Action("SCROLLDOWNINVNEXTDILOG", _("Go to previous dialogues"));
	act->setCustomEngineActionEvent(kActionPrevDialogue);
	act->addDefaultInputMapping("UP");
	gameKeyMap->addAction(act);

	act = new Action("SELECTDILOG", _("Select dialogue"));
	act->setCustomEngineActionEvent(kActionSelectDialogue);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("KP_ENTER");
	act->addDefaultInputMapping("JOY_RIGHT");
	gameKeyMap->addAction(act);

	act = new Action("SKIP", _("Skip video sequence or dialogue"));
	act->setCustomEngineActionEvent(kActionSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_BACK");
	gameKeyMap->addAction(act);


	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

} // End of namespace Stark

#if PLUGIN_ENABLED_DYNAMIC(STARK)
	REGISTER_PLUGIN_DYNAMIC(STARK, PLUGIN_TYPE_ENGINE, Stark::StarkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(STARK, PLUGIN_TYPE_ENGINE, Stark::StarkMetaEngine);
#endif
