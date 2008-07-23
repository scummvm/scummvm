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
 * CursorPositionProcess()
 * TagProcess()
 * PointProcess()
 */

#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/events.h"
#include "tinsel/font.h"
#include "tinsel/graphics.h"
#include "tinsel/multiobj.h"
#include "tinsel/object.h"
#include "tinsel/pcode.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/sched.h"
#include "tinsel/strres.h"
#include "tinsel/text.h"

namespace Tinsel {

//----------------- EXTERNAL GLOBAL DATA --------------------

#ifdef DEBUG
//extern int Overrun;		// The overrun counter, in DOS_DW.C

extern int newestString;	// The overrun counter, in STRRES.C
#endif


//----------------- EXTERNAL FUNCTIONS ---------------------

// in BG.C
extern int BackgroundWidth(void);
extern int BackgroundHeight(void);



//----------------- LOCAL DEFINES --------------------

#define LPOSX	295		// X-co-ord of lead actor's position display
#define CPOSX	24		// X-co-ord of cursor's position display
#define OPOSX	SCRN_CENTRE_X	// X-co-ord of overrun counter's display
#define SPOSX	SCRN_CENTRE_X	// X-co-ord of string numbner's display

#define POSY	0		// Y-co-ord of these position displays

#define ACTOR_TAG 0xffffffff


//----------------- LOCAL GLOBAL DATA --------------------

static bool DispPath = false;
static bool bShowString = false;

static int	TaggedActor = 0;
static HPOLYGON	hTaggedPolygon = NOPOLY;

static enum { TAGS_OFF, TAGS_ON } TagsActive = TAGS_ON;


#ifdef DEBUG
/**
 * Displays the cursor and lead actor's co-ordinates and the overrun
 * counter. Also which path polygon the cursor is in, if required.
 *
 * This process is only started up if a Glitter showpos() call is made.
 * Obviously, this is for testing purposes only...
 */
void CursorPositionProcess(CORO_PARAM) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		int prevsX, prevsY;	// Last screen top left
		int prevcX, prevcY;	// Last displayed cursor position
		int prevlX, prevlY;	// Last displayed lead actor position
//		int prevOver;		// Last displayed overrun
		int prevString;		// Last displayed string number

		OBJECT *cpText;		// cursor position text object pointer
		OBJECT *cpathText;	// cursor path text object pointer
		OBJECT *rpText;		// text object pointer
//		OBJECT *opText;		// text object pointer
		OBJECT *spText;		// string number text object pointer
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->prevsX = -1;
	_ctx->prevsY = -1;
	_ctx->prevcX = -1;
	_ctx->prevcY = -1;
	_ctx->prevlX = -1;
	_ctx->prevlY = -1;
//	_ctx->prevOver = -1;
	_ctx->prevString = -1;

	_ctx->cpText = NULL;
	_ctx->cpathText = NULL;
	_ctx->rpText = NULL;
//	_ctx->opText = NULL;
	_ctx->spText = NULL;


	int aniX, aniY;			// cursor/lead actor position
	int Loffset, Toffset;		// Screen top left

	char PositionString[64];	// sprintf() things into here

	PMACTOR pActor;		// Lead actor

	while (1) {
		PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

		/*-----------------------------------*\
		| Cursor's position and path display. |
		\*-----------------------------------*/
		GetCursorXY(&aniX, &aniY, false);

		// Change in cursor position?
		if (aniX != _ctx->prevcX || aniY != _ctx->prevcY ||
				Loffset != _ctx->prevsX || Toffset != _ctx->prevsY) {
			// kill current text objects
			if (_ctx->cpText) {
				MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->cpText);
			}
			if (_ctx->cpathText) {
				MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->cpathText);
				_ctx->cpathText = NULL;
			}

			// New text objects
			sprintf(PositionString, "%d %d", aniX + Loffset, aniY + Toffset);
			_ctx->cpText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), PositionString,
						0, CPOSX, POSY, hTagFontHandle(), TXT_CENTRE);
			if (DispPath) {
				HPOLYGON hp = InPolygon(aniX + Loffset, aniY + Toffset, PATH);
				if (hp == NOPOLY)
					sprintf(PositionString, "No path");
				else
					sprintf(PositionString, "%d,%d %d,%d %d,%d %d,%d",
						PolyCornerX(hp, 0), PolyCornerY(hp, 0),
						PolyCornerX(hp, 1), PolyCornerY(hp, 1),
						PolyCornerX(hp, 2), PolyCornerY(hp, 2),
						PolyCornerX(hp, 3), PolyCornerY(hp, 3));
				_ctx->cpathText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), PositionString,
							0, 4, POSY+ 10, hTagFontHandle(), 0);
			}

			// update previous position
			_ctx->prevcX = aniX;
			_ctx->prevcY = aniY;
		}

