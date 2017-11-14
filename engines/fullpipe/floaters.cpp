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
			FloaterArray1 *f = new FloaterArray1;

			f->val1 = sub->_subVars->_value.intValue;
			f->val2 = sub->_subVars->_nextVarObj->_value.intValue;

			_array1.push_back(f);

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

	ani->_movement->setDynamicPhaseIndex(g_fp->_rnd.getRandomNumber(nummoves - 1));

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
	for (uint i = 0; i < _array2.size(); ++i) {
		if (_array2[i]->val13 <= 0) {
			if (_array2[i]->val4 != _array2[i]->val2 || _array2[i]->val5 != _array2[i]->val3) {
				if (_array2[i]->val9 < 2.0)
					_array2[i]->val9 = 2.0;

				int dy = _array2[i]->val3 - _array2[i]->val5;
				int dx = _array2[i]->val2 - _array2[i]->val4;
				double dst = sqrt((double)(dy * dy + dx * dx));
				double at = atan2((double)dy, (double)dx);
				int newX = (int)(cos(at) * _array2[i]->val9);
				int newY = (int)(sin(at) * _array2[i]->val9);

				if (dst < _array2[i]->val9) {
					newX = _array2[i]->val2 - _array2[i]->val4;
					newY = _array2[i]->val3 - _array2[i]->val5;
				}
				if (dst <= 30.0) {
					if (dst < 30.0) {
						_array2[i]->val9 = _array2[i]->val9 - _array2[i]->val9 * 0.5;

						if (_array2[i]->val9 < 2.0)
							_array2[i]->val9 = 2.0;
					}
				} else {
					_array2[i]->val9 = _array2[i]->val9 * 0.5 + _array2[i]->val9;

					if (_array2[i]->val9 > _array2[i]->val11)
						_array2[i]->val9 = _array2[i]->val11;
				}

				_array2[i]->val4 += newX;
				_array2[i]->val5 += newY;
				_array2[i]->ani->setOXY(newX + _array2[i]->ani->_ox, newY + _array2[i]->ani->_oy);

				if (_array2[i]->val4 == _array2[i]->val2 && _array2[i]->val5 == _array2[i]->val3) {
					_array2[i]->val9 = 0.0;

					_array2[i]->val13 = g_fp->_rnd.getRandomNumber(200) + 20;

					if (_array2[i]->fflags & 1) {
						g_fp->_currentScene->deleteStaticANIObject(_array2[i]->ani);

						if (_array2[i]->ani)
							delete _array2[i]->ani;

						_array2.remove_at(i);

						i--;

						if (!_array2.size())
							g_fp->stopAllSoundInstances(SND_CMN_060);

						continue;
					}
				}
			} else {
				if ((_array2[i]->fflags & 4) && _array2[i]->countdown < 1) {
					_array2[i]->fflags |= 1;
					_array2[i]->val2 = _array2[i]->val6;
					_array2[i]->val3 = _array2[i]->val7;
				} else {
					if (_array2[i]->fflags & 2) {
						int idx1 = g_fp->_rnd.getRandomNumber(_array1.size() - 1);

						_array2[i]->val2 = _array1[idx1]->val1;
						_array2[i]->val3 = _array1[idx1]->val2;
					} else {
						Common::Rect rect;

						if (!_hRgn)
							error("Floaters::update(): empty fliers region");

						_hRgn->getBBox(&rect);

						int x2 = rect.left + g_fp->_rnd.getRandomNumber(rect.right - rect.left);
						int y2 = rect.top + g_fp->_rnd.getRandomNumber(rect.bottom - rect.top);

						if (_hRgn->pointInRegion(x2, y2)) {
							int dx = _array2[i]->val2 - x2;
							int dy = _array2[i]->val3 - y2;
							double dst = sqrt((double)(dy * dy + dx * dx));

							if (dst < 300.0 || !_hRgn->pointInRegion(_array2[i]->val4, _array2[i]->val5)) {
								_array2[i]->val2 = x2;
								_array2[i]->val3 = y2;
							}
						}
					}

					g_fp->playSound(SND_CMN_061, 0);

					if (_array2[i]->fflags & 4)
						_array2[i]->countdown--;
				}
			}
		} else {
			_array2[i]->val13--;
		}

		if (!_array2[i]->ani->_movement && _array2[i]->ani->_statics->_staticsId == ST_FLY_FLY) {
			if (!_array2[i]->val15) {
				g_fp->playSound(SND_CMN_060, 1);

				_array2[i]->val15 = 1;
			}

			_array2[i]->ani->startAnim(MV_FLY_FLY, 0, -1);
		}
	}
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
