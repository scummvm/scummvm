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

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"

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

/**
 * Call at beginning of game loop
 */

void Sword2Engine::resetMouseList(void) {
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
		if (_input->_mouseY < 400) {
			_mouseMode = MOUSE_normal;
			debug(5, "   releasing");
		}
		break;
	default:
		break;
	}
}

int Sword2Engine::menuClick(int menu_items) {
	if (_input->_mouseX < RDMENU_ICONSTART)
		return -1;

	if (_input->_mouseX > RDMENU_ICONSTART + menu_items * (RDMENU_ICONWIDE + RDMENU_ICONSPACING) - RDMENU_ICONSPACING)
		return -1;

	return (_input->_mouseX - RDMENU_ICONSTART) / (RDMENU_ICONWIDE + RDMENU_ICONSPACING);
}

void Sword2Engine::systemMenuMouse(void) {
	uint32 safe_looping_music_id;
	_mouseEvent *me;
	int hit;
	uint8 *icon;
	int32 pars[2];
	uint32 icon_list[5] = {
		OPTIONS_ICON,
		QUIT_ICON,
		SAVE_ICON,
		RESTORE_ICON,
		RESTART_ICON
	};

	// If the mouse is moved off the menu, close it. Unless the player is
	// dead, in which case the menu should always be visible.

	if (_input->_mouseY > 0 && !DEAD) {
		_mouseMode = MOUSE_normal;
		_graphics->hideMenu(RDMENU_TOP);
		return;
	}

	// Check if the user left-clicks anywhere in the menu area.

	me = _input->mouseEvent();

	if (!me || !(me->buttons & RD_LEFTBUTTONDOWN))
		return;

	if (_input->_mouseY > 0)
		return;

	hit = menuClick(ARRAYSIZE(icon_list));

	if (hit < 0)
		return;

	// No save when dead

	if (icon_list[hit] == SAVE_ICON && DEAD)
		return;

	// Gray out all he icons, except the one that was clicked

	for (int i = 0; i < ARRAYSIZE(icon_list); i++) {
		if (i != hit) {
			icon = _resman->openResource(icon_list[i]) + sizeof(_standardHeader);
			_graphics->setMenuIcon(RDMENU_TOP, i, icon);
			_resman->closeResource(icon_list[i]);
		}
	}

	_sound->pauseFx();

	// NB. Need to keep a safe copy of '_loopingMusicId' for savegame & for
	// playing when returning from control panels because control panel
	// music will overwrite it!

	safe_looping_music_id = _loopingMusicId;

	pars[0] = 221;
	pars[1] = FX_LOOP;
	_logic->fnPlayMusic(pars);

	// restore proper looping_music_id
	_loopingMusicId = safe_looping_music_id;

	// clear the screen & set up the new palette for the menus

	_graphics->clearScene();
	_graphics->processMenu();
	_graphics->resetRenderEngine();

	// call the relevent screen

	switch (hit) {
	case 0:
		_gui->optionControl();
		break;
	case 1:
		_gui->quitControl();
		break;
	case 2:
		_gui->saveControl();
		break;
	case 3:
		_gui->restoreControl();
		break;
	case 4:
		_gui->restartControl();
		break;
	}

	// Menu stays open on death screen. Otherwise it's closed.

	if (!DEAD) {
		_mouseMode = MOUSE_normal;
		_graphics->hideMenu(RDMENU_TOP);
	} else {
		setMouse(NORMAL_MOUSE_ID);
		buildSystemMenu();
	}

	// Clear the screen & restore the location palette

	_graphics->clearScene();
	_graphics->processMenu();

	// Reset game palette, but not after a successful restore or restart!
	// See RestoreFromBuffer() in save_rest.cpp

	if (_thisScreen.new_palette != 99) {
		// 0 means put back game screen palette; see build_display.cpp
		setFullPalette(0);

		// Stop the engine fading in the restored screens palette
		_thisScreen.new_palette = 0;
	} else
		_thisScreen.new_palette = 1;

	_sound->unpauseFx();

	// If there was looping music before coming into the control panels
	// then restart it! NB. This will also start music required when a game
	// has been restored

	if (_loopingMusicId) {
		pars[0] = _loopingMusicId;
		pars[1] = FX_LOOP;
		_logic->fnPlayMusic(pars);

		// cross-fades into the required music: either a restored game
		// tune, or music playing prior to entering control panels
	} else {
		// stop the control panel music
		_logic->fnStopMusic(NULL);
	}
}

