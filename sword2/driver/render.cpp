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
#include "d_draw.h"
#include "rdwin.h"
#include "_mouse.h"
#include "render.h"
#include "menu.h"
#include "../sword2.h"

#define MILLISECSPERCYCLE 83

// Scroll variables.  scrollx and scrolly hold the current scroll position, 
// and scrollxTarget and scrollyTarget are the target position for the end
// of the game cycle.

extern int16 scrollx;
extern int16 scrolly;
int16 parallaxScrollx;
int16 parallaxScrolly;
int16 locationWide;
int16 locationDeep;

static int16 scrollxTarget;
static int16 scrollyTarget;
static int16 scrollxOld;
static int16 scrollyOld;
static uint16 layer = 0;

#define RENDERAVERAGETOTAL 4

int32 renderCountIndex = 0;
int32 renderTimeLog[RENDERAVERAGETOTAL] = { 60, 60, 60, 60 };
int32 initialTime;
int32 startTime;
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

// blockSurfaces stores an array of sub-blocks for each of the parallax layers.

typedef struct {
	byte data[BLOCKWIDTH * BLOCKHEIGHT];
	bool transparent;
} BlockSurface;

BlockSurface **blockSurfaces[MAXLAYERS] = { 0, 0, 0, 0, 0 };

void UploadRect(ScummVM::Rect *r) {
	g_system->copy_rect(lpBackBuffer + r->top * screenWide + r->left,
		screenWide, r->left, r->top, r->right - r->left, r->bottom - r->top);
}

