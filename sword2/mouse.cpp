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
#include "mouse.h"
#include "object.h"
#include "protocol.h"
#include "resman.h"
#include "sound.h"
#include "sword2.h"

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
#define USE		3100

//the mouse list stuff

uint32 cur_mouse;
Mouse_unit mouse_list[TOTAL_mouse_list];

// set by Check_mouse_list
uint32 mouse_touching = 0;
uint32 old_mouse_touching = 0;

uint32 menu_selected_pos;
uint8 examining_menu_icon = 0;

// if it's NORMAL_MOUSE_ID (ie. normal pointer) then it's over a floor area
// (or hidden hot-zone)

uint32 mouse_pointer_res = 0;

// 0 normal in game
// 1 top menu down (bottom!)
// 2 dragging luggage
// 3 system menu chooser (top)
// 4 speech chooser

uint32 mouse_mode = 0;

// copy structure from list when moving onto a mouse area
// Object_mouse old_mouse_object;

uint32 menu_status;			// 0 available - 1 unavailable
uint32 mouse_status;			// human 0 on/1 off

// 0 not !0 mode cannot be changed from normal mouse to top menu (i.e. when
// carrying big objects)

uint32 mouse_mode_locked = 0;
uint32 current_luggage_resource = 0;

uint32 subject_status;			//0 off 1 on

// for the re-click stuff - must be same button you see
uint32 old_button = 0;
uint32 button_click = 0;

uint32 pointer_text_bloc_no = 0;
uint32 pointerTextSelected = 0;

uint32 player_activity_delay = 0;	// player activity delay counter

uint32 real_luggage_item = 0;		// last minute for pause mode

void CreatePointerText(uint32 TextId, uint32 pointerRes);
void Monitor_player_activity(void);

void No_human(void);

void Reset_mouse_list(void) {
	// call at beginning of gameloop
	cur_mouse = 1;
}

void Mouse_engine(void) {
	Monitor_player_activity();
	ClearPointerText();

	if (DEAD) {	//George is dead ;)
		if (mouse_mode != MOUSE_system_menu) {
			mouse_mode = MOUSE_system_menu;
			if (mouse_touching) {
				// we've moved off
				old_mouse_touching = 0;

				// we were on something but not anymore
				mouse_touching = 0;
			}

			Set_mouse(NORMAL_MOUSE_ID);
			Build_system_menu();
		}
		System_menu();
		return;
	}

	if (mouse_status) {
		// no human
		return;
	}

	switch (mouse_mode) {
	case MOUSE_normal:
		Normal_mouse();
		break;
	case MOUSE_top:
		Top_menu_mouse();
		break;
	case MOUSE_drag:
		Drag_mouse();
		break;
	case MOUSE_system_menu:
		System_menu();
		break;
	case MOUSE_holding:
		if (mousey < 400) {
			mouse_mode = MOUSE_normal;
			Zdebug("   releasing");
		}
		break;
	default:
		break;
	}
}

