/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "scumm/base-costume.h"

namespace Scumm {

byte BaseCostumeRenderer::drawCostume(const VirtScreen &vs, const CostumeData &cost) {
	int i;
	byte result = 0;

	_outptr = vs.screenPtr + vs.xstart;
	_outwidth = vs.width;
	_outheight = vs.height;
	_numStrips = vs.width / 8;

	if (_vm->_version == 1) {
		_xmove = 0;
		_ymove = 0;
	} else if (_vm->_features & GF_OLD_BUNDLE) {
		_xmove = -72;
		_ymove = -100;
	} else {
		_xmove = _ymove = 0;
	}
	for (i = 0; i < 16; i++)
		result |= drawLimb(cost, i);
	return result;
}

void BaseCostumeRenderer::codec1_ignorePakCols(int num) {
	num *= _height;

	do {
		v1.replen = *_srcptr++;
		v1.repcolor = v1.replen >> v1.shr;
		v1.replen &= v1.mask;

		if (!v1.replen)
			v1.replen = *_srcptr++;

		do {
			if (!--num)
				return;
		} while (--v1.replen);
	} while (1);
}

bool ScummEngine::isCostumeInUse(int cost) const {
	int i;
	Actor *a;

	if (_roomResource != 0)
		for (i = 1; i < _numActors; i++) {
			a = derefActor(i);
			if (a->isInCurrentRoom() && a->costume == cost)
				return true;
		}

	return false;
}

} // End of namespace Scumm
