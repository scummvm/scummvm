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

#ifndef FREESCAPE_CONNECTIONS_H
#define FREESCAPE_CONNECTIONS_H

#include "freescape/objects/object.h"

namespace Freescape {

class AreaConnections : public Object {
public:
	Common::Array<byte> _connections;
	AreaConnections(const Common::Array<byte> connections_) {
		_objectID = 254;
		_connections = connections_;
	}

	ObjectType getType() override { return ObjectType::kEntranceType; };
	void draw(Freescape::Renderer *gfx) override { error("cannot render AreaConnections"); };
	void scale(int factor) override { warning("cannot scale AreaConnections"); };
	Object *duplicate() override { error("cannot duplicate AreaConnections"); };
};

} // End of namespace Freescape

#endif // FREESCAPE_CONNECTIONS_H