void BlitBlockSurface(BlockSurface *s, ScummVM::Rect *r, ScummVM::Rect *clip_rect) {
	if (r->top > clip_rect->bottom || r->left > clip_rect->right || r->bottom <= clip_rect->top || r->right <= clip_rect->left)
		return;

	byte *src = s->data;

	if (r->top < clip_rect->top) {
		src -= BLOCKWIDTH * (r->top - clip_rect->top);
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

	byte *dst = lpBackBuffer + r->top * screenWide + r->left;
	int i, j;

	if (s->transparent) {
		for (i = 0; i < r->bottom - r->top; i++) {
			for (j = 0; j < r->right - r->left; j++) {
				if (src[j])
					dst[j] = src[j];
			}
			src += BLOCKWIDTH;
			dst += screenWide;
		}
	} else {
		for (i = 0; i < r->bottom - r->top; i++) {
			memcpy(dst, src, r->right - r->left);
			src += BLOCKWIDTH;
			dst += screenWide;
		}
	}

	// UploadRect(r);
	SetNeedRedraw();
}

#define SCALE_MAXWIDTH 512
#define SCALE_MAXHEIGHT 512

static uint16 xScale[SCALE_MAXWIDTH];
static uint16 yScale[SCALE_MAXHEIGHT];

// I've made the scaling two separate functions because there were cases from
// DrawSprite() where it wasn't obvious if the sprite should grow or shrink,
// which caused crashes.
//
// Keeping them separate might be a good idea anyway, for readability.
//
// The code is based on the original DrawSprite() code, so apart from not
// knowing if I got it right, I don't know how good the original really is.
//
// The backbuf parameter points to the buffer where the image will eventually
// be drawn. This is only used at the highest graphics detail setting (and not
// always even then) and is used to help anti-alias the image.

void SquashImage(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight, byte *backbuf) {
	int32 ince, incne, d;
	int16 x, y;

	// Work out the x-scale

	ince = 2 * dstWidth;
	incne = 2 * (dstWidth - srcWidth);
	d = 2 * dstWidth - srcWidth;
	x = y = 0;
	xScale[y] = x;

	while (x < srcWidth) {
		if (d <= 0) {
			d += ince;
			x++;
		} else {
			d += incne;
			x++;
			y++;
		}
		xScale[y] = x;
	}

	// Work out the y-scale

	ince = 2 * dstHeight;
	incne = 2 * (dstHeight - srcHeight);
	d = 2 * dstHeight - srcHeight;
	x = y = 0;
	yScale[y] = x;

	while (x < srcHeight) {
		if (d <= 0) {
			d += ince;
			x++;
		} else {
			d += incne;
			x++;
			y++;
		}
		yScale[y] = x;
	}

	// Copy the image (with or without anti-aliasing)

	if (backbuf) {
		for (y = 0; y < dstHeight; y++) {
			for (x = 0; x < dstWidth; x++) {
				uint8 p;
				uint8 p1 = 0;
				int count = 0;
				int spriteCount = 0;
				int red = 0;
				int green = 0;
				int blue = 0;
				int i, j;

				for (j = yScale[y]; j < yScale[y + 1]; j++) {
					for (i = xScale[x]; i < xScale[x + 1]; i++) {
						p = src[j * srcPitch + i];
						if (p) {
							red += palCopy[p][0];
							green += palCopy[p][1];
							blue += palCopy[p][2];
							p1 = p;
							spriteCount++;
						} else {
							red += palCopy[backbuf[x]][0];
							green += palCopy[backbuf[x]][1];
							blue += palCopy[backbuf[x]][2];
						}
						count++;
					}
				}
				if (spriteCount == 0)
					dst[x] = 0;
				else if (spriteCount == 1)
					dst[x] = p1;
				else
					dst[x] = QuickMatch((uint8) (red / count), (uint8) (green / count), (uint8) (blue / count));
			}
			dst += dstPitch;
			backbuf += screenWide;
		}
	} else {
		for (y = 0; y < dstHeight; y++) {
			for (x = 0; x < dstWidth; x++) {
				dst[x] = src[yScale[y] * srcPitch + xScale[x]];
			}
			dst += dstPitch;
		}
	}
}

void StretchImage(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight, byte *backbuf) {
	byte *origDst = dst;
	int32 ince, incne, d;
	int16 x, y, i, j, k;

	// Work out the x-scale

	ince = 2 * srcWidth;
	incne = 2 * (srcWidth - dstWidth);
	d = 2 * srcWidth - dstWidth;
	x = y = 0;
	xScale[y] = x;

	while (x < dstWidth) {
		if (d <= 0) {
			d += ince;
			x++;
		} else {
			d += incne;
			x++;
			y++;
			xScale[y] = x;
		}
	}

	// Work out the y-scale

	ince = 2 * srcHeight;
	incne = 2 * (srcHeight - dstHeight);
	d = 2 * srcHeight - dstHeight;
	x = y = 0;
	yScale[y] = x;
	while (x < dstHeight) {
		if (d <= 0) {
			d += ince;
			x++;
		} else {
			d += incne;
			x++;
			y++;
			yScale[y] = x;
		}
	}

	// Copy the image

	for (y = 0; y < srcHeight; y++) {
		for (j = yScale[y]; j < yScale[y + 1]; j++) {
			k = 0;
			for (x = 0; x < srcWidth; x++) {
				for (i = xScale[x]; i < xScale[x + 1]; i++) {
					dst[k++] = src[y * srcPitch + x];
				}
			}
			dst += dstPitch;
		}
	}

	// Anti-aliasing

	if (backbuf) {
		byte *newDst = (byte *) malloc(dstWidth * dstHeight);
		if (!newDst)
			return;

		memcpy(newDst, origDst, dstWidth);

		for (y = 1; y < dstHeight - 1; y++) {
			src = origDst + y * dstPitch;
			dst = newDst + y * dstWidth;
			*dst++ = *src++;
			for (x = 1; x < dstWidth - 1; x++) {
				byte pt[5];
				byte *p = backbuf + y * 640 + x;
				int count = 0;

				if (*src) {
					count++;
					pt[0] = *src;
				} else
					pt[0] = *p;

				pt[1] = *(src - dstPitch);
				if (pt[1] == 0)
					pt[1] = *(p - 640);
				else
					count++;

				pt[2] = *(src - 1);
				if (pt[2] == 0)
					pt[2] = *(p - 1);
				else
					count++;

				pt[3] = *(src + 1);
				if (pt[3] == 0)
					pt[3] = *(p + 1);
				else
					count++;

				pt[4] = *(src + dstPitch);
				if (pt[4] == 0)
					pt[4] = *(p + 640);
				else
					count++;

				if (count) {
					int red = palCopy[pt[0]][0] << 2;
					int green = palCopy[pt[0]][1] << 2;
					int blue = palCopy[pt[0]][2] << 2;
					for (i = 1; i < 5; i++) {
						red += palCopy[pt[i]][0];
						green += palCopy[pt[i]][1];
						blue += palCopy[pt[i]][2];
					}

					*dst++ = QuickMatch((uint8) (red >> 3), (uint8) (green >> 3), (uint8) (blue >> 3));
				} else
					*dst++ = 0;
				src++;
			}
			*dst++ = *src++;
		}
		memcpy(dst, src, dstWidth);

		src = newDst;
		dst = origDst;

		for (i = 0; i < dstHeight; i++) {
			memcpy(dst, src, dstWidth);
			dst += dstPitch;
			src += dstWidth;
		}

		free(newDst);
	}
}

int32 RestoreBackgroundLayer(_parallax *p, int16 l)
{
	int16 oldLayer = layer;

	debug(2, "RestoreBackgroundLayer %d", l);

	layer = l;
	if (blockSurfaces[l]) {
		for (int i = 0; i < xblocks[l] * yblocks[l]; i++) {
			if (blockSurfaces[l][i])
				free(blockSurfaces[l][i]);
		}

		free(blockSurfaces[l]);
		blockSurfaces[l] = NULL;
	}
	InitialiseBackgroundLayer(p);
	layer = oldLayer;
	return RD_OK;
}

/**
 * Plots a point relative to the top left corner of the screen. This is only
 * used for debugging.
 * @param x x-coordinate of the point
 * @param y y-coordinate of the point
 * @param colour colour of the point
 */

int32 PlotPoint(uint16 x, uint16 y, uint8 colour) {
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

/**
 * Draws a line from one point to another. This is only used for debugging.
 * @param x0 x-coordinate of the start point
 * @param y0 y-coordinate of the start point
 * @param x1 x-coordinate of the end point
 * @param y1 y-coordinate of the end point
 * @param colour colour of the line
 */

// Uses Bressnham's incremental algorithm!
int32 DrawLine(int16 x0, int16 y0, int16 x1, int16 y1, uint8 colour) {
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
	return RD_OK;
}

/**
 * This function tells the driver the size of the background screen for the
 * current location.
 * @param w width of the current location
 * @param h height of the current location
 */

int32 SetLocationMetrics(uint16 w, uint16 h) {
	locationWide = w;
	locationDeep = h;

	return RD_OK;
}

/**
 * Draws a parallax layer at the current position determined by the scroll. A
 * parallax can be either foreground, background or the main screen.
 */

int32 RenderParallax(_parallax *p, int16 l) {
	int16 x, y;
	ScummVM::Rect r;

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
	clip_rect.right = screenWide;
	clip_rect.top = MENUDEEP;
	clip_rect.bottom = screenDeep - MENUDEEP;

	for (int j = 0; j < yblocks[l]; j++) {
		for (int i = 0; i < xblocks[l]; i++) {
			if (blockSurfaces[l][i + j * xblocks[l]]) {
				r.left = i * BLOCKWIDTH - x;
				r.right = r.left + BLOCKWIDTH;
				r.top = j * BLOCKHEIGHT - y + 40;
				r.bottom = r.top + BLOCKHEIGHT;
				BlitBlockSurface(blockSurfaces[l][i + j * xblocks[l]], &r, &clip_rect);
			}
		}
	}

	parallaxScrollx = scrollx - x;
	parallaxScrolly = scrolly - y;

	return RD_OK;
}

// Uncomment this when benchmarking the drawing routines.
#define LIMIT_FRAME_RATE

/**
 * Initialises the timers before the render loop is entered.
 */

int32 InitialiseRenderCycle(void) {
	initialTime = SVM_timeGetTime();
	totalTime = initialTime + MILLISECSPERCYCLE;
	return RD_OK;
}

/**
 * This function should be called when the game engine is ready to start the
 * render cycle.
 */

int32 StartRenderCycle(void) {
	scrollxOld = scrollx;
	scrollyOld = scrolly;

	startTime = SVM_timeGetTime();

	if (startTime + renderAverageTime >= totalTime)	{
		scrollx = scrollxTarget;
		scrolly = scrollyTarget;
		renderTooSlow = 1;
	} else {
		scrollx = (int16) (scrollxOld + ((scrollxTarget - scrollxOld) * (startTime - initialTime + renderAverageTime)) / (totalTime - initialTime));
		scrolly = (int16) (scrollyOld + ((scrollyTarget - scrollyOld) * (startTime - initialTime + renderAverageTime)) / (totalTime - initialTime));
		renderTooSlow = 0;
	}

	framesPerGameCycle = 0;
	return RD_OK;
}

// FIXME: Move this to some better place?

void sleepUntil(int32 time) {
	while ((int32) SVM_timeGetTime() < time) {
		g_sword2->parseEvents();

		// Make sure menu animations and fades don't suffer
		ProcessMenu();
		ServiceWindows();

		g_system->delay_msecs(10);
	}
}

/**
 * This function should be called at the end of the render cycle.
 * @param end the function sets this to true if the render cycle is to be
 * terminated, or false if it should continue
 */

int32 EndRenderCycle(bool *end) {
	int32 time;

	time = SVM_timeGetTime();
	renderTimeLog[renderCountIndex] = time - startTime;
	startTime = time;
	renderAverageTime = (renderTimeLog[0] + renderTimeLog[1] + renderTimeLog[2] + renderTimeLog[3]) >> 2;

	framesPerGameCycle += 1;

	if (++renderCountIndex == RENDERAVERAGETOTAL)
		renderCountIndex = 0;

	if (renderTooSlow) {
		*end = true;
		InitialiseRenderCycle();
	} else if (startTime + renderAverageTime >= totalTime) {
		*end = true;
		totalTime += MILLISECSPERCYCLE;
		initialTime = time;
#ifdef LIMIT_FRAME_RATE
	} else if (scrollxTarget == scrollx && scrollyTarget == scrolly) {
		// If we have already reached the scroll target sleep for the
		// rest of the render cycle.
		*end = true;
		sleepUntil(totalTime);
		initialTime = SVM_timeGetTime();
		totalTime += MILLISECSPERCYCLE;
#endif
	} else {
		*end = false;

		// This is an attempt to ensure that we always reach the scroll
		// target. Otherwise the game frequently tries to pump out new
		// interpolation frames without ever getting anywhere.

		if (ABS(scrollx - scrollxTarget) <= 1 && ABS(scrolly - scrollyTarget) <= 1) {
			scrollx = scrollxTarget;
			scrolly = scrollyTarget;
		} else {
			scrollx = (int16) (scrollxOld + ((scrollxTarget - scrollxOld) * (startTime - initialTime + renderAverageTime)) / (totalTime - initialTime));
			scrolly = (int16) (scrollyOld + ((scrollyTarget - scrollyOld) * (startTime - initialTime + renderAverageTime)) / (totalTime - initialTime));
		}
	}

	return RD_OK;
}

/**
 * Sets the scroll target position for the end of the game cycle. The driver
 * will then automatically scroll as many times as it can to reach this
 * position in the allotted time.
 */

int32 SetScrollTarget(int16 sx, int16 sy) {
	scrollxTarget = sx;
	scrollyTarget = sy;

	return RD_OK;
}

/**
 * This function should be called five times with either the parallax layer
 * or a NULL pointer in order of background parallax to foreground parallax.
 */

int32 InitialiseBackgroundLayer(_parallax *p) {
	uint8 *memchunk;
	uint8 zeros;
	uint16 count;
	uint16 i, j, k;
	uint16 x;
	uint8 *data;
	uint8 *dst;
	_parallaxLine line;
	uint8 *pLine;

	debug(2, "InitialiseBackgroundLayer");

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

	blockSurfaces[layer] = (BlockSurface **) calloc(xblocks[layer] * yblocks[layer], sizeof(BlockSurface *));
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

		pLine = (uint8 *) p + FROM_LE_32(p->offset[i]);
		line.packets = READ_LE_UINT16(pLine);
		line.offset = READ_LE_UINT16(pLine + 2);
		data = pLine + sizeof(_parallaxLine);
		x = line.offset;

		dst = memchunk + i * p->w + x;

		zeros = 0;
		if (line.packets == 0)	{
			memcpy(dst, data, p->w);
			continue;
		}

		for (j = 0; j < line.packets; j++) {
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
		bool block_is_transparent = false;

		data = memchunk + (p->w * BLOCKHEIGHT * (i / xblocks[layer])) + BLOCKWIDTH * (i % xblocks[layer]);

		for (j = 0; j < BLOCKHEIGHT; j++) {
			for (k = 0; k < BLOCKWIDTH; k++) {
				if (data[j * p->w + k])
					block_has_data = true;
				else
					block_is_transparent = true;
			}
		}

		//  Only assign a surface to the block if it contains data.

		if (block_has_data) {
			blockSurfaces[layer][i] = (BlockSurface *) malloc(sizeof(BlockSurface));

			//  Copy the data into the surfaces.
			dst = blockSurfaces[layer][i]->data;
			for (j = 0; j < BLOCKHEIGHT; j++) {
				memcpy(dst, data, BLOCKWIDTH);
				data += p->w;
				dst += BLOCKWIDTH;
			}

			blockSurfaces[layer][i]->transparent = block_is_transparent;

		} else
			blockSurfaces[layer][i] = NULL;
	}

	free(memchunk);
	layer++;

	return RD_OK;
}

/**
 * Should be called once after leaving the room to free up memory.
 */

int32 CloseBackgroundLayer(void) {
	debug(2, "CloseBackgroundLayer");

	for (int j = 0; j < MAXLAYERS; j++) {
		if (blockSurfaces[j]) {
			for (int i = 0; i < xblocks[j] * yblocks[j]; i++)
				if (blockSurfaces[j][i])
					free(blockSurfaces[j][i]);
			free(blockSurfaces[j]);
			blockSurfaces[j] = NULL;
		}
	}

	layer = 0;
	return RD_OK;
}