void System_menu(void) {
	uint32 safe_looping_music_id;
	_mouseEvent *me;
	int j, hit;
	uint8 *icon;
	uint32 rv;	// for drivers return value
	int32 pars[2];
	uint32 icon_list[5] = {
		OPTIONS_ICON,
		QUIT_ICON,
		SAVE_ICON,
		RESTORE_ICON,
		RESTART_ICON
	};

	//can't close when player is dead
	if (mousey > 0 && !DEAD) {
		// close menu
		mouse_mode = MOUSE_normal;
		HideMenu(RDMENU_TOP);
		return;
	}

	me = MouseEvent();

	if (me && (me->buttons & RD_LEFTBUTTONDOWN)) {
		// clicked on a top mouse pointer?

		if (mousex >= 24 && mousex < 640 - 24 && mousey < 0) {
			// which are we over?
			hit = (mousex - 24) / 40;

			// no save when dead
			if (icon_list[hit] == SAVE_ICON && DEAD)
				return;

			//there are 5 system menu icons
			if (hit < ARRAYSIZE(icon_list)) {
				// build them all high in full colour - when
				// one is clicked on all the rest will grey out
				for (j = 0; j < ARRAYSIZE(icon_list); j++) {
					// change all others to grey
					if (j != hit) {
						icon = res_man.Res_open(icon_list[j]) + sizeof(_standardHeader);
						SetMenuIcon(RDMENU_TOP, j, icon);
						res_man.Res_close( icon_list[j] );
					}
				}

				rv = g_sound->PauseFx();
				if (rv != RD_OK)
					Zdebug("ERROR: PauseFx() returned %.8x in SystemMenu()", rv);

				// NB. Need to keep a safe copy of
				// 'looping_music_id' for savegame & for
				// playing when returning from control panels
				// because control panel music will overwrite
				// it!

				safe_looping_music_id = looping_music_id;

				pars[0] = 221;	// SystemM234 (M234.wav)
				pars[1] = FX_LOOP;
				FN_play_music(pars);

				// restore proper looping_music_id
				looping_music_id = safe_looping_music_id;

				// clear the screen & set up the new palette
				// for the menus

				EraseBackBuffer();
				ProcessMenu();
				ResetRenderEngine();

				// call the relevent screen
				switch (hit) {
				case 0:
					Option_control();
					break;
				case 1:
					Quit_control();
					break;
				case 2:
					Save_control();
					break;
				case 3:
					Restore_control();
					break;
				case 4:
					Restart_control();
					break;
				}

				// Menu stays open on death screen
				if (!DEAD) {
					mouse_mode = MOUSE_normal;
					HideMenu(RDMENU_TOP);
				} else {
					Set_mouse(NORMAL_MOUSE_ID);
					Build_system_menu();
				}

				// clear the screen & restore the location
				// palette

				EraseBackBuffer();
				ProcessMenu();

				// reset game palette, but not after a
				// successful restore or restart!

				// see RestoreFromBuffer() in save_rest.cpp
				if (this_screen.new_palette != 99) {
					// '0' means put back game screen
					// palette; see Build_display.cpp
					// (James17jun97)

					SetFullPalette(0);

					// stop the engine fading in the
					// restored screens palette

					this_screen.new_palette = 0;
				} else
					this_screen.new_palette = 1;

				rv = g_sound->UnpauseFx();
				if (rv != RD_OK)
					Zdebug("ERROR: UnpauseFx() returned %.8x in SystemMenu()", rv);

				// If there was looping music before coming
				// into the control panels then restart it!
				// NB. This will also start music required
				// when a game has been restored

				if (looping_music_id) {
					pars[0] = looping_music_id;
					pars[1] = FX_LOOP;
					FN_play_music(pars);

					// cross-fades into the required music:
					// - either a restored game tune
					// - or music playing prior to
					// entering control panels
				} else {
					// stop the control panel music
					FN_stop_music(NULL);
				}
			}
		}
	}
}

void Drag_mouse(void) {
	_mouseEvent *me;
	uint32 pos;
//	uint32 null_pc = 1;	//script 1 is combine script

	if (mousey < 400 && !menu_status) {
		// close menu
		mouse_mode = MOUSE_normal;
		HideMenu(RDMENU_BOTTOM);
		return;
	}

	// handles cursors and the luggage on/off according to type
	Mouse_on_off();

	// now do the normal click stuff

	me = MouseEvent();

	// we only care about left clicks when the mouse is over an object
	// we ignore mouse releases

	if (me && (me->buttons & RD_LEFTBUTTONDOWN)) {
		// there's a mouse event to be processed

		// could be clicking on an on screen object or on the top
		// menu which is currently displayed

		if (mouse_touching) {
			// mouse is over an on screen object - and we have
			// luggage

			// depending on type we'll maybe kill the object_held
			// - like for exits

			// Set global script variable 'button'. We know that
			// it was the left button, not the right one.

			LEFT_BUTTON = 1;
			RIGHT_BUTTON = 0;

			// these might be required by the action script about
			// to be run

			MOUSE_X = (uint32) mousex + this_screen.scroll_offset_x;
			MOUSE_Y = (uint32) mousey + this_screen.scroll_offset_y;

			// for scripts to know what's been clicked (21jan97).
			// First used for 'room_13_turning_script' in object
			// 'biscuits_13'

			CLICKED_ID = mouse_touching;

			Set_player_action_event(CUR_PLAYER_ID, mouse_touching);	// Tony4Dec96

#ifdef _SWORD2_DEBUG
			// Write to walkthrough file (zebug0.txt)
			Zdebug(0, "USED \"%s\" ICON ON %s", FetchObjectName(OBJECT_HELD), FetchObjectName(CLICKED_ID));
#endif

			// Hide menu - back to normal menu mode

			HideMenu(RDMENU_BOTTOM);
			mouse_mode=MOUSE_normal;
		} else {
			// better check for combine/cancel
			// cancel puts us back in Top_menu_mouse mode
			if (mousex >= 24 && mousex < 640 - 24) {
				// which are we over?
				pos = (mousex - 24) / 40;

				//clicked on something - what button?
				if (master_menu_list[pos].icon_resource) {
					// always back into top menu mode
					mouse_mode = MOUSE_top;

					// remove luggage
					Set_luggage(0);

					// if we've clicked on the same icon
					// as the one we're dragging

					if (pos == menu_selected_pos) {
						// reset first icon
						OBJECT_HELD = 0;
						menu_selected_pos = 0;
					} else {
						// combine the 2 icons
						// Zdebug("combine");

						//what we clicked on, not what
						// we're dragging

						COMBINE_BASE = master_menu_list[pos].icon_resource;
						Set_player_action_event(CUR_PLAYER_ID, MENU_MASTER_OBJECT);	//Tony4Dec96

						// turn off mouse now, to
						// prevent player trying to
						// click elsewhere BUT leave
						// the bottom menu open

						No_human();

#ifdef _SWORD2_DEBUG
						// Write to walkthrough file
						// (zebug0.txt)
						Zdebug(0, "USED \"%s\" ICON ON \"%s\" ICON", FetchObjectName(OBJECT_HELD), FetchObjectName(COMBINE_BASE));
#endif
					}

					// refresh the menu
					Build_top_menu();
					// Zdebug("switch to top mode");
				}
			}
		}
	}
}

