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


#include "bladerunner/bladerunner.h"
#include "bladerunner/savefile.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

class BladeRunnerMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	// Disable autosave (see mirrored method in bladerunner.h for detailed explanation)
	int getAutosaveSlot() const override { return -1; }
};

const char *BladeRunnerMetaEngine::getName() const {
	return "bladerunner";
}

Common::Error BladeRunnerMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new BladeRunner::BladeRunnerEngine(syst, desc);
	return Common::kNoError;
}

bool BladeRunnerMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		f == kSupportsListSaves ||
		f == kSupportsLoadingDuringStartup ||
		f == kSupportsDeleteSave ||
		f == kSavesSupportMetaInfo ||
		f == kSavesSupportThumbnail ||
		f == kSavesSupportCreationDate ||
		f == kSavesSupportPlayTime ||
		f == kSimpleSavesNames;
}

Common::KeymapArray BladeRunnerMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace BladeRunner;

	Common::String gameId = ConfMan.get("gameid", target);
	Common::U32String gameDesc;
	Keymap *commonKeymap;
	Keymap *gameplayKeymap;
	Keymap *kiaOnlyKeymap;

	if (gameId == "bladerunner") {
		gameDesc = "Blade Runner";
	} else if (gameId == "bladerunner-final") {
		gameDesc = "Blade Runner (Restored Content)";
	}

	if (gameDesc.empty()) {
		return AdvancedMetaEngine::initKeymaps(target);
	}

	// We use 3 keymaps: common (main game and KIA), gameplay (main game only) and kia (KIA only).
	// This helps us with disabling unneeded keymaps, which is especially useful in KIA, when typing in a saved game.
	// In general, Blade Runner by default, can bind a key (eg. spacebar) to multiple actions
	// (eg. skip cutscene, toggle combat, enter a blank space in save game input field).
	// We need to be able to disable the conflicting keymaps, while keeping others that should still work in KIA
	// (eg. "Esc" (by default) toggling KIA should work in normal gameplay and also within KIA).
	// Another related issue we tackle is that a custom action event does not maintain the keycode and ascii value
	// (if it was associated with a keyboard key), and there's no obvious way to retrieve those from it.
	// Thus, a custom action event cannot be somehow utilised to produce keyboard key presses
	// (again if a keyboard key is mapped to that action), so it cannot by itself be used
	// for text entering in the save file name input field, or for typing the Easter Egg strings.
	commonKeymap = new Keymap(Keymap::kKeymapTypeGame, BladeRunnerEngine::kCommonKeymapId, gameDesc + Common::U32String(" - ") + _("common shortcuts"));
	gameplayKeymap = new Keymap(Keymap::kKeymapTypeGame, BladeRunnerEngine::kGameplayKeymapId, gameDesc + Common::U32String(" - ") + _("main game shortcuts"));
	kiaOnlyKeymap = new Keymap(Keymap::kKeymapTypeGame, BladeRunnerEngine::kKiaKeymapId, gameDesc + Common::U32String(" - ") + _("KIA only shortcuts"));

	Action *act;

	// Look at backends\keymapper\hardware-input.cpp for the strings that can be used in InputMapping
	act = new Action(kStandardActionLeftClick, _("Walk / Look / Talk / Select / Shoot"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	commonKeymap->addAction(act);

	act = new Action("COMBAT", _("Toggle Combat"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionToggleCombat);
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("MOUSE_MIDDLE");
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping("SPACE");
	gameplayKeymap->addAction(act);

	act = new Action("SKIPVIDEO", _("Skip cutscene"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpblActionCutsceneSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("KP_ENTER");
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_Y");
	gameplayKeymap->addAction(act);

	act = new Action("SKIPDLG", _("Skip dialogue"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionDialogueSkip);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("KP_ENTER");
	act->addDefaultInputMapping("JOY_X");
	gameplayKeymap->addAction(act);

	act = new Action("KIAOPTS", _("Game Options"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionToggleKiaOptions);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	commonKeymap->addAction(act);

	act = new Action("KIADB", _("Open KIA Database"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKiaDatabase);
	act->addDefaultInputMapping("TAB");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameplayKeymap->addAction(act);

	act = new Action("KIASCROLLUP", _("Scroll Up"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionScrollUp);
	act->addDefaultInputMapping("MOUSE_WHEEL_UP");
	act->addDefaultInputMapping("JOY_UP");
	kiaOnlyKeymap->addAction(act);

	act = new Action("KIASCROLLDOWN", _("Scroll Down"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionScrollDown);
	act->addDefaultInputMapping("MOUSE_WHEEL_DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	kiaOnlyKeymap->addAction(act);

	act = new Action("KIAHLP", _("Help"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabHelp);
	act->addDefaultInputMapping("F1");
	commonKeymap->addAction(act);

	act = new Action("KIASAVE", _("Save Game"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabSaveGame);
	act->addDefaultInputMapping("F2");
	commonKeymap->addAction(act);

	act = new Action("KIALOAD", _("Load Game"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabLoadGame);
	act->addDefaultInputMapping("F3");
	commonKeymap->addAction(act);

	act = new Action("KIACRIMES", _("Crime Scene Database"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabCrimeSceneDatabase);
	act->addDefaultInputMapping("F4");
	commonKeymap->addAction(act);

	act = new Action("KIASUSPECTS", _("Suspect Database"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabSuspectDatabase);
	act->addDefaultInputMapping("F5");
	commonKeymap->addAction(act);

	act = new Action("KIACLUES", _("Clue Database"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabClueDatabase);
	act->addDefaultInputMapping("F6");
	commonKeymap->addAction(act);

	act = new Action("KIAQUIT", _("Quit Game"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabQuitGame);
	act->addDefaultInputMapping("F10");
	commonKeymap->addAction(act);

	KeymapArray keymaps(3);
	keymaps[0] = commonKeymap;
	keymaps[1] = gameplayKeymap;
	keymaps[2] = kiaOnlyKeymap;

	return keymaps;
}

SaveStateList BladeRunnerMetaEngine::listSaves(const char *target) const {
	return BladeRunner::SaveFileManager::list(this, target);
}

int BladeRunnerMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void BladeRunnerMetaEngine::removeSaveState(const char *target, int slot) const {
	BladeRunner::SaveFileManager::remove(target, slot);
}

SaveStateDescriptor BladeRunnerMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	return BladeRunner::SaveFileManager::queryMetaInfos(this, target, slot);
}

#if PLUGIN_ENABLED_DYNAMIC(BLADERUNNER)
	REGISTER_PLUGIN_DYNAMIC(BLADERUNNER, PLUGIN_TYPE_ENGINE, BladeRunnerMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(BLADERUNNER, PLUGIN_TYPE_ENGINE, BladeRunnerMetaEngine);
#endif
