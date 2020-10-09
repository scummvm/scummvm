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

#define SCROLLPIXELS 8 // Number of pixels to scroll per iteration

// Distance from edge that triggers a scroll
#define RLDISTANCE (TinselV2 ? _scrollData.xTrigger : 50)
#define UDISTANCE (TinselV2 ? _scrollData.yTriggerTop : 20)
#define DDISTANCE (TinselV2 ? _scrollData.yTriggerBottom : 20)

// Number of iterations to make
#define RLSCROLL 160 // 20*8 = 160 = half a screen
#define UDSCROLL 100 // 12.5*8 = 100 = half a screen

Scroll::Scroll() {
	_leftScroll = 0;
	_downScroll = 0;
	_scrollActor = 0;
	_pScrollMover = nullptr;
	_oldx = 0;
	_oldy = 0;

	_scrollData = {
	    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	    0,
	    0,
	    // DW2 fields
	    0,
	    0,
	    0,
	    0,
	    0,
	    0,
	    0};

	_imageH = 0;
	_imageW = 0;

	_scrollCursor = 0;

	_scrollPixelsX = SCROLLPIXELS;
	_scrollPixelsY = SCROLLPIXELS;
}

/**
 * Reset the ScrollCursor flag
 */
void Scroll::DontScrollCursor() {
	_scrollCursor = false;
}

/**
 * Set the ScrollCursor flag
 */
void Scroll::DoScrollCursor() {
	_scrollCursor = true;
}

/**
 * Configure a no-scroll boundary for a scene.
 */
void Scroll::SetNoScroll(int x1, int y1, int x2, int y2) {
	if (x1 == x2) {
		/* Vertical line */
		assert(_scrollData.NumNoH < MAX_HNOSCROLL);

		_scrollData.NoHScroll[_scrollData.NumNoH].ln = x1;	// X pos of vertical line
		_scrollData.NoHScroll[_scrollData.NumNoH].c1 = y1;
		_scrollData.NoHScroll[_scrollData.NumNoH].c2 = y2;
		_scrollData.NumNoH++;
	} else if (y1 == y2) {
		/* Horizontal line */
		assert(_scrollData.NumNoV < MAX_VNOSCROLL);

		_scrollData.NoVScroll[_scrollData.NumNoV].ln = y1;	// Y pos of horizontal line
		_scrollData.NoVScroll[_scrollData.NumNoV].c1 = x1;
		_scrollData.NoVScroll[_scrollData.NumNoV].c2 = x2;
		_scrollData.NumNoV++;
	} else {
		/* No-scroll lines must be horizontal or vertical */
	}
}

/**
 * Called from scroll process when it thinks that a scroll is in order.
 * Checks for no-scroll boundaries and sets off a scroll if allowed.
 */
