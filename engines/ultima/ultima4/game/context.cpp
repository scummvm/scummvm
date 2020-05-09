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

#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/views/stats.h"

namespace Ultima {
namespace Ultima4 {

Context *g_context;

Context::Context() : _stats(nullptr), _aura(nullptr) {
	g_context = this;
	reset();
}

Context::~Context() {
	g_context = nullptr;
	delete _stats;
	delete _aura;
}

void Context::reset() {
	delete _stats;
	delete _aura;
	_stats = nullptr;
	_aura = nullptr;

	_party = nullptr;
	_location = nullptr;
	_lastShip = nullptr;
	_line = _col = 0;
	_moonPhase = 0;
	_windDirection = 0;
	_windCounter = 0;
	_windLock = false;
	_horseSpeed = 0;
	_opacity = 0;
	_lastCommandTime = 0;
	_transportContext = TRANSPORT_ANY;
}

} // End of namespace Ultima4
} // End of namespace Ultima
