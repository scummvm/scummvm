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

#include "mads/mads.h"
#include "mads/camera.h"

namespace MADS {

Camera::Camera(MADSEngine *vm) : _vm(vm) {
	_panAllowedFl = false;
	_activeFl = false;
	_currentFrameFl = false;
	_manualFl = false;
	_speed = -1;
	_rate = -1;
	_target = -1;
	_distOffCenter = -1;
	_startTolerance = -1;
	_endTolerance = -1;
	_direction = -1;
	_timer = 0;
}

void Camera::setDefaultPanX() {
	_activeFl = false;
	Scene &scene = _vm->_game->_scene;
	_panAllowedFl = (scene._sceneInfo->_width > MADS_SCREEN_WIDTH);

	if (_panAllowedFl) {
		_manualFl = false;
		_rate = 4;
		_speed = 4;
		_target = 0;
		_distOffCenter = 80;
		_startTolerance = 80;
		_endTolerance = 4;
		_timer = scene._frameStartTime;
	}
}

void Camera::setDefaultPanY() {
	_activeFl = false;
	Scene &scene = _vm->_game->_scene;
	_panAllowedFl = (scene._sceneInfo->_height > MADS_SCENE_HEIGHT);

	if (_panAllowedFl) {
		_manualFl = true;
		_rate = 4;
		_speed = 2;
		_target = 0;
		_distOffCenter = 80;
		_startTolerance = 60;
		_endTolerance = 4;
		_timer = scene._frameStartTime;
	}
}

void Camera::camPanTo(int target) {
	if (_panAllowedFl) {
		_activeFl = true;
		_manualFl = true;
		_target = target;
		_timer = _vm->_game->_scene._frameStartTime;
	}
}

bool Camera::camPan(int16 *picture_view, int16 *player_loc, int display_size, int picture_size) {
	bool panningFl = false;
	if (_panAllowedFl) {
		Scene &scene = _vm->_game->_scene;
		Player &player = _vm->_game->_player;

		_currentFrameFl = false;

		uint32 timer;
		if ((abs((int32) (_timer - player._priorTimer)) < _rate) && (player._ticksAmount == _rate))
			timer = player._priorTimer;
		else
			timer = _timer;

		if (_activeFl && (scene._frameStartTime < timer))
			return (panningFl);

		_timer = scene._frameStartTime + _rate;

		if (_manualFl) {
			if (_activeFl) {
				int diff = _target - *picture_view;
				int direction = 0;
				if (diff < 0)
					direction = -1;
				else if (diff > 0)
					direction = 1;

				int magnitude = MIN(abs(diff), _speed);

				if (magnitude == 0)
					_activeFl = false;
				else {
					int panAmount;
					if (direction < 0)
						panAmount = -magnitude;
					else
						panAmount = magnitude;

					*picture_view += panAmount;

					panningFl = true;
					_currentFrameFl = true;
				}
			}
		} else {
			if (!_activeFl) {
				int lowEdge = *picture_view + _startTolerance;
				int highEdge = *picture_view - _startTolerance + display_size - 1;

				if ((*player_loc < lowEdge) && (*picture_view > 0)) {
					_activeFl = true;
					_direction = -1;
				}

				if ((*player_loc > highEdge) && (*picture_view < (picture_size - display_size))) {
					_activeFl = true;
					_direction = 1;
				}
			}

			int newTarget = *player_loc - (display_size >> 1);

			if (_direction < 0)
				newTarget -= _distOffCenter;
			else
				newTarget += _distOffCenter;

			newTarget = MAX(0, newTarget);
			newTarget = MIN(newTarget, (picture_size - display_size));

			_target = newTarget;

			int diff = newTarget - *picture_view;
			int magnitude = abs(diff);

			int direction = 0;
			if (diff < 0)
				direction = -1;
			else if (diff > 0)
				direction = 1;

			if (_activeFl && (magnitude <= _endTolerance))
				_activeFl = false;

			if (_activeFl) {
				magnitude = MIN(magnitude, _speed);

				int panAmount;
				if (direction < 0)
					panAmount = -magnitude;
				else
					panAmount = magnitude;

				if (panAmount) {
					*picture_view += panAmount;
					panningFl = true;
					_currentFrameFl = true;
				}
			}
		}
	}

	return (panningFl);
}

} // End of namespace MADS
