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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Background handling code.
 */

#include "tinsel/background.h"
#include "tinsel/cliprect.h"	// object clip rect defs
#include "tinsel/font.h"
#include "tinsel/graphics.h"
#include "tinsel/multiobj.h"
#include "tinsel/sched.h"	// process sheduler defs
#include "tinsel/object.h"
#include "tinsel/pid.h"	// process identifiers
#include "tinsel/tinsel.h"

namespace Tinsel {

Background::Background(Font* font) : _font(font), _pCurBgnd(nullptr), _hBgPal(0), _BGspeed(0), _hBackground(0), _bDoFadeIn(false), _bgReels(0) {
	for (int i = 0; i < MAX_BG; i++) {
		_pBG[i] = nullptr;
		_thisAnim[i].pObject = nullptr;
	}
}

/**
 * Called to initialize a background.
 */
void Background::InitBackground() {
	PLAYFIELD worldPlayfield = {
	    NULL,                                            // display list
	    0,                                               // init field x
	    0,                                               // init field y
	    0,                                               // x vel
	    0,                                               // y vel
	    Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), // clip rect
	    false                                            // moved flag
	};
	PLAYFIELD statusPlayfield = {
	    NULL,                                            // display list
	    0,                                               // init field x
	    0,                                               // init field y
	    0,                                               // x vel
	    0,                                               // y vel
	    Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), // clip rect
	    false                                            // moved flag
	};

	// set current background
	_pCurBgnd = new BACKGND();
	_pCurBgnd->rgbSkyColor = BLACK;
	_pCurBgnd->ptInitWorld = Common::Point(0, 0);
	_pCurBgnd->rcScrollLimits = Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_pCurBgnd->refreshRate = 0;	// no background update process
	_pCurBgnd->pXscrollTable = nullptr;
	_pCurBgnd->pYscrollTable = nullptr;
	_pCurBgnd->fieldArray.push_back(worldPlayfield);
	_pCurBgnd->fieldArray.push_back(statusPlayfield);
	_pCurBgnd->bAutoErase = false;

	// init background sky color
	SetBgndColor(_pCurBgnd->rgbSkyColor);

	// for each background playfield
	for (int i = 0; i < _pCurBgnd->fieldArray.size(); i++) {
		PLAYFIELD cur = _pCurBgnd->fieldArray[i];
		// init playfield pos
		cur.fieldX = intToFrac(_pCurBgnd->ptInitWorld.x);
		cur.fieldY = intToFrac(_pCurBgnd->ptInitWorld.y);

		// no scrolling
		cur.fieldXvel = intToFrac(0);
		cur.fieldYvel = intToFrac(0);

		// clear playfield display list
		cur.pDispList = nullptr;

		// clear playfield moved flag
		cur.bMoved = false;
	}
}

/**
 * Sets the xy position of the specified playfield in the current background.
 * @param which			Which playfield
 * @param newXpos		New x position
 * @param newYpos		New y position
 */

void Background::PlayfieldSetPos(int which, int newXpos, int newYpos) {
	PLAYFIELD *pPlayfield;	// pointer to relavent playfield

	// make sure there is a background
	assert(_pCurBgnd != NULL);

	// make sure the playfield number is in range
	assert(which >= 0 && which < _pCurBgnd->fieldArray.size());

	// get playfield pointer
	pPlayfield = &_pCurBgnd->fieldArray[which];

	// set new integer position
	pPlayfield->fieldX = intToFrac(newXpos);
	pPlayfield->fieldY = intToFrac(newYpos);

	// set moved flag
	pPlayfield->bMoved = true;
}

/**
 * Returns the xy position of the specified playfield in the current background.
 * @param which			Which playfield
 * @param pXpos			Returns current x position
 * @param pYpos			Returns current y position
 */

void Background::PlayfieldGetPos(int which, int *pXpos, int *pYpos) {
	PLAYFIELD *pPlayfield;	// pointer to relavent playfield

	// make sure there is a background
	assert(_pCurBgnd != NULL);

	// make sure the playfield number is in range
	assert(which >= 0 && which < _pCurBgnd->fieldArray.size());

	// get playfield pointer
	pPlayfield = &_pCurBgnd->fieldArray[which];

	// get current integer position
	*pXpos = fracToInt(pPlayfield->fieldX);
	*pYpos = fracToInt(pPlayfield->fieldY);
}

/**
 * Returns the x position of the center of the specified playfield
 * @param which			Which playfield
 */

