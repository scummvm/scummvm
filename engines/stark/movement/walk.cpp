/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/movement/walk.h"

#include "engines/stark/movement/shortestpath.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

#include "engines/stark/resources/item.h"
#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/floorface.h"

namespace Stark {

Walk::Walk(Resources::FloorPositionedItem *item) :
	Movement(item) {
}

Walk::~Walk() {
}

void Walk::start() {
	Movement::start();

	Resources::Floor *floor = StarkGlobal->getCurrent()->getFloor();

	Math::Vector3d startPosition = _item->getPosition3D();
	int32 startFloorFaceIndex = _item->getFloorFaceIndex();
	Resources::FloorFace *startFloorFace = floor->getFace(startFloorFaceIndex);
	Resources::FloorEdge *startFloorEdge = startFloorFace->findNearestEdge(startPosition);


	int32 destinationFloorFaceIndex = floor->findFaceContainingPoint(_destination);
	Resources::FloorFace *destinationFloorFace = floor->getFace(destinationFloorFaceIndex);
	Resources::FloorEdge *destinationFloorEdge = destinationFloorFace->findNearestEdge(_destination);

	ShortestPath pathSearch;
	ShortestPath::NodeList edgePath = pathSearch.search(startFloorEdge, destinationFloorEdge);

	Common::Debug debug = streamDbg();
	debug << "start: " << startPosition << "\n";

	for (ShortestPath::NodeList::const_iterator it = edgePath.begin(); it != edgePath.end(); it++) {
		debug << "step: " << (*it)->getPosition() << "\n";
	}

	debug << "destination: " << _destination << "\n";
}

void Walk::onGameLoop() {
	Resources::Floor *floor = StarkGlobal->getCurrent()->getFloor();

	// TODO: Complete, for now we just teleport to the target location
	_item->setPosition3D(_destination);

	int32 destinationFloorFaceIndex = floor->findFaceContainingPoint(_destination);
	_item->setFloorFaceIndex(destinationFloorFaceIndex);

	_ended = true;
}

void Walk::setDestination(const Math::Vector3d &destination) {
	_destination = destination;
}

} // End of namespace Stark