void Sword2Engine::dragMouse(void) {
	_mouseEvent *me;
	int hit;

	// We can use dragged object both on other inventory objects, or on
	// objects in the scene, so if the mouse moves off the inventory menu,
	// then close it.

	if (_input->_mouseY < 400) {
		_mouseMode = MOUSE_normal;
		_graphics->hideMenu(RDMENU_BOTTOM);
		return;
	}

	// Handles cursors and the luggage on/off according to type

	mouseOnOff();

	// Now do the normal click stuff

	me = _input->mouseEvent();

	if (!me)
		return;

#if 0
	// If the user right-clicks, cancel drag mode. The original code did
	// not do this, but it feels natural to me.

	if (me->buttons & RD_RIGHTBUTTONDOWN) {
		OBJECT_HELD = 0;
		_menuSelectedPos = 0;
		_mouseMode = MOUSE_menu;
		setLuggage(0);
		buildMenu();
		return;
	}
#endif

	if (!(me->buttons & RD_LEFTBUTTONDOWN))
		return;

	// there's a mouse event to be processed

	// could be clicking on an on screen object or on the menu
	// which is currently displayed

	if (_mouseTouching) {
		// mouse is over an on screen object - and we have luggage

		// Depending on type we'll maybe kill the object_held - like
		// for exits

		// Set global script variable 'button'. We know that it was the
		// left button, not the right one.

		LEFT_BUTTON = 1;
		RIGHT_BUTTON = 0;

		// These might be required by the action script about to be run

		MOUSE_X = _input->_mouseX + _thisScreen.scroll_offset_x;
		MOUSE_Y = _input->_mouseY + _thisScreen.scroll_offset_y;

		// For scripts to know what's been clicked. First used for
		// 'room_13_turning_script' in object 'biscuits_13'

		CLICKED_ID = _mouseTouching;

		_logic->setPlayerActionEvent(CUR_PLAYER_ID, _mouseTouching);

		debug(2, "Used \"%s\" on \"%s\"", fetchObjectName(OBJECT_HELD), fetchObjectName(CLICKED_ID));

		// Hide menu - back to normal menu mode

		_graphics->hideMenu(RDMENU_BOTTOM);
		_mouseMode = MOUSE_normal;

		return;
	}

	// Better check for combine/cancel. Cancel puts us back in MOUSE_menu
	// mode

	hit = menuClick(TOTAL_engine_pockets);

	if (hit < 0 || !_masterMenuList[hit].icon_resource)
		return;

	// Always back into menu mode. Remove the luggage as well.

	_mouseMode = MOUSE_menu;
	setLuggage(0);

	if ((uint) hit == _menuSelectedPos) {
		// If we clicked on the same icon again, reset the first icon

		OBJECT_HELD = 0;
		_menuSelectedPos = 0;
	} else {
		// Otherwise, combine the two icons

		COMBINE_BASE = _masterMenuList[hit].icon_resource;
		_logic->setPlayerActionEvent(CUR_PLAYER_ID, MENU_MASTER_OBJECT);

		// Turn off mouse now, to prevent player trying to click
		// elsewhere BUT leave the bottom menu open

		noHuman();

		debug(2, "Used \"%s\" on \"%s\"", fetchObjectName(OBJECT_HELD), fetchObjectName(COMBINE_BASE));
	}

	// Refresh the menu

	buildMenu();
}

