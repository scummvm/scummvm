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

#include "stdafx.h"
#include "common/rect.h"
#include "bs2/driver/driver96.h"
#include "bs2/driver/menu.h"
#include "bs2/driver/d_draw.h"
#include "bs2/driver/render.h"

namespace Sword2 {

#define MENUDEEP 40
#define MAXMENUANIMS 8

void Display::clearIconArea(int menu, int pocket, Common::Rect *r) {
	byte *dst;
	int i;

	r->top = menu * (RENDERDEEP + MENUDEEP) + (MENUDEEP - RDMENU_ICONDEEP) / 2;
	r->bottom = r->top + RDMENU_ICONDEEP;
	r->left = RDMENU_ICONSTART + pocket * (RDMENU_ICONWIDE + RDMENU_ICONSPACING);
	r->right = r->left + RDMENU_ICONWIDE;

	dst = _buffer + r->top * _screenWide + r->left;

	for (i = 0; i < RDMENU_ICONDEEP; i++) {
		memset(dst, 0, RDMENU_ICONWIDE);
		dst += _screenWide;
	}
}

/**
 * This function should be called regularly to process the menubar system. The
 * rate at which this function is called will dictate how smooth the menu
 * system is.
 */

void Display::processMenu(void) {
	byte *src, *dst;
	uint8 menu;
	uint8 i, j;
	uint8 complete;
	uint8 frameCount;
	int32 curx, xoff;
	int32 cury, yoff;
	Common::Rect r1, r2;
	int32 delta;
	static int32 lastTime = 0;

	if (lastTime == 0) {
		lastTime = SVM_timeGetTime();
		frameCount = 1;
	} else {
		delta = SVM_timeGetTime() - lastTime;
		if (delta > 250) {
			lastTime += delta;
			delta = 250;
			frameCount = 1;
		} else {
			frameCount = (uint8) ((_iconCount + 8) * delta / 750);
			lastTime += frameCount * 750 / (_iconCount + 8);
		}
	}

	while (frameCount-- > 0) {
		for (menu = RDMENU_TOP; menu <= RDMENU_BOTTOM; menu++) {
			if (_menuStatus[menu] == RDMENU_OPENING) {
				// The menu is opening, so process it here
				complete = 1;

				// Propagate the animation from the first icon.
				for (i = RDMENU_MAXPOCKETS - 1; i > 0; i--) {
					_pocketStatus[menu][i] = _pocketStatus[menu][i - 1];
					if (_pocketStatus[menu][i] != MAXMENUANIMS)
						complete = 0;
				}
				if (_pocketStatus[menu][i] != MAXMENUANIMS)
					complete = 0;

				// ... and animate the first icon
				if (_pocketStatus[menu][0] != MAXMENUANIMS)
					_pocketStatus[menu][0]++;

				// Check to see if the menu is fully open
				if (complete)
					_menuStatus[menu] = RDMENU_SHOWN;
			} else if (_menuStatus[menu] == RDMENU_CLOSING) {
				// The menu is closing, so process it here
				complete = 1;

				// Propagate the animation from the first icon.
				for (i = RDMENU_MAXPOCKETS - 1; i > 0; i--) {
					if (_icons[menu][i] && _pocketStatus[menu][i] != 0 && _pocketStatus[menu][i - 1] == 0) {
						clearIconArea(menu, i, &r1);
						updateRect(&r1);
					}

					_pocketStatus[menu][i] = _pocketStatus[menu][i - 1];
					if (_pocketStatus[menu][i] != 0)
						complete = 0;
				}
				if (_pocketStatus[menu][i] != 0)
					complete = 0;

				// ... and animate the first icon
				if (_pocketStatus[menu][0] != 0) {
					_pocketStatus[menu][0]--;

					if (_pocketStatus[menu][0] == 0) {
						clearIconArea(menu, 0, &r1);
						updateRect(&r1);
					}
				}

				// Check to see if the menu is fully closed
				if (complete)
					_menuStatus[menu] = RDMENU_HIDDEN;
			}
		}
	}
	
	// Does the menu need to be drawn?
	for (menu = RDMENU_TOP; menu <= RDMENU_BOTTOM; menu++) {
		if (_menuStatus[menu] != RDMENU_HIDDEN) {
			// Draw the menu here.
			curx = RDMENU_ICONSTART + RDMENU_ICONWIDE / 2;
			cury = (MENUDEEP / 2) + (RENDERDEEP + MENUDEEP) * menu;

			for (i = 0; i < RDMENU_MAXPOCKETS; i++) {
				if (_icons[menu][i]) {
					// Since we no longer clear the screen
					// after each frame we need to clear
					// the icon area.

					clearIconArea(menu, i, &r1);
					
					if (_pocketStatus[menu][i] == MAXMENUANIMS) {
						xoff = (RDMENU_ICONWIDE / 2);
						r2.left = curx - xoff;
						r2.right = r2.left + RDMENU_ICONWIDE;
						yoff = (RDMENU_ICONDEEP / 2);
						r2.top = cury - yoff;
						r2.bottom = r2.top + RDMENU_ICONDEEP;
					} else {
						xoff = (RDMENU_ICONWIDE / 2) * _pocketStatus[menu][i] / MAXMENUANIMS;
						r2.left = curx - xoff;
						r2.right = curx + xoff;
						yoff = (RDMENU_ICONDEEP / 2) * _pocketStatus[menu][i] / MAXMENUANIMS;
						r2.top = cury - yoff;
						r2.bottom = cury + yoff;
					}

					if (xoff != 0 && yoff != 0) {
						dst = _buffer + r2.top * _screenWide + r2.left;
						src = _icons[menu][i];

						if (_pocketStatus[menu][i] != MAXMENUANIMS) {
							squashImage(
								dst, _screenWide, r2.right - r2.left, r2.bottom - r2.top,
								src, RDMENU_ICONWIDE, RDMENU_ICONWIDE, RDMENU_ICONDEEP, NULL);
						} else {
							for (j = 0; j < RDMENU_ICONDEEP; j++) {
								memcpy(dst, src, RDMENU_ICONWIDE);
								src += RDMENU_ICONWIDE;
								dst += _screenWide;
							}
						}
						updateRect(&r1);
					}
				}
				curx += (RDMENU_ICONSPACING + RDMENU_ICONWIDE);
				r1.left += (RDMENU_ICONSPACING + RDMENU_ICONWIDE);
				r1.right += (RDMENU_ICONSPACING + RDMENU_ICONWIDE);
			}
		}
	}
}

/**
 * This function brings a specified menu into view. 
 * @param menu RDMENU_TOP or RDMENU_BOTTOM, depending on which menu to show
 * @return RD_OK, or an error code
 */

int32 Display::showMenu(uint8 menu) {
	// Check for invalid menu parameter
	if (menu > RDMENU_BOTTOM)
		return RDERR_INVALIDMENU;

	// Check that the menu is not currently shown, or in the process of
	// being shown.
	if (_menuStatus[menu] == RDMENU_SHOWN || _menuStatus[menu] == RDMENU_OPENING)
		return RDERR_INVALIDCOMMAND;

	_menuStatus[menu] = RDMENU_OPENING;
	return RD_OK;
}

/**
 * This function hides a specified menu.
 * @param menu RDMENU_TOP or RDMENU_BOTTOM depending on which menu to hide
 * @return RD_OK, or an error code
 */

int32 Display::hideMenu(uint8 menu) {
	// Check for invalid menu parameter
	if (menu > RDMENU_BOTTOM)
		return RDERR_INVALIDMENU;

	// Check that the menu is not currently hidden, or in the process of
	// being hidden.
	if (_menuStatus[menu] == RDMENU_HIDDEN || _menuStatus[menu] == RDMENU_CLOSING)
		return RDERR_INVALIDCOMMAND;

	_menuStatus[menu] = RDMENU_CLOSING;
	return RD_OK;
}

/**
 * This function hides both menus immediately.
 */

void Display::closeMenuImmediately(void) {
	Common::Rect r;
	int i;

	_menuStatus[0] = RDMENU_HIDDEN;
	_menuStatus[1] = RDMENU_HIDDEN;

	for (i = 0; i < RDMENU_MAXPOCKETS; i++) {
		if (_icons[0][i]) {
			clearIconArea(0, i, &r);
			updateRect(&r);
		}
		if (_icons[1][i]) {
			clearIconArea(1, i, &r);
			updateRect(&r);
		}
	}

	memset(_pocketStatus, 0, sizeof(uint8) * 2 * RDMENU_MAXPOCKETS);
}

/**
 * This function sets a menubar icon.
 * @param menu RDMENU_TOP or RDMENU_BOTTOM, depending on which menu to change
 * @param pocket the menu pocket to change
 * @param icon icon data, or NULL to clear the icon
 * @return RD_OK, or an error code
 */

int32 Display::setMenuIcon(uint8 menu, uint8 pocket, uint8 *icon) {
	Common::Rect r;

	// Check for invalid menu parameter.
	if (menu > RDMENU_BOTTOM)
		return RDERR_INVALIDMENU;
	
	// Check for invalid pocket parameter
	if (pocket >= RDMENU_MAXPOCKETS)
		return RDERR_INVALIDPOCKET;

	// If there is an icon in the requested menu/pocket, clear it out.
	if (_icons[menu][pocket]) {
		_iconCount--;
		free(_icons[menu][pocket]);
		_icons[menu][pocket] = NULL;
		clearIconArea(menu, pocket, &r);
		updateRect(&r);
	}

	// Only put the icon in the pocket if it is not NULL
	if (icon != NULL) {
		_iconCount++;
		_icons[menu][pocket] = (uint8 *) malloc(RDMENU_ICONWIDE * RDMENU_ICONDEEP);
		if (_icons[menu][pocket] == NULL)
			return RDERR_OUTOFMEMORY;
		memcpy(_icons[menu][pocket], icon, RDMENU_ICONWIDE * RDMENU_ICONDEEP);
	}

	return RD_OK;
}

} // End of namespace Sword2
