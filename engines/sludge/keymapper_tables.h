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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "common/translation.h"

namespace Sludge {
inline Common::KeymapArray getSludgeKeymaps(const char *target, const Common::String &gameId) {

	using namespace Common;

	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *debugKeyMap = new Keymap(Keymap::kKeymapTypeGame, "debug", _("Debug keymappings"));
	Keymap *menuKeymap = new Keymap(Keymap::kKeymapTypeGame, "menu", _("Menu keymappings"));
	Keymap *minigameKeymap = new Keymap(Keymap::kKeymapTypeGame, "minigame", _("Minigame keymappings"));
	Action *act;

	if (gameId == "outoforder") {
		act = new Action(kStandardActionLeftClick, _("Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Change action"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionPause, _("Pause"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);

		act = new Action("INVENTORY", _("Inventory"));
		act->setKeyEvent(KeyState(KEYCODE_SPACE, ASCII_SPACE));
		act->addDefaultInputMapping("SPACE");
		act->addDefaultInputMapping("JOY_Y");
		gameKeyMap->addAction(act);

		act = new Action("SAVE", _("Quick save"));
		act->setKeyEvent(KeyState(KEYCODE_s, 's'));
		act->addDefaultInputMapping("s");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("SPEEDUP", _("Speed up dialog"));
		act->setKeyEvent(KeyState(KEYCODE_PLUS, '+'));
		act->addDefaultInputMapping("PLUS");
		act->addDefaultInputMapping("JOY_UP");
		gameKeyMap->addAction(act);

		act = new Action("SPEEDDOWN", _("Slow down dialog"));
		act->setKeyEvent(KeyState(KEYCODE_MINUS, '-'));
		act->addDefaultInputMapping("MINUS");
		act->addDefaultInputMapping("JOY_DOWN");
		gameKeyMap->addAction(act);

	} else if (gameId == "tsotc") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Skip dialog"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action("QUIT", _("Quit"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeyMap->addAction(act);

		act = new Action("LOADAUTOSAVE", _("Load auto save"));
		act->setKeyEvent(KeyState(KEYCODE_a, 'a'));
		act->addDefaultInputMapping("a");
		act->addDefaultInputMapping("JOY_X");
		gameKeyMap->addAction(act);

		act = new Action("SAVE", _("Save game"));
		act->setKeyEvent(KeyState(KEYCODE_s, 's'));
		act->addDefaultInputMapping("s");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("LOAD", _("Load save"));
		act->setKeyEvent(KeyState(KEYCODE_l, 'l'));
		act->addDefaultInputMapping("l");
		act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("INVENTORY", _("Inventory"));
		act->setKeyEvent(KeyState(KEYCODE_i, 'i'));
		act->addDefaultInputMapping("i");
		act->addDefaultInputMapping("JOY_Y");
		gameKeyMap->addAction(act);

		act = new Action("PAUSE", _("Pause"));
		act->setKeyEvent(KeyState(KEYCODE_SPACE, ASCII_SPACE));
		act->addDefaultInputMapping("SPACE");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);

	} else if (gameId == "gjgagsas") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Skip dialog"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action("QUIT", _("Quit"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeyMap->addAction(act);

		act = new Action("LOADAUTOSAVE", _("Load auto save"));
		act->setKeyEvent(KeyState(KEYCODE_a, 'a'));
		act->addDefaultInputMapping("a");
		act->addDefaultInputMapping("JOY_X");
		gameKeyMap->addAction(act);

		act = new Action("SAVE", _("Save game"));
		act->setKeyEvent(KeyState(KEYCODE_s, 's'));
		act->addDefaultInputMapping("s");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("LOAD", _("Load save"));
		act->setKeyEvent(KeyState(KEYCODE_l, 'l'));
		act->addDefaultInputMapping("l");
		act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("INVENTORY", _("Inventory"));
		act->setKeyEvent(KeyState(KEYCODE_i, 'i'));
		act->addDefaultInputMapping("i");
		act->addDefaultInputMapping("JOY_Y");
		gameKeyMap->addAction(act);

	} else if (gameId == "robinsrescue") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Inventory"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionPause, _("Pause"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("q");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);

		act = new Action("SKIPDLG", _("Skip dialog"));
		act->setKeyEvent(KeyState(KEYCODE_PERIOD, '.'));
		act->addDefaultInputMapping("PERIOD");
		act->addDefaultInputMapping("JOY_X");
		gameKeyMap->addAction(act);