void Top_menu_mouse(void) {
	// top menu is down

	_mouseEvent *me;
	uint32 pos;

	if (mousey < 400 && !menu_status) {
		// close menu
		mouse_mode = MOUSE_normal;
		HideMenu(RDMENU_BOTTOM);
		return;
	}

	me = MouseEvent();

	// we only care about left clicks when the mouse is over an object
	// we ignore mouse releases

	if (me) {
		// there's a mouse event to be processed
		// now check if we've clicked on an actual icon

		if (mousex >= 24 && mousex < 640 - 24) {
			// which are we over?
			pos = (mousex - 24) / 40;

			// clicked on something - what button?
			if (master_menu_list[pos].icon_resource) {
				if (me->buttons & RD_RIGHTBUTTONDOWN) {
					// right button look
					examining_menu_icon = 1;

					// id the object via its graphic
					OBJECT_HELD = master_menu_list[pos].icon_resource;

					// (JEL09oct97) must clear this so
					// next click on exit becomes 1st
					// click again

					EXIT_CLICK_ID = 0;

#ifdef _SWORD2_DEBUG
					// Write to walkthrough file
					// (zebug0.txt)
					Zdebug(0, "RIGHT-CLICKED ON \"%s\" ICON", FetchObjectName(OBJECT_HELD));
#endif

					Set_player_action_event(CUR_PLAYER_ID, MENU_MASTER_OBJECT);	// Tony4Dec96

					// refresh the menu
					Build_top_menu();

					// turn off mouse now, to prevent
					// player trying to click elsewhere
					// BUT leave the bottom menu open

					No_human();
				} else if (me->buttons&RD_LEFTBUTTONDOWN) {
					// left button - highlight the object
					// and bung us into drag luggage mode

					// menu slot we clicked on - derive
					// luggage resource from this in
					// mouse_on_off()

					menu_selected_pos = pos;
					current_luggage_resource = master_menu_list[pos].luggage_resource;

					mouse_mode = MOUSE_drag;
					// Zdebug("setting OH in top menu");

					// id the object via its graphic
					OBJECT_HELD = master_menu_list[pos].icon_resource;

					// (JEL09oct97) must clear this so
					// next click on exit becomes 1st
					// click again

					EXIT_CLICK_ID = 0;

					// refresh the menu
					Build_top_menu();

					Set_luggage(master_menu_list[pos].luggage_resource);
					// Zdebug("switch to drag mode");
				}
			}
		}
	}
}

