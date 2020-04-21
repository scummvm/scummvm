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

#ifndef ULTIMA4_MAP_ANNOTATION_H
#define ULTIMA4_MAP_ANNOTATION_H

#include "ultima/ultima4/core/coords.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/map/map_tile.h"
#include "common/list.h"

namespace Ultima {
namespace Ultima4 {

class Annotation;

/**
 * Annotation are updates to a map.
 * There are three types of annotations:
 * - permanent: lasts until annotationClear is called
 * - turn based: lasts a given number of cycles
 * - time based: lasts a given number of time units (1/4 seconds)
 */
class Annotation {
public:
	typedef Common::List<Annotation> List;

	Annotation(const Coords &coords, MapTile tile, bool visual = false, bool coverUp = false);

	void debug_output() const;

	// Getters
	/**
	 * Returns the coordinates of the annotation
	 */
	const Coords &getCoords() const {
		return _coords;
	}

	/**
	 * Returns the annotation's tile
	 */
	MapTile &getTile() {
		return _tile;
	}

	/**
	 * Returns true for visual-only annotations
	 */
	bool isVisualOnly() const {
		return _visual;
	}

	/**
	 * Returns the number of turns the annotation has left to live
	 */
	int getTTL() const {
		return _ttl;
	}

	bool isCoverUp() const {
		return _coverUp;
	}

	// Setters
	/**
	 * Sets the coordinates for the annotation
	 */
	void setCoords(const Coords &c) {
		_coords = c;
	}

	/**
	 * Sets the tile for the annotation
	 */
	void setTile(const MapTile &t) {
		_tile = t;
	}

	/**
	 * Sets whether or not the annotation is visual-only
	 */
	void setVisualOnly(bool v) {
		_visual = v;
	}

	/**
	 * Sets the number of turns the annotation will live
	 */
	void setTTL(int turns) {
		_ttl = turns;
	}

	/**
	 * Passes a turn for the annotation
	 */
	void passTurn() {
		if (_ttl > 0) _ttl--;
	}

	bool operator==(const Annotation &) const;

	// Properties
private:
	Coords _coords;
	MapTile _tile;
	bool _visual;
	int _ttl;
	bool _coverUp;
};

/**
 * Manages annotations for the current map.  This includes
 * adding and removing annotations, as well as finding annotations
 * and managing their existence.
 */
class AnnotationMgr {
public:
	AnnotationMgr();

	/**
	 * Adds an annotation to the current map
	 */
	Annotation *add(Coords coords, MapTile tile, bool visual = false, bool isCoverUp = false);

	/**
	 * Returns all annotations found at the given map coordinates
	 */
	Annotation::List allAt(Coords pos);

	/**
	 * Returns pointers to all annotations found at the given map coordinates
	 */
	Common::List<Annotation *> ptrsToAllAt(Coords pos);

	/**
	 * Removes all annotations on the map
	 */
	void clear();

	/**
	 * Passes a turn for annotations and removes any
	 * annotations whose TTL has expired
	 */
	void passTurn();

	/**
	 * Removes an annotation from the current map
	 */
	void remove(Coords pos, MapTile tile);
	void remove(Annotation &);

	/**
	 * Removes an entire list of annotations
	 */
	void remove(Annotation::List);

	/**
	 * Returns the number of annotations on the map
	 */
	int size() const;

private:
	Annotation::List  _annotations;
	Annotation::List::iterator _it;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
