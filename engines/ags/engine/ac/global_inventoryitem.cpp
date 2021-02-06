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

#include "ags/shared/ac/common.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_inventoryitem.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/inventoryitem.h"
#include "ags/engine/ac/invwindow.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/gui/guimain.h"
#include "ags/shared/gui/guiinv.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

extern GameSetupStruct game;
extern GameState play;
extern int mouse_ifacebut_xoffs, mouse_ifacebut_yoffs;
extern const char *evblockbasename;
extern int evblocknum;
extern CharacterInfo *playerchar;


void set_inv_item_pic(int invi, int piccy) {
	if ((invi < 1) || (invi > game.numinvitems))
		quit("!SetInvItemPic: invalid inventory item specified");

	if (game.invinfo[invi].pic == piccy)
		return;

	if (game.invinfo[invi].pic == game.invinfo[invi].cursorPic) {
		// Backwards compatibility -- there didn't used to be a cursorPic,
		// so if they're the same update both.
		set_inv_item_cursorpic(invi, piccy);
	}

	game.invinfo[invi].pic = piccy;
	guis_need_update = 1;
}

void SetInvItemName(int invi, const char *newName) {
	if ((invi < 1) || (invi > game.numinvitems))
		quit("!SetInvName: invalid inventory item specified");

	// set the new name, making sure it doesn't overflow the buffer
	strncpy(game.invinfo[invi].name, newName, 25);
	game.invinfo[invi].name[24] = 0;

	// might need to redraw the GUI if it has the inv item name on it
	guis_need_update = 1;
}

int GetInvAt(int xxx, int yyy) {
	int ongui = GetGUIAt(xxx, yyy);
	if (ongui >= 0) {
		int mxwas = _G(mousex), mywas = _G(mousey);
		_G(mousex) = data_to_game_coord(xxx) - guis[ongui].X;
		_G(mousey) = data_to_game_coord(yyy) - guis[ongui].Y;
		int onobj = guis[ongui].FindControlUnderMouse();
		GUIObject *guio = guis[ongui].GetControl(onobj);
		if (guio) {
			mouse_ifacebut_xoffs = _G(mousex) - (guio->X);
			mouse_ifacebut_yoffs = _G(mousey) - (guio->Y);
		}
		_G(mousex) = mxwas;
		_G(mousey) = mywas;
		if (guio && (guis[ongui].GetControlType(onobj) == kGUIInvWindow))
			return offset_over_inv((GUIInvWindow *)guio);
	}
	return -1;
}

void GetInvName(int indx, char *buff) {
	VALIDATE_STRING(buff);
	if ((indx < 0) | (indx >= game.numinvitems)) quit("!GetInvName: invalid inventory item specified");
	strcpy(buff, get_translation(game.invinfo[indx].name));
}

int GetInvGraphic(int indx) {
	if ((indx < 0) | (indx >= game.numinvitems)) quit("!GetInvGraphic: invalid inventory item specified");

	return game.invinfo[indx].pic;
}

void RunInventoryInteraction(int iit, int modd) {
	if ((iit < 0) || (iit >= game.numinvitems))
		quit("!RunInventoryInteraction: invalid inventory number");

	evblocknum = iit;
	if (modd == MODE_LOOK)
		run_event_block_inv(iit, 0);
	else if (modd == MODE_HAND)
		run_event_block_inv(iit, 1);
	else if (modd == MODE_USE) {
		play.usedinv = playerchar->activeinv;
		run_event_block_inv(iit, 3);
	} else if (modd == MODE_TALK)
		run_event_block_inv(iit, 2);
	else // other click on invnetory
		run_event_block_inv(iit, 4);
}

int IsInventoryInteractionAvailable(int item, int mood) {
	if ((item < 0) || (item >= MAX_INV))
		quit("!IsInventoryInteractionAvailable: invalid inventory number");

	play.check_interaction_only = 1;

	RunInventoryInteraction(item, mood);

	int ciwas = play.check_interaction_only;
	play.check_interaction_only = 0;

	if (ciwas == 2)
		return 1;

	return 0;
}

int GetInvProperty(int item, const char *property) {
	return get_int_property(game.invProps[item], play.invProps[item], property);
}

void GetInvPropertyText(int item, const char *property, char *bufer) {
	get_text_property(game.invProps[item], play.invProps[item], property, bufer);
}

} // namespace AGS3
