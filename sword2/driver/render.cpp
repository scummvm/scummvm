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
//	Filename	:	render.c
//	Created		:	26th September 1996
//	By			:	P.R.Porter
//
//	Summary		:	This module holds the functions which deal with rendering
//					the background and parallax layers, and controlling the
//					speed of the scroll (number of frames)
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		26-Sep-96	PRP		The functions have been moved here from d_draw
//								because this is a more sensible place for them.
//
//	1.1		04-Oct-96	PRP		Added direct path to ddraw.h
//
//	1.2		09-Oct-96	PRP		Moved SOFTWARE_SCREEN_BUFFER into driver96.h,
//								and renamed to RD_SOFTWARESCREENBUFFER.
//
//	1.3		23-Oct-96	PRP		Moved definition of _parallaxLine structure to
//								render.h.  Got rid of the divide by zero error.
//
//	1.4		05-Nov-96	PRP		Moved defines of RENDERWIDE, RENDERDEEP, etc.,
//								to render.h
//
//	1.5		15-Nov-96	PRP		Definition of menu size is now obtained from
//								menu.h
//
//	1.6		18-Nov-96	PRP		Changed the direct draw interface to
//								IDirectDraw2.  Added the PlotPoint and 
//								DrawLine functions.
//
//	1,7		21-Nov-96	PRP		Implemented the DrawLine function.
//
//	1.8		24-Jan-97	PRP		Added parallaxScrollx and parallaxScrolly to 
//								define the offset for linking sprites to 
//								parallax layers.
//
//	1.9		06-Feb-97	PRP		Added rendering functions to draw from display
//								memory.  Made the code switchable so that if
//								the hardware is not available, the old software
//								rendering functions are used.
//
//	1.10	06-Feb-97	PRP		Fixed a bug with releasing the surfaces for a
//								room.
//
//	1.11	06-Feb-97	PRP		Implemented PlotPoint and DrawLine for hardware
//								version.
//
//	1.12	07-Feb-97	PRP		Implemented function to restore the background
//								layers.  Changed the layer rendering code to 
//								restore the layers when they've gone.
//
//	1.13	07-Feb-97	PRP		Fixed a bug in the layer renderer, which
//								dropped out when the first block in a layer was
//								blank.
//
//	1.14	10-Feb-97	PRP		Changed the calls to the direct draw error 
//								reporting code, and made the rendering dependant
//								upon the hardware functions availability.
//
//	1.15	12-Feb-97	PRP		Fixed rendering position bug.
//
//	1.16	18-Mar-97	PRP		Fixed the smooth scroll, and render timing.
//								Added a function to initialise the render timer
//
//	1.17	18-Mar-97	PRP		Added code to profile the rendering functions.
//
//	1.18	04-Apr-97	PRP		Restored the primary surface and back buffer
//								before the parallax layers.
//
//	1.19	07-Apr-97	PRP		No changes.
//
//	1.20	08-Apr-97	JEL		Made location wide and location deep global.
//
//	1.21	10-Apr-97	PRP		Added the return value RDERR_LOCKFAILED to 
//								the CopyScreenBuffer function.
//
//	1.22	11-Apr-97	PRP		Added function to clear the software render
//								buffer.
//
//	1.23	11-Apr-97	PRP		Fixed benign redefinition of type bug
//
//	1.24	13-Jun-97	PRP		Fixed switching between hardware and software
//								rendering during gameplay.
//
//	1.25	30-Jun-97	JEL		Tried to fix sc23 sprite position bug, but no changes made in the end
//
//	1.26	08-July-97	JEL		Fixed bug in RenderParallax for when not gotTheFocus, to prevent crash when switching task in NT
//
//	Functions
//	---------
//
//	---------------------------------------------------------------------------
//	
//	int32 RenderParallax(_parallax *p, int16 layer)
//
//	Draws a parallax layer at the current position determined by the scroll.
//	A parallax can be either foreground, background or the main screen.
//
//	---------------------------------------------------------------------------
//
//	int32 CopyScreenBuffer(void)
//
//	Copies the screen buffer to screen memory.  This function should be called
//	when the drawing should be done to the back buffer.  It only does this
//	when we are using a software screen buffer.
//
//	---------------------------------------------------------------------------
//
//	int32 SetScrollTarget(int16 sx, int16 sy)
//
//	Sets the scroll target position for the end of the game cycle.  The drivers
//	will then automatically scroll as many times as it can to reach this 
//	position in the allotted time.
//
//	--------------------------------------------------------------------------
//
//	int32 StartRenderCycle(void)
//
//	This function should be called when the game engine is ready to start
//	the render cycle.
//
//	--------------------------------------------------------------------------
//
//	int32 EndRenderCycle(BOOL *end)
//
//	This function should be called at the end of the render cycle.  If the
//	render cycle is to be terminated, the function sets *end to 1.  Otherwise,
//	the render cycle should continue.
//
//	--------------------------------------------------------------------------
//
//	int32 SetLocationMetrics(uint16 w, uint16 h)
//
//	This function tells the drivers the size of the background screen for the
//	current location.
//
//	--------------------------------------------------------------------------
//
//	int32 PlotPoint(uint16 x, uint16 y, uint8 colour)
//
//	Plots the point x,y in relation to the top left corner of the background.
//
//	--------------------------------------------------------------------------
//
//	int32 DrawLine(int16 x1, int16 y1, int16 x2, int16 y2, uint8 colour)
//
//	Draws a line from the point x1,y1 to x2,y2 of the specified colour.
//
//	--------------------------------------------------------------------------
//
//	int32 InitialiseBackgroundLayer(_parallax *p)
//
//	This function should be called five times with either the parallax layer
//	or a NULL pointer in order of background parallax to foreground parallax.
//
//	--------------------------------------------------------------------------
//
//	int32 CloseBackgroundLayer(void)
//
//	Should be called once after leaving a room to free up video memory.
//
//	--------------------------------------------------------------------------
//
//	int32 PlotDots(int16 x, int16 y, int16 count)
//
//	Plots 'count' dots at the position x,y.
//
//=============================================================================




