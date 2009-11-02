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
 * Handles scrolling
 */

#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/graphics.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/scroll.h"
#include "tinsel/sched.h"
#include "tinsel/sysvar.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

//----------------- LOCAL DEFINES --------------------

#define LEFT	'L'
#define RIGHT	'R'
#define UP	'U'
#define DOWN	'D'



//----------------- LOCAL GLOBAL DATA --------------------

static int LeftScroll = 0, DownScroll = 0;	// Number of iterations outstanding

static int scrollActor = 0;
static PMOVER pScrollMover = 0;
static int oldx = 0, oldy = 0;

/** Boundaries and numbers of boundaries */
static SCROLLDATA sd = {
		{
			{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
			{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}
		},
		{
			{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
			{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}
		},
		0,
		0,
		// DW2 fields
		0,
		0,
		0,
		0,
		0,
		0,
		0
	};

static int ImageH = 0, ImageW = 0;

static bool ScrollCursor = 0;	// If a TAG or EXIT polygon is clicked on,
				// the cursor is kept over that polygon
				// whilst scrolling

static int scrollPixelsX = SCROLLPIXELS;
static int scrollPixelsY = SCROLLPIXELS;


/**
 * Reset the ScrollCursor flag
 */
void DontScrollCursor() {
	ScrollCursor = false;
}

/**
 * Set the ScrollCursor flag
 */
void DoScrollCursor() {
	ScrollCursor = true;
}

/**
 * Configure a no-scroll boundary for a scene.
 */
void SetNoScroll(int x1, int y1, int x2, int y2) {
	if (x1 == x2) {
		/* Vertical line */
		assert(sd.NumNoH < MAX_HNOSCROLL);

		sd.NoHScroll[sd.NumNoH].ln = x1;	// X pos of vertical line
		sd.NoHScroll[sd.NumNoH].c1 = y1;
		sd.NoHScroll[sd.NumNoH].c2 = y2;
		sd.NumNoH++;
	} else if (y1 == y2) {
		/* Horizontal line */
		assert(sd.NumNoV < MAX_VNOSCROLL);

		sd.NoVScroll[sd.NumNoV].ln = y1;	// Y pos of horizontal line
		sd.NoVScroll[sd.NumNoV].c1 = x1;
		sd.NoVScroll[sd.NumNoV].c2 = x2;
		sd.NumNoV++;
	} else {
		/* No-scroll lines must be horizontal or vertical */
	}
}

/**
 * Called from scroll process when it thinks that a scroll is in order.
 * Checks for no-scroll boundaries and sets off a scroll if allowed.
 */
static void NeedScroll(int direction) {
	uint	i;
	int	BottomLine, RightCol;
	int	Loffset, Toffset;

	// get background offsets
	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	switch (direction) {
	case LEFT:	  /* Picture will go left, 'camera' right */

		BottomLine = Toffset + (SCREEN_HEIGHT - 1);
		RightCol = Loffset + (SCREEN_WIDTH - 1);

		for (i = 0; i < sd.NumNoH; i++) {
			if (RightCol >= sd.NoHScroll[i].ln - 1 && RightCol <= sd.NoHScroll[i].ln + 1 &&
					((sd.NoHScroll[i].c1 >= Toffset && sd.NoHScroll[i].c1 <= BottomLine) ||
					(sd.NoHScroll[i].c2 >= Toffset && sd.NoHScroll[i].c2 <= BottomLine) ||
					(sd.NoHScroll[i].c1 < Toffset && sd.NoHScroll[i].c2 > BottomLine)))
				return;
		}

		if (LeftScroll <= 0) {
			if (TinselV2) {
				scrollPixelsX = sd.xSpeed;
				LeftScroll += sd.xDistance;
			} else {
				scrollPixelsX = SCROLLPIXELS;
				LeftScroll = RLSCROLL;
			}
		}
		break;

	case RIGHT:	 /* Picture will go right, 'camera' left */

		BottomLine = Toffset + (SCREEN_HEIGHT - 1);

		for (i = 0; i < sd.NumNoH; i++) {
			if (Loffset >= sd.NoHScroll[i].ln - 1 && Loffset <= sd.NoHScroll[i].ln + 1 &&
					((sd.NoHScroll[i].c1 >= Toffset && sd.NoHScroll[i].c1 <= BottomLine) ||
					(sd.NoHScroll[i].c2 >= Toffset && sd.NoHScroll[i].c2 <= BottomLine) ||
					(sd.NoHScroll[i].c1 < Toffset && sd.NoHScroll[i].c2 > BottomLine)))
				return;
		}

		if (LeftScroll >= 0) {
			if (TinselV2) {
				scrollPixelsX = sd.xSpeed;
				LeftScroll -= sd.xDistance;
			} else {
				scrollPixelsX = SCROLLPIXELS;
				LeftScroll = -RLSCROLL;
			}
		}
		break;

	case UP:		/* Picture will go upwards, 'camera' downwards  */

		BottomLine = Toffset + (SCREEN_HEIGHT - 1);
		RightCol = Loffset + (SCREEN_WIDTH - 1);

		for (i = 0; i < sd.NumNoV; i++) {
			if ((BottomLine >= sd.NoVScroll[i].ln - 1 && BottomLine <= sd.NoVScroll[i].ln + 1) &&
					((sd.NoVScroll[i].c1 >= Loffset && sd.NoVScroll[i].c1 <= RightCol) ||
					(sd.NoVScroll[i].c2 >= Loffset && sd.NoVScroll[i].c2 <= RightCol) ||
					(sd.NoVScroll[i].c1 < Loffset && sd.NoVScroll[i].c2 > RightCol)))
				return;
			}

		if (DownScroll <= 0) {
			if (TinselV2) {
				scrollPixelsY = sd.ySpeed;
				DownScroll += sd.yDistance;
			} else {
				scrollPixelsY = SCROLLPIXELS;
				DownScroll = UDSCROLL;
			}
		}
		break;

	case DOWN:	  /* Picture will go downwards, 'camera' upwards  */

		RightCol = Loffset + (SCREEN_WIDTH - 1);

		for (i = 0; i < sd.NumNoV; i++) {
			if (Toffset >= sd.NoVScroll[i].ln - 1  && Toffset <= sd.NoVScroll[i].ln + 1  &&
					((sd.NoVScroll[i].c1 >= Loffset && sd.NoVScroll[i].c1 <= RightCol) ||
					(sd.NoVScroll[i].c2 >= Loffset && sd.NoVScroll[i].c2 <= RightCol) ||
					(sd.NoVScroll[i].c1 < Loffset && sd.NoVScroll[i].c2 > RightCol)))
				return;
		}

		if (DownScroll >= 0) {
			if (TinselV2) {
				scrollPixelsY = sd.ySpeed;
				DownScroll -= sd.yDistance;
			} else {
				scrollPixelsY = SCROLLPIXELS;
				DownScroll = -UDSCROLL;
			}
		}
		break;
	}
}

/**
 * Called from scroll process - Scrolls the image as appropriate.
 */
static void ScrollImage() {
	int OldLoffset = 0, OldToffset = 0;	// Used when keeping cursor on a tag
	int Loffset, Toffset;
	int curX, curY;

	// get background offsets
	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	/*
	 * Keeping cursor on a tag?
	 */
	if (ScrollCursor) {
		GetCursorXYNoWait(&curX, &curY, true);
		if (InPolygon(curX, curY, TAG) != NOPOLY || InPolygon(curX, curY, EXIT) != NOPOLY) {
			OldLoffset = Loffset;
			OldToffset = Toffset;
		} else
			ScrollCursor = false;
	}

	/*
	 * Horizontal scrolling
	 */
	if (LeftScroll > 0) {
		LeftScroll -= scrollPixelsX;
		if (LeftScroll < 0) {
			Loffset += LeftScroll;
			LeftScroll = 0;
		}
		Loffset += scrollPixelsX;		// Move right
		if (Loffset > ImageW - SCREEN_WIDTH)
			Loffset = ImageW - SCREEN_WIDTH;// Now at extreme right

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MaximumXoffset) &&  (Loffset > SysVar(SV_MaximumXoffset)))
			Loffset = SysVar(SV_MaximumXoffset);

	} else if (LeftScroll < 0) {
		LeftScroll += scrollPixelsX;
		if (LeftScroll > 0) {
			Loffset += LeftScroll;
			LeftScroll = 0;
		}
		Loffset -= scrollPixelsX;	// Move left
		if (Loffset < 0)
			Loffset = 0;		// Now at extreme left

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MinimumXoffset) &&  (Loffset < SysVar(SV_MinimumXoffset)))
			Loffset = SysVar(SV_MinimumXoffset);
	}

	/*
	 * Vertical scrolling
	 */
	if (DownScroll > 0) {
		DownScroll -= scrollPixelsY;
		if (DownScroll < 0) {
			Toffset += DownScroll;
			DownScroll = 0;
		}
		Toffset += scrollPixelsY;		// Move down

		if (Toffset > ImageH - SCREEN_HEIGHT)
			Toffset = ImageH - SCREEN_HEIGHT;// Now at extreme bottom

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MaximumYoffset) &&  Toffset > SysVar(SV_MaximumYoffset))
			Toffset = SysVar(SV_MaximumYoffset);

	} else if (DownScroll < 0) {
		DownScroll += scrollPixelsY;
		if (DownScroll > 0) {
			Toffset += DownScroll;
			DownScroll = 0;
		}
		Toffset -= scrollPixelsY;		// Move up

		if (Toffset < 0)
			Toffset = 0;			// Now at extreme top

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MinimumYoffset) &&  Toffset < SysVar(SV_MinimumYoffset))
			Toffset = SysVar(SV_MinimumYoffset);
	}

	/*
	 * Move cursor if keeping cursor on a tag.
	 */
	if (ScrollCursor)
		AdjustCursorXY(OldLoffset - Loffset, OldToffset - Toffset);

	PlayfieldSetPos(FIELD_WORLD, Loffset, Toffset);
}


