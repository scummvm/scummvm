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
#include "driver96.h"
#include "menu.h"
#include "d_draw.h"
#include "render.h"
#include "common/rect.h"

#define MENUDEEP 40
#define MAXMENUANIMS 8

static uint8 menuStatus[2] = {
	RDMENU_HIDDEN, RDMENU_HIDDEN
};

static byte *icons[2][RDMENU_MAXPOCKETS] = {
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }, 
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

static uint8 pocketStatus[2][RDMENU_MAXPOCKETS] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static uint8 iconCount = 0;

void ClearIconArea(int menu, int pocket, Common::Rect *r) {
	byte *dst;
	int i;

	r->top = menu * (RENDERDEEP + MENUDEEP) + (MENUDEEP - RDMENU_ICONDEEP) / 2;
	r->bottom = r->top + RDMENU_ICONDEEP;
	r->left = RDMENU_ICONSTART + pocket * (RDMENU_ICONWIDE + RDMENU_ICONSPACING);
	r->right = r->left + RDMENU_ICONWIDE;

	dst = lpBackBuffer + r->top * screenWide + r->left;

	for (i = 0; i < RDMENU_ICONDEEP; i++) {
		memset(dst, 0, RDMENU_ICONWIDE);
		dst += screenWide;
	}
}

/**
 * This function should be called regularly to process the menubar system. The
 * rate at which this function is called will dictate how smooth the menu
 * system is.
 */

int32 ProcessMenu(void) {
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
			frameCount = (uint8) ((iconCount + 8) * delta / 750);
			lastTime += frameCount * 750 / (iconCount + 8);
		}
	}

	while (frameCount-- > 0) {
		for (menu = RDMENU_TOP; menu <= RDMENU_BOTTOM; menu++) {
			if (menuStatus[menu] == RDMENU_OPENING) {
				// The menu is opening, so process it here
				complete = 1;

				// Propagate the animation from the first icon.
				for (i = RDMENU_MAXPOCKETS - 1; i > 0; i--) {
					pocketStatus[menu][i] = pocketStatus[menu][i - 1];
					if (pocketStatus[menu][i] != MAXMENUANIMS)
						complete = 0;
				}
				if (pocketStatus[menu][i] != MAXMENUANIMS)
					complete = 0;

				// ... and animate the first icon
				if (pocketStatus[menu][0] != MAXMENUANIMS)
					pocketStatus[menu][0]++;

				// Check to see if the menu is fully open
				if (complete)
					menuStatus[menu] = RDMENU_SHOWN;
			} else if (menuStatus[menu] == RDMENU_CLOSING) {
				// The menu is closing, so process it here
				complete = 1;

				// Propagate the animation from the first icon.
				for (i = RDMENU_MAXPOCKETS - 1; i > 0; i--) {
					if (icons[menu][i] && pocketStatus[menu][i] != 0 && pocketStatus[menu][i - 1] == 0) {
						ClearIconArea(menu, i, &r1);
						UploadRect(&r1);
					}

					pocketStatus[menu][i] = pocketStatus[menu][i - 1];
					if (pocketStatus[menu][i] != 0)
						complete = 0;
				}
				if (pocketStatus[menu][i] != 0)
					complete = 0;

				// ... and animate the first icon
				if (pocketStatus[menu][0] != 0) {
					pocketStatus[menu][0]--;

					if (pocketStatus[menu][0] == 0) {
						ClearIconArea(menu, 0, &r1);
						UploadRect(&r1);
					}
				}

				// Check to see if the menu is fully closed
				if (complete)
					menuStatus[menu] = RDMENU_HIDDEN;
			}
		}
	}
	
	// Does the menu need to be drawn?
	for (menu = RDMENU_TOP; menu <= RDMENU_BOTTOM; menu++) {
		if (menuStatus[menu] != RDMENU_HIDDEN) {
			// Draw the menu here.
			curx = RDMENU_ICONSTART + RDMENU_ICONWIDE / 2;
			cury = (MENUDEEP / 2) + (RENDERDEEP + MENUDEEP) * menu;

			for (i = 0; i < RDMENU_MAXPOCKETS; i++) {
				if (icons[menu][i]) {
					// Since we no longer clear the screen
					// after each frame we need to clear
					// the icon area.

					ClearIconArea(menu, i, &r1);
					
					if (pocketStatus[menu][i] == MAXMENUANIMS) {
						xoff = (RDMENU_ICONWIDE / 2);
						r2.left = curx - xoff;
						r2.right = r2.left + RDMENU_ICONWIDE;
						yoff = (RDMENU_ICONDEEP / 2);
						r2.top = cury - yoff;
						r2.bottom = r2.top + RDMENU_ICONDEEP;
					} else {
						xoff = (RDMENU_ICONWIDE / 2) * pocketStatus[menu][i] / MAXMENUANIMS;
						r2.left = curx - xoff;
						r2.right = curx + xoff;
						yoff = (RDMENU_ICONDEEP / 2) * pocketStatus[menu][i] / MAXMENUANIMS;
						r2.top = cury - yoff;
						r2.bottom = cury + yoff;
					}

					if (xoff != 0 && yoff != 0) {
						dst = lpBackBuffer + r2.top * screenWide + r2.left;
						src = icons[menu][i];

						if (pocketStatus[menu][i] != MAXMENUANIMS) {
							SquashImage(
								dst, screenWide, r2.right - r2.left, r2.bottom - r2.top,
								src, RDMENU_ICONWIDE, RDMENU_ICONWIDE, RDMENU_ICONDEEP, NULL);
						} else {
							for (j = 0; j < RDMENU_ICONDEEP; j++) {
								memcpy(dst, src, RDMENU_ICONWIDE);
								src += RDMENU_ICONWIDE;
								dst += screenWide;
							}
						}
						UploadRect(&r1);
					}
				}
				curx += (RDMENU_ICONSPACING + RDMENU_ICONWIDE);
				r1.left += (RDMENU_ICONSPACING + RDMENU_ICONWIDE);
				r1.right += (RDMENU_ICONSPACING + RDMENU_ICONWIDE);
			}
		}
	}
	
	return RD_OK;
}

