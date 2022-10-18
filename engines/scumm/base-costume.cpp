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
 *
 */


#include "scumm/base-costume.h"
#include "scumm/costume.h"

namespace Scumm {

byte BaseCostumeRenderer::drawCostume(const VirtScreen &vs, int numStrips, const Actor *a, bool drawToBackBuf) {
	int i;
	byte result = 0;

	_out = vs;
	if (drawToBackBuf)
		_out.setPixels(vs.getBackPixels(0, 0));
	else
		_out.setPixels(vs.getPixels(0, 0));

	_actorX += _vm->_virtscr[kMainVirtScreen].xstart & 7;
	_out.w = _out.pitch / _vm->_bytesPerPixel;
	// We do not use getBasePtr here because the offset to pixels never used
	// _vm->_bytesPerPixel, but it seems unclear why.
	_out.setPixels((byte *)_out.getPixels() - (_vm->_virtscr[kMainVirtScreen].xstart & 7));

	_numStrips = numStrips;

	if (_vm->_game.version <= 1) {
		_xMove = 0;
		_yMove = 0;
	} else if (_vm->_game.features & GF_OLD_BUNDLE) {
		_xMove = -72;
		_yMove = -100;
	} else {
		_xMove = _yMove = 0;
	}
	for (i = 0; i < 16; i++)
		result |= drawLimb(a, i);
	return result;
}

void BaseCostumeRenderer::skipCelLines(ByleRLEData &compData, int num) {
	num *= _height;

	do {
		compData.repLen = *_srcPtr++;
		compData.repColor = compData.repLen >> compData.shr;
		compData.repLen &= compData.mask;

		if (!compData.repLen)
			compData.repLen = *_srcPtr++;

		do {
			if (!--num)
				return;
		} while (--compData.repLen);
	} while (true);
}

bool ScummEngine::isCostumeInUse(int cost) const {
	Actor *a;

	if (_roomResource != 0)
		for (int i = 1; i < _numActors; i++) {
			a = derefActor(i);
			if (a->isInCurrentRoom() && a->_costume == cost)
				return true;
		}

	return false;
}

} // End of namespace Scumm
