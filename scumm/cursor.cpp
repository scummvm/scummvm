/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm/bomp.h"
#include "scumm/scumm.h"


namespace Scumm {

/*
 * Mouse cursor cycle colors (for the default crosshair).
 */
static const byte default_v1_cursor_colors[4] = {
	1, 1, 12, 11
};

static const byte default_cursor_colors[4] = {
	15, 15, 7, 8
};



static const uint16 default_cursor_images[5][16] = {
	/* cross-hair */
	{ 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000, 0x7e3f,
	  0x0000, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000 },
	/* hourglass */
	{ 0x0000, 0x7ffe, 0x6006, 0x300c, 0x1818, 0x0c30, 0x0660, 0x03c0,
	  0x0660, 0x0c30, 0x1998, 0x33cc, 0x67e6, 0x7ffe, 0x0000, 0x0000 },
	/* arrow */
	{ 0x0000, 0x4000, 0x6000, 0x7000, 0x7800, 0x7c00, 0x7e00, 0x7f00,
	  0x7f80, 0x78c0, 0x7c00, 0x4600, 0x0600, 0x0300, 0x0300, 0x0180 },
	/* hand */
	{ 0x1e00, 0x1200, 0x1200, 0x1200, 0x1200, 0x13ff, 0x1249, 0x1249,
	  0xf249, 0x9001, 0x9001, 0x9001, 0x8001, 0x8001, 0x8001, 0xffff },
	/* cross-hair zak256 - chrilith palmos */
/*
	{ 0x0080, 0x0080, 0x02a0, 0x01c0, 0x0080, 0x1004, 0x0808, 0x7c1f,
	  0x0808, 0x1004, 0x0080, 0x01c0, 0x02a0, 0x0080, 0x0080, 0x0000 },
*/
	{ 0x0080, 0x02a0, 0x01c0, 0x0080, 0x0000, 0x2002, 0x1004, 0x780f,
	  0x1004, 0x2002, 0x0000, 0x0080, 0x01c0, 0x02a0, 0x0080, 0x0000 },
};

static const byte default_cursor_hotspots[10] = {
	8, 7,   8, 7,   1, 1,   5, 0,
	8, 7, //zak256
};


void ScummEngine::setupCursor() {
	_cursor.animate = 1;
	if (_gameId == GID_TENTACLE && res.roomno[rtRoom][60]) {
		// HACK: For DOTT we manually set the default cursor. See also bug #786994
		setCursorImg(697, 60, 1);
		makeCursorColorTransparent(1);
	}
}

void ScummEngine::grabCursor(int x, int y, int w, int h) {
	VirtScreen *vs = findVirtScreen(y);

	if (vs == NULL) {
		warning("grabCursor: invalid Y %d", y);
		return;
	}

	grabCursor(vs->screenPtr + (y - vs->topline) * vs->width + x, w, h);

}

void ScummEngine::grabCursor(byte *ptr, int width, int height) {
	uint size;
	byte *dst;

	size = width * height;
	if (size > sizeof(_grabbedCursor))
		error("grabCursor: grabbed cursor too big");

	_cursor.width = width;
	_cursor.height = height;
	_cursor.animate = 0;

	dst = _grabbedCursor;
	for (; height; height--) {
		memcpy(dst, ptr, width);
		dst += width;
		ptr += _screenWidth;
	}

	updateCursor();
}

void ScummEngine::useIm01Cursor(const byte *im, int w, int h) {
	VirtScreen *vs = &virtscr[0];
	byte *buf, *dst;
	const byte *src;
	int i;

	w *= 8;
	h *= 8;

	dst = buf = (byte *) malloc(w * h);
	src = vs->screenPtr + vs->xstart;

	for (i = 0; i < h; i++) {
		memcpy(dst, src, w);
		dst += w;
		src += vs->width;
	}

	drawBox(0, 0, w - 1, h - 1, 0xFF);

	vs->hasTwoBuffers = false;
	gdi.disableZBuffer();
	gdi.drawBitmap(im, vs, _screenStartStrip, 0, w, h, 0, w / 8, 0);
	vs->hasTwoBuffers = true;
	gdi.enableZBuffer();

	grabCursor(vs->screenPtr + vs->xstart, w, h);

	src = buf;
	dst = vs->screenPtr + vs->xstart;

	for (i = 0; i < h; i++) {
		memcpy(dst, src, w);
		dst += vs->width;
		src += w;
	}

	free(buf);
}

void ScummEngine::setCursor(int cursor) {
	if (cursor >= 0 && cursor <= 3)
		_currentCursor = cursor;
	else
		warning("setCursor(%d)", cursor);
}

void ScummEngine::setCursorHotspot(int x, int y) {
	_cursor.hotspotX = x;
	_cursor.hotspotY = y;
	// FIXME this hacks around offset cursor in the humongous games
	if (_features & GF_HUMONGOUS) {
		_cursor.hotspotX += 15;
		_cursor.hotspotY += 15;
	}
}

void ScummEngine::updateCursor() {
	_system->set_mouse_cursor(_grabbedCursor, _cursor.width, _cursor.height,
							_cursor.hotspotX, _cursor.hotspotY);
}

void ScummEngine::animateCursor() {
	if (_cursor.animate) {
		if (!(_cursor.animateIndex & 0x1)) {
			decompressDefaultCursor((_cursor.animateIndex >> 1) & 3);
		}
		_cursor.animateIndex++;
	}
}

void ScummEngine::useBompCursor(const byte *im, int width, int height) {
	uint size;

	width *= 8;
	height *= 8;

	size = width * height;
	if (size > sizeof(_grabbedCursor))
		error("useBompCursor: cursor too big (%d)", size);

	_cursor.width = width;
	_cursor.height = height;
	_cursor.animate = 0;

	// Skip the header
	if (_version == 8) {
		im += 16;
	} else {
		im += 18;
	}
	decompressBomp(_grabbedCursor, im, width, height);

	updateCursor();
}

void ScummEngine::decompressDefaultCursor(int idx) {
	int i, j;
	byte color;

	memset(_grabbedCursor, 0xFF, sizeof(_grabbedCursor));

	if (_version == 1)
		color = default_v1_cursor_colors[idx];
	else
		color = default_cursor_colors[idx];

	// FIXME: None of the stock cursors are right for Loom. Why is that?

	if (_gameId == GID_LOOM || _gameId == GID_LOOM256) {
		int w = 0;

		_cursor.width = 8;
		_cursor.height = 8;
		_cursor.hotspotX = 0;
		_cursor.hotspotY = 0;
		
		for (i = 0; i < 8; i++) {
			w += (i >= 6) ? -2 : 1;
			for (j = 0; j < w; j++)
				_grabbedCursor[i * 8 + j] = color;
		}
	} else if (_version <= 2) {
		_cursor.width = 23;
		_cursor.height = 21;
		_cursor.hotspotX = 11;
		_cursor.hotspotY = 10;

		byte *hotspot = _grabbedCursor + _cursor.hotspotY * _cursor.width + _cursor.hotspotX;

		// Crosshair, slightly assymetric

		for (i = 0; i < 7; i++) {
			*(hotspot - 5 - i) = color;
			*(hotspot + 5 + i) = color;
		}

		for (i = 0; i < 8; i++) {
			*(hotspot - _cursor.width * (3 + i)) = color;
			*(hotspot + _cursor.width * (3 + i)) = color;
		}

		// Arrow heads, diagonal lines

		for (i = 1; i <= 3; i++) {
			*(hotspot - _cursor.width * i - 5 - i) = color;
			*(hotspot + _cursor.width * i - 5 - i) = color;
			*(hotspot - _cursor.width * i + 5 + i) = color;
			*(hotspot + _cursor.width * i + 5 + i) = color;
			*(hotspot - _cursor.width * (i + 3) - i) = color;
			*(hotspot - _cursor.width * (i + 3) + i) = color;
			*(hotspot + _cursor.width * (i + 3) - i) = color;
			*(hotspot + _cursor.width * (i + 3) + i) = color;
		}

		// Final touches

		*(hotspot - _cursor.width - 7) = color;
		*(hotspot - _cursor.width + 7) = color;
		*(hotspot + _cursor.width - 7) = color;
		*(hotspot + _cursor.width + 7) = color;
		*(hotspot - (_cursor.width * 5) - 1) = color;
		*(hotspot - (_cursor.width * 5) + 1) = color;
		*(hotspot + (_cursor.width * 5) - 1) = color;
		*(hotspot + (_cursor.width * 5) + 1) = color;
	} else {
		byte currentCursor = _currentCursor;

#ifdef __PALM_OS__
		if (_gameId == GID_ZAK256 && currentCursor == 0)
			currentCursor = 4;
#endif

		_cursor.width = 16;
		_cursor.height = 16;
		_cursor.hotspotX = default_cursor_hotspots[2 * currentCursor];
		_cursor.hotspotY = default_cursor_hotspots[2 * currentCursor + 1];

		for (i = 0; i < 16; i++) {
			for (j = 0; j < 16; j++) {
				if (default_cursor_images[currentCursor][i] & (1 << j))	
					_grabbedCursor[16 * i + 15 - j] = color;
			}
		}
	}

	updateCursor();
}

void ScummEngine::makeCursorColorTransparent(int a) {
	int i, size;

	size = _cursor.width * _cursor.height;

	for (i = 0; i < size; i++)
		if (_grabbedCursor[i] == (byte)a)
			_grabbedCursor[i] = 0xFF;

	updateCursor();
}

} // End of namespace Scumm