/**
 * This function brings a specified menu into view. 
 * @param menu RDMENU_TOP or RDMENU_BOTTOM, depending on which menu to show
 * @return RD_OK, or an error code
 */

int32 ShowMenu(uint8 menu) {
	// Check for invalid menu parameter
	if (menu > RDMENU_BOTTOM)
		return RDERR_INVALIDMENU;

	// Check that the menu is not currently shown, or in the process of
	// being shown.
	if (menuStatus[menu] == RDMENU_SHOWN || menuStatus[menu] == RDMENU_OPENING)
		return RDERR_INVALIDCOMMAND;

	menuStatus[menu] = RDMENU_OPENING;
	return RD_OK;
}

/**
 * This function hides a specified menu.
 * @param menu RDMENU_TOP or RDMENU_BOTTOM depending on which menu to hide
 * @return RD_OK, or an error code
 */

int32 HideMenu(uint8 menu) {
	// Check for invalid menu parameter
	if (menu > RDMENU_BOTTOM)
		return(RDERR_INVALIDMENU);

	// Check that the menu is not currently hidden, or in the process of
	// being hidden.
	if (menuStatus[menu] == RDMENU_HIDDEN || menuStatus[menu] == RDMENU_CLOSING)
		return RDERR_INVALIDCOMMAND;

	menuStatus[menu] = RDMENU_CLOSING;
	return RD_OK;
}

/**
 * This function hides both menus immediately.
 */

int32 CloseMenuImmediately(void) {
	Common::Rect r;
	int i;

	menuStatus[0] = RDMENU_HIDDEN;
	menuStatus[1] = RDMENU_HIDDEN;

	for (i = 0; i < RDMENU_MAXPOCKETS; i++) {
		if (icons[0][i]) {
			ClearIconArea(0, i, &r);
			UploadRect(&r);
		}
		if (icons[1][i]) {
			ClearIconArea(1, i, &r);
			UploadRect(&r);
		}
	}

	memset(pocketStatus, 0, sizeof(uint8) * 2 * RDMENU_MAXPOCKETS);
	return RD_OK;
}

/**
 * This function sets a menubar icon.
 * @param menu RDMENU_TOP or RDMENU_BOTTOM, depending on which menu to change
 * @param pocket the menu pocket to change
 * @param icon icon data, or NULL to clear the icon
 * @return RD_OK, or an error code
 */

int32 SetMenuIcon(uint8 menu, uint8 pocket, uint8 *icon) {
	Common::Rect r;

	debug(5, "stub SetMenuIcon( %d, %d )", menu, pocket);

	// Check for invalid menu parameter.
	if (menu > RDMENU_BOTTOM)
		return RDERR_INVALIDMENU;
	
	// Check for invalid pocket parameter
	if (pocket >= RDMENU_MAXPOCKETS)
		return RDERR_INVALIDPOCKET;

	// If there is an icon in the requested menu/pocket, clear it out.
	if (icons[menu][pocket]) {
		iconCount--;
		free(icons[menu][pocket]);
		icons[menu][pocket] = NULL;
		ClearIconArea(menu, pocket, &r);
		UploadRect(&r);
	}

	// Only put the icon in the pocket if it is not NULL
	if (icon != NULL) {
		iconCount++;
		icons[menu][pocket] = (uint8 *) malloc(RDMENU_ICONWIDE * RDMENU_ICONDEEP);
		if (icons[menu][pocket] == NULL)
			return RDERR_OUTOFMEMORY;
		memcpy(icons[menu][pocket], icon, RDMENU_ICONWIDE * RDMENU_ICONDEEP);
	}
	return RD_OK;
}

/**
 * @return The status of the menu
 */

uint8 GetMenuStatus(uint8 menu) {
	if (menu > RDMENU_BOTTOM)
		return RDMENU_HIDDEN;
	return menuStatus[menu];
}



