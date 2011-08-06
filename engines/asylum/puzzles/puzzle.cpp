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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

Puzzle::~Puzzle() {
	// Zero passed pointers
	_vm = NULL;
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
	}

	return false;
}

bool Puzzle::key(const AsylumEvent &evt) {
	switch (evt.kbd.keycode) {
	default:
		break;

	case Common::KEYCODE_TAB:
		getScreen()->takeScreenshot();
		break;
	}

	return true;
}

bool Puzzle::keyExit(const AsylumEvent &evt) {
	switch (evt.kbd.keycode) {
	default:
		_vm->switchEventHandler(getScene());
		break;

	case Common::KEYCODE_TAB:
		getScreen()->takeScreenshot();
		break;
	}

	return true;
}


void Puzzle::exitPuzzle() {
	getScreen()->clear();

	_vm->switchEventHandler(getScene());
}

//////////////////////////////////////////////////////////////////////////
// Hit test functions
//////////////////////////////////////////////////////////////////////////
bool Puzzle::hitTest(const Common::Point *polygonPoint, const Common::Point &point, uint32 index) const {
	Polygon polygon(polygonPoint[index], polygonPoint[index + 1], polygonPoint[index + 2], polygonPoint[index + 3]);

	return polygon.contains(point);
}

bool Puzzle::hitTest(const Common::Point *polygonPoint, const Common::Point &point) const {
	Polygon polygon(polygonPoint[0], polygonPoint[1], polygonPoint[2], polygonPoint[3]);

	return polygon.contains(point);
}

} // end of namespace Asylum
