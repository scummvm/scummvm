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

//=============================================================================
//
//	Filename	:	menu.c
//	Created		:	14th November 1996
//	By			:	P.R.Porter
//
//	Summary		:	This module holds the code for the driver96 menu system.
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		15-Nov-96	PRP		Initial Menu functions.
//
//	1.1		20-Nov-96	PRP		Fixed the displaying of the bottom menu.
//
//	1.2		08-Nov-96	PRP		Made the speed of the menubar dependent upon
//								the number of icons displayed.
//
//	1.3		24-Jan-97	PRP		Changed the creation of menu icon sprite 
//								surfaces depending upon whether the hardware
//								can stretch blit or not.  Also, made it so
//								that the full size icon sprite is not stretch
//								blitted.
//
//	1.4		10-Feb-97	PRP		Changed the creation of menu icon sprite
//								surfaces as the capability bits for the drivers
//								have been changed.  Also, changed the error
//								reporting code (for directDraw) so that it
//								works.
//
//	1.5		04-Mar-97	PRP		Tried to fix bug where running out of video 
//								memory creating menubar icon surfaces.
//
//	1.6		16-Apr-97	PRP		Got rid of bug where task switching causes
//								failure of icon draw.
//
//	1.7		23-Jul-97	PRP		Checked error value of stretched blit.
//
//	1.8		13-Aug-97	PRP		Added CloseMenuImmediately.
//
//	1.9		13-Aug-97	PRP		Fixed spelling of above
//
//
//	Functions
//	---------
//
//	--------------------------------------------------------------------------
//
//	int32 ProcessMenu(void)
//
//	This function should be called regularly to process the menuber system.
//	The rate at which this function is called will dictate how smooth the menu
//	system is.  The menu cannot be drawn at a higher rate than the system
//	vbl rate.
//
//	--------------------------------------------------------------------------
//
//	int32 ShowMenu(uint8 menu)
//
//	This function brings the menu in to view.  The choice of top or bottom menu
//	is defined by the parameter menu being either RDMENU_TOP or RDMENU_BOTTOM.
//	An error code is returned if the menu is already shown.
//
//	--------------------------------------------------------------------------
//
//	int32 HideMenu(uint8 menu)
//
//	This function hides the menu defined by the parameter menu.  If the menu is
//	already hidden, an error code is returned.
//
//	--------------------------------------------------------------------------
//
//	int32 SetMenuIcon(uint8 menu, uint8 pocket, uint8 *icon)
//
//	This function sets a menubar icon to that passed in.  If icon is NULL, the
//	pocket is cleared, otherwise, that icon is placed into pocket.  The menu is
//	either RDMENU_TOP or RDMENU_BOTTOM.  Valid error codes include
//	RDERR_INVALIDPOCKET if the pocket number does not exist.  Initially, there
//	are 15 pockets.
//
//	--------------------------------------------------------------------------
//
//	uint8 GetMenuStatus(uint8 menu)
//
//	This function returns the status of the menu passed in menu.  Return values
//	are RDMENU_OPENING, RDMENU_CLOSING, RDMENU_HIDDEN and RDMENU_SHOWN.
//
//=============================================================================


#define WIN32_LEAN_AND_MEAN

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

static uint8 *icons[2][RDMENU_MAXPOCKETS] = {
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }, 
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

static uint8 pocketStatus[2][RDMENU_MAXPOCKETS] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static uint8 iconCount = 0;

