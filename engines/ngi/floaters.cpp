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

#include "ngi/ngi.h"
#include "ngi/floaters.h"
#include "ngi/utils.h"
#include "ngi/objects.h"
#include "ngi/motion.h"
#include "ngi/statics.h"
#include "ngi/scene.h"
#include "ngi/constants.h"
#include "ngi/objectnames.h"

namespace NGI {

void Floaters::init(GameVar *var) {
	_array1.clear();
	_array2.clear();

	GameVar *varFliers = var->getSubVarByName(sO_Fliers);

	if (!varFliers)
		return;

	GameVar *sub = varFliers->getSubVarByName("flyIdleRegion");

	if (sub) {
		_hRgn.reset(new ReactPolygonal());

		_hRgn->_points.resize(sub->getSubVarsCount());

		sub = sub->_subVars;
		uint idx = 0;
		while (sub) {
			_hRgn->_points[idx].x = sub->_subVars->_value.intValue;
			_hRgn->_points[idx].y = sub->_subVars->_nextVarObj->_value.intValue;
			++idx;
			sub = sub->_nextVarObj;
		}
	}

	sub = varFliers->getSubVarByName("flyIdlePath");

	if (sub) {
		_array1.resize(sub->getSubVarsCount());

		sub = sub->_subVars;

		uint idx = 0;
		while (sub) {
			FloaterArray1 &f = _array1[idx];

			f.val1 = sub->_subVars->_value.intValue;
			f.val2 = sub->_subVars->_nextVarObj->_value.intValue;

			++idx;
			sub = sub->_nextVarObj;
		}
	}
}

void Floaters::genFlies(Scene *sc, int x, int y, int priority, int flags) {
	StaticANIObject *ani = new StaticANIObject(g_nmi->accessScene(SC_COMMON)->getStaticANIObject1ById(ANI_FLY, -1));

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

	ani->_movement->setDynamicPhaseIndex(g_nmi->_rnd.getRandomNumber(nummoves - 1));

	_array2.push_back(FloaterArray2());
	FloaterArray2 &arr2 = _array2.back();
	arr2.ani = ani;
	arr2.val11 = 15.0;
	arr2.val3 = y;
	arr2.val5 = y;
	arr2.val2 = x;
	arr2.val4 = x;
	arr2.fflags = flags;
}

void Floaters::update() {
	for (uint i = 0; i < _array2.size(); ++i) {
		FloaterArray2 &a2 = _array2[i];
		if (_array2[i].val13 <= 0) {
			if (_array2[i].val4 != a2.val2 || a2.val5 != a2.val3) {
				if (_array2[i].val9 < 2.0)
					_array2[i].val9 = 2.0;

				int dy = a2.val3 - a2.val5;
				int dx = a2.val2 - a2.val4;
				double dst = sqrt((double)(dy * dy + dx * dx));
				double at = atan2((double)dy, (double)dx);
				int newX = (int)(cos(at) * a2.val9);
				int newY = (int)(sin(at) * a2.val9);

				if (dst < a2.val9) {
					newX = a2.val2 - a2.val4;
					newY = a2.val3 - a2.val5;
				}
				if (dst <= 30.0) {
					if (dst < 30.0) {
						a2.val9 = a2.val9 - a2.val9 * 0.5;

						if (a2.val9 < 2.0)
							a2.val9 = 2.0;
					}
				} else {
					a2.val9 = a2.val9 * 0.5 + a2.val9;

					if (a2.val9 > a2.val11)
						a2.val9 = a2.val11;
				}

				a2.val4 += newX;
				a2.val5 += newY;
				a2.ani->setOXY(newX + a2.ani->_ox, newY + a2.ani->_oy);

				if (a2.val4 == a2.val2 && a2.val5 == a2.val3) {
					a2.val9 = 0.0;

					a2.val13 = g_nmi->_rnd.getRandomNumber(200) + 20;

					if (a2.fflags & 1) {
						g_nmi->_currentScene->deleteStaticANIObject(a2.ani);

						if (a2.ani)
							delete a2.ani;

						_array2.remove_at(i);

						i--;

						if (!_array2.size())
							g_nmi->stopAllSoundInstances(SND_CMN_060);

						continue;
					}
				}
			} else {
				if ((a2.fflags & 4) && a2.countdown < 1) {
					a2.fflags |= 1;
					a2.val2 = a2.val6;
					a2.val3 = a2.val7;
				} else {
					if (a2.fflags & 2) {
						int idx1 = g_nmi->_rnd.getRandomNumber(_array1.size() - 1);

						a2.val2 = _array1[idx1].val1;
						a2.val3 = _array1[idx1].val2;
					} else {
						if (!_hRgn)
							error("Floaters::update(): empty fliers region");

						const Common::Rect rect = _hRgn->getBBox();

						int x2 = rect.left + g_nmi->_rnd.getRandomNumber(rect.right - rect.left);
						int y2 = rect.top + g_nmi->_rnd.getRandomNumber(rect.bottom - rect.top);

						if (_hRgn->pointInRegion(x2, y2)) {
							int dx = a2.val2 - x2;
							int dy = a2.val3 - y2;
							double dst = sqrt((double)(dy * dy + dx * dx));

							if (dst < 300.0 || !_hRgn->pointInRegion(a2.val4, a2.val5)) {
								a2.val2 = x2;
								a2.val3 = y2;
							}
						}
					}

					g_nmi->playSound(SND_CMN_061, 0);

					if (a2.fflags & 4)
						a2.countdown--;
				}
			}
		} else {
			a2.val13--;
		}

		if (!a2.ani->_movement && a2.ani->_statics->_staticsId == ST_FLY_FLY) {
			if (!a2.val15) {
				g_nmi->playSound(SND_CMN_060, 1);

				a2.val15 = 1;
			}

			a2.ani->startAnim(MV_FLY_FLY, 0, -1);
		}
	}
}

void Floaters::stopAll() {
	for (uint i = 0; i < _array2.size(); i++) {
		FloaterArray2 &a2 = _array2[i];
		g_nmi->_currentScene->deleteStaticANIObject(a2.ani);
		delete a2.ani;
	}

	_array2.clear();

	g_nmi->stopAllSoundInstances(SND_CMN_060);
}


} // End of namespace NGI
