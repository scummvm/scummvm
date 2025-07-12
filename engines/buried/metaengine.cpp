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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "base/plugins.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

#include "buried/buried.h"
#include "buried/detection.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace Buried {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ALLOW_SKIP,
		{
			// I18N: This option allows the user to skip cutscenes.
			_s("Skip support"),
			_s("Allow cutscenes to be skipped"),
			"skip_support",
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

bool BuriedEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher)
		|| (f == kSupportsLoadingDuringRuntime)
		|| (f == kSupportsSavingDuringRuntime);
}

bool BuriedEngine::isDemo() const {
	// The trial is a demo for the user's sake, but not internally.
	return (_gameDescription->flags & ADGF_DEMO) != 0 && !isTrial();
}

bool BuriedEngine::isTrial() const {
	return (_gameDescription->flags & GF_TRIAL) != 0;
}

bool BuriedEngine::isTrueColor() const {
	return (_gameDescription->flags & GF_TRUECOLOR) != 0;
}

bool BuriedEngine::isWin95() const {
	return (_gameDescription->flags & GF_WIN95) != 0;
}

bool BuriedEngine::isCompressed() const {
	return (_gameDescription->flags & GF_COMPRESSED) != 0;
}

Common::Path BuriedEngine::getEXEName() const {
	return _gameDescription->filesDescriptions[0].fileName;
}

Common::Path BuriedEngine::getLibraryName() const {
	return _gameDescription->filesDescriptions[1].fileName;
}

Common::Language BuriedEngine::getLanguage() const {
	return _gameDescription->language;
}

} // End of namespace Buried

class BuriedMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "buried";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Buried::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	int getMaximumSaveSlot() const override { return 999; }
	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		// We set a standard target because saves are compatible among all versions
		return AdvancedMetaEngine::getSavegameFile(saveGameIdx, "buried");
	}
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool BuriedMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsLoadingDuringStartup) ||
		checkExtendedSaves(f);
}

Common::Error BuriedMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Buried::BuriedEngine(syst, desc);

	return Common::kNoError;
}

