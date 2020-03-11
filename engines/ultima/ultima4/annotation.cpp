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

#include "ultima/ultima4/annotation.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/event.h"
#include "ultima/ultima4/map.h"
#include "ultima/ultima4/settings.h"
#include "common/debug.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Annotation class implementation
 */ 
/**
 * Constructors
 */ 
Annotation::Annotation(const Coords &pos, MapTile t, bool v, bool coverUp) :
    coords(pos), 
    tile(t),
    visual(v),
    ttl(-1),
    coverUp(coverUp)
{}

/**
 * Members
 */ 
void Annotation::debug_output() const {        
    debug(1, "x: %d\n", coords.x);
    debug(1, "y: %d\n", coords.y);
    debug(1, "z: %d\n", coords.z);
    debug(1, "tile: %d\n", tile.getId());
    debug(1, "visual: %s\n", visual ? "Yes" : "No");
}

/**
 * Operators
 */ 
bool Annotation::operator==(const Annotation &a) const {
    return ((coords == a.getCoords()) && (tile == a.tile)) ? true : false;        
}

/**
 * AnnotationMgr implementation
 */
/**
 * Constructors
 */ 
AnnotationMgr::AnnotationMgr() {}

/**
 * Members
 */ 

/**
 * Adds an annotation to the current map
 */
Annotation *AnnotationMgr::add(Coords coords, MapTile tile, bool visual, bool isCoverUp) {
    /* new annotations go to the front so they're handled "on top" */
    annotations.push_front(Annotation(coords, tile, visual, isCoverUp));
    return &annotations.front();
}        

/**
 * Returns all annotations found at the given map coordinates
 */ 
Annotation::List AnnotationMgr::allAt(Coords coords) {
    Annotation::List list;

    for (i = annotations.begin(); i != annotations.end(); i++) {
        if (i->getCoords() == coords)
            list.push_back(*i);
    }
    
    return list;
}

/**
 * Returns pointers to all annotations found at the given map coordinates
 */ 
Common::List<Annotation *> AnnotationMgr::ptrsToAllAt(Coords coords) {
    Common::List<Annotation *> list;

    for (i = annotations.begin(); i != annotations.end(); i++) {
        if (i->getCoords() == coords)
            list.push_back(&(*i));
    }
    
    return list;
}

/**
 * Removes all annotations on the map 
 */ 
void AnnotationMgr::clear() {
    annotations.clear();        
}    

/**
 * Passes a turn for annotations and removes any
 * annotations whose TTL has expired
 */ 
void AnnotationMgr::passTurn() {
    for (i = annotations.begin(); i != annotations.end(); i++) {
        if (i->getTTL() == 0) {
            i = annotations.erase(i);
            if (i == annotations.end())
                break;
        }
        else if (i->getTTL() > 0)
            i->passTurn();
    }
}

/**
 * Removes an annotation from the current map
 */
void AnnotationMgr::remove(Coords coords, MapTile tile) {        
    Annotation look_for(coords, tile);
    remove(look_for);
}

void AnnotationMgr::remove(Annotation &a) {
    for (i = annotations.begin(); i != annotations.end(); i++) {
        if (*i == a) {
            i = annotations.erase(i);
            break;
        }
    }
}

/**
 * Removes an entire list of annotations 
 */ 
void AnnotationMgr::remove(Annotation::List l) {
    Annotation::List::iterator it;
    for (it = l.begin(); it != l.end(); it++) {
        remove(*it);
    }
}

/**
 * Returns the number of annotations on the map
 */ 
int AnnotationMgr::size() {
    return annotations.size();
}

} // End of namespace Ultima4
} // End of namespace Ultima
