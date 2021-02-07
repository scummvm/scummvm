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

#include <string.h>
#include <math.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/trecision.h"

namespace Trecision {

/*------------------------------------------------
					Do Action
--------------------------------------------------*/
void actorDoAction(int whatAction) {
	int len;
	extern uint8 _actionLen[];
	extern uint8 *_actionPointer[];
	extern uint16 _actionPosition[];

	_curStep = 1;
	float px = _actor._px + _actor._dx;
	float pz = _actor._pz + _actor._dz;
	float theta = _actor._theta;
	int b = 0;

	_step[b]._px = px;
	_step[b]._pz = pz;
	_step[b]._dx = 0.0;
	_step[b]._dz = 0.0;

	_step[b]._theta     = theta;
	_step[b]._curAction = hSTAND;
	_step[b]._curFrame  = 0;
	_step[b]._curPanel  = _curPanel;

	float t = ((270.0 - theta) * PI2) / 360.0;
	float ox = cos(t);
	float oz = sin(t);

	struct SVertex *v = (struct SVertex *)_characterArea;
	float firstFrame = FRAMECENTER(v);

	if (whatAction > hLAST) {
		v = (struct SVertex *)(_actionPointer[_actionPosition[actionInRoom(whatAction)]]);
		len = _actionLen[whatAction];
	} else {
		int cfp = 0;
		int cur = 0;
		while (cur < whatAction)
			cfp += _defActionLen[cur++];
		v = (struct SVertex *)_characterArea + cfp * _actor._vertexNum;

		if (whatAction == hWALKOUT)
			v = (struct SVertex *)_characterArea + _actor._vertexNum;
		else if (whatAction == hLAST)
			v = (struct SVertex *)_characterArea;

		len = _defActionLen[whatAction];
	}

	for (b = _curStep; b < len + _curStep; b++) {
		float curLen = FRAMECENTER(v) - firstFrame;

		_step[b]._dx = curLen * ox;
		_step[b]._dz = curLen * oz;
		_step[b]._px = px;
		_step[b]._pz = pz;

		_step[b]._curAction = whatAction;
		_step[b]._curFrame  = b - _curStep;

		_step[b]._theta    = theta;
		_step[b]._curPanel = _curPanel;

		v += _actor._vertexNum;

		if (whatAction > hLAST)
			v = (struct SVertex *)((uint8 *)v + 4);
		else if (whatAction == hLAST)
			v = (struct SVertex *)_characterArea;
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
	if (_obj[g_vm->_curObj]._flag & OBJFLAG_ROOMOUT)
		doEvent(MC_HOMO, ME_HOMOGOTOEXIT, MP_DEFAULT, _obj[g_vm->_curObj]._goRoom, 0, _obj[g_vm->_curObj]._ninv, g_vm->_curObj);
	else
		doEvent(MC_HOMO, ME_HOMODOACTION, MP_DEFAULT, 0, 0, 0, 0);
}

/*------------------------------------------------
				Stop character
--------------------------------------------------*/
void actorStop() {
	int b = 0;

	_step[b]._px = _actor._px + _actor._dx;
	_step[b]._pz = _actor._pz + _actor._dz;
	_step[b]._dx = 0.0;
	_step[b]._dz = 0.0;

	_step[b]._theta     = _actor._theta;
	_step[b]._curAction = hSTAND;
	_step[b]._curFrame  = 0;
	_step[b]._curPanel  = _curPanel;

	_characterGoToPosition = -1;

	_curStep = 0;
	_lastStep = 0;
}

/*------------------------------------------------
				Set Light Position
--------------------------------------------------*/
void setPosition(int num) {
	extern struct SLight *_light;

	_light = (struct SLight *)_actor._light;

	for (int a = 0; a < _actor._lightNum; a++) {
		// If it's off
		if (_light->_inten == 0) {
			// If it's the required position
			if (_light->_position == num) {
				_actor._px = _light->_x;
				_actor._pz = _light->_z;
				_actor._dx = 0.0;
				_actor._dz = 0.0;

				float ox = _light->_dx;
				float oz = _light->_dz;

				// If it's a null light
				if ((ox == 0.0) && (oz == 0.0))
					warning("setPosition: Unknown error : null light");

				float t = sqrt(ox * ox + oz * oz);
				ox /= t;
				oz /= t;

				float theta = sinCosAngle(ox, oz) * 180.0 / PI;
				if (theta >= 360.0)
					theta -= 360.0;
				if (theta < 0.0)
					theta += 360.0;

				_actor._theta = theta;

				_curStep = 0;
				_lastStep = 0;
				_curPanel = -1;
				_oldPanel = -1;

				_step[0]._px = _actor._px + _actor._dx;
				_step[0]._pz = _actor._pz + _actor._dz;
				_step[0]._dx = 0.0;
				_step[0]._dz = 0.0;

				_step[0]._theta     = _actor._theta;
				_step[0]._curAction = hSTAND;
				_step[0]._curFrame  = 0;
				_step[0]._curPanel  = _curPanel;

				_characterGoToPosition = num;
				return ;
			}
		}

		_light ++;
	}
}

/*------------------------------------------------
				Go To Light Position
--------------------------------------------------*/
void goToPosition(int num) {
	extern struct SLight *_light;
	extern float _lookX, _lookZ;

	_light = (struct SLight *)_actor._light;

	for (int a = 0; a < _actor._lightNum; a++) {
		// If it's off and if it's a position
		if (_light->_inten == 0) {
			// If it's the right position
			if (_light->_position == num) {
				_curX = _light->_x;
				_curZ = _light->_z;
				_lookX = _curX - _light->_dx;
				_lookZ = _curZ - _light->_dz;

				_curStep  = 0;
				_lastStep = 0;

				_step[0]._px = _actor._px + _actor._dx;
				_step[0]._pz = _actor._pz + _actor._dz;
				_step[0]._dx = 0.0;
				_step[0]._dz = 0.0;

				_step[0]._theta     = _actor._theta;
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

		_light ++;
	}
}

/*------------------------------------------------
					lookAt
--------------------------------------------------*/
void lookAt(float x, float z) {
	float ox = _step[_lastStep]._px - x;
	float oz = _step[_lastStep]._pz - z;

	// If the light is null
	if ((ox == 0.0) && (oz == 0.0)) {
		memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(struct SStep));
		memcpy(&_step[_lastStep + 2], &_step[_lastStep + 1], sizeof(struct SStep));
		_lastStep += 2;

		return ;
	}

	float t = sqrt(ox * ox + oz * oz);
	ox /= t;
	oz /= t;

	float theta = sinCosAngle(ox, oz) * 180.0 / PI;
	if (theta >= 360.0)
		theta -= 360.0;
	if (theta <  0.0)
		theta += 360.0;

	float approx = theta - _step[_lastStep]._theta;

	if ((approx < 30.0) && (approx > -30.0))
		approx = 0.0;
	else if (approx > 180.0)
		approx = -360.0 + approx;
	else if (approx < -180.0)
		approx = 360.0 + approx;

	approx /= 3.0;

	// Antepenultimate 1/3
	_step[_lastStep]._theta += approx;
	_step[_lastStep]._theta = (_step[_lastStep]._theta > 360.0) ? _step[_lastStep]._theta - 360.0 : (_step[_lastStep]._theta < 0.0) ? _step[_lastStep]._theta + 360.0 : _step[_lastStep]._theta;

	// Penultimate 2/3
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(struct SStep));
	_lastStep++;
	_step[_lastStep]._theta += approx;
	_step[_lastStep]._theta = (_step[_lastStep]._theta > 360.0) ? _step[_lastStep]._theta - 360.0 : (_step[_lastStep]._theta < 0.0) ? _step[_lastStep]._theta + 360.0 : _step[_lastStep]._theta;

	// Last right step
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(struct SStep));
	_lastStep++;
	_step[_lastStep]._theta = theta;

	//	????
	memcpy(&_step[_lastStep + 1], &_step[_lastStep], sizeof(struct SStep));
	_lastStep++;
	_step[_lastStep]._theta = theta;
}

} // End of namespace Trecision
