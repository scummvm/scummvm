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
#include "sword2/driver/driver96.h"
#include "sword2/build_display.h"
#include "sword2/console.h"
#include "sword2/controls.h"
#include "sword2/defs.h"
#include "sword2/events.h"
#include "sword2/icons.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/layers.h"
#include "sword2/maketext.h"
#include "sword2/mouse.h"
#include "sword2/object.h"
#include "sword2/protocol.h"
#include "sword2/resman.h"
#include "sword2/sound.h"
#include "sword2/sword2.h"

namespace Sword2 {

// pointer resource id's

enum {
	CROSHAIR	= 18,
	EXIT0		= 788,
	EXIT1		= 789,
	EXIT2		= 790,
	EXIT3		= 791,
	EXIT4		= 792,
	EXIT5		= 793,
	EXIT6		= 794,
	EXIT7		= 795,
	EXITDOWN	= 796,
	EXITUP		= 797,
	MOUTH		= 787,
	NORMAL		= 17,
	PICKUP		= 3099,
	SCROLL_L	= 1440,
	SCROLL_R	= 1441,
	USE		= 3100
};

void Sword2Engine::resetMouseList(void) {
	// call at beginning of gameloop
	_curMouse = 1;
}

/**
 * This function is called every game cycle.
 */

void Sword2Engine::mouseEngine(void) {
	monitorPlayerActivity();
	clearPointerText();

	// If George is dead, the system menu is visible all the time, and is
	// the only thing that can be used.

	if (DEAD) {
		if (_mouseMode != MOUSE_system_menu) {
			_mouseMode = MOUSE_system_menu;

			if (_mouseTouching) {
				_oldMouseTouching = 0;
				_mouseTouching = 0;
			}

			setMouse(NORMAL_MOUSE_ID);
			buildSystemMenu();
		}
		systemMenuMouse();
		return;
	}

	// If the mouse is not visible, do nothing

	if (_mouseStatus)
		return;

	switch (_mouseMode) {
	case MOUSE_normal:
		normalMouse();
		break;
	case MOUSE_menu:
		menuMouse();
		break;
	case MOUSE_drag:
		dragMouse();
		break;
	case MOUSE_system_menu:
		systemMenuMouse();
		break;
	case MOUSE_holding:
		if (g_display->_mouseY < 400) {
			_mouseMode = MOUSE_normal;
			debug(5, "   releasing");
		}
		break;
	default:
		break;
	}
}

void Sword2Engine::systemMenuMouse(void) {
	uint32 safe_looping_music_id;
	_mouseEvent *me;
	int j, hit;
	uint8 *icon;
	int32 pars[2];
	uint32 icon_list[5] = {
		OPTIONS_ICON,
		QUIT_ICON,
		SAVE_ICON,
		RESTORE_ICON,
		RESTART_ICON
	};

	// can't close when player is dead
	if (g_display->_mouseY > 0 && !DEAD) {
		// close menu
		_mouseMode = MOUSE_normal;
		g_display->hideMenu(RDMENU_TOP);
		return;
	}

	me = MouseEvent();

	if (me && (me->buttons & RD_LEFTBUTTONDOWN)) {
		// clicked on a top mouse pointer?

		if (g_display->_mouseX >= 24 && g_display->_mouseX < 640 - 24 && g_display->_mouseY < 0) {
			// which are we over?
			hit = (g_display->_mouseX - 24) / 40;

			// no save when dead
			if (icon_list[hit] == SAVE_ICON && DEAD)
				return;

			// there are 5 system menu icons
			if (hit < ARRAYSIZE(icon_list)) {
				// build them all high in full colour - when
				// one is clicked on all the rest will grey out
				for (j = 0; j < ARRAYSIZE(icon_list); j++) {
					// change all others to grey
					if (j != hit) {
						icon = res_man.open(icon_list[j]) + sizeof(_standardHeader);
						g_display->setMenuIcon(RDMENU_TOP, j, icon);
						res_man.close( icon_list[j] );
					}
				}

				g_sound->pauseFx();

				// NB. Need to keep a safe copy of
				// 'looping_music_id' for savegame & for
				// playing when returning from control panels
				// because control panel music will overwrite
				// it!

				safe_looping_music_id = looping_music_id;

				pars[0] = 221;	// SystemM234 (M234.wav)
				pars[1] = FX_LOOP;
				g_logic.fnPlayMusic(pars);

				// restore proper looping_music_id
				looping_music_id = safe_looping_music_id;

				// clear the screen & set up the new palette
				// for the menus

				g_display->clearScene();
				g_display->processMenu();
				g_display->resetRenderEngine();

				// call the relevent screen
				switch (hit) {
				case 0:
					gui->optionControl();
					break;
				case 1:
					gui->quitControl();
					break;
				case 2:
					gui->saveControl();
					break;
				case 3:
					gui->restoreControl();
					break;
				case 4:
					gui->restartControl();
					break;
				}

				// Menu stays open on death screen
				if (!DEAD) {
					_mouseMode = MOUSE_normal;
					g_display->hideMenu(RDMENU_TOP);
				} else {
					setMouse(NORMAL_MOUSE_ID);
					buildSystemMenu();
				}

				// clear the screen & restore the location
				// palette

				g_display->clearScene();
				g_display->processMenu();

				// reset game palette, but not after a
				// successful restore or restart!

				// see RestoreFromBuffer() in save_rest.cpp
				if (_thisScreen.new_palette != 99) {
					// '0' means put back game screen
					// palette; see Build_display.cpp

					setFullPalette(0);

					// stop the engine fading in the
					// restored screens palette

					_thisScreen.new_palette = 0;
				} else
					_thisScreen.new_palette = 1;

				g_sound->unpauseFx();

				// If there was looping music before coming
				// into the control panels then restart it!
				// NB. This will also start music required
				// when a game has been restored

				if (looping_music_id) {
					pars[0] = looping_music_id;
					pars[1] = FX_LOOP;
					g_logic.fnPlayMusic(pars);

					// cross-fades into the required music:
					// - either a restored game tune
					// - or music playing prior to
					// entering control panels
				} else {
					// stop the control panel music
					g_logic.fnStopMusic(NULL);
				}
			}
		}
	}
}

void Sword2Engine::dragMouse(void) {
	_mouseEvent *me;
	uint32 pos;

	if (g_display->_mouseY < 400) {
		// close menu
		_mouseMode = MOUSE_normal;
		g_display->hideMenu(RDMENU_BOTTOM);
		return;
	}

	// handles cursors and the luggage on/off according to type
	mouseOnOff();

	// now do the normal click stuff

	me = MouseEvent();

	// we only care about left clicks when the mouse is over an object
	// we ignore mouse releases

	if (me && (me->buttons & RD_LEFTBUTTONDOWN)) {
		// there's a mouse event to be processed

		// could be clicking on an on screen object or on the menu
		// which is currently displayed

		if (_mouseTouching) {
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

			MOUSE_X = (uint32) g_display->_mouseX + _thisScreen.scroll_offset_x;
			MOUSE_Y = (uint32) g_display->_mouseY + _thisScreen.scroll_offset_y;

			// for scripts to know what's been clicked (21jan97).
			// First used for 'room_13_turning_script' in object
			// 'biscuits_13'

			CLICKED_ID = _mouseTouching;

			setPlayerActionEvent(CUR_PLAYER_ID, _mouseTouching);

			debug(5, "USED \"%s\" ICON ON %s", FetchObjectName(OBJECT_HELD), FetchObjectName(CLICKED_ID));

			// Hide menu - back to normal menu mode

			g_display->hideMenu(RDMENU_BOTTOM);
			_mouseMode = MOUSE_normal;
		} else {
			// better check for combine/cancel
			// cancel puts us back in Menu_mouse mode
			if (g_display->_mouseX >= 24 && g_display->_mouseX < 640 - 24) {
				// which are we over?
				pos = (g_display->_mouseX - 24) / 40;

				//clicked on something - what button?
				if (_masterMenuList[pos].icon_resource) {
					// always back into menu mode
					_mouseMode = MOUSE_menu;

					// remove luggage
					setLuggage(0);

					// if we've clicked on the same icon
					// as the one we're dragging

					if (pos == _menuSelectedPos) {
						// reset first icon
						OBJECT_HELD = 0;
						_menuSelectedPos = 0;
					} else {
						// combine the 2 icons
						debug(5, "combine");

						// what we clicked on, not what
						// we're dragging

						COMBINE_BASE = _masterMenuList[pos].icon_resource;
						setPlayerActionEvent(CUR_PLAYER_ID, MENU_MASTER_OBJECT);

						// turn off mouse now, to
						// prevent player trying to
						// click elsewhere BUT leave
						// the bottom menu open

						noHuman();

						debug(5, "USED \"%s\" ICON ON \"%s\" ICON", FetchObjectName(OBJECT_HELD), FetchObjectName(COMBINE_BASE));
					}

					// refresh the menu
					buildMenu();
					debug(5, "switch to menu mode");
				}
			}
		}
	}
}

void Sword2Engine::menuMouse(void) {
	// menu is down

	_mouseEvent *me;
	uint32 pos;

	if (g_display->_mouseY < 400) {
		// close menu
		_mouseMode = MOUSE_normal;
		g_display->hideMenu(RDMENU_BOTTOM);
		return;
	}

	me = MouseEvent();

	// we only care about left clicks when the mouse is over an object
	// we ignore mouse releases

	if (me) {
		// there's a mouse event to be processed
		// now check if we've clicked on an actual icon

		if (g_display->_mouseX >= 24 && g_display->_mouseX < 640 - 24) {
			// which are we over?
			pos = (g_display->_mouseX - 24) / 40;

			// clicked on something - what button?
			if (_masterMenuList[pos].icon_resource) {
				if (me->buttons & RD_RIGHTBUTTONDOWN) {
					// right button look
					_examiningMenuIcon = true;

					// id the object via its graphic
					OBJECT_HELD = _masterMenuList[pos].icon_resource;

					// Must clear this so next click on
					// exit becomes 1st click again

					EXIT_CLICK_ID = 0;

					debug(5, "RIGHT-CLICKED ON \"%s\" ICON", FetchObjectName(OBJECT_HELD));

					setPlayerActionEvent(CUR_PLAYER_ID, MENU_MASTER_OBJECT);

					// refresh the menu
					buildMenu();

					// turn off mouse now, to prevent
					// player trying to click elsewhere
					// BUT leave the bottom menu open

					noHuman();
				} else if (me->buttons & RD_LEFTBUTTONDOWN) {
					// left button - highlight the object
					// and bung us into drag luggage mode

					// menu slot we clicked on - derive
					// luggage resource from this in
					// mouse_on_off()

					_menuSelectedPos = pos;
					_currentLuggageResource = _masterMenuList[pos].luggage_resource;

					_mouseMode = MOUSE_drag;
					debug(5, "setting OH in menu");

					// id the object via its graphic
					OBJECT_HELD = _masterMenuList[pos].icon_resource;

					// must clear this so next click on
					// exit becomes 1st click again

					EXIT_CLICK_ID = 0;

					// refresh the menu
					buildMenu();

					setLuggage(_masterMenuList[pos].luggage_resource);
					debug(5, "switch to drag mode");
				}
			}
		}
	}
}

void Sword2Engine::normalMouse(void) {
	// The gane is playing and none of the menus are activated - but, we
	// need to check if a menu is to start. Note, won't have luggage

	_mouseEvent *me;

	// no save in big-object menu lock situation
	if (g_display->_mouseY < 0 && !_mouseModeLocked && !OBJECT_HELD) {
		_mouseMode = MOUSE_system_menu;

		if (_mouseTouching) {
			// We were on something, but not anymore
			_oldMouseTouching = 0;
			_mouseTouching = 0;
		}

		// reset mouse cursor - in case we're between mice
		setMouse(NORMAL_MOUSE_ID);
		buildSystemMenu();
		return;
	}

	if (g_display->_mouseY > 399 && !_mouseModeLocked) {
		// If an object is being held, i.e. if the mouse cursor has a
		// luggage, we should be use dragging mode instead of inventory
		// menu mode.
		//
		// That way, we can still use an object on another inventory
		// object, even if the inventory menu was closed after the
		// first object was selected.

		if (!OBJECT_HELD)
			_mouseMode = MOUSE_menu;
		else
			_mouseMode = MOUSE_drag;

		// If mouse is moving off an object and onto the menu then do a
		// standard get-off

		if (_mouseTouching) {
			// We were on something, but not anymore

			_oldMouseTouching = 0;
			_mouseTouching = 0;
		}

		// reset mouse cursor
		setMouse(NORMAL_MOUSE_ID);
		buildMenu();
		return;
	}

	// check for moving the mouse on or off things

	mouseOnOff();

	// now do the normal click stuff

	me = MouseEvent();

	if (_debugger->_definingRectangles) {
		if (_debugger->_draggingRectangle == 0) {
			// not yet dragging a rectangle, so need click to start

			if (me && (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN))) {
				// set both (x1,y1) and (x2,y2) to this point
				_debugger->_rectX1 = _debugger->_rectX2 = (uint32) g_display->_mouseX + _thisScreen.scroll_offset_x;
				_debugger->_rectY1 = _debugger->_rectY2 = (uint32) g_display->_mouseY + _thisScreen.scroll_offset_y;
				_debugger->_draggingRectangle = 1;
			}
		} else if (_debugger->_draggingRectangle == 1) {
			// currently dragging a rectangle - click means reset

			if (me && (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN))) {
				// lock rectangle, so you can let go of mouse
				// to type in the coords
				_debugger->_draggingRectangle = 2;
			} else {
				// drag rectangle
				_debugger->_rectX2 = (uint32) g_display->_mouseX + _thisScreen.scroll_offset_x;
				_debugger->_rectY2 = (uint32) g_display->_mouseY + _thisScreen.scroll_offset_y;
			}
		} else {
			// currently locked to avoid knocking out of place
			// while reading off the coords

			if (me && (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN))) {
				// click means reset - back to start again
				_debugger->_draggingRectangle = 0;
			}
		}
	} else {
		// We only care about down clicks when the mouse is over an
		// object. We ignore mouse releases

		if (me && (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN)) && _mouseTouching) {
			// there's a mouse event to be processed and the mouse
			// is on something

			// ok, there are no menus about so its nice and simple
			// this is as close to the old advisor_188 script as we
			// get, I'm sorry to say.

			// if player is walking or relaxing then those need to
			// terminate correctly

			// otherwise set player run the targets action script
			// or, do a special walk if clicking on the scroll-more
			// icon

			// PLAYER_ACTION script variable - whatever catches
			// this must reset to 0 again
			// PLAYER_ACTION = _mouseTouching;

			// idle or router-anim will catch it

			// set global script variable 'button'

			if (me->buttons & RD_LEFTBUTTONDOWN) {
				LEFT_BUTTON  = 1;
				RIGHT_BUTTON = 0;
				_buttonClick = 0;	// for re-click
			} else {
				LEFT_BUTTON  = 0;
				RIGHT_BUTTON = 1;
				_buttonClick = 1;	// for re-click
			}

			// these might be required by the action script about
			// to be run
			MOUSE_X = (uint32) g_display->_mouseX + _thisScreen.scroll_offset_x;
			MOUSE_Y = (uint32) g_display->_mouseY + _thisScreen.scroll_offset_y;

			// only left button
			if (_mouseTouching == EXIT_CLICK_ID && (me->buttons & RD_LEFTBUTTONDOWN)) {
				// its the exit double click situation
				// let the existing interaction continue and
				// start fading down - switch the human off too

				g_logic.fnNoHuman(NULL);
				g_logic.fnFadeDown(NULL);
				EXIT_FADING = 1;	// tell the walker
			} else if (_oldButton == _buttonClick && _mouseTouching == CLICKED_ID && _mousePointerRes != NORMAL_MOUSE_ID) {
				// re-click - do nothing - except on floors
			} else {
				// allow the click
				_oldButton = _buttonClick;	// for re-click

				// for scripts to know what's been clicked
				// First used for 'room_13_turning_script' in
				// object 'biscuits_13'

				CLICKED_ID = _mouseTouching;

				// must clear these two double-click control
				// flags - do it here so reclicks after exit
				// clicks are cleared up

				EXIT_CLICK_ID = 0;
				EXIT_FADING = 0;

				setPlayerActionEvent(CUR_PLAYER_ID, _mouseTouching);

				if (OBJECT_HELD)
					debug(5, "USED \"%s\" ICON ON %s", FetchObjectName(OBJECT_HELD), FetchObjectName(CLICKED_ID));
				else if (LEFT_BUTTON)
					debug(5, "LEFT-CLICKED ON %s", FetchObjectName(CLICKED_ID));
				else	// RIGHT BUTTON
					debug(5, "RIGHT-CLICKED ON %s", FetchObjectName(CLICKED_ID));
			}
		}
	}
}