void Sword2Engine::menuMouse(void) {
	_mouseEvent *me;
	int hit;

	// If the mouse is moved off the menu, close it.

	if (_input->_mouseY < 400) {
		_mouseMode = MOUSE_normal;
		_graphics->hideMenu(RDMENU_BOTTOM);
		return;
	}

	me = _input->mouseEvent();

	if (!me)
		return;

	hit = menuClick(TOTAL_engine_pockets);

	// Check if we clicked on an actual icon.

	if (hit < 0 || !_masterMenuList[hit].icon_resource)
		return;

	if (me->buttons & RD_RIGHTBUTTONDOWN) {
		// Right button - examine an object, identified by its icon
		// resource id.

		_examiningMenuIcon = true;
		OBJECT_HELD = _masterMenuList[hit].icon_resource;

		// Must clear this so next click on exit becomes 1st click
		// again

		EXIT_CLICK_ID = 0;

		_logic->setPlayerActionEvent(CUR_PLAYER_ID, MENU_MASTER_OBJECT);

		// Refresh the menu

		buildMenu();

		// Turn off mouse now, to prevent player trying to click
		// elsewhere BUT leave the bottom menu open

		noHuman();

		debug(2, "Right-click on \"%s\" icon",
			fetchObjectName(OBJECT_HELD));

		return;
	}

	if (me->buttons & RD_LEFTBUTTONDOWN) {
		// Left button - bung us into drag luggage mode. The object is
		// identified by its icon resource id. We need the luggage
		// resource id for mouseOnOff

		_mouseMode = MOUSE_drag;

		_menuSelectedPos = hit;
		OBJECT_HELD = _masterMenuList[hit].icon_resource;
		_currentLuggageResource = _masterMenuList[hit].luggage_resource;

		// Must clear this so next click on exit becomes 1st click
		// again

		EXIT_CLICK_ID = 0;

		// Refresh the menu

		buildMenu();

		setLuggage(_masterMenuList[hit].luggage_resource);

		debug(2, "Left-clicked on \"%s\" icon - switch to drag mode",
			fetchObjectName(OBJECT_HELD));
	}
}

void Sword2Engine::normalMouse(void) {
	// The gane is playing and none of the menus are activated - but, we
	// need to check if a menu is to start. Note, won't have luggage

	_mouseEvent *me;

	// Check if the cursor has moved onto the system menu area. No save in
	// big-object menu lock situation, of if the player is dragging an
	// object.

	if (_input->_mouseY < 0 && !_mouseModeLocked && !OBJECT_HELD) {
		_mouseMode = MOUSE_system_menu;

		if (_mouseTouching) {
			// We were on something, but not anymore
			_oldMouseTouching = 0;
			_mouseTouching = 0;
		}

		// Reset mouse cursor - in case we're between mice

		setMouse(NORMAL_MOUSE_ID);
		buildSystemMenu();
		return;
	}

	// Check if the cursor has moved onto the inventory menu area. No
	// inventory in big-object menu lock situation,

	if (_input->_mouseY > 399 && !_mouseModeLocked) {
		// If an object is being held, i.e. if the mouse cursor has a
		// luggage, go to drag mode instead of menu mode, but the menu
		// is still opened.
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
			_oldMouseTouching = 0;
			_mouseTouching = 0;
		}

		// Reset mouse cursor

		setMouse(NORMAL_MOUSE_ID);
		buildMenu();
		return;
	}

	// Check for moving the mouse on or off things

	mouseOnOff();

	me = _input->mouseEvent();

	if (!me)
		return;

	bool button_down = (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN)) != 0;

	// For debugging. We can draw a rectangle on the screen and see its
	// coordinates. This was probably used to help defining hit areas.

	if (_debugger->_definingRectangles) {
		if (_debugger->_draggingRectangle == 0) {
			// Not yet dragging a rectangle, so need click to start

			if (button_down) {
				// set both (x1,y1) and (x2,y2) to this point
				_debugger->_rectX1 = _debugger->_rectX2 = (uint32) _input->_mouseX + _thisScreen.scroll_offset_x;
				_debugger->_rectY1 = _debugger->_rectY2 = (uint32) _input->_mouseY + _thisScreen.scroll_offset_y;
				_debugger->_draggingRectangle = 1;
			}
		} else if (_debugger->_draggingRectangle == 1) {
			// currently dragging a rectangle - click means reset

			if (button_down) {
				// lock rectangle, so you can let go of mouse
				// to type in the coords
				_debugger->_draggingRectangle = 2;
			} else {
				// drag rectangle
				_debugger->_rectX2 = (uint32) _input->_mouseX + _thisScreen.scroll_offset_x;
				_debugger->_rectY2 = (uint32) _input->_mouseY + _thisScreen.scroll_offset_y;
			}
		} else {
			// currently locked to avoid knocking out of place
			// while reading off the coords

			if (button_down) {
				// click means reset - back to start again
				_debugger->_draggingRectangle = 0;
			}
		}

		return;
	}