/**
 * See if the actor on whom the camera is is approaching an edge.
 * Request a scroll if he is.
 */
static void MonitorScroll() {
	int newx, newy;
	int Loffset, Toffset;

	/*
	 * Only do it if the actor is there and is visible
	 */
	if (!pScrollMover || MoverHidden(pScrollMover) || !MoverIs(pScrollMover))
		return;

	GetActorPos(scrollActor, &newx, &newy);

	if (oldx == newx && oldy == newy)
		return;

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	/*
	 * Approaching right side or left side of the screen?
	 */
	if (newx > Loffset+SCREEN_WIDTH - RLDISTANCE && Loffset < ImageW - SCREEN_WIDTH) {
		if (newx > oldx)
				NeedScroll(LEFT);
	} else if (newx < Loffset + RLDISTANCE  &&  Loffset) {
		if (newx < oldx)
				NeedScroll(RIGHT);
	}

	/*
	 * Approaching bottom or top of the screen?
	 */
	if (newy > Toffset+SCREEN_HEIGHT-DDISTANCE && Toffset < ImageH-SCREEN_HEIGHT) {
		if (newy > oldy)
				NeedScroll(UP);
	} else if (Toffset && newy < Toffset + UDISTANCE + GetActorBottom(scrollActor) - GetActorTop(scrollActor)) {
		if (newy < oldy)
				NeedScroll(DOWN);
	}

	oldx = newx;
	oldy = newy;
}