#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
//#include <windowsx.h>
//#include <mmsystem.h>
#include <stdio.h>

//#include "ddraw.h"

#include "stdafx.h"
#include "driver96.h"
#include "d_draw.h"
#include "rdwin.h"
#include "_mouse.h"
#include "render.h"
#include "menu.h"
#include "../sword2.h"



#define MILLISECSPERCYCLE 83



#if PROFILING == 1
int32 profileCopyScreenBuffer = 0;
int32 profileRenderLayers = 0;
int32 profileSpriteRender = 0;
int32 profileDecompression = 0;
#endif




uint8 myScreenBuffer[RENDERWIDE * RENDERDEEP];





// Scroll variables.  scrollx and scrolly hold the current scroll position, 
//	and scrollxTarget and scrollyTarget are the target position for the end
//	of the game cycle.

//int16		scrollx;
//int16		scrolly;
extern int16		scrollx;
extern int16		scrolly;
int16			parallaxScrollx;
int16			parallaxScrolly;
int16	locationWide;
int16	locationDeep;

static	int16	scrollxTarget;
static	int16	scrollyTarget;
static	int16	scrollxOld;
static	int16	scrollyOld;
static	uint16	layer = 0;



#define RENDERAVERAGETOTAL 4
int32 renderCountIndex = 0;
int32 renderTimeLog[RENDERAVERAGETOTAL] = {60, 60, 60, 60};
int32 initialTime;
int32 startTime;
int32 originTime;
int32 totalTime;
int32 renderAverageTime = 60;
int32 framesPerGameCycle;
int32 renderTooSlow;




#define BLOCKWIDTH 64
#define BLOCKHEIGHT 64
#define BLOCKWBITS 6
#define BLOCKHBITS 6
#define MAXLAYERS 5

uint8 xblocks[MAXLAYERS];
uint8 yblocks[MAXLAYERS];
uint8 restoreLayer[MAXLAYERS];

// Each layer is composed by several sub-blocks

Surface **blockSurfaces[MAXLAYERS] = { 0, 0, 0, 0, 0 };



void Surface::clear() {
	memset(_pixels, 0, _width * _height);
	g_sword2->_system->copy_rect(_pixels, _width, 0, 0, _width, _height);
}

void Surface::blit(Surface *s, ScummVM::Rect *r) {
	ScummVM::Rect clip_rect;

	clip_rect.left = 0;
	clip_rect.top = 0;
	clip_rect.right = 640;
	clip_rect.bottom = 480;

	blit(s, r, &clip_rect);
}

void Surface::blit(Surface *s, ScummVM::Rect *r, ScummVM::Rect *clip_rect) {
	if (r->top > clip_rect->bottom || r->left > clip_rect->right || r->bottom <= clip_rect->top || r->right <= clip_rect->left)
		return;

	byte *src = s->_pixels;

	if (r->top < clip_rect->top) {
		src -= s->_width * (r->top - clip_rect->top);
		r->top = clip_rect->top;
	}
	if (r->left < clip_rect->left) {
		src -= (r->left - clip_rect->left);
		r->left = clip_rect->left;
	}
	if (r->bottom > clip_rect->bottom)
		r->bottom = clip_rect->bottom;
	if (r->right > clip_rect->right)
		r->right = clip_rect->right;

	byte *dst = _pixels + r->top * _width + r->left;
	int i, j;

	// FIXME: We first render the data to the back buffer, and then copy
	// it to the backend. Since the same area will probably be copied
	// several times, as each new parallax layer is rendered, this may be
	// a bit inefficient.

	if (s->_colorKey >= 0) {
		for (i = 0; i < r->bottom - r->top; i++) {
			for (j = 0; j < r->right - r->left; j++) {
				if (src[j] != s->_colorKey)
					dst[j] = src[j];
			}
			src += s->_width;
			dst += _width;
		}
	} else {
		for (i = 0; i < r->bottom - r->top; i++) {
			memcpy(dst, src, r->right - r->left);
			src += s->_width;
			dst += _width;
		}
	}

	upload(r);
}

void Surface::upload(ScummVM::Rect *r) {
	g_sword2->_system->copy_rect(_pixels + r->top * _width + r->left, _width, r->left, r->top, r->right - r->left, r->bottom - r->top);
}