void Normal_mouse(void) {
	// The gane is playing and none of the menus are activated - but, we
	// need to check if the top menu is to start. Note, wont have luggage

	_mouseEvent *me;

	//no save in big-object menu lock situation
	if (mousey < 0 && !menu_status && !mouse_mode_locked && !OBJECT_HELD) {
		mouse_mode = MOUSE_system_menu;

		if (mouse_touching) {
			// We were on something, but not anymore
			old_mouse_touching = 0;
			mouse_touching = 0;
		}

		// reset mouse cursor - in case we're between mice
		Set_mouse(NORMAL_MOUSE_ID);

		Build_system_menu();
	}

	if (mousey > 399 && !menu_status && !mouse_mode_locked) {
		// why are we testing for this?
		if (!OBJECT_HELD) {
			// bring down top menu
			mouse_mode = MOUSE_top;
		} else {
			mouse_mode = MOUSE_drag;
		}

		// if mouse is moving off an object and onto the top menu
		// then do a standard get-off

		if (mouse_touching) {
			// We were on something, but not anymore

			old_mouse_touching = 0;
			mouse_touching = 0;
		}

		// reset mouse cursor
		Set_mouse(NORMAL_MOUSE_ID);

		// build menu and start the menu coming down
		Build_top_menu();

		return;
	}

	// check also for bringing the bottom menu up
	Mouse_on_off();

	// now do the normal click stuff

	me = MouseEvent();

#ifdef _SWORD2_DEBUG
	if (definingRectangles)	{
		if (draggingRectangle == 0) {
			// not yet dragging a rectangle, so need click to start

			if (me && (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN))) {
				// set both (x1,y1) and (x2,y2) to this point
				rect_x1 = rect_x2 = (uint32) mousex + this_screen.scroll_offset_x;
				rect_y1 = rect_y2 = (uint32) mousey + this_screen.scroll_offset_y;
				draggingRectangle = 1;
			}
		} else if (draggingRectangle == 1) {
			// currently dragging a rectangle - click means reset

			if (me && (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN))) {
				// lock rectangle, so you can let go of mouse
				// to type in the coords
				draggingRectangle = 2;
			} else {
				// drag rectangle
				rect_x2 = (uint32) mousex + this_screen.scroll_offset_x;
				rect_y2 = (uint32) mousey + this_screen.scroll_offset_y;
			}
		} else {
			// currently locked to avoid knocking out of place
			// while reading off the coords

			if (me && (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN))) {
				// click means reset - back to start again
				draggingRectangle = 0;
			}
		}
	} else {
#endif

	// we only care about down clicks when the mouse is over an object
	// we ignore mouse releases

	if (me && (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN)) && mouse_touching) {
		// there's a mouse event to be processed and the mouse is on
		// something

		// ok, there are no menus about so its nice and simple
		// this is as close to the old advisor_188 script as we get
		// I'm sorry to say.

		// if player is walking or relaxing then those need to
		// terminate correctly

		// otherwise set player run the targets action script
		// or, do a special walk if clicking on the scroll-more icon

		// PLAYER_ACTION script variable - whatever catches this must
		// reset to 0 again
		// PLAYER_ACTION = mouse_touching;

		// idle or router-anim will catch it

		// set global script variable 'button'

		if (me->buttons & RD_LEFTBUTTONDOWN) {
			LEFT_BUTTON  = 1;
			RIGHT_BUTTON = 0;
			button_click = 0;	//for re-click
		} else {
			LEFT_BUTTON  = 0;
			RIGHT_BUTTON = 1;
			button_click = 1;	//for re-click
		}

		// these might be required by the action script about to be run
		MOUSE_X = (uint32) mousex + this_screen.scroll_offset_x;
		MOUSE_Y = (uint32) mousey + this_screen.scroll_offset_y;

		// only left button
		if (mouse_touching == EXIT_CLICK_ID && me->buttons & RD_LEFTBUTTONDOWN) {
			// its the exit double click situation
			// let the existing interaction continue and start
			// fading down - switch the human off too

			FN_no_human(NULL);
			FN_fade_down(NULL);
			EXIT_FADING = 1;	//tell the walker
		} else if (old_button == button_click && mouse_touching == CLICKED_ID && mouse_pointer_res != NORMAL_MOUSE_ID) {
			//re-click - do nothing - except on floors
		} else {
			// allow the click
			old_button = button_click;	//for re-click

			// for scripts to know what's been clicked (21jan97).
			// First used for 'room_13_turning_script' in object
			// 'biscuits_13'

			CLICKED_ID = mouse_touching;

			// must clear these two double-click control flags -
			// do it here so reclicks after exit clicks are
			// cleared up

			EXIT_CLICK_ID = 0;
			EXIT_FADING = 0;

			Set_player_action_event(CUR_PLAYER_ID, mouse_touching);	//Tony4Dec96

#ifdef _SWORD2_DEBUG
			// Write to walkthrough file (zebug0.txt)
			if (OBJECT_HELD)
				Zdebug(0, "USED \"%s\" ICON ON %s", FetchObjectName(OBJECT_HELD), FetchObjectName(CLICKED_ID));
			else if (LEFT_BUTTON)
				Zdebug(0, "LEFT-CLICKED ON %s", FetchObjectName(CLICKED_ID));
			else	// RIGHT BUTTON
				Zdebug(0, "RIGHT-CLICKED ON %s", FetchObjectName(CLICKED_ID));
#endif
		}
	}

#ifdef _SWORD2_DEBUG
	}
#endif
}

