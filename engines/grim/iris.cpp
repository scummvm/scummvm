/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/iris.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"

namespace Grim {

Iris::Iris() :
	_playing(false), _direction(Open) {

}

Iris::~Iris() {

}

void Iris::play(Iris::Direction dir, int x, int y, int lenght) {
	_playing = true;
	_direction = dir;
	_x = x;
	_y = y;
	_lenght = lenght;
	_currTime = 0;
}

void Iris::draw() {
	if (!_playing) {
		if (_direction == Close) {
			g_driver->dimScreen();
		}
		return;
	}

	_currTime += g_grim->getFrameTime();
	if (_currTime >= _lenght) {
		_playing = false;
		if (_direction == Close) {
			g_driver->dimScreen();
		}
		return;
	}

	float factor = (float)_currTime / (float)_lenght;
	if (_direction == Open) {
		factor = 1 - factor;
	}

	int y = (int)(_y * factor);
	int x = (int)(_x * factor);

	// Why doesn't 480 work here??
	g_driver->dimRegion(0, 0, 640, y, 0);
	g_driver->dimRegion(0, y, x, 479 - y, 0);
	g_driver->dimRegion(x, 479 - y, 640 - x, y, 0);
	g_driver->dimRegion(640 - x, y, x, 479 - y, 0);
}

void Iris::saveState(SaveGame *state) const {
	state->beginSection('IRIS');

	state->writeLEBool(_playing);
	state->writeLEUint32((uint32)_direction);
	state->writeLEUint32(_x);
	state->writeLEUint32(_y);
	state->writeLEUint32(_lenght);
	state->writeLEUint32(_currTime);

	state->endSection();
}

void Iris::restoreState(SaveGame *state) {
	state->beginSection('IRIS');

	_playing = state->readLEBool();
	_direction = (Direction)state->readLEUint32();
	_x = state->readLEUint32();
	_y = state->readLEUint32();
	_lenght = state->readLEUint32();
	_currTime = state->readLEUint32();

	state->endSection();
}

}
