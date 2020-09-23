/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 */

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "graphics/tinygl/zblit.h"
#include "graphics/tinygl/zgl.h"
#include "graphics/pixelbuffer.h"
#include "common/array.h"
#include "graphics/tinygl/zdirtyrect.h"
#include "graphics/tinygl/gl.h"
#include <math.h>

namespace Graphics {

Common::Point transformPoint(float x, float y, int rotation);
Common::Rect rotateRectangle(int x, int y, int width, int height, int rotation, int originX, int originY);

struct BlitImage {
public:
	BlitImage() : _isDisposed(false), _version(0), _binaryTransparent(false), _refcount(1) { }

	void loadData(const Graphics::Surface &surface, uint32 colorKey, bool applyColorKey) {
		const Graphics::PixelFormat textureFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
		int size = surface.w * surface.h;
		_surface.create(surface.w, surface.h, textureFormat);
		Graphics::PixelBuffer buffer(surface.format, (byte *)const_cast<void *>(surface.getPixels()));
		Graphics::PixelBuffer dataBuffer(textureFormat, (byte *)const_cast<void *>(_surface.getPixels()));
		dataBuffer.copyBuffer(0, 0, size, buffer);
		if (applyColorKey) {
			for (int x = 0; x < size; x++) {
				if (buffer.getValueAt(x) == colorKey) {
					// Color keyed pixels become transparent white.
					dataBuffer.setPixelAt(x, 0, 255, 255, 255);
				}
			}
		}

		// Create opaque lines data.
		// A line of pixels can not wrap more that one line of the image, since it would break
		// blitting of bitmaps with a non-zero x position.
		Graphics::PixelBuffer srcBuf = dataBuffer;
		_lines.clear();
		_binaryTransparent = true;
		for (int y = 0; y < surface.h; y++) {
			int start = -1;
			for (int x = 0; x < surface.w; ++x) {
				// We found a transparent pixel, so save a line from 'start' to the pixel before this.
				uint8 r, g, b, a;
				srcBuf.getARGBAt(x, a, r, g, b);
				if (a != 0 && a != 0xFF) {
					_binaryTransparent = false;
				}
				if (a == 0 && start >= 0) {
					_lines.push_back(Line(start, y, x - start, srcBuf.getRawBuffer(start), textureFormat));
					start = -1;
				} else if (a != 0 && start == -1) {
					start = x;
				}
			}
			// end of the bitmap line. if start is an actual pixel save the line.
			if (start >= 0) {
				_lines.push_back(Line(start, y, surface.w - start, srcBuf.getRawBuffer(start), textureFormat));
			}
			srcBuf.shiftBy(surface.w);
		}

		_version++;
	}

	int getVersion() const {
		return _version;
	}

	~BlitImage() {
		_surface.free();
	}

	struct Line {
		int _x;
		int _y;
		int _length;
		byte *_pixels;
		Graphics::PixelBuffer _buf; // This is needed for the conversion.

		Line() : _x(0), _y(0), _length(0), _pixels(nullptr) { }
		Line(int x, int y, int length, byte *pixels, const Graphics::PixelFormat &textureFormat) : _buf(TinyGL::gl_get_context()->fb->cmode, length, DisposeAfterUse::NO),
					_x(x), _y(y), _length(length) {
			// Performing texture to screen conversion.
			Graphics::PixelBuffer srcBuf(textureFormat, pixels);
			_buf.copyBuffer(0, 0, length, srcBuf);
			_pixels = _buf.getRawBuffer();
		}

		Line(const Line& other) : _buf(other._buf.getFormat(), other._length, DisposeAfterUse::NO),
					_x(other._x), _y(other._y), _length(other._length){
			_buf.copyBuffer(0, 0, _length, other._buf);
			_pixels = _buf.getRawBuffer();
		}

		~Line() {
			_buf.free();
		}
	};

