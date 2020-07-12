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

#ifndef CINE_MAINLOOP_H
#define CINE_MAINLOOP_H

namespace Cine {

enum CallSource {
	EXECUTE_PLAYER_INPUT, ///< Called from executePlayerInput function.
	MAIN_LOOP_WAIT_FOR_PLAYER_CLICK, ///< Called from mainLoop function's waiting for player click section.
	MAKE_MENU_CHOICE, ///< Called from makeMenuChoice function.
	MAKE_SYSTEM_MENU, ///< Called from makeSystemMenu function.
	MAKE_TEXT_ENTRY_MENU, ///< Called from makeTextEntryMenu function.
	PROCESS_INVENTORY, ///< Called from processInventory function.
	WAIT_PLAYER_INPUT ///< Called from waitPlayerInput function.
};

enum EventTarget {
	UNTIL_MOUSE_BUTTON_UP_DOWN_UP, ///< Wait until first mouse buttons all up, then at least one down, finally all up.
	UNTIL_MOUSE_BUTTON_DOWN_UP, ///< Wait until first at least one mouse button down, finally all up.
	UNTIL_MOUSE_BUTTON_UP, ///< Wait until all mouse buttons up.
	UNTIL_MOUSE_BUTTON_UP_AND_WAIT_ENDED, ///< Wait until all mouse buttons up and wait period (getTimerDelay()) ended.
	UNTIL_MOUSE_BUTTON_UP_DOWN, ///< Wait until first all mouse buttons up, finally at least one down.
	UNTIL_MOUSE_BUTTON_DOWN, ///< Wait until at least one mouse button down.
	UNTIL_MOUSE_BUTTON_DOWN_OR_KEY_UP_OR_DOWN_OR_IN_RECTS, ///< Wait until at least one mouse button down, up key pressed, down key pressed or mouse position in at least one of the given rectangles.
	UNTIL_MOUSE_BUTTON_DOWN_OR_KEY_INPUT, ///< Wait until at least one mouse button down or a key pressed.
	UNTIL_WAIT_ENDED, ///< Wait until wait period (getTimerDelay()) ended.
	EMPTY_EVENT_QUEUE ///< Empty the event queue.
};

void mainLoop(int bootScriptIdx);
void manageEvents(CallSource callSource, EventTarget eventTarget, bool useMaxMouseButtonState = false, Common::Array<Common::Rect> rects = Common::Array<Common::Rect>());

} // End of namespace Cine

#endif
