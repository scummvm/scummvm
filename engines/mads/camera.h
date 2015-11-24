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

#ifndef MADS_CAMERA_H
#define MADS_CAMERA_H

#include "mads/scene.h"
#include "mads/player.h"
#include "mads/camera.h"

namespace MADS {

class MADSEngine;

class Camera {
private:
	MADSEngine *_vm;

public:
	bool _panAllowedFl;
	bool _activeFl;
	bool _currentFrameFl;
	bool _manualFl;

	int _speed;
	int _rate;
	int _target;
	int _distOffCenter;
	int _startTolerance;
	int _endTolerance;
	int _direction;
	uint32 _timer;

	Camera(MADSEngine *vm);

	void camPanTo(int target);
	bool camPan(int16 *picture_view, int16 *player_loc, int display_size, int picture_size);
	void setDefaultPanX();
	void setDefaultPanY();
};

} // End of namespace MADS

#endif /* MADS_CAMERA_H */
