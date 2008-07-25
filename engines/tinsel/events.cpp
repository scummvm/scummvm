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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * Main purpose is to process user events.
 * Also provides a couple of utility functions.
 */

#include "tinsel/actors.h"
#include "tinsel/config.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/events.h"
#include "tinsel/handle.h"	// For LockMem()
#include "tinsel/inventory.h"
#include "tinsel/move.h"	// For walking lead actor
#include "tinsel/pcode.h"	// For Interpret()
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"	// For walking lead actor
#include "tinsel/sched.h"
#include "tinsel/scroll.h"	// For DontScrollCursor()
#include "tinsel/timers.h"	// DwGetCurrentTime()
#include "tinsel/tinlib.h"	// For control()
#include "tinsel/token.h"

namespace Tinsel {

//----------------- EXTERNAL FUNCTIONS ---------------------

// in PDISPLAY.C
extern int GetTaggedActor(void);
extern HPOLYGON GetTaggedPoly(void);


//----------------- EXTERNAL GLOBAL DATA ---------------------

extern bool bEnableF1;


//----------------- LOCAL GLOBAL DATA --------------------

static int userEvents = 0;		// Whenever a button or a key comes in
static uint32 lastUserEvent = 0;	// Time it hapenned
static int butEvents = 0;	// Single or double, left or right. Or escape key.
static int escEvents = 0;	// Escape key


static int eCount = 0;

/**
 * Gets called before each schedule, only 1 user action per schedule
 * is allowed.
 */
void ResetEcount(void) {
	eCount = 0;
}


void IncUserEvents(void) {
	userEvents++;
	lastUserEvent = DwGetCurrentTime();
}

/**
 * If this is a single click, wait to check it's not the first half of a
 * double click.
 * If this is a double click, the process from the waiting single click
 * gets killed.
 */
void AllowDclick(CORO_PARAM, BUTEVENT be) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	if (be == BE_SLEFT) {
		GetToken(TOKEN_LEFT_BUT);
		CORO_SLEEP(dclickSpeed+1);
		FreeToken(TOKEN_LEFT_BUT);

		// Prevent activation of 2 events on the same tick
		if (++eCount != 1)
			CORO_KILL_SELF();

		break;

	} else if (be == BE_DLEFT) {
		GetToken(TOKEN_LEFT_BUT);
		FreeToken(TOKEN_LEFT_BUT);
	}
	CORO_END_CODE;
}

/**
 * Take control from player, if the player has it.
 * Return TRUE if control taken, FALSE if not.
 */

bool GetControl(int param) {
	if (TestToken(TOKEN_CONTROL)) {
		control(param);
		return true;
	} else
		return false;
}

struct TP_INIT {
	HPOLYGON	hPoly;		// Polygon
	USER_EVENT	event;		// Trigerring event
	BUTEVENT	bev;		// To allow for double clicks
	bool		take_control;	// Set if control should be taken
					// while code is running.
	int		actor;
};

/**
 * Runs glitter code associated with a polygon.
 */
static void PolyTinselProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
		bool took_control;	// Set if this function takes control
	CORO_END_CONTEXT(_ctx);

	TP_INIT *to = (TP_INIT *)param;	// get the stuff copied to process when it was created

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(AllowDclick, to->bev);	// May kill us if single click

	// Control may have gone off during AllowDclick()
	if (!TestToken(TOKEN_CONTROL)
	    && (to->event == WALKTO || to->event == ACTION || to->event == LOOK))
		CORO_KILL_SELF();

	// Take control, if requested
	if (to->take_control)
		_ctx->took_control = GetControl(CONTROL_OFF);
	else
		_ctx->took_control = false;

	// Hide conversation if appropriate
	if (to->event == CONVERSE)
		convHide(true);

	// Run the code
	_ctx->pic = InitInterpretContext(GS_POLYGON, getPolyScript(to->hPoly), to->event, to->hPoly, to->actor, NULL);
	CORO_INVOKE_1(Interpret, _ctx->pic);

	// Free control if we took it
	if (_ctx->took_control)
		control(CONTROL_ON);

	// Restore conv window if applicable
	if (to->event == CONVERSE)
		convHide(false);

	CORO_END_CODE;
}

/**
 * Runs glitter code associated with a polygon.
 */
void RunPolyTinselCode(HPOLYGON hPoly, USER_EVENT event, BUTEVENT be, bool tc) {
	TP_INIT to = { hPoly, event, be, tc, 0 };

	g_scheduler->createProcess(PID_TCODE, PolyTinselProcess, &to, sizeof(to));
}

void effRunPolyTinselCode(HPOLYGON hPoly, USER_EVENT event, int actor) {
	TP_INIT to = { hPoly, event, BE_NONE, false, actor };

	g_scheduler->createProcess(PID_TCODE, PolyTinselProcess, &to, sizeof(to));
}

//-----------------------------------------------------------------------

struct WP_INIT {
	int	x;	// } Where to walk to
	int	y;	// }
};

/**
 * Perform a walk directly initiated by a click.
 */
static void WalkProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		PMACTOR pActor;
	CORO_END_CONTEXT(_ctx);

	WP_INIT *to = (WP_INIT *)param;	// get the co-ordinates - copied to process when it was created

	CORO_BEGIN_CODE(_ctx);

	_ctx->pActor = GetMover(LEAD_ACTOR);
	if (_ctx->pActor->MActorState == NORM_MACTOR) {
		assert(_ctx->pActor->hCpath != NOPOLY); // Lead actor is not in a path

		GetToken(TOKEN_LEAD);
		SetActorDest(_ctx->pActor, to->x, to->y, false, 0);
		DontScrollCursor();

		while (MAmoving(_ctx->pActor))
			CORO_SLEEP(1);

		FreeToken(TOKEN_LEAD);
	}

	CORO_END_CODE;
}