void Mouse_on_off(void) {
	// this handles the cursor graphic when moving on and off mouse areas
	// it also handles the luggage thingy

	uint32 pointer_type;
	static uint8 mouse_flicked_off = 0;

	old_mouse_touching = mouse_touching;

	// don't detect objects that are hidden behind the menu bars (ie. in
	// the scrolled-off areas of the screen)

	if (mousey < 0 || mousey > 399)	{	
		pointer_type = 0;
		mouse_touching = 0;
	} else {
		// set 'mouse_touching' & return pointer_type
		pointer_type = Check_mouse_list();
	}

	// same as previous cycle?
	if (!mouse_flicked_off && old_mouse_touching == mouse_touching) {
		// yes, so nothing to do
		// BUT CARRY ON IF MOUSE WAS FLICKED OFF!
		return;
	}

	// can reset this now
	mouse_flicked_off = 0;

	//the cursor has moved onto something
	if (!old_mouse_touching && mouse_touching) {
		// make a copy of the object we've moved onto because one day
		// we'll move back off again! (but the list positioning could
		// theoretically have changed)

		// we can only move onto something from being on nothing - we
		// stop the system going from one to another when objects
		// overlap

		// memcpy(&old_mouse_object, &mouse_list[mouse_touching], sizeof(Object_mouse));

		old_mouse_touching = mouse_touching;

		// run get on

		if (pointer_type) {
			// 'pointer_type' holds the resource id of the
			// pointer anim

			Set_mouse(pointer_type);

			// setup luggage icon
			if (OBJECT_HELD) {
				// Set_luggage(master_menu_list[menu_selected_pos].luggage_resource);
				Set_luggage(current_luggage_resource);
			}
		} else
			Con_fatal_error("ERROR: mouse.pointer==0 for object %d (%s) - update logic script!", mouse_touching, FetchObjectName(mouse_touching));
	} else if (old_mouse_touching && !mouse_touching) {
		// the cursor has moved off something - reset cursor to
		// normal pointer

		old_mouse_touching = 0;
		Set_mouse(NORMAL_MOUSE_ID);

		// reset luggage only when necessary
	} else if (old_mouse_touching && mouse_touching) {
		// The cursor has moved off something and onto something
		// else. Flip to a blank cursor for a cycle.

		// ignore the new id this cycle - should hit next cycle
		mouse_touching = 0;
		old_mouse_touching = 0;
		Set_mouse(0);

		// so we know to set the mouse pointer back to normal if 2nd
		// hot-spot doesn't register because mouse pulled away 
		// quickly (onto nothing)

		mouse_flicked_off = 1;
		
		// reset luggage only when necessary
	} else {
		// Mouse was flicked off for one cycle, but then moved onto
		// nothing before 2nd hot-spot registered

		// both 'old_mouse_touching' & 'mouse_touching' will be zero
		// reset cursor to normal pointer

		Set_mouse(NORMAL_MOUSE_ID);
	}

	// possible check for edge of screen more-to-scroll here on large
	// screens
}

void Set_mouse(uint32 res) {
	uint8 *icon;
	uint32 len;

	//high level - whats the mouse - for the engine
	mouse_pointer_res = res;

	if (res) {
		icon = res_man.Res_open(res) + sizeof(_standardHeader);
		len = res_man.resList[res]->size - sizeof(_standardHeader);

		// don't pulse the normal pointer - just do the regular anim
		// loop

		if (res == NORMAL_MOUSE_ID)
			SetMouseAnim(icon, len, RDMOUSE_NOFLASH);
		else
 			SetMouseAnim(icon, len, RDMOUSE_FLASH);

		res_man.Res_close(res);
	} else {
		// blank cursor
		SetMouseAnim(NULL, 0, 0);
	}
}

void Set_luggage(uint32	res) {
	uint8 *icon;
	uint32 len;

	real_luggage_item = res;

	if (res) {
		icon = res_man.Res_open(res) + sizeof(_standardHeader);
		len = res_man.resList[res]->size - sizeof(_standardHeader);

		SetLuggageAnim(icon, len);

		res_man.Res_close(res);
	} else
		SetLuggageAnim(NULL, 0);
}

uint32 Check_mouse_list(void) {
	int32 priority = 0;
	uint32 j = 1;

	if (cur_mouse > 1) {
		//number of priorities subject to implementation needs
		while (priority < 10) {
			// if the mouse pointer is over this
			// mouse-detection-box

			if (mouse_list[j].priority == priority &&
				mousex + this_screen.scroll_offset_x >= mouse_list[j].x1 &&
				mousex + this_screen.scroll_offset_x <= mouse_list[j].x2 &&
				mousey + this_screen.scroll_offset_y >= mouse_list[j].y1 &&
				mousey + this_screen.scroll_offset_y <= mouse_list[j].y2) {
				// record id
				mouse_touching = mouse_list[j].id;

				// change all COGS pointers to CROSHAIR
				if (mouse_list[j].pointer == USE)
					mouse_list[j].pointer = CROSHAIR;

				CreatePointerText(mouse_list[j].pointer_text, mouse_list[j].pointer);

				// return pointer type
				return(mouse_list[j].pointer);
			}

			j++;
			if (j == cur_mouse) {
				j = 0;
				// next priority - 0 being the highest, 9 the
				// lowest
				priority++;
			}
		}
	}

	mouse_touching = 0;	// touching nothing
	return 0;		// no pointer to return
}

