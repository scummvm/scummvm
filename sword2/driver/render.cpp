/* Copyright (C) 1994-2004 Revolution Software Ltd
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

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/driver/animation.h"
#include "sword2/driver/d_draw.h"
#include "sword2/driver/menu.h"
#include "sword2/driver/render.h"

namespace Sword2 {

#define MILLISECSPERCYCLE	83

#define BLOCKWBITS		6
#define BLOCKHBITS		6

void Graphics::updateRect(Common::Rect *r) {
	_vm->_system->copyRectToScreen(_buffer + r->top * _screenWide + r->left,
		_screenWide, r->left, r->top, r->right - r->left,
		r->bottom - r->top);
}

void Graphics::blitBlockSurface(BlockSurface *s, Common::Rect *r, Common::Rect *clipRect) {
	if (!r->intersects(*clipRect))
		return;

	byte *src = s->data;

	if (r->top < clipRect->top) {
		src -= BLOCKWIDTH * (r->top - clipRect->top);
		r->top = clipRect->top;
	}
	if (r->left < clipRect->left) {
		src -= (r->left - clipRect->left);
		r->left = clipRect->left;
	}
	if (r->bottom > clipRect->bottom)
		r->bottom = clipRect->bottom;
	if (r->right > clipRect->right)
		r->right = clipRect->right;

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
}

// There are two different separate functions for scaling the image - one fast
// and one good. Or at least that's the theory. I'm sure there are better ways
// to scale an image than this. The latter is used at the highest graphics
// quality setting. Note that the "good" scaler takes an extra parameter, a
// pointer to the area of the screen where the sprite will be drawn.
//
// This code isn't quite like the original DrawSprite(), but should be close
// enough.

void Graphics::scaleImageFast(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight) {
	int x, y;

	for (x = 0; x < dstWidth; x++)
		_xScale[x] = (x * srcWidth) / dstWidth;

	for (y = 0; y < dstHeight; y++)
		_yScale[y] = (y * srcHeight) / dstHeight;

	for (y = 0; y < dstHeight; y++) {
		for (x = 0; x < dstWidth; x++) {
			dst[x] = src[_yScale[y] * srcPitch + _xScale[x]];
		}
		dst += dstPitch;
	}
}

void Graphics::scaleImageGood(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight, byte *backbuf) {
	for (int y = 0; y < dstHeight; y++) {
		for (int x = 0; x < dstWidth; x++) {
			uint8 c1, c2, c3, c4;

			uint32 xPos = (x * srcWidth) / dstWidth;
			uint32 yPos = (y * srcHeight) / dstHeight;
			uint32 xFrac = dstWidth - (x * srcWidth) % dstWidth;
			uint32 yFrac = dstHeight - (y * srcHeight) % dstHeight;

			byte *srcPtr = src + yPos * srcPitch + xPos;
			byte *backPtr = backbuf + y * _screenWide + x;

			bool transparent = true;

			if (*srcPtr) {
				c1 = *srcPtr;
				transparent = false;
			} else
				c1 = *backPtr;

			if (x < dstWidth - 1) {
				if (*(srcPtr + 1)) {
					c2 = *(srcPtr + 1);
					transparent = false;
				} else
					c2 = *(backPtr + 1);
			} else
				c2 = c1;

			if (y < dstHeight - 1) {
				if (*(srcPtr + srcPitch)) {
					c3 = *(srcPtr + srcPitch);
					transparent = false;
				} else
					c3 = *(backPtr + _screenWide);
			} else
				c3 = c1;

			if (x < dstWidth - 1 && y < dstHeight - 1) {
				if (*(srcPtr + srcPitch + 1)) {
					c4 = *(srcPtr + srcPitch + 1);
					transparent = false;
				} else
					c4 = *(backPtr + _screenWide + 1);
			} else
				c4 = c3;

			if (!transparent) {
				uint32 r1 = _palCopy[c1][0];
				uint32 g1 = _palCopy[c1][1];
				uint32 b1 = _palCopy[c1][2];

				uint32 r2 = _palCopy[c2][0];
				uint32 g2 = _palCopy[c2][1];
				uint32 b2 = _palCopy[c2][2];

				uint32 r3 = _palCopy[c3][0];
				uint32 g3 = _palCopy[c3][1];
				uint32 b3 = _palCopy[c3][2];

				uint32 r4 = _palCopy[c4][0];
				uint32 g4 = _palCopy[c4][1];
				uint32 b4 = _palCopy[c4][2];

				uint32 r5 = (r1 * xFrac + r2 * (dstWidth - xFrac)) / dstWidth;
				uint32 g5 = (g1 * xFrac + g2 * (dstWidth - xFrac)) / dstWidth;
				uint32 b5 = (b1 * xFrac + b2 * (dstWidth - xFrac)) / dstWidth;

				uint32 r6 = (r3 * xFrac + r4 * (dstWidth - xFrac)) / dstWidth;
				uint32 g6 = (g3 * xFrac + g4 * (dstWidth - xFrac)) / dstWidth;
				uint32 b6 = (b3 * xFrac + b4 * (dstWidth - xFrac)) / dstWidth;

				uint32 r = (r5 * yFrac + r6 * (dstHeight - yFrac)) / dstHeight;
				uint32 g = (g5 * yFrac + g6 * (dstHeight - yFrac)) / dstHeight;
				uint32 b = (b5 * yFrac + b6 * (dstHeight - yFrac)) / dstHeight;

				dst[y * dstWidth + x] = quickMatch(r, g, b);
			} else
				dst[y * dstWidth + x] = 0;
		}
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
	byte *buf = _buffer + 40 * RENDERWIDE;
	int16 newx, newy;
	
	newx = x - _scrollX;
	newy = y - _scrollY;

	if (newx >= 0 && newx < RENDERWIDE && newy >= 0 && newy < RENDERDEEP) {
		buf[newy * RENDERWIDE + newx] = colour;
		markAsDirty(newx, newy + 40, newx, newy + 40);
	}
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
	byte *buf = _buffer + 40 * RENDERWIDE;
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

	markAsDirty(MIN(x0, x1), MIN(y0, y1) + 40, MAX(x0, x1), MAX(y0, y1) + 40);

	// Make sure we're going from left to right

	if (x1 < x0) {
		SWAP(x0, x1);
		SWAP(y0, y1);
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
		// OK, y is now going to be the single increment.
		//	Ensure the line is going top to bottom
		if (y1 < y0) {
			SWAP(x0, x1);
			SWAP(y0, y1);
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
	setNeedFullRedraw();
}

/**
 * Draws a parallax layer at the current position determined by the scroll. A
 * parallax can be either foreground, background or the main screen.
 */

