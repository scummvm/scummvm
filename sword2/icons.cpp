/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "sword2/sword2.h"
#include "sword2/driver/driver96.h"
#include "sword2/defs.h"
#include "sword2/icons.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/mouse.h"

namespace Sword2 {

// tempory list
menu_object temp_list[TOTAL_engine_pockets];
uint32 total_temp = 0;

menu_object master_menu_list[TOTAL_engine_pockets];
uint32 total_masters=0;

int32 Logic::fnAddMenuObject(int32 *params) {
	// params:	0 pointer to a menu_object structure to copy down

#ifdef _SWORD2_DEBUG
	if (total_temp == TOTAL_engine_pockets)
		error("TOTAL_engine_pockets exceeded!");
#endif

	// copy the structure to our in-the-engine list
	memcpy(&temp_list[total_temp], (uint8 *) params[0], sizeof(menu_object));
	total_temp++;

	// script continue
	return IR_CONT;
}

int32 Logic::fnRefreshInventory(int32 *params) {
	// called from 'menu_look_or_combine' script in 'menu_master' object
	// to update the menu to display a combined object while George runs
	// voice-over. Note that 'object_held' must be set to the graphic of
	// the combined object

	// params:	none

	// can reset this now
	COMBINE_BASE = 0;

	// so that the icon in 'object_held' is coloured while the rest are
	// grey
	examining_menu_icon = 1;
	Build_menu();
 	examining_menu_icon = 0;

	// script continue
	return IR_CONT;
}

void Build_menu(void) {
	// create and start the inventory menu - NOW AT THE BOTTOM OF THE
	// SCREEN!

	uint32 null_pc = 0;
	uint32 j, k;
	uint8 icon_coloured;
	uint8 *icon;
	uint8 *head;
	uint32 res;

	// reset temp list which will be totally rebuilt
	total_temp = 0;

	debug(5, "build top menu %d", total_masters);

	// clear the temp list before building a new temp list in-case list
	// gets smaller. check each master

	for (j = 0; j < TOTAL_engine_pockets; j++)
		temp_list[j].icon_resource = 0;

	// Call menu builder script which will register all carried menu
	// objects. Run the 'build_menu' script in the 'menu_master' object

	head = res_man.open(MENU_MASTER_OBJECT);
	g_logic.runScript((char*) head, (char*) head, &null_pc);
	res_man.close(MENU_MASTER_OBJECT);

	// Compare new with old. Anything in master thats not in new gets
	// removed from master - if found in new too, remove from temp

	if (total_masters) {
		// check each master

		for (j = 0; j < total_masters; j++) {
			for (k = 0; k < TOTAL_engine_pockets; k++) {
				res = 0;
				// if master is in temp
				if (master_menu_list[j].icon_resource == temp_list[k].icon_resource) {
					// kill it in the temp
					temp_list[k].icon_resource = 0;
					res = 1;
					break;
				}
			}
			if (!res) {
				// otherwise not in temp so kill in main
				master_menu_list[j].icon_resource = 0;
				debug(5, "Killed menu %d", j);
			}
		}
	}

	// merge master downwards

	total_masters = 0;

	//check each master slot

	for (j = 0; j < TOTAL_engine_pockets; j++) {
		// not current end - meaning out over the end so move down
		if (master_menu_list[j].icon_resource && j != total_masters) {
			memcpy(&master_menu_list[total_masters++], &master_menu_list[j], sizeof(menu_object));

			// moved down now so kill here
			master_menu_list[j].icon_resource = 0;
		} else if (master_menu_list[j].icon_resource) {
			// skip full slots
			total_masters++;
		}
	}

	// add those new to menu still in temp but not yet in master to the
	// end of the master

	// check each master slot

	for (j = 0; j < TOTAL_engine_pockets; j++) {
		if (temp_list[j].icon_resource) {
			// here's a new temp
			memcpy(&master_menu_list[total_masters++], &temp_list[j], sizeof(menu_object));
		}
	}

	// init top menu from master list

	for (j = 0; j < 15; j++) {
		if (master_menu_list[j].icon_resource) {
			// 'res' is now the resource id of the icon
			res = master_menu_list[j].icon_resource;

			if (examining_menu_icon) {
				// WHEN AN ICON HAS BEEN RIGHT-CLICKED FOR
				// 'EXAMINE' - SELECTION COLOURED, THE REST
				// GREYED OUT

				// If this is the icon being examined, make
				// it coloured. If not, grey this one out.

				if (res == OBJECT_HELD)
					icon_coloured = 1;
				else
					icon_coloured = 0;
			} else if (COMBINE_BASE) {
				// WHEN ONE MENU OBJECT IS BEING USED WITH
				// ANOTHER - BOTH TO BE COLOURED, THE REST
				// GREYED OUT

				// if this if either of the icons being
				// combined...

				if (res == OBJECT_HELD || res == COMBINE_BASE)
					icon_coloured = 1;
				else
					icon_coloured = 0;
			} else {
				// NORMAL ICON SELECTION - SELECTION GREYED
				// OUT, THE REST COLOURED

				// If this is the selction, grey it out. If
				// not, make it coloured.

  				if (res == OBJECT_HELD)
					icon_coloured = 0;
				else
					icon_coloured = 1;
			}

			icon = res_man.open(master_menu_list[j].icon_resource) + sizeof(_standardHeader);

			// The coloured icon is stored directly after the
			// greyed out one.

			if (icon_coloured)
				icon += (RDMENU_ICONWIDE * RDMENU_ICONDEEP);

			g_display->setMenuIcon(RDMENU_BOTTOM, j, icon);
			res_man.close(res);
		} else {
			// no icon here
			g_display->setMenuIcon(RDMENU_BOTTOM, j, NULL);
			debug(5, " NULL for %d", j);
		}
	}

	g_display->showMenu(RDMENU_BOTTOM);
}

void Build_system_menu(void) {
	// start a fresh top system menu

	uint8 *icon;

	uint32 icon_list[5] = {
		OPTIONS_ICON,
		QUIT_ICON,
		SAVE_ICON,
		RESTORE_ICON,
		RESTART_ICON
	};

	// build them all high in full colour - when one is clicked on all the
	// rest will grey out

	for (int j = 0; j < ARRAYSIZE(icon_list); j++) {
		icon = res_man.open(icon_list[j]) + sizeof(_standardHeader);
		
		// The only case when an icon is grayed is when the player
		// is dead. Then SAVE is not available.

		if (!DEAD || icon_list[j] != SAVE_ICON)
			icon += (RDMENU_ICONWIDE * RDMENU_ICONDEEP);

		g_display->setMenuIcon(RDMENU_TOP, j, icon);
		res_man.close(icon_list[j]);
	}

	g_display->showMenu(RDMENU_TOP);
}

} // End of namespace Sword2