int32 ProcessMenu(void) {
	uint8	menu;
	uint8	i;
	uint8	complete;
	uint8	frameCount;
	int32	curx, xoff;
	int32	cury, yoff;
	ScummVM::Rect r;
	int32	delta;
	static	int32 lastTime = 0;

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
					pocketStatus[menu][i] = pocketStatus[menu][i - 1];
					if (pocketStatus[menu][i] != 0)
						complete = 0;
				}
				if (pocketStatus[menu][i] != 0)
					complete = 0;

				// ... and animate the first icon
				if (pocketStatus[menu][0] != 0)
					pocketStatus[menu][0]--;

				// Check to see if the menu is fully open
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
					if (pocketStatus[menu][i] == MAXMENUANIMS) {
						xoff = (RDMENU_ICONWIDE / 2);
						r.left = curx - xoff;
						r.right = r.left + RDMENU_ICONWIDE;
						yoff = (RDMENU_ICONDEEP / 2);
						r.top = cury - yoff;
						r.bottom = r.top + RDMENU_ICONDEEP;
					} else {
						xoff = (RDMENU_ICONWIDE / 2) * pocketStatus[menu][i] / MAXMENUANIMS;
						r.left = curx - xoff;
						r.right = curx + xoff;
						yoff = (RDMENU_ICONDEEP / 2) * pocketStatus[menu][i] / MAXMENUANIMS;
						r.top = cury - yoff;
						r.bottom = cury + yoff;
					}

					if ((xoff != 0) && (yoff != 0)) {
						SquashImage(
							lpBackBuffer->_pixels + r.top * lpBackBuffer->_width + r.left,
							lpBackBuffer->_width,
							r.right - r.left,
							r.bottom - r.top,
							icons[menu][i],
							RDMENU_ICONWIDE,
							RDMENU_ICONWIDE,
							RDMENU_ICONDEEP);
						lpBackBuffer->upload(&r);
					}
				}
				curx += (RDMENU_ICONSPACING + RDMENU_ICONWIDE);
			}
		}
	}
	