	FORCEINLINE bool clipBlitImage(TinyGL::GLContext *c, int &srcX, int &srcY, int &srcWidth, int &srcHeight, int &width, int &height, int &dstX, int &dstY, int &clampWidth, int &clampHeight) {
		if (srcWidth == 0 || srcHeight == 0) {
			srcWidth = _surface.w;
			srcHeight = _surface.h;
		}

		if (width == 0 && height == 0) {
			width = srcWidth;
			height = srcHeight;
		}

		if (dstX >= c->_scissorRect.right || dstY >= c->_scissorRect.bottom)
			return false;

		if (dstX + width < c->_scissorRect.left || dstY + height < c->_scissorRect.top) {
			return false;
		}

		if (dstX < c->_scissorRect.left) {
			srcX += (c->_scissorRect.left - dstX);
			width -= (c->_scissorRect.left - dstX);
			dstX = c->_scissorRect.left;
		}
		
		if (dstY < c->_scissorRect.top) {
			srcY += (c->_scissorRect.top - dstY);
			height -= (c->_scissorRect.top - dstY);
			dstY = c->_scissorRect.top;
		}

		if (width < 0 || height < 0) {
			return false;
		}

		if (dstX + width >= c->_scissorRect.right) {
			clampWidth = c->_scissorRect.right - dstX;
		} else {
			clampWidth = width;
		}

		if (dstY + height >= c->_scissorRect.bottom) {
			clampHeight = c->_scissorRect.bottom - dstY;
		} else {
			clampHeight = height;
		}

		return true;
	}

	// Blits an image to the z buffer.
	// The function only supports clipped blitting without any type of transformation or tinting.
	void tglBlitZBuffer(int dstX, int dstY) {
		TinyGL::GLContext *c = TinyGL::gl_get_context();

		int clampWidth, clampHeight;
		int width = _surface.w, height = _surface.h;
		int srcWidth = 0, srcHeight = 0;
		int srcX = 0, srcY = 0;
		if (clipBlitImage(c, srcX, srcY, srcWidth, srcHeight, width, height, dstX, dstY, clampWidth, clampHeight) == false)
			return;

		Graphics::PixelBuffer srcBuf(_surface.format, (byte *)const_cast<void *>(_surface.getPixels())); // Blit image buffer
		Graphics::PixelBuffer dstBuf(_surface.format, (byte *)c->fb->getZBuffer()); // TinyGL z buffer

		srcBuf.shiftBy(srcY * _surface.w);

		dstBuf.shiftBy(dstY * c->fb->xsize);
		for (int y = 0; y < clampHeight; y++) {
			dstBuf.copyBuffer(dstX, srcX, clampWidth, srcBuf);
			dstBuf.shiftBy(c->fb->xsize);
			srcBuf.shiftBy(_surface.w);
		}
	}

	template <bool kDisableColoring, bool kDisableBlending, bool kEnableAlphaBlending>
	FORCEINLINE void tglBlitRLE(int dstX, int dstY, int srcX, int srcY, int srcWidth, int srcHeight, float aTint, float rTint, float gTint, float bTint);

	template <bool kDisableBlending, bool kDisableColoring, bool kFlipVertical, bool kFlipHorizontal>
	FORCEINLINE void tglBlitSimple(int dstX, int dstY, int srcX, int srcY, int srcWidth, int srcHeight, float aTint, float rTint, float gTint, float bTint);

	template <bool kDisableBlending, bool kDisableColoring, bool kFlipVertical, bool kFlipHorizontal>
	FORCEINLINE void tglBlitScale(int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, float aTint, float rTint, float gTint, float bTint);

	template <bool kDisableBlending, bool kDisableColoring, bool kFlipVertical, bool kFlipHorizontal>
	FORCEINLINE void tglBlitRotoScale(int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, int rotation,
		int originX, int originY, float aTint, float rTint, float gTint, float bTint);

