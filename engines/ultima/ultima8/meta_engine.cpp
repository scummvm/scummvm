/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/meta_engine.h"
#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/ultima8.h"
#include "common/translation.h"
#include "common/ustr.h"
#include "backends/keymapper/action.h"

namespace Ultima {
namespace Ultima8 {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_pressMethod;
	const char *_releaseMethod;
	const char *_key;
	const char *_joy;
	const uint16 _flags;
};

enum KeybindingFlags {
	FLAG_MENU_ENABLED = 0x01
};

static const KeybindingRecord COMMON_KEYS[] = {
	{ ACTION_QUICKSAVE, "QUICKSAVE", "Quick Save", "GUIApp::saveGame QuickSave", nullptr, "F1", nullptr, 0 },
	{ ACTION_SAVE, "SAVE", "Save Game", "GUIApp::saveGame", nullptr, "F5", nullptr, 0 },
	{ ACTION_LOAD, "LOAD", "Load Game", "GUIApp::loadGame", nullptr, "F7", nullptr, 0 },
	{ ACTION_COMBAT, "COMBAT", "Combat", "MainActor::toggleCombat", nullptr, "c", "JOY_X", 0 },
	{ ACTION_MENU, "MENU", "Game Menu", "MenuGump::showMenu", nullptr, "ESCAPE", "JOY_Y", FLAG_MENU_ENABLED },
	{ ACTION_HIGHLIGHT_ITEMS, "HIGHLIGHT_ITEMS", "Show Highlight Items", "GameMapGump::toggleHighlightItems",
		"GameMapGump::toggleHighlightItems", "TAB", nullptr, 0 },
	{ ACTION_TOGGLE_TOUCHING, "TOUCHING", "Show Touching Items", "GUIApp::toggleShowTouchingItems", nullptr, "h", nullptr, 0 },
	{ ACTION_TURN_LEFT, "TURN_LEFT", "Turn Left", "AvatarMoverProcess::startTurnLeft", "AvatarMoverProcess::stopTurnLeft", "LEFT", nullptr, FLAG_MENU_ENABLED },
	{ ACTION_TURN_RIGHT, "TURN_RIGHT", "Turn Right", "AvatarMoverProcess::startTurnRight", "AvatarMoverProcess::stopTurnRight", "RIGHT", nullptr, FLAG_MENU_ENABLED },
	{ ACTION_MOVE_FORWARD, "MOVE_FORWARD", "Move Forward", "AvatarMoverProcess::startMoveForward", "AvatarMoverProcess::stopMoveForward", "UP", nullptr, FLAG_MENU_ENABLED },
	{ ACTION_MOVE_BACK, "MOVE_BACK", "Move Back", "AvatarMoverProcess::startMoveBack", "AvatarMoverProcess::stopMoveBack", "DOWN", nullptr, FLAG_MENU_ENABLED },
	{ ACTION_MOVE_UP, "MOVE_UP", "Move Up", "AvatarMoverProcess::startMoveUp", "AvatarMoverProcess::stopMoveUp", nullptr, "JOY_UP", FLAG_MENU_ENABLED },
	{ ACTION_MOVE_DOWN, "MOVE_DOWN", "Move Down", "AvatarMoverProcess::startMoveDown", "AvatarMoverProcess::stopMoveDown", nullptr, "JOY_DOWN", FLAG_MENU_ENABLED },
	{ ACTION_MOVE_LEFT, "MOVE_LEFT", "Move Left", "AvatarMoverProcess::startMoveLeft", "AvatarMoverProcess::stopMoveLeft", nullptr, "JOY_LEFT", FLAG_MENU_ENABLED },
	{ ACTION_MOVE_RIGHT, "MOVE_RIGHT", "Move Right", "AvatarMoverProcess::startMoveRight", "AvatarMoverProcess::stopMoveRight", nullptr, "JOY_RIGHT", FLAG_MENU_ENABLED },
	{ ACTION_MOVE_RUN, "MOVE_RUN", "Run", "AvatarMoverProcess::startMoveRun", "AvatarMoverProcess::stopMoveRun", "LSHIFT", "JOY_RIGHT_TRIGGER", FLAG_MENU_ENABLED },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

static const KeybindingRecord U8_KEYS[] = {
	{ ACTION_BEDROLL, "BEDROLL", "Bedroll", "MainActor::useBedroll", nullptr, "b", nullptr, 0 },
	{ ACTION_BACKPACK, "BACKPACK", "Use Backpack", "MainActor::useBackpack", nullptr, "i", nullptr, 0 },
	{ ACTION_KEYRING, "KEYRING", "Keyring", "MainActor::useKeyring", nullptr, "k", nullptr, 0 },
	{ ACTION_MINIMAP, "MINIMAP", "Toggle Minimap", "MiniMapGump::toggle", nullptr, "m", "JOY_LEFT_TRIGGER", 0 },
	{ ACTION_RECALL, "RECALL", "Use Recall", "MainActor::useRecall", nullptr, "r", nullptr, 0 },
	{ ACTION_INVENTORY, "INVENTORY", "Inventory", "MainActor::useInventory", nullptr, "z", "JOY_LEFT_SHOULDER", 0 },
	{ ACTION_CLOSE_GUMPS, "CLOSE_GUMPS", "Close Gumps", "GUIApp::closeItemGumps", nullptr, "BACKSPACE", nullptr, 0 },
	{ ACTION_JUMP, "JUMP", "Jump (fake both-button-click)", "AvatarMoverProcess::startJump", "AvatarMoverProcess::stopJump", "SPACE", nullptr, FLAG_MENU_ENABLED },
	{ ACTION_MOVE_STEP, "MOVE_STEP", "Step", "AvatarMoverProcess::startMoveStep", "AvatarMoverProcess::stopMoveStep", "LCTRL", "JOY_RIGHT_SHOULDER", FLAG_MENU_ENABLED },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

static const KeybindingRecord CRUSADER_KEYS[] = {
	{ ACTION_NEXT_WEAPON, "NEXT_WEAPON", "Next Weapon", "MainActor::nextWeapon", nullptr, "w", nullptr, 0 },
	{ ACTION_NEXT_INVENTORY, "NEXT_INVENTORY", "Next Inventory Item", "MainActor::nextInvItem", nullptr, "i", nullptr, 0 },
	{ ACTION_USE_INVENTORY, "USE_INVENTORY", "Use Inventroy Item", "MainActor::useInventoryItem", nullptr, "u", nullptr, 0 },
	{ ACTION_USE_MEDIKIT, "USE_MEDIKIT", "Use Medical Kit", "MainActor::useMedikit", nullptr, "M", nullptr, 0 },
	{ ACTION_DETONATE_BOMB, "DETONATE_BOMB", "Detonate Bomb", "MainActor::detonateBomb", nullptr, "b", nullptr, 0 },
	{ ACTION_SELECT_ITEMS, "SELECT_ITEM", "Select Item", "ItemSelectionProcess::startSelection", nullptr, "s", nullptr, 0 },
	{ ACTION_USE_SELECTION, "USE_SELECTION", "Use Selection", "ItemSelectionProcess::useSelectedItem", nullptr, "RETURN", nullptr, 0 },
	{ ACTION_ATTACK, "ATTACK", "Attack", "AvatarMoverProcess::tryAttack", nullptr, "SPACE", nullptr, 0 },
	{ ACTION_CAMERA_AVATAR, "CAMERA_AVATAR", "Focus Camera on Silencer", "CameraProcess::moveToAvatar", nullptr, "z", nullptr, 0 },
	{ ACTION_JUMP, "JUMP", "Jump / Roll / Crouch", "AvatarMoverProcess::startJump", "AvatarMoverProcess::stopJump", "LCTRL", nullptr, FLAG_MENU_ENABLED },
	{ ACTION_MOVE_STEP, "MOVE_STEP", "Side Step / Advance / Retreat", "AvatarMoverProcess::startMoveStep", "AvatarMoverProcess::stopMoveStep", "LALT", "JOY_RIGHT_SHOULDER", FLAG_MENU_ENABLED },
	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

static const KeybindingRecord CHEAT_KEYS[] = {
	{ ACTION_CHEAT_MODE, "CHEAT_MODE", "Toggle Cheat Mode", "Cheat::toggle", nullptr, "BACKQUOTE", nullptr, 0 },
	{ ACTION_CLIPPING, "CLIPPING", "Toggle Clipping", "QuickAvatarMoverProcess::toggleClipping", nullptr, "INSERT", nullptr, 0 },
	{ ACTION_DEC_SORT_ORDER, "DEC_SORT_ORDER", "Decrement Map Sort Order", "GameMapGump::decrementSortOrder", nullptr, "LEFTBRACKET", nullptr, 0 },
	{ ACTION_INC_SORT_ORDER, "INC_SORT_ORDER", "Increment Map Sort Order", "GameMapGump::incrementSortOrder", nullptr, "RIGHTBRACKET", nullptr, 0 },
	{ ACTION_QUICK_MOVE_ASCEND, "ASCEND", "Ascend", "QuickAvatarMoverProcess::startAscend", "QuickAvatarMoverProcess::stopAscend", "HOME", nullptr, 0 },
	{ ACTION_QUICK_MOVE_DESCEND, "DESCEND", "Descend", "QuickAvatarMoverProcess::startDescend", "QuickAvatarMoverProcess::stopDescend", "END", nullptr, FLAG_MENU_ENABLED },
	{ ACTION_QUICK_MOVE_UP, "MOVE_UP", "Move Up", "QuickAvatarMoverProcess::startMoveUp", "QuickAvatarMoverProcess::stopMoveUp", "A+UP", nullptr, FLAG_MENU_ENABLED },
	{ ACTION_QUICK_MOVE_DOWN, "MOVE_DOWN", "Move Down", "QuickAvatarMoverProcess::startMoveDown", "QuickAvatarMoverProcess::stopMoveDown", "A+DOWN", nullptr, FLAG_MENU_ENABLED },
	{ ACTION_QUICK_MOVE_LEFT, "MOVE_LEFT", "Move Left", "QuickAvatarMoverProcess::startMoveLeft", "QuickAvatarMoverProcess::stopMoveLeft", "A+LEFT", nullptr, FLAG_MENU_ENABLED },
	{ ACTION_QUICK_MOVE_RIGHT, "MOVE_RIGHT", "Move Right", "QuickAvatarMoverProcess::startMoveRight", "QuickAvatarMoverProcess::stopMoveRight", "A+RIGHT", nullptr, FLAG_MENU_ENABLED },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

#ifndef RELEASE_BUILD
static const KeybindingRecord DEBUG_KEYS[] = {
	{ ACTION_TOGGLE_PAINT, "TOGGLE_PAINT", "Toggle Paint Editor Items", "GUIApp::togglePaintEditorItems", nullptr, "e", nullptr, 0 },
	{ ACTION_ENGINE_STATS, "STATS", "List engine stats", "GUIApp::engineStats", nullptr, "t", nullptr, 0 },
	{ ACTION_FRAME_BY_FRAME, "FRAME_BY_FRAME", "Toggle Frame By Frame", "Kernel::toggleFrameByFrame", nullptr, "F12", nullptr, 0 },
	{ ACTION_ADVANCE_FRAME, "ADVANCE_FRAME", "Advance Frame", "Kernel::advanceFrame", nullptr, "f", nullptr, 0 },
	{ ACTION_TOGGLE_STASIS, "TOGGLE_STASIS", "Toggle Avatar In Stasis", "GUIApp::toggleAvatarInStasis", nullptr, "F10", nullptr, 0 },
	{ ACTION_SHAPE_VIEWER, "SHAPE_VIEWER", "Show Shape Viewer", "ShapeViewerGump::U8ShapeViewer", nullptr, "F11", nullptr, 0 },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};
#endif


Common::KeymapArray MetaEngine::initKeymaps(const Common::String &gameId, bool isMenuActive) {
	Common::KeymapArray keymapArray;

	// Core keymaps
	Common::U32String desc = (gameId == "ultima8" ? _("Ultima VIII") : _("Crusader"));

	Common::Keymap *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, gameId, desc);
	keymapArray.push_back(keyMap);

	Common::Action *act;

	act = new Common::Action("LCLK", _("Interact via Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keyMap->addAction(act);

	act = new Common::Action("RCLK", _("Interact via Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	keyMap->addAction(act);

	for (const KeybindingRecord *r = COMMON_KEYS; r->_id; ++r) {
		if (!isMenuActive || (r->_flags & FLAG_MENU_ENABLED)) {
			act = new Common::Action(r->_id, _(r->_desc));
			act->setCustomEngineActionEvent(r->_action);
			if (r->_key)
				act->addDefaultInputMapping(r->_key);
			if (r->_joy)
				act->addDefaultInputMapping(r->_joy);
			keyMap->addAction(act);
		}
	}

	// Game specific keymaps
	const KeybindingRecord *game_keys = (gameId.equals("ultima8") ? U8_KEYS : CRUSADER_KEYS);
	for (const KeybindingRecord *r = game_keys; r->_id; ++r) {
		if (!isMenuActive || (r->_flags & FLAG_MENU_ENABLED)) {
			act = new Common::Action(r->_id, _(r->_desc));
			act->setCustomEngineActionEvent(r->_action);
			if (r->_key)
				act->addDefaultInputMapping(r->_key);
			if (r->_joy)
				act->addDefaultInputMapping(r->_joy);
			keyMap->addAction(act);
		}
	}


	// Cheat keymaps
	keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "ultima8_cheats", _("Ultima VIII Cheats"));
	keymapArray.push_back(keyMap);

	for (const KeybindingRecord *r = CHEAT_KEYS; r->_id; ++r) {
		if (!isMenuActive || (r->_flags & FLAG_MENU_ENABLED)) {
			act = new Common::Action(r->_id, _(r->_desc));
			act->setCustomEngineActionEvent(r->_action);
			if (r->_key)
				act->addDefaultInputMapping(r->_key);
			if (r->_joy)
				act->addDefaultInputMapping(r->_joy);
			keyMap->addAction(act);
		}
	}

#ifndef RELEASE_BUILD
	// Debug keymaps
	keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "ultima8_debug", _("Ultima VIII Debug"));
	keymapArray.push_back(keyMap);

	for (const KeybindingRecord *r = DEBUG_KEYS; r->_id; ++r) {
		if (!isMenuActive || (r->_flags & FLAG_MENU_ENABLED)) {
			act = new Common::Action(r->_id, _(r->_desc));
			act->setCustomEngineActionEvent(r->_action);
			if (r->_key)
				act->addDefaultInputMapping(r->_key);
			if (r->_joy)
				act->addDefaultInputMapping(r->_joy);
			keyMap->addAction(act);
		}
	}
#endif

	return keymapArray;
}

void MetaEngine::setGameMenuActive(bool isActive) {
	Common::Keymapper *const mapper = g_engine->getEventManager()->getKeymapper();
	mapper->cleanupGameKeymaps();

	const Common::String gameId = CoreApp::get_instance()->getGameInfo()->_name;

	Common::KeymapArray arr = initKeymaps(gameId, isActive);

	for (uint idx = 0; idx < arr.size(); ++idx)
		mapper->addGameKeymap(arr[idx]);
}


void MetaEngine::pressAction(KeybindingAction keyAction) {
	Common::String methodName = getMethod(keyAction, true);
	if (!methodName.empty())
		g_debugger->executeCommand(methodName);
}

void MetaEngine::releaseAction(KeybindingAction keyAction) {
	Common::String methodName = getMethod(keyAction, false);
	if (!methodName.empty())
		g_debugger->executeCommand(methodName);
}

Common::String MetaEngine::getMethod(KeybindingAction keyAction, bool isPress) {
#ifdef RELEASE_BUILD
	const KeybindingRecord *KEY_ARRAYS[] = { COMMON_KEYS, U8_KEYS, CRUSADER_KEYS, CHEAT_KEYS, nullptr };
#else
	const KeybindingRecord *KEY_ARRAYS[] = { COMMON_KEYS, U8_KEYS, CRUSADER_KEYS, CHEAT_KEYS, DEBUG_KEYS, nullptr };
#endif

	for (const KeybindingRecord **arr = KEY_ARRAYS; *arr; ++arr) {
		for (const KeybindingRecord *r = *arr; r->_id; ++r) {
			if (r->_action == keyAction) {
				return Common::String(isPress ? r->_pressMethod : r->_releaseMethod);
			}
		}
	}

	return Common::String();
}

} // End of namespace Ultima8
} // End of namespace Ultima