void Graphics::renderParallax(Parallax *p, int16 l) {
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

	Common::Rect clipRect;

	// Leave enough space for the top and bottom menues

	clipRect.left = 0;
	clipRect.right = _screenWide;
	clipRect.top = MENUDEEP;
	clipRect.bottom = _screenDeep - MENUDEEP;

	for (int j = 0; j < _yBlocks[l]; j++) {
		for (int i = 0; i < _xBlocks[l]; i++) {
			if (_blockSurfaces[l][i + j * _xBlocks[l]]) {
				r.left = i * BLOCKWIDTH - x;
				r.right = r.left + BLOCKWIDTH;
				r.top = j * BLOCKHEIGHT - y + 40;
				r.bottom = r.top + BLOCKHEIGHT;
				blitBlockSurface(_blockSurfaces[l][i + j * _xBlocks[l]], &r, &clipRect);
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
	_initialTime = _vm->_system->get_msecs();
	_totalTime = _initialTime + MILLISECSPERCYCLE;
}

/**
 * This function should be called when the game engine is ready to start the
 * render cycle.
 */

void Graphics::startRenderCycle(void) {
	_scrollXOld = _scrollX;
	_scrollYOld = _scrollY;

	_startTime = _vm->_system->get_msecs();

	if (_startTime + _renderAverageTime >= _totalTime)	{
		_scrollX = _scrollXTarget;
		_scrollY = _scrollYTarget;
		_renderTooSlow = true;
	} else {
		_scrollX = (int16) (_scrollXOld + ((_scrollXTarget - _scrollXOld) * (_startTime - _initialTime + _renderAverageTime)) / (_totalTime - _initialTime));
		_scrollY = (int16) (_scrollYOld + ((_scrollYTarget - _scrollYOld) * (_startTime - _initialTime + _renderAverageTime)) / (_totalTime - _initialTime));
		_renderTooSlow = false;
	}

	if (_scrollXOld != _scrollX || _scrollYOld != _scrollY)
		setNeedFullRedraw();

	_framesPerGameCycle = 0;
}

/**
 * This function should be called at the end of the render cycle.
 * @return true if the render cycle is to be terminated, 
 *         or false if it should continue
 */

bool Graphics::endRenderCycle(void) {
	static int32 renderTimeLog[4] = { 60, 60, 60, 60 };
	static int32 renderCountIndex = 0;
	int32 time;

	time = _vm->_system->get_msecs();
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
		_vm->sleepUntil(_totalTime);
		_initialTime = _vm->_system->get_msecs();
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

	if (_scrollX != _scrollXOld || _scrollY != _scrollYOld)
		setNeedFullRedraw();

#ifdef LIMIT_FRAME_RATE
	// Give the other threads some breathing space. This apparently helps
	// against bug #875683, though I was never able to reproduce it for
	// myself.
	_vm->_system->delay_msecs(10);
#endif

	return false;
}

/**
 * Reset scrolling stuff. This function is called from initBackground()
 */

void Graphics::resetRenderEngine(void) {
	_parallaxScrollX = 0;
	_parallaxScrollY = 0;
	_scrollX = 0;
	_scrollY = 0;
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

int32 Graphics::initialiseBackgroundLayer(Parallax *p) {
	byte *memchunk;
	uint8 zeros;
	uint16 count;
	uint16 i, j, k;
	uint16 x;
	byte *data;
	byte *dst;
	ParallaxLine line;
	byte *pLine;

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

	memchunk = (byte *) calloc(_xBlocks[_layer] * _yBlocks[_layer], BLOCKWIDTH * BLOCKHEIGHT);
	if (!memchunk)
		return RDERR_OUTOFMEMORY;

	for (i = 0; i < p->h; i++) {
		if (p->offset[i] == 0)
			continue;

		pLine = (byte *) p + FROM_LE_32(p->offset[i]);
		line.packets = READ_LE_UINT16(pLine);
		line.offset = READ_LE_UINT16(pLine + 2);
		data = pLine + sizeof(ParallaxLine);
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

		// FIXME: The 'block_is_transparent' flag should only consider
		// data that is inside the parallax layer. Still, it won't do
		// any harm to leave it this way...

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

#ifdef BACKEND_8BIT
void Graphics::plotYUV(byte *lut, int width, int height, byte *const *dat) {
	byte *buf = _buffer + ((480 - height) / 2) * RENDERWIDE + (640 - width) / 2;

	int x, y;

	int ypos = 0;
	int cpos = 0;
	int linepos = 0;

	for (y = 0; y < height; y += 2) {
		for (x = 0; x < width; x += 2) {
			int i = ((((dat[2][cpos] + ROUNDADD) >> SHIFT) * (BITDEPTH+1)) + ((dat[1][cpos] + ROUNDADD)>>SHIFT)) * (BITDEPTH+1);
			cpos++;

			buf[linepos               ] = lut[i + ((dat[0][        ypos  ] + ROUNDADD) >> SHIFT)];
			buf[RENDERWIDE + linepos++] = lut[i + ((dat[0][width + ypos++] + ROUNDADD) >> SHIFT)];
			buf[linepos               ] = lut[i + ((dat[0][        ypos  ] + ROUNDADD) >> SHIFT)];
			buf[RENDERWIDE + linepos++] = lut[i + ((dat[0][width + ypos++] + ROUNDADD) >> SHIFT)];
		}
		linepos += (2 * RENDERWIDE - width);
		ypos += width;
	}
}
#endif


} // End of namespace Sword2
