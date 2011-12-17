/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 */


#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/screen_rpg.h"
#include "kyra/kyra_v1.h"

#include "common/endian.h"

namespace Kyra {

Screen_Rpg::Screen_Rpg(KyraEngine_v1 *vm, OSystem *system, const ScreenDim *dimTable, int dimTableSize) : Screen(vm, system), _screenDimTable(dimTable), _screenDimTableCount(dimTableSize) {
	_customDimTable = new ScreenDim*[_screenDimTableCount];
	memset(_customDimTable, 0, sizeof(ScreenDim *)* _screenDimTableCount);
	_curDimIndex = 0;
}

Screen_Rpg::~Screen_Rpg() {
	for (int i = 0; i < _screenDimTableCount; i++)
		delete _customDimTable[i];
	delete[] _customDimTable;
}

void Screen_Rpg::setScreenDim(int dim) {
	assert(dim < _screenDimTableCount);
	_curDim = _customDimTable[dim] ? (const ScreenDim *)_customDimTable[dim] : &_screenDimTable[dim];
	_curDimIndex = dim;
}

const ScreenDim *Screen_Rpg::getScreenDim(int dim) {
	assert(dim < _screenDimTableCount);
	return _customDimTable[dim] ? (const ScreenDim *)_customDimTable[dim] : &_screenDimTable[dim];
}

void Screen_Rpg::modifyScreenDim(int dim, int x, int y, int w, int h) {
	if (!_customDimTable[dim])
		_customDimTable[dim] = new ScreenDim;

	memcpy(_customDimTable[dim], &_screenDimTable[dim], sizeof(ScreenDim));
	_customDimTable[dim]->sx = x;
	_customDimTable[dim]->sy = y;
	_customDimTable[dim]->w = w;
	_customDimTable[dim]->h = h;
	if (dim == _curDimIndex || _vm->game() == GI_LOL)
		setScreenDim(dim);
}

void Screen_Rpg::crossFadeRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage) {
	if (srcPage > 13 || dstPage > 13)
		error("Screen_Rpg::crossFadeRegion(): attempting to use temp page as source or dest page.");

	hideMouse();

	uint16 *wB = (uint16*)_pagePtrs[14];
	uint8 *hB = _pagePtrs[14] + 640;

	for (int i = 0; i < w; i++)
		wB[i] = i;

	for (int i = 0; i < h; i++)
		hB[i] = i;

	for (int i = 0; i < w; i++)
		SWAP(wB[_vm->_rnd.getRandomNumberRng(0, w - 1)], wB[i]);

	for (int i = 0; i < h; i++)
		SWAP(hB[_vm->_rnd.getRandomNumberRng(0, h - 1)], hB[i]);

	uint8 *s = _pagePtrs[srcPage];
	uint8 *d = _pagePtrs[dstPage];

	for (int i = 0; i < h; i++) {
		int iH = i;
		uint32 end = _system->getMillis() + 3;
		for (int ii = 0; ii < w; ii++) {
			int sX = x1 + wB[ii];
			int sY = y1 + hB[iH];
			int dX = x2 + wB[ii];
			int dY = y2 + hB[iH];

			if (++iH >= h)
				iH = 0;

			d[dY * 320 + dX] = s[sY * 320 + sX];
			addDirtyRect(dX, dY, 1, 1);
		}

		// This tries to speed things up, to get similiar speeds as in DOSBox etc.
		// We can't write single pixels directly into the video memory like the original did.
		// We also (unlike the original) want to aim at similiar speeds for all platforms.
		if (!(i % 10))
			updateScreen();

		uint32 cur = _system->getMillis();
		if (end > cur)
			_system->delayMillis(end - cur);
	}

	updateScreen();
	showMouse();
}

}	// End of namespace Kyra

#endif // ENABLE_EOB || ENABLE_LOL
