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
#include "driver/driver96.h"
#include "build_display.h"
#include "console.h"
#include "controls.h"
#include "debug.h"
#include "defs.h"
#include "events.h"
#include "icons.h"
#include "interpreter.h"
#include "layers.h"
#include "maketext.h"
#include "mouse.h"	//assure integrety
#include "object.h"
#include "protocol.h"
#include "resman.h"
#include "sound.h"
#include "sword2.h"	// for PauseGame() & UnpauseGame()
//------------------------------------------------------------------------------------
// pointer resource id's

#define CROSHAIR	18
#define EXIT0		788
#define EXIT1		789
#define EXIT2		790
#define EXIT3		791
#define EXIT4		792
#define EXIT5		793
#define EXIT6		794
#define EXIT7		795
#define EXITDOWN	796
#define EXITUP		797
#define MOUTH		787
#define NORMAL		17
#define PICKUP		3099
#define SCROLL_L	1440
#define SCROLL_R	1441
#define USE			3100
//------------------------------------------------------------------------------------
//the mouse list stuff
uint32	cur_mouse;
Mouse_unit mouse_list[TOTAL_mouse_list];

uint32	mouse_touching=0;	//set by Check_mouse_list
uint32	old_mouse_touching=0;

uint32	menu_selected_pos;
uint8	examining_menu_icon=0;

uint32	mouse_pointer_res=0;	// if it's NORMAL_MOUSE_ID (ie. normal pointer) then it's over a floor area (or hidden hot-zone
uint32	mouse_mode=0;	//0 normal in game
							//1 top menu down (bottom!)
							//2 dragging luggage
							//3 system menu chooser (top)
							//4 speech chooser

//Object_mouse	old_mouse_object;	//copy structure from list when moving onto a mouse area

uint32	menu_status;	//0 available - 1 unavailable
uint32	mouse_status;	//human 0 on/1 off

uint32	mouse_mode_locked=0;	//0 not !0 mode cannot be changed from normal mouse to top menu (i.e. when carrying big objects)
uint32	current_luggage_resource=0;

uint32	subject_status;	//0 off	1 on

uint32	old_button=0;	//for the re-click stuff - must be same button you see
uint32	button_click=0;

uint32	pointer_text_bloc_no=0;
uint32	pointerTextSelected=0;

uint32 player_activity_delay=0;	// player activity delay counter

uint32	real_luggage_item=0;	//last minute for pause mode

//------------------------------------------------------------------------------------
/*
#define RD_LEFTBUTTONDOWN			0x01
#define RD_LEFTBUTTONUP				0x02
#define RD_RIGHTBUTTONDOWN			0x04
#define RD_RIGHTBUTTONUP			0x08
*/
//------------------------------------------------------------------------------------
// local function prototypes

//uint8 Check_sprite_pixel( uint32 j );
void CreatePointerText(uint32 TextId, uint32 pointerRes);	// James16jun97
void Monitor_player_activity(void);	// James23july97

void	No_human(void);