#define POINTER_TEXT_WIDTH	640		// just in case!
#define POINTER_TEXT_PEN	184		// white

void CreatePointerText(uint32 textId, uint32 pointerRes) {
	uint32 local_text;
	uint32 text_res;
	uint8 *text;
	// offsets for pointer text sprite from pointer position
	int16 xOffset, yOffset;
	uint8 justification;

	if (pointerTextSelected) {
		if (textId) {
			// check what the pointer is, to set offsets
			// correctly for text position

			switch (pointerRes) {
			case CROSHAIR:
				yOffset = -7;
				xOffset = +10;
				break;
			case EXIT0:
				yOffset = +15;
				xOffset = +20;
				break;
			case EXIT1:
				yOffset = +16;
				xOffset = -10;
				break;
			case EXIT2:
				yOffset = +10;
				xOffset = -22;
				break;
			case EXIT3:
				yOffset = -16;
				xOffset = -10;
				break;
			case EXIT4:
				yOffset = -15;
				xOffset = +15;
				break;
			case EXIT5:
				yOffset = -12;
				xOffset = +10;
				break;
			case EXIT6:
				yOffset = +10;
				xOffset = +25;
				break;
			case EXIT7:
				yOffset = +16;
				xOffset = +20;
				break;
			case EXITDOWN:
				yOffset = -20;
				xOffset = -10;
				break;
			case EXITUP:
				yOffset = +20;
				xOffset = +20;
				break;
			case MOUTH:
				yOffset = -10;
				xOffset = +15;
				break;
			case NORMAL:
				yOffset = -10;
				xOffset = +15;
				break;
			case PICKUP:
				yOffset = -40;
				xOffset = +10;
				break;
			case SCROLL_L:
				yOffset = -20;
				xOffset = +20;
				break;
			case SCROLL_R:
				yOffset = -20;
				xOffset = -20;
				break;
			case USE:
				yOffset = -8;
				xOffset = +20;
				break;
			default:
				// shouldn't happen if we cover all
				// the different mouse pointers above
				yOffset = -10;
				xOffset = +10;
				break;
			}

			// set up justification for text sprite, based on its
			// offsets from the pointer position

			if (yOffset < 0) {
				// above pointer
				if (xOffset < 0) {
					// above left
					justification = POSITION_AT_RIGHT_OF_BASE;
				} else if (xOffset > 0) {
					// above right
					justification = POSITION_AT_LEFT_OF_BASE;
				} else {
					// above centre
					justification = POSITION_AT_CENTRE_OF_BASE;
				}
			} else if (yOffset > 0) {
				// below pointer
				if (xOffset < 0) {
					// below left
					justification = POSITION_AT_RIGHT_OF_TOP;
				} else if (xOffset > 0) {
					// below right
					justification = POSITION_AT_LEFT_OF_TOP;
				} else {
					// below centre
					justification = POSITION_AT_CENTRE_OF_TOP;
				}
			} else {
				// same y-coord as pointer
				if (xOffset < 0) {
					// centre left
					justification = POSITION_AT_RIGHT_OF_CENTRE;
				} else if (xOffset > 0) {
					// centre right
					justification = POSITION_AT_LEFT_OF_CENTRE;
				} else {
					// centre centre - shouldn't happen
					// anyway!
					justification = POSITION_AT_LEFT_OF_CENTRE;
				}
			}

			// text resource number, and line number within the
			// resource

			text_res = textId / SIZE;
			local_text = textId & 0xffff;

			// open text file & get the line
			text = FetchTextLine(res_man.Res_open(text_res), local_text);

			// 'text+2' to skip the first 2 bytes which form the
			// line reference number

			pointer_text_bloc_no = Build_new_block(
				text + 2, mousex + xOffset, mousey + yOffset,
				POINTER_TEXT_WIDTH, POINTER_TEXT_PEN,
				RDSPR_TRANS | RDSPR_DISPLAYALIGN,
				speech_font_id, justification);

			// now ok to close the text file
			res_man.Res_close(text_res);
		}
	}
}

void ClearPointerText(void) {
	if (pointer_text_bloc_no) {
		Kill_text_bloc(pointer_text_bloc_no);
		pointer_text_bloc_no = 0;
	}
}

int32 FN_no_human(int32 *params) {	// Tony30Sept96
	// param	none

	// for logic scripts
	MOUSE_AVAILABLE = 0;

	ClearPointerText();

	//human/mouse off
	mouse_status = 1;

	Set_mouse(0);
	Set_luggage(0);

	// must be normal mouse situation or a largely neutral situation -
	// special menus use No_human

	//dont hide menu in conversations
	if (TALK_FLAG == 0)
		HideMenu(RDMENU_BOTTOM);

	if (mouse_mode == MOUSE_system_menu) {
		// close menu
		mouse_mode = MOUSE_normal;
		HideMenu(RDMENU_TOP);
	}

	// script continue
	return IR_CONT;
}

