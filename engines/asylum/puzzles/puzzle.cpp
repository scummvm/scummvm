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
	uint32 counter = 0;

	for (uint32 i = index; i < index + 3; i++)
		if (Polygons::contains(point.x, point.y,
		                       polygonPoint[index + 1].x + point.x + 100, point.y,
		                       polygonPoint[i].x, polygonPoint[i].y,
		                       polygonPoint[i + 1].x, polygonPoint[i + 1].y))
			++counter;

	if (Polygons::contains(point.x, point.y,
	                       polygonPoint[index + 1].x + point.x + 100, point.y,
	                       polygonPoint[index].x, polygonPoint[index].y,
	                       polygonPoint[index + 3].x, polygonPoint[index + 3].y))
		++counter;

	return (counter & 1);
}

bool Puzzle::hitTest(const Common::Point *polygonPoint, Common::Point point) {
	uint32 counter = 0;

	for (uint32 i = 3; i > 1; i--)
		if (Polygons::contains(point.x, point.y,
		                       polygonPoint[1].x + point.x + 700, point.y,
		                       polygonPoint[i].x, polygonPoint[i].y,
		                       polygonPoint[i + 1].x, polygonPoint[i + 1].y))
			++counter;

	if (Polygons::contains(point.x, point.y,
	                       polygonPoint[0].x + point.x + 100, point.y,
	                       polygonPoint[0].x, polygonPoint[0].y,
	                       polygonPoint[3].x, polygonPoint[3].y))
		++counter;

	return (counter & 1);
}

} // end of namespace Asylum
