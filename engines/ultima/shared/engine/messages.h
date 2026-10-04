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

#ifndef ULTIMA_SHARED_ENGINE_MESSAGES_H
#define ULTIMA_SHARED_ENGINE_MESSAGES_H

#include "graphics/views/messages.h"

namespace Ultima {
namespace Shared {

/**
 * Sent to a map view to have it briefly overlay a tile directly on-screen
 * at a given map position - e.g. a projectile flying towards the player,
 * or a hit flash - before restoring the view back to normal.
 */
struct AttackTileMessage : public Graphics::Views::Message {
	int _x, _y;
	int _tileId;

	AttackTileMessage() : Message(), _x(0), _y(0), _tileId(0) {
	}
	AttackTileMessage(int x, int y, int tileId) : Message(),
		_x(x), _y(y), _tileId(tileId) {
	}
};

} // namespace Shared
} // namespace Ultima

#endif