void No_human(void) {
	// leaves the menus open
	// used by the system when clicking right on a menu item to examine
	// it and when combining objects

	// for logic scripts
	MOUSE_AVAILABLE = 0;

	// human/mouse off
	mouse_status = 1;

	Set_mouse(0);
	Set_luggage(0);
}

int32 FN_add_human(int32 *params) {
	// param	none

	// for logic scripts
	MOUSE_AVAILABLE = 1;

	//off
	if (mouse_status) {
		mouse_status = 0;	// on
		mouse_touching = 1;	// forces engine to choose a cursor
	}

	//clear this to reset no-second-click system
	CLICKED_ID = 0;

	// this is now done outside the OBJECT_HELD check in case it's set to
	// zero before now!

	// unlock the mouse from possible large object lock situtations - see
	// syphon in rm 3

	mouse_mode_locked = 0;

	if (OBJECT_HELD) {
		// was dragging something around
		// need to clear this again
		OBJECT_HELD = 0;

		// and these may also need clearing, just in case
		examining_menu_icon = 0;
		COMBINE_BASE = 0;

		Set_luggage(0);
	}

	// if mouse is over menu area
	if (mousey > 399) {
		if (mouse_mode != MOUSE_holding) {
			// VITAL - reset things & rebuild the menu
			mouse_mode = MOUSE_normal;
			Set_mouse(NORMAL_MOUSE_ID);
		} else
			Set_mouse(NORMAL_MOUSE_ID);
	}

	// enabled/disabled from console; status printed with on-screen debug
	// info

#ifdef _SWORD2_DEBUG
	uint8 black[4] = {   0,  0,    0,   0 };
	uint8 white[4] = { 255, 255, 255,   0 };

	if (testingSnR) {
		// testing logic scripts by simulating an instant Save &
		// Restore

		BS2_SetPalette(0, 1, white, RDPAL_INSTANT);

		// stops all fx & clears the queue - eg. when leaving a
		// location

		Clear_fx_queue();

		// Trash all object resources so they load in fresh & restart
		// their logic scripts

		res_man.Kill_all_objects(0);

		BS2_SetPalette(0, 1, black, RDPAL_INSTANT);
	}
#endif

	return IR_CONT;
}

int32 FN_register_mouse(int32 *params) {
	// this call would be made from an objects service script 0
	// the object would be one with no graphic but with a mouse - i.e. a
	// floor or one whose mouse area is manually defined rather than
	// intended to fit sprite shape

	// param	0 pointer to Object_mouse or 0 for no write to mouse
	//		  list

	// Zdebug(1, "cur_mouse = %d", cur_mouse);

	Object_mouse *ob_mouse = (Object_mouse *) params[0];

	// only if 'pointer' isn't NULL
	if (ob_mouse->pointer) {
#ifdef _SWORD2_DEBUG
		if (cur_mouse == TOTAL_mouse_list)
			Con_fatal_error("ERROR: mouse_list full [%s line %u]", __FILE__, __LINE__);
#endif

		mouse_list[cur_mouse].x1 = ob_mouse->x1;
		mouse_list[cur_mouse].y1 = ob_mouse->y1;
		mouse_list[cur_mouse].x2 = ob_mouse->x2;
		mouse_list[cur_mouse].y2 = ob_mouse->y2;

		mouse_list[cur_mouse].priority = ob_mouse->priority;
		mouse_list[cur_mouse].pointer = ob_mouse->pointer;

		// (James17jun97) check if pointer text field is set due to
		// previous object using this slot (ie. not correct for this
		// one)

		// if 'pointer_text' field is set, but the 'id' field isn't
		// same is current id, then we don't want this "left over"
		// pointer text

		if (mouse_list[cur_mouse].pointer_text && mouse_list[cur_mouse].id != (int32) ID)
			mouse_list[cur_mouse].pointer_text = 0;

		// get id from system variable 'id' which is correct for
		// current object

		mouse_list[cur_mouse].id = ID;

		// not using sprite as mask - this is only done from
		// FN_register_frame()

		mouse_list[cur_mouse].anim_resource = 0;
		mouse_list[cur_mouse].anim_pc = 0;

		// Zdebug("mouse id %d", mouse_list[cur_mouse].id);
		cur_mouse++;
	}

	return IR_CONT;
}

// use this in the object's service script prior to registering the mouse area
// ie. before FN_register_mouse or FN_register_frame
// - best if kept at very top of service script

