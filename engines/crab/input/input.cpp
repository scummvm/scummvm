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
#include "common/translation.h"

#include "crab/crab.h"
#include "crab/input/input.h"

namespace Crab {

using namespace pyrodactyl::input;
//using namespace boost::filesystem;

//------------------------------------------------------------------------
// Purpose: Return pressed/depressed state of key
//------------------------------------------------------------------------
bool InputManager::State(const InputType &val) {
	return _ivState[val];

#if 0
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if (keystate[iv[val].key] || keystate[iv[val].alt])
		return true;

	if (controller != nullptr) {
		if (iv[val].c_bu >= 0 && SDL_GameControllerGetButton(controller, iv[val].c_bu) == SDL_PRESSED)
			return true;
		else if (iv[val].c_ax.id != SDL_CONTROLLER_AXIS_INVALID) {
			int av = SDL_GameControllerGetAxis(controller, iv[val].c_ax.id);
			return (iv[val].c_ax.greater && av > iv[val].c_ax.val) || (!iv[val].c_ax.greater && av < iv[val].c_ax.val);
		}
	}
#endif

	//return false;
}

#if 0
//------------------------------------------------------------------------
// Purpose: Check for controller and keyboard input simultaneously
// Return value 2 is a special case used for analog stick hotkey input
//------------------------------------------------------------------------
const int InputManager::Equals(const InputType &val, const SDL_Event &Event) {
	switch (Event.type) {
	case SDL_KEYDOWN:
		if (iv[val].Equals(Event.key))
			return SDL_PRESSED;
		break;
	case SDL_KEYUP:
		if (iv[val].Equals(Event.key))
			return SDL_RELEASED;
		break;

	case SDL_CONTROLLERBUTTONDOWN:
		if (iv[val].Equals(Event.cbutton))
			return SDL_PRESSED;
		break;
	case SDL_CONTROLLERBUTTONUP:
		if (iv[val].Equals(Event.cbutton))
			return SDL_RELEASED;
		break;

	case SDL_CONTROLLERAXISMOTION:
		// HACK to make left analog stick work in menus and right analog stick work in hotkeys
		if (iv[val].Equals(Event.caxis)) {
			if (iv[val].c_ax.id == SDL_CONTROLLER_AXIS_LEFTX || iv[val].c_ax.id == SDL_CONTROLLER_AXIS_LEFTY)
				return SDL_PRESSED;
			else
				return ANALOG_PRESSED;
		}
		break;
	default:
		break;
	}

	return -1;
}
#endif

//------------------------------------------------------------------------
// Purpose: Load from file
//------------------------------------------------------------------------
void InputManager::Init() {

	const Common::String DEFAULT_FILENAME = "res/controls.xml";
	Load(DEFAULT_FILENAME);

#if 0
	const Common::String DEFAULT_FILENAME = "res/controls.xml";

	Common::String filename = g_engine->_filePath->appdata.c_str();
	filename += "controls.xml";

	if (!is_regular_file(filename)) {
		// The other file does not exist, just use the default file
		Load(DEFAULT_FILENAME);
	} else {
		// We are using the other file, check if it is up to date or not
		if (Version(DEFAULT_FILENAME) > Version(filename)) {
			// The game has been updated to a different control scheme, use the default file
			Load(DEFAULT_FILENAME);
		} else {
			// The version set by the player is fine, just use that
			Load(filename);
		}
	}

	CreateBackup();
#endif
}

//------------------------------------------------------------------------
// Purpose: Load key & controller binding settings from file
//------------------------------------------------------------------------
void InputManager::Load(const Common::String &filename) {
	XMLDoc control_list(filename);
	if (control_list.ready()) {
		rapidxml::xml_node<char> *node = control_list.Doc()->first_node("controls");
		if (NodeValid(node)) {
			LoadNum(version, "version", node);

			int i = 0;
			for (auto n = node->first_node(); n != NULL && i < IT_TOTAL; n = n->next_sibling(), ++i)
				iv[i].LoadState(n);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Initialize the controller if it is plugged in
//------------------------------------------------------------------------
void InputManager::AddController() {
	warning("STUB: InputManager::AddController()");

#if 0
	if (SDL_NumJoysticks() > 0) {
		if (SDL_IsGameController(0))
			controller = SDL_GameControllerOpen(0);
		else
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Controller not recognized", "Unsupported device", NULL);
	}
#endif
}

#if 0
void InputManager::HandleController(const SDL_Event &Event) {
	if (Event.type == SDL_CONTROLLERDEVICEREMOVED && controller != nullptr) {
		SDL_GameControllerClose(controller);
		controller = nullptr;
	} else if (Event.type == SDL_CONTROLLERDEVICEADDED && controller == nullptr)
		AddController();
}
#endif

//------------------------------------------------------------------------
// Purpose: Create and restore backup
//------------------------------------------------------------------------
void InputManager::CreateBackup() {
	for (int i = 0; i < IT_TOTAL; ++i)
		backup[i] = iv[i];
}

void InputManager::RestoreBackup() {
	for (int i = 0; i < IT_TOTAL; ++i)
		iv[i] = backup[i];
}

void InputManager::PopulateKeyTable() {
	for (unsigned int type = IG_START; type < IT_TOTAL; type++) {
		_keyDescs[type] = '\0';
	}

	SetKeyBindingMode(KBM_GAME);
	for (unsigned int i = IG_START; i < IG_SIZE + IG_START; i++) {
		GetAssociatedKey((InputType)i);
	}

	SetKeyBindingMode(KBM_UI);
	for (unsigned int i = IU_START; i < IU_SIZE + IU_START; i++) {
		GetAssociatedKey((InputType)i);
	}
}

Common::String InputManager::GetAssociatedKey(const InputType &type) {
	// Return cached copy if available
	if (_keyDescs[type].size() > 0)
		return _keyDescs[type];

	Common::Keymap *keymap = g_system->getEventManager()->getKeymapper()->getKeymap("Unrest");
	const Common::Keymap::ActionArray actions = keymap->getActions();
	for (Common::Action *action : actions) {
		if ((int)action->event.customType == type) {
			_keyDescs[type] = Common::String(keymap->getActionMapping(action)[0].description);
			_keyDescs[type].toUppercase();
			break;
		}
	}

	return _keyDescs[type];
}

//------------------------------------------------------------------------
// Purpose: Save to file
//------------------------------------------------------------------------
void InputManager::Save() {
	warning("STUB: InputManager::Save()");

#if 0
	rapidxml::xml_document<char> doc;

	Common::String filename = g_engine->_filePath->appdata;
	filename += "controls.xml";

	// xml declaration
	rapidxml::xml_node<char> *decl = doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	// root node
	rapidxml::xml_node<char> *root = doc.allocate_node(rapidxml::node_element, "controls");
	root->append_attribute(doc.allocate_attribute("version", gStrPool->Get(version)));
	for (int i = 0; i < IT_TOTAL; i++)
		iv[i].SaveState(doc, root, "i");

	doc.append_node(root);
	Common::String xml_as_string;
	rapidxml::print(std::back_inserter(xml_as_string), doc);

	std::ofstream save(filename, std::ios::out);
	if (save.is_open()) {
		save << xml_as_string;
		save.close();
	}

	doc.clear();
	CreateBackup();
#endif
}

Common::Keymap* InputManager::GetDefaultKeyMapsForGame() {
	using namespace Common;

	Keymap *keymap = new Keymap(Keymap::kKeymapTypeGame, "Unrest", "Keymappings for Game");

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

Common::Keymap* InputManager::GetDefaultKeyMapsForUI() {
	using namespace Common;

	Keymap *uiKeymap = new Keymap(Keymap::kKeymapTypeGame, "Unrest", "Keymappings for UI");

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

Common::Keymap* InputManager::GetDefaultKeyMapsForHUD() {
	using namespace Common;

	Keymap *hudKeymap = new Keymap(Keymap::kKeymapTypeGame, "Unrest", "Keymappings for HUD");

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

void InputManager::SetKeyBindingMode(KeyBindingMode mode) {
	_keyMode = mode;

	Common::Keymap *gameKeymaps = GetDefaultKeyMapsForGame();
	Common::Keymap *uiKeymaps = GetDefaultKeyMapsForUI();
	Common::Keymap *hudKeymaps = GetDefaultKeyMapsForHUD();

	Common::Keymapper *const mapper = g_engine->getEventManager()->getKeymapper();
	mapper->cleanupGameKeymaps();

	Common::Keymap *keymap;

	switch (mode) {

		case KBM_GAME:
			keymap = gameKeymaps;
			break;

		case KBM_UI:
			keymap = uiKeymaps;
			break;
	}
	for (Common::Action *a: hudKeymaps->getActions()) {
		keymap->addAction(a);
	}

	mapper->addGameKeymap(keymap);

	// Clear All inputs
	ClearInputs();
}

} // End of namespace Crab
