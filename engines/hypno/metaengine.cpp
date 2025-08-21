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

#include "common/translation.h"

#include "hypno/hypno.h"
#include "hypno/detection.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_CHEATS,
		{
			_s("Enable original cheats"),
			_s("Allow cheats using the C key."),
			"cheats",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_INFINITE_HEALTH,
		{
			_s("Enable infinite health cheat"),
			_s("Player health will never decrease (except for game over scenes)."),
			"infiniteHealth",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_INFINITE_AMMO,
		{
			_s("Enable infinite ammo cheat"),
			_s("Player ammo will never decrease."),
			"infiniteAmmo",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_UNLOCK_ALL_LEVELS,
		{
			_s("Unlock all levels"),
			_s("All levels will be available to play."),
			"unlockAllLevels",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_RESTORED_CONTENT,
		{
			_s("Enable restored content"),
			_s("Add additional content that is not enabled the original implementation."),
			"restored",
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class HypnoMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "hypno";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error HypnoMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (Common::String(desc->gameId) == "wetlands") {
		*engine = (Engine *)new Hypno::WetEngine(syst, desc);
	} else if (Common::String(desc->gameId) == "sinistersix") {
		*engine = (Engine *)new Hypno::SpiderEngine(syst, desc);
	} else if (Common::String(desc->gameId) == "soldierboyz") {
		*engine = (Engine *)new Hypno::BoyzEngine(syst, desc);
	} else
		return Common::kUnsupportedGameidError;

	return Common::kNoError;
}


Common::KeymapArray HypnoMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Hypno;

	Common::String gameId = ConfMan.get("gameid", target);

	KeymapArray keymaps;

	Keymap *engineKeymap = new Keymap(Keymap::kKeymapTypeGame, "hypno-default", _("Default keymappings"));
	Keymap *introKeymap = new Keymap(Keymap::kKeymapTypeGame, "intro", _("Intro keymappings"));
	Keymap *cutsceneKeymap = new Keymap(Keymap::kKeymapTypeGame, "cutscene", _("Cutscene keymappings"));

	Common::Action *act;

	act = new Common::Action(kStandardActionLeftClick, _("Primary shoot"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeymap->addAction(act);

	act = new Common::Action(kStandardActionRightClick, _("Secondary shoot"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeymap->addAction(act);

	act = new Common::Action("SKIPINTRO", _("Skip intro"));
	act->setCustomEngineActionEvent(kActionSkipIntro);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	introKeymap->addAction(act);

	act = new Common::Action("SKIPCUTSCENE", _("Skip cutscene"));
	act->setCustomEngineActionEvent(kActionSkipCutscene);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	cutsceneKeymap->addAction(act);

	keymaps.push_back(engineKeymap);
	keymaps.push_back(introKeymap);
	keymaps.push_back(cutsceneKeymap);

	introKeymap->setEnabled(false);
	cutsceneKeymap->setEnabled(false);

	if (gameId == "soldierboyz") {
		Keymap *gameKeymap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
		Keymap *exitMenuKeymap = new Keymap(Keymap::kKeymapTypeGame, "exit-menu", _("Exit menu keymappings"));
		Keymap *difficulyMenuKeymap = new Keymap(Keymap::kKeymapTypeGame, "difficulty-menu", _("Difficulty selection menu keymappings"));
		Keymap *retryMenuKeymap = new Keymap(Keymap::kKeymapTypeGame, "retry-menu", _("Retry menu keymappings"));

		if (ConfMan.getBool("cheats",target)) {
			act = new Common::Action("SKIPLEVEL", _("Skip level (cheat)"));
			act->setCustomEngineActionEvent(kActionSkipLevel);
			act->addDefaultInputMapping("c");
			act->addDefaultInputMapping("JOY_X");
			gameKeymap->addAction(act);
		}

		// I18N: (Game name: Soldier Boyz) player refers to the users own character.
		act = new Common::Action("KILLPLAYER", _("Kill player"));
		act->setCustomEngineActionEvent(kActionKillPlayer);
		act->addDefaultInputMapping("k");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeymap->addAction(act);

		act = new Common::Action("PAUSE", _("Pause"));
		act->setCustomEngineActionEvent(kActionPause);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_UP");
		gameKeymap->addAction(act);

		act = new Common::Action("YES", _("Yes"));
		act->setCustomEngineActionEvent(kActionYes);
		act->addDefaultInputMapping("y");
		act->addDefaultInputMapping("JOY_A");
		exitMenuKeymap->addAction(act);

		act = new Common::Action("NO", _("No"));
		act->setCustomEngineActionEvent(kActionNo);
		act->addDefaultInputMapping("n");
		act->addDefaultInputMapping("JOY_B");
		exitMenuKeymap->addAction(act);

		// I18N: (Game name: Soldier Boyz) the game has 3 difficulty levels: Chump, Punk and Badass. Chump is the easy mode.
		act = new Common::Action("CHUMP", _("Chump"));
		act->setCustomEngineActionEvent(kActionDifficultyChump);
		act->addDefaultInputMapping("c");
		act->addDefaultInputMapping("JOY_LEFT");
		difficulyMenuKeymap->addAction(act);

		// I18N: (Game name: Soldier Boyz) the game has 3 difficulty levels: Chump, Punk and Badass. Punk is the medium mode.
		act = new Common::Action("PUNK", _("Punk"));
		act->setCustomEngineActionEvent(kActionDifficultyPunk);
		act->addDefaultInputMapping("p");
		act->addDefaultInputMapping("JOY_UP");
		difficulyMenuKeymap->addAction(act);

		// I18N: (Game name: Soldier Boyz) the game has 3 difficulty levels: Chump, Punk and Badass. Badass is the hard mode.
		act = new Common::Action("BADASS", _("Badass"));
		act->setCustomEngineActionEvent(kActionDifficultyBadass);
		act->addDefaultInputMapping("b");
		act->addDefaultInputMapping("JOY_RIGHT");
		difficulyMenuKeymap->addAction(act);

		act = new Common::Action("CANCEL", _("Cancel"));
		act->setCustomEngineActionEvent(kActionDifficultExit);
		act->addDefaultInputMapping("a");
		act->addDefaultInputMapping("JOY_DOWN");
		difficulyMenuKeymap->addAction(act);

		// I18N: (Game name: Soldier Boyz) This makes the player restart from the last checkpoint.
		act = new Common::Action("RETRY", _("Retry sector"));
		act->setCustomEngineActionEvent(kActionRetry);
		act->addDefaultInputMapping("s");
		act->addDefaultInputMapping("JOY_LEFT");
		retryMenuKeymap->addAction(act);

		// I18N: (Game name: Soldier Boyz) This makes the player restart the current mission / level.
		act = new Common::Action("RESTART", _("Restart territory"));
		act->setCustomEngineActionEvent(kActionRestart);
		act->addDefaultInputMapping("t");
		act->addDefaultInputMapping("JOY_UP");
		retryMenuKeymap->addAction(act);

		act = new Common::Action("EXIT", _("Begin new mission"));
		act->setCustomEngineActionEvent(kActionNewMission);
		act->addDefaultInputMapping("n");
		act->addDefaultInputMapping("JOY_RIGHT");
		retryMenuKeymap->addAction(act);

		act = new Common::Action("QUIT", _("Quit"));
		act->setCustomEngineActionEvent(kActionQuit);
		act->addDefaultInputMapping("q");
		act->addDefaultInputMapping("JOY_DOWN");
		retryMenuKeymap->addAction(act);

		keymaps.push_back(gameKeymap);
		keymaps.push_back(exitMenuKeymap);
		keymaps.push_back(difficulyMenuKeymap);
		keymaps.push_back(retryMenuKeymap);

		exitMenuKeymap->setEnabled(false);
		difficulyMenuKeymap->setEnabled(false);
		retryMenuKeymap->setEnabled(false);

	} else if (gameId == "sinistersix") {
		Keymap *gameKeymap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));

		if (ConfMan.getBool("cheats", target)) {
			act = new Common::Action("SKIPLEVEL", _("Skip level (cheat)"));
			act->setCustomEngineActionEvent(kActionSkipLevel);
			act->addDefaultInputMapping("c");
			act->addDefaultInputMapping("JOY_X");
			gameKeymap->addAction(act);
		}

		// I18N: (Game name: Marvel Comics Spider-Man: The Sinister Six) player refers to the users own character.
		act = new Common::Action("KILLPLAYER", _("Kill player"));
		act->setCustomEngineActionEvent(kActionKillPlayer);
		act->addDefaultInputMapping("k");
		act->addDefaultInputMapping("JOY_Y");
		gameKeymap->addAction(act);

		act = new Common::Action("LEFT", _("Move left"));
		act->setCustomEngineActionEvent(kActionLeft);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeymap->addAction(act);

		act = new Common::Action("DOWN", _("Move down"));
		act->setCustomEngineActionEvent(kActionDown);
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("JOY_DOWN");
		gameKeymap->addAction(act);

		act = new Common::Action("RIGHT", _("Move right"));
		act->setCustomEngineActionEvent(kActionRight);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeymap->addAction(act);

		act = new Common::Action("UP", _("Move up"));
		act->setCustomEngineActionEvent(kActionUp);
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("JOY_UP");
		gameKeymap->addAction(act);

		keymaps.push_back(gameKeymap);

	} else if (gameId == "wetlands") {
		Keymap *gameKeymap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
		Keymap *menuKeymap = new Keymap(Keymap::kKeymapTypeGame, "menu", _("Menu keymappings"));
		Keymap *pauseKeymap = new Keymap(Keymap::kKeymapTypeGame, "pause", _("Pause keymappings"));
		Keymap *directionKeymap = new Keymap(Keymap::kKeymapTypeGame, "direction", _("Direction keymappings"));

		act = new Common::Action("CREDITS", _("Show credits"));
		act->setCustomEngineActionEvent(kActionCredits);
		act->addDefaultInputMapping("c");
		act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
		gameKeymap->addAction(act);

		act = new Common::Action("SKIPLEVEL", _("Skip level (cheat)"));
		act->setCustomEngineActionEvent(kActionSkipLevel);
		act->addDefaultInputMapping("s");
		act->addDefaultInputMapping("JOY_X");
		gameKeymap->addAction(act);

		// I18N: (Game name: Wetlands) player refers to the users own character.
		act = new Common::Action("KILLPLAYER", _("Kill player"));
		act->setCustomEngineActionEvent(kActionKillPlayer);
		act->addDefaultInputMapping("k");
		act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
		gameKeymap->addAction(act);

		act = new Common::Action("LEFT", _("Move left"));
		act->setCustomEngineActionEvent(kActionLeft);
		act->allowKbdRepeats();
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("JOY_LEFT");
		directionKeymap->addAction(act);

		act = new Common::Action("DOWN", _("Move down"));
		act->setCustomEngineActionEvent(kActionDown);
		act->allowKbdRepeats();
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("JOY_DOWN");
		directionKeymap->addAction(act);

		act = new Common::Action("RIGHT", _("Move right"));
		act->setCustomEngineActionEvent(kActionRight);
		act->allowKbdRepeats();
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("JOY_RIGHT");
		directionKeymap->addAction(act);

		act = new Common::Action("UP", _("Move up"));
		act->setCustomEngineActionEvent(kActionUp);
		act->allowKbdRepeats();
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("JOY_UP");
		directionKeymap->addAction(act);

		act = new Common::Action("SELECT", _("Select"));
		act->setCustomEngineActionEvent(kActionSelect);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("JOY_X");
		menuKeymap->addAction(act);

		act = new Common::Action("PAUSE", _("Pause"));
		act->setCustomEngineActionEvent(kActionPause);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		pauseKeymap->addAction(act);

		keymaps.push_back(gameKeymap);
		keymaps.push_back(menuKeymap);
		keymaps.push_back(pauseKeymap);
		keymaps.push_back(directionKeymap);

		menuKeymap->setEnabled(false);
	}

	return keymaps;
}

namespace Hypno {

bool HypnoEngine::isDemo() const {
	return (bool)(_gameDescription->flags & ADGF_DEMO);
}

} // End of namespace Hypno

#if PLUGIN_ENABLED_DYNAMIC(HYPNO)
REGISTER_PLUGIN_DYNAMIC(HYPNO, PLUGIN_TYPE_ENGINE, HypnoMetaEngine);
#else
REGISTER_PLUGIN_STATIC(HYPNO, PLUGIN_TYPE_ENGINE, HypnoMetaEngine);
#endif

