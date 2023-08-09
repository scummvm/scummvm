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
#include "common/system.h"
#include "common/translation.h"
#include "crab/crab.h"
#include "crab/loaders.h"
#include "crab/XMLDoc.h"
#include "crab/input/input.h"

namespace Crab {

using namespace pyrodactyl::input;

//------------------------------------------------------------------------
// Purpose: Return pressed/depressed state of key
//------------------------------------------------------------------------
bool InputManager::state(const InputType &val) {
	return _ivState[val];
}

void InputManager::populateKeyTable() {
	for (uint type = IG_START; type < IT_TOTAL; type++) {
		_keyDescs[type] = '\0';
	}

	setKeyBindingMode(KBM_GAME);
	for (uint i = IG_START; i < IG_SIZE + IG_START; i++) {
		getAssociatedKey((InputType)i);
	}

	setKeyBindingMode(KBM_UI);
	for (uint i = IU_START; i < IU_SIZE + IU_START; i++) {
		getAssociatedKey((InputType)i);
	}
}

Common::String InputManager::getAssociatedKey(const InputType &type) {
	// Return cached copy if available
	if (_keyDescs[type].size() > 0)
		return _keyDescs[type];

	Common::KeymapArray keymapArr = g_system->getEventManager()->getKeymapper()->getKeymaps();
	for(Common::Keymap *keymap : keymapArr) {
		if (keymap->getType() != Common::Keymap::kKeymapTypeGame)
			continue;

		const Common::Keymap::ActionArray actions = keymap->getActions();
		for (Common::Action *action : actions) {
			if ((int)action->event.customType == type) {
				_keyDescs[type] = Common::String(keymap->getActionMapping(action)[0].description);
				_keyDescs[type].toUppercase();
				break;
			}
		}
	}

	return _keyDescs[type];
}

Common::Keymap* InputManager::getDefaultKeyMapsForGame() {
	using namespace Common;

	Keymap *keymap = new Keymap(Keymap::kKeymapTypeGame, "Unrest-Game", "Keymappings for Game");

	Action *act;

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setCustomEngineActionEvent(IG_UP);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	keymap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setCustomEngineActionEvent(IG_DOWN);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	keymap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setCustomEngineActionEvent(IG_LEFT);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	keymap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setCustomEngineActionEvent(IG_RIGHT);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	keymap->addAction(act);

	act = new Action("TALK", _("Talk/Interact"));
	act->setCustomEngineActionEvent(IG_TALK);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_A");
	keymap->addAction(act);

	return keymap;
}

Common::Keymap* InputManager::getDefaultKeyMapsForUI() {
	using namespace Common;

	Keymap *uiKeymap = new Keymap(Keymap::kKeymapTypeGame, "Unrest-UI", "Keymappings for UI");

	Action *act;

	act = new Action("UI_UP", _("Up"));
	act->setCustomEngineActionEvent(IU_UP);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	uiKeymap->addAction(act);

	act = new Action("UI_DOWN", _("Down"));
	act->setCustomEngineActionEvent(IU_DOWN);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	uiKeymap->addAction(act);

	act = new Action("UI_LEFT", _("Left"));
	act->setCustomEngineActionEvent(IU_LEFT);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	uiKeymap->addAction(act);

	act = new Action("UI_RIGHT", _("Right"));
	act->setCustomEngineActionEvent(IU_RIGHT);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	uiKeymap->addAction(act);

	act = new Action("UI_ACCEPT", _("Accept"));
	act->setCustomEngineActionEvent(IU_ACCEPT);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_A");
	uiKeymap->addAction(act);

	act = new Action("REPLY1", _("Reply 1"));
	act->setCustomEngineActionEvent(IU_REPLY_0);
	act->addDefaultInputMapping("1");
	uiKeymap->addAction(act);

	act = new Action("REPLY2", _("Reply 2"));
	act->setCustomEngineActionEvent(IU_REPLY_1);
	act->addDefaultInputMapping("2");
	uiKeymap->addAction(act);

	act = new Action("REPLY3", _("Reply 3"));
	act->setCustomEngineActionEvent(IU_REPLY_2);
	act->addDefaultInputMapping("3");
	uiKeymap->addAction(act);

	act = new Action("REPLY4", _("Reply 4"));
	act->setCustomEngineActionEvent(IU_REPLY_3);
	act->addDefaultInputMapping("4");
	uiKeymap->addAction(act);

	act = new Action("REPLY5", _("Reply 5"));
	act->setCustomEngineActionEvent(IU_REPLY_4);
	act->addDefaultInputMapping("5");
	uiKeymap->addAction(act);

	act = new Action("REPLY6", _("Reply 6"));
	act->setCustomEngineActionEvent(IU_REPLY_5);
	act->addDefaultInputMapping("6");
	uiKeymap->addAction(act);

	return uiKeymap;
}

Common::Keymap* InputManager::getDefaultKeyMapsForHUD() {
	using namespace Common;

	Keymap *hudKeymap = new Keymap(Keymap::kKeymapTypeGame, "Unrest-HUD", "Keymappings for HUD");

	Action *act;

	act = new Action("MAP", _("Map"));
	act->setCustomEngineActionEvent(IG_MAP);
	act->addDefaultInputMapping("m");
	hudKeymap->addAction(act);

	act = new Action("JOURNAL", _("Journal"));
	act->setCustomEngineActionEvent(IG_JOURNAL);
	act->addDefaultInputMapping("j");
	hudKeymap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(IG_INVENTORY);
	act->addDefaultInputMapping("i");
	hudKeymap->addAction(act);

	act = new Action("TRAITS", _("Traits"));
	act->setCustomEngineActionEvent(IG_CHARACTER);
	act->addDefaultInputMapping("t");
	hudKeymap->addAction(act);

	return hudKeymap;
}

void InputManager::setKeyBindingMode(KeyBindingMode mode) {
	_keyMode = mode;

	Common::Keymapper *const mapper = g_engine->getEventManager()->getKeymapper();
	mapper->disableAllGameKeymaps();

	mapper->setGameKeymapState("Unrest-HUD", true);

	switch (mode) {
	case KBM_GAME:
		mapper->setGameKeymapState("Unrest-Game", true);
		break;

	case KBM_UI:
		mapper->setGameKeymapState("Unrest-UI", true);
		break;
	}

	// Clear All inputs
	clearInputs();
}

} // End of namespace Crab