void walkto(int x, int y) {
	WP_INIT to = { x, y };

	g_scheduler->createProcess(PID_TCODE, WalkProcess, &to, sizeof(to));
}

/**
 * Run appropriate actor or polygon glitter code.
 * If none, and it's a WALKTO event, do a walk.
 */
static void User_Event(USER_EVENT uEvent, BUTEVENT be) {
	int	actor;
	int	aniX, aniY;
	HPOLYGON hPoly;

	// Prevent activation of 2 events on the same tick
	if (++eCount != 1)
		return;

	if ((actor = GetTaggedActor()) != 0)
		actorEvent(actor, uEvent, be);
	else if ((hPoly = GetTaggedPoly()) != NOPOLY)
		RunPolyTinselCode(hPoly, uEvent, be, false);
	else {
		GetCursorXY(&aniX, &aniY, true);

		// There could be a poly involved which has no tag.
		if ((hPoly = InPolygon(aniX, aniY, TAG)) != NOPOLY
			|| (hPoly = InPolygon(aniX, aniY, EXIT)) != NOPOLY) {
			RunPolyTinselCode(hPoly, uEvent, be, false);
		} else if (uEvent == WALKTO)
			walkto(aniX, aniY);
	}
}


/**
 * ProcessButEvent
 */
void ProcessButEvent(BUTEVENT be) {
	IncUserEvents();

	if (bSwapButtons) {
		switch (be) {
		case BE_SLEFT:
			be = BE_SRIGHT;
			break;
		case BE_DLEFT:
			be = BE_DRIGHT;
			break;
		case BE_SRIGHT:
			be = BE_SLEFT;
			break;
		case BE_DRIGHT:
			be = BE_DLEFT;
			break;
		case BE_LDSTART:
			be = BE_RDSTART;
			break;
		case BE_LDEND:
			be = BE_RDEND;
			break;
		case BE_RDSTART:
			be = BE_LDSTART;
			break;
		case BE_RDEND:
			be = BE_LDEND;
			break;
		default:
			break;
		}
	}

//	if (be == BE_SLEFT || be == BE_DLEFT || be == BE_SRIGHT || be == BE_DRIGHT)
	if (be == BE_SLEFT || be == BE_SRIGHT)
		butEvents++;

	if (!TestToken(TOKEN_CONTROL) && be != BE_LDEND)
		return;

	if (InventoryActive()) {
		ButtonToInventory(be);
	} else {
		switch (be) {
		case BE_SLEFT:
			User_Event(WALKTO, BE_SLEFT);
			break;
	
		case BE_DLEFT:
			User_Event(ACTION, BE_DLEFT);
			break;
	
		case BE_SRIGHT:
			User_Event(LOOK, BE_SRIGHT);
			break;
	
		default:
			break;
		}
	}
}

/**
 * ProcessKeyEvent
 */

void ProcessKeyEvent(KEYEVENT ke) {
	// This stuff to allow F1 key during startup.
	if (bEnableF1 && ke == OPTION_KEY)
		control(CONTROL_ON);
	else
		IncUserEvents();

	if (ke == ESC_KEY) {
		escEvents++;
		butEvents++;		// Yes, I do mean this
	}

	// FIXME: This comparison is weird - I added (BUTEVENT) cast for now to suppress warning
	if (!TestToken(TOKEN_CONTROL) && (BUTEVENT)ke != BE_LDEND)
		return;

	switch (ke) {
	case QUIT_KEY:
		PopUpConf(QUIT);
		break;

	case OPTION_KEY:
		PopUpConf(OPTION);
		break;

	case SAVE_KEY:
		PopUpConf(SAVE);
		break;

	case LOAD_KEY:
		PopUpConf(LOAD);
		break;

	case WALKTO_KEY:
		if (InventoryActive())
			ButtonToInventory(BE_SLEFT);
		else
			User_Event(WALKTO, BE_NONE);
		break;

	case ACTION_KEY:
		if (InventoryActive())
			ButtonToInventory(BE_DLEFT);
		else
			User_Event(ACTION, BE_NONE);
		break;

	case LOOK_KEY:
		if (InventoryActive())
			ButtonToInventory(BE_SRIGHT);
		else
			User_Event(LOOK, BE_NONE);
		break;

	case ESC_KEY:
	case PGUP_KEY:
	case PGDN_KEY:
	case HOME_KEY:
	case END_KEY:
		if (InventoryActive())
			KeyToInventory(ke);
		break;

	default:
		break;
	}
}

/**
 * For ESCapable Glitter sequences
 */

int GetEscEvents(void) {
	return escEvents;
}

/**
 * For cutting short talk()s etc.
 */

int GetLeftEvents(void) {
	return butEvents;
}

/**
 * For waitkey() Glitter function
 */

int getUserEvents(void) {
	return userEvents;
}

uint32 getUserEventTime(void) {
	return DwGetCurrentTime() - lastUserEvent;
}

void resetUserEventTime(void) {
	lastUserEvent = DwGetCurrentTime();
}

} // end of namespace Tinsel