Common::KeymapArray BuriedMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Buried;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "buried-default", _("Default keymappings"));
	Keymap *cutSceneKeyMap = new Keymap(Keymap::kKeymapTypeGame, "cutscene", _("Cutscene keymappings"));
	Keymap *mainMenuKeyMap = new Keymap(Keymap::kKeymapTypeGame, "main-menu", _("Main menu keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *inventoryKeyMap = new Keymap(Keymap::kKeymapTypeGame, "inventory", _("Inventory keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Select / Interact"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	// I18N: This action refers to the control key on the keyboard.
	act = new Action("CTRL", _("Control key"));
	act->setCustomEngineActionEvent(kActionControl);
	act->addDefaultInputMapping("LCTRL");
	act->addDefaultInputMapping("RCTRL");
	engineKeyMap->addAction(act);

	act = new Action("QUIT", _("Quit"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("ESCAPE");
	mainMenuKeyMap->addAction(act);

	if (ConfMan.getBool("skip_support", target)) {
		act = new Action("SKIP", _("Skip cutscene"));
		act->setCustomEngineActionEvent(kActionSkip);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		cutSceneKeyMap->addAction(act);
	}

	act = new Action("PAUSE", _("Pause"));
	act->setCustomEngineActionEvent(kActionPause);
	act->addDefaultInputMapping("C+p");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("QUITMENU", _("Quit to main menu"));
	act->setCustomEngineActionEvent(kActionQuitToMainMenu);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	gameKeyMap->addAction(act);

	act = new Action("SAVE", _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("C+s");
	gameKeyMap->addAction(act);

	act = new Action("LOAD", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("C+l");
	act->addDefaultInputMapping("C+o");
	gameKeyMap->addAction(act);

	act = new Action("MOVEUP", _("Look up"));
	act->setCustomEngineActionEvent(kActionMoveUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("KP8");
	act->addDefaultInputMapping("JOY_UP");
	gameKeyMap->addAction(act);

	act = new Action("MOVEDOWN", _("Look down"));
	act->setCustomEngineActionEvent(kActionMoveDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("KP2");
	act->addDefaultInputMapping("JOY_DOWN");
	gameKeyMap->addAction(act);

	act = new Action("MOVELEFT", _("Look left"));
	act->setCustomEngineActionEvent(kActionMoveLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("KP4");
	act->addDefaultInputMapping("JOY_LEFT");
	gameKeyMap->addAction(act);

	act = new Action("MOVERIGHT", _("Look right"));
	act->setCustomEngineActionEvent(kActionMoveRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("KP6");
	act->addDefaultInputMapping("JOY_RIGHT");
	gameKeyMap->addAction(act);

	act = new Action("MOVEFORWARD", _("Move forward"));
	act->setCustomEngineActionEvent(kActionMoveForward);
	act->addDefaultInputMapping("KP5");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	act = new Action("QUITMENUINV", _("Quit to main menu"));
	act->setCustomEngineActionEvent(kActionQuitToMainMenuInv);
	act->addDefaultInputMapping("C+q");
	gameKeyMap->addAction(act);

	// I18N: AI is Artificial Intelligence
	act = new Action("AICOMMENTS", _("Replay last AI comment"));
	act->setCustomEngineActionEvent(kActionAIComment);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	inventoryKeyMap->addAction(act);

	// I18N: The game has an inventory with a list of biochips. This action is used to switch the equipped biochip to the Artificial Intelligence biochip.
	act = new Action("BIOAI", _("Biochip AI"));
	act->setCustomEngineActionEvent(kActionBiochipAI);
	act->addDefaultInputMapping("C+a");
	inventoryKeyMap->addAction(act);

	// I18N: The game has an inventory with a list of biochips. This action is used to switch the equipped biochip to the blank biochip.
	act = new Action("BIOBLANK", _("Biochip blank"));
	act->setCustomEngineActionEvent(kActionBiochipBlank);
	act->addDefaultInputMapping("C+b");
	inventoryKeyMap->addAction(act);

	// I18N: The game has an inventory with a list of biochips. This action is used to switch the equipped biochip to the cloak biochip.
	act = new Action("BIOCLOAK", _("Biochip cloak"));
	act->setCustomEngineActionEvent(kActionBiochipCloak);
	act->addDefaultInputMapping("C+c");
	inventoryKeyMap->addAction(act);

	// I18N: The game has an inventory with a list of biochips. This action is used to switch the equipped biochip to the evidence biochip.
	act = new Action("BIOEVIDENCE", _("Biochip evidence"));
	act->setCustomEngineActionEvent(kActionBiochipEvidence);
	act->addDefaultInputMapping("C+e");
	inventoryKeyMap->addAction(act);

	// I18N: The game has an inventory with a list of biochips. This action is used to switch the equipped biochip to the files biochip.
	act = new Action("BIOFILES", _("Biochip files"));
	act->setCustomEngineActionEvent(kActionBiochipFiles);
	act->addDefaultInputMapping("C+f");
	inventoryKeyMap->addAction(act);

	// I18N: The game has an inventory with a list of biochips. This action is used to switch the equipped biochip to the interface biochip.
	act = new Action("BIOINTERFACE", _("Biochip interface"));
	act->setCustomEngineActionEvent(kActionBiochipInterface);
	act->addDefaultInputMapping("C+i");
	inventoryKeyMap->addAction(act);

	// I18N: The game has an inventory with a list of biochips. This action is used to switch the equipped biochip to the jump biochip.
	act = new Action("BIOJUMP", _("Biochip jump"));
	act->setCustomEngineActionEvent(kActionBiochipJump);
	act->addDefaultInputMapping("C+j");
	inventoryKeyMap->addAction(act);

	// I18N: The game has an inventory with a list of biochips. This action is used to switch the equipped biochip to the translate biochip.
	act = new Action("BIOTRANSLATE", _("Biochip translate"));
	act->setCustomEngineActionEvent(kActionBiochipTranslate);
	act->addDefaultInputMapping("C+t");
	inventoryKeyMap->addAction(act);

	// I18N: Shows a summary of collected points in the game.
	act = new Action("SHOWPOINTS", _("Show points"));
	act->setCustomEngineActionEvent(kActionPoints);
	act->addDefaultInputMapping("C+d");
	inventoryKeyMap->addAction(act);

	Common::KeymapArray keymaps(5);

	keymaps[0] = engineKeyMap;
	keymaps[1] = cutSceneKeyMap;
	keymaps[2] = mainMenuKeyMap;
	keymaps[3] = gameKeyMap;
	keymaps[4] = inventoryKeyMap;

	cutSceneKeyMap->setEnabled(false);
	mainMenuKeyMap->setEnabled(false);
	gameKeyMap->setEnabled(false);
	inventoryKeyMap->setEnabled(false);

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(BURIED)
	REGISTER_PLUGIN_DYNAMIC(BURIED, PLUGIN_TYPE_ENGINE, BuriedMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(BURIED, PLUGIN_TYPE_ENGINE, BuriedMetaEngine);
#endif
