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
 * Handles walking and use of the path system.
 *
 * Contains the dodgiest code in the whole system.
 */

#include "tinsel/actors.h"
#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/graphics.h"
#include "tinsel/move.h"
#include "tinsel/multiobj.h"	// multi-part object defintions etc.
#include "tinsel/object.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/scroll.h"
#include "tinsel/tinlib.h"	// For stand()

namespace Tinsel {

//----------------- DEVELOPMENT OPTIONS --------------------

#define SLOW_RINCE_DOWN		0

//----------------- EXTERNAL FUNCTIONS ---------------------

// in BG.C
extern int BackgroundWidth(void);
extern int BackgroundHeight(void);


// in POLYGONS.C
// Deliberatley defined here, and not in polygons.h
HPOLYGON InitExtraBlock(PMACTOR ca, PMACTOR ta);

//----------------- LOCAL DEFINES --------------------

#define XMDIST	4
#define XHMDIST	2
#define YMDIST	2
#define YHMDIST	2

#define XTHERE		1
#define XRESTRICT	2
#define YTHERE		4
#define YRESTRICT	8
#define STUCK		16

#define LEAVING_PATH	0x100
#define ENTERING_BLOCK	0x200
#define ENTERING_MBLOCK	0x400

#define ALL_SORTED	1
#define NOT_SORTED	0


//----------------- LOCAL GLOBAL DATA --------------------

#if SLOW_RINCE_DOWN
static int Interlude = 0;	// For slowing down walking, for testing
static int BogusVar = 0;	// For slowing down walking, for testing
#endif

static int32 DefaultRefer = 0;
static int hSlowVar = 0;	// used by MoveActor()


//----------------- FORWARD REFERENCES --------------------

static void NewCoOrdinates(int fromx, int fromy, int *targetX, int *targetY,
			int *newx, int *newy, int *s1, int *s2, HPOLYGON *hS2p,
			bool bOver, bool bBodge,
			PMACTOR pActor, PMACTOR *collisionActor = 0);


#if SLOW_RINCE_DOWN
/**
 * AddInterlude
 */

void AddInterlude(int n) {
	Interlude += n;
	if (Interlude < 0)
		Interlude = 0;
}
#endif

/**
 * Given (x, y) of a click within a path polygon, checks that the
 * co-ordinates are not within a blocking polygon. If it is not, the
 * destination is the click point, otherwise tries to find a legal point
 * below or above the click point.
 * Returns:
 *	NOT_SORTED - if a destination is worked out (movement required)
 *	ALL_SORTED - no destination found (so no movement required)
 */
static int ClickedOnPath(int clickX, int clickY, int *ptgtX, int *ptgtY) {
	int Loffset, Toffset;
	int i;

	/*--------------------------------------
	 Clicked within a path,
	 go to where requested unless blocked.
	 --------------------------------------*/
	if (InPolygon(clickX, clickY, BLOCKING) == NOPOLY) {
		// Not in a blocking polygon - go to where requested.
		*ptgtX = clickX;
		*ptgtY = clickY;
	} else {
		/*------------------------------------------------------
		 In a Blocking polygon - try searching down and up.
		 If still nowhere (for now) give up!
		 ------------------------------------------------------*/
		PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

		for (i = clickY+1; i < SCREEN_HEIGHT + Toffset; i++) {
			// Don't leave the path system
			if (InPolygon(clickX, i, PATH) == NOPOLY) {
				i = SCREEN_HEIGHT;
				break;
			}
			if (InPolygon(clickX, i, BLOCKING) == NOPOLY) {
				*ptgtX = clickX;
				*ptgtY = i;
				break;
			}
		}
		if (i == SCREEN_HEIGHT) {
			for (i = clickY-1; i >= Toffset; i--) {
				// Don't leave the path system
				if (InPolygon(clickX, i, PATH) == NOPOLY) {
					i = -1;
					break;
				}
				if (InPolygon(clickX, i, BLOCKING) == NOPOLY) {
					*ptgtX = clickX;
					*ptgtY = i;
					break;
				}
			}
		}
		if (i < 0) {
			return ALL_SORTED;
		}
	}
	return NOT_SORTED;
}

/**
 * Given (x, y) of a click within a referral polygon, works out the
 * destination according to the referral type.
 * Returns:
 *   NOT_SORTED - if a destination is worked out (movement required)
 *   ALL_SORTED - no destination found (so no movement required)
 */
static int ClickedOnRefer(HPOLYGON hRefpoly, int clickX, int clickY, int *ptgtX, int *ptgtY) {
	int	i;
	int	end;		// Extreme of the scene
	int	Loffset, Toffset;

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
	*ptgtX = *ptgtY = -1;

	switch (PolySubtype(hRefpoly)) {
	case REF_POINT:				// Go to specified node
		getPolyNode(hRefpoly, ptgtX, ptgtY);
		assert(InPolygon(*ptgtX, *ptgtY, PATH) != NOPOLY); // POINT Referral to illegal point
		break;

	case REF_DOWN:				// Search downwards
		end = BackgroundHeight();
		for (i = clickY+1; i < end; i++)
			if (InPolygon(clickX, i, PATH) != NOPOLY
					&& InPolygon(clickX, i, BLOCKING) == NOPOLY) {
				*ptgtX = clickX;
				*ptgtY = i;
				break;
			}
		break;

	case REF_UP:				// Search upwards
		for (i = clickY-1; i >= 0; i--)
			if (InPolygon(clickX, i, PATH) != NOPOLY
					&& InPolygon(clickX, i, BLOCKING) == NOPOLY) {
				*ptgtX = clickX;
				*ptgtY = i;
				break;
			}
		break;

	case REF_RIGHT:				// Search to the right
		end = BackgroundWidth();
		for (i = clickX+1; i < end; i++)
			if (InPolygon(i, clickY, PATH) != NOPOLY
			&& InPolygon(i, clickY, BLOCKING) == NOPOLY) {
				*ptgtX = i;
				*ptgtY = clickY;
				break;
			}
		break;

	case REF_LEFT:				// Search to the left
		for (i = clickX-1; i >= 0; i--)
			if (InPolygon(i, clickY, PATH) != NOPOLY
			&& InPolygon(i, clickY, BLOCKING) == NOPOLY) {
				*ptgtX = i;
				*ptgtY = clickY;
				break;
			}
		break;
	}
	if (*ptgtX != -1 && *ptgtY != -1) {
		return NOT_SORTED;
	} else
		return ALL_SORTED;
}

/**
 * Given (x, y) of a click, works out the destination according to the
 * default referral type.
 * Returns:
 *   NOT_SORTED - if a destination is worked out (movement required)
 *   ALL_SORTED - no destination found (so no movement required)
 */
static int ClickedOnNothing(int clickX, int clickY, int *ptgtX, int *ptgtY) {
	int	i;
	int	end;		// Extreme of the scene
	int	Loffset, Toffset;

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	switch (DefaultRefer) {
	case REF_DEFAULT:
		// Try searching down and up (onscreen).
		for (i = clickY+1; i < SCREEN_HEIGHT+Toffset; i++)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		for (i = clickY-1; i >= Toffset; i--)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		// Try searching down and up (offscreen).
		end = BackgroundHeight();
		for (i = clickY+1; i < end; i++)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		for (i = clickY-1; i >= 0; i--)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		break;

	case REF_UP:
		for (i = clickY-1; i >= 0; i--)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		break;

	case REF_DOWN:
		end = BackgroundHeight();
		for (i = clickY+1; i < end; i++)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		break;

	case REF_LEFT:
		for (i = clickX-1; i >= 0; i--)
			if (InPolygon(i, clickY, PATH) != NOPOLY) {
				return ClickedOnPath(i, clickY, ptgtX, ptgtY);
			}
		break;

	case REF_RIGHT:
		end = BackgroundWidth();
		for (i = clickX + 1; i < end; i++)
			if (InPolygon(i, clickY, PATH) != NOPOLY) {
				return ClickedOnPath(i, clickY, ptgtX, ptgtY);
			}
		break;
	}

	// Going nowhere!
	return ALL_SORTED;
}

/**
 * Given (x, y) of the click, ascertains whether the click is within a
 * path, within a referral poly, or niether. The appropriate function
 * then gets called to give us a revised destination.
 * Returns:
 *   NOT_SORTED - if a destination is worked out (movement required)
 *   ALL_SORTED - no destination found (so no movement required)
 */
static int WorkOutDestination(int clickX, int clickY, int *ptgtX, int *ptgtY) {
	HPOLYGON hPoly;

	/*--------------------------------------
	 Clicked within a path?
	 if not, within a referral poly?
	 if not, try and sort something out.
	---------------------------------------*/
	if (InPolygon(clickX, clickY, PATH) != NOPOLY) {
		return ClickedOnPath(clickX, clickY, ptgtX, ptgtY);
	} else if ((hPoly = InPolygon(clickX, clickY, REFER)) != NOPOLY) {
		return ClickedOnRefer(hPoly, clickX, clickY, ptgtX, ptgtY);
	} else {
		return ClickedOnNothing(clickX, clickY, ptgtX, ptgtY);
	}
}

/**
 * Work out which reel to adopt for a section of movement.
 */
static DIRREEL GetDirectionReel(int fromx, int fromy, int tox, int toy, DIRREEL lastreel, HPOLYGON hPath) {
	int	xchange = 0, ychange = 0;
	enum {X_NONE, X_LEFT, X_RIGHT, X_NO} xdir;
	enum {Y_NONE, Y_UP, Y_DOWN, Y_NO} ydir;

	DIRREEL	reel = lastreel;	// Leave alone if can't decide

	/*
	 * Determine size and direction of X movement.
	 * i.e. left, right, none or not allowed.
	 */
	if (getPolyReelType(hPath) == REEL_VERT)
		xdir = X_NO;
	else if (tox == -1)
		xdir = X_NONE;
	else {
		xchange = tox - fromx;
		if (xchange > 0)
			xdir = X_RIGHT;
		else if (xchange < 0) {
			xchange = -xchange;
			xdir = X_LEFT;
		} else
			xdir = X_NONE;
	}

	/*
	 * Determine size and direction of Y movement.
	 * i.e. up, down, none or not allowed.
	 */
	if (getPolyReelType(hPath) == REEL_HORIZ)
		ydir = Y_NO;
	else if (toy == -1)
		ydir = Y_NONE;
	else {
		ychange = toy - fromy;
		if (ychange > 0)
			ydir = Y_DOWN;
		else if (ychange < 0) {
			ychange = -ychange;
			ydir = Y_UP;
		} else
			ydir = Y_NONE;
	}

	/*
	 * Some adjustment to allow for different x and y pixell sizes.
	 */
	ychange += ychange;	// Double y distance to cover

	/*
	 * Determine which reel to use.
	 */
	if (xdir == X_NO) {
		// Forced to be FORWARD or AWAY
		switch (ydir) {
		case Y_DOWN:
			reel = FORWARD;
			break;
		case Y_UP:
			reel = AWAY;
			break;
		default:
			if (reel != AWAY)	// No gratuitous turn
				reel = FORWARD;
			break;
		}
	} else if (ydir == Y_NO) {
		// Forced to be LEFTREEL or RIGHTREEL
		switch (xdir) {
		case X_LEFT:
			reel = LEFTREEL;
			break;
		case X_RIGHT:
			reel = RIGHTREEL;
			break;
		default:
			if (reel != LEFTREEL)	// No gratuitous turn
				reel = RIGHTREEL;
			break;
		}
	} else if (xdir != X_NONE || ydir != Y_NONE) {
		if (xdir == X_NONE)
			reel = (ydir == Y_DOWN) ? FORWARD : AWAY;
		else if (ydir == Y_NONE)
			reel = (xdir == X_LEFT) ? LEFTREEL : RIGHTREEL;
		else {
			bool DontBother = false;

			if (xchange <= 4 && ychange <= 4) {
				switch (reel) {
				case LEFTREEL:
					if (xdir == X_LEFT)
						DontBother = true;
					break;
				case RIGHTREEL:
					if (xdir == X_RIGHT)
						DontBother = true;
					break;
				case FORWARD:
					if (ydir == Y_DOWN)
						DontBother = true;
					break;
				case AWAY:
					if (ydir == Y_UP)
						DontBother = true;
					break;
				}
			}
			if (!DontBother) {
				if (xchange > ychange)
					reel = (xdir == X_LEFT) ? LEFTREEL : RIGHTREEL;
				else
					reel = (ydir == Y_DOWN) ? FORWARD : AWAY;
			}
		}
	}
	return reel;
}

/**
 * Haven't moved, look towards the cursor.
 */
static void GotThereWithoutMoving(PMACTOR pActor) {
	int	curX, curY;
	DIRREEL	reel;

	if (!pActor->TagReelRunning) {
		GetCursorXYNoWait(&curX, &curY, true);

		reel = GetDirectionReel(pActor->objx, pActor->objy, curX, curY, pActor->dirn, pActor->hCpath);

		if (reel != pActor->dirn)
			SetMActorWalkReel(pActor, reel, pActor->scale, false);
	}
}

/**
 * Arrived at final destination.
 */
static void GotThere(PMACTOR pActor) {
	pActor->targetX = pActor->targetY = -1;		// 4/1/95
	pActor->ItargetX = pActor->ItargetY = -1;
	pActor->UtargetX = pActor->UtargetY = -1;

	// Perhaps we have'nt moved.
	if (pActor->objx == (int)pActor->fromx && pActor->objy == (int)pActor->fromy)
		GotThereWithoutMoving(pActor);

	ReTagActor(pActor->actorID);	// Tag allowed while stationary

	SetMActorStanding(pActor);

	pActor->bMoving = false;
}

enum cgt { GT_NOTL, GT_NOTB, GT_NOT2, GT_OK, GT_MAY };

/**
 * Can we get straight there?
 */
static cgt CanGetThere(PMACTOR pActor, int tx, int ty) {
	int s1, s2;		// s2 not used here!
	HPOLYGON hS2p;		// nor is s2p!
	int nextx, nexty;

	int targetX = tx;
	int targetY = ty;		// Ultimate destination
	int x = pActor->objx;
	int y = pActor->objy;		// Present position

	while (targetX != -1 || targetY != -1) {
		NewCoOrdinates(x, y, &targetX, &targetY, &nextx, &nexty,
				&s1, &s2, &hS2p, pActor->over, false, pActor);

		if (s1 == (XTHERE | YTHERE)) {
			return GT_OK;	// Can get there directly.
		} else if (s1 == (XTHERE | YRESTRICT) || s1 == (YTHERE | XRESTRICT)) {
			return GT_MAY;	// Can't get there directly.
		} else if (s1 & STUCK) {
			if (s2 == LEAVING_PATH)
				return GT_NOTL;	// Can't get there.
			else
				return GT_NOTB;	// Can't get there.
		} else if (x == nextx && y == nexty) {
			return GT_NOT2;	// Can't get there.
		}
		x = nextx;
		y = nexty;
	}
	return GT_MAY;
}


/**
 * Set final destination.
 */
static void SetMoverUltDest(PMACTOR pActor, int x, int y) {
	pActor->UtargetX = x;
	pActor->UtargetY = y;
	pActor->hUpath = InPolygon(x, y, PATH);

	assert(pActor->hUpath != NOPOLY || pActor->bIgPath); // Invalid ultimate destination
}

/**
 * Set intermediate destination.
 *
 * If in final destination path, go straight to target.
 * If in a neighbouring path to the final destination, if the target path
 * is a follow nodes path, head for the end node, otherwise head straight
 * for the target.
 * Otherwise, head towards the pseudo-centre or end node of the first
 * en-route path.
 */
static void SetMoverIntDest(PMACTOR pActor, int x, int y) {
	HPOLYGON hIpath, hTpath;
	int	node;

	hTpath = InPolygon(x, y, PATH);		// Target path
#ifdef DEBUG
	if (!pActor->bIgPath)
		assert(hTpath != NOPOLY); // SetMoverIntDest() - target not in path
#endif

	if (pActor->hCpath == hTpath || pActor->bIgPath
		|| IsInPolygon(pActor->objx, pActor->objy, hTpath)) {
		// In destination path - head straight for the target.
		pActor->ItargetX = x;
		pActor->ItargetY = y;
		pActor->hIpath = hTpath;
	} else if (IsAdjacentPath(pActor->hCpath, hTpath)) {
		// In path adjacent to target
		if (PolySubtype(hTpath) != NODE) {
			// Target path is normal - head for target.
			// Added 26/01/95, innroom
			if (CanGetThere(pActor, x, y) == GT_NOTL) {
				NearestCorner(&x, &y, pActor->hCpath, hTpath);
			}
			pActor->ItargetX = x;
			pActor->ItargetY = y;
		} else {
			// Target path is node - head for end node.
			node = NearestEndNode(hTpath, pActor->objx, pActor->objy);
			getNpathNode(hTpath, node, &pActor->ItargetX, &pActor->ItargetY);

		}
		pActor->hIpath = hTpath;
	} else {
		assert(hTpath != NOPOLY); // Error 701
		hIpath = getPathOnTheWay(pActor->hCpath, hTpath);

		if (hIpath != NOPOLY) {
			/* Head for an en-route path */
			if (PolySubtype(hIpath) != NODE) {
				/* En-route path is normal - head for pseudo centre. */
				if (CanGetThere(pActor, x, y) == GT_OK) {
					pActor->ItargetX = x;
					pActor->ItargetY = y;
				} else {
					pActor->ItargetX = PolyCentreX(hIpath);
					pActor->ItargetY = PolyCentreY(hIpath);
				}
			} else {
				/* En-route path is node - head for end node. */
				node = NearestEndNode(hIpath, pActor->objx, pActor->objy);
				getNpathNode(hIpath, node, &pActor->ItargetX, &pActor->ItargetY);
			}
			pActor->hIpath = hIpath;
		}
	}

	pActor->InDifficulty = NO_PROB;
}

/**
 * Set short-term destination and adopt the appropriate reel.
 */
static void SetMoverDest(PMACTOR pActor, int x, int y) {
	int	scale;
	DIRREEL	reel;

	// Set the co-ordinates requested.
	pActor->targetX = x;
	pActor->targetY = y;
	pActor->InDifficulty = NO_PROB;

	reel = GetDirectionReel(pActor->objx, pActor->objy, x, y, pActor->dirn, pActor->hCpath);
	scale = GetScale(pActor->hCpath, pActor->objy);
	if (scale != pActor->scale || reel != pActor->dirn) {
		SetMActorWalkReel(pActor, reel, scale, false);
	}
}

/**
 * SetNextDest
 */
static void SetNextDest(PMACTOR pActor) {
	int	targetX, targetY;		// Ultimate destination
	int	x, y;				// Present position
	int	nextx, nexty;
	int	s1, lstatus = 0;
	int	s2;
	HPOLYGON	hS2p;
	int	i;
	HPOLYGON	hNpoly;
	HPOLYGON	hPath;
	int	znode;
	int	nx, ny;
	int	sx, sy;
	HPOLYGON	hEb;

	int	ss1, ss2;
	HPOLYGON shS2p;
	PMACTOR collisionActor;
#if 1
	int	sTargetX, sTargetY;
#endif

	/*
	 * Desired destination (Itarget) is already set
	 */
	x = pActor->objx;		// Current position
	y = pActor->objy;
	targetX = pActor->ItargetX;	// Desired position
	targetY = pActor->ItargetY;

	/*
	 * If we're where we're headed, end it all (the moving).
	 */
//	if (x == targetX && y == targetY)
	if (ABS(x - targetX) < XMDIST && ABS(y - targetY) < YMDIST) {
		if (targetX == pActor->UtargetX && targetY == pActor->UtargetY) {
			// Desired position
			GotThere(pActor);
			return;
		} else {
 			assert(pActor->bIgPath || InPolygon(pActor->UtargetX, pActor->UtargetY, PATH) != NOPOLY); // Error 5001
			SetMoverIntDest(pActor, pActor->UtargetX, pActor->UtargetY);
		}
	}

	if (pActor->bNoPath || pActor->bIgPath) {
		/* Can get there directly. */
		SetMoverDest(pActor, targetX, targetY);
		pActor->over = false;
		return;
	}

	/*----------------------------------------------------------------------
	| Some work to do here if we're in a follow nodes polygon - basically
	| head for the next node.
	----------------------------------------------------------------------*/
	hNpoly = pActor->hFnpath;		// The node path we're in (if any)
	switch (pActor->npstatus) {
	case NOT_IN:
		break;

	case ENTERING:
		znode = NearestEndNode(hNpoly, x, y);
		/* Hang on, we're probably here already! */
		if (znode) {
			pActor->npstatus = GOING_DOWN;
			pActor->line = znode-1;
			getNpathNode(hNpoly, znode - 1, &nx, &ny);
		} else {
			pActor->npstatus = GOING_UP;
			pActor->line = znode;
			getNpathNode(hNpoly, 1, &nx, &ny);
		}
		SetMoverDest(pActor, nx, ny);

		// Test for pseudo-one-node npaths
		if (numNodes(hNpoly) == 2 &&
				ABS(pActor->objx - pActor->targetX) < XMDIST &&
				ABS(pActor->objy - pActor->targetY) < YMDIST) {
			// That's enough, we're leaving
			pActor->npstatus = LEAVING;
		} else {
			// Normal situation
			pActor->over = true;
			return;
		}
		// Fall through for LEAVING

	case LEAVING:
 		assert(pActor->bIgPath || InPolygon(pActor->UtargetX, pActor->UtargetY, PATH) != NOPOLY); // Error 5002
		SetMoverIntDest(pActor, pActor->UtargetX, pActor->UtargetY);
		targetX = pActor->ItargetX;	// Desired position
		targetY = pActor->ItargetY;
		break;

	case GOING_UP:
		i = pActor->line;		// The line we're on

		// Is this the final target line?
		if (i+1 == pActor->Tline && hNpoly == pActor->hUpath) {
			// The final leg of the journey
			pActor->line = i+1;
			SetMoverDest(pActor, pActor->UtargetX, pActor->UtargetY);
			pActor->over = false;
			return;
		} else {
			// Go to the next node unless we're at the last one
			i++;				// The node we're at
			if (++i < numNodes(hNpoly)) {
				getNpathNode(hNpoly, i, &nx, &ny);
				SetMoverDest(pActor, nx, ny);
				pActor->line = i-1;
				if (ABS(pActor->UtargetX - pActor->targetX) < XMDIST &&
				   ABS(pActor->UtargetY - pActor->targetY) < YMDIST)
					pActor->over = false;
				else
					pActor->over = true;
				return;
			} else {
				// Last node - we're off
				pActor->npstatus = LEAVING;
		 		assert(pActor->bIgPath || InPolygon(pActor->UtargetX, pActor->UtargetY, PATH) != NOPOLY); // Error 5003
				SetMoverIntDest(pActor, pActor->UtargetX, pActor->UtargetY);
				targetX = pActor->ItargetX;	// Desired position
				targetY = pActor->ItargetY;
				break;
			}
		}

	case GOING_DOWN:
		i = pActor->line;		// The line we're on and the node we're at

		// Is this the final target line?
		if (i - 1 == pActor->Tline && hNpoly == pActor->hUpath) {
			// The final leg of the journey
			SetMoverDest(pActor, pActor->UtargetX, pActor->UtargetY);
			pActor->line = i-1;
			pActor->over = false;
			return;
		} else {
			// Go to the next node unless we're at the last one
			if (--i >= 0) {
				getNpathNode(hNpoly, i, &nx, &ny);
				SetMoverDest(pActor, nx, ny);
				pActor->line--;		/* The next node to head for */
				if (ABS(pActor->UtargetX - pActor->targetX) < XMDIST &&
				   ABS(pActor->UtargetY - pActor->targetY) < YMDIST)
					pActor->over = false;
				else
					pActor->over = true;
				return;
			} else {
				// Last node - we're off
				pActor->npstatus = LEAVING;
		 		assert(pActor->bIgPath || InPolygon(pActor->UtargetX, pActor->UtargetY, PATH) != NOPOLY); // Error 5004
				SetMoverIntDest(pActor, pActor->UtargetX, pActor->UtargetY);
				targetX = pActor->ItargetX;	// Desired position
				targetY = pActor->ItargetY;
				break;
			}
		}
	}




	/*------------------------------------------------------
	| See if it can get there directly. There may be an
	| intermediate destination to head for.
	------------------------------------------------------*/

	while (targetX != -1 || targetY != -1) {
#if 1
		// 'push' the target
		sTargetX = targetX;
		sTargetY = targetY;
#endif
		NewCoOrdinates(x, y, &targetX, &targetY, &nextx, &nexty,
					&s1, &s2, &hS2p, pActor->over, false, pActor, &collisionActor);

		if (s1 != (XTHERE | YTHERE) && x == nextx && y == nexty) {
			ss1 = s1;
			ss2 = s2;
			shS2p = hS2p;
#if 1
			// 'pop' the target
			targetX = sTargetX;
			targetY = sTargetY;
#endif
			// Note: this aint right - targetX/Y (may) have been
			// nobbled by that last call to NewCoOrdinates()
			// Re-instating them (can) leads to oscillation
			NewCoOrdinates(x, y, &targetX, &targetY, &nextx, &nexty,
						&s1, &s2, &hS2p, pActor->over, true, pActor, &collisionActor);

			if (x == nextx && y == nexty) {
				s1 = ss1;
				s2 = ss2;
				hS2p = shS2p;
			}
		}

		if (s1 == (XTHERE | YTHERE)) {
			/* Can get there directly. */
			SetMoverDest(pActor, nextx, nexty);
			pActor->over = false;
			break;
		} else if ((s1 & STUCK) || s1 == (XRESTRICT + YRESTRICT)
		     || s1 == (XTHERE | YRESTRICT) || s1 == (YTHERE | XRESTRICT)) {
			/*-------------------------------------------------
			 Can't go any further in this direction.	   |
			 If it's because of a blocking polygon, try to do |
			 something about it.				   |
			 -------------------------------------------------*/
			if (s2 & ENTERING_BLOCK) {
				x = pActor->objx;	// Current position
				y = pActor->objy;
				// Go to the nearest corner of the blocking polygon concerned
				BlockingCorner(hS2p, &x, &y, pActor->ItargetX, pActor->ItargetY);
				SetMoverDest(pActor, x, y);
				pActor->over = false;
			} else if (s2 & ENTERING_MBLOCK) {
				if (InMActorBlock(pActor, pActor->UtargetX, pActor->UtargetY)) {
					// The best we're going to achieve
					SetMoverUltDest(pActor, x, y);
					SetMoverDest(pActor, x, y);
				} else {
					sx = pActor->objx;
					sy = pActor->objy;
//					pActor->objx = x;
//					pActor->objy = y;

					hEb = InitExtraBlock(pActor, collisionActor);
					x = pActor->objx;
					y = pActor->objy;
					BlockingCorner(hEb, &x, &y, pActor->ItargetX, pActor->ItargetY);

					pActor->objx = sx;
					pActor->objy = sy;
					SetMoverDest(pActor, x, y);
					pActor->over = false;
				}
			} else {
				/*----------------------------------------
				 Currently, this is as far as we can go. |
				 Definitely room for improvement here!   |
				 ----------------------------------------*/
				hPath = InPolygon(pActor->ItargetX, pActor->ItargetY, PATH);
				if (hPath != pActor->hIpath) {
					if (IsInPolygon(pActor->ItargetX, pActor->ItargetY, pActor->hIpath))
						hPath = pActor->hIpath;
				}
				assert(hPath == pActor->hIpath);

				if (pActor->InDifficulty == NO_PROB) {
					x = PolyCentreX(hPath);
					y = PolyCentreY(hPath);
					SetMoverDest(pActor, x, y);
					pActor->InDifficulty = TRY_CENTRE;
					pActor->over = false;
				} else if (pActor->InDifficulty == TRY_CENTRE) {
					NearestCorner(&x, &y, pActor->hCpath, pActor->hIpath);
					SetMoverDest(pActor, x, y);
					pActor->InDifficulty = TRY_CORNER;
					pActor->over = false;
				} else if (pActor->InDifficulty == TRY_CORNER) {
					NearestCorner(&x, &y, pActor->hCpath, pActor->hIpath);
					SetMoverDest(pActor, x, y);
					pActor->InDifficulty = TRY_NEXTCORNER;
					pActor->over = false;
				}
			}
			break;
		}
		else if (((lstatus & YRESTRICT) && !(s1 & YRESTRICT))
		     ||  ((lstatus & XRESTRICT) && !(s1 & XRESTRICT))) {
			/*-----------------------------------------------
			 A restriction in a direction has been removed. |
			 Use this as an intermediate destination.	 |
			 -----------------------------------------------*/
			SetMoverDest(pActor, nextx, nexty);
			pActor->over = false;
			break;
		}

		x = nextx;
		y = nexty;

		/*-------------------------
		 Change of path polygon?  |
		 -------------------------*/
		hPath = InPolygon(x, y, PATH);
		if (pActor->hCpath != hPath &&
		   !IsInPolygon(x, y, pActor->hCpath) &&
		   !IsAdjacentPath(pActor->hCpath, pActor->hIpath)) {
			/*----------------------------------------------------------
			 If just entering a follow nodes polygon, go to first node.|
			 Else if just going to pass through, go to pseudo-centre.  |
			 ----------------------------------------------------------*/
			if (PolySubtype(hPath) == NODE && pActor->hFnpath != hPath && pActor->npstatus != LEAVING) {
				int node = NearestEndNode(hPath, x, y);
				getNpathNode(hPath, node, &nx, &ny);
				SetMoverDest(pActor, nx, ny);
				pActor->over = true;
			} else if (!IsInPolygon(pActor->ItargetX, pActor->ItargetY, hPath) &&
				!IsInPolygon(pActor->ItargetX, pActor->ItargetY, pActor->hCpath)) {
				SetMoverDest(pActor, PolyCentreX(hPath), PolyCentreY(hPath));
				pActor->over = true;
			} else {
				SetMoverDest(pActor, pActor->ItargetX, pActor->ItargetY);
			}
			break;
		}

		lstatus = s1;
	}
}

/**
 * Work out where the next position should be.
 * Check that it's in a path and not in a blocking polygon.
 */
static void NewCoOrdinates(int fromx, int fromy, int *targetX, int *targetY,
				int *newx, int *newy, int *s1, int *s2,
				HPOLYGON *hS2p, bool bOver, bool bBodge,
				PMACTOR pActor, PMACTOR *collisionActor) {
	HPOLYGON hPoly;
	int sidem, depthm;
	int sidesteps, depthsteps;
	PMACTOR	ma;

	*s1 = *s2 = 0;

	/*------------------------------------------------
	 Don't overrun if this is the final destination. |
	 ------------------------------------------------*/
	if (*targetX == pActor->UtargetX && (*targetY == -1 || *targetY == pActor->UtargetY) ||
			*targetY == pActor->UtargetY && (*targetX == -1 || *targetX == pActor->UtargetX))
		bOver = false;

	/*----------------------------------------------------
	 Decide how big a step to attempt in each direction. |
	 ----------------------------------------------------*/
	sidesteps = *targetX == -1 ? 0 : *targetX - fromx;
	sidesteps = ABS(sidesteps);

	depthsteps = *targetY == -1 ? 0 : *targetY - fromy;
	depthsteps = ABS(depthsteps);

	if (sidesteps && depthsteps > sidesteps) {
		depthm = YMDIST;
		sidem = depthm * sidesteps/depthsteps;

		if (!sidem)
			sidem = 1;
	} else if (depthsteps && sidesteps > depthsteps) {
		sidem = XMDIST;
		depthm = sidem * depthsteps/sidesteps;

		if (!depthm) {
			if (bBodge)
				depthm = 1;
		} else if (depthm > YMDIST)
			depthm = YMDIST;
	} else {
		sidem = sidesteps ? XMDIST : 0;
		depthm = depthsteps ? YMDIST : 0;
	}

	*newx = fromx;
	*newy = fromy;

	/*------------------------------------------------------------
	 If Left-Right movement is required - then make the move,    |
	 but don't overshoot, and do notice when we're already there |
	 ------------------------------------------------------------*/
	if (*targetX == -1)
		*s1 |= XTHERE;
	else {
		if (*targetX > fromx) {		/* To the right?	*/
			*newx += sidem;		// Move to the right...
			if (*newx == *targetX)
				*s1 |= XTHERE;
			else if (*newx > *targetX) {	// ...but don't overshoot
				if (!bOver)
					*newx = *targetX;
				else
					*targetX = *newx;
				*s1 |= XTHERE;
			}
		} else if (*targetX < fromx) {	/* To the left?		*/
			*newx -= sidem;		// Move to the left...
			if (*newx == *targetX)
				*s1 |= XTHERE;
			else if (*newx < *targetX) {	// ...but don't overshoot
				if (!bOver)
					*newx = *targetX;
				else
					*targetX = *newx;
				*s1 |= XTHERE;
			}
		} else {
			*targetX = -1;		// We're already there!
			*s1 |= XTHERE;
		}
	}

	/*--------------------------------------------------------------
	 If Up-Down movement is required - then make the move,
	 but don't overshoot, and do notice when we're already there
	 --------------------------------------------------------------*/
	if (*targetY == -1)
		*s1 |= YTHERE;
	else {
		if (*targetY > fromy) {		/* Downwards?		*/
			*newy += depthm;	// Move down...
			if (*newy == *targetY)	// ...but don't overshoot
				*s1 |= YTHERE;
			else if (*newy > *targetY) {	// ...but don't overshoot
				if (!bOver)
					*newy = *targetY;
				else
					*targetY = *newy;
				*s1 |= YTHERE;
			}
		} else if (*targetY < fromy) {	/* Upwards?		*/
			*newy -= depthm;	// Move up...
			if (*newy == *targetY)	// ...but don't overshoot
				*s1 |= YTHERE;
			else if (*newy < *targetY) {	// ...but don't overshoot
				if (!bOver)
					*newy = *targetY;
				else
					*targetY = *newy;
				*s1 |= YTHERE;
			}
		} else {
			*targetY = -1;		// We're already there!
			*s1 |= YTHERE;
		}
	}

	/* Give over if this is it */
	if (*s1 == (XTHERE | YTHERE))
		return;

	/*------------------------------------------------------
	 Have worked out where an optimum step would take us.
	 Must now check if it's in a legal spot.
	 ------------------------------------------------------*/

	if (!pActor->bNoPath && !pActor->bIgPath) {
		/*------------------------------
		 Must stay in a path polygon.
		-------------------------------*/
		hPoly = InPolygon(*newx, *newy, PATH);
		if (hPoly == NOPOLY) {
			*s2 = LEAVING_PATH;	// Trying to leave the path polygons

			if (*newx != fromx && InPolygon(*newx, fromy, PATH) != NOPOLY && InPolygon(*newx, fromy, BLOCKING) == NOPOLY) {
				*newy = fromy;
				*s1 |= YRESTRICT;
			} else if (*newy != fromy && InPolygon(fromx, *newy, PATH) != NOPOLY && InPolygon(fromx, *newy, BLOCKING) == NOPOLY) {
				*newx = fromx;
				*s1 |= XRESTRICT;
			} else {
				*newx = fromx;
				*newy = fromy;
#if 1
				*targetX = *targetY = -1;
#endif
				*s1 |= STUCK;
				return;
			}
		}

		/*--------------------------------------
		 Must stay out of blocking polygons.
		 --------------------------------------*/
		hPoly = InPolygon(*newx, *newy, BLOCKING);
		if (hPoly != NOPOLY) {
			*s2 = ENTERING_BLOCK;	// Trying to enter a blocking poly
			*hS2p = hPoly;

			if (*newx != fromx && InPolygon(*newx, fromy, BLOCKING) == NOPOLY && InPolygon(*newx, fromy, PATH) != NOPOLY) {
				*newy = fromy;
				*s1 |= YRESTRICT;
			} else if (*newy != fromy && InPolygon(fromx, *newy, BLOCKING) == NOPOLY && InPolygon(fromx, *newy, PATH) != NOPOLY) {
				*newx = fromx;
				*s1 |= XRESTRICT;
			} else {
				*newx = fromx;
				*newy = fromy;
#if 1
				*targetX = *targetY = -1;
#endif
				*s1 |= STUCK;
			}
		}
		/*------------------------------------------------------
		 Must stay out of moving actors' blocking polygons.
		 ------------------------------------------------------*/
		ma = InMActorBlock(pActor, *newx, *newy);
		if (ma != NULL) {
			// Ignore if already in it (it may have just appeared)
			if (!InMActorBlock(pActor, pActor->objx, pActor->objy)) {
				*s2 = ENTERING_MBLOCK;	// Trying to walk through an actor
				
				*hS2p = -1;
				if (collisionActor)
					*collisionActor = ma;

				if (*newx != fromx && InMActorBlock(pActor, *newx, fromy) == NULL
				    && InPolygon(*newx, fromy, BLOCKING) == NOPOLY && InPolygon(*newx, fromy, PATH) != NOPOLY) {
					*newy = fromy;
					*s1 |= YRESTRICT;
				} else if (*newy != fromy && InMActorBlock(pActor, fromx, *newy) == NULL
				           && InPolygon(fromx, *newy, BLOCKING) == NOPOLY && InPolygon(fromx, *newy, PATH) != NOPOLY) {
					*newx = fromx;
					*s1 |= XRESTRICT;
				} else {
					*newx = fromx;
					*newy = fromy;
#if 1
					*targetX = *targetY = -1;
#endif
					*s1 |= STUCK;
				}
			}
		}
	}
}

/**
 * SetOffWithinNodePath
 */
static void SetOffWithinNodePath(PMACTOR pActor, HPOLYGON StartPath, HPOLYGON DestPath,
								 int targetX, int targetY) {
	int endnode;
	HPOLYGON hIpath;
	int	nx, ny;
	int	x, y;

	if (StartPath == DestPath) {
		if (pActor->line == pActor->Tline) {
			SetMoverDest(pActor, pActor->UtargetX, pActor->UtargetY);
			pActor->over = false;
		} else if (pActor->line < pActor->Tline) {
			getNpathNode(StartPath, pActor->line+1, &nx, &ny);
			SetMoverDest(pActor, nx, ny);
			pActor->npstatus = GOING_UP;
		} else if (pActor->line > pActor->Tline) {
			getNpathNode(StartPath, pActor->line, &nx, &ny);
			SetMoverDest(pActor, nx, ny);
			pActor->npstatus = GOING_DOWN;
		}
	} else {
		/*
		 * Leaving this path - work out
		 * which end of this path to head for.
		 */
		assert(DestPath != NOPOLY); // Error 702
		if ((hIpath = getPathOnTheWay(StartPath, DestPath)) == NOPOLY) {
			// This should never happen!
			// It's the old code that didn't always work.
			endnode = NearestEndNode(StartPath, targetX, targetY);
		} else {
			if (PolySubtype(hIpath) != NODE) {
				x = PolyCentreX(hIpath);
				y = PolyCentreY(hIpath);
				endnode = NearestEndNode(StartPath, x, y);
			} else {
				endnode = NearEndNode(StartPath, hIpath);
			}
		}

#if 1
		if ((pActor->npstatus == LEAVING) &&
			endnode == NearestEndNode(StartPath, pActor->objx, pActor->objy)) {
			// Leave it be
		} else
#endif
		{
			if (endnode) {
				getNpathNode(StartPath, pActor->line+1, &nx, &ny);
				SetMoverDest(pActor, nx, ny);
				pActor->npstatus = GOING_UP;
			} else {
				getNpathNode(StartPath, pActor->line, &nx, &ny);
				SetMoverDest(pActor, nx, ny);
				pActor->npstatus = GOING_DOWN;
			}
		}
	}
}

/**
 * Restore a movement, called from restoreMovement() in ACTORS.CPP
 */
void SSetActorDest(PMACTOR pActor) {
	if (pActor->UtargetX != -1 && pActor->UtargetY != -1) {
		stand(pActor->actorID, pActor->objx, pActor->objy, 0);

		if (pActor->UtargetX != -1 && pActor->UtargetY != -1) {
			SetActorDest(pActor, pActor->UtargetX, pActor->UtargetY,
					pActor->bIgPath, 0);
		}
	} else {
		stand(pActor->actorID, pActor->objx, pActor->objy, 0);
	}
}

/**
 * Initiate a movement, called from WalkTo_Event()
 */
void SetActorDest(PMACTOR pActor, int clickX, int clickY, bool igPath, SCNHANDLE film) {
	HPOLYGON StartPath, DestPath = 0;
	int targetX, targetY;

	if (pActor->actorID == LeadId())		// Now only for lead actor
		UnTagActor(pActor->actorID);	// Tag not allowed while moving
	pActor->ticket++;
	pActor->stop = false;
	pActor->over = false;
	pActor->fromx = pActor->objx;
	pActor->fromy = pActor->objy;
	pActor->bMoving = true;
	pActor->bIgPath = igPath;

	// Use the supplied reel or restore the normal actor.
	if (film != 0)
		AlterMActor(pActor, film, AR_WALKREEL);
	else
		AlterMActor(pActor, 0, AR_NORMAL);

	if (igPath) {
		targetX = clickX;
		targetY = clickY;
	} else {
		if (WorkOutDestination(clickX, clickY, &targetX, &targetY) == ALL_SORTED) {
			GotThere(pActor);
			return;
		}
		assert(InPolygon(targetX, targetY, PATH) != NOPOLY); // illegal destination!
		assert(InPolygon(targetX, targetY, BLOCKING) == NOPOLY); // illegal destination!
	}


	/***** Now have a destination to aim for. *****/

	/*----------------------------------
	| Don't move if it's not worth it.
	----------------------------------*/
	if (ABS(targetX - pActor->objx) < XMDIST && ABS(targetY - pActor->objy) < YMDIST) {
		GotThere(pActor);
		return;
	}

	/*------------------------------------------------------
	| If the destiation is within a follow nodes polygon,
	| set destination as the nearest node.
	------------------------------------------------------*/
	if (!igPath) {
		DestPath = InPolygon(targetX, targetY, PATH);
		if (PolySubtype(DestPath) == NODE) {
			// Find the nearest point on a line, or nearest node
			FindBestPoint(DestPath, &targetX, &targetY, &pActor->Tline);
		}
	}

 	assert(pActor->bIgPath || InPolygon(targetX, targetY, PATH) != NOPOLY); // Error 5005
	SetMoverUltDest(pActor, targetX, targetY);
	SetMoverIntDest(pActor, targetX, targetY);

	/*-------------------------------------------------------------------
	| If in a follow nodes path, need to set off in the right direction! |
	-------------------------------------------------------------------*/
	if ((StartPath = pActor->hFnpath) != NOPOLY && !igPath) {
		SetOffWithinNodePath(pActor, StartPath, DestPath, targetX, targetY);
	} else {
		// Set off!
		SetNextDest(pActor);
	}
}

/**
 * Change scale if appropriate.
 */
static void CheckScale(PMACTOR pActor, HPOLYGON hPath, int ypos) {
	int scale;

	scale = GetScale(hPath, ypos);
	if (scale != pActor->scale) {
		SetMActorWalkReel(pActor, pActor->dirn, scale, false);
	}
}

/**
 * Not going anywhere - Kick off again if not at final destination.
 */
static void NotMoving(PMACTOR pActor, int x, int y) {
	pActor->targetX = pActor->targetY = -1;

//	if (x == pActor->UtargetX && y == pActor->UtargetY)
	if (ABS(x - pActor->UtargetX) < XMDIST && ABS(y - pActor->UtargetY) < YMDIST) {
		GotThere(pActor);
		return;
	}

	if (pActor->ItargetX != -1 || pActor->ItargetY != -1) {
		SetNextDest(pActor);
	} else if (pActor->UtargetX != -1 || pActor->UtargetY != -1) {
 		assert(pActor->bIgPath || InPolygon(pActor->UtargetX, pActor->UtargetY, PATH) != NOPOLY); // Error 5006
		SetMoverIntDest(pActor, pActor->UtargetX, pActor->UtargetY);
		SetNextDest(pActor);
	}
}

/**
 * Does the necessary business when entering a different path polygon.
 */
static void EnteringNewPath(PMACTOR pActor, HPOLYGON hPath, int x, int y) {
	int	firstnode;	// First node to go to
	int	lastnode;	// Last node to go to
	HPOLYGON hIpath;
	int	nx, ny;
	int	nxl, nyl;

	pActor->hCpath = hPath;		// current path

	if (hPath == NOPOLY) {
		// Not proved this ever happens, but just in case
		pActor->hFnpath = NOPOLY;
		pActor->npstatus = NOT_IN;
		return;
	}

	// Is new path a node path?
	if (PolySubtype(hPath) == NODE) {
		// Node path - usually go to nearest end node
		firstnode = NearestEndNode(hPath, x, y);
		lastnode = -1;

		// If this is not the destination path,
		// find which end nodfe we wish to leave via
		if (hPath != pActor->hUpath) {
			if (pActor->bIgPath) {
				lastnode = NearestEndNode(hPath, pActor->UtargetX, pActor->UtargetY);
			} else {
				assert(pActor->hUpath != NOPOLY); // Error 703
				hIpath = getPathOnTheWay(hPath, pActor->hUpath);
				assert(hIpath != NOPOLY); // No path on the way

				if (PolySubtype(hIpath) != NODE) {
					lastnode = NearestEndNode(hPath, PolyCentreX(hIpath), PolyCentreY(hIpath));
				} else {
					lastnode = NearEndNode(hPath, hIpath);
				}
			}
		}
		// Test for pseudo-one-node npaths
		if (lastnode != -1 && numNodes(hPath) == 2) {
			getNpathNode(hPath, firstnode, &nx, &ny);
			getNpathNode(hPath, lastnode, &nxl, &nyl);
			if (nxl == nx && nyl == ny)
				firstnode = lastnode;
		}

		// If leaving by same node as entering, don't bother.
		if (lastnode == firstnode) {
			pActor->hFnpath = NOPOLY;
			pActor->npstatus = NOT_IN;
	 		assert(pActor->bIgPath || InPolygon(pActor->UtargetX, pActor->UtargetY, PATH) != NOPOLY); // Error 5007
			SetMoverIntDest(pActor, pActor->UtargetX, pActor->UtargetY);
			SetNextDest(pActor);
		} else {
			// Head for first node
			pActor->over = true;
			pActor->npstatus = ENTERING;
			pActor->hFnpath = hPath;
			pActor->line = firstnode ? firstnode - 1 : firstnode;
			if (pActor->line == pActor->Tline && hPath == pActor->hUpath) {
		 		assert(pActor->bIgPath || InPolygon(pActor->UtargetX, pActor->UtargetY, PATH) != NOPOLY); // Error 5008
				SetMoverIntDest(pActor, pActor->UtargetX, pActor->UtargetY);
				SetMoverDest(pActor, pActor->UtargetX, pActor->UtargetY);
			} else {
				// This doesn't seem right
				getNpathNode(hPath, firstnode, &nx, &ny);
				if (ABS(pActor->objx - nx) < XMDIST
						&& ABS(pActor->objy - ny) < YMDIST) {
					pActor->npstatus = ENTERING;
					pActor->hFnpath = hPath;
					SetNextDest(pActor);
				} else {
					getNpathNode(hPath, firstnode, &nx, &ny);
					SetMoverDest(pActor, nx, ny);
				}
			}
		}
		return;
	} else {
		pActor->hFnpath = NOPOLY;
		pActor->npstatus = NOT_IN;
 		assert(pActor->bIgPath || InPolygon(pActor->UtargetX, pActor->UtargetY, PATH) != NOPOLY); // Error 5009
// Added 26/01/95
		if (IsPolyCorner(hPath, pActor->ItargetX, pActor->ItargetY))
			return;

		SetMoverIntDest(pActor, pActor->UtargetX, pActor->UtargetY);
		SetNextDest(pActor);
	}
}

/**
 * Move
 */
void Move(PMACTOR pActor, int newx, int newy, HPOLYGON hPath) {
	MultiSetAniXY(pActor->actorObj, newx, newy);
	MAsetZPos(pActor, newy, getPolyZfactor(hPath));
	if (StepAnimScript(&pActor->actorAnim) == ScriptFinished) {
		// The end of a scale-change reel
		// Revert to normal walking reel
		pActor->walkReel = false;
		pActor->scount = 0;
		SetMActorWalkReel(pActor, pActor->dirn, pActor->scale, true);
	}
	pActor->objx = newx;
	pActor->objy = newy;

	// Synchronised walking reels
	if (++pActor->scount >= 6)
		pActor->scount = 0;
}

/**
 * Called from MActorProcess() on every tick.
 *
 * Moves the actor as appropriate.
 */
void MoveActor(PMACTOR pActor) {
	int newx, newy;
	HPOLYGON hPath;
	int status, s2;		// s2 not used here!
	HPOLYGON hS2p;		// nor is s2p!
	HPOLYGON hEb;
	PMACTOR ma;
	int	sTargetX, sTargetY;
	bool bNewPath = false;

	// Only do anything if the actor needs to move!
	if (pActor->targetX == -1 && pActor->targetY == -1)
		return;

	if (pActor->stop) {
		GotThere(pActor);
		pActor->stop = false;
		SetMActorStanding(pActor);
		return;
	}

#if SLOW_RINCE_DOWN
/**/	if (BogusVar++ < Interlude)	// Temporary slow-down-the-action code
/**/		return;			//
/**/	BogusVar = 0;			//
#endif

	// During swalk()s, movement while hidden may be slowed down.
	if (pActor->aHidden) {
		if (++hSlowVar < pActor->SlowFactor)
			return;
		hSlowVar = 0;
	}

	// 'push' the target
	sTargetX = pActor->targetX;
	sTargetY = pActor->targetY;

	NewCoOrdinates(pActor->objx, pActor->objy, &pActor->targetX, &pActor->targetY,
			&newx, &newy, &status, &s2, &hS2p, pActor->over, false, pActor);

	if (newx == pActor->objx && newy == pActor->objy) {
		// 'pop' the target
		pActor->targetX = sTargetX;
		pActor->targetY = sTargetY;

		NewCoOrdinates(pActor->objx, pActor->objy, &pActor->targetX, &pActor->targetY, &newx, &newy,
				&status, &s2, &hS2p, pActor->over, true, pActor);
		if (newx == pActor->objx && newy == pActor->objy) {
			NotMoving(pActor, newx, newy);
			return;
		}
	}

	// Find out which path we're in now
	hPath = InPolygon(newx, newy, PATH);
	if (hPath == NOPOLY) {
		if (pActor->bNoPath) {
			Move(pActor, newx, newy, pActor->hCpath);
			return;
		} else {
			// May be marginally outside!
			// OR bIgPath may be set.
			hPath = pActor->hCpath;
		}
	} else if (pActor->bNoPath) {
		pActor->bNoPath = false;
		bNewPath = true;
	} else if (hPath != pActor->hCpath) {
		if (IsInPolygon(newx, newy, pActor->hCpath))
			hPath = pActor->hCpath;
	}

	CheckScale(pActor, hPath, newy);

	/*
	* Must stay out of moving actors' blocking polygons.
	*/
	ma = InMActorBlock(pActor, newx, newy);
	if (ma != NULL) {
		// Stop if there's no chance of arriving
		if (InMActorBlock(pActor, pActor->UtargetX, pActor->UtargetY)) {
			GotThere(pActor);
			return;
		}

		if (InMActorBlock(pActor, pActor->objx, pActor->objy))
			;
		else {
			hEb = InitExtraBlock(pActor, ma);
			newx = pActor->objx;
			newy = pActor->objy;
			BlockingCorner(hEb, &newx, &newy, pActor->ItargetX, pActor->ItargetY);
			SetMoverDest(pActor, newx, newy);
			return;
		}
	}

	/*--------------------------------------
	 This is where it actually gets moved.
	 --------------------------------------*/
	Move(pActor, newx, newy, hPath);

	// Entering a new path polygon?
	if (hPath != pActor->hCpath || bNewPath)
		EnteringNewPath(pActor, hPath, newx, newy);
}

/**
 * Store the default refer type for the current scene.
 */
void SetDefaultRefer(int32 defRefer) {
	DefaultRefer = defRefer;
}

/**
 * DoMoveActor
 */
void DoMoveActor(PMACTOR pActor) {
	int wasx, wasy;
	int i;

#define NUMBER 1

	wasx = pActor->objx;
	wasy = pActor->objy;

	MoveActor(pActor);

	if ((pActor->targetX != -1 || pActor->targetY != -1)
	&&  (wasx == pActor->objx && wasy == pActor->objy)) 	{
		for (i=0; i < NUMBER; i++) {
			MoveActor(pActor);
			if (wasx != pActor->objx || wasy != pActor->objy)
				break;
		}
//		assert(i<NUMBER);
	}
}

} // end of namespace Tinsel
