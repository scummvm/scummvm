/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This code is based on Nine Patch code by Matthew Leverton
   taken from https://github.com/konforce/Allegro-Nine-Patch

   You may read more about the image format here:
      https://radleymarx.com/2011/simple-guide-to-9-patch

   Copyright (C) 2011 Matthew Leverton

   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
 */


#include "common/array.h"
#include "graphics/nine_patch.h"
#include "graphics/macgui/macwindowmanager.h"

#include "graphics/managed_surface.h"

namespace Graphics {

NinePatchSide::~NinePatchSide() {
	for (uint i = 0; i < _m.size(); i++)
		delete _m[i];

	_m.clear();
}


bool NinePatchSide::init(Graphics::ManagedSurface *bmp, bool vertical, uint32 black, uint32 white, int titlePos, int *titleIndex) {
	const uint32 key = bmp->getTransparentColor();
	const uint len = vertical ? bmp->h : bmp->w;
	uint i;
	int s, t, z;

	int titleWidth = 0;
	int index = 0;
	_m.clear();

	for (i = 1, s = -1, t = 0, z = -1; i < len; ++i) {
		int zz;
		uint32 color = vertical ? bmp->getPixel(0, i) : bmp->getPixel(i, 0);

		if (i == len - 1) {
			zz = -1;
		} else if (color == black) {
			zz = 0;
		} else if (color == key || color == white) {
			zz = 1;
		} else {
			warning("NinePatchSide::init(): Bad pixel at %d,%d", (vertical ? 0 : i), (vertical ? i : 0));
			return false;
		}

		if (z != zz) {
			if (s != -1) {
				NinePatchMark *mrk = new NinePatchMark;

				mrk->offset = s;
				mrk->length = i - s;
				if (z == 0) {
					mrk->ratio = 1;
					t += mrk->length;
				} else {
					mrk->ratio = 0;
				}
				// if there is title, then we try to recalc the t, because we want to fix the title width
				if (titlePos > 0 && (int)i == titlePos) {
					t -= mrk->length;
					titleWidth = mrk->length;
					if (titleIndex)
						*titleIndex = index;
				}
				_m.push_back(mrk);
				index++;
			}
			s = i;
			z = zz;
		}
	}

	_fix = len - 2 - t;
	// here, titleWidth represent the width in 9-patch image, and we delete it from fix size
	// so, currently, we will have 3 part, fix size, stretchable size and title size
	if (titleWidth)
		_fix -= titleWidth;
	for (i = 0; i < _m.size(); ++i) {
		if (_m[i]->ratio)
			_m[i]->ratio = _m[i]->length / (float)t;
	}

	return true;
}

void NinePatchSide::calcOffsets(int len, int titleIndex, int titleWidth) {
	uint i, j;
	int dest_offset = 0;
	// if we don't got titleIndex, then we better set titleWidth to 0
	if (titleIndex == 0)
		titleWidth = 0;
	int remaining_stretch = len - _fix - titleWidth;
	// if titleWidth is too big, which may cause the remaining_stretch be 0, so we check it here
	if (remaining_stretch < 0)
		remaining_stretch = 0;

	for (i = 0, j = 0; i < _m.size(); ++i) {
		_m[i]->dest_offset = dest_offset;

		if (titleIndex > 0 && i == (uint)titleIndex) {
			_m[i]->dest_length = titleWidth;
		} else {
			if (_m[i]->ratio == 0) {
				_m[i]->dest_length = _m[i]->length;
			} else {
				_m[i]->dest_length = (len - _fix - titleWidth) * _m[i]->ratio + 0.5;
				remaining_stretch -= _m[i]->dest_length;
				j = i;
			}
		}
		dest_offset += _m[i]->dest_length;
	}

	if (remaining_stretch && _m.size()) {
		_m[j]->dest_length += remaining_stretch;
		if (j + 1 < _m.size())
			_m[j + 1]->dest_offset += remaining_stretch;
	}
}

int NinePatchBitmap::getTitleOffset() {
	if (_titleIndex == 0)
		return 0;
	return _h._m[_titleIndex]->dest_offset;
}

NinePatchBitmap::NinePatchBitmap(Graphics::ManagedSurface *bmp, bool owns_bitmap, int titlePos) {
	int i;

	_bmp = bmp;
	_destroy_bmp = owns_bitmap;
	_h._m.clear();
	_v._m.clear();
	_cached_dw = 0;
	_cached_dh = 0;
	_width = bmp->w - 2;
	_height = bmp->h - 2;
	_titleIndex = 0;
	_titleWidth = 0;
	_titlePos = titlePos;

	const uint32 key = bmp->getTransparentColor();
	uint32 black, white;

	if (bmp->format.isCLUT8()) {
		Palette palette(256);
		bmp->grabPalette(palette, 0, 256);

		black = (uint32)-1;
		white = (uint32)-1;

		for (int j = 0; j < 256; j++) {
			byte r = palette.data[(j * 3) + 0];
			byte g = palette.data[(j * 3) + 1];
			byte b = palette.data[(j * 3) + 2];

			if (black == uint32(-1) && r == 0 && g == 0 && b == 0)
				black = j;
			else if (white == uint32(-1) && r == 255 && g == 255 && b == 255)
				white = j;
		}

		if (black == uint32(-1) || white == uint32(-1))
			goto bad_bitmap;
	} else {
		black = bmp->format.RGBToColor(0, 0, 0);
		white = bmp->format.RGBToColor(255, 255, 255);
	}

	if (_width <= 0 || _height <= 0)
		goto bad_bitmap;

	if (!bmp->hasTransparentColor())
		goto bad_bitmap;

	/* make sure all four corners are transparent */
#define _check_pixel(x, y) \
	if (bmp->getPixel(x, y) != key) goto bad_bitmap;

	_check_pixel(0, 0);
	_check_pixel(bmp->w - 1, 0);
	_check_pixel(0, bmp->h - 1);
	_check_pixel(bmp->w - 1, bmp->h - 1);
#undef _check_pixel

	_padding.top = _padding.right = _padding.bottom = _padding.left = -1;

	i = 1;
	while (i < bmp->w) {
		uint32 color = bmp->getPixel(i, bmp->h - 1);

		if (color == black) {
			if (_padding.left == -1)
				_padding.left = i - 1;
			else if (_padding.right != -1)
				goto bad_bitmap;
		} else if (color == key) {
			if (_padding.left != -1 && _padding.right == -1)
				_padding.right = bmp->w - i - 1;
		}
		++i;
	}

	i = 1;
	while (i < bmp->h) {
		uint32 color = bmp->getPixel(bmp->w - 1, i);

		if (color == black) {
			if (_padding.top == -1)
				_padding.top = i - 1;
			else if (_padding.bottom != -1)
				goto bad_bitmap;
		} else if (color == key) {
			if (_padding.top != -1 && _padding.bottom == -1)
				_padding.bottom = bmp->h - i - 1;
		}
		++i;
	}

	if (!_h.init(bmp, false, black, white, titlePos, &_titleIndex) || !_v.init(bmp, true, black, white)) {
bad_bitmap:
		warning("NinePatchBitmap::NinePatchBitmap(): Bad bitmap");

		_h._m.clear();
		_v._m.clear();
	}
}

void NinePatchBitmap::modifyTitleWidth(int titleWidth) {
	if (_titlePos == 0) return;
	_titleWidth = titleWidth;
	_h.calcOffsets(_cached_dw, _titleIndex, _titleWidth);
}

void NinePatchBitmap::blit(Graphics::ManagedSurface &target, int dx, int dy, int dw, int dh, MacWindowManager *wm) {
	/* don't draw bitmaps that are smaller than the fixed area */
	if (dw < _h._fix || dh < _v._fix)
		return;

	if (dw < _h._fix + _titleWidth)
		dw = _h._fix + _titleWidth;

	/* only recalculate the offsets if they have changed since the last draw */
	if (_cached_dw != dw || _cached_dh != dh) {
		_h.calcOffsets(dw, _titleIndex, _titleWidth);
		_v.calcOffsets(dh);

		_cached_dw = dw;
		_cached_dh = dh;
	}

	/* Handle CLUT8 */
	if (target.format.bytesPerPixel == 1) {
		if (!wm)
			error("NinePatchBitmap::blit(): Trying to blit into a surface with 8bpp, you need a palette.");

		uint32 transColor = _bmp->getTransparentColor();
		ManagedSurface *srf = new ManagedSurface();
		srf->create(target.w, target.h, _bmp->format);
		srf->fillRect(Common::Rect(srf->w, srf->h), transColor);

		drawRegions(*srf, dx, dy, dw, dh);

		if (srf->format.isCLUT8()) {
			Palette palette(256);
			_bmp->grabPalette(palette, 0, 256);

			for (int i = 0; i < srf->w; ++i) {
				for (int j = 0; j < srf->h; ++j) {
					byte color = *(byte*)srf->getBasePtr(i, j);
					if (color != transColor) {
						byte r = palette.data[(color * 3) + 0];
						byte g = palette.data[(color * 3) + 1];
						byte b = palette.data[(color * 3) + 2];
						*((byte *)target.getBasePtr(i, j)) = wm->findBestColor(r, g, b);
					}
				}
			}
		} else {
			for (int i = 0; i < srf->w; ++i) {
				for (int j = 0; j < srf->h; ++j) {
					uint32 color = srf->getPixel(i, j);
					if (color != transColor) {
						byte a, r, g, b;
						srf->format.colorToARGB(color, a, r, g, b);
						*((byte *)target.getBasePtr(i, j)) = wm->findBestColor(r, g, b);
					}
				}
			}
		}

		srf->free();
		delete srf;

		return;
	}

	/* if the bitmap is the same size as the origin, then draw it as-is */
	if (dw == _width && dh == _height) {
		Common::Rect r(1, 1, dw, dh);

		target.blitFrom(*_bmp, r, Common::Point(dx, dy));
		return;
	}

	/* Else, draw regions normally */
	drawRegions(target, dx, dy, dw, dh);
}

NinePatchBitmap::~NinePatchBitmap() {
	if (_destroy_bmp) {
		_bmp->free();
		delete _bmp;
	}
}

void NinePatchBitmap::drawRegions(Graphics::ManagedSurface &target, int dx, int dy, int dw, int dh) {
	/* draw each region */
	for (uint i = 0; i < _v._m.size(); ++i) {
		for (uint j = 0; j < _h._m.size(); ++j) {
			Common::Rect srcRect(_h._m[j]->offset, _v._m[i]->offset,
						_h._m[j]->offset + _h._m[j]->length,
						 _v._m[i]->offset + _v._m[i]->length);
			Common::Rect dstRect(dx + _h._m[j]->dest_offset, dy + _v._m[i]->dest_offset,
						dx + _h._m[j]->dest_offset + _h._m[j]->dest_length,
						dy + _v._m[i]->dest_offset + _v._m[i]->dest_length);

			target.blitFrom(*_bmp, srcRect, dstRect);
		}
	}
}

} // end of namespace Graphics