void Scroll::NeedScroll(int direction) {
	uint	i;
	int	BottomLine, RightCol;
	int	Loffset, Toffset;

	// get background offsets
	_vm->_bg->PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	switch (direction) {
	case LEFT:	  /* Picture will go left, 'camera' right */

		BottomLine = Toffset + (SCREEN_HEIGHT - 1);
		RightCol = Loffset + (SCREEN_WIDTH - 1);

		for (i = 0; i < _scrollData.NumNoH; i++) {
			if (RightCol >= _scrollData.NoHScroll[i].ln - 1 && RightCol <= _scrollData.NoHScroll[i].ln + 1 &&
					((_scrollData.NoHScroll[i].c1 >= Toffset && _scrollData.NoHScroll[i].c1 <= BottomLine) ||
					(_scrollData.NoHScroll[i].c2 >= Toffset && _scrollData.NoHScroll[i].c2 <= BottomLine) ||
					(_scrollData.NoHScroll[i].c1 < Toffset && _scrollData.NoHScroll[i].c2 > BottomLine)))
				return;
		}

		if (_leftScroll <= 0) {
			if (TinselV2) {
				_scrollPixelsX = _scrollData.xSpeed;
				_leftScroll += _scrollData.xDistance;
			} else {
				_scrollPixelsX = SCROLLPIXELS;
				_leftScroll = RLSCROLL;
			}
		}
		break;

	case RIGHT:	 /* Picture will go right, 'camera' left */

		BottomLine = Toffset + (SCREEN_HEIGHT - 1);

		for (i = 0; i < _scrollData.NumNoH; i++) {
			if (Loffset >= _scrollData.NoHScroll[i].ln - 1 && Loffset <= _scrollData.NoHScroll[i].ln + 1 &&
					((_scrollData.NoHScroll[i].c1 >= Toffset && _scrollData.NoHScroll[i].c1 <= BottomLine) ||
					(_scrollData.NoHScroll[i].c2 >= Toffset && _scrollData.NoHScroll[i].c2 <= BottomLine) ||
					(_scrollData.NoHScroll[i].c1 < Toffset && _scrollData.NoHScroll[i].c2 > BottomLine)))
				return;
		}

		if (_leftScroll >= 0) {
			if (TinselV2) {
				_scrollPixelsX = _scrollData.xSpeed;
				_leftScroll -= _scrollData.xDistance;
			} else {
				_scrollPixelsX = SCROLLPIXELS;
				_leftScroll = -RLSCROLL;
			}
		}
		break;

	case UP:		/* Picture will go upwards, 'camera' downwards  */

		BottomLine = Toffset + (SCREEN_HEIGHT - 1);
		RightCol = Loffset + (SCREEN_WIDTH - 1);

		for (i = 0; i < _scrollData.NumNoV; i++) {
			if ((BottomLine >= _scrollData.NoVScroll[i].ln - 1 && BottomLine <= _scrollData.NoVScroll[i].ln + 1) &&
					((_scrollData.NoVScroll[i].c1 >= Loffset && _scrollData.NoVScroll[i].c1 <= RightCol) ||
					(_scrollData.NoVScroll[i].c2 >= Loffset && _scrollData.NoVScroll[i].c2 <= RightCol) ||
					(_scrollData.NoVScroll[i].c1 < Loffset && _scrollData.NoVScroll[i].c2 > RightCol)))
				return;
			}

		if (_downScroll <= 0) {
			if (TinselV2) {
				_scrollPixelsY = _scrollData.ySpeed;
				_downScroll += _scrollData.yDistance;
			} else {
				_scrollPixelsY = SCROLLPIXELS;
				_downScroll = UDSCROLL;
			}
		}
		break;

	case DOWN:	  /* Picture will go downwards, 'camera' upwards  */

		RightCol = Loffset + (SCREEN_WIDTH - 1);

		for (i = 0; i < _scrollData.NumNoV; i++) {
			if (Toffset >= _scrollData.NoVScroll[i].ln - 1  && Toffset <= _scrollData.NoVScroll[i].ln + 1  &&
					((_scrollData.NoVScroll[i].c1 >= Loffset && _scrollData.NoVScroll[i].c1 <= RightCol) ||
					(_scrollData.NoVScroll[i].c2 >= Loffset && _scrollData.NoVScroll[i].c2 <= RightCol) ||
					(_scrollData.NoVScroll[i].c1 < Loffset && _scrollData.NoVScroll[i].c2 > RightCol)))
				return;
		}

		if (_downScroll >= 0) {
			if (TinselV2) {
				_scrollPixelsY = _scrollData.ySpeed;
				_downScroll -= _scrollData.yDistance;
			} else {
				_scrollPixelsY = SCROLLPIXELS;
				_downScroll = -UDSCROLL;
			}
		}
		break;

	default:
		break;
	}
}

/**
 * Called from scroll process - Scrolls the image as appropriate.
 */