#if 0
	// If user right-clicks while holding an object, release it. The
	// original code did not do this, but it feels natural to me.

	if (OBJECT_HELD && (me->buttons & RD_RIGHTBUTTONDOWN)) {
		OBJECT_HELD = 0;
		_menuSelectedPos = 0;
		setLuggage(0);
		return;
	}
#endif

	// Now do the normal click stuff

	// We only care about down clicks when the mouse is over an object. We
	// ignore mouse releases

	if (!_mouseTouching || !button_down)
		return;

	// There's a mouse event to be processed and the mouse is on something.
	// Notice that the floor itself is considered an object.

	// There are no menus about so its nice and simple. This is as close to
	// the old advisor_188 script as we get, I'm sorry to say.

	// If player is walking or relaxing then those need to terminate
	// correctly. Otherwise set player run the targets action script or, do
	// a special walk if clicking on the scroll-more icon

	// PLAYER_ACTION script variable - whatever catches this must reset to
	// 0 again
	// PLAYER_ACTION = _mouseTouching;

	// Idle or router-anim will catch it

	// Set global script variable 'button'

	if (me->buttons & RD_LEFTBUTTONDOWN) {
		LEFT_BUTTON  = 1;
		RIGHT_BUTTON = 0;
		_buttonClick = 0;	// for re-click
	} else {
		LEFT_BUTTON  = 0;
		RIGHT_BUTTON = 1;
		_buttonClick = 1;	// for re-click
	}

	// These might be required by the action script about to be run

	MOUSE_X = _input->_mouseX + _thisScreen.scroll_offset_x;
	MOUSE_Y = _input->_mouseY + _thisScreen.scroll_offset_y;

	if (_mouseTouching == EXIT_CLICK_ID && (me->buttons & RD_LEFTBUTTONDOWN)) {
		// It's the exit double click situation. Let the existing
		// interaction continue and start fading down. Switch the human
		// off too

		_logic->fnNoHuman(NULL);
		_logic->fnFadeDown(NULL);

		// Tell the walker

		EXIT_FADING = 1;
	} else if (_oldButton == _buttonClick && _mouseTouching == CLICKED_ID && _mousePointerRes != NORMAL_MOUSE_ID) {
		// Re-click. Do nothing, except on floors
	} else {
		// For re-click

		_oldButton = _buttonClick;

		// For scripts to know what's been clicked. First used for
		// 'room_13_turning_script' in object 'biscuits_13'

		CLICKED_ID = _mouseTouching;

		// Must clear these two double-click control flags - do it here
		// so reclicks after exit clicks are cleared up

		EXIT_CLICK_ID = 0;
		EXIT_FADING = 0;

		// WORKAROUND: Examining the lift while at the top of the
		// pyramid causes the game to hang. It looks like a script
		// bug to me: the script hides the mouse cursor, checks if the
		// player pressed the left mouse button and, if not, jumps to
		// an end of script instruction.
		//
		// One idea would be to redirect the action to the elevator
		// object at the bottom of the pyramid instead, but I don't
		// know if that's a safe thing to do so for now I've disabled
		// it. Maybe we could find a better workaround if we had a
		// script decompiler...
		//
		// I'm checking the status of the left button rather than the
		// right button because that's what I think the script does.

		if (_mouseTouching == 2773 && !LEFT_BUTTON) {
			warning("Ignoring action to work around script bug at pyramid top");
			// _logic->setPlayerActionEvent(CUR_PLAYER_ID, 2737);
		} else
			_logic->setPlayerActionEvent(CUR_PLAYER_ID, _mouseTouching);

		if (OBJECT_HELD)
			debug(2, "Used \"%s\" on \"%s\"", fetchObjectName(OBJECT_HELD), fetchObjectName(CLICKED_ID));
		else if (LEFT_BUTTON)
			debug(2, "Left-clicked on \"%s\"", fetchObjectName(CLICKED_ID));
		else	// RIGHT BUTTON
			debug(2, "Right-clicked on \"%s\"", fetchObjectName(CLICKED_ID));
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

	if (_input->_mouseY < 0 || _input->_mouseY > 399) {
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
			error("ERROR: mouse.pointer==0 for object %d (%s) - update logic script!", _mouseTouching, fetchObjectName(_mouseTouching));
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
		icon = _resman->openResource(res) + sizeof(_standardHeader);
		len = _resman->_resList[res]->size - sizeof(_standardHeader);

		// don't pulse the normal pointer - just do the regular anim
		// loop

		if (res == NORMAL_MOUSE_ID)
			_graphics->setMouseAnim(icon, len, RDMOUSE_NOFLASH);
		else
 			_graphics->setMouseAnim(icon, len, RDMOUSE_FLASH);

		_resman->closeResource(res);
	} else {
		// blank cursor
		_graphics->setMouseAnim(NULL, 0, 0);
	}
}

