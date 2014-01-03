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

#include "fullpipe/fullpipe.h"
#include "fullpipe/floaters.h"
#include "fullpipe/utils.h"
#include "fullpipe/objects.h"
#include "fullpipe/motion.h"
#include "fullpipe/objectnames.h"

namespace Fullpipe {

Floaters::~Floaters() {
	delete _hRgn;
}

void Floaters::init(GameVar *var) {
	_array1.clear();
	_array2.clear();

	GameVar *varFliers = var->getSubVarByName(sO_Fliers);

	if (!varFliers)
		return;

	GameVar *sub = varFliers->getSubVarByName("flyIdleRegion");

	if (sub) {
		_hRgn = new ReactPolygonal();

		_hRgn->_pointCount = sub->getSubVarsCount();
		_hRgn->_points = (Common::Point **)malloc(sizeof(Common::Point *) * _hRgn->_pointCount);

		sub = sub->_subVars;

		int idx = 0;

		while (sub) {
			_hRgn->_points[idx] = new Common::Point;
			_hRgn->_points[idx]->x = sub->_subVars->_value.intValue;
			_hRgn->_points[idx]->y = sub->_subVars->_nextVarObj->_value.intValue;

			idx++;
			sub = sub->_nextVarObj;
		}
	}

	sub = varFliers->getSubVarByName("flyIdlePath");

	if (sub) {
		_array1.reserve(sub->getSubVarsCount());

		sub = sub->_subVars;

		int idx = 0;

		while (sub) {
			_array1[idx]->val1 = sub->_subVars->_value.intValue;
			_array1[idx]->val2 = sub->_subVars->_nextVarObj->_value.intValue;

			idx++;
			sub = sub->_nextVarObj;
		}

	}
}

void Floaters::genFlies(Scene *sc, int x, int y, int a5, int a6) {
	warning("STUB: Floaters::genFlies()");
}

void Floaters::update() {
	warning("STUB: Floaters::update()");
}

void Floaters::stopAll() {
	warning("STUB: Floaters::stopAll()");
}


} // End of namespace Fullpipe