void Sword2Engine::mouseOnOff(void) {
	// this handles the cursor graphic when moving on and off mouse areas
	// it also handles the luggage thingy

	uint32 pointer_type;
	static uint8 mouse_flicked_off = 0;

	_oldMouseTouching = _mouseTouching;

	// don't detect objects that are hidden behind the menu bars (ie. in
	// the scrolled-off areas of the screen)

	if (g_display->_mouseY < 0 || g_display->_mouseY > 399)	{	
		pointer_type = 0;
		_mouseTouching = 0;
	} else {
		// set '_mouseTouching' & return pointer_type
		pointer_type = checkMouseList();
	}

	// same as previous cycle?
	if (!mouse_flicked_off && _oldMouseTouching == _mouseTouching) {
		// yes, so nothing to do
		// BUT CARRY ON IF MOUSE WAS FLICKED OFF!
		return;
	}

	// can reset this now
	mouse_flicked_off = 0;

	//the cursor has moved onto something
	if (!_oldMouseTouching && _mouseTouching) {
		// make a copy of the object we've moved onto because one day
		// we'll move back off again! (but the list positioning could
		// theoretically have changed)

		// we can only move onto something from being on nothing - we
		// stop the system going from one to another when objects
		// overlap

		_oldMouseTouching = _mouseTouching;

		// run get on

		if (pointer_type) {
			// 'pointer_type' holds the resource id of the
			// pointer anim

			setMouse(pointer_type);

			// setup luggage icon
			if (OBJECT_HELD) {
				setLuggage(_currentLuggageResource);
			}
		} else
			error("ERROR: mouse.pointer==0 for object %d (%s) - update logic script!", _mouseTouching, FetchObjectName(_mouseTouching));
	} else if (_oldMouseTouching && !_mouseTouching) {
		// the cursor has moved off something - reset cursor to
		// normal pointer

		_oldMouseTouching = 0;
		setMouse(NORMAL_MOUSE_ID);

		// reset luggage only when necessary
	} else if (_oldMouseTouching && _mouseTouching) {
		// The cursor has moved off something and onto something
		// else. Flip to a blank cursor for a cycle.

		// ignore the new id this cycle - should hit next cycle
		_mouseTouching = 0;
		_oldMouseTouching = 0;
		setMouse(0);

		// so we know to set the mouse pointer back to normal if 2nd
		// hot-spot doesn't register because mouse pulled away 
		// quickly (onto nothing)

		mouse_flicked_off = 1;
		
		// reset luggage only when necessary
	} else {
		// Mouse was flicked off for one cycle, but then moved onto
		// nothing before 2nd hot-spot registered

		// both '_oldMouseTouching' & '_mouseTouching' will be zero
		// reset cursor to normal pointer

		setMouse(NORMAL_MOUSE_ID);
	}

	// possible check for edge of screen more-to-scroll here on large
	// screens
}