static void RestoreScrollDefaults() {
	sd.xTrigger		= SysVar(SV_SCROLL_XTRIGGER);
	sd.xDistance	= SysVar(SV_SCROLL_XDISTANCE);
	sd.xSpeed		= SysVar(SV_SCROLL_XSPEED);
	sd.yTriggerTop	= SysVar(SV_SCROLL_YTRIGGERTOP);
	sd.yTriggerBottom= SysVar(SV_SCROLL_YTRIGGERBOT);
	sd.yDistance	= SysVar(SV_SCROLL_YDISTANCE);
	sd.ySpeed		= SysVar(SV_SCROLL_YSPEED);
}

/**
 * Does the obvious - called at the end of a scene.
 */
void DropScroll() {
	sd.NumNoH = sd.NumNoV = 0;
	if (TinselV2) {
		LeftScroll = DownScroll = 0;		// No iterations outstanding
		oldx = oldy = 0;
		scrollPixelsX = sd.xSpeed;
		scrollPixelsY = sd.ySpeed;
		RestoreScrollDefaults();
	}
}

/**
 * Decide when to scroll and scroll when decided to.
 */
void ScrollProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// In Tinsel v2, scenes may play movies, so the background may not always
	// already be initialised like it is in v1
	while (!GetBgObject())
		CORO_SLEEP(1);

	ImageH = BgHeight();		// Dimensions
	ImageW = BgWidth();		//  of this scene.

	// Give up if there'll be no purpose in this process
	if (ImageW == SCREEN_WIDTH  &&  ImageH == SCREEN_HEIGHT)
		CORO_KILL_SELF();

	if (!TinselV2) {
		LeftScroll = DownScroll = 0;		// No iterations outstanding
		oldx = oldy = 0;
		scrollPixelsX = scrollPixelsY = SCROLLPIXELS;
	}

	if (!scrollActor)
		scrollActor = GetLeadId();

	pScrollMover = GetMover(scrollActor);

	while (1) {
		MonitorScroll();		// Set scroll requirement

		if (LeftScroll || DownScroll)	// Scroll if required
			ScrollImage();

		CORO_SLEEP(1);		// allow re-scheduling
	}

	CORO_END_CODE;
}