int32 RestoreBackgroundLayer(_parallax *p, int16 l)
{
	int16 oldLayer = layer;
	int16 i;

	debug(0, "RestoreBackgroundLayer %d", l);

	layer = l;
	if (blockSurfaces[l]) {
		for (i = 0; i < xblocks[l] * yblocks[l]; i++)
			if (blockSurfaces[l][i])
				delete blockSurfaces[l][i];

		free(blockSurfaces[l]);
		blockSurfaces[l] = NULL;
	}
	RestoreSurfaces();
	InitialiseBackgroundLayer(p);
	layer = oldLayer;

/*
	int16 oldLayer = layer;
	int16 i;

	layer = l;
	if (blockSurfaces[l])
	{
		for (i=0; i<xblocks[l] * yblocks[l]; i++)
			if (*(blockSurfaces[l]+i))
				IDirectDrawSurface2_Release(*(blockSurfaces[l] + i));
		free(blockSurfaces[l]);
		blockSurfaces[l] = NULL;
	}
	RestoreSurfaces();
	InitialiseBackgroundLayer(p);
	layer = oldLayer;
*/
	return(RD_OK);

}






int32 PlotPoint(uint16 x, uint16 y, uint8 colour)

{
	warning("stub PlotPoint( %d, %d, %d )", x, y, colour);
/*
	int16 newx, newy;
	
	newx = x - scrollx;
	newy = y - scrolly;

	if ((newx < 0) || (newx > RENDERWIDE) || (newy < 0) || (newy > RENDERDEEP))
	{
		return(RD_OK);
	}
	if (renderCaps & RDBLTFX_ALLHARDWARE)
	{
		DDSURFACEDESC ddsd;
		HRESULT hr;

		ddsd.dwSize = sizeof(DDSURFACEDESC);
		hr = IDirectDrawSurface2_Lock(lpBackBuffer, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		if (hr != DD_OK)
		{
			RestoreSurfaces();
			hr = IDirectDrawSurface2_Lock(lpBackBuffer, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		}

		if (hr == DD_OK)
		{

			*((uint8 *) ddsd.lpSurface + (newy + 40) * ddsd.lPitch + newx) = colour;
			IDirectDrawSurface2_Unlock(lpBackBuffer, ddsd.lpSurface);
		}
	}
	else
		myScreenBuffer[newy * RENDERWIDE + newx] = colour;
*/	
	return(RD_OK);

}


// Uses Bressnham's incremental algorithm!
int32 DrawLine(int16 x0, int16 y0, int16 x1, int16 y1, uint8 colour)

{
	warning("stub DrawLine( %d, %d, %d, %d, %d )", x0, y0, x1, y1, colour);
/*
	int dx, dy;
	int dxmod, dymod;
	int ince, incne;
	int	d;
	int x, y;
	int addTo;
	DDSURFACEDESC ddsd;
	HRESULT hr;

	x1 -= scrollx;
	y1 -= scrolly;
	x0 -= scrollx;
	y0 -= scrolly;


	// Lock the surface if we're rendering to the back buffer.

	if (renderCaps & RDBLTFX_ALLHARDWARE)
	{
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		hr = IDirectDrawSurface2_Lock(lpBackBuffer, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		if (hr != DD_OK)
		{
			RestoreSurfaces();
			hr = IDirectDrawSurface2_Lock(lpBackBuffer, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		}
		if (hr != DD_OK)
			return(RD_OK);

		(uint8 *) ddsd.lpSurface += (40 * ddsd.lPitch);

	}




	//Make sure we're going from left to right
	if (x1 < x0)
	{
		x = x1;
		x1 = x0;
		x0 = x;
		y = y1;
		y1 = y0;
		y0 = y;
	}
	dx = x1 - x0;
	dy = y1 - y0;
	
	if (dx < 0)
		dxmod = -dx;
	else
		dxmod = dx;

	if (dy < 0)
		dymod = -dy;
	else
		dymod = dy;

	if (dxmod >= dymod)
	{
		if (dy > 0)
		{
			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			if ((x >= 0) && (x < RENDERWIDE) && (y >= 0) && (y < RENDERDEEP))
				if (renderCaps & RDBLTFX_ALLHARDWARE)
					*((uint8 *) ddsd.lpSurface + y * ddsd.lPitch + x) = colour;
				else
					myScreenBuffer[y * RENDERWIDE + x] = colour;
			while (x < x1)
			{
				if (d <= 0)
				{
					d += ince;
					x += 1;
				}
				else
				{
					d += incne;
					x += 1;
					y += 1;
				}
				if ((x >= 0) && (x < RENDERWIDE) && (y >= 0) && (y < RENDERDEEP))
					if (renderCaps & RDBLTFX_ALLHARDWARE)
						*((uint8 *) ddsd.lpSurface + y * ddsd.lPitch + x) = colour;
					else
						myScreenBuffer[y * RENDERWIDE + x] = colour;
			}
		}
		else
		{
			addTo = y0;
			y0 = 0;
			y1 -= addTo;
			y1 = -y1;
			dy = y1 - y0;

			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			if ((x >= 0) && (x < RENDERWIDE) && (addTo - y >= 0) && (addTo - y < RENDERDEEP))
				if (renderCaps & RDBLTFX_ALLHARDWARE)
					*((uint8 *) ddsd.lpSurface + (addTo - y) * ddsd.lPitch + x) = colour;
				else
					myScreenBuffer[(addTo - y) * RENDERWIDE + x] = colour;
			while (x < x1)
			{
				if (d <= 0)
				{
					d += ince;
					x += 1;
				}
				else
				{
					d += incne;
					x += 1;
					y += 1;
				}
				if ((x >= 0) && (x < RENDERWIDE) && (addTo - y >= 0) && (addTo - y < RENDERDEEP))
					if (renderCaps & RDBLTFX_ALLHARDWARE)
						*((uint8 *) ddsd.lpSurface + (addTo - y) * ddsd.lPitch + x) = colour;
					else
						myScreenBuffer[(addTo - y) * RENDERWIDE + x] = colour;
			}

		}
	}
	else
	{
		//OK, y is now going to be the single increment.
		//	Ensure the line is going top to bottom
		if (y1 < y0)
		{
			x = x1;
			x1 = x0;
			x0 = x;
			y = y1;
			y1 = y0;
			y0 = y;
		}
		dx = x1 - x0;
		dy = y1 - y0;

		if (dx > 0)
		{
			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;
			if ((x >= 0) && (x < RENDERWIDE) && (y >= 0) && (y < RENDERDEEP))
				if (renderCaps & RDBLTFX_ALLHARDWARE)
					*((uint8 *) ddsd.lpSurface + y * ddsd.lPitch + x) = colour;
				else
					myScreenBuffer[y * RENDERWIDE + x] = colour;
			while (y < y1)
			{
				if (d <= 0)
				{
					d += ince;
					y += 1;
				}
				else
				{
					d += incne;
					x += 1;
					y += 1;
				}
				if ((x >= 0) && (x < RENDERWIDE) && (y >= 0) && (y < RENDERDEEP))
					if (renderCaps & RDBLTFX_ALLHARDWARE)
						*((uint8 *) ddsd.lpSurface + y * ddsd.lPitch + x) = colour;
					else
						myScreenBuffer[y * RENDERWIDE + x] = colour;
			}
		}
		else
		{
			addTo = x0;
			x0 = 0;
			x1 -= addTo;
			x1 = -x1;
			dx = x1 - x0;

			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;
			if ((addTo - x >= 0) && (addTo - x < RENDERWIDE) && (y >= 0) && (y < RENDERDEEP))
				if (renderCaps & RDBLTFX_ALLHARDWARE)
					*((uint8 *) ddsd.lpSurface + y * ddsd.lPitch + addTo - x) = colour;
				else
					myScreenBuffer[y * RENDERWIDE + addTo - x] = colour;
			while (y < y1)
			{
				if (d <= 0)
				{
					d += ince;
					y += 1;
				}
				else
				{
					d += incne;
					x += 1;
					y += 1;
				}
				if ((addTo - x >= 0) && (addTo - x < RENDERWIDE) && (y >= 0) && (y < RENDERDEEP))
					if (renderCaps & RDBLTFX_ALLHARDWARE)
						*((uint8 *) ddsd.lpSurface + y * ddsd.lPitch + addTo - x) = colour;
					else
						myScreenBuffer[y * RENDERWIDE + addTo - x] = colour;
			}

		}

	}

	if (renderCaps & RDBLTFX_ALLHARDWARE)
	{
		(uint8 *) ddsd.lpSurface -= (40 * ddsd.lPitch);
		IDirectDrawSurface2_Unlock(lpBackBuffer, ddsd.lpSurface);
	}
*/
	return(RD_OK);

}



