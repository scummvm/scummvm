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

#include "base/plugins.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "common/translation.h"
#include "harvester/detection.h"
#include "harvester/metaengine.h"
#include "harvester/harvester.h"
#include "harvester/scene/input.h"

static const ADExtraGuiOptionsMap gameGuiOptions[] = {
	{
		GAMEOPTION_GORE,
		{
			_s("Enable gore"),
			_s("When disabled, the startup movie player follows the original GORE=NO censorship path."),
			"gore",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_SHOW_CD_CHANGE_PROMPTS,
		{
			_s("Show CD change prompts"),
			_s("Display the original CD swap image and wait for a left click when the script requests disc 2 or 3."),
			"show_cd_change_prompts",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const char *HarvesterMetaEngine::getName() const {
	return "harvester";
}

Common::KeymapArray HarvesterMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Harvester;

	KeymapArray keymaps = MetaEngine::initKeymaps(target);
	Keymap *gameplay = new Keymap(Keymap::kKeymapTypeGame, "harvester-gameplay", _("Gameplay"));
	Action *click = new Action(kStandardActionLeftClick, _("Left click"));
	click->setLeftClickEvent();
	click->addDefaultInputMapping("MOUSE_LEFT");
	click->addDefaultInputMapping("JOY_A");
	gameplay->addAction(click);
	click = new Action(kStandardActionRightClick, _("Right click"));
	click->setRightClickEvent();
	click->addDefaultInputMapping("MOUSE_RIGHT");
	click->addDefaultInputMapping("JOY_B");
	gameplay->addAction(click);
	const struct {
		const char *id;
		const char *description;
		GameAction action;
		const char *mapping;
		const char *alternateMapping;
	} bindings[] = {
		{ "LEFT", _s("Move left"), kActionMoveLeft, "LEFT", "JOY_LEFT" },
		{ "RIGHT", _s("Move right"), kActionMoveRight, "RIGHT", "JOY_RIGHT" },
		{ "UP", _s("Move up"), kActionMoveUp, "UP", "JOY_UP" },
		{ "DOWN", _s("Move down"), kActionMoveDown, "DOWN", "JOY_DOWN" },
		{ "SPEEDUP", _s("Speed up"), kActionSpeedUp, "LALT", "RALT" },
		{ "INVENTORY", _s("Inventory"), kActionInventory, "i", nullptr },
		{ "MENU", _s("Game menu"), kActionMenu, "ESCAPE", "JOY_Y" },
		{ "HELP", _s("Help menu"), kActionHelp, "F1", nullptr },
		{ "ATTACK", _s("Attack modifier"), kActionAttackModifier, "LCTRL", "RCTRL" },
		{ "HIGHATTACK", _s("High attack"), kActionHighAttack, nullptr, nullptr },
		{ "MEDIUMATTACK", _s("Medium attack"), kActionMediumAttack, nullptr, nullptr },
		{ "LOWATTACK", _s("Low attack"), kActionLowAttack, nullptr, nullptr },
		{ "EVADE", _s("Evasive movement modifier"), kActionEvasiveModifier, "RSHIFT", nullptr },
		{ "DUCK", _s("Duck"), kActionDuck, nullptr, nullptr },
		{ "JUMP", _s("Jump"), kActionJump, nullptr, nullptr }
	};
	for (const auto &binding : bindings) {
		Action *action = new Action(binding.id, _(binding.description));
		action->setCustomEngineActionEvent(binding.action);
		if (binding.action <= kActionSpeedUp || binding.action >= kActionHighAttack)
			action->allowKbdRepeats();
		if (binding.mapping)
			action->addDefaultInputMapping(binding.mapping);
		if (binding.alternateMapping)
			action->addDefaultInputMapping(binding.alternateMapping);
		gameplay->addAction(action);
	}
	gameplay->setEnabled(false);
	keymaps.push_back(gameplay);
	return keymaps;
}

const ADExtraGuiOptionsMap *HarvesterMetaEngine::getAdvancedExtraGuiOptions() const {
	return gameGuiOptions;
}

Common::Error HarvesterMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Harvester::HarvesterEngine(syst, desc);
	return Common::kNoError;
}

int HarvesterMetaEngine::getMaximumSaveSlot() const {
	return 24;
}

int HarvesterMetaEngine::getAutosaveSlot() const {
	return -1;
}

bool HarvesterMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(HARVESTER)
REGISTER_PLUGIN_DYNAMIC(HARVESTER, PLUGIN_TYPE_ENGINE, HarvesterMetaEngine);
#else
REGISTER_PLUGIN_STATIC(HARVESTER, PLUGIN_TYPE_ENGINE, HarvesterMetaEngine);
#endif
