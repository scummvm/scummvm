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

void actorDoAction(int action) {
	if (action > hLAST)
		error("error in actorDoAction, invalid action (should be called as an animation)");

	_curStep = 1;
	float px = g_vm->_actor->_px + g_vm->_actor->_dx;
	float pz = g_vm->_actor->_pz + g_vm->_actor->_dz;
	float theta = g_vm->_actor->_theta;
	int b = 0;

	_step[b]._px = px;
	_step[b]._pz = pz;
	_step[b]._dx = 0.0f;
	_step[b]._dz = 0.0f;

	_step[b]._theta     = theta;
	_step[b]._curAction = hSTAND;
	_step[b]._curFrame  = 0;
	_step[b]._curPanel  = _curPanel;

	float t = ((270.0f - theta) * PI2) / 360.0f;
	float ox = cos(t);
	float oz = sin(t);

	SVertex *v = g_vm->_actor->_characterArea;
	float firstFrame = FRAMECENTER(v);

	int len;
	int cfp = 0;
	int cur = 0;

	while (cur < action)
		cfp += _defActionLen[cur++];
	v = &g_vm->_actor->_characterArea[cfp * g_vm->_actor->_vertexNum];

	if (action == hWALKOUT)
		v = &g_vm->_actor->_characterArea[g_vm->_actor->_vertexNum];
	else if (action == hLAST)
		v = g_vm->_actor->_characterArea;

	len = _defActionLen[action];

	for (b = _curStep; b < len + _curStep; b++) {
		float curLen = FRAMECENTER(v) - firstFrame;

		_step[b]._dx = curLen * ox;
		_step[b]._dz = curLen * oz;
		_step[b]._px = px;
		_step[b]._pz = pz;

		_step[b]._curAction = action;
		_step[b]._curFrame  = b - _curStep;

		_step[b]._theta    = theta;
		_step[b]._curPanel = _curPanel;

		v += g_vm->_actor->_vertexNum;

		if (action == hLAST)
			v = g_vm->_actor->_characterArea;
	}

	_step[b]._px = px;
	_step[b]._pz = pz;
	_step[b]._dx = 0.0;
	_step[b]._dz = 0.0;

	_step[b]._theta     = theta;
	_step[b]._curAction = hSTAND;
	_step[b]._curFrame  = 0;
	_step[b]._curPanel  = _curPanel;

	_lastStep = b;		// Last step

	// Starts action
	if (g_vm->_obj[g_vm->_curObj]._flag & kObjFlagRoomOut)
		doEvent(MC_CHARACTER, ME_CHARACTERGOTOEXIT, MP_DEFAULT, g_vm->_obj[g_vm->_curObj]._goRoom, 0, g_vm->_obj[g_vm->_curObj]._ninv, g_vm->_curObj);
	else
		doEvent(MC_CHARACTER, ME_CHARACTERDOACTION, MP_DEFAULT, 0, 0, 0, 0);
}

void actorStop() {
	int b = 0;

	_step[b]._px = g_vm->_actor->_px + g_vm->_actor->_dx;
	_step[b]._pz = g_vm->_actor->_pz + g_vm->_actor->_dz;
	_step[b]._dx = 0.0;
	_step[b]._dz = 0.0;

	_step[b]._theta = g_vm->_actor->_theta;
	_step[b]._curAction = hSTAND;
	_step[b]._curFrame  = 0;
	_step[b]._curPanel  = _curPanel;

	_characterGoToPosition = -1;

	_curStep = 0;
	_lastStep = 0;
}

void setPosition(int num) {
	SLight *_curLight = g_vm->_actor->_light;

	for (int a = 0; a < g_vm->_actor->_lightNum; a++) {
		// If it's off
		if (_curLight->_inten == 0) {
			// If it's the required position
			if (_curLight->_position == num) {
				g_vm->_actor->_px = _curLight->_x;
				g_vm->_actor->_pz = _curLight->_z;
				g_vm->_actor->_dx = 0.0;
				g_vm->_actor->_dz = 0.0;

				float ox = _curLight->_dx;
				float oz = _curLight->_dz;

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

		_curLight++;
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
