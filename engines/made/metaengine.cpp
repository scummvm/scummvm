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
#include "made/made.h"
#include "made/detection.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"


namespace Made {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_INTRO_MUSIC_DIGITAL,
		{
			_s("Play a digital soundtrack during the opening movie"),
			_s("If selected, the game will use a digital soundtrack during the introduction. Otherwise, it will play MIDI music."),
			"intro_music_digital",
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

uint32 MadeEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 MadeEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform MadeEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 MadeEngine::getVersion() const {
	return _gameDescription->version;
}

} // End of namespace Made

class MadeMetaEngine : public AdvancedMetaEngine<Made::MadeGameDescription> {
public:
	const char *getName() const override {
		return "made";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Made::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const Made::MadeGameDescription *desc) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool MadeMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		false;
}

bool Made::MadeEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher);
}

Common::Error MadeMetaEngine::createInstance(OSystem *syst, Engine **engine, const Made::MadeGameDescription *desc) const {
	*engine = new Made::MadeEngine(syst,desc);
	return Common::kNoError;
}

Common::KeymapArray MadeMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Made;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "made-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	act->addDefaultInputMapping("KP_PLUS");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping("KP_MINUS");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionSkip, _("Skip"));
	act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action("CRSORUP", _("Cursor up"));
	act->setCustomEngineActionEvent(kActionCursorUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("KP8");
	act->allowKbdRepeats();
	gameKeyMap->addAction(act);

	act = new Action("CRSORDOWN", _("Cursor down"));
	act->setCustomEngineActionEvent(kActionCursorDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("KP2");
	act->allowKbdRepeats();
	gameKeyMap->addAction(act);

	act = new Action("CRSORLEFT", _("Cursor left"));
	act->setCustomEngineActionEvent(kActionCursorLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("KP4");
	act->allowKbdRepeats();
	gameKeyMap->addAction(act);

	act = new Action("CRSORRIGHT", _("Cursor right"));
	act->setCustomEngineActionEvent(kActionCursorRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("KP6");
	act->allowKbdRepeats();
	gameKeyMap->addAction(act);

	act = new Action("MENU", _("Menu"));
	act->setCustomEngineActionEvent(kActionMenu);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_GUIDE");
	gameKeyMap->addAction(act);

	act = new Action("SAVEGAME", _("Save game"));
	act->setCustomEngineActionEvent(kActionSaveGame);
	act->addDefaultInputMapping("F2");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("LOADGAME", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoadGame);
	act->addDefaultInputMapping("F3");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("RPTMSG", _("Repeat last message"));
	act->setCustomEngineActionEvent(kActionRepeatMessage);
	act->addDefaultInputMapping("F4");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(MADE)
	REGISTER_PLUGIN_DYNAMIC(MADE, PLUGIN_TYPE_ENGINE, MadeMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MADE, PLUGIN_TYPE_ENGINE, MadeMetaEngine);
#endif
