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

#include "colony/colony.h"
#include "colony/detection.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/config-manager.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

namespace Colony {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_WIDESCREEN,
		{
			_s("Widescreen mod"),
			_s("Enable widescreen rendering in fullscreen mode."),
			"widescreen_mod",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class ColonyMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "colony";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override {
		*engine = new ColonyEngine(syst, gd);
		return Common::kNoError;
	}

	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::KeymapArray ColonyMetaEngine::initKeymaps(const char *target) const {
	Common::Keymap *engineKeyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "colony", "The Colony");
	Common::Action *act;

	act = new Common::Action(Common::kStandardActionMoveUp, _("Move forward"));
	act->setCustomEngineActionEvent(kActionMoveForward);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("w");
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionMoveDown, _("Move backward"));
	act->setCustomEngineActionEvent(kActionMoveBackward);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("s");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionMoveLeft, _("Strafe left"));
	act->setCustomEngineActionEvent(kActionStrafeLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("a");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionMoveRight, _("Strafe right"));
	act->setCustomEngineActionEvent(kActionStrafeRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("d");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Common::Action("ROTL", _("Rotate left"));
	act->setCustomEngineActionEvent(kActionRotateLeft);
	act->addDefaultInputMapping("q");
	engineKeyMap->addAction(act);

	act = new Common::Action("ROTR", _("Rotate right"));
	act->setCustomEngineActionEvent(kActionRotateRight);
	act->addDefaultInputMapping("e");
	engineKeyMap->addAction(act);

	act = new Common::Action("LOOKL", _("Look left"));
	act->setCustomEngineActionEvent(kActionLookLeft);
	act->addDefaultInputMapping("z");
	engineKeyMap->addAction(act);

	act = new Common::Action("LOOKR", _("Look right"));
	act->setCustomEngineActionEvent(kActionLookRight);
	act->addDefaultInputMapping("c");
	engineKeyMap->addAction(act);

	act = new Common::Action("LOOKB", _("Look behind"));
	act->setCustomEngineActionEvent(kActionLookBehind);
	act->addDefaultInputMapping("x");
	engineKeyMap->addAction(act);

	act = new Common::Action("MOUSE", _("Toggle mouselook"));
	act->setCustomEngineActionEvent(kActionToggleMouselook);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Common::Action("DASH", _("Toggle dashboard"));
	act->setCustomEngineActionEvent(kActionToggleDashboard);
	act->addDefaultInputMapping("F7");
	engineKeyMap->addAction(act);

	act = new Common::Action("WIRE", _("Toggle wireframe"));
	act->setCustomEngineActionEvent(kActionToggleWireframe);
	act->addDefaultInputMapping("F8");
	engineKeyMap->addAction(act);

	act = new Common::Action("FULL", _("Toggle fullscreen"));
	act->setCustomEngineActionEvent(kActionToggleFullscreen);
	act->addDefaultInputMapping("F11");
	engineKeyMap->addAction(act);

	act = new Common::Action("SKIP", _("Skip intro"));
	act->setCustomEngineActionEvent(kActionSkipIntro);
	act->addDefaultInputMapping("S+s");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Common::Action("ESCAPE", _("Menu"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_BACK");
	engineKeyMap->addAction(act);

	act = new Common::Action("FIRE", _("Fire weapon"));
	act->setCustomEngineActionEvent(kActionFire);
	act->addDefaultInputMapping("f");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	return Common::Keymap::arrayOf(engineKeyMap);
}

} // End of namespace Colony

#if PLUGIN_ENABLED_DYNAMIC(COLONY)
REGISTER_PLUGIN_DYNAMIC(COLONY, PLUGIN_TYPE_ENGINE, Colony::ColonyMetaEngine);
#else
REGISTER_PLUGIN_STATIC(COLONY, PLUGIN_TYPE_ENGINE, Colony::ColonyMetaEngine);
#endif