	//Utility function that calls the correct blitting function.
	template <bool kDisableBlending, bool kDisableColoring, bool kDisableTransform, bool kFlipVertical, bool kFlipHorizontal, bool kEnableAlphaBlending>
	FORCEINLINE void tglBlitGeneric(const BlitTransform &transform) {
		if (kDisableTransform) {
			if ((kDisableBlending || kEnableAlphaBlending) && kFlipVertical == false && kFlipHorizontal == false) {
				tglBlitRLE<kDisableColoring, kDisableBlending, kEnableAlphaBlending>(transform._destinationRectangle.left,
					transform._destinationRectangle.top, transform._sourceRectangle.left, transform._sourceRectangle.top, 
					transform._sourceRectangle.width() , transform._sourceRectangle.height(), transform._aTint,
					transform._rTint, transform._gTint, transform._bTint);
			} else {
				tglBlitSimple<kDisableBlending, kDisableColoring, kFlipVertical, kFlipHorizontal>(transform._destinationRectangle.left, 
					transform._destinationRectangle.top, transform._sourceRectangle.left, transform._sourceRectangle.top, 
					transform._sourceRectangle.width() , transform._sourceRectangle.height(),
					transform._aTint, transform._rTint, transform._gTint, transform._bTint);
			}
		} else {
			if (transform._rotation == 0) {
				tglBlitScale<kDisableBlending, kDisableColoring, kFlipVertical, kFlipHorizontal>(transform._destinationRectangle.left,
					transform._destinationRectangle.top, transform._destinationRectangle.width(), transform._destinationRectangle.height(),
					transform._sourceRectangle.left, transform._sourceRectangle.top, transform._sourceRectangle.width(), transform._sourceRectangle.height(),
					transform._aTint, transform._rTint, transform._gTint, transform._bTint);
			} else {
				tglBlitRotoScale<kDisableBlending, kDisableColoring, kFlipVertical, kFlipHorizontal>(transform._destinationRectangle.left,
					transform._destinationRectangle.top, transform._destinationRectangle.width(), transform._destinationRectangle.height(),
					transform._sourceRectangle.left, transform._sourceRectangle.top, transform._sourceRectangle.width(),
					transform._sourceRectangle.height(), transform._rotation, transform._originX, transform._originY, transform._aTint,
					transform._rTint, transform._gTint, transform._bTint);
			}
		}
	}

	int getWidth() const { return _surface.w; }
	int getHeight() const { return _surface.h; }
	void incRefCount() { _refcount++; }
	void dispose() { if (--_refcount == 0) _isDisposed = true; }
	bool isDisposed() const { return _isDisposed; }
private:
	bool _isDisposed;
	bool _binaryTransparent;
	Common::Array<Line> _lines;
	Graphics::Surface _surface;
	int _version;
	int _refcount;
};

void tglGetBlitImageSize(BlitImage *blitImage, int &width, int &height) {
	width = blitImage->getWidth();
	height = blitImage->getHeight();
}

void tglIncBlitImageRef(BlitImage *blitImage) {
	blitImage->incRefCount();
}

int tglGetBlitImageVersion(BlitImage *blitImage) {
	return blitImage->getVersion();
}

BlitImage *tglGenBlitImage() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	BlitImage *image = new BlitImage();
	c->_blitImages.push_back(image);
	return image;
}

void tglUploadBlitImage(BlitImage *blitImage, const Graphics::Surface& surface, uint32 colorKey, bool applyColorKey) {
	if (blitImage != nullptr) {
		blitImage->loadData(surface, colorKey, applyColorKey);
	}
}

void tglDeleteBlitImage(BlitImage *blitImage) {
	if (blitImage != nullptr) {
		blitImage->dispose();
	}
}

