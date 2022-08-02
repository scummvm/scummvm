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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/puzzles/puzzle.h"

#include "asylum/resources/polygons.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

Puzzle::Puzzle(AsylumEngine *engine): _vm(engine) {
}

void Puzzle::saveLoadWithSerializer(Common::Serializer &) {
	// By default, we do not save any data
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool Puzzle::handleEvent(const AsylumEvent &evt) {
	switch ((int32)evt.type) {
	default:
		break;

	case EVENT_ASYLUM_INIT:
		return init(evt);

	case EVENT_ASYLUM_ACTIVATE:
		return activate(evt);

	case EVENT_ASYLUM_UPDATE:
		return update(evt);

	case Common::EVENT_KEYDOWN:
		return key(evt);

	case Common::EVENT_LBUTTONDOWN:
		return mouseLeftDown(evt);

	case Common::EVENT_LBUTTONUP:
		return mouseLeftUp(evt);

	case Common::EVENT_RBUTTONDOWN:
		return mouseRightDown(evt);

	case Common::EVENT_RBUTTONUP:
		return mouseRightUp(evt);

	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		return keyExit(evt);
	}

	return false;
}

bool Puzzle::update(const AsylumEvent &evt) {
	uint32 ticks = _vm->getTick();

	updateCursor();

	if (!getSharedData()->getFlag(kFlagRedraw)) {
		updateScreen();

		getSharedData()->setFlag(kFlagRedraw, true);
	}

	if (ticks > getSharedData()->getNextScreenUpdate()) {
		if (getSharedData()->getFlag(kFlagRedraw)) {
			if (!getScreen()->isGraphicQueueEmpty())
				getScreen()->drawGraphicsInQueue();
			getScreen()->copyBackBufferToScreen();

			getSharedData()->setEventUpdate(getSharedData()->getEventUpdate() ^ 1);

			getSharedData()->setFlag(kFlagRedraw, false);
			getSharedData()->setNextScreenUpdate(ticks + 40);
		}
	}

	return true;
}

bool Puzzle::keyExit(const AsylumEvent &evt) {
	_vm->switchEventHandler(getScene());

	return true;
}


void Puzzle::exitPuzzle() {
	getScreen()->clear();

	_vm->switchEventHandler(getScene());
}

//////////////////////////////////////////////////////////////////////////
// Hit test functions
//////////////////////////////////////////////////////////////////////////

bool Puzzle::hitTest(const int16 (*polygonPoint)[2], const Common::Point &point, uint32 index) const {
	Common::Point p1(polygonPoint[index + 0][0], polygonPoint[index + 0][1]),
				  p2(polygonPoint[index + 1][0], polygonPoint[index + 1][1]),
				  p3(polygonPoint[index + 2][0], polygonPoint[index + 2][1]),
				  p4(polygonPoint[index + 3][0], polygonPoint[index + 3][1]);

	Polygon polygon(p1, p2, p3, p4);

	return polygon.contains(point);
}

bool Puzzle::hitTest(const int16 (*polygonPoint)[2], const Common::Point &point) const {
	return hitTest(polygonPoint, point, 0);
}

} // end of namespace Asylum
