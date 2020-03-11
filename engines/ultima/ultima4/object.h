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
#ifndef ULTIMA4_OBJECT_H
#define ULTIMA4_OBJECT_H

#include "ultima/ultima4/coords.h"
#include "ultima/ultima4/tile.h"
#include "ultima/ultima4/types.h"

namespace Ultima {
namespace Ultima4 {

typedef Std::deque<class Object *> ObjectDeque;

typedef enum {
    MOVEMENT_FIXED,
    MOVEMENT_WANDER,
    MOVEMENT_FOLLOW_AVATAR,
    MOVEMENT_ATTACK_AVATAR
} ObjectMovementBehavior;

class Object {
public:
    enum Type {
        UNKNOWN,
        CREATURE,
        PERSON
    };

    Object(Type type = UNKNOWN) :    
      tile(0),
      prevTile(0),      
      movement_behavior(MOVEMENT_FIXED),
      objType(type), 
      focused(false),
      visible(true),
      animated(true)
    {}
    
    virtual ~Object() {}    

    // Methods
    MapTile& getTile()                      { return tile; }
    MapTile& getPrevTile()                  { return prevTile; }
    const Coords& getCoords() const         { return coords; }
    const Coords& getPrevCoords() const     { return prevCoords; }    
    const ObjectMovementBehavior getMovementBehavior() const    { return movement_behavior; }
    const Type getType() const              { return objType; }
    bool hasFocus() const                   { return focused; }
    bool isVisible() const                  { return visible; }
    bool isAnimated() const                 { return animated; }    

    void setTile(MapTile t)                 { tile = t; }
    void setTile(Tile *t)                   {tile = t->getId();}
    void setPrevTile(MapTile t)             { prevTile = t; }
    void setCoords(Coords c)                { prevCoords = coords; coords = c; }
    void setPrevCoords(Coords c)            { prevCoords = c; }    
    void setMovementBehavior(ObjectMovementBehavior b)          { movement_behavior = b; }
    void setType(Type t)                    { objType = t; }
    void setFocus(bool f = true)            { focused = f; }
    void setVisible(bool v = true)          { visible = v; }
    void setAnimated(bool a = true)         { animated = a; }
    
    void setMap(class Map *m);
    Map *getMap();    
    void remove();  /**< Removes itself from any maps that it is a part of */

    bool setDirection(Direction d);

    void animateMovement();

    // Properties
protected:
    MapTile tile, prevTile;
    Coords coords, prevCoords;
    ObjectMovementBehavior movement_behavior;
    Type objType;
    Std::deque<class Map *> maps;           /**< A list of maps this object is a part of */    
    
    bool focused;
    bool visible;
    bool animated;    
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
