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

void FlyPack::init(GameVar *var) {
	_idlePoints.clear();
	_flies.clear();

	GameVar *varFliers = var->getSubVarByName(sO_Fliers);

	if (!varFliers)
		return;

	GameVar *sub = varFliers->getSubVarByName("flyIdleRegion");

	if (sub) {
		_idleRegion.reset(new ReactPolygonal());

		_idleRegion->_points.resize(sub->getSubVarsCount());

		sub = sub->_subVars;
		uint idx = 0;
		while (sub) {
			_idleRegion->_points[idx].x = sub->_subVars->_value.intValue;
			_idleRegion->_points[idx].y = sub->_subVars->_nextVarObj->_value.intValue;
			++idx;
			sub = sub->_nextVarObj;
		}
	}

	sub = varFliers->getSubVarByName("flyIdlePath");

	if (sub) {
		_idlePoints.resize(sub->getSubVarsCount());

		sub = sub->_subVars;

		uint idx = 0;
		while (sub) {
			FloaterArray1 &f = _idlePoints[idx];

			f.val1 = sub->_subVars->_value.intValue;
			f.val2 = sub->_subVars->_nextVarObj->_value.intValue;

			++idx;
			sub = sub->_nextVarObj;
		}
	}
}

void FlyPack::addFly(Scene *sc, int x, int y, int priority, int flags) {
	AniObject *ani = new AniObject(g_nmi->findScene(SC_COMMON)->findAniObjectById(ANI_FLY, -1));

	ani->_statics = ani->getStatic(ST_FLY_FLY);
	ani->_movement = nullptr;
	ani->setPos(x, y);
	ani->_flags |= 4;
	ani->_z = priority;

	sc->addAni(ani, 1);

	ani->startMovement(MV_FLY_FLY, 0, -1);

	int nummoves;

	if (ani->_movement->_currMovement)
		nummoves = ani->_movement->_currMovement->_phases.size();
	else
		nummoves = ani->_movement->_phases.size();

	ani->_movement->setCurFrame(g_nmi->_rnd.getRandomNumber(nummoves - 1));

	_flies.push_back(FloaterArray2());
	FloaterArray2 &arr2 = _flies.back();
	arr2.ani = ani;
	arr2.val11 = 15.0;
	arr2.val3 = y;
	arr2.val5 = y;
	arr2.val2 = x;
	arr2.val4 = x;
	arr2.fflags = flags;
}

void FlyPack::onUpdate() {
	for (uint i = 0; i < _flies.size(); ++i) {
		FloaterArray2 &a2 = _flies[i];
		if (_flies[i].val13 <= 0) {
			if (_flies[i].val4 != a2.val2 || a2.val5 != a2.val3) {
				if (_flies[i].val9 < 2.0)
					_flies[i].val9 = 2.0;

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
				a2.ani->setPos(newX + a2.ani->_x, newY + a2.ani->_y);

				if (a2.val4 == a2.val2 && a2.val5 == a2.val3) {
					a2.val9 = 0.0;

					a2.val13 = g_nmi->_rnd.getRandomNumber(200) + 20;

					if (a2.fflags & 1) {
						g_nmi->_currentScene->removeAni(a2.ani);

						if (a2.ani)
							delete a2.ani;

						_flies.remove_at(i);

						i--;

						if (!_flies.size())
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
						int idx1 = g_nmi->_rnd.getRandomNumber(_idlePoints.size() - 1);

						a2.val2 = _idlePoints[idx1].val1;
						a2.val3 = _idlePoints[idx1].val2;
					} else {
						if (!_idleRegion)
							error("FlyPack::onUpdate(): empty fliers region");

						const Common::Rect rect = _idleRegion->getBBox();

						int x2 = rect.left + g_nmi->_rnd.getRandomNumber(rect.right - rect.left);
						int y2 = rect.top + g_nmi->_rnd.getRandomNumber(rect.bottom - rect.top);

						if (_idleRegion->hitTest(x2, y2)) {
							int dx = a2.val2 - x2;
							int dy = a2.val3 - y2;
							double dst = sqrt((double)(dy * dy + dx * dx));

							if (dst < 300.0 || !_idleRegion->hitTest(a2.val4, a2.val5)) {
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

			a2.ani->startMovement(MV_FLY_FLY, 0, -1);
		}
	}
}

void FlyPack::deleteAllFlies() {
	for (uint i = 0; i < _flies.size(); i++) {
		FloaterArray2 &a2 = _flies[i];
		g_nmi->_currentScene->removeAni(a2.ani);
		delete a2.ani;
	}

	_flies.clear();

	g_nmi->stopAllSoundInstances(SND_CMN_060);
}


} // End of namespace NGI