// This function uses RLE encoding to skip transparent bitmap parts
// This blit only supports tinting but it will fall back to simpleBlit
// if flipping is required (or anything more complex than that, including rotationd and scaling).
template <bool kDisableColoring, bool kDisableBlending, bool kEnableAlphaBlending>
FORCEINLINE void BlitImage::tglBlitRLE(int dstX, int dstY, int srcX, int srcY, int srcWidth, int srcHeight, float aTint, float rTint, float gTint, float bTint) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	int clampWidth, clampHeight;
	int width = srcWidth, height = srcHeight;
	if (clipBlitImage(c, srcX, srcY, srcWidth, srcHeight, width, height, dstX, dstY, clampWidth, clampHeight) == false)
		return;

	if (aTint <= 0.0f)
		return;

	Graphics::PixelBuffer srcBuf(_surface.format, (byte *)_surface.getPixels());
	srcBuf.shiftBy(srcX + (srcY * _surface.w));

	Graphics::PixelBuffer dstBuf(c->fb->cmode, c->fb->getPixelBuffer());
	dstBuf.shiftBy(dstY * c->fb->xsize + dstX);

	int kBytesPerPixel = c->fb->cmode.bytesPerPixel;

	uint32 lineIndex = 0;
	int maxY = srcY + clampHeight;
	int maxX = srcX + clampWidth;
	while (lineIndex < _lines.size() && _lines[lineIndex]._y < srcY) {
		lineIndex++;
	}

	if (_binaryTransparent || (kDisableBlending || !kEnableAlphaBlending)) { // If bitmap is binary transparent or if  we need complex forms of blending (not just alpha) we need to use writePixel, which is slower 
		while (lineIndex < _lines.size() && _lines[lineIndex]._y < maxY) {
			const BlitImage::Line &l = _lines[lineIndex];
			if (l._x < maxX && l._x + l._length > srcX) {
				int length = l._length;
				int skipStart = (l._x < srcX) ? (srcX - l._x) : 0;
				length -= skipStart;
				int skipEnd   = (l._x + l._length > maxX) ? (l._x + l._length - maxX) : 0;
				length -= skipEnd;
				if (kDisableColoring && (kEnableAlphaBlending == false || kDisableBlending)) {
					memcpy(dstBuf.getRawBuffer((l._y - srcY) * c->fb->xsize + MAX(l._x - srcX, 0)),
						l._pixels + skipStart * kBytesPerPixel, length * kBytesPerPixel);
				} else {
					int xStart = MAX(l._x - srcX, 0);
					if (kDisableColoring) {
						dstBuf.copyBuffer(xStart + (l._y - srcY) * c->fb->xsize, skipStart, length, l._buf);
					} else {
						for(int x = xStart; x < xStart + length; x++) {
							byte aDst, rDst, gDst, bDst;
							srcBuf.getARGBAt((l._y - srcY) * _surface.w + x, aDst, rDst, gDst, bDst);
							c->fb->writePixel((dstX + x) + (dstY + (l._y - srcY)) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
						}
					}

				}
			}
			lineIndex++;
		}
	} else { // Otherwise can use setPixel in some cases which speeds up things quite a bit
		while (lineIndex < _lines.size() && _lines[lineIndex]._y < maxY) {
			const BlitImage::Line &l = _lines[lineIndex];
			if (l._x < maxX && l._x + l._length > srcX) {
				int length = l._length;
				int skipStart = (l._x < srcX) ? (srcX - l._x) : 0;
				length -= skipStart;
				int skipEnd   = (l._x + l._length > maxX) ? (l._x + l._length - maxX) : 0;
				length -= skipEnd;
				if (kDisableColoring && (kEnableAlphaBlending == false || kDisableBlending)) {
					memcpy(dstBuf.getRawBuffer((l._y - srcY) * c->fb->xsize + MAX(l._x - srcX, 0)),
						l._pixels + skipStart * kBytesPerPixel, length * kBytesPerPixel);
				} else {
					int xStart = MAX(l._x - srcX, 0);
					for(int x = xStart; x < xStart + length; x++) {
						byte aDst, rDst, gDst, bDst;
						srcBuf.getARGBAt((l._y - srcY) * _surface.w + x, aDst, rDst, gDst, bDst);
						if (kDisableColoring) {
							if (aDst != 0xFF) {
								c->fb->writePixel((dstX + x) + (dstY + (l._y - srcY)) * c->fb->xsize, aDst, rDst, gDst, bDst);
							} else {
								dstBuf.setPixelAt(x + (l._y - srcY) * c->fb->xsize, aDst, rDst, gDst, bDst);
							}
						} else {
							c->fb->writePixel((dstX + x) + (dstY + (l._y - srcY)) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
						}
					}
				}
			}
			lineIndex++;
		}
	}
}

// This blit function is called when flipping is needed but transformation isn't.
template <bool kDisableBlending, bool kDisableColoring, bool kFlipVertical, bool kFlipHorizontal>
FORCEINLINE void BlitImage::tglBlitSimple(int dstX, int dstY, int srcX, int srcY, int srcWidth, int srcHeight, float aTint, float rTint, float gTint, float bTint) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	int clampWidth, clampHeight;
	int width = srcWidth, height = srcHeight;
	if (clipBlitImage(c, srcX, srcY, srcWidth, srcHeight, width, height, dstX, dstY, clampWidth, clampHeight) == false)
		return;

	Graphics::PixelBuffer srcBuf(_surface.format, (byte *)_surface.getPixels());

	if (kFlipVertical) {
		srcBuf.shiftBy(((srcHeight - srcY - 1) * _surface.w));
	} else {
		srcBuf.shiftBy((srcY * _surface.w));
	}

	Graphics::PixelBuffer dstBuf(c->fb->cmode, c->fb->getPixelBuffer());

	for (int y = 0; y < clampHeight; y++) {
		for (int x = 0; x < clampWidth; ++x) {
			byte aDst, rDst, gDst, bDst;
			if (kFlipHorizontal) {
				srcBuf.getARGBAt(srcX + clampWidth - x, aDst, rDst, gDst, bDst);
			} else {
				srcBuf.getARGBAt(srcX + x, aDst, rDst, gDst, bDst);
			}

			// Those branches are needed to favor speed: avoiding writePixel always yield a huge performance boost when blitting images.
			if (kDisableColoring) { 
				if (kDisableBlending && aDst != 0) {
					dstBuf.setPixelAt((dstX + x) + (dstY + y) * c->fb->xsize, aDst, rDst, gDst, bDst);
				} else {
					c->fb->writePixel((dstX + x) + (dstY + y) * c->fb->xsize, aDst, rDst, gDst, bDst);
				}
			} else {
				if (kDisableBlending && aDst * aTint != 0) {
					dstBuf.setPixelAt((dstX + x) + (dstY + y) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
				} else {
					c->fb->writePixel((dstX + x) + (dstY + y) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
				}
			}
		}
		if (kFlipVertical) {
			srcBuf.shiftBy(-_surface.w);
		} else {
			srcBuf.shiftBy(_surface.w);
		}
	}
}

// This function is called when scale is needed: it uses a simple nearest
// filter to scale the blit image before copying it to the screen.
template <bool kDisableBlending, bool kDisableColoring, bool kFlipVertical, bool kFlipHorizontal>
FORCEINLINE void BlitImage::tglBlitScale(int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight,
					 float aTint, float rTint, float gTint, float bTint) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	int clampWidth, clampHeight;
	if (clipBlitImage(c, srcX, srcY, srcWidth, srcHeight, width, height, dstX, dstY, clampWidth, clampHeight) == false)
		return;

	Graphics::PixelBuffer srcBuf(_surface.format, (byte *)_surface.getPixels());
	srcBuf.shiftBy(srcX + (srcY * _surface.w));

	Graphics::PixelBuffer dstBuf(c->fb->cmode, c->fb->getPixelBuffer());

	for (int y = 0; y < clampHeight; y++) {
		for (int x = 0; x < clampWidth; ++x) {
			byte aDst, rDst, gDst, bDst;
			int xSource, ySource;
			if (kFlipVertical) {
				ySource = clampHeight - y - 1;
			} else {
				ySource = y;
			}

			if (kFlipHorizontal) {
				xSource = clampWidth - x - 1;
			} else {
				xSource = x;
			}

			srcBuf.getARGBAt(((ySource * srcHeight) / height) * _surface.w + ((xSource * srcWidth) / width), aDst, rDst, gDst, bDst);

			if (kDisableColoring) {
				if (kDisableBlending && aDst != 0) {
					dstBuf.setPixelAt((dstX + x) + (dstY + y) * c->fb->xsize, aDst, rDst, gDst, bDst);
				} else {
					c->fb->writePixel((dstX + x) + (dstY + y) * c->fb->xsize, aDst, rDst, gDst, bDst);
				}
			} else {
				if (kDisableBlending && aDst * aTint != 0) {
					dstBuf.setPixelAt((dstX + x) + (dstY + y) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
				} else {
					c->fb->writePixel((dstX + x) + (dstY + y) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
				}
			}
		}
	}
}

/*

The below two functions are adapted from SDL_rotozoom.c,
taken from SDL_gfx-2.0.18.

Its copyright notice:

=============================================================================
SDL_rotozoom.c: rotozoomer, zoomer and shrinker for 32bit or 8bit surfaces

Copyright (C) 2001-2012  Andreas Schiffler

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.

Andreas Schiffler -- aschiffler at ferzkopp dot net
=============================================================================


The functions have been adapted for different structures and coordinate
systems.

*/

template <bool kDisableBlending, bool kDisableColoring, bool kFlipVertical, bool kFlipHorizontal>
FORCEINLINE void BlitImage::tglBlitRotoScale(int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, int rotation,
							 int originX, int originY, float aTint, float rTint, float gTint, float bTint) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	
	int clampWidth, clampHeight;
	if (clipBlitImage(c, srcX, srcY, srcWidth, srcHeight, width, height, dstX, dstY, clampWidth, clampHeight) == false)
		return;
	
	Graphics::PixelBuffer srcBuf(_surface.format, (byte *)_surface.getPixels());
	srcBuf.shiftBy(srcX + (srcY * _surface.w));
	
	Graphics::PixelBuffer dstBuf(c->fb->cmode, c->fb->getPixelBuffer());
	
	// Transform destination rectangle accordingly.
	Common::Rect destinationRectangle = rotateRectangle(dstX, dstY, width, height, rotation, originX, originY);
	
	if (dstX + destinationRectangle.width() > c->fb->xsize)
		clampWidth = c->fb->xsize - dstX;
	else
		clampWidth = destinationRectangle.width();
	if (dstY + destinationRectangle.height() > c->fb->ysize)
		clampHeight = c->fb->ysize - dstY;
	else
		clampHeight = destinationRectangle.height();
	
	uint32 invAngle = 360 - (rotation % 360);
	float invCos = cos(invAngle * (float)M_PI / 180.0f);
	float invSin = sin(invAngle * (float)M_PI / 180.0f);
	
	int icosx = (int)(invCos * (65536.0f * srcWidth / width));
	int isinx = (int)(invSin * (65536.0f * srcWidth / width));
	int icosy = (int)(invCos * (65536.0f * srcHeight / height));
	int isiny = (int)(invSin * (65536.0f * srcHeight / height));
	
	int xd = (srcX + originX) << 16;
	int yd = (srcY + originY) << 16;
	int cx = originX * ((float)width / srcWidth);
	int cy = originY * ((float)height / srcHeight);
	
	int ax = -icosx * cx;
	int ay = -isiny * cx;
	int sw = width - 1;
	int sh = height - 1;
	
	for (int y = 0; y < clampHeight; y++) {
		int t = cy - y;
		int sdx = ax + (isinx * t) + xd;
		int sdy = ay - (icosy * t) + yd;
		for (int x = 0; x < clampWidth; ++x) {
			byte aDst, rDst, gDst, bDst;
			
			int dx = (sdx >> 16);
			int dy = (sdy >> 16);
			
			if (kFlipHorizontal) {
				dx = sw - dx;
			}

			if (kFlipVertical) {
				dy = sh - dy;
			}
			
			if ((dx >= 0) && (dy >= 0) && (dx < srcWidth) && (dy < srcHeight)) {
				srcBuf.getARGBAt(dy * _surface.w + dx, aDst, rDst, gDst, bDst);
				if (kDisableColoring) {
					if (kDisableBlending && aDst != 0) {
						dstBuf.setPixelAt((dstX + x) + (dstY + y) * c->fb->xsize, aDst, rDst, gDst, bDst);
					} else {
						c->fb->writePixel((dstX + x) + (dstY + y) * c->fb->xsize, aDst, rDst, gDst, bDst);
					}
				} else {
					if (kDisableBlending && aDst * aTint != 0) {
						dstBuf.setPixelAt((dstX + x) + (dstY + y) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
					} else {
						c->fb->writePixel((dstX + x) + (dstY + y) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
					}
				}
			}
			sdx += icosx;
			sdy += isiny;
		}
	}
}

void tglBlit(BlitImage *blitImage, int x, int y) {
	BlitTransform transform(x, y);
	TinyGL::tglIssueDrawCall(new BlittingDrawCall(blitImage, transform, BlittingDrawCall::BlitMode_Regular));
}

void tglBlit(BlitImage *blitImage, const BlitTransform &transform) {
	TinyGL::tglIssueDrawCall(new BlittingDrawCall(blitImage, transform, BlittingDrawCall::BlitMode_Regular));
}

void tglBlitNoBlend(BlitImage *blitImage, const BlitTransform &transform) {
	TinyGL::tglIssueDrawCall(new BlittingDrawCall(blitImage, transform, BlittingDrawCall::BlitMode_NoBlend));
}

void tglBlitFast(BlitImage *blitImage, int x, int y) {
	BlitTransform transform(x, y);
	TinyGL::tglIssueDrawCall(new BlittingDrawCall(blitImage, transform, BlittingDrawCall::BlitMode_Fast));
}

void tglBlitZBuffer(BlitImage *blitImage, int x, int y) {
	BlitTransform transform(x, y);
	TinyGL::tglIssueDrawCall(new BlittingDrawCall(blitImage, transform, BlittingDrawCall::BlitMode_ZBuffer));
}


namespace Internal {

template <bool kEnableAlphaBlending, bool kDisableColor, bool kDisableTransform, bool kDisableBlend>
void tglBlit(BlitImage *blitImage, const BlitTransform &transform) {
	if (transform._flipHorizontally) {
		if (transform._flipVertically) {
			blitImage->tglBlitGeneric<kDisableBlend, kDisableColor, kDisableTransform, true, true, kEnableAlphaBlending>(transform);
		} else {
			blitImage->tglBlitGeneric<kDisableBlend, kDisableColor, kDisableTransform, false, true, kEnableAlphaBlending>(transform);
		}
	} else if (transform._flipVertically) {
		blitImage->tglBlitGeneric<kDisableBlend, kDisableColor, kDisableTransform, true, false, kEnableAlphaBlending>(transform);
	} else {
		blitImage->tglBlitGeneric<kDisableBlend, kDisableColor, kDisableTransform, false, false, kEnableAlphaBlending>(transform);
	}
}

template <bool kEnableAlphaBlending, bool kDisableColor, bool kDisableTransform>
void tglBlit(BlitImage *blitImage, const BlitTransform &transform, bool disableBlend) {
	if (disableBlend) {
		tglBlit<kEnableAlphaBlending, kDisableColor, kDisableTransform, true>(blitImage, transform);
	} else {
		tglBlit<kEnableAlphaBlending, kDisableColor, kDisableTransform, false>(blitImage, transform);
	}
}

template <bool kEnableAlphaBlending, bool kDisableColor>
void tglBlit(BlitImage *blitImage, const BlitTransform &transform, bool disableTransform, bool disableBlend) {
	if (disableTransform) {
		tglBlit<kEnableAlphaBlending, kDisableColor, true>(blitImage, transform, disableBlend);
	} else {
		tglBlit<kEnableAlphaBlending, kDisableColor, false>(blitImage, transform, disableBlend);
	}
}

template <bool kEnableAlphaBlending>
void tglBlit(BlitImage *blitImage, const BlitTransform &transform, bool disableColor, bool disableTransform, bool disableBlend) {
	if (disableColor) {
		tglBlit<kEnableAlphaBlending, true>(blitImage, transform, disableTransform, disableBlend);
	} else {
		tglBlit<kEnableAlphaBlending, false>(blitImage, transform, disableTransform, disableBlend);
	}
}

void tglBlit(BlitImage *blitImage, const BlitTransform &transform) {
	TinyGL::GLContext *c =TinyGL::gl_get_context();
	bool disableColor = transform._aTint == 1.0f && transform._bTint == 1.0f && transform._gTint == 1.0f && transform._rTint == 1.0f;
	bool disableTransform = transform._destinationRectangle.width() == 0 && transform._destinationRectangle.height() == 0 && transform._rotation == 0;
	bool disableBlend = c->fb->isBlendingEnabled() == false;
	bool enableAlphaBlending = c->fb->isAlphaBlendingEnabled();

	if (enableAlphaBlending) {
		tglBlit<true>(blitImage, transform, disableColor, disableTransform, disableBlend);
	} else {
		tglBlit<false>(blitImage, transform, disableColor, disableTransform, disableBlend);
	}
}

void tglBlitNoBlend(BlitImage *blitImage, const BlitTransform &transform) {
	if (transform._flipHorizontally == false && transform._flipVertically == false) {
		blitImage->tglBlitGeneric<true, false, false, false, false, false>(transform);
	} else if(transform._flipHorizontally == false) {
		blitImage->tglBlitGeneric<true, false, false, true, false, false>(transform);
	} else {
		blitImage->tglBlitGeneric<true, false, false, false, true, false>(transform);
	}
}

void tglBlitFast(BlitImage *blitImage, int x, int y) {
	BlitTransform transform(x, y);
	blitImage->tglBlitGeneric<true, true, true, false, false, false>(transform);
}

void tglBlitZBuffer(BlitImage *blitImage, int x, int y) {
	blitImage->tglBlitZBuffer(x, y);
}

void tglCleanupImages() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	Common::List<BlitImage *>::iterator it = c->_blitImages.begin();
	while (it != c->_blitImages.end()) {
		if ((*it)->isDisposed()) {
			delete (*it);
			it = c->_blitImages.erase(it);
		} else {
			++it;
		}
	}
}

void tglBlitSetScissorRect(const Common::Rect &rect) {
	TinyGL::gl_get_context()->_scissorRect = rect;
}

void tglBlitResetScissorRect(void) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	c->_scissorRect = c->renderRect;
}

} // end of namespace Internal

Common::Point transformPoint(float x, float y, int rotation) {
	float rotateRad = rotation * (float)M_PI / 180.0f;
	Common::Point newPoint;
	newPoint.x = x * cos(rotateRad) - y * sin(rotateRad);
	newPoint.y = x * sin(rotateRad) + y * cos(rotateRad);
	return newPoint;
}

Common::Rect rotateRectangle(int x, int y, int width, int height, int rotation, int originX, int originY) {
	Common::Point nw, ne, sw, se;
	nw = transformPoint(x - originX, y - originY, rotation);
	ne = transformPoint(x + width - originX, y - originY, rotation);
	sw = transformPoint(x + width - originX, y + height -	 originY, rotation);
	se = transformPoint(x - originX, y + height - originY, rotation);

	float top = MIN(nw.y, MIN(ne.y, MIN(sw.y, se.y)));
	float bottom = MAX(nw.y, MAX(ne.y, MAX(sw.y, se.y)));
	float left = MIN(nw.x, MIN(ne.x, MIN(sw.x, se.x)));
	float right = MAX(nw.x, MAX(ne.x, MAX(sw.x, se.x)));

	Common::Rect res;
	res.top = (int32)(floor(top)) + originY;
	res.bottom = (int32)(ceil(bottom)) + originY;
	res.left = (int32)(floor(left)) + originX;
	res.right = (int32)(ceil(right)) + originX;

	return res;
}

}