int32 FN_register_pointer_text(int32 *params) {
	// param	0 local id of text line to use as pointer text

#ifdef _SWORD2_DEBUG
	if (cur_mouse == TOTAL_mouse_list)
		Con_fatal_error("ERROR: mouse_list full [%s line %u]", __FILE__, __LINE__);
#endif

	// current object id - used for checking pointer_text when mouse area
	// registered (in FN_register_mouse & FN_register_frame)

	mouse_list[cur_mouse].id = ID;
	mouse_list[cur_mouse].pointer_text = params[0];

	return IR_CONT;
}

int32 FN_blank_mouse(int32 *params) {
	//set mouse to normal pointer - used in speech
	//no params

	Set_mouse(0);
	return IR_CONT;
}

int32 FN_init_floor_mouse(int32 *params) {
	// params	0 pointer to object's mouse structure

 	Object_mouse *ob_mouse = (Object_mouse *) params[0];

	// floor is always lowest priority

	ob_mouse->x1 = 0;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = this_screen.screen_wide - 1;
	ob_mouse->y2 = this_screen.screen_deep - 1;
	ob_mouse->priority = 9;
	ob_mouse->pointer = NORMAL_MOUSE_ID;

	return IR_CONT;
}

#define SCROLL_MOUSE_WIDTH 20

int32 FN_set_scroll_left_mouse(int32 *params) {
	// params	0 pointer to object's mouse structure

 	Object_mouse	*ob_mouse = (Object_mouse *) params[0];

	// Highest priority

	ob_mouse->x1 = 0;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = this_screen.scroll_offset_x + SCROLL_MOUSE_WIDTH;
	ob_mouse->y2 = this_screen.screen_deep - 1;
	ob_mouse->priority = 0;

	if (this_screen.scroll_offset_x > 0) {
		// not fully scrolled to the left
		ob_mouse->pointer = SCROLL_LEFT_MOUSE_ID;
	} else {
		// so the mouse area doesn't get registered
		ob_mouse->pointer = 0;
	}

	return IR_CONT;
}

int32 FN_set_scroll_right_mouse(int32 *params) {
	// params	0 pointer to object's mouse structure

 	Object_mouse	*ob_mouse = (Object_mouse *) params[0];

	// Highest priority

	ob_mouse->x1 = this_screen.scroll_offset_x + screenWide - SCROLL_MOUSE_WIDTH;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = this_screen.screen_wide - 1;
	ob_mouse->y2 = this_screen.screen_deep - 1;
	ob_mouse->priority = 0;

	if (this_screen.scroll_offset_x < this_screen.max_scroll_offset_x) {
		// not fully scrolled to the right
		ob_mouse->pointer = SCROLL_RIGHT_MOUSE_ID;
	} else {
		// so the mouse area doesn't get registered
		ob_mouse->pointer = 0;
	}

	return IR_CONT;
}

int32 FN_set_object_held(int32 *params) {
	// params	0 luggage icon to set

	Set_luggage(params[0]);

	OBJECT_HELD = params[0];
	current_luggage_resource = params[0];

	// mode locked - no top menu available
	mouse_mode_locked = 1;

	return IR_CONT;
}

// called from speech scripts to remove the chooser bar when it's not
// appropriate to keep it displayed

int32 FN_remove_chooser(int32 *params) {
	HideMenu(RDMENU_BOTTOM);
	return(IR_CONT);
}

int32 FN_disable_menu(int32 *params) {
	// mode locked - no top menu available
	mouse_mode_locked = 1;
	mouse_mode = MOUSE_normal;

	HideMenu(RDMENU_TOP);
	HideMenu(RDMENU_BOTTOM);

	return IR_CONT;
}

int32 FN_enable_menu(int32 *params) {
	//mode locked - no top menu available
	mouse_mode_locked = 0;
	return IR_CONT;
}

int32 FN_check_player_activity(int32 *params) {
	// Used to decide when to trigger music cues described as "no player
	// activity for a while"

	// params	0 threshold delay in seconds, ie. what we want to
	//		  check the actual delay against

	uint32 threshold = params[0] * 12;	// in game cycles

	// if the actual delay is at or above the given threshold
	if (player_activity_delay >= threshold) {
		// reset activity delay counter, now that we've got a
		// positive check

		player_activity_delay = 0;
		RESULT = 1;
	} else
		RESULT = 0;

	return IR_CONT;
}

int32 FN_reset_player_activity_delay(int32 *params) {
	// Use if you want to deliberately reset the "no player activity"
	// counter for any reason
	// no params

	player_activity_delay = 0;
	return IR_CONT;
}

void Monitor_player_activity(void) {
	// if there is at least one mouse event outstanding
	if (CheckForMouseEvents()) {
		// reset activity delay counter
		player_activity_delay = 0;
	} else {
		// no. of game cycles since mouse event queue last empty
		player_activity_delay++;
	}
}
