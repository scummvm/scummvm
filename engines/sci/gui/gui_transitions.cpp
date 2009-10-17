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
 */

#include "common/util.h"
#include "common/stack.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/gui/gui.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_palette.h"
#include "sci/gui/gui_transitions.h"

namespace Sci {

SciGuiTransitions::SciGuiTransitions(SciGui *gui, SciGuiScreen *screen, SciGuiPalette *palette, bool isVGA)
	: _gui(gui), _screen(screen), _palette(palette), _isVGA(isVGA) {
	init();
}

SciGuiTransitions::~SciGuiTransitions() {
	delete[] _oldScreen;
}

// This table contains a mapping between oldIDs (prior SCI1LATE) and newIDs
static const GuiTransitionTranslateEntry oldTransitionIDs[] = {
	{   0, SCI_TRANSITIONS_VERTICALROLL_FROMCENTER,		false },
	{   1, SCI_TRANSITIONS_HORIZONTALROLL_FROMCENTER,	false },
	{   2, SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT,			false },
	{   3, SCI_TRANSITIONS_STRAIGHT_FROM_LEFT,			false },
	{   4, SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM,		false },
	{   5, SCI_TRANSITIONS_STRAIGHT_FROM_TOP,			false },
	{   6, SCI_TRANSITIONS_DIAGONALROLL_FROMCENTER,		false },
	{   7, SCI_TRANSITIONS_DIAGONALROLL_TOCENTER,		false },
	{   8, SCI_TRANSITIONS_BLOCKS,						false },
	{   9, SCI_TRANSITIONS_VERTICALROLL_TOCENTER,		false },
	{  10, SCI_TRANSITIONS_HORIZONTALROLL_TOCENTER,		false },
	{  11, SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT,			true },
	{  12, SCI_TRANSITIONS_STRAIGHT_FROM_LEFT,			true },
	{  13, SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM,		true },
	{  14, SCI_TRANSITIONS_STRAIGHT_FROM_TOP,			true },
	{  15, SCI_TRANSITIONS_DIAGONALROLL_FROMCENTER,		true },
	{  16, SCI_TRANSITIONS_DIAGONALROLL_TOCENTER,		true },
	{  17, SCI_TRANSITIONS_BLOCKS,						true },
	{  18, SCI_TRANSITIONS_PIXELATION,					false },
	{  27, SCI_TRANSITIONS_PIXELATION	,				true },
	{  30, SCI_TRANSITIONS_FADEPALETTE,					false },
	{  40, SCI_TRANSITIONS_SCROLL_RIGHT,				false },
	{  41, SCI_TRANSITIONS_SCROLL_LEFT,					false },
	{  42, SCI_TRANSITIONS_SCROLL_UP,					false },
	{  43, SCI_TRANSITIONS_SCROLL_DOWN,					false },
	{ 100, SCI_TRANSITIONS_NONE,						false },
	{ 255, 255,											false }
};

// this table defines the blackout-transition that needs to be done prior doing the actual transition
static const GuiTransitionTranslateEntry blackoutTransitionIDs[] = {
	{ SCI_TRANSITIONS_VERTICALROLL_FROMCENTER,			SCI_TRANSITIONS_VERTICALROLL_TOCENTER,		true },
	{ SCI_TRANSITIONS_HORIZONTALROLL_FROMCENTER,		SCI_TRANSITIONS_HORIZONTALROLL_TOCENTER,	true },
	{ SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT,				SCI_TRANSITIONS_STRAIGHT_FROM_LEFT,			true },
	{ SCI_TRANSITIONS_STRAIGHT_FROM_LEFT,				SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT,		true },
	{ SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM,				SCI_TRANSITIONS_STRAIGHT_FROM_TOP,			true },
	{ SCI_TRANSITIONS_STRAIGHT_FROM_TOP,				SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM,		true },
	{ SCI_TRANSITIONS_DIAGONALROLL_FROMCENTER,			SCI_TRANSITIONS_DIAGONALROLL_TOCENTER,		true },
	{ SCI_TRANSITIONS_DIAGONALROLL_TOCENTER,			SCI_TRANSITIONS_DIAGONALROLL_FROMCENTER,	true },
	{ SCI_TRANSITIONS_BLOCKS,							SCI_TRANSITIONS_BLOCKS,						true },
	{ SCI_TRANSITIONS_PIXELATION,						SCI_TRANSITIONS_PIXELATION,					true },
	{ SCI_TRANSITIONS_FADEPALETTE,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_SCROLL_RIGHT,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_SCROLL_LEFT,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_SCROLL_UP,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_SCROLL_DOWN,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_NONE_LONGBOW,						SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_NONE,								SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_VERTICALROLL_TOCENTER,			SCI_TRANSITIONS_NONE,						true },
	{ SCI_TRANSITIONS_HORIZONTALROLL_TOCENTER,			SCI_TRANSITIONS_NONE,						true },
	{ 255,												255,										true }
};

void SciGuiTransitions::init() {
	_oldScreen = new byte[_screen->_displayHeight * _screen->_displayWidth];

	if (getSciVersion() >= SCI_VERSION_1_LATE)
		_translationTable = NULL;
	else
		_translationTable = oldTransitionIDs;

	// setup default transition
	_number = SCI_TRANSITIONS_HORIZONTALROLL_FROMCENTER;
	_blackoutFlag = false;
}

void SciGuiTransitions::setup(int16 number, bool blackoutFlag) {
	if (number != -1) {
		_number = number;
		_blackoutFlag = blackoutFlag;
	}
}

// will translate a number and return corresponding translationEntry
const GuiTransitionTranslateEntry *SciGuiTransitions::translateNumber (int16 number, const GuiTransitionTranslateEntry *tablePtr) {
	while (1) {
		if (tablePtr->orgId == 255)
			return NULL;
		if (tablePtr->orgId == number)
			return tablePtr;
		tablePtr++;
	}
}

void SciGuiTransitions::doit(Common::Rect picRect) {
	const GuiTransitionTranslateEntry *translationEntry = _translationTable;

	_picRect = picRect;

	if (_translationTable) {
		// We need to translate the ID
		translationEntry = translateNumber(_number, _translationTable);
		if (translationEntry) {
			_number = translationEntry->newId;
			_blackoutFlag = translationEntry->blackoutFlag;
		} else {
			warning("SciGuiTransitions: old ID %d not supported", _number);
			_number = SCI_TRANSITIONS_NONE;
			_blackoutFlag = false;
		}
	}

	if (_blackoutFlag) {
		// We need to find out what transition we are supposed to use for blackout
		translationEntry = translateNumber(_number, blackoutTransitionIDs);
		if (translationEntry) {
			doTransition(translationEntry->newId, true);
		} else {
			warning("SciGuiTransitions: ID %d not listed in blackoutTransitionIDs", _number);
		}
	}

	// Now we do the actual transition to the new screen
	doTransition(_number, false);

	_screen->_picNotValid = 0;
}

// This may get called twice, if blackoutFlag is set. It will get once called with blackoutFlag set and another time
//  with no blackoutFlag.
void SciGuiTransitions::doTransition(int16 number, bool blackoutFlag) {
	if (number != SCI_TRANSITIONS_FADEPALETTE) {
		setNewPalette(blackoutFlag);
	}

	switch (number) {
	case SCI_TRANSITIONS_VERTICALROLL_FROMCENTER:
		verticalRollFromCenter(blackoutFlag);
		break;
	case SCI_TRANSITIONS_VERTICALROLL_TOCENTER:
		verticalRollFromCenter(blackoutFlag);
		break;
	case SCI_TRANSITIONS_HORIZONTALROLL_FROMCENTER:
		horizontalRollFromCenter(blackoutFlag);
		break;
	case SCI_TRANSITIONS_HORIZONTALROLL_TOCENTER:
		horizontalRollToCenter(blackoutFlag);
		break;
	case SCI_TRANSITIONS_DIAGONALROLL_TOCENTER:
		diagonalRollToCenter(blackoutFlag);
		break;
	case SCI_TRANSITIONS_DIAGONALROLL_FROMCENTER:
		diagonalRollFromCenter(blackoutFlag);
		break;

	case SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT:
	case SCI_TRANSITIONS_STRAIGHT_FROM_LEFT:
	case SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM:
	case SCI_TRANSITIONS_STRAIGHT_FROM_TOP:
		straight(number, blackoutFlag);
		break;

	case SCI_TRANSITIONS_PIXELATION:
		pixelation(blackoutFlag);
		break;

	case SCI_TRANSITIONS_BLOCKS:
		blocks(blackoutFlag);
		break;

	case SCI_TRANSITIONS_FADEPALETTE:
		if (!blackoutFlag) {
			fadeOut(); setNewScreen(blackoutFlag); fadeIn();
		}
		break;

	case SCI_TRANSITIONS_SCROLL_RIGHT:
	case SCI_TRANSITIONS_SCROLL_LEFT:
	case SCI_TRANSITIONS_SCROLL_UP:
	case SCI_TRANSITIONS_SCROLL_DOWN:
		scroll(number);
		break;

	case SCI_TRANSITIONS_NONE_LONGBOW:
	case SCI_TRANSITIONS_NONE:
		setNewScreen(blackoutFlag);
		break;

	default:
		warning("SciGuiTransitions: ID %d not implemented", number);
		setNewScreen(blackoutFlag);
	}
}

void SciGuiTransitions::setNewPalette(bool blackoutFlag) {
	if (!blackoutFlag)
		if (_isVGA)
			_palette->setOnScreen();
}

void SciGuiTransitions::setNewScreen(bool blackoutFlag) {
	if (!blackoutFlag) {
		_screen->copyRectToScreen(_picRect);
		g_system->updateScreen();
	}
}

void SciGuiTransitions::copyRectToScreen(const Common::Rect rect, bool blackoutFlag) {
	if (!blackoutFlag) {
		_screen->copyRectToScreen(rect);
	} else {
		Graphics::Surface *surface = g_system->lockScreen();
		surface->fillRect(rect, 0);
		g_system->unlockScreen();
	}
}

// Note: dont do too many steps in here, otherwise cpu will crap out because of the load
void SciGuiTransitions::fadeOut() {
	byte oldPalette[4 * 256], workPalette[4 * 256];
	int16 stepNr, colorNr;

	g_system->grabPalette(oldPalette, 0, 256);

	for (stepNr = 100; stepNr >= 0; stepNr -= 10) {
		for (colorNr = 1; colorNr < 255; colorNr++){
			workPalette[colorNr * 4 + 0] = oldPalette[colorNr * 4] * stepNr / 100;
			workPalette[colorNr * 4 + 1] = oldPalette[colorNr * 4 + 1] * stepNr / 100;
			workPalette[colorNr * 4 + 2] = oldPalette[colorNr * 4 + 2] * stepNr / 100;
		}
		g_system->setPalette(workPalette + 4, 1, 254);
		_gui->wait(2);
	}
}

// Note: dont do too many steps in here, otherwise cpu will crap out because of the load
void SciGuiTransitions::fadeIn() {
	int16 stepNr;

	for (stepNr = 0; stepNr <= 100; stepNr += 10) {
		_palette->setIntensity(1, 255, stepNr, true);
		_gui->wait(2);
	}
}

// pixelates the new picture over the old one - works against the whole screen
// TODO: it seems this needs to get applied on _picRect only if possible
void SciGuiTransitions::pixelation (bool blackoutFlag) {
	uint16 mask = 0x40, stepNr = 0;
	Common::Rect pixelRect;

	do {
		mask = (mask & 1) ? (mask >> 1) ^ 0xB400 : mask >> 1;
		if (mask >= 320 * 200)
			continue;
		pixelRect.left = mask % 320; pixelRect.right = pixelRect.left + 1;
		pixelRect.top = mask / 320;	pixelRect.bottom = pixelRect.top + 1;
		copyRectToScreen(pixelRect, blackoutFlag);
		if ((stepNr & 0x3FF) == 0) {
			g_system->updateScreen();
			g_system->delayMillis(5);
		}
		stepNr++;
	} while (mask != 0x40);
}

// like pixelation but uses 8x8 blocks - works against the whole screen
// TODO: it seems this needs to get applied on _picRect only if possible
void SciGuiTransitions::blocks(bool blackoutFlag) {
	uint16 mask = 0x40, stepNr = 0;
	Common::Rect blockRect;

	do {
		mask = (mask & 1) ? (mask >> 1) ^ 0x240 : mask >> 1;
		if (mask >= 40 * 25)
			continue;
		blockRect.left = (mask % 40) << 3; blockRect.right = blockRect.left + 8;
		blockRect.top = (mask / 40) << 3; blockRect.bottom = blockRect.top + 8;
		copyRectToScreen(blockRect, blackoutFlag);
		if ((stepNr & 7) == 0) {
			g_system->updateScreen();
			g_system->delayMillis(4);
		}
		stepNr++;
	} while (mask != 0x40);
}

// directly shows new screen starting up/down/left/right and going to the opposite direction - works on _picRect area only
void SciGuiTransitions::straight(int16 number, bool blackoutFlag) {
	int16 stepNr = 0;
	Common::Rect newScreenRect = _picRect;

	switch (number) {
	case SCI_TRANSITIONS_STRAIGHT_FROM_RIGHT:
		newScreenRect.left = newScreenRect.right - 1;
		while (newScreenRect.left >= _picRect.left) {
			copyRectToScreen(newScreenRect, blackoutFlag);
			if ((stepNr & 1) == 0) {
				g_system->updateScreen();
				g_system->delayMillis(1);
			}
			stepNr++;
			newScreenRect.translate(-1, 0);
		}
		break;

	case SCI_TRANSITIONS_STRAIGHT_FROM_LEFT:
		newScreenRect.right = newScreenRect.left + 1;
		while (newScreenRect.right <= _picRect.right) {
			copyRectToScreen(newScreenRect, blackoutFlag);
			if ((stepNr & 1) == 0) {
				g_system->updateScreen();
				g_system->delayMillis(1);
			}
			stepNr++;
			newScreenRect.translate(1, 0);
		}
		break;

	case SCI_TRANSITIONS_STRAIGHT_FROM_BOTTOM:
		newScreenRect.top = newScreenRect.bottom - 1;
		while (newScreenRect.top >= _picRect.top) {
			copyRectToScreen(newScreenRect, blackoutFlag);
			g_system->updateScreen();
			g_system->delayMillis(3);
			stepNr++;
			newScreenRect.translate(0, -1);
		}
		break;

	case SCI_TRANSITIONS_STRAIGHT_FROM_TOP:
		newScreenRect.bottom = newScreenRect.top + 1;
		while (newScreenRect.bottom <= _picRect.bottom) {
			copyRectToScreen(newScreenRect, blackoutFlag);
			g_system->updateScreen();
			g_system->delayMillis(3);
			stepNr++;
			newScreenRect.translate(0, 1);
		}
		break;
	}
}

// scroll old screen (up/down/left/right) and insert new screen that way - works on _picRect area only
void SciGuiTransitions::scroll(int16 number) {
	int16 screenWidth, screenHeight;
	byte *oldScreenPtr;
	int16 stepNr = 0;
	Common::Rect oldMoveRect = _picRect;
	Common::Rect newMoveRect = _picRect;
	Common::Rect newScreenRect = _picRect;

	_screen->copyFromScreen(_oldScreen);
	screenWidth = _screen->_displayWidth; screenHeight = _screen->_displayHeight;

	oldScreenPtr = _oldScreen + _picRect.left + _picRect.top * screenWidth;

	switch (number) {
	case SCI_TRANSITIONS_SCROLL_LEFT:
		newScreenRect.right = newScreenRect.left;
		newMoveRect.left = newMoveRect.right;
		while (oldMoveRect.left < oldMoveRect.right) {
			oldScreenPtr++; oldMoveRect.right--;
			if (oldMoveRect.right > oldMoveRect.left)
				g_system->copyRectToScreen(oldScreenPtr, screenWidth, oldMoveRect.left, oldMoveRect.top, oldMoveRect.width(), oldMoveRect.height());
			newScreenRect.right++; newMoveRect.left--;
			_screen->copyRectToScreen(newScreenRect, newMoveRect.left, newMoveRect.top);
			if ((stepNr & 1) == 0) {
				g_system->updateScreen();
				g_system->delayMillis(1);
			}
			stepNr++;
		}
		if ((stepNr & 1) == 0)
			g_system->updateScreen();
		break;

	case SCI_TRANSITIONS_SCROLL_RIGHT:
		newScreenRect.left = newScreenRect.right;
		while (oldMoveRect.left < oldMoveRect.right) {
			oldMoveRect.left++;
			if (oldMoveRect.right > oldMoveRect.left)
				g_system->copyRectToScreen(oldScreenPtr, screenWidth, oldMoveRect.left, oldMoveRect.top, oldMoveRect.width(), oldMoveRect.height());
			newScreenRect.left--;
			_screen->copyRectToScreen(newScreenRect, newMoveRect.left, newMoveRect.top);
			if ((stepNr & 1) == 0) {
				g_system->updateScreen();
				g_system->delayMillis(1);
			}
			stepNr++;
		}
		if ((stepNr & 1) == 0)
			g_system->updateScreen();
		break;

	case SCI_TRANSITIONS_SCROLL_UP:
		newScreenRect.bottom = newScreenRect.top;
		newMoveRect.top = newMoveRect.bottom;
		while (oldMoveRect.top < oldMoveRect.bottom) {
			oldScreenPtr += screenWidth; oldMoveRect.top++;
			if (oldMoveRect.top < oldMoveRect.bottom)
				g_system->copyRectToScreen(oldScreenPtr, screenWidth, _picRect.left, _picRect.top, oldMoveRect.width(), oldMoveRect.height());
			newScreenRect.bottom++;	newMoveRect.top--;
			_screen->copyRectToScreen(newScreenRect, newMoveRect.left, newMoveRect.top);
			g_system->updateScreen();
			g_system->delayMillis(3);
		}
		break;

	case SCI_TRANSITIONS_SCROLL_DOWN:
		newScreenRect.top = newScreenRect.bottom;
		while (oldMoveRect.top < oldMoveRect.bottom) {
			oldMoveRect.top++;
			if (oldMoveRect.top < oldMoveRect.bottom)
				g_system->copyRectToScreen(oldScreenPtr, screenWidth, oldMoveRect.left, oldMoveRect.top, oldMoveRect.width(), oldMoveRect.height());
			newScreenRect.top--;
			_screen->copyRectToScreen(newScreenRect, _picRect.left, _picRect.top);
			g_system->updateScreen();
			g_system->delayMillis(3);
		}
		break;
	}
}

// vertically displays new screen starting from center - works on _picRect area only
void SciGuiTransitions::verticalRollFromCenter(bool blackoutFlag) {
	Common::Rect leftRect = Common::Rect(_picRect.left + (_picRect.width() / 2) -1, _picRect.top, _picRect.left + (_picRect.width() / 2), _picRect.bottom);
	Common::Rect rightRect = Common::Rect(leftRect.right, _picRect.top, leftRect.right + 1, _picRect.bottom);

	while ((leftRect.left >= _picRect.left) || (rightRect.right <= _picRect.right)) {
		if (leftRect.left < _picRect.left)
			leftRect.translate(1, 0);
		if (rightRect.right > _picRect.right)
			rightRect.translate(-1, 0);
		copyRectToScreen(leftRect, blackoutFlag); leftRect.translate(-1, 0);
		copyRectToScreen(rightRect, blackoutFlag); rightRect.translate(1, 0);
		g_system->updateScreen();
		g_system->delayMillis(2);
	}
}

// vertically displays new screen starting from edges - works on _picRect area only
void SciGuiTransitions::verticalRollToCenter(bool blackoutFlag) {
	Common::Rect leftRect = Common::Rect(_picRect.left, _picRect.top, _picRect.left + 1, _picRect.bottom);
	Common::Rect rightRect = Common::Rect(leftRect.right - 1, _picRect.top, leftRect.right, _picRect.bottom);

	while (leftRect.left < rightRect.right) {
		copyRectToScreen(leftRect, blackoutFlag); leftRect.translate(1, 0);
		copyRectToScreen(rightRect, blackoutFlag); rightRect.translate(-1, 0);
		g_system->updateScreen();
		g_system->delayMillis(2);
	}
}

// horizontally displays new screen starting from center - works on _picRect area only
void SciGuiTransitions::horizontalRollFromCenter(bool blackoutFlag) {
	Common::Rect upperRect = Common::Rect(_picRect.left, _picRect.top + (_picRect.height() / 2) - 1, _picRect.right, _picRect.top + (_picRect.height() / 2));
	Common::Rect lowerRect = Common::Rect(upperRect.left, upperRect.bottom, upperRect.right, upperRect.bottom + 1);

	while ((upperRect.top >= _picRect.top) || (lowerRect.bottom <= _picRect.bottom)) {
		if (upperRect.top < _picRect.top)
			upperRect.translate(0, 1);
		if (lowerRect.bottom > _picRect.bottom)
			lowerRect.translate(0, -1);
		copyRectToScreen(upperRect, blackoutFlag); upperRect.translate(0, -1);
		copyRectToScreen(lowerRect, blackoutFlag); lowerRect.translate(0, 1);
		g_system->updateScreen();
		g_system->delayMillis(3);
	}
}

// horizontally displays new screen starting from upper and lower edge - works on _picRect area only
void SciGuiTransitions::horizontalRollToCenter(bool blackoutFlag) {
	Common::Rect upperRect = Common::Rect(_picRect.left, _picRect.top, _picRect.right, _picRect.top + 1);
	Common::Rect lowerRect = Common::Rect(upperRect.left, _picRect.bottom - 1, upperRect.right, _picRect.bottom);

	while (upperRect.top < lowerRect.bottom) {
		copyRectToScreen(upperRect, blackoutFlag); upperRect.translate(0, 1);
		copyRectToScreen(lowerRect, blackoutFlag); lowerRect.translate(0, -1);
		g_system->updateScreen();
		g_system->delayMillis(3);
	}
}

// diagonally displays new screen starting from center - works on _picRect area only
//  assumes that height of rect is larger than width
void SciGuiTransitions::diagonalRollFromCenter(bool blackoutFlag) {
	int16 halfHeight = _picRect.height() / 2;
	Common::Rect upperRect(_picRect.left + halfHeight - 2, _picRect.top + halfHeight, _picRect.right - halfHeight + 1, _picRect.top + halfHeight + 1);
	Common::Rect lowerRect(upperRect.left, upperRect.top, upperRect.right, upperRect.bottom);
	Common::Rect leftRect(upperRect.left, upperRect.top, upperRect.left + 1, lowerRect.bottom);
	Common::Rect rightRect(upperRect.right, upperRect.top, upperRect.right + 1, lowerRect.bottom);

	while ((upperRect.top >= _picRect.top) || (lowerRect.bottom <= _picRect.bottom)) {
		if (upperRect.top < _picRect.top) {
			upperRect.translate(0, 1); leftRect.top++; rightRect.top++;
		}
		if (lowerRect.bottom > _picRect.bottom) {
			lowerRect.translate(0, -1); leftRect.bottom--; rightRect.bottom--;
		}
		if (leftRect.left < _picRect.left) {
			leftRect.translate(1, 0); upperRect.left++; lowerRect.left++;
		}
		if (rightRect.right > _picRect.right) {
			rightRect.translate(-1, 0); upperRect.right--; lowerRect.right--;
		}
		copyRectToScreen(upperRect, blackoutFlag); upperRect.translate(0, -1); upperRect.left--; upperRect.right++;
		copyRectToScreen(lowerRect, blackoutFlag); lowerRect.translate(0, 1); lowerRect.left--; lowerRect.right++;
		copyRectToScreen(leftRect, blackoutFlag); leftRect.translate(-1, 0);	leftRect.top--; leftRect.bottom++;
		copyRectToScreen(rightRect, blackoutFlag); rightRect.translate(1, 0); rightRect.top--; rightRect.bottom++;
		g_system->updateScreen();
		g_system->delayMillis(3);
	}
}

// diagonally displays new screen starting from edges - works on _picRect area only
//  assumes that height of rect is larger than width
void SciGuiTransitions::diagonalRollToCenter(bool blackoutFlag) {
	Common::Rect upperRect(_picRect.left, _picRect.top, _picRect.right, _picRect.top + 1);
	Common::Rect lowerRect(_picRect.left, _picRect.bottom - 1, _picRect.right, _picRect.bottom);
	Common::Rect leftRect(_picRect.left, _picRect.top, _picRect.left + 1, _picRect.bottom);
	Common::Rect rightRect(_picRect.right - 1, _picRect.top, _picRect.right, _picRect.bottom);

	while (upperRect.top < lowerRect.bottom) {
		copyRectToScreen(upperRect, blackoutFlag); upperRect.translate(0, 1); upperRect.left++; upperRect.right--;
		copyRectToScreen(lowerRect, blackoutFlag); lowerRect.translate(0, -1); lowerRect.left++; lowerRect.right--;
		copyRectToScreen(leftRect, blackoutFlag); leftRect.translate(1, 0);
		copyRectToScreen(rightRect, blackoutFlag); rightRect.translate(-1, 0);
		g_system->updateScreen();
		g_system->delayMillis(3);
	}
}

} // End of namespace Sci