int32 SetLocationMetrics(uint16 w, uint16 h)

{

	locationWide = w;
	locationDeep = h;

	return(RD_OK);

}



int32 RenderParallax(_parallax *p, int16 l) {
	int16 x, y;
	int16 i, j;
	ScummVM::Rect r;

	debug(0, "RenderParallax %d", l);

	if (locationWide == screenWide)
		x = 0;
	else
		x = ((int32) ((p->w - screenWide) * scrollx) / (int32) (locationWide - screenWide));

	if (locationDeep == (screenDeep - MENUDEEP * 2))
		y = 0;
	else
		y = ((int32) ((p->h - (screenDeep - MENUDEEP * 2)) * scrolly) / (int32) (locationDeep - (screenDeep - MENUDEEP * 2)));

	ScummVM::Rect clip_rect;

	// Leave enough space for the top and bottom menues

	clip_rect.left = 0;
	clip_rect.right = 640;
	clip_rect.top = 40;
	clip_rect.bottom = 440;

	for (j = 0; j < yblocks[l]; j++) {
		for (i = 0; i < xblocks[l]; i++) {
			if (blockSurfaces[l][i + j * xblocks[l]]) {
				r.left = i * BLOCKWIDTH - x;
				r.right = r.left + BLOCKWIDTH;
				r.top = j * BLOCKHEIGHT - y + 40;
				r.bottom = r.top + BLOCKHEIGHT;
				lpBackBuffer->blit(blockSurfaces[l][i + j * xblocks[l]], &r, &clip_rect);
			}
		}
	}

	parallaxScrollx = scrollx - x;
	parallaxScrolly = scrolly - y;

/*

#if PROFILING == 1

	long int startTime, endTime;

	QueryPerformanceCounter(&startTime);

#endif

	if ((renderCaps & RDBLTFX_ALLHARDWARE) || ((renderCaps & RDBLTFX_FGPARALLAX) && (l > 2)))
	{

		int16 x, y;
		int16 i, j;
		int16 restoreSurfaces = 0;
		HRESULT hr = 0;
		RECT r, rd;

		if (restoreLayer[l])
		{
			RestoreBackgroundLayer(p, l);
			restoreLayer[l] = 0;
		}

		if (locationWide == screenWide)
			x = 0;
		else
			x = ((int32) ((p->w - screenWide) * scrollx) / (int32) (locationWide - screenWide));

		if (locationDeep == (screenDeep - MENUDEEP*2))
			y = 0;
		else
			y = ((int32) ((p->h - (screenDeep - MENUDEEP*2)) * scrolly) / (int32) (locationDeep - (screenDeep - MENUDEEP*2)));


		while (TRUE)
		{
			j = 0;
			while (j < yblocks[l])
			{
				i = 0;
				while (i < xblocks[l])
				{
 					if (*(blockSurfaces[l] + i + j * xblocks[l]))
					{
						r.left = i * BLOCKWIDTH - x;
						r.right = r.left + BLOCKWIDTH;
						r.top = j * BLOCKHEIGHT - y + 40;
						r.bottom = r.top + BLOCKHEIGHT;
						rd.left = 0;
						rd.right = BLOCKWIDTH;
						rd.top = 0;
						rd.bottom = BLOCKHEIGHT;

						if ((r.left < 0) && (r.left > 0 - BLOCKWIDTH))
						{
							rd.left = 0 - r.left;
							r.left = 0;
						}
						if ((r.top < 40) && (r.top > 40 - BLOCKHEIGHT))
						{
							rd.top = 40 - r.top;
							r.top = 40;
						}
						if ((r.right > 640) && (r.right < 640 + BLOCKWIDTH))
						{
							rd.right = BLOCKWIDTH - (r.right - 640);
							r.right = 640;
						}
						if ((r.bottom > 440) && (r.bottom < 440 + BLOCKHEIGHT))
						{
							rd.bottom = BLOCKHEIGHT - (r.bottom - 440);
							r.bottom = 440;
						}
						hr = IDirectDrawSurface2_Blt(lpBackBuffer, &r, *(blockSurfaces[l] + i + j * xblocks[l]), &rd, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
						if (hr == DDERR_INVALIDRECT)
							hr = 0;
						if (hr)
							break;
					}
					i++;
					if (hr)
						break;
				}
				j++;
				if (hr)
					break;
			}
			if (hr)
			{
				if (hr == DDERR_SURFACELOST)
				{
					if (gotTheFocus)
					{
						if (++restoreSurfaces == 4)
							return(RDERR_RESTORELAYERS);
						else
							RestoreBackgroundLayer(p, l);
					}
					else
						return(RD_OK);
				}
				else
					return(hr);

			}
			else
				break;
		}

		parallaxScrollx = scrollx - x;
		parallaxScrolly = scrolly - y;
	}
	else
	{
		uint8			zeros;
		uint16			count;
		uint16			skip;
		uint16			i, j;
		uint16			x;
		int32			px, py;
		uint8			*data;
		uint8			*dst;
		_parallaxLine	*line;

		if (locationWide == screenWide)
			px = 0;
		else
			px = ( (int32) ((p->w - screenWide) * scrollx) / (int32) (locationWide - screenWide));

		if (locationDeep == (screenDeep - MENUDEEP*2))
			py = 0;
		else
			py = ( (int32) ((p->h - (screenDeep - MENUDEEP*2)) * scrolly) / (int32) (locationDeep - (screenDeep - MENUDEEP*2)));


		for (i = py; i < py + (screenDeep - MENUDEEP * 2); i++)
		{
			if (p->offset[i] == 0)
				continue;

			line = (_parallaxLine *) ((uint8 *) p + p->offset[i]);
			data = (uint8 *) line + sizeof(_parallaxLine);
			x = line->offset;
			if (x > px)
				skip = x - px;
			else
				skip = 0;

			dst = myScreenBuffer + (i - py) * RENDERWIDE + skip;

			
			zeros = 0;
			if (line->packets == 0)
			{
				data += px;
				memcpy(dst, data, screenWide);
				continue;
			}

			for (j=0; j<line->packets; j++)
			{
				if (zeros)
				{
					if (x >= px)
					{
						dst += *data;
						x += *data;
					}
					else
					{
						x += *data;
						if (x > px)
						{
							dst += (x - px);
						}
					}
					data += 1;
					zeros = 0;
				}
				else
				{
					if (*data == 0)
					{
						data ++;
					}
					else if (x >= px)
					{
						if (x + *data <= px + screenWide)
						{
							count = *data++;
							memcpy(dst, data, count);
							data += count;
							dst += count;
							x += count;
						}
						else if (x < px + screenWide)
						{
							data++;
							count = screenWide - (x - px);
							memcpy(dst, data, count);
							j = line->packets;
						}
					}
					else
					{
						count = *data++;

						if (x + count > px)
						{
							skip = px - x;
							data += skip;
							count -= skip;
							memcpy(dst, data, count);
							data += count;
							dst += count;
							x += count + skip;
						}
						else
						{
							data += count;
							x += count;
							if (x > px)
							{
								dst += (x - px);
							}
						}
					}
					zeros = 1;
				}
			}
		}

		parallaxScrollx = scrollx - px;
		parallaxScrolly = scrolly - py;
	}

#if PROFILING == 1
	QueryPerformanceCounter(&endTime);
	profileRenderLayers += (endTime.LowPart - startTime.LowPart);
#endif
*/
	return(RD_OK);

}






