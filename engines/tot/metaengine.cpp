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
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "common/translation.h"

#include "tot/metaengine.h"
#include "tot/detection.h"
#include "tot/statics.h"
#include "tot/tot.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COPY_PROTECTION,
		{
			_s("Copy protection"),
			_s("Enable copy protection"),
			"copy_protection",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_NO_TRANSITIONS,
		{
			_s("Disable scene transitions"),
			_s("Disable original transition effects between scenes"),
			"transitions_disable",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_ORIGINAL_SAVELOAD_DIALOG,
		{
			_s("Original save/load dialog"),
			_s("Use original save and load dialogs"),
			"originalsaveload",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_OPL3_MODE,
		{
			_s("AdLib OPL3 mode"),
			_s("When AdLib is selected, OPL3 features will be used. Depending on the game, this will prevent cut-off notes, add extra notes or instruments and/or add stereo."),
			"opl3_mode",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const char *TotMetaEngine::getName() const {
	return "tot";
}

Common::Error TotMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Tot::TotEngine(syst, desc);
	return Common::kNoError;
}

bool TotMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
	(f == kSupportsLoadingDuringStartup);
}

const ADExtraGuiOptionsMap *TotMetaEngine::getAdvancedExtraGuiOptions() const {
	return optionsList;
}

Common::KeymapArray TotMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Tot;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "tot-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *quitDialogKeyMap = new Keymap(Keymap::kKeymapTypeGame, "quit-dialog", _("Quit dialog keymappings"));

	Common::Language lang = Common::parseLanguage(ConfMan.get("language", target));

	static const char *const *defaultTotKeys = (lang == ES_ESP) ? keys[0] : keys[1];

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Move / Interact"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Default action"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("TALK", _("Talk"));
	act->setCustomEngineActionEvent(kActionTalk);
	act->addDefaultInputMapping(defaultTotKeys[KEY_TALK]);
	gameKeyMap->addAction(act);

	act = new Action("PICK", _("Pick"));
	act->setCustomEngineActionEvent(kActionPickup);
	act->addDefaultInputMapping(defaultTotKeys[KEY_PICKUP]);
	gameKeyMap->addAction(act);

	act = new Action("LOOK", _("Look"));
	act->setCustomEngineActionEvent(kActionLookAt);
	act->addDefaultInputMapping(defaultTotKeys[KEY_LOOKAT]);
	gameKeyMap->addAction(act);

	act = new Action("USE", _("Use"));
	act->setCustomEngineActionEvent(kActionUse);
	act->addDefaultInputMapping(defaultTotKeys[KEY_USE]);
	gameKeyMap->addAction(act);

	act = new Action("OPEN", _("Open"));
	act->setCustomEngineActionEvent(kActionOpen);
	act->addDefaultInputMapping(defaultTotKeys[KEY_OPEN]);
	gameKeyMap->addAction(act);

	act = new Action("CLOSE", _("Close"));
	act->setCustomEngineActionEvent(kActionClose);
	act->addDefaultInputMapping(defaultTotKeys[KEY_CLOSE]);
	gameKeyMap->addAction(act);

	act = new Action("SAVELOAD", _("Save and load game"));
	act->setCustomEngineActionEvent(kActionSaveLoad);
	act->addDefaultInputMapping("F2");
	gameKeyMap->addAction(act);

	act = new Action("VOLCONTROLS", _("Volume controls"));
	act->setCustomEngineActionEvent(kActionVolume);
	act->addDefaultInputMapping("F1");
	gameKeyMap->addAction(act);

	act = new Action("MAINMENU", _("Main menu/Exit"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	gameKeyMap->addAction(act);

	act = new Action("QUITCONFIRM", _("Confirm quit"));
	act->setCustomEngineActionEvent(kActionYes);
	act->addDefaultInputMapping(defaultTotKeys[KEY_YES]);
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	quitDialogKeyMap->addAction(act);

	act = new Action("QUITCANCEL", _("Cancel quit"));
	act->setCustomEngineActionEvent(kActionNo);
	act->addDefaultInputMapping(defaultTotKeys[KEY_NO]);
	act->addDefaultInputMapping("JOY_KEFT_TRIGGER");
	quitDialogKeyMap->addAction(act);

	KeymapArray keymaps(3);

	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;
	keymaps[2] = quitDialogKeyMap;

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(TOT)
REGISTER_PLUGIN_DYNAMIC(TOT, PLUGIN_TYPE_ENGINE, TotMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TOT, PLUGIN_TYPE_ENGINE, TotMetaEngine);
#endif
