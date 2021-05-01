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

#include "trecision/actor.h"

#include "common/scummsys.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/extern.h"
#include "trecision/defines.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/trecision.h"

namespace Trecision {

void setPosition(int num) {
	SLight *curLight = g_vm->_actor->_light;

	for (int a = 0; a < g_vm->_actor->_lightNum; a++) {
		// If it's off
		if (curLight->_inten == 0) {
			// If it's the required position
			if (curLight->_position == num) {
				g_vm->_actor->_px = curLight->_x;
				g_vm->_actor->_pz = curLight->_z;
				g_vm->_actor->_dx = 0.0;
				g_vm->_actor->_dz = 0.0;

				float ox = curLight->_dx;
				float oz = curLight->_dz;

				// If it's a null light
				if ((ox == 0.0) && (oz == 0.0))
					warning("setPosition: Unknown error : null light");

				float t = sqrt(ox * ox + oz * oz);
				ox /= t;
				oz /= t;

				float theta = g_vm->sinCosAngle(ox, oz) * 180.0f / PI;
				if (theta >= 360.0)
					theta -= 360.0;
				if (theta < 0.0)
					theta += 360.0;

				g_vm->_actor->_theta = theta;

				_curStep = 0;
				_lastStep = 0;
				_curPanel = -1;
				_oldPanel = -1;

				_step[0]._px = g_vm->_actor->_px + g_vm->_actor->_dx;
				_step[0]._pz = g_vm->_actor->_pz + g_vm->_actor->_dz;
				_step[0]._dx = 0.0;
				_step[0]._dz = 0.0;

				_step[0]._theta = g_vm->_actor->_theta;
				_step[0]._curAction = hSTAND;
				_step[0]._curFrame  = 0;
				_step[0]._curPanel  = _curPanel;

				_characterGoToPosition = num;
				return;
			}
		}

		curLight++;
	}
}

void goToPosition(int num) {
	extern float _lookX, _lookZ;

	SLight *_curLight = g_vm->_actor->_light;

	for (int a = 0; a < g_vm->_actor->_lightNum; a++) {
		// If it's off and if it's a position
		if (_curLight->_inten == 0) {
			// If it's the right position
			if (_curLight->_position == num) {
				_curX = _curLight->_x;
				_curZ = _curLight->_z;
				_lookX = _curX - _curLight->_dx;
				_lookZ = _curZ - _curLight->_dz;

				_curStep  = 0;
				_lastStep = 0;

				_step[0]._px = g_vm->_actor->_px + g_vm->_actor->_dx;
				_step[0]._pz = g_vm->_actor->_pz + g_vm->_actor->_dz;
				_step[0]._dx = 0.0;
				_step[0]._dz = 0.0;

				_step[0]._theta = g_vm->_actor->_theta;
				_step[0]._curAction = hSTAND;
				_step[0]._curFrame  = 0;
				_step[0]._curPanel  = _curPanel;

				_oldPanel = _curPanel;
				_curPanel = -1;

				findPath();

				_characterGoToPosition = num;
				break;
			}
		}

		_curLight++;
	}
}

void lookAt(float x, float z) {
	float ox = _step[_lastStep]._px - x;
	float oz = _step[_lastStep]._pz - z;

	// If the light is null
	if ((ox == 0.0) && (oz == 0.0)) {
		memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
		memcpy(&_step[_lastStep + 2], &_step[_lastStep + 1], sizeof(SStep));
		_lastStep += 2;

		return;
	}

	float t = sqrt(ox * ox + oz * oz);
	ox /= t;
	oz /= t;

	float theta = g_vm->sinCosAngle(ox, oz) * 180.0f / PI;
	if (theta >= 360.0f)
		theta -= 360.0f;
	if (theta <  0.0f)
		theta += 360.0f;

	float approx = theta - _step[_lastStep]._theta;

	if ((approx < 30.0f) && (approx > -30.0f))
		approx = 0.0f;
	else if (approx > 180.0f)
		approx = -360.0f + approx;
	else if (approx < -180.0)
		approx = 360.0f + approx;

	approx /= 3.0;

	// Antepenultimate 1/3
	_step[_lastStep]._theta += approx;
	_step[_lastStep]._theta = (_step[_lastStep]._theta > 360.0) ? _step[_lastStep]._theta - 360.0 : (_step[_lastStep]._theta < 0.0) ? _step[_lastStep]._theta + 360.0 : _step[_lastStep]._theta;

	// Penultimate 2/3
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
	_lastStep++;
	_step[_lastStep]._theta += approx;
	_step[_lastStep]._theta = (_step[_lastStep]._theta > 360.0) ? _step[_lastStep]._theta - 360.0 : (_step[_lastStep]._theta < 0.0) ? _step[_lastStep]._theta + 360.0 : _step[_lastStep]._theta;

	// Last right step
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
	_lastStep++;
	_step[_lastStep]._theta = theta;

	//	????
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(SStep));
	_lastStep++;
	_step[_lastStep]._theta = theta;
}

} // End of namespace Trecision