/*
#define LOGSIZE 10
int32 previousTimeLog[LOGSIZE];
int32 renderCycleStartLog[LOGSIZE];
int32 lastRenderTimeLog[LOGSIZE];
int32 renderCycleEndLog[LOGSIZE];
int32 timeLeftLog[LOGSIZE];
int32 scrollxOldLog[LOGSIZE];
int32 scrollxLog[LOGSIZE];
int32 scrollxTargetLog[LOGSIZE];


void LogMe(int32 in)
{
	static int32 i;

	if (in == 0)
		i = 0;

	previousTimeLog[i] = previousTime;
	aveRenderCycleLog[i] = aveRenderCycle;
	renderCycleStartLog[i] = renderCycleStart;
	lastRenderTimeLog[i] = lastRenderTime;
	renderCycleEndLog[i] = renderCycleEnd;
	timeLeftLog[i] = timeLeft;
	scrollxOldLog[i] = scrollxOld;
	scrollxLog[i] = scrollx;
	scrollxTargetLog[i] = scrollxTarget;
	
	if (++i == LOGSIZE)
		i = 0;

}
*/


int32 InitialiseRenderCycle(void)

{

	initialTime = SVM_timeGetTime();
	originTime = initialTime;
	totalTime = initialTime + MILLISECSPERCYCLE;

	return(RD_OK);

}



