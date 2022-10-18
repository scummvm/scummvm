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

#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_inventory_item.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/inventory_item.h"
#include "ags/engine/ac/inv_window.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_inv.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game_state.h"

namespace AGS3 {

using namespace AGS::Shared;

void set_inv_item_pic(int invi, int piccy) {
	if ((invi < 1) || (invi > _GP(game).numinvitems))
		quit("!SetInvItemPic: invalid inventory item specified");

	if (_GP(game).invinfo[invi].pic == piccy)
		return;

	if (_GP(game).invinfo[invi].pic == _GP(game).invinfo[invi].cursorPic) {
		// Backwards compatibility -- there didn't used to be a cursorPic,
		// so if they're the same update both.
		set_inv_item_cursorpic(invi, piccy);
	}

	_GP(game).invinfo[invi].pic = piccy;
	GUI::MarkInventoryForUpdate(-1, false);
}

void SetInvItemName(int invi, const char *newName) {
	if ((invi < 1) || (invi > _GP(game).numinvitems))
		quit("!SetInvName: invalid inventory item specified");

	// set the new name, making sure it doesn't overflow the buffer
	strncpy(_GP(game).invinfo[invi].name, newName, 25);
	_GP(game).invinfo[invi].name[24] = 0;

	// might need to redraw the GUI if it has the inv item name on it
	GUI::MarkSpecialLabelsForUpdate(kLabelMacro_Overhotspot);
}

int GetInvAt(int atx, int aty) {
	int ongui = GetGUIAt(atx, aty);
	if (ongui >= 0) {
		data_to_game_coords(&atx, &aty);
		int32_t onobj = _GP(guis)[ongui].FindControlAt(atx, aty);
		GUIObject *guio = _GP(guis)[ongui].GetControl(onobj);
		if (guio) {
			_G(mouse_ifacebut_xoffs) = atx - _GP(guis)[ongui].X - guio->X;
			_G(mouse_ifacebut_yoffs) = aty - _GP(guis)[ongui].Y - guio->Y;
		}
		if (guio && (_GP(guis)[ongui].GetControlType(onobj) == kGUIInvWindow))
			return offset_over_inv((GUIInvWindow *)guio);
	}
	return -1;
}

void GetInvName(int indx, char *buff) {
	VALIDATE_STRING(buff);
	if ((indx < 0) | (indx >= _GP(game).numinvitems)) quit("!GetInvName: invalid inventory item specified");
	snprintf(buff, MAX_MAXSTRLEN, "%s", get_translation(_GP(game).invinfo[indx].name));
}

int GetInvGraphic(int indx) {
	if ((indx < 0) | (indx >= _GP(game).numinvitems)) quit("!GetInvGraphic: invalid inventory item specified");

	return _GP(game).invinfo[indx].pic;
}

void RunInventoryInteraction(int iit, int modd) {
	if ((iit < 0) || (iit >= _GP(game).numinvitems))
		quit("!RunInventoryInteraction: invalid inventory number");

	_G(evblocknum) = iit;
	if (modd == MODE_LOOK)
		run_event_block_inv(iit, 0);
	else if (modd == MODE_HAND)
		run_event_block_inv(iit, 1);
	else if (modd == MODE_USE) {
		_GP(play).usedinv = _G(playerchar)->activeinv;
		run_event_block_inv(iit, 3);
	} else if (modd == MODE_TALK)
		run_event_block_inv(iit, 2);
	else // other click on invnetory
		run_event_block_inv(iit, 4);
}

int IsInventoryInteractionAvailable(int item, int mood) {
	if ((item < 0) || (item >= MAX_INV))
		quit("!IsInventoryInteractionAvailable: invalid inventory number");

	_GP(play).check_interaction_only = 1;

	RunInventoryInteraction(item, mood);

	int ciwas = _GP(play).check_interaction_only;
	_GP(play).check_interaction_only = 0;

	if (ciwas == 2)
		return 1;

	return 0;
}

int GetInvProperty(int item, const char *property) {
	return get_int_property(_GP(game).invProps[item], _GP(play).invProps[item], property);
}

void GetInvPropertyText(int item, const char *property, char *bufer) {
	get_text_property(_GP(game).invProps[item], _GP(play).invProps[item], property, bufer);
}

} // namespace AGS3
