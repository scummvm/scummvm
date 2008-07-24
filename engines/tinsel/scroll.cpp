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

namespace Tinsel {

//----------------- EXTERNAL FUNCTIONS ---------------------

// in BG.C
extern int BackgroundWidth(void);
extern int BackgroundHeight(void);



//----------------- LOCAL DEFINES --------------------

#define LEFT	'L'
#define RIGHT	'R'
#define UP	'U'
#define DOWN	'D'



//----------------- LOCAL GLOBAL DATA --------------------

static int LeftScroll = 0, DownScroll = 0;	// Number of iterations outstanding

static int scrollActor = 0;
static PMACTOR psActor = 0;
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
		0
	};

static int ImageH = 0, ImageW = 0;

static bool ScrollCursor = 0;	// If a TAG or EXIT polygon is clicked on,
				// the cursor is kept over that polygon
				// whilst scrolling

static int scrollPixels = SCROLLPIXELS;


/**
 * Reset the ScrollCursor flag
 */
void DontScrollCursor(void) {
	ScrollCursor = false;
}

/**
 * Set the ScrollCursor flag
 */
void DoScrollCursor(void) {
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
 * Does the obvious - called at the end of a scene.
 */
void DropNoScrolls(void) {
	sd.NumNoH = sd.NumNoV = 0;
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
			scrollPixels = SCROLLPIXELS;
			LeftScroll = RLSCROLL;
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
			scrollPixels = SCROLLPIXELS;
			LeftScroll = -RLSCROLL;
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
			scrollPixels = SCROLLPIXELS;
			DownScroll = UDSCROLL;
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
			scrollPixels = SCROLLPIXELS;
			DownScroll = -UDSCROLL;
		}
		break;
	}
}

/**
 * Called from scroll process - Scrolls the image as appropriate.
 */
static void ScrollImage(void) {
	int OldLoffset = 0, OldToffset = 0;	// Used when keeping cursor on a tag
	int Loffset, Toffset;
	int curX, curY;

	// get background offsets
	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	/*
	 * Keeping cursor on a tag?
	 */
	if (ScrollCursor) {
		GetCursorXY(&curX, &curY, true);
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
		LeftScroll -= scrollPixels;
		if (LeftScroll < 0) {
			Loffset += LeftScroll;
			LeftScroll = 0;
		}
		Loffset += scrollPixels;		// Move right
		if (Loffset > ImageW - SCREEN_WIDTH)
			Loffset = ImageW - SCREEN_WIDTH;// Now at extreme right
	} else if (LeftScroll < 0) {
		LeftScroll += scrollPixels;
		if (LeftScroll > 0) {
			Loffset += LeftScroll;
			LeftScroll = 0;
		}
		Loffset -= scrollPixels;	// Move left
		if (Loffset < 0)
			Loffset = 0;		// Now at extreme left
	}

	/*
	 * Vertical scrolling
	 */
	if (DownScroll > 0) {
		DownScroll -= scrollPixels;
		if (DownScroll < 0) {
			Toffset += DownScroll;
			DownScroll = 0;
		}
		Toffset += scrollPixels;		// Move down

		if (Toffset > ImageH - SCREEN_HEIGHT)
			Toffset = ImageH - SCREEN_HEIGHT;// Now at extreme bottom

	} else if (DownScroll < 0) {
		DownScroll += scrollPixels;
		if (DownScroll > 0) {
			Toffset += DownScroll;
			DownScroll = 0;
		}
		Toffset -= scrollPixels;		// Move up

		if (Toffset < 0)
			Toffset = 0;			// Now at extreme top
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
static void MonitorScroll(void) {
	int newx, newy;
	int Loffset, Toffset;

	/*
	 * Only do it if the actor is there and is visible
	 */
	if (!psActor || getMActorHideState(psActor)
			|| getMActorState(psActor) == NO_MACTOR)
		return;

	GetActorPos(scrollActor, &newx, &newy);

	if (oldx == newx && oldy == newy)
		return;

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	/*
	 * Approaching right side or left side of the screen?
	 */
	if (newx > Loffset+SCREEN_WIDTH-RLDISTANCE && Loffset < ImageW-SCREEN_WIDTH) {
		if (newx > oldx)
				NeedScroll(LEFT);
	} else if (newx < Loffset + RLDISTANCE  &&  Loffset) {
		if (newx < oldx)
				NeedScroll(RIGHT);
	}

	/*
	 * Approaching bottom or top of the screen?
	 */
	if (newy > Toffset+SCREEN_HEIGHT-UDDISTANCE && Toffset < ImageH-SCREEN_HEIGHT) {
		if (newy > oldy)
				NeedScroll(UP);
	} else if (Toffset && newy < Toffset + UDDISTANCE + GetActorBottom(scrollActor) - GetActorTop(scrollActor)) {
		if (newy < oldy)
				NeedScroll(DOWN);
	}

	oldx = newx;
	oldy = newy;
}

/**
 * Decide when to scroll and scroll when decided to.
 */
void ScrollProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	ImageH = BackgroundHeight();		// Dimensions
	ImageW = BackgroundWidth();		//  of this scene.

	// Give up if there'll be no purpose in this process
	if (ImageW == SCREEN_WIDTH  &&  ImageH == SCREEN_HEIGHT)
		CORO_KILL_SELF();

	LeftScroll = DownScroll = 0;		// No iterations outstanding
	oldx = oldy = 0;
	scrollPixels = SCROLLPIXELS;

	if (!scrollActor)
		scrollActor = LeadId();

	psActor = GetMover(scrollActor);

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

		psActor = ano ? GetMover(scrollActor) : NULL;
	}
}

/**
 * Scroll to abslote position.
 */
void ScrollTo(int x, int y, int iter) {
	int Loffset, Toffset;		// for background offsets

	scrollPixels = iter != 0 ? iter : SCROLLPIXELS;

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);	// get background offsets

	LeftScroll = x - Loffset;
	DownScroll = y - Toffset;
}

/**
 * Kill of any current scroll.
 */
void KillScroll(void) {
	LeftScroll = DownScroll = 0;
}


void GetNoScrollData(SCROLLDATA *ssd) {
	memcpy(ssd, &sd, sizeof(SCROLLDATA));
}

void RestoreNoScrollData(SCROLLDATA *ssd) {
	memcpy(&sd, ssd, sizeof(SCROLLDATA));
}

} // end of namespace Tinsel
