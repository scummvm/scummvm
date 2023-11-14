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

#include "ultima8/detection.h"
#include "ultima8/metaengine.h"
#include "ultima8/misc/debugger.h"
#include "ultima8/ultima8.h"
#include "ultima8/filesys/savegame.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"

const char *Ultima8MetaEngine::getName() const {
	return "ultima8";
}

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FRAME_SKIPPING,
		{
			_s("Enable frame skipping"),
			_s("Allow the game to skip animation frames when running too slow."),
			"frameSkip",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FRAME_LIMITING,
		{
			_s("Enable frame limiting"),
			_s("Limits the speed of the game to prevent running too fast."),
			"frameLimit",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_CHEATS,
		{
			_s("Enable cheats"),
			_s("Allow cheats by commands and a menu when player is clicked."),
			"cheat",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_HIGH_RESOLUTION,
		{
			_s("Enable high resolution"),
			_s("Enable a higher resolution for the game"),
			"usehighres",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FOOTSTEP_SOUNDS,
		{
			_s("Play foot step sounds"),
			_s("Plays sound when the player moves."),
			"footsteps",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_JUMP_TO_MOUSE,
		{
			_s("Enable jump to mouse position"),
			_s("Jumping while not moving targets the mouse cursor rather than direction only."),
			"targetedjump",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_FONT_REPLACEMENT,
		{
			_s("Enable font replacement"),
			_s("Replaces game fonts with rendered fonts"),
			"font_override",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FONT_ANTIALIASING,
		{
			_s("Enable font anti-aliasing"),
			_s("When font anti-aliasing is enabled, the text is smoother."),
			"font_antialiasing",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_CAMERA_WITH_SILENCER,
		{
			// I18N: Silencer is the player-character in Crusader games, known as the Avatar in Ultima series.
			_s("Camera moves with Silencer"),
			_s("Camera tracks the player movement rather than snapping to defined positions."),
			"camera_on_player",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_ALWAYS_CHRISTMAS,
		{
			_s("Always enable Christmas easter-egg"),
			_s("Enable the Christmas music at any time of year."),
			"always_christmas",
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const ADExtraGuiOptionsMap *Ultima8MetaEngine::getAdvancedExtraGuiOptions() const {
	return optionsList;
}
namespace Ultima8 {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_pressMethod;
	const char *_releaseMethod;
	const char *_input1;
	const char *_input2;
};

static const KeybindingRecord COMMON_KEYS[] = {
	{ ACTION_QUICKSAVE, "QUICKSAVE", "Quick Save", "GUIApp::saveGame QuickSave", nullptr, "F1", nullptr },
	{ ACTION_SAVE, "SAVE", "Save Game", "GUIApp::saveGame", nullptr, "F5", nullptr },
	{ ACTION_LOAD, "LOAD", "Load Game", "GUIApp::loadGame", nullptr, "F7", nullptr },
	{ ACTION_COMBAT, "COMBAT", "Combat", "MainActor::toggleCombat", nullptr, "c", "JOY_X" },
	{ ACTION_MENU, "MENU", "Game Menu", "MenuGump::showMenu", nullptr, "ESCAPE", "JOY_Y" },
	{ ACTION_TURN_LEFT, "TURN_LEFT", "Turn Left", "AvatarMoverProcess::startTurnLeft", "AvatarMoverProcess::stopTurnLeft", "LEFT", "KP4" },
	{ ACTION_TURN_RIGHT, "TURN_RIGHT", "Turn Right", "AvatarMoverProcess::startTurnRight", "AvatarMoverProcess::stopTurnRight", "RIGHT", "KP6" },
	{ ACTION_MOVE_FORWARD, "MOVE_FORWARD", "Move Forward", "AvatarMoverProcess::startMoveForward", "AvatarMoverProcess::stopMoveForward", "UP", "KP8" },
	{ ACTION_MOVE_BACK, "MOVE_BACK", "Move Back", "AvatarMoverProcess::startMoveBack", "AvatarMoverProcess::stopMoveBack", "DOWN", "KP2" },
	{ ACTION_MOVE_UP, "MOVE_UP", "Move Up", "AvatarMoverProcess::startMoveUp", "AvatarMoverProcess::stopMoveUp", nullptr, "JOY_UP" },
	{ ACTION_MOVE_DOWN, "MOVE_DOWN", "Move Down", "AvatarMoverProcess::startMoveDown", "AvatarMoverProcess::stopMoveDown", nullptr, "JOY_DOWN" },
	{ ACTION_MOVE_LEFT, "MOVE_LEFT", "Move Left", "AvatarMoverProcess::startMoveLeft", "AvatarMoverProcess::stopMoveLeft", nullptr, "JOY_LEFT" },
	{ ACTION_MOVE_RIGHT, "MOVE_RIGHT", "Move Right", "AvatarMoverProcess::startMoveRight", "AvatarMoverProcess::stopMoveRight", nullptr, "JOY_RIGHT" },
	{ ACTION_MOVE_RUN, "MOVE_RUN", "Run", "AvatarMoverProcess::startMoveRun", "AvatarMoverProcess::stopMoveRun", "LSHIFT", "JOY_RIGHT_TRIGGER" },
	{ ACTION_HIGHLIGHT_ITEMS, "HIGHLIGHT_ITEMS", "Highlight Items", "GameMapGump::startHighlightItems", "GameMapGump::stopHighlightItems", "h", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord U8_KEYS[] = {
	{ ACTION_BEDROLL, "BEDROLL", "Bedroll", "MainActor::useBedroll", nullptr, "b", nullptr },
	{ ACTION_BACKPACK, "BACKPACK", "Use Backpack", "MainActor::useBackpack", nullptr, "i", nullptr },
	{ ACTION_KEYRING, "KEYRING", "Keyring", "MainActor::useKeyring", nullptr, "k", nullptr },
	{ ACTION_MINIMAP, "MINIMAP", "Toggle Minimap", "MiniMapGump::toggle", nullptr, "m", "JOY_LEFT_TRIGGER" },
	{ ACTION_RECALL, "RECALL", "Use Recall", "MainActor::useRecall", nullptr, "r", nullptr },
	{ ACTION_INVENTORY, "INVENTORY", "Inventory", "MainActor::useInventory", nullptr, "z", "JOY_LEFT_SHOULDER" },
	{ ACTION_CLOSE_GUMPS, "CLOSE_GUMPS", "Close Gumps", "GUIApp::closeItemGumps", nullptr, "BACKSPACE", nullptr },
	{ ACTION_JUMP, "JUMP", "Jump (fake both-button-click)", "AvatarMoverProcess::startJump", "AvatarMoverProcess::stopJump", "SPACE", nullptr },
	{ ACTION_MOVE_STEP, "MOVE_STEP", "Step", "AvatarMoverProcess::startMoveStep", "AvatarMoverProcess::stopMoveStep", "LCTRL", "JOY_RIGHT_SHOULDER" },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CRUSADER_KEYS[] = {
	{ ACTION_NEXT_WEAPON, "NEXT_WEAPON", "Next Weapon", "MainActor::nextWeapon", nullptr, "w", "KP_MULTIPLY" },
	{ ACTION_NEXT_INVENTORY, "NEXT_INVENTORY", "Next Inventory Item", "MainActor::nextInvItem", nullptr, "i", "KP_MINUS" },
	{ ACTION_USE_INVENTORY, "USE_INVENTORY", "Use Inventroy Item", "MainActor::useInventoryItem", nullptr, "u", "KP_PERIOD" },
	{ ACTION_USE_MEDIKIT, "USE_MEDIKIT", "Use Medical Kit", "MainActor::useMedikit", nullptr, "m", nullptr },
	{ ACTION_USE_ENERGYCUBE, "USE_ENERGYCUBE", "Use Energy Cube", "MainActor::useEnergyCube", nullptr, "e", nullptr },
	{ ACTION_DETONATE_BOMB, "DETONATE_BOMB", "Detonate Bomb", "MainActor::detonateBomb", nullptr, "b", nullptr },
	// TODO: The same key should be "use datalink" in no regret.
	{ ACTION_DROP_WEAPON, "DROP_WEAPON", "Drop Weapon", "MainActor::dropWeapon", nullptr, "C+d", nullptr },
	{ ACTION_SELECT_ITEMS, "SELECT_ITEM", "Select Item", "ItemSelectionProcess::startSelection", nullptr, "s", "KP_PLUS" },
	{ ACTION_USE_SELECTION, "USE_SELECTION", "Use Selection", "ItemSelectionProcess::useSelectedItem", nullptr, "RETURN", "KP_ENTER" },
	{ ACTION_GRAB_ITEMS, "GRAB_ITEM", "Grab Items", "ItemSelectionProcess::grabItems", nullptr, "g", nullptr },
	{ ACTION_ATTACK, "ATTACK", "Attack", "AvatarMoverProcess::startAttack", "AvatarMoverProcess::stopAttack", "SPACE", "KP0" },
	{ ACTION_CAMERA_AVATAR, "CAMERA_AVATAR", "Focus Camera on Silencer", "CameraProcess::moveToAvatar", nullptr, "z", nullptr },
	{ ACTION_JUMP, "JUMP", "Jump / Roll / Crouch", "AvatarMoverProcess::startJump", "AvatarMoverProcess::stopJump", "LCTRL", nullptr },
	{ ACTION_SHORT_JUMP, "SHORT_JUMP", "Short Jump", "AvatarMoverProcess::shortJump", nullptr, "TAB", "j" },
	{ ACTION_STEP_LEFT, "STEP_LEFT", "Sidestep Left", "AvatarMoverProcess::stepLeft", nullptr, "DELETE", "KP7" },
	{ ACTION_STEP_RIGHT, "STEP_RIGHT", "Sidestep Rgiht", "AvatarMoverProcess::stepRight", nullptr, "PAGEDOWN", "KP9" },
	{ ACTION_STEP_FORWARD, "STEP_FORWARD", "Step Forward", "AvatarMoverProcess::stepForward", nullptr, "HOME", "KP_DIVIDE" },
	{ ACTION_STEP_BACK, "STEP_BACK", "Step Back", "AvatarMoverProcess::stepBack", nullptr, "END", "KP2" },
	{ ACTION_ROLL_LEFT, "ROLL_LEFT", "Roll Left", "AvatarMoverProcess::rollLeft", nullptr, "INSERT", "KP1" },
	{ ACTION_ROLL_RIGHT, "ROLL_RIGHT", "Roll Right", "AvatarMoverProcess::rollRight", nullptr, "PAGEUP", "KP3" },
	{ ACTION_TOGGLE_CROUCH, "TOGGLE_CROUCH", "Toggle Crouch", "AvatarMoverProcess::toggleCrouch", nullptr, "KP5", nullptr },
	{ ACTION_MOVE_STEP, "MOVE_STEP", "Side Step / Advance / Retreat", "AvatarMoverProcess::startMoveStep", "AvatarMoverProcess::stopMoveStep", "LALT", "JOY_RIGHT_SHOULDER" },
	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CHEAT_KEYS[] = {
	{ ACTION_CLIPPING, "CLIPPING", "Toggle Clipping", "QuickAvatarMoverProcess::toggleClipping", nullptr, "M+INSERT", nullptr },
	{ ACTION_QUICK_MOVE_ASCEND, "ASCEND", "Ascend", "QuickAvatarMoverProcess::startAscend", "QuickAvatarMoverProcess::stopAscend", "M+HOME", nullptr },
	{ ACTION_QUICK_MOVE_DESCEND, "DESCEND", "Descend", "QuickAvatarMoverProcess::startDescend", "QuickAvatarMoverProcess::stopDescend", "M+END", nullptr },
	{ ACTION_QUICK_MOVE_UP, "MOVE_UP", "Move Up", "QuickAvatarMoverProcess::startMoveUp", "QuickAvatarMoverProcess::stopMoveUp", "M+UP", nullptr },
	{ ACTION_QUICK_MOVE_DOWN, "MOVE_DOWN", "Move Down", "QuickAvatarMoverProcess::startMoveDown", "QuickAvatarMoverProcess::stopMoveDown", "M+DOWN", nullptr },
	{ ACTION_QUICK_MOVE_LEFT, "MOVE_LEFT", "Move Left", "QuickAvatarMoverProcess::startMoveLeft", "QuickAvatarMoverProcess::stopMoveLeft", "M+LEFT", nullptr },
	{ ACTION_QUICK_MOVE_RIGHT, "MOVE_RIGHT", "Move Right", "QuickAvatarMoverProcess::startMoveRight", "QuickAvatarMoverProcess::stopMoveRight", "M+RIGHT", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

#ifndef RELEASE_BUILD
static const KeybindingRecord DEBUG_KEYS[] = {
	{ ACTION_TOGGLE_TOUCHING, "TOUCHING", "Show Touching Items", "GUIApp::toggleShowTouchingItems", nullptr, "C+h", nullptr },
	{ ACTION_TOGGLE_PAINT, "TOGGLE_PAINT", "Toggle Paint Editor Items", "GUIApp::togglePaintEditorItems", nullptr, "F9", nullptr },
	{ ACTION_DEC_SORT_ORDER, "DEC_SORT_ORDER", "Decrement Map Sort Order", "GameMapGump::decrementSortOrder", nullptr, "LEFTBRACKET", nullptr },
	{ ACTION_INC_SORT_ORDER, "INC_SORT_ORDER", "Increment Map Sort Order", "GameMapGump::incrementSortOrder", nullptr, "RIGHTBRACKET", nullptr },
	{ ACTION_ENGINE_STATS, "STATS", "List engine stats", "GUIApp::engineStats", nullptr, "t", nullptr },
	{ ACTION_FRAME_BY_FRAME, "FRAME_BY_FRAME", "Toggle Frame By Frame", "Kernel::toggleFrameByFrame", nullptr, "F12", nullptr },
	{ ACTION_ADVANCE_FRAME, "ADVANCE_FRAME", "Advance Frame", "Kernel::advanceFrame", nullptr, "f", nullptr },
	{ ACTION_TOGGLE_STASIS, "TOGGLE_STASIS", "Toggle Avatar In Stasis", "GUIApp::toggleAvatarInStasis", nullptr, "F10", nullptr },
	{ ACTION_SHAPE_VIEWER, "SHAPE_VIEWER", "Show Shape Viewer", "ShapeViewerGump::U8ShapeViewer", nullptr, "F11", nullptr },

	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};
#endif

} // End of namespace Ultima8

Common::KeymapArray Ultima8MetaEngine::initKeymaps(const char *target) const {
	const Common::String gameId = getGameId(target);
	Common::KeymapArray keymapArray;

	// Core keymaps
	Common::U32String desc = (gameId == "ultima8" ? _("Ultima VIII") : _("Crusader"));

	Common::Keymap *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, gameId, desc);
	keymapArray.push_back(keyMap);

	Common::Action *act;

	act = new Common::Action(Common::kStandardActionLeftClick, _("Interact via Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionRightClick, _("Interact via Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	keyMap->addAction(act);

	for (const Ultima8::KeybindingRecord *r = Ultima8::COMMON_KEYS; r->_id; ++r) {
		act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		if (r->_input1)
			act->addDefaultInputMapping(r->_input1);
		if (r->_input2)
			act->addDefaultInputMapping(r->_input2);
		keyMap->addAction(act);
	}

	// Game specific keymaps
	const Ultima8::KeybindingRecord *game_keys = (gameId.equals("ultima8") ? Ultima8::U8_KEYS : Ultima8::CRUSADER_KEYS);
	for (const Ultima8::KeybindingRecord *r = game_keys; r->_id; ++r) {
		act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		if (r->_input1)
			act->addDefaultInputMapping(r->_input1);
		if (r->_input2)
			act->addDefaultInputMapping(r->_input2);
		keyMap->addAction(act);
	}


	// Cheat keymaps
	keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "ultima8_cheats", _("Ultima VIII Cheats"));
	keymapArray.push_back(keyMap);

	for (const Ultima8::KeybindingRecord *r = Ultima8::CHEAT_KEYS; r->_id; ++r) {
		act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		if (r->_input1)
			act->addDefaultInputMapping(r->_input1);
		if (r->_input2)
			act->addDefaultInputMapping(r->_input2);
		keyMap->addAction(act);
	}

#ifndef RELEASE_BUILD
	// Debug keymaps
	keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "ultima8_debug", _("Ultima VIII Debug"));
	keymapArray.push_back(keyMap);

	for (const Ultima8::KeybindingRecord *r = Ultima8::DEBUG_KEYS; r->_id; ++r) {
		act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		if (r->_input1)
			act->addDefaultInputMapping(r->_input1);
		if (r->_input2)
			act->addDefaultInputMapping(r->_input2);
		keyMap->addAction(act);
	}
#endif

	return keymapArray;
}

void Ultima8MetaEngine::pressAction(Ultima8::KeybindingAction keyAction) {
	Common::String methodName = getMethod(keyAction, true);
	if (!methodName.empty())
		Ultima8::g_debugger->executeCommand(methodName);
}

void Ultima8MetaEngine::releaseAction(Ultima8::KeybindingAction keyAction) {
	Common::String methodName = getMethod(keyAction, false);
	if (!methodName.empty())
		Ultima8::g_debugger->executeCommand(methodName);
}

Common::String Ultima8MetaEngine::getMethod(Ultima8::KeybindingAction keyAction, bool isPress) {
#ifdef RELEASE_BUILD
	const Ultima8::KeybindingRecord *KEY_ARRAYS[] = { Ultima8::COMMON_KEYS, Ultima8::U8_KEYS, Ultima8::CRUSADER_KEYS, Ultima8::CHEAT_KEYS, nullptr };
#else
	const Ultima8::KeybindingRecord *KEY_ARRAYS[] = { Ultima8::COMMON_KEYS, Ultima8::U8_KEYS, Ultima8::CRUSADER_KEYS, Ultima8::CHEAT_KEYS, Ultima8::DEBUG_KEYS, nullptr };
#endif

	for (const Ultima8::KeybindingRecord **arr = KEY_ARRAYS; *arr; ++arr) {
		for (const Ultima8::KeybindingRecord *r = *arr; r->_id; ++r) {
			if (r->_action == keyAction) {
				return Common::String(isPress ? r->_pressMethod : r->_releaseMethod);
			}
		}
	}

	return Common::String();
}

bool Ultima8MetaEngine::querySaveMetaInfos(const Common::String &filename, SaveStateDescriptor& desc) {
	Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(filename));

	if (f) {
		Ultima8::SavegameReader sg(f.get(), true);
		desc.setDescription(sg.getDescription());
		return sg.isValid();
	}

	return false;
}

Common::Error Ultima8MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Ultima8::Ultima8GameDescription *gd = (const Ultima8::Ultima8GameDescription *)desc;
	switch (gd->gameId) {
	case Ultima8::GAME_ULTIMA8:
	case Ultima8::GAME_CRUSADER_REG:
	case Ultima8::GAME_CRUSADER_REM:
		*engine = new Ultima8::Ultima8Engine(syst, gd);
		break;

	default:
		return Common::kUnsupportedGameidError;
	}
	return Common::kNoError;
}

int Ultima8MetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}
SaveStateDescriptor Ultima8MetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor desc = AdvancedMetaEngine::querySaveMetaInfos(target, slot);
	if (!desc.isValid() && slot > 0) {
		Common::String gameId = getGameId(target);
		if (gameId == "ultima8") {
			Common::String filename = getSavegameFile(slot, target);
			desc = SaveStateDescriptor(this, slot, Common::U32String());
			if (!querySaveMetaInfos(filename, desc))
				return SaveStateDescriptor();
		}
	}

	return desc;
}

Common::String Ultima8MetaEngine::getGameId(const Common::String& target) {
	// Store a copy of the active domain
	Common::String currDomain = ConfMan.getActiveDomainName();

	// Switch to the given target domain and get it's game Id
	ConfMan.setActiveDomain(target);
	Common::String gameId = ConfMan.get("gameid");

	// Switch back to the original domain and return the game Id
	ConfMan.setActiveDomain(currDomain);
	return gameId;
}

#if PLUGIN_ENABLED_DYNAMIC(ULTIMA8)
	REGISTER_PLUGIN_DYNAMIC(ULTIMA8, PLUGIN_TYPE_ENGINE, Ultima8MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ULTIMA8, PLUGIN_TYPE_ENGINE, Ultima8MetaEngine);
#endif
