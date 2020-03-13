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
};

static const KeybindingRecord KEYS[] = {
	{ ACTION_QUICKSAVE, "QUICKSAVE", "Quick Save", "GUIApp::saveGame QuickSave", nullptr, "F1", nullptr },
	{ ACTION_SAVE, "SAVE", "Save Game", "GUIApp::saveGame", nullptr, "F5", nullptr },
	{ ACTION_LOAD, "LOAD", "Load Game", "GUIApp::loadGame", nullptr, "F7", nullptr },
	{ ACTION_BEDROLL, "BEDROLL", "Bedroll", "MainActor::useBedroll", nullptr, "b", nullptr },
	{ ACTION_COMBAT, "COMBAT", "Combat", "MainActor::toggleCombat", nullptr, "c", "JOY_X" },
	{ ACTION_BACKPACK, "BACKPACK", "Use Backpack", "MainActor::useBackpack", nullptr, "i", nullptr },
	{ ACTION_KEYRING, "KEYRING", "Keyring", "MainActor::useKeyring", nullptr, "k", nullptr },
	{ ACTION_MINIMAP, "MINIMAP", "Toggle Minimap", "MiniMapGump::toggle", nullptr, "m", "JOY_LEFT_TRIGGER" },
	{ ACTION_RECALL, "RECALL", "Use Recall", "MainActor::useRecall", nullptr, "r", nullptr },
	{ ACTION_INVENTORY, "INVENTORY", "Inventory", "MainActor::useInventory", nullptr, "z", "JOY_LEFT_SHOULDER" },
	{ ACTION_MENU, "MENU", "Game Menu", "MenuGump::showMenu", nullptr, "ESCAPE", "JOY_Y" },
	{ ACTION_CLOSE_GUMPS, "CLOSE_GUMPS", "Close Gumps", "GUIApp::closeItemGumps", nullptr, "BACKSPACE", "JOY_RIGHT_TRIGGER" },
	{ ACTION_HIGHLIGHT_ITEMS, "HIGHLIGHT_ITEMS", "Show Highlight Items", "GameMapGump::toggleHighlightItems",
		"GameMapGump::toggleHighlightItems", "TAB", nullptr },
	{ ACTION_TOGGLE_TOUCHING, "TOUCHING", "Show Touching Items", "GUIApp::toggleShowTouchingItems", nullptr, "h", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CHEAT_KEYS[] = {
	{ ACTION_CHEAT_MODE, "CHEAT_MODE", "Toggle Cheat Mode", "Cheat::toggle", nullptr, "BACKQUOTE", nullptr },
	{ ACTION_CLIPPING, "CLIPPING", "Toggle Clipping", "QuickAvatarMoverProcess::toggleClipping", nullptr, "INSERT", nullptr },
	{ ACTION_DEC_SORT_ORDER, "DEC_SORT_ORDER", "Decrement Map Sort Order", "GameMapGump::decrementSortOrder", nullptr, "LEFTBRACKET", nullptr },
	{ ACTION_INC_SORT_ORDER, "INC_SORT_ORDER", "Increment Map Sort Order", "GameMapGump::incrementSortOrder", nullptr, "RIGHTBRACKET", nullptr },
	{ ACTION_ASCEND, "ASCEND", "Ascend", "QuickAvatarMoverProcess::startAscend", "QuickAvatarMoverProcess::stopAscend", "HOME", nullptr },
	{ ACTION_DESCEND, "DESCEND", "Descend", "QuickAvatarMoverProcess::startDescend", "QuickAvatarMoverProcess::stopDescend", "END", nullptr },
	{ ACTION_MOVE_UP, "MOVE_UP", "Move Up", "QuickAvatarMoverProcess::startMoveUp", "QuickAvatarMoverProcess::stopMoveUp", "UP", nullptr },
	{ ACTION_MOVE_DOWN, "MOVE_DOWN", "Move Down", "QuickAvatarMoverProcess::startMoveDown", "QuickAvatarMoverProcess::stopMoveDown", "DOWN", nullptr },
	{ ACTION_MOVE_LEFT, "MOVE_LEFT", "Move Left", "QuickAvatarMoverProcess::startMoveLeft", "QuickAvatarMoverProcess::stopMoveLeft", "LEFT", nullptr },
	{ ACTION_RIGHT, "MOVE_RIGHT", "Move Right", "QuickAvatarMoverProcess::startMoveRight", "QuickAvatarMoverProcess::stopMoveRight", "RIGHT", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

#ifndef RELEASE_BUILD
static const KeybindingRecord DEBUG_KEYS[] = {
	{ ACTION_TOGGLE_PAINT, "TOGGLE_PAINT", "Toggle Paint Editor Items", "GUIApp::togglePaintEditorItems", nullptr, "e", nullptr },
	{ ACTION_ENGINE_STATS, "STATS", "List engine stats", "GUIApp::engineStats", nullptr, "t", nullptr },
	{ ACTION_FRAME_BY_FRAME, "FRAME_BY_FRAME", "Toggle Frame By Frame", "Kernel::toggleFrameByFrame", nullptr, "F12", nullptr },
	{ ACTION_ADVANCE_FRAME, "ADVANCE_FRAME", "Advance Frame", "Kernel::advanceFrame", nullptr, "SPACE", nullptr },
	{ ACTION_TOGGLE_STASIS, "TOGGLE_STASIS", "Toggle Avatar In Stasis", "GUIApp::toggleAvatarInStasis", nullptr, "F10", nullptr },
	{ ACTION_SHAPE_VIEWER, "SHAPE_VIEWER", "Show Shape Viewer", "ShapeViewerGump::U8ShapeViewer", nullptr, "F11", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};
#endif


Common::KeymapArray MetaEngine::initKeymaps(bool isMenuActive) {
	Common::KeymapArray keymapArray;

	// Core keymaps
	Common::Keymap *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "ultima8", _("Ultima VIII"));
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

	for (const KeybindingRecord *r = KEYS; r->_id; ++r) {
		if (!isMenuActive || !strcmp(r->_id, "MENU")) {
			act = new Common::Action(r->_id, _(r->_desc));
			act->setCustomEngineActionEvent(r->_action);
			act->addDefaultInputMapping(r->_key);
			if (r->_joy)
				act->addDefaultInputMapping(r->_joy);
			keyMap->addAction(act);
		}
	}

	if (!isMenuActive) {
		// Cheat keymaps
		keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "ultima8_cheats", _("Ultima VIII Cheats"));
		keymapArray.push_back(keyMap);

		for (const KeybindingRecord *r = CHEAT_KEYS; r->_id; ++r) {
			act = new Common::Action(r->_id, _(r->_desc));
			act->setCustomEngineActionEvent(r->_action);
			act->addDefaultInputMapping(r->_key);
			if (r->_joy)
				act->addDefaultInputMapping(r->_joy);
			keyMap->addAction(act);
		}

#ifndef RELEASE_BUILD
		// Debug keymaps
		keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "ultima8_debug", _("Ultima VIII Debug"));
		keymapArray.push_back(keyMap);

		for (const KeybindingRecord *r = DEBUG_KEYS; r->_id; ++r) {
			act = new Common::Action(r->_id, _(r->_desc));
			act->setCustomEngineActionEvent(r->_action);
			act->addDefaultInputMapping(r->_key);
			if (r->_joy)
				act->addDefaultInputMapping(r->_joy);
			keyMap->addAction(act);
		}
#endif
	}

	return keymapArray;
}

void MetaEngine::setGameMenuActive(bool isActive) {
	Common::Keymapper *const mapper = g_engine->getEventManager()->getKeymapper();
	mapper->cleanupGameKeymaps();

	Common::KeymapArray arr = initKeymaps(isActive);

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
	const KeybindingRecord *KEY_ARRAYS[] = { KEYS, CHEAT_KEYS, nullptr };
#else
	const KeybindingRecord *KEY_ARRAYS[] = { KEYS, CHEAT_KEYS, DEBUG_KEYS, nullptr };
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
