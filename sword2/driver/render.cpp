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
#include "sword2/driver/driver96.h"
#include "sword2/driver/d_draw.h"
#include "sword2/driver/render.h"
#include "sword2/driver/menu.h"
#include "sword2/sword2.h"

namespace Sword2 {

#define MILLISECSPERCYCLE	83

#define BLOCKWBITS		6
#define BLOCKHBITS		6

void Graphics::updateRect(Common::Rect *r) {
	g_system->copy_rect(_buffer + r->top * _screenWide + r->left,
		_screenWide, r->left, r->top, r->right - r->left,
		r->bottom - r->top);
}

void Graphics::blitBlockSurface(BlockSurface *s, Common::Rect *r, Common::Rect *clip_rect) {
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

	byte *dst = _buffer + r->top * _screenWide + r->left;
	int i, j;

	if (s->transparent) {
		for (i = 0; i < r->bottom - r->top; i++) {
			for (j = 0; j < r->right - r->left; j++) {
				if (src[j])
					dst[j] = src[j];
			}
			src += BLOCKWIDTH;
			dst += _screenWide;
		}
	} else {
		for (i = 0; i < r->bottom - r->top; i++) {
			memcpy(dst, src, r->right - r->left);
			src += BLOCKWIDTH;
			dst += _screenWide;
		}
	}

	// UploadRect(r);
	setNeedFullRedraw();
}

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

void Graphics::squashImage(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight, byte *backbuf) {
	int32 ince, incne, d;
	int16 x, y;

	// Work out the x-scale

	ince = 2 * dstWidth;
	incne = 2 * (dstWidth - srcWidth);
	d = 2 * dstWidth - srcWidth;
	x = y = 0;
	_xScale[y] = x;

	while (x < srcWidth) {
		if (d <= 0) {
			d += ince;
			x++;
		} else {
			d += incne;
			x++;
			y++;
		}
		_xScale[y] = x;
	}

	// Work out the y-scale

	ince = 2 * dstHeight;
	incne = 2 * (dstHeight - srcHeight);
	d = 2 * dstHeight - srcHeight;
	x = y = 0;
	_yScale[y] = x;

	while (x < srcHeight) {
		if (d <= 0) {
			d += ince;
			x++;
		} else {
			d += incne;
			x++;
			y++;
		}
		_yScale[y] = x;
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

				for (j = _yScale[y]; j < _yScale[y + 1]; j++) {
					for (i = _xScale[x]; i < _xScale[x + 1]; i++) {
						p = src[j * srcPitch + i];
						if (p) {
							red += _palCopy[p][0];
							green += _palCopy[p][1];
							blue += _palCopy[p][2];
							p1 = p;
							spriteCount++;
						} else {
							red += _palCopy[backbuf[x]][0];
							green += _palCopy[backbuf[x]][1];
							blue += _palCopy[backbuf[x]][2];
						}
						count++;
					}
				}
				if (spriteCount == 0)
					dst[x] = 0;
				else if (spriteCount == 1)
					dst[x] = p1;
				else
					dst[x] = quickMatch((uint8) (red / count), (uint8) (green / count), (uint8) (blue / count));
			}
			dst += dstPitch;
			backbuf += _screenWide;
		}
	} else {
		for (y = 0; y < dstHeight; y++) {
			for (x = 0; x < dstWidth; x++) {
				dst[x] = src[_yScale[y] * srcPitch + _xScale[x]];
			}
			dst += dstPitch;
		}
	}
}

