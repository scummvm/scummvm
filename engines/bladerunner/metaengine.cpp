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
#include "bladerunner/detection.h"
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

namespace BladeRunner {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_SITCOM,
		{
			_s("Sitcom mode"),
			_s("Game will add laughter after actor's line or narration"),
			"sitcom",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_SHORTY,
		{
			_s("Shorty mode"),
			_s("Game will shrink the actors and make their voices high pitched"),
			"shorty",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FRAMELIMITER_NODELAYMILLIS,
		{
			_s("Frame limiter high performance mode"),
			_s("This mode may result in high CPU usage! It avoids use of delayMillis() function."),
			"nodelaymillisfl",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FRAMELIMITER_FPS,
		{
			_s("Max frames per second limit"),
			_s("This mode targets a maximum of 120 fps. When disabled, the game targets 60 fps"),
			"frames_per_secondfl",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_DISABLE_STAMINA_DRAIN,
		{
			_s("Disable McCoy's quick stamina drain"),
			_s("When running, McCoy won't start slowing down as soon as the player stops clicking the mouse"),
			"disable_stamina_drain",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_SHOW_SUBS_IN_CRAWL,
		{
			_s("Show subtitles during text crawl"),
			_s("During the intro cutscene, show subtitles during the text crawl"),
			"use_crawl_subs",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_FIX_SPANISH_CREDITS,
		{
			_s("Fix credits for voice actors"),
			_s("Updates the end credits with corrected credits for the Spanish voice actors"),
			"correct_spanish_credits",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace BladeRunner

class BladeRunnerMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override;

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return BladeRunner::optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
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
	} else if (gameId == "bladerunner-ee") {
		gameDesc = "Blade Runner: Enhanced Edition";
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
	// I18N: These are keymaps that work in the main gameplay and also when KIA (Knowledge Integration Assistant) is open.
	commonKeymap = new Keymap(Keymap::kKeymapTypeGame, BladeRunnerEngine::kCommonKeymapId, gameDesc + Common::U32String(" - ") + _("common shortcuts"));
	// I18N: These are keymaps which work only in the main gameplay and not within KIA's (Knowledge Integration Assistant) screens.
	gameplayKeymap = new Keymap(Keymap::kKeymapTypeGame, BladeRunnerEngine::kGameplayKeymapId, gameDesc + Common::U32String(" - ") + _("main game shortcuts"));
	// I18N: These are keymaps that work only within KIA's (Knowledge Integration Assistant) screens.
	kiaOnlyKeymap = new Keymap(Keymap::kKeymapTypeGame, BladeRunnerEngine::kKiaKeymapId, gameDesc + Common::U32String(" - ") + _("KIA only shortcuts"));

	Action *act;

	// Look at backends\keymapper\hardware-input.cpp for the strings that can be used in InputMapping
	// I18N: This keymap is the main way for the user interact with the game.
	// It is used with the game's cursor to select, walk-to, run-to, look-at, talk-to, pick up, use, shoot (combat mode), open KIA (when clicking on McCoy).
	act = new Action(kStandardActionLeftClick, _("Walk / Look / Talk / Select / Shoot"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	commonKeymap->addAction(act);

	// I18N: This keymap toggles McCoy's status between combat mode (drawing his gun) and normal mode (holstering his gun)
	// In Blade Runner's official localizations, there is a description of this keymap
	// on the KIA Help Page, under Keyboard Shortcuts. In the English version it is
	// TOGGLE MCCOY'S COMBAT MODE
	act = new Action("COMBAT", _("Toggle Combat"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionToggleCombat);
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("MOUSE_MIDDLE");
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping("SPACE");
	gameplayKeymap->addAction(act);

	// I18N: This keymap allows skipping video cutscenes
	act = new Action("SKIPVIDEO", _("Skip cutscene"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionCutsceneSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("KP_ENTER");
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_Y");
	gameplayKeymap->addAction(act);

	// I18N: This keymap allows skipping the current line of dialogue.
	// In Blade Runner's official localizations, there is a description of this keymap
	// on the KIA Help Page, under Keyboard Shortcuts. In the English version it is
	// SKIP PAST CURRENT LINE OF DIALOGUE
	act = new Action("SKIPDLG", _("Skip dialogue"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionDialogueSkip);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("KP_ENTER");
	act->addDefaultInputMapping("JOY_X");
	gameplayKeymap->addAction(act);

	// I18N: This keymap toggles between opening the KIA in the Game Options tab, and closing the KIA.
	// In Blade Runner's official localizations, there is a description of this keymap
	// on the KIA Help Page, under Keyboard Shortcuts. In the English version it is
	// GAME OPTIONS
	act = new Action("KIAOPTS", _("Game Options"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionToggleKiaOptions);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	commonKeymap->addAction(act);

	// I18N: This keymap opens the KIA database on one of the investigation tabs,
	// CRIME SCENE DATABASE, SUSPECT DATABASE and CLUES DATABASE.
	// In Blade Runner's official localizations, there is a description of this keymap
	// on the KIA Help Page, under Keyboard Shortcuts. In the English version it is
	// ACTIVATE KIA CLUE DATABASE SYSTEM
	act = new Action("KIADB", _("Open KIA Database"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKiaDatabase);
	act->addDefaultInputMapping("TAB");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameplayKeymap->addAction(act);

	// I18N: This keymap works within the KIA Save Game screen
	// and allows confirming popup dialogue prompts (eg. for save game deletion or overwriting)
	// and also submitting a new save game name, or choosing an existing save game for overwriting.
	act = new Action("KIACONFIRMDLG", _("Confirm"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpConfirmDlg);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("KP_ENTER");
	act->addDefaultInputMapping("JOY_B");
	kiaOnlyKeymap->addAction(act);

	// I18N: This keymap works within the KIA Save Game screen
	// and allows submitting a selected existing save game for deletion.
	act = new Action("KIADELETESVDGAME", _("Delete Selected Saved Game"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpDeleteSelectedSvdGame);
	act->addDefaultInputMapping("DELETE");
	// TODO In the original KP_PERIOD with NUMLOCK on, would work as a normal '.' character.
	// KP_PERIOD with NUMLOCK off, would work as a delete request for the selected saved game.
	// However, NUMLOCK is currently not working as a modifier key for keymaps,
	// so maybe we should implement this original behavior more accurately,
	// when that is fixed in the keymapper or hardware-input code.
	// For now, KP_PERIOD will work (by default) as a delete request.
	act->addDefaultInputMapping("KP_PERIOD");
	act->addDefaultInputMapping("JOY_X");
	kiaOnlyKeymap->addAction(act);

	// I18N: This keymap allows scrolling texts and lists upwards
	act = new Action("KIASCROLLUP", _("Scroll Up"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionScrollUp);
	act->addDefaultInputMapping("MOUSE_WHEEL_UP");
	act->addDefaultInputMapping("JOY_UP");
	kiaOnlyKeymap->addAction(act);

	// I18N: This keymap allows scrolling texts and lists downwards
	act = new Action("KIASCROLLDOWN", _("Scroll Down"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionScrollDown);
	act->addDefaultInputMapping("MOUSE_WHEEL_DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	kiaOnlyKeymap->addAction(act);

	// I18N: This keymap allows (in KIA only) for a clue to be set as private or public
	// (only when the KIA is upgraded).
	act = new Action("KIATOGGLECLUEPRIVACY", _("Toggle Clue Privacy"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionToggleCluePrivacy);
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	kiaOnlyKeymap->addAction(act);

	// I18N: This keymap opens KIA's HELP tab.
	// In Blade Runner's official localizations, there is a description of this keymap
	// on the KIA Help Page, under Keyboard Shortcuts. In the English version it is
	// ONLINE HELP
	act = new Action("KIAHLP", _("Help"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabHelp);
	act->addDefaultInputMapping("F1");
	commonKeymap->addAction(act);

	// I18N: This keymap opens KIA's SAVE GAME tab.
	// In Blade Runner's official localizations, there is a description of this keymap
	// on the KIA Help Page, under Keyboard Shortcuts. In the English version it is
	// SAVE GAME
	act = new Action("KIASAVE", _("Save Game"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabSaveGame);
	act->addDefaultInputMapping("F2");
	commonKeymap->addAction(act);

	// I18N: This keymap opens KIA's LOAD GAME tab.
	// In Blade Runner's official localizations, there is a description of this keymap
	// on the KIA Help Page, under Keyboard Shortcuts. In the English version it is
	// LOAD GAME
	act = new Action("KIALOAD", _("Load Game"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabLoadGame);
	act->addDefaultInputMapping("F3");
	commonKeymap->addAction(act);

	// I18N: This keymap opens KIA's CRIME SCENE DATABASE tab.
	// In Blade Runner's official localizations, there is a description of this keymap
	// on the KIA Help Page, under Keyboard Shortcuts. In the English version it is
	// CRIME SCENE DATABASE
	act = new Action("KIACRIMES", _("Crime Scene Database"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabCrimeSceneDatabase);
	act->addDefaultInputMapping("F4");
	commonKeymap->addAction(act);

	// I18N: This keymap opens KIA's SUSPECT DATABASE tab.
	// In Blade Runner's official localizations, there is a description of this keymap
	// on the KIA Help Page, under Keyboard Shortcuts. In the English version it is
	// SUSPECT DATABASE
	act = new Action("KIASUSPECTS", _("Suspect Database"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabSuspectDatabase);
	act->addDefaultInputMapping("F5");
	commonKeymap->addAction(act);

	// I18N: This keymap opens KIA's CLUE DATABASE tab.
	// In Blade Runner's official localizations, there is a description of this keymap
	// on the KIA Help Page, under Keyboard Shortcuts. In the English version it is
	// CLUE DATABASE
	act = new Action("KIACLUES", _("Clue Database"));
	act->setCustomEngineActionEvent(BladeRunnerEngine::kMpActionOpenKIATabClueDatabase);
	act->addDefaultInputMapping("F6");
	commonKeymap->addAction(act);

	// I18N: This keymap opens KIA's QUIT GAME tab.
	// In Blade Runner's official localizations, there is a description of this keymap
	// on the KIA Help Page, under Keyboard Shortcuts. In the English version it is
	// QUIT GAME
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

bool BladeRunnerMetaEngine::removeSaveState(const char *target, int slot) const {
	return BladeRunner::SaveFileManager::remove(target, slot);
}

SaveStateDescriptor BladeRunnerMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	return BladeRunner::SaveFileManager::queryMetaInfos(this, target, slot);
}

#if PLUGIN_ENABLED_DYNAMIC(BLADERUNNER)
	REGISTER_PLUGIN_DYNAMIC(BLADERUNNER, PLUGIN_TYPE_ENGINE, BladeRunnerMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(BLADERUNNER, PLUGIN_TYPE_ENGINE, BladeRunnerMetaEngine);
#endif
