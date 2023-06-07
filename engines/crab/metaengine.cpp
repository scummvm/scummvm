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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

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

#include "crab/crab.h"
#include "crab/detection.h"
#include "crab/metaengine.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace Crab {

static const ADExtraGuiOptionsMap optionsList[] = {
	{GAMEOPTION_ORIGINAL_SAVELOAD,
	 {_s("Use original save/load screens"),
	  _s("Use the original save/load screens instead of the ScummVM ones"),
	  "original_menus",
	  false,
	  0,
	  0}},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR};

} // End of namespace Crab

const char *CrabMetaEngine::getName() const {
	return "crab";
}

const ADExtraGuiOptionsMap *CrabMetaEngine::getAdvancedExtraGuiOptions() const {
	return Crab::optionsList;
}

Common::Error CrabMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Crab::CrabEngine(syst, desc);
	return Common::kNoError;
}

Common::KeymapArray CrabMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *keymap = new Keymap(Keymap::kKeymapTypeGame, "Unrest", "Unrest");

	Action *act;

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IG_UP);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	keymap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IG_DOWN);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	keymap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IG_LEFT);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	keymap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IG_RIGHT);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	keymap->addAction(act);

	act = new Action("TALK", _("Talk/Interact"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IG_TALK);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_A");
	keymap->addAction(act);

	act = new Action("MAP", _("Map"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IG_MAP);
	act->addDefaultInputMapping("m");
	keymap->addAction(act);

	act = new Action("JOURNAL", _("Journal"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IG_JOURNAL);
	act->addDefaultInputMapping("j");
	keymap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IG_INVENTORY);
	act->addDefaultInputMapping("i");
	keymap->addAction(act);

	act = new Action("TRAITS", _("Traits"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IG_CHARACTER);
	act->addDefaultInputMapping("t");
	keymap->addAction(act);


	act = new Action("UI_UP", _("Up"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IU_UP);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	keymap->addAction(act);

	act = new Action("UI_DOWN", _("Down"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IU_DOWN);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	keymap->addAction(act);

	act = new Action("UI_LEFT", _("Left"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IU_LEFT);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	keymap->addAction(act);

	act = new Action("UI_RIGHT", _("Right"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IU_RIGHT);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	keymap->addAction(act);

	act = new Action("REPLY1", _("Reply 1"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IU_REPLY_0);
	act->addDefaultInputMapping("1");
	keymap->addAction(act);

	act = new Action("REPLY2", _("Reply 2"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IU_REPLY_1);
	act->addDefaultInputMapping("2");
	keymap->addAction(act);

	act = new Action("REPLY3", _("Reply 3"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IU_REPLY_2);
	act->addDefaultInputMapping("3");
	keymap->addAction(act);

	act = new Action("REPLY4", _("Reply 4"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IU_REPLY_3);
	act->addDefaultInputMapping("4");
	keymap->addAction(act);

	act = new Action("REPLY5", _("Reply 5"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IU_REPLY_4);
	act->addDefaultInputMapping("5");
	keymap->addAction(act);

	act = new Action("REPLY6", _("Reply 6"));
	act->setCustomEngineActionEvent(Crab::pyrodactyl::input::IU_REPLY_5);
	act->addDefaultInputMapping("6");
	keymap->addAction(act);

	return Keymap::arrayOf(keymap);
}

bool CrabMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		   (f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(CRAB)
REGISTER_PLUGIN_DYNAMIC(CRAB, PLUGIN_TYPE_ENGINE, CrabMetaEngine);
#else
REGISTER_PLUGIN_STATIC(CRAB, PLUGIN_TYPE_ENGINE, CrabMetaEngine);
#endif
