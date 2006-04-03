/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "simon/simon.h"
#include "simon/intern.h"
#include "simon/vga.h"

#include <sys/stat.h>

namespace Simon {

void SimonEngine::hyperLinkOn(uint16 x)
{
	if ((_bitArray[3] & (1 << 3)) == 0)
		return;

	_hyperLink = x;
	_variableArray[50] = _textWindow->textColumn+_textWindow->x;
	_variableArray[51] = _textWindow->textRow+_textWindow->y+
						(_oracleMaxScrollY - _textWindow->textColumnOffset) * 15;
}


void SimonEngine::hyperLinkOff()
{
	if ((_bitArray[3] & (1 << 3)) == 0)
		return;

	_variableArray[52] = _textWindow->x + _textWindow->textColumn - _variableArray[50];
	addNewHitArea(_variableArray[53], _variableArray[50], _variableArray[51], _variableArray[52], 15, 145, 208, _dummyItem1);
	_variableArray[53]++;
	_hyperLink = 0;
}

void SimonEngine::oracleLogo() {
	Common::Rect srcRect, dstRect;
	byte *src, *dst;
	uint16 w, h;

	dstRect.left = 16;
	dstRect.top = 16;
	dstRect.right = 58;
	dstRect.bottom = 59;

	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = 42;
	srcRect.bottom = 43;

	src = _iconFilePtr;
	dst = getBackBuf() + _screenWidth * dstRect.top + dstRect.left;

	for (h = 0; h < dstRect.height(); h++) {
		for (w = 0; w < dstRect.width(); w++) {
			if (src[w])
				dst[w] = src[w];
		}
		src += 336;
		dst += _screenWidth;
	}
}

void SimonEngine::swapCharacterLogo() {
	Common::Rect srcRect, dstRect;
	byte *src, *dst;
	uint16 w, h;
	int x;

	dstRect.left = 64;
	dstRect.top = 16;
	dstRect.right = 106;
	dstRect.bottom = 59;

	srcRect.top = 0;
	srcRect.bottom = 43;

	x = _variableArray[91];
	if (x > _variableArray[90])
		x--;
	if( x < _variableArray[90])
		x++;
	_variableArray[91] = x;

	x++;
	x *= 42;

	srcRect.left = x;
	srcRect.right = srcRect.left + 42;

	src = _iconFilePtr + srcRect.top * 336 + srcRect.left;
	dst = getBackBuf() + _screenWidth * dstRect.top + dstRect.left;

	for (h = 0; h < dstRect.height(); h++) {
		for (w = 0; w < dstRect.width(); w++) {
			if (src[w])
				dst[w] = src[w];
		}
		src += 336;
		dst += _screenWidth;
	}
}

} // End of namespace Simon