int Background::PlayfieldGetCenterX(int which) {
	PLAYFIELD *pPlayfield; // pointer to relavent playfield

	// make sure there is a background
	assert(_pCurBgnd != NULL);

	// make sure the playfield number is in range
	assert(which >= 0 && which < _pCurBgnd->fieldArray.size());

	// get playfield pointer
	pPlayfield = &_pCurBgnd->fieldArray[which];

	// get current integer position
	return fracToInt(pPlayfield->fieldX) + SCREEN_WIDTH/2;
}

/**
 * Returns the display list for the specified playfield.
 * @param which			Which playfield
 */

OBJECT **Background::GetPlayfieldList(int which) {
	PLAYFIELD *pPlayfield;	// pointer to relavent playfield

	// make sure there is a background
	assert(_pCurBgnd != NULL);

	// make sure the playfield number is in range
	assert(which >= 0 && which < _pCurBgnd->fieldArray.size());

	// get playfield pointer
	pPlayfield = &_pCurBgnd->fieldArray[which];

	// return the display list pointer for this playfield
	return &pPlayfield->pDispList;
}

/**
 * Draws all the playfield object lists for the current background.
 * The playfield velocity is added to the playfield position in order
 * to scroll each playfield before it is drawn.
 */

void Background::DrawBackgnd() {
	int i;			// playfield counter
	PLAYFIELD *pPlay;	// playfield pointer
	int prevX, prevY;	// save interger part of position
	Common::Point ptWin;	// window top left

	if (_pCurBgnd == NULL)
		return;		// no current background

	// scroll each background playfield
	for (i = 0; i < _pCurBgnd->fieldArray.size(); i++) {
		// get pointer to correct playfield
		pPlay = &_pCurBgnd->fieldArray[i];

		// save integer part of position
		prevX = fracToInt(pPlay->fieldX);
		prevY = fracToInt(pPlay->fieldY);

		// update scrolling
		pPlay->fieldX += pPlay->fieldXvel;
		pPlay->fieldY += pPlay->fieldYvel;

		// convert fixed point window pos to a int
		ptWin.x = fracToInt(pPlay->fieldX);
		ptWin.y = fracToInt(pPlay->fieldY);

		// set the moved flag if the playfield has moved
		if (prevX != ptWin.x || prevY != ptWin.y)
			pPlay->bMoved = true;

		// sort the display list for this background - just in case somebody has changed object Z positions
		SortObjectList(&pPlay->pDispList);

		// generate clipping rects for all objects that have moved etc.
		FindMovingObjects(&pPlay->pDispList, &ptWin,
			&pPlay->rcClip,	false, pPlay->bMoved);

		// clear playfield moved flag
		pPlay->bMoved = false;
	}

	// merge the clipping rectangles
	MergeClipRect();

	// redraw all playfields within the clipping rectangles
	const RectList &clipRects = GetClipRects();
	for (RectList::const_iterator r = clipRects.begin(); r != clipRects.end(); ++r) {
		// clear the clip rectangle on the virtual screen
		// for each background playfield
		for (i = 0; i < _pCurBgnd->fieldArray.size(); i++) {
			Common::Rect rcPlayClip;	// clip rect for this playfield

			// get pointer to correct playfield
			pPlay = &_pCurBgnd->fieldArray[i];

			// convert fixed point window pos to a int
			ptWin.x = fracToInt(pPlay->fieldX);
			ptWin.y = fracToInt(pPlay->fieldY);

			if (IntersectRectangle(rcPlayClip, pPlay->rcClip, *r))
				// redraw all objects within this clipping rect
				UpdateClipRect(&pPlay->pDispList, &ptWin,	&rcPlayClip);
		}
	}

	// transfer any new palettes to the video DAC
	PalettesToVideoDAC();

	// update the screen within the clipping rectangles
	for (RectList::const_iterator r = clipRects.begin(); r != clipRects.end(); ++r) {
		UpdateScreenRect(*r);
	}

	g_system->updateScreen();

	// delete all the clipping rectangles
	ResetClipRect();
}

int Background::BgWidth() {
	assert(_pBG[0]);
	return MultiRightmost(_pBG[0]) + 1;
}

int Background::BgHeight() {
	assert(_pBG[0]);
	return MultiLowest(_pBG[0]) + 1;
}

void Background::SetBackPal(SCNHANDLE hPal) {
	_hBgPal = hPal;

	_font->FettleFontPal(_hBgPal);
	CreateTranslucentPalette(_hBgPal);
}

void Background::DropBackground() {
	_pBG[0] = nullptr;	// No background

	if (!TinselV2)
		_hBgPal = 0;	// No background palette
}

void Background::ChangePalette(SCNHANDLE hPal) {
	SwapPalette(FindPalette(_hBgPal), hPal);
	SetBackPal(hPal);
}

} // End of namespace Tinsel