#if 0
		/*------------------------*\
		| Overrun counter display. |
		\*------------------------*/
		if (Overrun != _ctx->prevOver) {
			// kill current text objects
			if (_ctx->opText) {
				MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->opText);
			}

			sprintf(PositionString, "%d", Overrun);
			_ctx->opText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), PositionString,
						0, OPOSX, POSY, hTagFontHandle(), TXT_CENTRE);

			// update previous value
			_ctx->prevOver = Overrun;
		}
#endif

		/*----------------------*\
		| Lead actor's position. |
		\*----------------------*/
		pActor = GetMover(LEAD_ACTOR);
		if (pActor && pActor->MActorState == NORM_MACTOR) {
			// get lead's animation position
			GetActorPos(LEAD_ACTOR, &aniX, &aniY);

			// Change in position?
			if (aniX != _ctx->prevlX || aniY != _ctx->prevlY ||
					Loffset != _ctx->prevsX || Toffset != _ctx->prevsY) {
				// Kill current text objects
				if (_ctx->rpText) {
					MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->rpText);
				}

				// create new text object list
				sprintf(PositionString, "%d %d", aniX, aniY);
				_ctx->rpText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), PositionString,
								0, LPOSX, POSY,	hTagFontHandle(), TXT_CENTRE);

				// update previous position
				_ctx->prevlX = aniX;
				_ctx->prevlY = aniY;
			}
		}

		/*-------------*\
		| String number	|
		\*-------------*/
		if (bShowString && newestString != _ctx->prevString) {
			// kill current text objects
			if (_ctx->spText) {
				MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->spText);
			}

			sprintf(PositionString, "String: %d", newestString);
			_ctx->spText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), PositionString,
						0, SPOSX, POSY+10, hTalkFontHandle(), TXT_CENTRE);

			// update previous value
			_ctx->prevString = newestString;
		}

		// update previous playfield position
		_ctx->prevsX = Loffset;
		_ctx->prevsY = Toffset;

		CORO_SLEEP(1);		// allow re-scheduling
	}
	CORO_END_CODE;
}
#endif

/**
 * Tag process keeps us updated as to which tagged actor is currently tagged
 * (if one is). Tag process asks us for this information, as does User_Event().
 */
static void SaveTaggedActor(int ano) {
	TaggedActor = ano;
}

/**
 * Tag process keeps us updated as to which tagged actor is currently tagged
 * (if one is). Tag process asks us for this information, as does User_Event().
 */
int GetTaggedActor(void) {
	return TaggedActor;
}

/**
 * Tag process keeps us updated as to which polygon is currently tagged 
 * (if one is). Tag process asks us for this information, as does User_Event().
 */
static void SaveTaggedPoly(HPOLYGON hp) {
	hTaggedPolygon = hp;
}

HPOLYGON GetTaggedPoly(void) {
	return hTaggedPolygon;
}

/**
 * Given cursor position and an actor number, ascertains whether the
 * cursor is within the actor's tag area.
 * Returns TRUE for a positive result, FALSE for negative.
 * If TRUE, the mid-top co-ordinates of the actor's tag area are also
 * returned.
 */
static bool InHotSpot(int ano, int aniX, int aniY, int *pxtext, int *pytext) {
	int	Top, Bot;		// Top and bottom limits of active area
	int	left, right;	// left and right of active area
	int	qrt = 0;		// 1/4 of height (sometimes 1/2)

	// First check if within x-range
	if (aniX > (left = GetActorLeft(ano)) && aniX < (right = GetActorRight(ano))) {
		Top = GetActorTop(ano);
		Bot = GetActorBottom(ano);

		// y-range varies according to tag-type
		switch (TagType(ano)) {
		case TAG_DEF:
			// Next to bottom 1/4 of the actor's area
			qrt = (Bot - Top) >> 1;		// Half actor's height
			Top += qrt;			// Top = mid-height

			qrt = qrt >> 1;			// Quarter height
			Bot -= qrt;			// Bot = 1/4 way up
			break;

		case TAG_Q1TO3:
			// Top 3/4 of the actor's area
			qrt = (Bot - Top) >> 2;		// 1/4 actor's height
			Bot -= qrt;			// Bot = 1/4 way up
			break;

		case TAG_Q1TO4:
			// All the actor's area
			break;

		default:
			error("illegal tag area type");
		}

		// Now check if within y-range
		if (aniY >= Top && aniY <= Bot) {
			if (TagType(ano) == TAG_Q1TO3)
				*pytext = Top + qrt;
			else
				*pytext = Top;
			*pxtext = (left + right) / 2;
			return true;
		}
	}
	return false;
}

/**
 * See if the cursor is over a tagged actor's hot-spot. If so, display
 * the tag or, if tag already displayed, maintain the tag's position on
 * the screen.
 */