int32 StartRenderCycle(void)

{


	scrollxOld = scrollx;
	scrollyOld = scrolly;

	startTime = SVM_timeGetTime();

	if (startTime + renderAverageTime >= totalTime)
	{
		scrollx = scrollxTarget;
		scrolly = scrollyTarget;
		renderTooSlow = 1;
	}
	else
	{
		scrollx = (int16) (scrollxOld + ((scrollxTarget - scrollxOld) * (startTime - initialTime + renderAverageTime)) / (totalTime - initialTime));
		scrolly = (int16) (scrollyOld + ((scrollyTarget - scrollyOld) * (startTime - initialTime + renderAverageTime)) / (totalTime - initialTime));
		renderTooSlow = 0;
	}

	framesPerGameCycle = 0;

	return(RD_OK);

}



int32 EndRenderCycle(BOOL *end)

{

	int32 time;

	time = SVM_timeGetTime();
	renderTimeLog[renderCountIndex] = time - startTime;
	startTime += renderTimeLog[renderCountIndex];
	renderAverageTime = (renderTimeLog[0] + renderTimeLog[1] + renderTimeLog[2] + renderTimeLog[3]) >> 2;

	framesPerGameCycle += 1;

	if (++renderCountIndex == RENDERAVERAGETOTAL)
		renderCountIndex = 0;

	if (renderTooSlow)
	{
		*end = TRUE;
		InitialiseRenderCycle();
	}
	else if (startTime + renderAverageTime >= totalTime)
	{
		*end = TRUE;
		originTime = totalTime;
		totalTime += MILLISECSPERCYCLE;
		initialTime = time;
	}
	else
	{
		*end = FALSE;
		scrollx = (int16) (scrollxOld + ((scrollxTarget - scrollxOld) * (startTime - initialTime + renderAverageTime)) / (totalTime - initialTime));
		scrolly = (int16) (scrollyOld + ((scrollyTarget - scrollyOld) * (startTime - initialTime + renderAverageTime)) / (totalTime - initialTime));
	}

	return(RD_OK);

}


int32 SetScrollTarget(int16 sx, int16 sy)

{

	scrollxTarget = sx;
	scrollyTarget = sy;

	return(RD_OK);

}

int32 CopyScreenBuffer(void)

{
	debug(0, "CopyScreenBuffer");

	// FIXME: The backend should keep track of dirty rects, but I have a
	// feeling each one may be drawn several times, so we may have do add
	// our own handling of them instead.

	g_sword2->_system->update_screen();

/*

	uint8			*dst, *src;
	int16			i;
	DDSURFACEDESC	ddDescription;
	HRESULT			hr;

#if PROFILING == 1
	static long int	endTime;
	long int	startTime;
	int32					lastEndTime, profileTotalTime;
#endif


#if PROFILING == 1
	QueryPerformanceCounter(&startTime);
//	time = timeGetTime();
#endif


	if (!(renderCaps & RDBLTFX_ALLHARDWARE))
	{
		ddDescription.dwSize = sizeof(ddDescription);
		hr = IDirectDrawSurface2_Lock(lpBackBuffer, NULL, &ddDescription, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		if (hr != DD_OK)
		{
			RestoreSurfaces();
			hr = IDirectDrawSurface2_Lock(lpBackBuffer, NULL, &ddDescription, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		}
		if (hr == DD_OK)
		{
			dst = (uint8 *) ddDescription.lpSurface + MENUDEEP * ddDescription.lPitch;
			src = myScreenBuffer;
			for (i=0; i<RENDERDEEP; i++)
			{
				memcpy(dst, src, screenWide);
				src += RENDERWIDE;
				dst += ddDescription.lPitch;
			}
			IDirectDrawSurface2_Unlock(lpBackBuffer, ddDescription.lpSurface);
		}
		else
		{
			return(RDERR_LOCKFAILED);
		}
	}

#if PROFILING == 1
	lastEndTime = endTime.LowPart;
	QueryPerformanceCounter(&endTime);
	profileCopyScreenBuffer = (int32) (endTime.LowPart - startTime.LowPart) / 1000;
	profileTotalTime = (endTime.LowPart - lastEndTime) / 1000;
	if ((profileTotalTime > 0) && (profileTotalTime < 5000))
		PlotDots(0, 5, (int16) profileTotalTime);
	PlotDots(0, 10, (int16) profileCopyScreenBuffer);
	PlotDots(0, 15, (int16) (profileRenderLayers / 1000));
	PlotDots(0, 20, (int16) (profileSpriteRender / 1000));
	PlotDots(0, 25, (int16) (profileDecompression / 1000));
	profileRenderLayers = 0;
	profileSpriteRender = 0;
	profileDecompression = 0;
#endif
*/
	return(RD_OK);
}



