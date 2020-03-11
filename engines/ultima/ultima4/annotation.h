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

#ifndef ULTIMA4_ANNOTATION_H
#define ULTIMA4_ANNOTATION_H

#include "ultima/ultima4/coords.h"
#include "ultima/ultima4/types.h"
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
    const Coords& getCoords() const {return coords; } /**< Returns the coordinates of the annotation */
    MapTile& getTile()              {return tile;   } /**< Returns the annotation's tile */
    bool isVisualOnly() const {return visual; } /**< Returns true for visual-only annotations */
    const int getTTL() const        {return ttl;    } /**< Returns the number of turns the annotation has left to live */
    bool isCoverUp()                {return coverUp;}

    // Setters
    void setCoords(const Coords &c) {coords = c;    } /**< Sets the coordinates for the annotation */
    void setTile(const MapTile &t)  {tile = t;      } /**< Sets the tile for the annotation */
    void setVisualOnly(bool v)      {visual = v;    } /**< Sets whether or not the annotation is visual-only */
    void setTTL(int turns)          {ttl = turns;   } /**< Sets the number of turns the annotation will live */
    void passTurn()                 {if (ttl > 0) ttl--; } /**< Passes a turn for the annotation */

    bool operator==(const Annotation&) const;    

    // Properties
private:        
    Coords coords;
    MapTile tile;        
    bool visual;
    int ttl;
    bool coverUp;
};

/** 
 * Manages annotations for the current map.  This includes
 * adding and removing annotations, as well as finding annotations
 * and managing their existence.
 */
class AnnotationMgr {    
public:        
    AnnotationMgr();

    Annotation       *add(Coords coords, MapTile tile, bool visual = false, bool isCoverUp = false);
    Annotation::List allAt(Coords pos);
    Common::List<Annotation *> ptrsToAllAt(Coords pos);
    void             clear();
    void             passTurn();
    void             remove(Coords pos, MapTile tile);
    void             remove(Annotation&);
    void             remove(Annotation::List);
    int              size();

private:        
    Annotation::List  annotations;
    Annotation::List::iterator i;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