void Graphics::stretchImage(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight, byte *backbuf) {
	byte *origDst = dst;
	int32 ince, incne, d;
	int16 x, y, i, j, k;

	// Work out the x-scale

	ince = 2 * srcWidth;
	incne = 2 * (srcWidth - dstWidth);
	d = 2 * srcWidth - dstWidth;
	x = y = 0;
	_xScale[y] = x;

	while (x < dstWidth) {
		if (d <= 0) {
			d += ince;
			x++;
		} else {
			d += incne;
			x++;
			y++;
			_xScale[y] = x;
		}
	}

	// Work out the y-scale

	ince = 2 * srcHeight;
	incne = 2 * (srcHeight - dstHeight);
	d = 2 * srcHeight - dstHeight;
	x = y = 0;
	_yScale[y] = x;
	while (x < dstHeight) {
		if (d <= 0) {
			d += ince;
			x++;
		} else {
			d += incne;
			x++;
			y++;
			_yScale[y] = x;
		}
	}

	// Copy the image

	for (y = 0; y < srcHeight; y++) {
		for (j = _yScale[y]; j < _yScale[y + 1]; j++) {
			k = 0;
			for (x = 0; x < srcWidth; x++) {
				for (i = _xScale[x]; i < _xScale[x + 1]; i++) {
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
					int red = _palCopy[pt[0]][0] << 2;
					int green = _palCopy[pt[0]][1] << 2;
					int blue = _palCopy[pt[0]][2] << 2;
					for (i = 1; i < 5; i++) {
						red += _palCopy[pt[i]][0];
						green += _palCopy[pt[i]][1];
						blue += _palCopy[pt[i]][2];
					}

					*dst++ = quickMatch((uint8) (red >> 3), (uint8) (green >> 3), (uint8) (blue >> 3));
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

/**
 * Plots a point relative to the top left corner of the screen. This is only
 * used for debugging.
 * @param x x-coordinate of the point
 * @param y y-coordinate of the point
 * @param colour colour of the point
 */

void Graphics::plotPoint(uint16 x, uint16 y, uint8 colour) {
	uint8 *buf = _buffer + 40 * RENDERWIDE;
	int16 newx, newy;
	
	newx = x - _scrollX;
	newy = y - _scrollY;

	if (newx >= 0 && newx < RENDERWIDE && newy >= 0 && newy < RENDERDEEP)
		buf[newy * RENDERWIDE + newx] = colour;
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
void Graphics::drawLine(int16 x0, int16 y0, int16 x1, int16 y1, uint8 colour) {
	uint8 *buf = _buffer + 40 * RENDERWIDE;
	int dx, dy;
	int dxmod, dymod;
	int ince, incne;
	int d;
	int x, y;
	int addTo;

	x1 -= _scrollX;
	y1 -= _scrollY;
	x0 -= _scrollX;
	y0 -= _scrollY;

	// Lock the surface if we're rendering to the back buffer.

	//Make sure we're going from left to right

	if (x1 < x0) {
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

	if (dxmod >= dymod) {
		if (dy > 0) {
			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			if (x >= 0 && x < RENDERWIDE && y >= 0 && y < RENDERDEEP)
				buf[y * RENDERWIDE + x] = colour;

			while (x < x1) {
				if (d <= 0) {
					d += ince;
					x++;
				} else {
					d += incne;
					x++;
					y++;
				}
				if (x >= 0 && x < RENDERWIDE && y >= 0 && y < RENDERDEEP)
					buf[y * RENDERWIDE + x] = colour;
			}
		} else {
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
			if (x >= 0 && x < RENDERWIDE && addTo - y >= 0 && addTo - y < RENDERDEEP)
				buf[(addTo - y) * RENDERWIDE + x] = colour;

			while (x < x1) {
				if (d <= 0) {
					d += ince;
					x++;
				} else {
					d += incne;
					x++;
					y++;
				}
				if (x >= 0 && x < RENDERWIDE && addTo - y >= 0 && addTo - y < RENDERDEEP)
					buf[(addTo - y) * RENDERWIDE + x] = colour;
			}
		}
	} else {
		//OK, y is now going to be the single increment.
		//	Ensure the line is going top to bottom
		if (y1 < y0) {
			x = x1;
			x1 = x0;
			x0 = x;
			y = y1;
			y1 = y0;
			y0 = y;
		}
		dx = x1 - x0;
		dy = y1 - y0;

		if (dx > 0) {
			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;
			if (x >= 0 && x < RENDERWIDE && y >= 0 && y < RENDERDEEP)
				buf[y * RENDERWIDE + x] = colour;

			while (y < y1) {
				if (d <= 0) {
					d += ince;
					y++;
				} else {
					d += incne;
					x++;
					y++;
				}
				if (x >= 0 && x < RENDERWIDE && y >= 0 && y < RENDERDEEP)
					buf[y * RENDERWIDE + x] = colour;
			}
		} else {
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
			if (addTo - x >= 0 && addTo - x < RENDERWIDE && y >= 0 && y < RENDERDEEP)
				buf[y * RENDERWIDE + addTo - x] = colour;

			while (y < y1) {
				if (d <= 0) {
					d += ince;
					y++;
				} else {
					d += incne;
					x++;
					y++;
				}
				if (addTo - x >= 0 && addTo - x < RENDERWIDE && y >= 0 && y < RENDERDEEP)
					buf[y * RENDERWIDE + addTo - x] = colour;
			}
		}
	}
}

/**
 * This function tells the driver the size of the background screen for the
 * current location.
 * @param w width of the current location
 * @param h height of the current location
 */

void Graphics::setLocationMetrics(uint16 w, uint16 h) {
	_locationWide = w;
	_locationDeep = h;
}

/**
 * Draws a parallax layer at the current position determined by the scroll. A
 * parallax can be either foreground, background or the main screen.
 */

void Graphics::renderParallax(_parallax *p, int16 l) {
	int16 x, y;
	Common::Rect r;

	if (_locationWide == _screenWide)
		x = 0;
	else
		x = ((int32) ((p->w - _screenWide) * _scrollX) / (int32) (_locationWide - _screenWide));

	if (_locationDeep == _screenDeep - MENUDEEP * 2)
		y = 0;
	else
		y = ((int32) ((p->h - (_screenDeep - MENUDEEP * 2)) * _scrollY) / (int32) (_locationDeep - (_screenDeep - MENUDEEP * 2)));

	Common::Rect clip_rect;

	// Leave enough space for the top and bottom menues

	clip_rect.left = 0;
	clip_rect.right = _screenWide;
	clip_rect.top = MENUDEEP;
	clip_rect.bottom = _screenDeep - MENUDEEP;

	for (int j = 0; j < _yBlocks[l]; j++) {
		for (int i = 0; i < _xBlocks[l]; i++) {
			if (_blockSurfaces[l][i + j * _xBlocks[l]]) {
				r.left = i * BLOCKWIDTH - x;
				r.right = r.left + BLOCKWIDTH;
				r.top = j * BLOCKHEIGHT - y + 40;
				r.bottom = r.top + BLOCKHEIGHT;
				blitBlockSurface(_blockSurfaces[l][i + j * _xBlocks[l]], &r, &clip_rect);
			}
		}
	}

	_parallaxScrollX = _scrollX - x;
	_parallaxScrollY = _scrollY - y;
}

// Uncomment this when benchmarking the drawing routines.
#define LIMIT_FRAME_RATE

/**
 * Initialises the timers before the render loop is entered.
 */

void Graphics::initialiseRenderCycle(void) {
	_initialTime = SVM_timeGetTime();
	_totalTime = _initialTime + MILLISECSPERCYCLE;
}

/**
 * This function should be called when the game engine is ready to start the
 * render cycle.
 */

void Graphics::startRenderCycle(void) {
	_scrollXOld = _scrollX;
	_scrollYOld = _scrollY;

	_startTime = SVM_timeGetTime();

	if (_startTime + _renderAverageTime >= _totalTime)	{
		_scrollX = _scrollXTarget;
		_scrollY = _scrollYTarget;
		_renderTooSlow = true;
	} else {
		_scrollX = (int16) (_scrollXOld + ((_scrollXTarget - _scrollXOld) * (_startTime - _initialTime + _renderAverageTime)) / (_totalTime - _initialTime));
		_scrollY = (int16) (_scrollYOld + ((_scrollYTarget - _scrollYOld) * (_startTime - _initialTime + _renderAverageTime)) / (_totalTime - _initialTime));
		_renderTooSlow = false;
	}

	_framesPerGameCycle = 0;
}

/**
 * This function should be called at the end of the render cycle.
 * @param end the function sets this to true if the render cycle is to be
 * terminated, or false if it should continue
 */

bool Graphics::endRenderCycle(void) {
	static int32 renderTimeLog[4] = { 60, 60, 60, 60 };
	static int32 renderCountIndex = 0;
	int32 time;

	time = SVM_timeGetTime();
	renderTimeLog[renderCountIndex] = time - _startTime;
	_startTime = time;
	_renderAverageTime = (renderTimeLog[0] + renderTimeLog[1] + renderTimeLog[2] + renderTimeLog[3]) >> 2;

	_framesPerGameCycle++;

	if (++renderCountIndex == RENDERAVERAGETOTAL)
		renderCountIndex = 0;

	if (_renderTooSlow) {
		initialiseRenderCycle();
		return true;
	}

	if (_startTime + _renderAverageTime >= _totalTime) {
		_totalTime += MILLISECSPERCYCLE;
		_initialTime = time;
		return true;
	}

#ifdef LIMIT_FRAME_RATE
	if (_scrollXTarget == _scrollX && _scrollYTarget == _scrollY) {
		// If we have already reached the scroll target sleep for the
		// rest of the render cycle.
		g_sword2->sleepUntil(_totalTime);
		_initialTime = SVM_timeGetTime();
		_totalTime += MILLISECSPERCYCLE;
		return true;
	}
#endif

	// This is an attempt to ensure that we always reach the scroll target.
	// Otherwise the game frequently tries to pump out new interpolation
	// frames without ever getting anywhere.

	if (ABS(_scrollX - _scrollXTarget) <= 1 && ABS(_scrollY - _scrollYTarget) <= 1) {
		_scrollX = _scrollXTarget;
		_scrollY = _scrollYTarget;
	} else {
		_scrollX = (int16) (_scrollXOld + ((_scrollXTarget - _scrollXOld) * (_startTime - _initialTime + _renderAverageTime)) / (_totalTime - _initialTime));
		_scrollY = (int16) (_scrollYOld + ((_scrollYTarget - _scrollYOld) * (_startTime - _initialTime + _renderAverageTime)) / (_totalTime - _initialTime));
	}

	return false;
}

/**
 * Sets the scroll target position for the end of the game cycle. The driver
 * will then automatically scroll as many times as it can to reach this
 * position in the allotted time.
 */

void Graphics::setScrollTarget(int16 sx, int16 sy) {
	_scrollXTarget = sx;
	_scrollYTarget = sy;
}

/**
 * This function should be called five times with either the parallax layer
 * or a NULL pointer in order of background parallax to foreground parallax.
 */

int32 Graphics::initialiseBackgroundLayer(_parallax *p) {
	uint8 *memchunk;
	uint8 zeros;
	uint16 count;
	uint16 i, j, k;
	uint16 x;
	uint8 *data;
	uint8 *dst;
	_parallaxLine line;
	uint8 *pLine;

	debug(2, "initialiseBackgroundLayer");

	// This function is called to re-initialise the layers if they have
	// been lost. We know this if the layers have already been assigned.

	if (_layer == MAXLAYERS)
		closeBackgroundLayer();

	if (!p) {
		_layer++;
		return RD_OK;
	}

	_xBlocks[_layer] = (p->w + BLOCKWIDTH - 1) >> BLOCKWBITS;
	_yBlocks[_layer] = (p->h + BLOCKHEIGHT - 1) >> BLOCKHBITS;

	_blockSurfaces[_layer] = (BlockSurface **) calloc(_xBlocks[_layer] * _yBlocks[_layer], sizeof(BlockSurface *));
	if (!_blockSurfaces[_layer])
		return RDERR_OUTOFMEMORY;

	// Decode the parallax layer into a large chunk of memory

	memchunk = (uint8 *) malloc(_xBlocks[_layer] * BLOCKWIDTH * _yBlocks[_layer] * BLOCKHEIGHT);
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

	for (i = 0; i < _xBlocks[_layer] * _yBlocks[_layer]; i++) {
		bool block_has_data = false;
		bool block_is_transparent = false;

		data = memchunk + (p->w * BLOCKHEIGHT * (i / _xBlocks[_layer])) + BLOCKWIDTH * (i % _xBlocks[_layer]);

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
			_blockSurfaces[_layer][i] = (BlockSurface *) malloc(sizeof(BlockSurface));

			//  Copy the data into the surfaces.
			dst = _blockSurfaces[_layer][i]->data;
			for (j = 0; j < BLOCKHEIGHT; j++) {
				memcpy(dst, data, BLOCKWIDTH);
				data += p->w;
				dst += BLOCKWIDTH;
			}

			_blockSurfaces[_layer][i]->transparent = block_is_transparent;

		} else
			_blockSurfaces[_layer][i] = NULL;
	}

	free(memchunk);
	_layer++;

	return RD_OK;
}

/**
 * Should be called once after leaving the room to free up memory.
 */

void Graphics::closeBackgroundLayer(void) {
	debug(2, "CloseBackgroundLayer");

	for (int j = 0; j < MAXLAYERS; j++) {
		if (_blockSurfaces[j]) {
			for (int i = 0; i < _xBlocks[j] * _yBlocks[j]; i++)
				if (_blockSurfaces[j][i])
					free(_blockSurfaces[j][i]);
			free(_blockSurfaces[j]);
			_blockSurfaces[j] = NULL;
		}
	}

	_layer = 0;
}

} // End of namespace Sword2
