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
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v3.h"

namespace Kyra {

void KyraEngine_v3::clearAnimObjects() {
	debugC(9, kDebugLevelAnimator, "KyraEngine_v3::clearAnimObjects()");

	for (int i = 0; i < 67; ++i)
		_animObjects[i].enabled = false;

	_animObjects[0].index = 0;
	_animObjects[0].type = 0;
	_animObjects[0].enabled = true;
	_animObjects[0].unk8 = 1;
	_animObjects[0].flags = 0x800;
	_animObjects[0].width = 57;
	_animObjects[0].height = 91;
	_animObjects[0].width2 = 4;
	_animObjects[0].height2 = 10;

	for (int i = 1; i < 17; ++i) {
		_animObjects[i].type = 2;
		_animObjects[i].flags = 0;
		_animObjects[i].enabled = false;
		_animObjects[i].needRefresh = 0;
		_animObjects[i].unk8 = 1;
	}

	for (int i = 17; i <= 66; ++i) {
		_animObjects[i].type = 1;
		_animObjects[i].unk8 = 1;
		_animObjects[i].flags = 0x800;
		_animObjects[i].width = 24;
		_animObjects[i].height = 20;
		_animObjects[i].width2 = 0;
		_animObjects[i].height2 = 0;
	}
}

} // end of namespace Kyra
