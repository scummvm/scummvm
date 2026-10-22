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

#include "common/translation.h"

#include "scooby/metaengine.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"
#include "scooby/detection.h"
#include "scooby/scooby.h"

namespace Scooby {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"original_menus",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Scooby

const char *ScoobyMetaEngine::getName() const {
	return "scooby";
}

const ADExtraGuiOptionsMap *ScoobyMetaEngine::getAdvancedExtraGuiOptions() const {
	return Scooby::optionsList;
}

Common::Error ScoobyMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Scooby::ScoobyEngine(syst, desc);
	return Common::kNoError;
}

bool ScoobyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

Common::KeymapArray ScoobyMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "scooby", "Scooby-Doo Mystery");

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Select"));
	act->setCustomEngineActionEvent(Scooby::kScoobyActionSelect);
	// The mouse buttons are handled directly by the engine (input/mouse_input.h), not mapped here.
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping("x");
	engineKeyMap->addAction(act);

	act = new Action("CANCEL", _("Cancel"));
	act->setCustomEngineActionEvent(Scooby::kScoobyActionCancel);
	act->addDefaultInputMapping("JOY_A");
	act->addDefaultInputMapping("z");
	engineKeyMap->addAction(act);

	act = new Action("TOGGLEINVENTORY", _("Toggle Inventory"));
	act->setCustomEngineActionEvent(Scooby::kScoobyActionToggleInventory);
	act->addDefaultInputMapping("JOY_C");
	act->addDefaultInputMapping("c");
	engineKeyMap->addAction(act);

	act = new Action("QUIT", _("Quit game"));
	act->setCustomEngineActionEvent(Scooby::kScoobyActionQuit);
	act->addDefaultInputMapping("C+q");
	engineKeyMap->addAction(act);

	act = new Action("SKIPCUTSCENE", _("Skip cutscene"));
	act->setCustomEngineActionEvent(Scooby::kScoobyActionSkipCutscene);
	act->addDefaultInputMapping("ESCAPE");
	engineKeyMap->addAction(act);

	act = new Action("START", _("Start"));
	act->setCustomEngineActionEvent(Scooby::kScoobyActionStart);
	act->addDefaultInputMapping("RETURN");
	engineKeyMap->addAction(act);

	act = new Action("UP", _("Up"));
	act->setCustomEngineActionEvent(Scooby::kScoobyActionUp);
	act->addDefaultInputMapping("UP");
	engineKeyMap->addAction(act);

	act = new Action("DOWN", _("Down"));
	act->setCustomEngineActionEvent(Scooby::kScoobyActionDown);
	act->addDefaultInputMapping("DOWN");
	engineKeyMap->addAction(act);

	act = new Action("LEFT", _("Left"));
	act->setCustomEngineActionEvent(Scooby::kScoobyActionLeft);
	act->addDefaultInputMapping("LEFT");
	engineKeyMap->addAction(act);

	act = new Action("RIGHT", _("Right"));
	act->setCustomEngineActionEvent(Scooby::kScoobyActionRight);
	act->addDefaultInputMapping("RIGHT");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

#if PLUGIN_ENABLED_DYNAMIC(SCOOBY)
REGISTER_PLUGIN_DYNAMIC(SCOOBY, PLUGIN_TYPE_ENGINE, ScoobyMetaEngine);
#else
REGISTER_PLUGIN_STATIC(SCOOBY, PLUGIN_TYPE_ENGINE, ScoobyMetaEngine);
#endif