static bool ActorTag(int curX, int curY, SCNHANDLE *pTag, OBJECT **ppText) {
	static int Loffset = 0, Toffset = 0;	// Values when tag was displayed
	int	nLoff, nToff;		// new values, to keep tag in place
	int	ano;
	int	xtext, ytext;
	bool	newActor;

	// For each actor with a tag....
	FirstTaggedActor();
	while ((ano = NextTaggedActor()) != 0) {
		if (InHotSpot(ano, curX, curY, &xtext, &ytext)) {
			// Put up or maintain actor tag
			if (*pTag != ACTOR_TAG)
				newActor = true;
			else if (ano != GetTaggedActor())
				newActor = true;	// Different actor
			else
				newActor = false;	// Same actor

			if (newActor) {
				// Display actor's tag

				if (*ppText)
					MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), *ppText);

				*pTag = ACTOR_TAG;
				SaveTaggedActor(ano);	// This actor tagged
				SaveTaggedPoly(NOPOLY);	// No tagged polygon

				PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
				LoadStringRes(GetActorTag(ano), tBufferAddr(), TBUFSZ);
				*ppText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(),
							0, xtext - Loffset, ytext - Toffset, hTagFontHandle(), TXT_CENTRE);
				assert(*ppText); // Actor tag string produced NULL text
				MultiSetZPosition(*ppText, Z_TAG_TEXT);
			} else {
				// Maintain actor tag's position

				PlayfieldGetPos(FIELD_WORLD, &nLoff, &nToff);
				if (nLoff != Loffset || nToff != Toffset) {
					MultiMoveRelXY(*ppText, Loffset - nLoff, Toffset - nToff);
					Loffset = nLoff;
					Toffset = nToff;
				}
			}
			return true;
		}
	}

	// No tagged actor
	if (*pTag == ACTOR_TAG) {
		*pTag = 0;
		SaveTaggedActor(0);
	}
	return false;
}

/**
 * Perhaps some comment in due course.
 *
 * Under control of PointProcess(), when the cursor is over a TAG or
 * EXIT polygon, its pointState flag is set to POINTING. If its Glitter
 * code contains a printtag() call, its tagState flag gets set to TAG_ON.
 */
static bool PolyTag(SCNHANDLE *pTag, OBJECT **ppText) {
	static int	Loffset = 0, Toffset = 0;	// Values when tag was displayed
	int		nLoff, nToff;		// new values, to keep tag in place
	HPOLYGON	hp;
	bool	newPoly;
	int	shift;

	int	tagx, tagy;	// Tag display co-ordinates
	SCNHANDLE hTagtext;	// Tag text

	// For each polgon with a tag....
	for (int i = 0; i < MAX_POLY; i++) {
		hp = GetPolyHandle(i);

		// Added code for un-tagged tags
		if (hp != NOPOLY && PolyPointState(hp) == POINTING && PolyTagState(hp) != TAG_ON) {
			// This poly is entitled to be tagged
			if (hp != GetTaggedPoly()) {
				if (*ppText) {
					MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), *ppText);
					*ppText = NULL;
				}
				*pTag = POLY_TAG;
				SaveTaggedActor(0);	// No tagged actor
				SaveTaggedPoly(hp);	// This polygon tagged
			}
			return true;
		} else if (hp != NOPOLY && PolyTagState(hp) == TAG_ON) {
			// Put up or maintain polygon tag
			if (*pTag != POLY_TAG)
				newPoly = true;		// A new polygon (no current)
			else if (hp != GetTaggedPoly())
				newPoly = true;		// Different polygon
			else
				newPoly = false;	// Same polygon

			if (newPoly) {
				if (*ppText)
					MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), *ppText);

				*pTag = POLY_TAG;
				SaveTaggedActor(0);	// No tagged actor
				SaveTaggedPoly(hp);	// This polygon tagged

				PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
				getPolyTagInfo(hp, &hTagtext, &tagx, &tagy);

				int strLen;
				if (PolyTagHandle(hp) != 0)
					strLen = LoadStringRes(PolyTagHandle(hp), tBufferAddr(), TBUFSZ);
				else 
					strLen = LoadStringRes(hTagtext, tBufferAddr(), TBUFSZ);

				if (strLen == 0)
					// No valid string returned, so leave ppText as NULL
					ppText = NULL;
				else {
					// Handle displaying the tag text on-screen
					*ppText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(),
							0, tagx - Loffset, tagy - Toffset,
							hTagFontHandle(), TXT_CENTRE);
					assert(*ppText); // Polygon tag string produced NULL text
					MultiSetZPosition(*ppText, Z_TAG_TEXT);
				

					/*
					* New feature: Don't go off the side of the background
					*/
					shift = MultiRightmost(*ppText) + Loffset + 2;
					if (shift >= BackgroundWidth())			// Not off right
						MultiMoveRelXY(*ppText, BackgroundWidth() - shift, 0);
					shift = MultiLeftmost(*ppText) + Loffset - 1;
					if (shift <= 0)					// Not off left
						MultiMoveRelXY(*ppText, -shift, 0);
					shift = MultiLowest(*ppText) + Toffset;
					if (shift > BackgroundHeight())			// Not off bottom
						MultiMoveRelXY(*ppText, 0, BackgroundHeight() - shift);
				}
			} else {
				PlayfieldGetPos(FIELD_WORLD, &nLoff, &nToff);
				if (nLoff != Loffset || nToff != Toffset) {
					MultiMoveRelXY(*ppText, Loffset - nLoff, Toffset - nToff);
					Loffset = nLoff;
					Toffset = nToff;
				}
			}
			return true;
		}
	}

	// No tagged polygon
	if (*pTag == POLY_TAG) {
		*pTag = 0;
		SaveTaggedPoly(NOPOLY);
	}
	return false;
}