/*

	uint8	menu;
	uint8	i;
	uint8	complete;
	uint8	frameCount;
//	uint8	*src, *dst;
	int32	curx, xoff;
	int32	cury, yoff;
	HRESULT	hr;
	RECT	r;
	int32	delta;
	static	int32 lastTime = 0;

	if (lastTime == 0)
	{
		lastTime = timeGetTime();
		frameCount = 1;
	}
	else
	{
		delta = timeGetTime() - lastTime;
		if (delta > 250)
		{
			lastTime += delta;
			delta = 250;
			frameCount = 1;
		}
		else
		{
			frameCount = (uint8) ((iconCount+8) * delta / 750);
			lastTime += frameCount * 750 / (iconCount + 8);
		}

	}



	while (frameCount-- > 0)
	{
		for (menu = RDMENU_TOP; menu <= RDMENU_BOTTOM; menu++)
		{
			if (menuStatus[menu] == RDMENU_OPENING)
			{
				//	The menu is opening, so process it here
				complete = 1;

				//	Propagate the animation from the first icon.
				for (i=RDMENU_MAXPOCKETS-1; i>0; i--)
				{
					pocketStatus[menu][i] = pocketStatus[menu][i-1];
					if (pocketStatus[menu][i] != MAXMENUANIMS)
					{
						complete = 0;
					}
				}
				if (pocketStatus[menu][i] != MAXMENUANIMS)
					complete = 0;

				//	... and animate the first icon
				if (pocketStatus[menu][0] != MAXMENUANIMS)
					pocketStatus[menu][0] += 1;

				//	Check to see if the menu is fully open
				if (complete)
				{
					menuStatus[menu] = RDMENU_SHOWN;
				}
			}
			else if (menuStatus[menu] == RDMENU_CLOSING)
			{
				//	The menu is closing, so process it here
				complete = 1;

				//	Propagate the animation from the first icon.
				for (i=RDMENU_MAXPOCKETS-1; i>0; i--)
				{
					pocketStatus[menu][i] = pocketStatus[menu][i-1];
					if (pocketStatus[menu][i] != 0)
					{
						complete = 0;
					}
				}
				if (pocketStatus[menu][i] != 0)
					complete = 0;

				//	... and animate the first icon
				if (pocketStatus[menu][0] != 0)
					pocketStatus[menu][0] -= 1;

				//	Check to see if the menu is fully open
				if (complete)
				{
					menuStatus[menu] = RDMENU_HIDDEN;
				}
			}
		}

	}
	
	//	Does the menu need to be drawn?
	for (menu = RDMENU_TOP; menu <= RDMENU_BOTTOM; menu++)
	{
		if (menuStatus[menu] != RDMENU_HIDDEN)
		{
			//	Draw the menu here.
			curx = RDMENU_ICONSTART + RDMENU_ICONWIDE / 2;
			cury = (MENUDEEP / 2) + (RENDERDEEP + MENUDEEP) * menu;

			for (i=0; i<RDMENU_MAXPOCKETS; i++)
			{
				if (lpIconSurface[menu][i])
				{
					if (pocketStatus[menu][i] == MAXMENUANIMS)
					{
						xoff = (RDMENU_ICONWIDE / 2);
						r.left = curx - xoff;
						r.right = r.left + RDMENU_ICONWIDE;
						yoff = (RDMENU_ICONDEEP / 2);
						r.top = cury - yoff;
						r.bottom = r.top + RDMENU_ICONDEEP;
					}
					else
					{
						xoff = (RDMENU_ICONWIDE / 2) * pocketStatus[menu][i] / MAXMENUANIMS;
						r.left = curx - xoff;
						r.right = curx + xoff;
						yoff = (RDMENU_ICONDEEP / 2) * pocketStatus[menu][i] / MAXMENUANIMS;
						r.top = cury - yoff;
						r.bottom = cury + yoff;
					}

					if ((xoff != 0) && (yoff != 0))
					{
						hr = IDirectDrawSurface2_Blt(lpBackBuffer, &r, lpIconSurface[menu][i], NULL, DDBLT_WAIT, NULL);
						if (hr != DD_OK)
						{
							switch (hr)
							{
							case DDERR_GENERIC :
								hr = 0;
								break;
							case DDERR_INVALIDCLIPLIST :
								hr = 0;
								break;
							case DDERR_INVALIDOBJECT :
								hr = 0;
								break;
							case DDERR_INVALIDPARAMS :
								hr = 0;
								break;
							case DDERR_INVALIDRECT :
								hr = 0;
								break;
							case DDERR_NOALPHAHW :
								hr = 0;
								break;
							case DDERR_NOBLTHW :
								hr = 0;
								break;
							case DDERR_NOCLIPLIST :
								hr = 0;
								break;
							case DDERR_NODDROPSHW :
								hr = 0;
								break;
							case DDERR_NOMIRRORHW :
								hr = 0;
								break;
							case DDERR_NORASTEROPHW :
								hr = 0;
								break;
							case DDERR_NOROTATIONHW :
								hr = 0;
								break;
							case DDERR_NOSTRETCHHW :
								hr = 0;
								break;
							case DDERR_NOZBUFFERHW :
								hr = 0;
								break;
							case DDERR_SURFACEBUSY :
								hr = 0;
								break;
							case DDERR_SURFACELOST :
								hr = 0;
								break;
							case DDERR_UNSUPPORTED :
								hr = 0;
								break;
							default: //shit
								hr = 0;
								break;
							}
	//							if (hr == DDERR_INVALIDOBJECT)
	//							{
								CreateIconSurface(menu, i);
								LoadIconSurface(menu, i);
								hr = IDirectDrawSurface2_Blt(lpBackBuffer, &r, lpIconSurface[menu][i], NULL, DDBLT_WAIT, NULL);
	//							}
							if (hr != DD_OK)
							{
								if (hr != DDERR_SURFACELOST)
								{
									DirectDrawError("Unable to blt icon", hr);
									return(hr);
								}
							}
						}
					}
				}
				curx += (RDMENU_ICONSPACING + RDMENU_ICONWIDE);
			}
		}
	}
*/
	return RD_OK;
}

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

int32 CloseMenuImmediately(void)
{
	menuStatus[0] = RDMENU_HIDDEN;
	menuStatus[1] = RDMENU_HIDDEN;
	memset(pocketStatus, 0, sizeof(uint8) * 2 * RDMENU_MAXPOCKETS);
	return (RD_OK);
}

int32 SetMenuIcon(uint8 menu, uint8 pocket, uint8 *icon) {
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

uint8 GetMenuStatus(uint8 menu) {
	if (menu > RDMENU_BOTTOM)
		return RDMENU_HIDDEN;
	return menuStatus[menu];
}



