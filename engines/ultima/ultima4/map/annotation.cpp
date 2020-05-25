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

#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/core/settings.h"
#include "common/debug.h"

namespace Ultima {
namespace Ultima4 {

Annotation::Annotation(const Coords &pos, MapTile t, bool v, bool coverUp) :
	_coords(pos),
	_tile(t),
	_visual(v),
	_ttl(-1),
	_coverUp(coverUp) {
}

void Annotation::debug_output() const {
	debug(1, "x: %d\n", _coords.x);
	debug(1, "y: %d\n", _coords.y);
	debug(1, "z: %d\n", _coords.z);
	debug(1, "tile: %d\n", _tile.getId());
	debug(1, "visual: %s\n", _visual ? "Yes" : "No");
}

bool Annotation::operator==(const Annotation &a) const {
	return ((_coords == a.getCoords()) && (_tile == a._tile)) ? true : false;
}

/*-------------------------------------------------------------------*/

AnnotationMgr::AnnotationMgr() {}

Annotation *AnnotationMgr::add(Coords coords, MapTile tile, bool visual, bool isCoverUp) {
	// New annotations go to the front so they're handled "on top"
	_annotations.push_front(Annotation(coords, tile, visual, isCoverUp));
	return &_annotations.front();
}

Annotation::List AnnotationMgr::allAt(Coords coords) {
	Annotation::List list;

	for (_it = _annotations.begin(); _it != _annotations.end(); _it++) {
		if (_it->getCoords() == coords)
			list.push_back(*_it);
	}

	return list;
}

Common::List<Annotation *> AnnotationMgr::ptrsToAllAt(Coords coords) {
	Common::List<Annotation *> list;

	for (_it = _annotations.begin(); _it != _annotations.end(); _it++) {
		if (_it->getCoords() == coords)
			list.push_back(&(*_it));
	}

	return list;
}

void AnnotationMgr::clear() {
	_annotations.clear();
}

void AnnotationMgr::passTurn() {
	for (_it = _annotations.begin(); _it != _annotations.end(); _it++) {
		if (_it->getTTL() == 0) {
			_it = _annotations.erase(_it);
			if (_it == _annotations.end())
				break;
		} else if (_it->getTTL() > 0) {
			_it->passTurn();
		}
	}
}

void AnnotationMgr::remove(Coords coords, MapTile tile) {
	Annotation look_for(coords, tile);
	remove(look_for);
}

void AnnotationMgr::remove(Annotation &a) {
	for (_it = _annotations.begin(); _it != _annotations.end(); _it++) {
		if (*_it == a) {
			_it = _annotations.erase(_it);
			break;
		}
	}
}

void AnnotationMgr::remove(Annotation::List l) {
	Annotation::List::iterator it;
	for (it = l.begin(); it != l.end(); it++) {
		remove(*it);
	}
}

int AnnotationMgr::size() const {
	return _annotations.size();
}

} // End of namespace Ultima4
} // End of namespace Ultima
