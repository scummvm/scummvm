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
 * Data structures used for handling backgrounds
 */

#ifndef TINSEL_BACKGND_H     // prevent multiple includes
#define TINSEL_BACKGND_H

#include "common/array.h"
#include "common/coroutines.h"
#include "common/frac.h"
#include "common/rect.h"
#include "tinsel/anim.h"	// for ANIM
#include "tinsel/dw.h"	// for SCNHANDLE
#include "tinsel/object.h"	// for POBJECT
#include "tinsel/palette.h"	// palette definitions

namespace Tinsel {

struct OBJECT;


/** Scrolling padding. Needed because scroll process does not normally run on every frame */
enum {
	SCROLLX_PAD	= 64,
	SCROLLY_PAD	= 64
};

/** When module BLK_INFO list is this long, switch from a binary to linear search */
#define	LINEAR_SEARCH	5

/** background playfield structure - a playfield is a container for modules */
struct PLAYFIELD {
	OBJECT *pDispList;	///< object display list for this playfield
	frac_t fieldX;		///< current world x position of playfield
	frac_t fieldY;		///< current world y position of playfield
	frac_t fieldXvel;	///< current x velocity of playfield
	frac_t fieldYvel;	///< current y velocity of playfield
	Common::Rect rcClip;	///< clip rectangle for this playfield
	bool bMoved;		///< set when playfield has moved
};

/** multi-playfield background structure - a backgnd is a container of playfields */
struct BACKGND {
	COLORREF rgbSkyColor;	///< background sky color
	Common::Point ptInitWorld;		///< initial world position
	Common::Rect rcScrollLimits;	///< scroll limits
	int refreshRate;		///< background update process refresh rate
	frac_t *pXscrollTable;	///< pointer to x direction scroll table for this background
	frac_t *pYscrollTable;	///< pointer to y direction scroll table for this background
	Common::Array<PLAYFIELD> fieldArray;	///< list of all playfields for this background
	bool bAutoErase;		///< when set - screen is cleared before anything is plotted (unused)
};


/*----------------------------------------------------------------------*\
|*			Background Function Prototypes			*|
\*----------------------------------------------------------------------*/

#define MAX_BG	10

class Font;

class Background {
public:
	Background(Font* font);

	void InitBackground();

	void DrawBackgnd();		// Draws all playfields for the current background

	/**
	 * Called before scene change.
	 */
	void DropBackground();

	void ResetBackground() {
		_pCurBgnd->fieldArray.clear();
		delete _pCurBgnd;
		_pCurBgnd = nullptr;
	}

	void StartupBackground(CORO_PARAM, SCNHANDLE hFilm);

	void PlayfieldSetPos(		// Sets the xy position of the specified playfield in the current background
		int which,		// which playfield
		int newXpos,		// new x position
		int newYpos);		// new y position

	void PlayfieldGetPos(		// Returns the xy position of the specified playfield in the current background
		int which,		// which playfield
		int* pXpos,		// returns current x position
		int* pYpos);		// returns current y position

	int PlayfieldGetCenterX(	// Returns the xy position of the specified playfield in the current background
		int which);		// which playfield

	OBJECT** GetPlayfieldList(	// Returns the display list for the specified playfield
		int which);		// which playfield

	OBJECT* GetBgObject() { return _pBG[0]; }

	void ChangePalette(SCNHANDLE hPal);

	SCNHANDLE BgPal() { return _hBgPal; }

	void SetDoFadeIn(bool tf) { _bDoFadeIn = tf; }

	bool GetDoFadeIn() { return _bDoFadeIn; }

	/**
	 * Return the current scene handle.
	 */
	SCNHANDLE GetBgroundHandle() { return _hBackground; }

	/**
	 * Return the width of the current background.
	 */
	int BgWidth();

	/**
	 * Return the height of the current background.
	 */
	int BgHeight();

	void SetBackPal(SCNHANDLE hPal);

	int getBgSpeed() { return _BGspeed; }

private:
	Font *_font;

	// current background
	BACKGND *_pCurBgnd;

	SCNHANDLE _hBgPal;	// Background's palette
	int _BGspeed;
	SCNHANDLE _hBackground;	// Current scene handle - stored in case of Save_Scene()
	bool _bDoFadeIn;

public:
	int _bgReels;
	POBJECT _pBG[MAX_BG];
	ANIM	_thisAnim[MAX_BG];	// used by BGmainProcess()
};

} // End of namespace Tinsel

#endif	// TINSEL_BACKGND_H