void Scroll::ScrollImage() {
	int OldLoffset = 0, OldToffset = 0;	// Used when keeping cursor on a tag
	int Loffset, Toffset;
	int curX, curY;

	if (!_leftScroll && !_downScroll) // Only scroll if required
		return;

	// get background offsets
	_vm->_bg->PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	/*
	 * Keeping cursor on a tag?
	 */
	if (_scrollCursor) {
		_vm->_cursor->GetCursorXYNoWait(&curX, &curY, true);
		if (InPolygon(curX, curY, TAG) != NOPOLY || InPolygon(curX, curY, EXIT) != NOPOLY) {
			OldLoffset = Loffset;
			OldToffset = Toffset;
		} else
			_scrollCursor = false;
	}

	/*
	 * Horizontal scrolling
	 */
	if (_leftScroll > 0) {
		_leftScroll -= _scrollPixelsX;
		if (_leftScroll < 0) {
			Loffset += _leftScroll;
			_leftScroll = 0;
		}
		Loffset += _scrollPixelsX;		// Move right
		if (Loffset > _imageW - SCREEN_WIDTH)
			Loffset = _imageW - SCREEN_WIDTH;// Now at extreme right

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MaximumXoffset) &&  (Loffset > SysVar(SV_MaximumXoffset)))
			Loffset = SysVar(SV_MaximumXoffset);

	} else if (_leftScroll < 0) {
		_leftScroll += _scrollPixelsX;
		if (_leftScroll > 0) {
			Loffset += _leftScroll;
			_leftScroll = 0;
		}
		Loffset -= _scrollPixelsX;	// Move left
		if (Loffset < 0)
			Loffset = 0;		// Now at extreme left

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MinimumXoffset) &&  (Loffset < SysVar(SV_MinimumXoffset)))
			Loffset = SysVar(SV_MinimumXoffset);
	}

	/*
	 * Vertical scrolling
	 */
	if (_downScroll > 0) {
		_downScroll -= _scrollPixelsY;
		if (_downScroll < 0) {
			Toffset += _downScroll;
			_downScroll = 0;
		}
		Toffset += _scrollPixelsY;		// Move down

		if (Toffset > _imageH - SCREEN_HEIGHT)
			Toffset = _imageH - SCREEN_HEIGHT;// Now at extreme bottom

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MaximumYoffset) &&  Toffset > SysVar(SV_MaximumYoffset))
			Toffset = SysVar(SV_MaximumYoffset);

	} else if (_downScroll < 0) {
		_downScroll += _scrollPixelsY;
		if (_downScroll > 0) {
			Toffset += _downScroll;
			_downScroll = 0;
		}
		Toffset -= _scrollPixelsY;		// Move up

		if (Toffset < 0)
			Toffset = 0;			// Now at extreme top

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MinimumYoffset) &&  Toffset < SysVar(SV_MinimumYoffset))
			Toffset = SysVar(SV_MinimumYoffset);
	}

	/*
	 * Move cursor if keeping cursor on a tag.
	 */
	if (_scrollCursor)
		_vm->_cursor->AdjustCursorXY(OldLoffset - Loffset, OldToffset - Toffset);

	_vm->_bg->PlayfieldSetPos(FIELD_WORLD, Loffset, Toffset);
}


/**
 * See if the actor on whom the camera is is approaching an edge.
 * Request a scroll if he is.
 */
void Scroll::MonitorScroll() {
	int newx, newy;
	int Loffset, Toffset;

	/*
	 * Only do it if the actor is there and is visible
	 */
	if (!_pScrollMover || MoverHidden(_pScrollMover) || !MoverIs(_pScrollMover))
		return;

	_vm->_actor->GetActorPos(_scrollActor, &newx, &newy);

	if (_oldx == newx && _oldy == newy)
		return;

	_vm->_bg->PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	/*
	 * Approaching right side or left side of the screen?
	 */
	if (newx > Loffset+SCREEN_WIDTH - RLDISTANCE && Loffset < _imageW - SCREEN_WIDTH) {
		if (newx > _oldx)
				NeedScroll(LEFT);
	} else if (newx < Loffset + RLDISTANCE  &&  Loffset) {
		if (newx < _oldx)
				NeedScroll(RIGHT);
	}

	/*
	 * Approaching bottom or top of the screen?
	 */
	if (newy > Toffset+SCREEN_HEIGHT-DDISTANCE && Toffset < _imageH-SCREEN_HEIGHT) {
		if (newy > _oldy)
				NeedScroll(UP);
	} else if (Toffset && newy < Toffset + UDISTANCE + _vm->_actor->GetActorBottom(_scrollActor) - _vm->_actor->GetActorTop(_scrollActor)) {
		if (newy < _oldy)
				NeedScroll(DOWN);
	}

	_oldx = newx;
	_oldy = newy;
}

void Scroll::RestoreScrollDefaults() {
	_scrollData.xTrigger		= SysVar(SV_SCROLL_XTRIGGER);
	_scrollData.xDistance	= SysVar(SV_SCROLL_XDISTANCE);
	_scrollData.xSpeed		= SysVar(SV_SCROLL_XSPEED);
	_scrollData.yTriggerTop	= SysVar(SV_SCROLL_YTRIGGERTOP);
	_scrollData.yTriggerBottom= SysVar(SV_SCROLL_YTRIGGERBOT);
	_scrollData.yDistance	= SysVar(SV_SCROLL_YDISTANCE);
	_scrollData.ySpeed		= SysVar(SV_SCROLL_YSPEED);
}

/**
 * Does the obvious - called at the end of a scene.
 */
