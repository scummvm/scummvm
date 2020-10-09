/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/iris.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/savegame.h"
#include "engines/grim/grim.h"

namespace Grim {

Iris::Iris() : _playing(false), _direction(Open), _x1(0), _y1(0), _x2(0), _y2(0),
		_targetX(0), _targetY(0), _length(0), _currTime(0) {

}

Iris::~Iris() {

}

void Iris::play(Iris::Direction dir, int x, int y, int length) {
	_playing = true;
	_direction = dir;
	_targetX = x;
	_targetY = y;
	_length = length;
	_currTime = 0;
}

void Iris::draw() {
	if (!_playing) {
		if (_direction == Close && g_grim->getMode() != GrimEngine::SmushMode) {
			g_driver->irisAroundRegion(320, 240, 320, 240);
		}
		return;
	}

	g_driver->irisAroundRegion(_x1, _y1, _x2, _y2);
}

void Iris::update(int frameTime) {
	if (!_playing) {
		return;
	}

	_currTime += frameTime;
	if (_currTime >= _length) {
		_playing = false;
		return;
	}

	float factor = (float)_currTime / (float)_length;
	if (_direction == Open) {
		factor = 1 - factor;
	}

	_y1 = (int)(_targetY * factor);
	_x1 = (int)(_targetX * factor);
	_y2 = (int)(480 - (480 - _targetY) * factor);
	_x2 = (int)(640 - (640 - _targetX) * factor);
}

void Iris::saveState(SaveGame *state) const {
	state->beginSection('IRIS');

	state->writeBool(_playing);
	state->writeLEUint32((uint32)_direction);
	state->writeLESint32(_x1);
	state->writeLESint32(_y1);
	state->writeLESint32(_x2);
	state->writeLESint32(_y2);
	state->writeLESint32(_length);
	state->writeLESint32(_currTime);

	state->endSection();
}

void Iris::restoreState(SaveGame *state) {
	state->beginSection('IRIS');

	_playing = state->readBool();
	_direction = (Direction)state->readLEUint32();
	_x1 = state->readLESint32();
	_y1 = state->readLESint32();
	_x2 = state->readLESint32();
	_y2 = state->readLESint32();
	_length = state->readLESint32();
	_currTime = state->readLESint32();

	state->endSection();
}

}