//------------------------------------------------------------------------------------
void	Reset_mouse_list(void)	//Tony26Sept96
{
//call at beginning of gameloop

	cur_mouse=1;
}
//------------------------------------------------------------------------------------
void	Mouse_engine(void)	//Tony30Sept96
{

	Monitor_player_activity();	// James23july97

	ClearPointerText();	// James16jun97


	if	(DEAD)	//George is dead ;)
	{
		if	(mouse_mode!=MOUSE_system_menu)
		{
			mouse_mode=MOUSE_system_menu;
			if	(mouse_touching)
			{
//				get off
				old_mouse_touching=0;	//we've moved off
				mouse_touching=0;	//we were on something but not anymore
			}

			Set_mouse(NORMAL_MOUSE_ID);

			Build_system_menu();	//Tony19Mar97
		}
		System_menu();

		return;
	}

		
	if	(mouse_status)	//no human
		return;


	switch(mouse_mode)
	{
		case	MOUSE_normal:	//0 normal in game
			Normal_mouse();
			break;

		case	MOUSE_top:	//1 top menu down
			Top_menu_mouse();
			break;

		case	MOUSE_drag:	//2 dragging luggage
			Drag_mouse();
			break;

		case	MOUSE_system_menu:	//3 in game bottom menu - save/restore, etc?
			System_menu();
			break;

		case	MOUSE_holding:	//4 wait for mouse to move off bottom menu - after speech
			if	(mousey<400)
			{	mouse_mode=MOUSE_normal;
				Zdebug("   releasing");
			}
			break;

		default:
			break;
	}

}
//------------------------------------------------------------------------------------
void	System_menu(void)	//Tony19Mar97
{
	uint32 safe_looping_music_id;
	_mouseEvent	*me;
	int	j,hit;
	uint8	*icon;
	uint32	rv;	// for drivers return value
	int32	pars[2];
	uint32	icon_list[5] =
	{
		OPTIONS_ICON,
		QUIT_ICON,
		SAVE_ICON,
		RESTORE_ICON,
		RESTART_ICON
	};



	if	((mousey>0)&&(!DEAD))	//can't close when player is dead
	{	mouse_mode=MOUSE_normal;	//close menu

//		start the menu coming down
		HideMenu(RDMENU_TOP);

		return;
	}

	me = MouseEvent();	//get mouse event


	if	((me!=NULL)&&(me->buttons&RD_LEFTBUTTONDOWN))	//there's a mouse event to be processed
	{

//		clicked on a top mouse pointer?

			if	((mousex>=24)&&(mousex<640-24)&&(mousey<0))
			{
				hit=(mousex-24)/40;	//which are we over?

				if	((hit==2)&&(DEAD))
					return;	//no save when dead

				if	(hit<5)	//there are 5 system menus
				{	for	(j=0;j<5;j++)	//build them all high in full colour - when on eis clicked on all the rest will grey out
						if	(j!=hit)	//change all others to grey
						{
							icon = res_man.Res_open( icon_list[j] ) + sizeof(_standardHeader);
							SetMenuIcon(RDMENU_TOP, j, icon);
							res_man.Res_close( icon_list[j] );
						}

					//------------------------
					rv = g_sword2->_sound->PauseFx();
					if (rv != RD_OK)
						Zdebug("ERROR: PauseFx() returned %.8x in SystemMenu()", rv);
  					//------------------------
					// NB. Need to keep a safe copy of 'looping_music_id' for savegame
					// & for playing when returning from control panels
					// because control panel music will overwrite it!
					safe_looping_music_id = looping_music_id;
					pars[0] = 221;	// SystemM234 (M234.wav)
					pars[1] = FX_LOOP;
					FN_play_music(pars);
					looping_music_id = safe_looping_music_id;	// restore proper looping_music_id
					//------------------------
  					//------------------------
					// clear the screen & set up the new palette for the menus

					EraseBackBuffer();
					ProcessMenu();	// drivers to redraw menu over this blank screen!
					ResetRenderEngine();
  					//------------------------

					switch(hit)	//call the relevent screen
					{
						case	0:	// options
							Option_control();	// game options
							break;

						case	1:	// quit
							Quit_control();		// quit to windows
							break;

						case	2:	// save
							Save_control();		// save the game
							break;

						case	3:	// restore
							Restore_control();	// restore a game
							break;

						case	4:	// restart
							Restart_control();	// restart the game
							break;

					}
					//------------------------
//					start the menu coming down
					if	(!DEAD)	//not death screen
					{	mouse_mode=MOUSE_normal;	//close menu
						HideMenu(RDMENU_TOP);	//but not when dead
					}
					else
					{	Set_mouse(NORMAL_MOUSE_ID);
						Build_system_menu();	//reset top menu
					}
  					//------------------------
					// clear the screen & restore the location palette

					EraseBackBuffer();
					ProcessMenu();	// drivers to redraw menu over this blank screen!

					//------------------------
					// reset game palette, but not after a successful restore or restart!

					if (this_screen.new_palette != 99)	// see RestoreFromBuffer() in save_rest.cpp
					{
						SetFullPalette(0);	// '0' means put back game screen palette; see Build_display.cpp (James17jun97)
						this_screen.new_palette=0;	//stop the engine fading in the restored screens palette
					}
					else
						this_screen.new_palette=1;

					//------------------------
					rv = g_sword2->_sound->UnpauseFx();
					if (rv != RD_OK)
						Zdebug("ERROR: UnpauseFx() returned %.8x in SystemMenu()", rv);
					//------------------------
					// If there was looping music before coming into the control panels then restart it!
					// NB. This will also start music required when a game has been restored

					if (looping_music_id)
					{
						pars[0] = looping_music_id;
						pars[1] = FX_LOOP;
						FN_play_music(pars);
						// cross-fades into the required music:
						// - either a restored game tune
						// - or music playing prior to entering control panels
					}
					else
						FN_stop_music(NULL);	// stop the control panel music
					//------------------------
				}
			}
	}

}
//------------------------------------------------------------------------------------
void	Drag_mouse(void)	//Tony21Nov96
{
	_mouseEvent	*me;
	uint32	pos;
//	uint32	null_pc=1;	//script 1 is combine script


	if	((mousey<400)&&(!menu_status))
	{	mouse_mode=MOUSE_normal;	//close menu

//		start the menu coming down
		HideMenu(RDMENU_BOTTOM);

		return;
	}


	Mouse_on_off();	//handles cursors and the luggage on/off according to type



//	now do the normal click stuff

	me = MouseEvent();	//get mouse event


//	we only care about left clicks when the mouse is over an object
//	we ignore mouse releases

	if	((me!=NULL)&&(me->buttons&RD_LEFTBUTTONDOWN))	//there's a mouse event to be processed
	{

//		could be clicking on an on screen object or on the top menu which is currently displayed

		if	(mouse_touching)	//mouse is over an on screen object - and we have luggage
		{							//depending on type we'll maybe kill the object_held - like for exits
			if (me->buttons&RD_LEFTBUTTONDOWN)	// set global script variable 'button'
			{
				LEFT_BUTTON  = 1;
				RIGHT_BUTTON = 0;
			}
			else
			{
				LEFT_BUTTON  = 0;
				RIGHT_BUTTON = 1;
			}

			MOUSE_X=(uint32)mousex+this_screen.scroll_offset_x;
			MOUSE_Y=(uint32)mousey+this_screen.scroll_offset_y;	//these might be required by the action script about to be run

			CLICKED_ID = mouse_touching;	// for scripts to know what's been clicked (21jan97). First used for 'room_13_turning_script' in object 'biscuits_13'



			Set_player_action_event(CUR_PLAYER_ID, mouse_touching);	//Tony4Dec96

			//--------------------------------------
			// Write to walkthrough file (zebug0.txt)
			#ifdef _SWORD2_DEBUG
			Zdebug(0,"USED \"%s\" ICON ON %s", FetchObjectName(OBJECT_HELD), FetchObjectName(CLICKED_ID));
			#endif
			//--------------------------------------


			HideMenu(RDMENU_BOTTOM);	//hide menu too
			mouse_mode=MOUSE_normal;	//back to normal menu mode
		}
		else	//better check for combine/cancel	cancel puts us back in Top_menu_mouse mode
		{
			if	((mousex>=24)&&(mousex<640-24))
			{
				pos=(mousex-24)/40;	//which are we over?

				if	(master_menu_list[pos].icon_resource)	//clicked on something - what button?
				{
					mouse_mode=MOUSE_top;	//always back into top menu mode

					Set_luggage(0);	// remove luggage

					if	(pos==menu_selected_pos)	// if we've clicked on the same icon as the one we're dragging
					{
						OBJECT_HELD=0;	// reset first icon
						menu_selected_pos=0;
					}
					else	// combine the 2 icons
					{
//						Zdebug("combine");
						COMBINE_BASE=master_menu_list[pos].icon_resource;	//what we clicked on, not what we're dragging

						Set_player_action_event(CUR_PLAYER_ID, MENU_MASTER_OBJECT);	//Tony4Dec96

						No_human();	// turn off mouse now, to prevent player trying to click elsewhere BUT leave the bottom menu open

						//--------------------------------------
						// Write to walkthrough file (zebug0.txt)
						#ifdef _SWORD2_DEBUG
						Zdebug(0,"USED \"%s\" ICON ON \"%s\" ICON", FetchObjectName(OBJECT_HELD), FetchObjectName(COMBINE_BASE));
						#endif
						//--------------------------------------
					}

					Build_top_menu();	// refresh the menu


//					Zdebug("switch to top mode");
				}
			}
		}
	}


}
//------------------------------------------------------------------------------------
void	Top_menu_mouse(void)	//Tony3Oct96
{
//top menu is down
	_mouseEvent	*me;
	uint32	pos;

	if	((mousey<400)&&(!menu_status))
	{	mouse_mode=MOUSE_normal;	//close menu

//		start the menu coming down
		HideMenu(RDMENU_BOTTOM);

		return;
	}

	me=MouseEvent();	//get mouse event



//	we only care about left clicks when the mouse is over an object
//	we ignore mouse releases

	if	(me!=NULL)	//there's a mouse event to be processed
	{
//		now check if we've clicked on an actual icon

		if	((mousex>=24)&&(mousex<640-24))
		{
			pos=(mousex-24)/40;	//which are we over?

			if	(master_menu_list[pos].icon_resource)	//clicked on something - what button?
			{

				if	(me->buttons&RD_RIGHTBUTTONDOWN)	//right button look
				{
					examining_menu_icon=1;
					OBJECT_HELD = master_menu_list[pos].icon_resource;	//id the object via its graphic
					EXIT_CLICK_ID=0;	// (JEL09oct97) must clear this so next click on exit becomes 1st click again

					//--------------------------------------
					// Write to walkthrough file (zebug0.txt)
					#ifdef _SWORD2_DEBUG
					Zdebug(0,"RIGHT-CLICKED ON \"%s\" ICON", FetchObjectName(OBJECT_HELD));
					#endif
 					//--------------------------------------

					Set_player_action_event(CUR_PLAYER_ID, MENU_MASTER_OBJECT);	//Tony4Dec96

					Build_top_menu();	// refresh the menu
					No_human();	// turn off mouse now, to prevent player trying to click elsewhere BUT leave the bottom menu open
				}
				else	if	(me->buttons&RD_LEFTBUTTONDOWN)	//left button - highlight the object and bung us into drag luggage mode
				{
					menu_selected_pos=pos;	//menu slot we clicked on - derive luggage resource from this in mouse_on_off()
					current_luggage_resource=master_menu_list[pos].luggage_resource;

					mouse_mode=MOUSE_drag;
//					Zdebug("setting OH in top menu");
					OBJECT_HELD = master_menu_list[pos].icon_resource;	//id the object via its graphic
					EXIT_CLICK_ID=0;	// (JEL09oct97) must clear this so next click on exit becomes 1st click again

					Build_top_menu();	// refresh the menu

					Set_luggage(master_menu_list[pos].luggage_resource);

//					Zdebug("switch to drag mode");
				}
			}
		}
	}
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void	Normal_mouse(void)	//Tony30Sept96
{
//the gane is playing and none of the menus are activated - but, we need to check if the top menu is to start
//note, wont have luggage

	_mouseEvent	*me;




	if	((mousey<0)&&(!menu_status)&&(!mouse_mode_locked)&&(!OBJECT_HELD))	//no save in big-object menu lock situation
	{
		mouse_mode=MOUSE_system_menu;

		if	(mouse_touching)
		{
//			get off
			old_mouse_touching=0;	//we've moved off
			mouse_touching=0;	//we were on something but not anymore

		}

//		reset mouse cursor - in case we're between mice
		Set_mouse(NORMAL_MOUSE_ID);

		Build_system_menu();	//Tony19Mar97
	}





	if	((mousey>399)&&(!menu_status)&&(!mouse_mode_locked))
	{


		if	(!OBJECT_HELD)	//why are we testing for this?
		{
			mouse_mode=MOUSE_top;	//bring down top menu
		}
		else
		{
			mouse_mode=MOUSE_drag;
		}

//		if mouse is moving off an object and onto the top menu then do a standard get-off
		if	(mouse_touching)
		{
//			get off

			old_mouse_touching=0;	//we've moved off
			mouse_touching=0;	//we were on something but not anymore

		}

//		reset mouse cursor
		Set_mouse(NORMAL_MOUSE_ID);

//		build menu and start the menu coming down

		Build_top_menu();

		return;
	}


//check also for bringing the bottom menu up




	Mouse_on_off();	//handles



//	now do the normal click stuff

	me = MouseEvent();	//get mouse event

	//-----------------------------------------------------
#ifdef _SWORD2_DEBUG
	if (definingRectangles)
	{
		if (draggingRectangle==0)	// not yet dragging a rectangle, so need click to start
		{
			if ( (me!=NULL) && ((me->buttons&RD_LEFTBUTTONDOWN)||(me->buttons&RD_RIGHTBUTTONDOWN)) )
			{
				rect_x1 = rect_x2 = (uint32)mousex+this_screen.scroll_offset_x;		// set both (x1,y1)
				rect_y1 = rect_y2 = (uint32)mousey+this_screen.scroll_offset_y;	// & (x2,y2) to this point
				draggingRectangle=1;
			}
		}
		else if (draggingRectangle==1)	// currently dragging a rectangle
		{
			if ( (me!=NULL) && ((me->buttons&RD_LEFTBUTTONDOWN)||(me->buttons&RD_RIGHTBUTTONDOWN)) )	// click means reset
			{
				draggingRectangle=2;	// lock rectangle, so you can let go of mouse to type in the coords
			}
			else	// drag rectangle
			{
				rect_x2 = (uint32)mousex+this_screen.scroll_offset_x;
				rect_y2 = (uint32)mousey+this_screen.scroll_offset_y;
			}
		}
		else	// currently locked to avoid knocking out of place while reading off the coords
		{
			if ( (me!=NULL) && ((me->buttons&RD_LEFTBUTTONDOWN)||(me->buttons&RD_RIGHTBUTTONDOWN)) )	// click means reset
			{
				draggingRectangle=0;	// back to start again
			}
		}
	}
	else
#endif	// _SWORD2_DEBUG
	//-----------------------------------------------------
	{

		// we only care about down clicks when the mouse is over an object
		// we ignore mouse releases

		if	((me!=NULL)&&((me->buttons&RD_LEFTBUTTONDOWN)||(me->buttons&RD_RIGHTBUTTONDOWN))&&(mouse_touching))	//there's a mouse event to be processed and the mouse is on something
		{
//			ok, there are no menus about so its nice and simple
//			this is as close to the old advisor_188 script as we get I'm sorry to say.


//			if player is walking or relaxing then those need to terminate correctly


//			otherwise set player run the targets action script
//			or, do a special walk if clicking on the scroll-more icon

//			PLAYER_ACTION = mouse_touching;	//PLAYER_ACTION script variable - whatever catches this must reset to 0 again
													//idle or router-anim will catch it


			if (me->buttons&RD_LEFTBUTTONDOWN)	// set global script variable 'button'
			{
				LEFT_BUTTON  = 1;
				RIGHT_BUTTON = 0;
				button_click=0;	//for re-click
			}
			else
			{
				LEFT_BUTTON  = 0;
				RIGHT_BUTTON = 1;
				button_click=1;	//for re-click
			}

			MOUSE_X=(uint32)mousex+this_screen.scroll_offset_x;
			MOUSE_Y=(uint32)mousey+this_screen.scroll_offset_y;	//these might be required by the action script about to be run


			if	((mouse_touching==EXIT_CLICK_ID)&&(me->buttons&RD_LEFTBUTTONDOWN))	//only left button
			{
//				its the exit double click situation
//				let the existing interaction continue and start fading down - switch the human off too

				FN_no_human(NULL);
				FN_fade_down(NULL);
				EXIT_FADING=1;	//tell the walker

			}
			else	if	((old_button==button_click)&&(mouse_touching==CLICKED_ID)&&(mouse_pointer_res!=NORMAL_MOUSE_ID))
			{	//re-click - do nothing - except on floors
			}
			else	//allow the click
			{
				old_button=button_click;	//for re-click

				CLICKED_ID = mouse_touching;	// for scripts to know what's been clicked (21jan97). First used for 'room_13_turning_script' in object 'biscuits_13'
				EXIT_CLICK_ID=0;	//must clear these two double-click control flags - do it here so reclicks after exit clicks are cleared up
				EXIT_FADING=0;
				Set_player_action_event(CUR_PLAYER_ID, mouse_touching);	//Tony4Dec96

				//--------------------------------------
				// Write to walkthrough file (zebug0.txt)
				#ifdef _SWORD2_DEBUG
				if (OBJECT_HELD)
					Zdebug(0,"USED \"%s\" ICON ON %s", FetchObjectName(OBJECT_HELD), FetchObjectName(CLICKED_ID));
				else if (LEFT_BUTTON)
					Zdebug(0,"LEFT-CLICKED ON %s", FetchObjectName(CLICKED_ID));
				else	// RIGHT BUTTON
					Zdebug(0,"RIGHT-CLICKED ON %s", FetchObjectName(CLICKED_ID));
				#endif
				//--------------------------------------
			}
		}
	}
}
//------------------------------------------------------------------------------------
void	Mouse_on_off(void)	//Tony30Sept96
{
	//this handles the cursor graphic when moving on and off mouse areas
	//it also handles the luggage thingy

	uint32	pointer_type;
	static uint8	mouse_flicked_off=0;


	old_mouse_touching=mouse_touching;


	if	((mousey<0)||(mousey>399))	// don't detect objects that are hidden behind the menu bars (ie. in the scrolled-off areas of the screen)
	{	
		pointer_type=0;
		mouse_touching=0;
	}
	else	
		pointer_type = Check_mouse_list();	// set 'mouse_touching' & return pointer_type




	if	((!mouse_flicked_off)&&(old_mouse_touching==mouse_touching))	//same as previous cycle?
		return;	//yes, so nothing to do		BUT CARRY ON IF MOUSE WAS FLICKED OFF!

	mouse_flicked_off=0;	// can reset this now


	if	((!old_mouse_touching)&&(mouse_touching))	//the cursor has moved onto something
	{

//		make a copy of the object we've moved onto
//		because one day we'll move back off again! (but the list positioning could theoretically have changed)
//		we can only move onto something from being on nothing - we stop the system going from one to another when objects overlap

//		memcpy( &old_mouse_object, &mouse_list[mouse_touching], sizeof(Object_mouse));

		old_mouse_touching=mouse_touching;	//

//		run get on

		if (pointer_type)
		{
			Set_mouse(pointer_type);	// 'pointer_type' holds the resource id of the pointer anim

			if (OBJECT_HELD)			// setup luggage icon
//				Set_luggage(master_menu_list[menu_selected_pos].luggage_resource);
				Set_luggage(current_luggage_resource);

		}
		else
			Con_fatal_error("ERROR: mouse.pointer==0 for object %d (%s) - update logic script!", mouse_touching, FetchObjectName(mouse_touching));
	}
	else	if	((old_mouse_touching)&&(!mouse_touching))	// the cursor has moved off something
	{
		old_mouse_touching=0;		// we've moved off
		Set_mouse(NORMAL_MOUSE_ID);	// reset cursor to normal pointer
		// reset luggage only when necessary
	}
	else	if	((old_mouse_touching)&&(mouse_touching))	// the cursor has moved off something and onto something else
	{														// flip to a blank cursor for a cycle
		mouse_touching=0;			// ignore the new id this cycle - should hit next cycle
		old_mouse_touching=0;		// we've moved off
		Set_mouse(0);			// blank cursor
		mouse_flicked_off=1;		// so we know to set the mouse pointer back to normal if 2nd hot-spot doesn't register because mouse pulled away quickly (onto nothing)
		
		// reset luggage only when necessary
	}
	else	// for when mouse was flicked off for one cycle, but then moved onot nothing before 2nd hot-spot registered
	{
		// both 'old_mouse_touching' & 'mouse_touching' will be zero
		Set_mouse(NORMAL_MOUSE_ID);	// reset cursor to normal pointer
	}

	// possible check for edge of screen more-to-scroll here on large screens

}
//------------------------------------------------------------------------------------
void Set_mouse(uint32 res)	// (4dec96 JEL)
{
	uint8	*icon;
	uint32	len;

	mouse_pointer_res=res;	//high level - whats the mouse - for the engine

	if (res)	// if it's not NULL
	{
		icon = res_man.Res_open( res ) + sizeof(_standardHeader);
		len = res_man.resList[res]->size - sizeof(_standardHeader);

			if (res == NORMAL_MOUSE_ID)	// don't pulse the normal pointer
				SetMouseAnim(icon, len, RDMOUSE_NOFLASH);	// 0 means don't pulse this pointer, just do the regular anim loop
			else
	 			SetMouseAnim(icon, len, RDMOUSE_FLASH);	// 1 mean pulse before starting regular anim loop

		res_man.Res_close( res );
	}
	else
		SetMouseAnim(NULL,0,0);	// blank cursor
}
//------------------------------------------------------------------------------------
void Set_luggage(uint32	res)	//Tony26Nov96
{
	uint8	*icon;
	uint32	len;

	if (res)	// if not NULL
	{
		real_luggage_item=res;

		icon = res_man.Res_open( res ) + sizeof(_standardHeader);
		len = res_man.resList[res]->size - sizeof(_standardHeader);

			SetLuggageAnim(icon, len);

		res_man.Res_close( res );
	}
	else
	{	real_luggage_item=0;
		SetLuggageAnim(NULL, 0);
	}
}
//------------------------------------------------------------------------------------
uint32	Check_mouse_list(void)	//Tony30Sept96
{
	int32	priority=0;
	uint32	j=1;


	if	(cur_mouse>1)
	{
		while(priority<10)	//number of priorities subject to implementation needs
		{
			if ((mouse_list[j].priority==priority) &&	// if the mouse pointer is over this mouse-detection-box
				(mousex+this_screen.scroll_offset_x >= mouse_list[j].x1) &&
				(mousex+this_screen.scroll_offset_x <= mouse_list[j].x2) &&
				(mousey+this_screen.scroll_offset_y >= mouse_list[j].y1) &&
				(mousey+this_screen.scroll_offset_y <= mouse_list[j].y2))
			{

/*
				if (mouse_list[j].anim_resource)	// want to use sprite as a mouse mask, for better accuracy of detection (25oct96 JEL)
				{
					// only works for uncompressed sprite data!!
					// THIS IS NEVER USED IN SWORD2

	   				if (Check_sprite_pixel(j))	// if the mouse is touching a non-zero pixel of the sprite
					{
						mouse_touching=mouse_list[j].id;	// record id
						CreatePointerText(mouse_list[j].pointer_text);	// James16jun97
						return(mouse_list[j].pointer);	//return pointer type
					}
				}
				else	// ok, we're touching the detection-box
*/
				{
					mouse_touching=mouse_list[j].id;	// record id

					// change all COGS pointers to CROSHAIR
					if (mouse_list[j].pointer == USE)
						mouse_list[j].pointer = CROSHAIR;
					CreatePointerText(mouse_list[j].pointer_text, mouse_list[j].pointer);	// James16jun97
					return(mouse_list[j].pointer);	//return pointer type
				}
			}

			j++;	//next
			if	(j==cur_mouse)
			{
				j=0;
				priority++;				//next priority - 0 being the highest, 9 the lowest
			}
		}
	}

	mouse_touching=0;	// touching nothing
	return(0);			// no pointer to return
}
//------------------------------------------------------------------------------------

void CreatePointerText(uint32 textId, uint32 pointerRes)	// James16jun97
{
	uint32	local_text;
	uint32	text_res;
	uint8	*text;
	int16	xOffset, yOffset;	// offsets for pointer text sprite from pointer position
	uint8	justification;

	#define POINTER_TEXT_WIDTH	640		// just in case!
	#define POINTER_TEXT_PEN	184		// white

	if (pointerTextSelected)
	{
		if (textId)
		{
			//-------------------------------------------
			// check what the pointer is, to set offsets correctly for text position

			switch(pointerRes)
			{
				case CROSHAIR:
					yOffset = -7;	// above	(above & to the right of the pointer coordinate)
					xOffset = +10;	// right
					break;

				case EXIT0:
					yOffset = +15;	// below
					xOffset = +20;	// right
					break;

				case EXIT1:
					yOffset = +16;	// below
					xOffset = -10;	// left
					break;

				case EXIT2:
					yOffset = +10;	// below
					xOffset = -22;	// left
					break;

				case EXIT3:
					yOffset = -16;	// above
					xOffset = -10;	// left
					break;

				case EXIT4:
					yOffset = -15;	// above
					xOffset = +15;	// right
					break;

				case EXIT5:
					yOffset = -12;	// above
					xOffset = +10;	// right
					break;

				case EXIT6:
					yOffset = +10;	// below
					xOffset = +25;	// right
					break;

				case EXIT7:
					yOffset = +16;	// below
					xOffset = +20;	// right
					break;

				case EXITDOWN:
					yOffset = -20;	// above
					xOffset = -10;	// left
					break;

				case EXITUP:
					yOffset = +20;	// below
					xOffset = +20;	// right
					break;

				case MOUTH:
					yOffset = -10;	// above
					xOffset = +15;	// right
					break;

				case NORMAL:
					yOffset = -10;	// above
					xOffset = +15;	// right
					break;

				case PICKUP:
					yOffset = -40;	// above
					xOffset = +10;	// right
					break;

				case SCROLL_L:
					yOffset = -20;	// above
					xOffset = +20;	// right
					break;

				case SCROLL_R:
					yOffset = -20;	// above
					xOffset = -20;	// left
					break;

				case USE:
					yOffset = -8;	// above
					xOffset = +20;	// right
					break;

				default:	// shouldn't happen if we cover all the different mouse pointers above
					yOffset = -10;	// above
					xOffset = +10;	// right
			}

   			//-------------------------------------------
			// set up justification for text sprite,
			// based on it's offsets from the pointer position

			// from maketext.h
			//#define NO_JUSTIFICATION				0	// only for debug text, since it doesn't keep text inside the screen margin!
			//#define POSITION_AT_CENTRE_OF_BASE	1	// these all force text inside the screen edge margin when necessary
			//#define POSITION_AT_CENTRE_OF_TOP		2
			//#define POSITION_AT_LEFT_OF_TOP		3
			//#define POSITION_AT_RIGHT_OF_TOP		4
			//#define POSITION_AT_LEFT_OF_BASE		5
			//#define POSITION_AT_RIGHT_OF_BASE		6
			//#define POSITION_AT_LEFT_OF_CENTRE	7
			//#define POSITION_AT_RIGHT_OF_CENTRE	8

			//-----------------------------------------
			if (yOffset < 0)		// if above pointer
			{
				if (xOffset < 0)		// above left
				{
					justification = POSITION_AT_RIGHT_OF_BASE;
				}
				else if (xOffset > 0)	// above right
				{
					justification = POSITION_AT_LEFT_OF_BASE;	// text sprite is justified from it's bottom-left corner
				}
				else	// (xOffset==0)	// above centre
				{
					justification = POSITION_AT_CENTRE_OF_BASE;
				}
			}
			//-----------------------------------------
			else if (yOffset > 0)	// if below pointer
			{
				if (xOffset < 0)		// below left
				{
					justification = POSITION_AT_RIGHT_OF_TOP;
				}
				else if (xOffset > 0)	// below right
				{
					justification = POSITION_AT_LEFT_OF_TOP;
				}
				else	// (xOffset==0)	// below centre
				{
					justification = POSITION_AT_CENTRE_OF_TOP;
				}
			}
			//-----------------------------------------
			else					// if at same y-coord as pointer
			{
				if (xOffset < 0)		// centre left
				{
					justification = POSITION_AT_RIGHT_OF_CENTRE;
				}
				else if (xOffset > 0)	// centre right
				{
					justification = POSITION_AT_LEFT_OF_CENTRE;
				}
				else	// (xOffset==0)	// centre centre
				{
					justification = POSITION_AT_LEFT_OF_CENTRE;		// shouldn't happen anyway!
				}
			}
			//-------------------------------------------

			text_res	= textId/SIZE;		// text resource number
			local_text	= textId&0xffff;	// text line number within the resource

			text = FetchTextLine( res_man.Res_open(text_res), local_text );	// open text file & get the line

			// 'text+2' to skip the first 2 bytes which form the line reference number
			pointer_text_bloc_no = Build_new_block(text+2, mousex+xOffset, mousey+yOffset, POINTER_TEXT_WIDTH, POINTER_TEXT_PEN, RDSPR_TRANS+RDSPR_DISPLAYALIGN, speech_font_id, justification);

			res_man.Res_close(text_res);	// now ok to close the text file
		}
	}
}
//------------------------------------------------------------------------------------
void ClearPointerText(void)	// James16jun97
{
	if (pointer_text_bloc_no)
	{
		Kill_text_bloc(pointer_text_bloc_no);
		pointer_text_bloc_no=0;
	}
}
//------------------------------------------------------------------------------------
/* NOT USED IN SWORD2
uint8 Check_sprite_pixel( uint32 j )	// (25oct96 JEL)
{
	// only works for uncompressed sprite data!!

	uint8 *file;
	uint8 hit;
	_frameHeader *frame_head;
	int16 sprite_x, sprite_y;
	int16 in_sprite_x, in_sprite_y;
	uint8 *sprite_data;


	sprite_x = mouse_list[j].x1;	// sprite coords have been copied to mouse area coords by FN_register_X_frame (X = sort, back or fore)
	sprite_y = mouse_list[j].y1;	// so easier to get them from there than from the anim file again


	file = res_man.Res_open(mouse_list[j].anim_resource);		// open the anim file & point to start of it
	frame_head = FetchFrameHeader(file,mouse_list[j].anim_pc);	// point to frame header of current frame
	sprite_data = (uint8 *)(frame_head + 1);					// point to start of frame data

	in_sprite_x = mousex + this_screen.scroll_offset_x - sprite_x;	// x-coord of mouse from origin at top-left of sprite
	in_sprite_y = mousey + this_screen.scroll_offset_y - sprite_y;	// y-coord of mouse from origin at top-left of sprite

	hit = sprite_data[in_sprite_y * frame_head->width + in_sprite_x];	// hit = value of pixel to which the mouse is pointing

 	res_man.Res_close(mouse_list[j].anim_resource);				// close anim file

	return (hit);	// we are touching the sprite if 'hit' is non-zero
}
*/
//------------------------------------------------------------------------------------
int32 FN_no_human(int32 *params)	//Tony30Sept96
{
//param	none

	MOUSE_AVAILABLE = 0;	// for logic scripts (James21may97)

	ClearPointerText();

	mouse_status=1;	//human/mouse off

	Set_mouse(0);			// blank cursor
	Set_luggage(0);			// blank cursor


//must be normal mouse situation or a largely neutral situation - special menus use No_human
	if	(TALK_FLAG==0)	//dont hide menu in conversations
		HideMenu(RDMENU_BOTTOM);


	if	(mouse_mode==MOUSE_system_menu)
	{
		mouse_mode=MOUSE_normal;	//close menu
		HideMenu(RDMENU_TOP);	//	start the menu coming down
	}


	if (params);


	return(1);	//script continue
}
//------------------------------------------------------------------------------------
void	No_human(void)	//Tony4June97
{
//leaves the menus open
//used by the system when clicking right on a menu item to examine it and
//when combining objects

	MOUSE_AVAILABLE = 0;	// for logic scripts (James21may97)

	mouse_status=1;	//human/mouse off

	Set_mouse(0);			// blank cursor
	Set_luggage(0);			// blank cursor
}
//------------------------------------------------------------------------------------
int32 FN_add_human(int32 *params)	//Tony30Sept96
{
	//param	none
#ifdef _SWORD2_DEBUG
	uint8 black[4]	= {0,0,0,0};
	uint8 white[4]	= {255,255,255,0};
#endif									// ('0' means don't print to console, but console isn't up anyway)



	MOUSE_AVAILABLE = 1;	// for logic scripts (James21may97)


	if	(mouse_status)	//off
	{
		mouse_status=0;	//on
		mouse_touching=1;	//forces engine to choose a cursor
	}

	CLICKED_ID=0;	//clear this to reset no-second-click system

	// this is now done outside the OBJECT_HELD check in case it's set to zero before now! (James 10july97)
	mouse_mode_locked=0;	//unlock the mouse from possible large object lock situtations - see syphon in rm 3

	if	(OBJECT_HELD)	//was dragging something around
	{
		OBJECT_HELD=0;			// need to clear this again

		examining_menu_icon=0;	// and these may also need clearing
		COMBINE_BASE=0;			// - just in case


		Set_luggage(0);			// blank cursor
	}



	if ((mousey>399)&&(mouse_mode!=MOUSE_holding))	// if mouse is over menu area
	{
		mouse_mode=MOUSE_normal;	// VITAL - reset things & rebuild the menu
		Set_mouse(NORMAL_MOUSE_ID);
	}
	else if (mousey>399)
		Set_mouse(NORMAL_MOUSE_ID);


	//----------------------------------------------------------------------------------------------
	// enabled/disabled from console; status printed with on-screen debug info
#ifdef _SWORD2_DEBUG
	if (testingSnR)						// testing logic scripts by simulating an instant Save & Restore
	{
		BS2_SetPalette(0, 1, white, RDPAL_INSTANT);

		Clear_fx_queue();				// stops all fx & clears the queue - eg. when leaving a location
		res_man.Kill_all_objects(0);	// ie. trashing all object resources so they load in fresh & restart their logic scripts

		BS2_SetPalette(0, 1, black, RDPAL_INSTANT);
	}
#endif									// ('0' means don't print to console, but console isn't up anyway)
	//----------------------------------------------------------------------------------------------
	if (params);

	return(1);	//script continue
}
//------------------------------------------------------------------------------------
int32 FN_register_mouse(int32 *params)	//Tony29Oct96
{
//this call would be made from an objects service script 0
//the object would be one with no graphic but with a mouse - i.e. a floor
// or one whose mouse area is manually defined rather than intended to fit sprite shape

//param	0 pointer to Object_mouse or 0 for no write to mouse list

//	Zdebug(1,"cur_mouse = %d", cur_mouse);


   Object_mouse *ob_mouse;

	ob_mouse = (Object_mouse *) params[0];	// param 1 is pointer to mouse structure

	if (ob_mouse->pointer)	// only if 'pointer' isn't NULL
	{
		#ifdef _SWORD2_DEBUG
		if	(cur_mouse==TOTAL_mouse_list)
			Con_fatal_error("ERROR: mouse_list full [%s line %u]",__FILE__,__LINE__);
		#endif

		mouse_list[cur_mouse].x1			= ob_mouse->x1;
		mouse_list[cur_mouse].y1			= ob_mouse->y1;
		mouse_list[cur_mouse].x2			= ob_mouse->x2;
		mouse_list[cur_mouse].y2			= ob_mouse->y2;

		mouse_list[cur_mouse].priority		= ob_mouse->priority;
		mouse_list[cur_mouse].pointer		= ob_mouse->pointer;

		//-----------------------------------------------
		// (James17jun97)
		// check if pointer text field is set due to previous object using this slot (ie. not correct for this one)
		if ((mouse_list[cur_mouse].pointer_text) && (mouse_list[cur_mouse].id != (int32)ID))	// if 'pointer_text' field is set, but the 'id' field isn't same is current id
			mouse_list[cur_mouse].pointer_text=0;												// then we don't want this "left over" pointer text
		//-----------------------------------------------

		mouse_list[cur_mouse].id			= ID;	// get id from system variable 'id' which is correct for current object

		mouse_list[cur_mouse].anim_resource	= 0;	// not using sprite as mask - this is only done from FN_register_frame()
		mouse_list[cur_mouse].anim_pc		= 0;

		//Zdebug("mouse id %d", mouse_list[cur_mouse].id);

		cur_mouse++;
	}

	return(IR_CONT);	//	continue script
}
//------------------------------------------------------------------------------------
// use this in the object's service script prior to registering the mouse area
// ie. before FN_register_mouse or FN_register_frame
// - best if kept at very top of service script

int32 FN_register_pointer_text(int32 *params)	// James16jun97
{
//	param 0 local id of text line to use as pointer text

	#ifdef _SWORD2_DEBUG
	if	(cur_mouse==TOTAL_mouse_list)
		Con_fatal_error("ERROR: mouse_list full [%s line %u]",__FILE__,__LINE__);
	#endif

	mouse_list[cur_mouse].id			= ID;	// current object id - used for checking pointer_text when mouse area registered (in FN_register_mouse & FN_register_frame)
	mouse_list[cur_mouse].pointer_text	= params[0];

	return(IR_CONT);	//	continue script

}
//------------------------------------------------------------------------------------
int32 FN_blank_mouse(int32 *params)	//Tony29Oct96
{
//set mouse to normal pointer - used in speech

//no params


	Set_mouse(0);

	if (params);

	return(1);	//cont
}
//------------------------------------------------------------------------------------
int32 FN_init_floor_mouse(int32 *params)	// James29nov96
{
	// params	0 pointer to object's mouse structure

 	Object_mouse	*ob_mouse = (Object_mouse *) params[0];

	ob_mouse->x1		= 0;
	ob_mouse->y1		= 0;
	ob_mouse->x2		= this_screen.screen_wide-1;
	ob_mouse->y2		= this_screen.screen_deep-1;
	ob_mouse->priority	= 9;				// floor is always lowest priority
	ob_mouse->pointer	= NORMAL_MOUSE_ID;	// normal pointer


	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
#define SCROLL_MOUSE_WIDTH	20					// James13feb97 (updated by James 25mar97)
//---------------------------------------------------------------------------------------------------------------------
int32 FN_set_scroll_left_mouse(int32 *params)	// James13feb97
{
	// params	0 pointer to object's mouse structure

 	Object_mouse	*ob_mouse = (Object_mouse *) params[0];


	ob_mouse->x1		= 0;
	ob_mouse->y1		= 0;
	ob_mouse->x2		= this_screen.scroll_offset_x + SCROLL_MOUSE_WIDTH;
	ob_mouse->y2		= this_screen.screen_deep-1;
	ob_mouse->priority	= 0;	// highest priority

	if (this_screen.scroll_offset_x > 0)	// if not fully scrolled to the left
		ob_mouse->pointer = SCROLL_LEFT_MOUSE_ID;
	else
		ob_mouse->pointer = 0;	// so the mouse area doesn't get registered


	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_set_scroll_right_mouse(int32 *params)	// James13feb97
{
	// params	0 pointer to object's mouse structure

 	Object_mouse	*ob_mouse = (Object_mouse *) params[0];


	ob_mouse->x1		= this_screen.scroll_offset_x + screenWide - SCROLL_MOUSE_WIDTH;
	ob_mouse->y1		= 0;
	ob_mouse->x2		= this_screen.screen_wide-1;
	ob_mouse->y2		= this_screen.screen_deep-1;
	ob_mouse->priority	= 0;	// highest priority

	if (this_screen.scroll_offset_x < this_screen.max_scroll_offset_x)	// if not fully scrolled to the right
		ob_mouse->pointer = SCROLL_RIGHT_MOUSE_ID;
	else
		ob_mouse->pointer = 0;	// so the mouse area doesn't get registered


	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32	FN_set_object_held(int32	*params)	//tony19May97
{
//params	0 luggage icon to set

	Set_luggage(params[0]);

	OBJECT_HELD=params[0];
	current_luggage_resource=params[0];

	mouse_mode_locked=1;	//mode locked - no top menu available



	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
// called from speech scripts to remove the chooser bar when it's not appropriate to keep it displayed
int32 FN_remove_chooser(int32 *params)	// James13aug97
{
	HideMenu(RDMENU_BOTTOM);

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_disable_menu(int32 *params)	//Tony1Oct96
{
	mouse_mode_locked=1;	//mode locked - no top menu available
	mouse_mode=MOUSE_normal;

	HideMenu(RDMENU_TOP);
	HideMenu(RDMENU_BOTTOM);

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32	FN_enable_menu(int32	*params)	//tony4June97
{


	mouse_mode_locked=0;	//mode locked - no top menu available

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_check_player_activity(int32 *params)	// James23july97
{
//	Used to decide when to trigger music cues described as "no player activity for a while"

//	params 0	threshold delay in seconds, ie. what we want to check the actual delay against

	uint32 threshold = params[0]*12;	// in game cycles

	if (player_activity_delay >= threshold)	// if the actual delay is at or above the given threshold
	{
		player_activity_delay=0;	// reset activity delay counter, now that we've got a positive check
		RESULT=1;
	}
	else
		RESULT=0;

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
int32 FN_reset_player_activity_delay(int32 *params)	// James23july97
{
// Use if you want to deliberately reset the "no player activity" counter for any reason

//	no params

	player_activity_delay=0;	// reset activity delay counter

	return(IR_CONT);	//	continue script
}
//---------------------------------------------------------------------------------------------------------------------
void Monitor_player_activity(void)	// James23july97
{
	if (CheckForMouseEvents())	// if there is at least one mouse event outstanding
		player_activity_delay=0;	// reset activity delay counter
	else
		player_activity_delay++;	// no. of game cycles since mouse event queue last empty
}
//---------------------------------------------------------------------------------------------------------------------