void Scroll::DropScroll() {
	_scrollData.NumNoH = _scrollData.NumNoV = 0;
	if (TinselV2) {
		_leftScroll = _downScroll = 0;		// No iterations outstanding
		_oldx = _oldy = 0;
		_scrollPixelsX = _scrollData.xSpeed;
		_scrollPixelsY = _scrollData.ySpeed;
		RestoreScrollDefaults();
	}
}

/**
 * Change which actor the camera is following.
 */
void Scroll::ScrollFocus(int ano) {
	if (_scrollActor != ano) {
		_oldx = _oldy = 0;
		_scrollActor = ano;

		_pScrollMover = ano ? GetMover(_scrollActor) : NULL;
	}
}

/**
 * Returns the actor which the camera is following
 */
int Scroll::GetScrollFocus() {
	return _scrollActor;
}

/**
 * Scroll to abslote position.
 */
void Scroll::ScrollTo(int x, int y, int xIter, int yIter) {
	int Loffset, Toffset;		// for background offsets

	_scrollPixelsX = xIter != 0 ? xIter : (TinselV2 ? _scrollData.xSpeed : SCROLLPIXELS);
	_scrollPixelsY = yIter != 0 ? yIter : (TinselV2 ? _scrollData.ySpeed : SCROLLPIXELS);

	_vm->_bg->PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);	// get background offsets

	_leftScroll = x - Loffset;
	_downScroll = y - Toffset;
}

/**
 * Kill of any current scroll.
 */
void Scroll::KillScroll() {
	_leftScroll = _downScroll = 0;
}

void Scroll::GetNoScrollData(SCROLLDATA *ssd) {
	memcpy(ssd, &_scrollData, sizeof(SCROLLDATA));
}

void Scroll::RestoreNoScrollData(SCROLLDATA *ssd) {
	memcpy(&_scrollData, ssd, sizeof(SCROLLDATA));
}

/**
 * SetScrollParameters
 */
void Scroll::SetScrollParameters(int xTrigger, int xDistance, int xSpeed, int yTriggerTop,
		int yTriggerBottom, int yDistance, int ySpeed) {
	if (xTrigger == 0 && xDistance == 0 && xSpeed == 0
	 && yTriggerTop == 0 && yTriggerBottom && yDistance == 0 && ySpeed == 0) {
		// Restore defaults
		RestoreScrollDefaults();
	} else {
		if (xTrigger)
			_scrollData.xTrigger = xTrigger;
		if (xDistance)
			_scrollData.xDistance = xDistance;
		if (xSpeed)
			_scrollData.xSpeed = xSpeed;
		if (yTriggerTop)
			_scrollData.yTriggerTop = yTriggerTop;
		if (yTriggerBottom)
			_scrollData.yTriggerBottom = yTriggerBottom;
		if (yDistance)
			_scrollData.yDistance = yDistance;
		if (ySpeed)
			_scrollData.ySpeed = ySpeed;
	}
}

bool Scroll::IsScrolling() {
	return (_leftScroll || _downScroll);
}

void Scroll::InitScroll(int width, int height) {
	_imageH = height; // Dimensions
	_imageW = width;  //  of this scene.

	if (!TinselV2) {
		_leftScroll = _downScroll = 0; // No iterations outstanding
		_oldx = _oldy = 0;
		_scrollPixelsX = _scrollPixelsY = SCROLLPIXELS;
	}

	if (!_scrollActor)
		_scrollActor = _vm->_actor->GetLeadId();

	_pScrollMover = GetMover(_scrollActor);
}

/**
 * Decide when to scroll and scroll when decided to.
 */
void ScrollProcess(CORO_PARAM, const void *) {
	int width, height;

	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// In Tinsel v2, scenes may play movies, so the background may not always
	// already be initialized like it is in v1
	while (!_vm->_bg->GetBgObject())
		CORO_SLEEP(1);

	width = _vm->_bg->BgWidth();   // Dimensions
	height = _vm->_bg->BgHeight(); // of this scene.

	// Give up if there'll be no purpose in this process
	if (width == SCREEN_WIDTH && height == SCREEN_HEIGHT)
		CORO_KILL_SELF();

	_vm->_scroll->InitScroll(width, height);

	while (1) {
		_vm->_scroll->MonitorScroll(); // Set scroll requirement
		_vm->_scroll->ScrollImage();

		CORO_SLEEP(1); // allow re-scheduling
	}

	CORO_END_CODE;
}

} // End of namespace Tinsel