void Sword2Engine::setMouse(uint32 res) {
	uint8 *icon;
	uint32 len;

	// high level - whats the mouse - for the engine
	_mousePointerRes = res;

	if (res) {
		icon = res_man.open(res) + sizeof(_standardHeader);
		len = res_man._resList[res]->size - sizeof(_standardHeader);

		// don't pulse the normal pointer - just do the regular anim
		// loop

		if (res == NORMAL_MOUSE_ID)
			g_display->setMouseAnim(icon, len, RDMOUSE_NOFLASH);
		else
 			g_display->setMouseAnim(icon, len, RDMOUSE_FLASH);

		res_man.close(res);
	} else {
		// blank cursor
		g_display->setMouseAnim(NULL, 0, 0);
	}
}

void Sword2Engine::setLuggage(uint32 res) {
	uint8 *icon;
	uint32 len;

	_realLuggageItem = res;

	if (res) {
		icon = res_man.open(res) + sizeof(_standardHeader);
		len = res_man._resList[res]->size - sizeof(_standardHeader);

		g_display->setLuggageAnim(icon, len);

		res_man.close(res);
	} else
		g_display->setLuggageAnim(NULL, 0);
}

uint32 Sword2Engine::checkMouseList(void) {
	int32 priority = 0;
	uint32 j = 1;

	if (_curMouse > 1) {
		//number of priorities subject to implementation needs
		while (priority < 10) {
			// if the mouse pointer is over this
			// mouse-detection-box

			if (_mouseList[j].priority == priority &&
				g_display->_mouseX + _thisScreen.scroll_offset_x >= _mouseList[j].x1 &&
				g_display->_mouseX + _thisScreen.scroll_offset_x <= _mouseList[j].x2 &&
				g_display->_mouseY + _thisScreen.scroll_offset_y >= _mouseList[j].y1 &&
				g_display->_mouseY + _thisScreen.scroll_offset_y <= _mouseList[j].y2) {
				// record id
				_mouseTouching = _mouseList[j].id;

				// change all COGS pointers to CROSHAIR
				if (_mouseList[j].pointer == USE)
					_mouseList[j].pointer = CROSHAIR;

				createPointerText(_mouseList[j].pointer_text, _mouseList[j].pointer);

				// return pointer type
				return _mouseList[j].pointer;
			}

			j++;
			if (j == _curMouse) {
				j = 0;
				// next priority - 0 being the highest, 9 the
				// lowest
				priority++;
			}
		}
	}

	_mouseTouching = 0;	// touching nothing
	return 0;		// no pointer to return
}