/**
 * Change which actor the camera is following.
 */
void ScrollFocus(int ano) {
	if (scrollActor != ano) {
		oldx = oldy = 0;
		scrollActor = ano;

		pScrollMover = ano ? GetMover(scrollActor) : NULL;
	}
}

/**
 * Returns the actor which the camera is following
 */
int GetScrollFocus() {
	return scrollActor;
}


/**
 * Scroll to abslote position.
 */
void ScrollTo(int x, int y, int xIter, int yIter) {
	int Loffset, Toffset;		// for background offsets

	scrollPixelsX = xIter != 0 ? xIter : (TinselV2 ? sd.xSpeed : SCROLLPIXELS);
	scrollPixelsY = yIter != 0 ? yIter : (TinselV2 ? sd.ySpeed : SCROLLPIXELS);

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);	// get background offsets

	LeftScroll = x - Loffset;
	DownScroll = y - Toffset;
}

/**
 * Kill of any current scroll.
 */
void KillScroll() {
	LeftScroll = DownScroll = 0;
}


void GetNoScrollData(SCROLLDATA *ssd) {
	memcpy(ssd, &sd, sizeof(SCROLLDATA));
}

void RestoreNoScrollData(SCROLLDATA *ssd) {
	memcpy(&sd, ssd, sizeof(SCROLLDATA));
}

/**
 * SetScrollParameters
 */
void SetScrollParameters(int xTrigger, int xDistance, int xSpeed, int yTriggerTop,
		int yTriggerBottom, int yDistance, int ySpeed) {
	if (xTrigger == 0 && xDistance == 0 && xSpeed == 0
	 && yTriggerTop == 0 && yTriggerBottom && yDistance == 0 && ySpeed == 0) {
		// Restore defaults
		RestoreScrollDefaults();
	} else {
		if (xTrigger)
			sd.xTrigger = xTrigger;
		if (xDistance)
			sd.xDistance = xDistance;
		if (xSpeed)
			sd.xSpeed = xSpeed;
		if (yTriggerTop)
			sd.yTriggerTop = yTriggerTop;
		if (yTriggerBottom)
			sd.yTriggerBottom = yTriggerBottom;
		if (yDistance)
			sd.yDistance = yDistance;
		if (ySpeed)
			sd.ySpeed = ySpeed;
	}
}

bool IsScrolling() {
	return (LeftScroll || DownScroll);
}

} // End of namespace Tinsel
