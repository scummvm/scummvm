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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "stdafx.h"
#include "driver/driver96.h"
#include "console.h"
#include "icons.h"
#include "interpreter.h"
#include "logic.h"
#include "mouse.h"
#include "object.h"

//------------------------------------------------------------------------------------
menu_object temp_list[TOTAL_engine_pockets];
uint32	total_temp=0;	//tempory list

menu_object master_menu_list[TOTAL_engine_pockets];
uint32	total_masters=0;
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
int32 FN_add_menu_object(int32 *params)	//Tony1Oct96
{
//param	0 pointer to a menu_object structure to copy down

//	Zdebug("FN_add_menu_object icon res");

#ifdef _SWORD2_DEBUG
	if (total_temp == TOTAL_engine_pockets)
		Con_fatal_error("TOTAL_engine_pockets exceeded! (%s line %u)",__FILE__,__LINE__);
#endif

//	copy the structure to our in-the-engine list
	memcpy( &temp_list[total_temp], (uint8*) *params, sizeof(menu_object));	//
	total_temp++;

	return(IR_CONT);	// script continue
}
//------------------------------------------------------------------------------------
int32 FN_refresh_inventory(int32 *params)	// (James28aug97)
{
	// called from 'menu_look_or_combine' script in 'menu_master' object
	// to update the menu to display a combined object while George runs voice-over
	// Note that 'object_held' must be set to the graphic of the combined object

	COMBINE_BASE=0;			// can reset this now

	examining_menu_icon=1;	// so that the icon in 'object_held' is coloured while the rest are grey
	Build_top_menu();
 	examining_menu_icon=0;

	return(IR_CONT);	// script continue
}
//------------------------------------------------------------------------------------
void	Build_top_menu(void)	//Tony19Nov96
{
 // create and start the inventory menu - NOW AT THE BOTTOM OF THE SCREEN!

	uint32	null_pc=0;
	uint32	j,k;
	uint8	icon_coloured;
	uint8	*icon;
	uint8	*head;
	uint32	res;

	total_temp=0;	//reset temp list which will be totally rebuilt



//	Zdebug("\nbuild top menu %d", total_masters);


//clear the temp list before building a new temp list in-case list gets smaller
	for	(j=0;j<TOTAL_engine_pockets;j++)	//check each master
		temp_list[j].icon_resource=0;	//


//call menu builder script which will register all carried menu objects
	head = res_man.Res_open(MENU_MASTER_OBJECT);
	RunScript( (char*)head, (char*)head, &null_pc );	// run the 'build_menu' script in the 'menu_master' object
	res_man.Res_close(MENU_MASTER_OBJECT);

//compare new with old
//anything in master thats not in new gets removed from master - if found in new too, remove from temp

	if	(total_masters)
	{
		for	(j=0;j<total_masters;j++)	//check each master
		{
			for	(k=0;k<TOTAL_engine_pockets;k++)
			{
				res=0;
				if	(master_menu_list[j].icon_resource == temp_list[k].icon_resource)	//if master is in temp
				{
					temp_list[k].icon_resource=0;	//kill it in the temp
					res=1;
					break;
				}
			}
			if	(!res)
			{	master_menu_list[j].icon_resource=0;	//otherwise not in temp so kill in main
//				Zdebug("Killed menu %d",j);
			}
		}
	}

//merge master downwards

	total_masters=0;
	for	(j=0;j<TOTAL_engine_pockets;j++)	//check each master slot
	{
		if	((master_menu_list[j].icon_resource)&&(j!=total_masters))	//not current end - meaning out over the end so move down
		{
			memcpy( &master_menu_list[total_masters++], &master_menu_list[j], sizeof(menu_object));	//
			master_menu_list[j].icon_resource=0;	//moved down now so kill here
		}
		else if (master_menu_list[j].icon_resource)	//skip full slots
			total_masters++;
	}

//add those new to menu still in temp but not yet in master to the end of the master
	for	(j=0;j<TOTAL_engine_pockets;j++)	//check each master slot
		if	(temp_list[j].icon_resource)	//here's a new temp
			memcpy( &master_menu_list[total_masters++], &temp_list[j], sizeof(menu_object));	//


//init top menu from master list
	for	(j=0;j<15;j++)
	{
		if	(master_menu_list[j].icon_resource)
		{
			res = master_menu_list[j].icon_resource;	// 'res' is now the resource id of the icon

			//-----------------------------------------------------------------------------------------------------
			// WHEN AN ICON HAS BEEN RIGHT-CLICKED FOR 'EXAMINE' - SELECTION COLOURED, THE REST GREYED OUT

			if (examining_menu_icon)		// '1' when examining a menu-icon ('OBJECT_HELD' is the resource of the icon being examined)
			{
				if (res == OBJECT_HELD)		// if this is the icon being examined, make it coloured
					icon_coloured=1;
				else						// if not, grey this one out
					icon_coloured=0;
			}
			//-----------------------------------------------------------------------------------------------------
			// WHEN ONE MENU OBJECT IS BEING USED WITH ANOTHER - BOTH TO BE COLOURED, THE REST GREYED OUT

			else if (COMBINE_BASE)	// resource of second icon clicked
			{
				if ((res == OBJECT_HELD)||(res == COMBINE_BASE))	// if this if either of the icons being combined...
					icon_coloured=1;
				else
					icon_coloured=0;
			}
			//-----------------------------------------------------------------------------------------------------
			// NORMAL ICON SELECTION - SELECTION GREYED OUT, THE REST COLOURED

			else
			{
  				if (res == OBJECT_HELD)	// if this is the selction, grey it out
					icon_coloured=0;
				else						// if not, make it coloured
					icon_coloured=1;
			}
		 
 			//-----------------------------------------------------------------------------------------------------


			if (icon_coloured)	// coloured
				icon = res_man.Res_open( master_menu_list[j].icon_resource ) + sizeof(_standardHeader) + RDMENU_ICONWIDE*RDMENU_ICONDEEP;
			else				// greyed out
				icon = res_man.Res_open( master_menu_list[j].icon_resource ) + sizeof(_standardHeader);

			SetMenuIcon(RDMENU_BOTTOM, j, icon);
			res_man.Res_close( res );
		}
		else
		{
			SetMenuIcon(RDMENU_BOTTOM, j, NULL);	//no icon here
			//Zdebug(" NULL for %d", j);
		}
	}

	ShowMenu(RDMENU_BOTTOM);

}



//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void	Build_system_menu(void)	//Tony19Mar97
{
//start a fresh top system menu

	uint8	*icon;
	int	j;

	uint32	icon_list[5] =
	{
		OPTIONS_ICON,
		QUIT_ICON,
		SAVE_ICON,
		RESTORE_ICON,
		RESTART_ICON
	};


	for	(j=0;j<5;j++)	//build them all high in full colour - when one is clicked on all the rest will grey out
	{
		if	((DEAD)&&(j==2))	//dead then SAVE not available
			icon = res_man.Res_open( icon_list[j] ) + sizeof(_standardHeader);

		else	icon = res_man.Res_open( icon_list[j] ) + sizeof(_standardHeader) + RDMENU_ICONWIDE*RDMENU_ICONDEEP;
		SetMenuIcon(RDMENU_TOP, j, icon);
		res_man.Res_close( icon_list[j] );
	}


	ShowMenu(RDMENU_TOP);
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