int32 InitialiseBackgroundLayer(_parallax *p) {
	uint8 *memchunk;
	uint32 *quaddata;
	uint8 zeros;
	uint16 count;
	uint16 i, j, k;
	uint16 x;
	uint8 *data;
	uint8 *dst;
	_parallaxLine *line;

	debug(0, "InitialiseBackgroundLayer");

	// This function is called to re-initialise the layers if they have
	// been lost. We know this if the layers have already been assigned.

	// TODO: Can layers still be lost, or is that a DirectDraw-ism?

	if (layer == MAXLAYERS)
		CloseBackgroundLayer();

	if (!p) {
		layer++;
		return RD_OK;
	}

	xblocks[layer] = (p->w + BLOCKWIDTH - 1) >> BLOCKWBITS;
	yblocks[layer] = (p->h + BLOCKHEIGHT - 1) >> BLOCKHBITS;

	blockSurfaces[layer] = (Surface **) calloc(xblocks[layer] * yblocks[layer], sizeof(Surface *));
	if (!blockSurfaces[layer])
		return RDERR_OUTOFMEMORY;

	// Decode the parallax layer into a large chunk of memory

	memchunk = (uint8 *) malloc(xblocks[layer] * BLOCKWIDTH * yblocks[layer] * BLOCKHEIGHT);
	if (!memchunk)
		return RDERR_OUTOFMEMORY;

	// We clear not the entire memory chunk, but enough of it to store
	// the entire parallax layer.

	memset(memchunk, 0, p->w * p->h);

	for (i = 0; i < p->h; i++) {
		if (p->offset[i] == 0)
			continue;

		line = (_parallaxLine *) ((uint8 *) p + p->offset[i]);
		data = (uint8 *) line + sizeof(_parallaxLine);
		x = line->offset;

		dst = memchunk + i * p->w + x;

		zeros = 0;
		if (line->packets == 0)	{
			memcpy(dst, data, p->w);
			continue;
		}

		for (j = 0; j < line->packets; j++) {
			if (zeros) {
				dst += *data;
				x += *data;
				data++;
				zeros = 0;
			} else if (*data == 0) {
				data++;
				zeros = 1;
			} else {
				count = *data++;
				memcpy(dst, data, count);
				data += count;
				dst += count;
				x += count;
				zeros = 1;
			}
		}
	}

	// Now create the surfaces!

	for (i = 0; i < xblocks[layer] * yblocks[layer]; i++) {
		bool block_has_data = false;

		data = memchunk + (p->w * BLOCKHEIGHT * (i / xblocks[layer])) + BLOCKWIDTH * (i % xblocks[layer]);

		quaddata = (uint32 *) data;

		for (j = 0; j < BLOCKHEIGHT; j++) {
			for (k = 0; k < BLOCKWIDTH / 4; k++) {
				if (*quaddata) {
					block_has_data = true;
					goto bailout;
				}
				quaddata++;
			}
			quaddata += ((p->w - BLOCKWIDTH) / 4);
		}

bailout:

		//  Only assign a surface to the block if it contains data.

		if (block_has_data) {
			blockSurfaces[layer][i] = new Surface(BLOCKWIDTH, BLOCKHEIGHT);
			blockSurfaces[layer][i]->setColorKey(0);

			//  Copy the data into the surfaces.
			dst = blockSurfaces[layer][i]->_pixels;
			for (j = 0; j < BLOCKHEIGHT; j++) {
				memcpy(dst, data, BLOCKWIDTH);
				data += p->w;
				dst += BLOCKWIDTH;
			}
		} else
			blockSurfaces[layer][i] = NULL;
	}
	free(memchunk);
	layer++;

/*
	uint8			*memchunk;
	uint32			*quaddata;
	uint8			zeros;
	uint16			count;
//	uint16			skip;
	uint16			i, j, k;
	uint16			x;
	uint8			*data;
	uint8			*dst;
	_parallaxLine	*line;
	HRESULT			hr;
	DDSURFACEDESC	ddsd;


	if ((renderCaps & RDBLTFX_ALLHARDWARE) || ((renderCaps & RDBLTFX_FGPARALLAX) && (layer > 2)))
	{

		//  This function is called to re-initialise the layers if they have been lost.
		//	We know this if the layers have already been assigned.
		if (layer == MAXLAYERS)
		{
			CloseBackgroundLayer();
		//	RestoreSurfaces();		// for the primary and back buffer.
		}
		

		if (p == NULL)
		{
			layer += 1;
			return(RD_OK);
		}

		xblocks[layer] = (p->w + BLOCKWIDTH - 1) >> BLOCKWBITS;
		yblocks[layer] = (p->h + BLOCKHEIGHT - 1) >> BLOCKHBITS;
		blockSurfaces[layer] = (LPDIRECTDRAWSURFACE *) malloc(xblocks[layer] * yblocks[layer] * sizeof(LPDIRECTDRAWSURFACE));
		if (blockSurfaces[layer] == NULL)
			return(RDERR_OUTOFMEMORY);

		// Decode the parallax layer into a large chunk of memory
		memchunk = (uint8 *) malloc(xblocks[layer] * BLOCKWIDTH * yblocks[layer] * BLOCKHEIGHT);
		if (memchunk == NULL)
			return(RDERR_OUTOFMEMORY);
		
		memset(memchunk, 0, p->w * p->h);

		for (i = 0; i < p->h; i++)
		{
			if (p->offset[i] == 0)
				continue;

			line = (_parallaxLine *) ((uint8 *) p + p->offset[i]);
			data = (uint8 *) line + sizeof(_parallaxLine);
			x = line->offset;
			
			dst = memchunk + i * p->w + x;

			zeros = 0;
			if (line->packets == 0)
			{
				memcpy(dst, data, p->w);
				continue;
			}

			for (j=0; j<line->packets; j++)
			{
				if (zeros)
				{
					dst += *data;
					x += *data;
					data += 1;
					zeros = 0;
				}
				else
				{
					if (*data == 0)
					{
						data ++;
						zeros = 1;
					}
					else
					{
						count = *data++;
						memcpy(dst, data, count);
						data += count;
						dst += count;
						x += count;
						zeros = 1;
					}
				}
			}
		}

		// Now create the surfaces!
		memset(&ddsd, 0, sizeof(DDSURFACEDESC));
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		if (dxHalCaps & RDCAPS_SRCBLTCKEY)
			ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		else
			ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

		ddsd.dwWidth = BLOCKWIDTH;
		ddsd.dwHeight = BLOCKHEIGHT;
		for (i=0; i<xblocks[layer] * yblocks[layer]; i++)
		{
			//  Only assign a surface to the block if it contains data.
			quaddata = (int32 *) (memchunk + (p->w * BLOCKHEIGHT * (i / xblocks[layer])) + BLOCKWIDTH * (i % xblocks[layer]));
			for (j=0; j<BLOCKHEIGHT; j++)
			{
				for (k=0; k<BLOCKWIDTH / 4; k++)
				{
					if (*quaddata)
						break;
					quaddata++;
				}
				quaddata += ((p->w - BLOCKWIDTH) / 4);
				if (k < BLOCKWIDTH / 4)
					break;
			}
			
			if (k < BLOCKWIDTH / 4)
			{
				hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, blockSurfaces[layer] + i, NULL);
				if (hr != DD_OK)
				{
					if (hr == DDERR_OUTOFVIDEOMEMORY)
					{
						ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
						hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, blockSurfaces[layer] + i, NULL);
					}
					if (hr != DD_OK)
					{
						DirectDrawError("Cannot create parallax surface", hr);
						return(hr);
					}
				}
				//	Set the surface blt source colour key
				hr = IDirectDrawSurface2_SetColorKey(*(blockSurfaces[layer] + i), DDCKEY_SRCBLT, &blackColorKey);

				//  Copy the data into the surfaces.
				memset(&ddsd, 0, sizeof(DDSURFACEDESC));
				ddsd.dwSize = sizeof(DDSURFACEDESC);
				hr = IDirectDrawSurface2_Lock(*(blockSurfaces[layer] + i), NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
				if (hr != DD_OK)
				{
					IDirectDrawSurface2_Restore(*(blockSurfaces[layer] + i));
					hr = IDirectDrawSurface2_Lock(*(blockSurfaces[layer] + i), NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
					if (hr != DD_OK)
					{
						DirectDrawError("Cannot lock parallax surface", hr);
						return(hr);
					}
				}

				data = memchunk + (p->w * BLOCKHEIGHT * (i / xblocks[layer])) + BLOCKWIDTH * (i % xblocks[layer]);
				dst = ddsd.lpSurface;
				for (j=0; j<BLOCKHEIGHT; j++)
				{
					memcpy(dst, data, BLOCKWIDTH);
					data += p->w;
					dst += ddsd.lPitch;
				}
				IDirectDrawSurface2_Unlock(*(blockSurfaces[layer] + i), ddsd.lpSurface);
			}
			else
			{
				*(blockSurfaces[layer] + i) = NULL;
			}
		}
		free(memchunk);
	}
	layer += 1;
*/
	return(RD_OK);

}


int32 CloseBackgroundLayer(void)

{
	debug(0, "CloseBackgroundLayer");

	int16 i, j;

	for (j = 0; j < MAXLAYERS; j++) {
		if (blockSurfaces[j]) {
			for (i = 0; i < xblocks[j] * yblocks[j]; i++)
				if (blockSurfaces[j][i])
					delete blockSurfaces[j][i];
			free(blockSurfaces[j]);
		}
	}

	layer = 0;
/*
	int16			i, j;

//	if (renderCaps & RDBLTFX_ALLHARDWARE)
//	{
		for (j=0; j<MAXLAYERS; j++)
		{
			if (blockSurfaces[j])
			{
				for (i=0; i<xblocks[j] * yblocks[j]; i++)
					if (*(blockSurfaces[j]+i))
						IDirectDrawSurface2_Release(*(blockSurfaces[j] + i));
				free(blockSurfaces[j]);
				blockSurfaces[j] = NULL;
			}
		}
		layer = 0;
//	}
*/
	return(RD_OK);

}


int32 EraseSoftwareScreenBuffer(void)
{
	memset(myScreenBuffer, 0, RENDERWIDE * RENDERDEEP);
	return(RD_OK);
}