/**
 * Handle display of tagged actor and polygon tags.
 * Tagged actor's get priority over polygons.
 */
void TagProcess(CORO_PARAM) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		OBJECT	*pText;	// text object pointer
		SCNHANDLE Tag;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	
	_ctx->pText = NULL;
	_ctx->Tag = 0;

	SaveTaggedActor(0);		// No tagged actor yet
	SaveTaggedPoly(NOPOLY);		// No tagged polygon yet

	while (1) {
		if (TagsActive == TAGS_ON) {
			int	curX, curY;	// cursor position
			while (!GetCursorXYNoWait(&curX, &curY, true))
				CORO_SLEEP(1);

			if (!ActorTag(curX, curY, &_ctx->Tag, &_ctx->pText)
					&& !PolyTag(&_ctx->Tag, &_ctx->pText)) {
				// Nothing tagged. Remove tag, if there is one
				if (_ctx->pText) {
					MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pText);
					_ctx->pText = NULL;
				}
			}
		} else {
			SaveTaggedActor(0);
			SaveTaggedPoly(NOPOLY);

			// Remove tag, if there is one
			if (_ctx->pText) {
				// kill current text objects
				MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pText);
				_ctx->pText = NULL;
				_ctx->Tag = 0;
			}
		}

		CORO_SLEEP(1);		// allow re-scheduling
	}

	CORO_END_CODE;
}

/**
 * Called from PointProcess() as appropriate.
 */
static void enteringpoly(HPOLYGON hp) {
	SetPolyPointState(hp, POINTING);

	RunPolyTinselCode(hp, POINTED, BE_NONE, false);
}

/**
 * Called from PointProcess() as appropriate.
 */
static void leavingpoly(HPOLYGON hp) {
	SetPolyPointState(hp, NOT_POINTING);

	if (PolyTagState(hp) == TAG_ON) {
		// Delete this tag entry 
		SetPolyTagState(hp, TAG_OFF);
	}
}

/**
 * For TAG and EXIT polygons, monitor cursor entering and leaving.
 * Maintain the polygons' pointState and tagState flags accordingly.
 * Also run the polygon's Glitter code when the cursor enters.
 */
void PointProcess(CORO_PARAM) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	while (1) {
		int	aniX, aniY;	// cursor/tagged actor position
		while (!GetCursorXYNoWait(&aniX, &aniY, true))
			CORO_SLEEP(1);

		/*----------------------------------*\
		| For polygons of type TAG and EXIT. |
		\*----------------------------------*/
		for (int i = 0; i < MAX_POLY; i++) {
			HPOLYGON hp = GetPolyHandle(i);

			if (hp != NOPOLY && (PolyType(hp) == TAG || PolyType(hp) == EXIT)) {
				if (PolyPointState(hp) == NOT_POINTING) {
					if (IsInPolygon(aniX, aniY, hp)) {
						enteringpoly(hp);
					}
				} else if (PolyPointState(hp) == POINTING) {
					if (!IsInPolygon(aniX, aniY, hp)) {
						leavingpoly(hp);
					}
				}
			}
		}

		// allow re-scheduling
		CORO_SLEEP(1);
	}

	CORO_END_CODE;
}

void DisableTags(void) {
	TagsActive = TAGS_OFF;
}

void EnableTags(void) {
	TagsActive = TAGS_ON;
}

bool DisableTagsIfEnabled(void) {
	if (TagsActive == TAGS_OFF)
		return false;
	else {
		TagsActive = TAGS_OFF;
		return true;
	}
}

/**
 * For testing purposes only.
 * Causes CursorPositionProcess() to display, or not, the path that the
 * cursor is in.
 */
void TogglePathDisplay(void) {
	DispPath ^= 1;	// Toggle path display (XOR with true)
}


void setshowstring(void) {
	bShowString = true;
}

} // end of namespace Tinsel