		act = new Action("SHOWFLOOR", _("DEBUG: Show floor"));
		act->setKeyEvent(KeyState(KEYCODE_f, 'f'));
		act->addDefaultInputMapping("f");
		debugKeyMap->addAction(act);

		act = new Action("SHOWBOXES", _("DEBUG: Show boxes"));
		act->setKeyEvent(KeyState(KEYCODE_b, 'b'));
		act->addDefaultInputMapping("b");
		debugKeyMap->addAction(act);

	} else if (gameId == "atw") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Inventory"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action("QUIT", _("Quit"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("q");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeyMap->addAction(act);
	
	} else if (gameId == "cubert") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Inventory / Examine"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionPause, _("Pause"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);

	} else if (gameId == "frasse") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Inventory"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionPause, _("Pause"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);

		act = new Action("SKIPDLG", _("Skip dialog"));
		act->setKeyEvent(KeyState(KEYCODE_PERIOD, '.'));
		act->addDefaultInputMapping("PERIOD");
		act->addDefaultInputMapping("JOY_X");
		gameKeyMap->addAction(act);

	} else if (gameId == "tgttpoacs") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Examine / Skip dialog"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionPause, _("Pause / Skip cutscene"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("SPEEDUP", _("Speed up dialog"));
		act->setKeyEvent(KeyState(KEYCODE_PLUS, '+'));
		act->addDefaultInputMapping("PLUS");
		gameKeyMap->addAction(act);

		act = new Action("SPEEDDOWN", _("Slow down dialog"));
		act->setKeyEvent(KeyState(KEYCODE_MINUS, '-'));
		act->addDefaultInputMapping("MINUS");
		gameKeyMap->addAction(act);

		act = new Action("SAVE", _("Quick save"));
		act->setKeyEvent(KeyState(KEYCODE_s, 's'));
		act->addDefaultInputMapping("s");
		gameKeyMap->addAction(act);

		act = new Action("LOAD", _("Quick load"));
		act->setKeyEvent(KeyState(KEYCODE_l, 'l'));
		act->addDefaultInputMapping("l");
		gameKeyMap->addAction(act);

		act = new Action("EXAMINE", _("Examine"));
		act->setKeyEvent(KeyState(KEYCODE_e, 'e'));
		act->addDefaultInputMapping("e");
		gameKeyMap->addAction(act);

		act = new Action("PICKUP", _("Pick up"));
		act->setKeyEvent(KeyState(KEYCODE_p, 'p'));
		act->addDefaultInputMapping("p");
		gameKeyMap->addAction(act);

		act = new Action("TALK", _("Talk to"));
		act->setKeyEvent(KeyState(KEYCODE_t, 't'));
		act->addDefaultInputMapping("t");
		gameKeyMap->addAction(act);

		act = new Action("SMELL", _("Smell"));
		act->setKeyEvent(KeyState(KEYCODE_m, 'm'));
		act->addDefaultInputMapping("m");
		gameKeyMap->addAction(act);

		act = new Action("USE", _("Use"));
		act->setKeyEvent(KeyState(KEYCODE_u, 'u'));
		act->addDefaultInputMapping("u");
		gameKeyMap->addAction(act);

		act = new Action("CONSUME", _("Consume"));
		act->setKeyEvent(KeyState(KEYCODE_c, 'c'));
		act->addDefaultInputMapping("c");
		gameKeyMap->addAction(act);

		act = new Action("QUIT", _("Quit"));
		act->setKeyEvent(KeyState(KEYCODE_q, 'q'));
		act->addDefaultInputMapping("q");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("START", _("Start"));
		act->setKeyEvent(KeyState(KEYCODE_RETURN, ASCII_RETURN));
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("JOY_Y");
		minigameKeymap->addAction(act);

		act = new Action("QUIT", _("Quit"));
		act->setKeyEvent(KeyState(KEYCODE_q, 'q'));
		act->addDefaultInputMapping("q");
		act->addDefaultInputMapping("JOY_X");
		minigameKeymap->addAction(act);

		act = new Action("CASTEARTH", _("Cast earth spell"));
		act->setKeyEvent(KeyState(KEYCODE_z, 'z'));
		act->addDefaultInputMapping("z");
		act->addDefaultInputMapping("JOY_LEFT");
		minigameKeymap->addAction(act);

		act = new Action("CASTAIR", _("Cast air spell"));
		act->setKeyEvent(KeyState(KEYCODE_x, 'x'));
		act->addDefaultInputMapping("x");
		act->addDefaultInputMapping("JOY_UP");
		minigameKeymap->addAction(act);

		act = new Action("CASTFIRE", _("Cast fire spell"));
		act->setKeyEvent(KeyState(KEYCODE_c, 'c'));
		act->addDefaultInputMapping("c");
		act->addDefaultInputMapping("JOY_RIGHT");
		minigameKeymap->addAction(act);

		act = new Action("CASTWATER", _("Cast water spell"));
		act->setKeyEvent(KeyState(KEYCODE_v, 'v'));
		act->addDefaultInputMapping("v");
		act->addDefaultInputMapping("JOY_DOWN");
		minigameKeymap->addAction(act);

	} else if (gameId == "interview") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Skip dialog"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionPause, _("Pause / Skip cutscene"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);

		act = new Action("LOAD", _("Quick load"));
		act->setKeyEvent(KeyState(KEYCODE_l, 'l'));
		act->addDefaultInputMapping("l");
		act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("SAVE", _("Quick save"));
		act->setKeyEvent(KeyState(KEYCODE_s, 's'));
		act->addDefaultInputMapping("s");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("QUIT", _("Quit"));
		act->setKeyEvent(KeyState(KEYCODE_q, 'q'));
		act->addDefaultInputMapping("q");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeyMap->addAction(act);

	} else if (gameId == "leptonsquest") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact / Skip dialog"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action("QUIT", _("Quit"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeyMap->addAction(act);

		act = new Action("SHOWFLOOR", _("DEBUG: Show floor"));
		act->setKeyEvent(KeyState(KEYCODE_f, 'f'));
		act->addDefaultInputMapping("f");
		debugKeyMap->addAction(act);

		act = new Action("SHOWBOXES", _("DEBUG: Show boxes"));
		act->setKeyEvent(KeyState(KEYCODE_b, 'b'));
		act->addDefaultInputMapping("b");
		debugKeyMap->addAction(act);

	} else if (gameId == "life") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action("SKIPDLG", _("Skip dialog"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping(".");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionPause, _("Pause"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);

		act = new Action("SAVE", _("Quick save"));
		act->setKeyEvent(KeyState(KEYCODE_s, 's'));
		act->addDefaultInputMapping("s");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("LOAD", _("Quick load"));
		act->setKeyEvent(KeyState(KEYCODE_l, 'l'));
		act->addDefaultInputMapping("l");
		act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
		gameKeyMap->addAction(act);

	} else if (gameId == "mandy") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Skip dialog"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionPause, _("Pause"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);

		act = new Action("SAVE", _("Quick save"));
		act->setKeyEvent(KeyState(KEYCODE_s, 's'));
		act->addDefaultInputMapping("s");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("LOAD", _("Quick load"));
		act->setKeyEvent(KeyState(KEYCODE_l, 'l'));
		act->addDefaultInputMapping("l");
		act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
		gameKeyMap->addAction(act);

	
	} else if (gameId == "nsc") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact / Skip dialog"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Inventory / Skip dialog"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionPause, _("Pause / Skip cutscene"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);

	} else if (gameId == "verbcoin") {
		act = new Action(kStandardActionLeftClick, _("Move / Interact"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Skip dialog"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action("QUIT", _("Quit"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeyMap->addAction(act);

		act = new Action("SHOWBOXES", _("DEBUG: Show boxes"));
		act->setKeyEvent(KeyState(KEYCODE_SPACE, ASCII_SPACE));
		act->addDefaultInputMapping("SPACE");
		debugKeyMap->addAction(act);

		act = new Action("SHOWFLOOR", _("DEBUG: Show floor"));
		act->setKeyEvent(KeyState(KEYCODE_f, 'f'));
		act->addDefaultInputMapping("f");
		debugKeyMap->addAction(act);

	} else {
		// Default keymap for other games
		warning("Game-specific keymap is not defined. Using default SLUDGE engine keymap");

		act = new Action(kStandardActionLeftClick, _("Left click"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionRightClick, _("Right click"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		gameKeyMap->addAction(act);

		act = new Action(kStandardActionPause, _("Pause game"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);
	}

	act = new Action(kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	menuKeymap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	menuKeymap->addAction(act);

	KeymapArray keyMaps(4);
	keyMaps[0] = gameKeyMap;
	keyMaps[1] = debugKeyMap;
	keyMaps[2] = menuKeymap;
	keyMaps[3] = minigameKeymap;

	menuKeymap->setEnabled(false);

	return keyMaps;
}

} // End of namespace Sludge
