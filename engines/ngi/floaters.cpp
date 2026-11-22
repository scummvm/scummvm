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
			f.targetX = sub->_subVars->_nextVarObj->_value.intValue;

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

	_flies.push_back(GutlFly());
	GutlFly &arr2 = _flies.back();
	arr2.ani = ani;
	arr2.maxSpeed = 15.0;
	arr2.targetY = y;
	arr2.y = y;
	arr2.targetX = x;
	arr2.x = x;
	arr2.flags = flags;
}

void FlyPack::onUpdate() {
	for (uint i = 0; i < _flies.size(); ++i) {
		GutlFly &a2 = _flies[i];
		if (_flies[i].idleTicks <= 0) {
			if (_flies[i].x != a2.targetX || a2.y != a2.targetY) {
				if (_flies[i].speed < 2.0)
					_flies[i].speed = 2.0;

				int dy = a2.targetY - a2.y;
				int dx = a2.targetX - a2.x;
				double dst = sqrt((double)(dy * dy + dx * dx));
				double at = atan2((double)dy, (double)dx);
				int newX = (int)(cos(at) * a2.speed);
				int newY = (int)(sin(at) * a2.speed);

				if (dst < a2.speed) {
					newX = a2.targetX - a2.x;
					newY = a2.targetY - a2.y;
				}
				if (dst <= 30.0) {
					if (dst < 30.0) {
						a2.speed = a2.speed - a2.speed * 0.5;

						if (a2.speed < 2.0)
							a2.speed = 2.0;
					}
				} else {
					a2.speed = a2.speed * 0.5 + a2.speed;

					if (a2.speed > a2.maxSpeed)
						a2.speed = a2.maxSpeed;
				}

				a2.x += newX;
				a2.y += newY;
				a2.ani->setPos(newX + a2.ani->_x, newY + a2.ani->_y);

				if (a2.x == a2.targetX && a2.y == a2.targetY) {
					a2.speed = 0.0;

					a2.idleTicks = g_nmi->_rnd.getRandomNumber(200) + 20;

					if (a2.flags & 1) {
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
				if ((a2.flags & 4) && a2.countdown < 1) {
					a2.flags |= 1;
					a2.targetX = a2.exitX;
					a2.targetY = a2.exitY;
				} else {
					if (a2.flags & 2) {
						int idx1 = g_nmi->_rnd.getRandomNumber(_idlePoints.size() - 1);

						a2.targetX = _idlePoints[idx1].val1;
						a2.targetY = _idlePoints[idx1].targetX;
					} else {
						if (!_idleRegion)
							error("FlyPack::onUpdate(): empty fliers region");

						const Common::Rect rect = _idleRegion->getBBox();

						int x2 = rect.left + g_nmi->_rnd.getRandomNumber(rect.right - rect.left);
						int y2 = rect.top + g_nmi->_rnd.getRandomNumber(rect.bottom - rect.top);

						if (_idleRegion->hitTest(x2, y2)) {
							int dx = a2.targetX - x2;
							int dy = a2.targetY - y2;
							double dst = sqrt((double)(dy * dy + dx * dx));

							if (dst < 300.0 || !_idleRegion->hitTest(a2.x, a2.y)) {
								a2.targetX = x2;
								a2.targetY = y2;
							}
						}
					}

					g_nmi->playSound(SND_CMN_061, 0);

					if (a2.flags & 4)
						a2.countdown--;
				}
			}
		} else {
			a2.idleTicks--;
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
		GutlFly &a2 = _flies[i];
		g_nmi->_currentScene->removeAni(a2.ani);
		delete a2.ani;
	}

	_flies.clear();

	g_nmi->stopAllSoundInstances(SND_CMN_060);
}


} // End of namespace NGI
