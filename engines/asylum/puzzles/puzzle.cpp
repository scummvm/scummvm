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
 * $URL$
 * $Id$
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

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool Puzzle::handleEvent(const AsylumEvent &evt) {
	switch ((uint32)evt.type) {
	default:
		break;

	case EVENT_ASYLUM_INIT:
		return init();
		break;

	case EVENT_ASYLUM_ACTIVATE:
		return activate();
		break;

	case EVENT_ASYLUM_UPDATE:
		return update();
		break;

	case Common::EVENT_KEYDOWN:
		return key(evt);
		break;

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		return mouse(evt);
		break;
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

void Puzzle::exit() {
	getScreen()->clear();

	_vm->switchEventHandler(getScene());
}

//////////////////////////////////////////////////////////////////////////
// Hit test functions
//////////////////////////////////////////////////////////////////////////
bool Puzzle::hitTest(const Common::Point *polygonPoint, Common::Point point, uint32 index) {
	PolyDefinitions polygon(polygonPoint[index], polygonPoint[index + 1], polygonPoint[index + 2], polygonPoint[index + 3]);

	return polygon.contains(point);
}

bool Puzzle::hitTest(const Common::Point *polygonPoint, Common::Point point) {
	PolyDefinitions polygon(polygonPoint[0], polygonPoint[1], polygonPoint[2], polygonPoint[3]);

	return polygon.contains(point);
}

} // end of namespace Asylum
