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

#include "ultima/ultima4/game/aura.h"

namespace Ultima {
namespace Ultima4 {

Aura::Aura() : _type(NONE), _duration(0) {}

void Aura::setDuration(int d) {
	_duration = d;
	setChanged();
	notifyObservers(nullptr);
}

void Aura::set(Type t, int d) {
	_type = t;
	_duration = d;
	setChanged();
	notifyObservers(nullptr);
}

void Aura::setType(Type t) {
	_type = t;
	setChanged();
	notifyObservers(nullptr);
}

void Aura::passTurn() {
	if (_duration > 0) {
		_duration--;

		if (_duration == 0) {
			_type = NONE;

			setChanged();
			notifyObservers(nullptr);
		}
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
