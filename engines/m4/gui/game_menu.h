
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

#ifndef M4_GUI_GUI_GAME_MENU_H
#define M4_GUI_GUI_GAME_MENU_H

#include "m4/m4_types.h"
#include "m4/gui/gui_menu_items.h"

namespace M4 {
namespace GUI {

class SaveLoadMenuBase {
public:
	static void init();

	static uint16 SL_THUMBNAIL_W;
	static uint16 SL_THUMBNAIL_H;

	static uint16 SL_DIALOG_BOX;
	static uint16 SL_EMPTY_THUMB;
	static uint16 SL_SAVE_BTN_GREY;
	static uint16 SL_SAVE_BTN_NORM;
	static uint16 SL_SAVE_BTN_OVER;
	static uint16 SL_SAVE_BTN_PRESS;
	static uint16 SL_LOAD_BTN_GREY;
	static uint16 SL_LOAD_BTN_NORM;
	static uint16 SL_LOAD_BTN_OVER;
	static uint16 SL_LOAD_BTN_PRESS;
	static uint16 SL_CANCEL_BTN_NORM;
	static uint16 SL_CANCEL_BTN_OVER;
	static uint16 SL_CANCEL_BTN_PRESS;
	static uint16 SL_UP_BTN_GREY;
	static uint16 SL_UP_BTN_NORM;
	static uint16 SL_UP_BTN_OVER;
	static uint16 SL_UP_BTN_PRESS;
	static uint16 SL_DOWN_BTN_GREY;
	static uint16 SL_DOWN_BTN_NORM;
	static uint16 SL_DOWN_BTN_OVER;
	static uint16 SL_DOWN_BTN_PRESS;
	static uint16 SL_SAVE_LABEL;
	static uint16 SL_LOAD_LABEL;
	static uint16 SL_SLIDER_BTN_NORM;
	static uint16 SL_SLIDER_BTN_OVER;
	static uint16 SL_SLIDER_BTN_PRESS;
	static uint16 SL_LINE_NORM;
	static uint16 SL_LINE_OVER;
	static uint16 SL_LINE_PRESS;
	static uint16 SL_SCROLL_BAR;
	static uint16 SL_TOTAL_SPRITES;

protected:
	static void initializeSlotTables();
	static Sprite *menu_CreateThumbnail(int32 *spriteSize);
	static bool loadThumbnail(int32 slotNum);
	static void unloadThumbnail(int32 slotNum);
	static void setFirstSlot(int32 firstSlot, guiMenu *myMenu);

public:
	static void updateThumbnails(int32 firstSlot, guiMenu *myMenu);
};

} // namespace GUI
} // namespace M4

#endif
