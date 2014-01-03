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
#include "fullpipe/statics.h"
#include "fullpipe/scene.h"
#include "fullpipe/constants.h"
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

void Floaters::genFlies(Scene *sc, int x, int y, int priority, int flags) {
	StaticANIObject *ani = new StaticANIObject(g_fp->accessScene(SC_COMMON)->getStaticANIObject1ById(ANI_FLY, -1));

	ani->_statics = ani->getStaticsById(ST_FLY_FLY);
	ani->_movement = 0;
	ani->setOXY(x, y);
	ani->_flags |= 4;
	ani->_priority = priority;

	sc->addStaticANIObject(ani, 1);

	ani->startAnim(MV_FLY_FLY, 0, -1);

	int nummoves;

	if (ani->_movement->_currMovement)
		nummoves = ani->_movement->_currMovement->_dynamicPhases.size();
	else
		nummoves = ani->_movement->_dynamicPhases.size();

	ani->_movement->setDynamicPhaseIndex(g_fp->_rnd->getRandomNumber(nummoves));

	FloaterArray2 *arr2 = new FloaterArray2;

	arr2->ani = ani;
	arr2->val11 = 15.0;
	arr2->val3 = y;
	arr2->val5 = y;
	arr2->val2 = x;
	arr2->val4 = x;
	arr2->fflags = flags;

	_array2.push_back(arr2);
}

void Floaters::update() {
	warning("STUB: Floaters::update()");
}

void Floaters::stopAll() {
	for (uint i = 0; i < _array2.size(); i++) {
		g_fp->_currentScene->deleteStaticANIObject(_array2[i]->ani);

		delete _array2[i]->ani;
	}

	_array2.clear();

	g_fp->stopAllSoundInstances(SND_CMN_060);
}


} // End of namespace Fullpipe