void Sword2Engine::setLuggage(uint32 res) {
	uint8 *icon;
	uint32 len;

	_realLuggageItem = res;

	if (res) {
		icon = _resman->openResource(res) + sizeof(_standardHeader);
		len = _resman->_resList[res]->size - sizeof(_standardHeader);

		_graphics->setLuggageAnim(icon, len);

		_resman->closeResource(res);
	} else
		_graphics->setLuggageAnim(NULL, 0);
}

uint32 Sword2Engine::checkMouseList(void) {
	int32 priority = 0;
	uint32 j = 1;

	if (_curMouse > 1) {
		// Number of priorities subject to implementation needs

		while (priority < 10) {
			// If the mouse pointer is over this
			// mouse-detection-box

			if (_mouseList[j].priority == priority &&
			    _input->_mouseX + _thisScreen.scroll_offset_x >= _mouseList[j].x1 &&
			    _input->_mouseX + _thisScreen.scroll_offset_x <= _mouseList[j].x2 &&
			    _input->_mouseY + _thisScreen.scroll_offset_y >= _mouseList[j].y1 &&
			    _input->_mouseY + _thisScreen.scroll_offset_y <= _mouseList[j].y2) {
				// Record id
				_mouseTouching = _mouseList[j].id;

				// Change all COGS pointers to CROSHAIR
				if (_mouseList[j].pointer == USE)
					_mouseList[j].pointer = CROSHAIR;

				createPointerText(_mouseList[j].pointer_text, _mouseList[j].pointer);

				// Return pointer type
				return _mouseList[j].pointer;
			}

			j++;
			if (j == _curMouse) {
				j = 0;
				// Next priority - 0 being the highest, 9 the
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

	if (!_gui->_pointerTextSelected || !text_id)
		return;

	// Check what the pointer is, to set offsets correctly for text
	// position

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
		// Shouldn't happen if we cover all the different mouse
		// pointers above
		yOffset = -10;
		xOffset = +10;
		break;
	}

	// Set up justification for text sprite, based on its offsets from the
	// pointer position

	if (yOffset < 0) {
		// Above pointer
		if (xOffset < 0) {
			// Above left
			justification = POSITION_AT_RIGHT_OF_BASE;
		} else if (xOffset > 0) {
			// Above right
			justification = POSITION_AT_LEFT_OF_BASE;
		} else {
			// Above centre
			justification = POSITION_AT_CENTRE_OF_BASE;
		}
	} else if (yOffset > 0) {
		// Below pointer
		if (xOffset < 0) {
			// Below left
			justification = POSITION_AT_RIGHT_OF_TOP;
		} else if (xOffset > 0) {
			// Below right
			justification = POSITION_AT_LEFT_OF_TOP;
		} else {
			// Below centre
			justification = POSITION_AT_CENTRE_OF_TOP;
		}
	} else {
		// Same y-coord as pointer
		if (xOffset < 0) {
			// Centre left
			justification = POSITION_AT_RIGHT_OF_CENTRE;
		} else if (xOffset > 0) {
			// Centre right
			justification = POSITION_AT_LEFT_OF_CENTRE;
		} else {
			// Centre centre - shouldn't happen anyway!
			justification = POSITION_AT_LEFT_OF_CENTRE;
		}
	}

	// Text resource number, and line number within the resource

	text_res = text_id / SIZE;
	local_text = text_id & 0xffff;

	// open text file & get the line
	text = fetchTextLine(_resman->openResource(text_res), local_text);

	// 'text+2' to skip the first 2 bytes which form the
	// line reference number

	_pointerTextBlocNo = _fontRenderer->buildNewBloc(
		text + 2, _input->_mouseX + xOffset,
		_input->_mouseY + yOffset,
		POINTER_TEXT_WIDTH, POINTER_TEXT_PEN,
		RDSPR_TRANS | RDSPR_DISPLAYALIGN,
		_speechFontId, justification);

	// now ok to close the text file
	_resman->closeResource(text_res);
}

void Sword2Engine::clearPointerText(void) {
	if (_pointerTextBlocNo) {
		_fontRenderer->killTextBloc(_pointerTextBlocNo);
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

	if (!ob_mouse->pointer)
		return;

	assert(_curMouse < TOTAL_mouse_list);

	_mouseList[_curMouse].x1 = ob_mouse->x1;
	_mouseList[_curMouse].y1 = ob_mouse->y1;
	_mouseList[_curMouse].x2 = ob_mouse->x2;
	_mouseList[_curMouse].y2 = ob_mouse->y2;

	_mouseList[_curMouse].priority = ob_mouse->priority;
	_mouseList[_curMouse].pointer = ob_mouse->pointer;

	// Check if pointer text field is set due to previous object using this
	// slot (ie. not correct for this one)

	// If 'pointer_text' field is set, but the 'id' field isn't same is
	// current id, then we don't want this "left over" pointer text

	if (_mouseList[_curMouse].pointer_text && _mouseList[_curMouse].id != (int32) ID)
		_mouseList[_curMouse].pointer_text = 0;

	// Get id from system variable 'id' which is correct for current object

	_mouseList[_curMouse].id = ID;

	// Not using sprite as mask - this is only done from fnRegisterFrame()

	_mouseList[_curMouse].anim_resource = 0;
	_mouseList[_curMouse].anim_pc = 0;

	debug(5, "mouse id %d", _mouseList[_curMouse].id);
	_curMouse++;
}

void Sword2Engine::monitorPlayerActivity(void) {
	// if there is at least one mouse event outstanding
	if (_input->checkForMouseEvents()) {
		// reset activity delay counter
		_playerActivityDelay = 0;
	} else {
		// no. of game cycles since mouse event queue last empty
		_playerActivityDelay++;
	}
}

int32 Logic::fnNoHuman(int32 *params) {
	// params:	none

	_vm->noHuman();
	_vm->clearPointerText();

	// must be normal mouse situation or a largely neutral situation -
	// special menus use noHuman

	// dont hide menu in conversations
	if (TALK_FLAG == 0)
		_vm->_graphics->hideMenu(RDMENU_BOTTOM);

	if (_vm->_mouseMode == MOUSE_system_menu) {
		// close menu
		_vm->_mouseMode = MOUSE_normal;
		_vm->_graphics->hideMenu(RDMENU_TOP);
	}

	// script continue
	return IR_CONT;
}

int32 Logic::fnAddHuman(int32 *params) {
	// params:	none

	// for logic scripts
	MOUSE_AVAILABLE = 1;

	// off
	if (_vm->_mouseStatus) {
		_vm->_mouseStatus = false;	// on
		_vm->_mouseTouching = 1;	// forces engine to choose a cursor
	}

	//clear this to reset no-second-click system
	CLICKED_ID = 0;

	// this is now done outside the OBJECT_HELD check in case it's set to
	// zero before now!

	// unlock the mouse from possible large object lock situtations - see
	// syphon in rm 3

	_vm->_mouseModeLocked = false;

	if (OBJECT_HELD) {
		// was dragging something around
		// need to clear this again
		OBJECT_HELD = 0;

		// and these may also need clearing, just in case
		_vm->_examiningMenuIcon = false;
		COMBINE_BASE = 0;

		_vm->setLuggage(0);
	}

	// if mouse is over menu area
	if (_vm->_input->_mouseY > 399) {
		if (_vm->_mouseMode != MOUSE_holding) {
			// VITAL - reset things & rebuild the menu
			_vm->_mouseMode = MOUSE_normal;
			_vm->setMouse(NORMAL_MOUSE_ID);
		} else
			_vm->setMouse(NORMAL_MOUSE_ID);
	}

	// enabled/disabled from console; status printed with on-screen debug
	// info

	if (_vm->_debugger->_testingSnR) {
		uint8 black[4] = {   0,  0,    0,   0 };
		uint8 white[4] = { 255, 255, 255,   0 };

		// testing logic scripts by simulating an instant Save &
		// Restore

		_vm->_graphics->setPalette(0, 1, white, RDPAL_INSTANT);

		// stops all fx & clears the queue - eg. when leaving a
		// location

		_vm->clearFxQueue();

		// Trash all object resources so they load in fresh & restart
		// their logic scripts

		_vm->_resman->killAllObjects(false);

		_vm->_graphics->setPalette(0, 1, black, RDPAL_INSTANT);
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

	_vm->registerMouse((Object_mouse *) _vm->_memory->intToPtr(params[0]));
	return IR_CONT;
}

// use this in the object's service script prior to registering the mouse area
// ie. before fnRegisterMouse or fnRegisterFrame
// - best if kept at very top of service script

int32 Logic::fnRegisterPointerText(int32 *params) {
	// params:	0 local id of text line to use as pointer text

	assert(_vm->_curMouse < TOTAL_mouse_list);

	// current object id - used for checking pointer_text when mouse area
	// registered (in fnRegisterMouse and fnRegisterFrame)

	_vm->_mouseList[_vm->_curMouse].id = ID;
	_vm->_mouseList[_vm->_curMouse].pointer_text = params[0];

	return IR_CONT;
}

int32 Logic::fnInitFloorMouse(int32 *params) {
	// params:	0 pointer to object's mouse structure

 	Object_mouse *ob_mouse = (Object_mouse *) _vm->_memory->intToPtr(params[0]);

	// floor is always lowest priority

	ob_mouse->x1 = 0;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = _vm->_thisScreen.screen_wide - 1;
	ob_mouse->y2 = _vm->_thisScreen.screen_deep - 1;
	ob_mouse->priority = 9;
	ob_mouse->pointer = NORMAL_MOUSE_ID;

	return IR_CONT;
}

#define SCROLL_MOUSE_WIDTH 20

int32 Logic::fnSetScrollLeftMouse(int32 *params) {
	// params:	0 pointer to object's mouse structure

 	Object_mouse *ob_mouse = (Object_mouse *) _vm->_memory->intToPtr(params[0]);

	// Highest priority

	ob_mouse->x1 = 0;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = _vm->_thisScreen.scroll_offset_x + SCROLL_MOUSE_WIDTH;
	ob_mouse->y2 = _vm->_thisScreen.screen_deep - 1;
	ob_mouse->priority = 0;

	if (_vm->_thisScreen.scroll_offset_x > 0) {
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

	Object_mouse *ob_mouse = (Object_mouse *) _vm->_memory->intToPtr(params[0]);

	// Highest priority

	ob_mouse->x1 = _vm->_thisScreen.scroll_offset_x + _vm->_graphics->_screenWide - SCROLL_MOUSE_WIDTH;
	ob_mouse->y1 = 0;
	ob_mouse->x2 = _vm->_thisScreen.screen_wide - 1;
	ob_mouse->y2 = _vm->_thisScreen.screen_deep - 1;
	ob_mouse->priority = 0;

	if (_vm->_thisScreen.scroll_offset_x < _vm->_thisScreen.max_scroll_offset_x) {
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

	_vm->setLuggage(params[0]);

	OBJECT_HELD = params[0];
	_vm->_currentLuggageResource = params[0];

	// mode locked - no menu available
	_vm->_mouseModeLocked = true;

	return IR_CONT;
}

// called from speech scripts to remove the chooser bar when it's not
// appropriate to keep it displayed

int32 Logic::fnRemoveChooser(int32 *params) {
	// params:	none

	_vm->_graphics->hideMenu(RDMENU_BOTTOM);
	return IR_CONT;
}

int32 Logic::fnCheckPlayerActivity(int32 *params) {
	// Used to decide when to trigger music cues described as "no player
	// activity for a while"

	// params:	0 threshold delay in seconds, ie. what we want to
	//		  check the actual delay against

	uint32 threshold = params[0] * 12;	// in game cycles

	// if the actual delay is at or above the given threshold
	if (_vm->_playerActivityDelay >= threshold) {
		// reset activity delay counter, now that we've got a
		// positive check

		_vm->_playerActivityDelay = 0;
		RESULT = 1;
	} else
		RESULT = 0;

	return IR_CONT;
}

int32 Logic::fnResetPlayerActivityDelay(int32 *params) {
	// Use if you want to deliberately reset the "no player activity"
	// counter for any reason

	// params:	none

	_vm->_playerActivityDelay = 0;
	return IR_CONT;
}

} // End of namespace Sword2