#define POINTER_TEXT_WIDTH	640		// just in case!
#define POINTER_TEXT_PEN	184		// white

void Sword2Engine::createPointerText(uint32 text_id, uint32 pointer_res) {
	uint32 local_text;
	uint32 text_res;
	uint8 *text;
	// offsets for pointer text sprite from pointer position
	int16 xOffset, yOffset;
	uint8 justification;

	if (gui->_pointerTextSelected) {
		if (text_id) {
			// check what the pointer is, to set offsets
			// correctly for text position

			switch (pointer_res) {
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
				// shouldn't happen if we cover all the
				// different mouse pointers above
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

			text_res = text_id / SIZE;
			local_text = text_id & 0xffff;

			// open text file & get the line
			text = FetchTextLine(res_man.open(text_res), local_text);

			// 'text+2' to skip the first 2 bytes which form the
			// line reference number

			_pointerTextBlocNo = fontRenderer.buildNewBloc(
				text + 2, g_display->_mouseX + xOffset,
				g_display->_mouseY + yOffset,
				POINTER_TEXT_WIDTH, POINTER_TEXT_PEN,
				RDSPR_TRANS | RDSPR_DISPLAYALIGN,
				_speechFontId, justification);

			// now ok to close the text file
			res_man.close(text_res);
		}
	}
}

void Sword2Engine::clearPointerText(void) {
	if (_pointerTextBlocNo) {
		fontRenderer.killTextBloc(_pointerTextBlocNo);
		_pointerTextBlocNo = 0;
	}
}

void Sword2Engine::noHuman(void) {
	// leaves the menus open
	// used by the system when clicking right on a menu item to examine
	// it and when combining objects

	// for logic scripts
	MOUSE_AVAILABLE = 0;

	// human/mouse off
	_mouseStatus = true;

	setMouse(0);
	setLuggage(0);
}

void Sword2Engine::registerMouse(Object_mouse *ob_mouse) {
	debug(5, "_curMouse = %d", _curMouse);

	// only if 'pointer' isn't NULL
	if (ob_mouse->pointer) {
		assert(_curMouse < TOTAL_mouse_list);

		_mouseList[_curMouse].x1 = ob_mouse->x1;
		_mouseList[_curMouse].y1 = ob_mouse->y1;
		_mouseList[_curMouse].x2 = ob_mouse->x2;
		_mouseList[_curMouse].y2 = ob_mouse->y2;

		_mouseList[_curMouse].priority = ob_mouse->priority;
		_mouseList[_curMouse].pointer = ob_mouse->pointer;

		// Check if pointer text field is set due to previous object
		// using this slot (ie. not correct for this one)

		// if 'pointer_text' field is set, but the 'id' field isn't
		// same is current id, then we don't want this "left over"
		// pointer text

		if (_mouseList[_curMouse].pointer_text && _mouseList[_curMouse].id != (int32) ID)
			_mouseList[_curMouse].pointer_text = 0;

		// get id from system variable 'id' which is correct for
		// current object

		_mouseList[_curMouse].id = ID;

		// not using sprite as mask - this is only done from
		// fnRegisterFrame()

		_mouseList[_curMouse].anim_resource = 0;
		_mouseList[_curMouse].anim_pc = 0;

		debug(5, "mouse id %d", _mouseList[_curMouse].id);
		_curMouse++;
	}
}

void Sword2Engine::monitorPlayerActivity(void) {
	// if there is at least one mouse event outstanding
	if (CheckForMouseEvents()) {
		// reset activity delay counter
		_playerActivityDelay = 0;
	} else {
		// no. of game cycles since mouse event queue last empty
		_playerActivityDelay++;
	}
}

int32 Logic::fnNoHuman(int32 *params) {
	// params:	none

	g_sword2->noHuman();
	g_sword2->clearPointerText();

	// must be normal mouse situation or a largely neutral situation -
	// special menus use noHuman

	// dont hide menu in conversations
	if (TALK_FLAG == 0)
		g_display->hideMenu(RDMENU_BOTTOM);

	if (g_sword2->_mouseMode == MOUSE_system_menu) {
		// close menu
		g_sword2->_mouseMode = MOUSE_normal;
		g_display->hideMenu(RDMENU_TOP);
	}

	// script continue
	return IR_CONT;
}

int32 Logic::fnAddHuman(int32 *params) {
	// params:	none

	// for logic scripts
	MOUSE_AVAILABLE = 1;

	// off
	if (g_sword2->_mouseStatus) {
		g_sword2->_mouseStatus = false;	// on
		g_sword2->_mouseTouching = 1;	// forces engine to choose a cursor
	}

	//clear this to reset no-second-click system
	CLICKED_ID = 0;

	// this is now done outside the OBJECT_HELD check in case it's set to
	// zero before now!

	// unlock the mouse from possible large object lock situtations - see
	// syphon in rm 3

	g_sword2->_mouseModeLocked = false;

	if (OBJECT_HELD) {
		// was dragging something around
		// need to clear this again
		OBJECT_HELD = 0;

		// and these may also need clearing, just in case
		g_sword2->_examiningMenuIcon = false;
		COMBINE_BASE = 0;

		g_sword2->setLuggage(0);
	}

	// if mouse is over menu area
	if (g_display->_mouseY > 399) {
		if (g_sword2->_mouseMode != MOUSE_holding) {
			// VITAL - reset things & rebuild the menu
			g_sword2->_mouseMode = MOUSE_normal;
			g_sword2->setMouse(NORMAL_MOUSE_ID);
		} else
			g_sword2->setMouse(NORMAL_MOUSE_ID);
	}

	// enabled/disabled from console; status printed with on-screen debug
	// info

	if (g_sword2->_debugger->_testingSnR) {
		uint8 black[4] = {   0,  0,    0,   0 };
		uint8 white[4] = { 255, 255, 255,   0 };

		// testing logic scripts by simulating an instant Save &
		// Restore

		g_display->setPalette(0, 1, white, RDPAL_INSTANT);

		// stops all fx & clears the queue - eg. when leaving a
		// location

		Clear_fx_queue();

		// Trash all object resources so they load in fresh & restart
		// their logic scripts

		res_man.killAllObjects(false);

		g_display->setPalette(0, 1, black, RDPAL_INSTANT);
	}

	return IR_CONT;
}

int32 Logic::fnRegisterMouse(int32 *params) {
	// this call would be made from an objects service script 0
	// the object would be one with no graphic but with a mouse - i.e. a
	// floor or one whose mouse area is manually defined rather than
	// intended to fit sprite shape

	// params:	0 pointer to Object_mouse or 0 for no write to mouse
	//		  list

	g_sword2->registerMouse((Object_mouse *) params[0]);
	return IR_CONT;
}

// use this in the object's service script prior to registering the mouse area
// ie. before fnRegisterMouse or fnRegisterFrame
// - best if kept at very top of service script

int32 Logic::fnRegisterPointerText(int32 *params) {
	// params:	0 local id of text line to use as pointer text

	assert(g_sword2->_curMouse < TOTAL_mouse_list);

	// current object id - used for checking pointer_text when mouse area
	// registered (in fnRegisterMouse and fnRegisterFrame)

	g_sword2->_mouseList[g_sword2->_curMouse].id = ID;
	g_sword2->_mouseList[g_sword2->_curMouse].pointer_text = params[0];

	return IR_CONT;
}

int32 Logic::fnInitFloorMouse(int32 *params) {
	// params:	0 pointer to object's mouse structure

 	Object_mouse *ob_mouse = (Object_mouse *) params[0];

	// floor is always lowest priority

	ob_mouse->x1 = 0;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = g_sword2->_thisScreen.screen_wide - 1;
	ob_mouse->y2 = g_sword2->_thisScreen.screen_deep - 1;
	ob_mouse->priority = 9;
	ob_mouse->pointer = NORMAL_MOUSE_ID;

	return IR_CONT;
}

#define SCROLL_MOUSE_WIDTH 20

int32 Logic::fnSetScrollLeftMouse(int32 *params) {
	// params:	0 pointer to object's mouse structure

 	Object_mouse *ob_mouse = (Object_mouse *) params[0];

	// Highest priority

	ob_mouse->x1 = 0;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = g_sword2->_thisScreen.scroll_offset_x + SCROLL_MOUSE_WIDTH;
	ob_mouse->y2 = g_sword2->_thisScreen.screen_deep - 1;
	ob_mouse->priority = 0;

	if (g_sword2->_thisScreen.scroll_offset_x > 0) {
		// not fully scrolled to the left
		ob_mouse->pointer = SCROLL_LEFT_MOUSE_ID;
	} else {
		// so the mouse area doesn't get registered
		ob_mouse->pointer = 0;
	}

	return IR_CONT;
}

int32 Logic::fnSetScrollRightMouse(int32 *params) {
	// params:	0 pointer to object's mouse structure

 	Object_mouse *ob_mouse = (Object_mouse *) params[0];

	// Highest priority

	ob_mouse->x1 = g_sword2->_thisScreen.scroll_offset_x + g_display->_screenWide - SCROLL_MOUSE_WIDTH;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = g_sword2->_thisScreen.screen_wide - 1;
	ob_mouse->y2 = g_sword2->_thisScreen.screen_deep - 1;
	ob_mouse->priority = 0;

	if (g_sword2->_thisScreen.scroll_offset_x < g_sword2->_thisScreen.max_scroll_offset_x) {
		// not fully scrolled to the right
		ob_mouse->pointer = SCROLL_RIGHT_MOUSE_ID;
	} else {
		// so the mouse area doesn't get registered
		ob_mouse->pointer = 0;
	}

	return IR_CONT;
}

int32 Logic::fnSetObjectHeld(int32 *params) {
	// params:	0 luggage icon to set

	g_sword2->setLuggage(params[0]);

	OBJECT_HELD = params[0];
	g_sword2->_currentLuggageResource = params[0];

	// mode locked - no menu available
	g_sword2->_mouseModeLocked = true;

	return IR_CONT;
}

// called from speech scripts to remove the chooser bar when it's not
// appropriate to keep it displayed

int32 Logic::fnRemoveChooser(int32 *params) {
	// params:	none

	g_display->hideMenu(RDMENU_BOTTOM);
	return IR_CONT;
}

int32 Logic::fnCheckPlayerActivity(int32 *params) {
	// Used to decide when to trigger music cues described as "no player
	// activity for a while"

	// params:	0 threshold delay in seconds, ie. what we want to
	//		  check the actual delay against

	uint32 threshold = params[0] * 12;	// in game cycles

	// if the actual delay is at or above the given threshold
	if (g_sword2->_playerActivityDelay >= threshold) {
		// reset activity delay counter, now that we've got a
		// positive check

		g_sword2->_playerActivityDelay = 0;
		RESULT = 1;
	} else
		RESULT = 0;

	return IR_CONT;
}

int32 Logic::fnResetPlayerActivityDelay(int32 *params) {
	// Use if you want to deliberately reset the "no player activity"
	// counter for any reason

	// params:	none

	g_sword2->_playerActivityDelay = 0;
	return IR_CONT;
}

} // End of namespace Sword2
