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

#ifndef ULTIMA4_TYPEDEFS_H
#define ULTIMA4_TYPEDEFS_H

#include "ultima/ultima4/direction.h"

namespace Ultima {
namespace Ultima4 {

class Tile;

typedef unsigned int TileId;
typedef unsigned char MapId;

typedef enum {
    FAST,
    SLOW,
    VSLOW,
    VVSLOW
} TileSpeed;

typedef enum {
    EFFECT_NONE,
    EFFECT_FIRE,
    EFFECT_SLEEP,
    EFFECT_POISON,
    EFFECT_POISONFIELD,
    EFFECT_ELECTRICITY,
    EFFECT_LAVA
} TileEffect;

typedef enum {
    ANIM_NONE,
    ANIM_SCROLL,
    ANIM_CAMPFIRE,
    ANIM_CITYFLAG,
    ANIM_CASTLEFLAG,
    ANIM_SHIPFLAG,    
    ANIM_LCBFLAG,
    ANIM_FRAMES
} TileAnimationStyle;

/**
 * A MapTile is a specific instance of a Tile.
 */
class MapTile {
public:
    MapTile() : id(0), frame(0) {}
    MapTile(const TileId &i, unsigned char f = 0) : id(i), frame(f), freezeAnimation(false) {}
    MapTile(const MapTile &t) : id(t.id), frame(t.frame), freezeAnimation(t.freezeAnimation) {}

    TileId getId() const			{return id;}
    unsigned char getFrame() const	{return frame;}
    bool getFreezeAnimation() const {return freezeAnimation;}

    bool operator==(const MapTile &m) const  { return id == m.id; }
    bool operator==(const TileId &i) const   { return id == i; }
    bool operator!=(const MapTile &m) const  { return id != m.id; }
    bool operator!=(const TileId &i) const   { return id != i; }
    bool operator<(const MapTile &m) const   { return id < m.id; } /* for Std::less */
    
    Direction getDirection() const;
    bool setDirection(Direction d);

    const Tile *getTileType() const;

    // Properties
    TileId id;
    unsigned char frame;
    bool freezeAnimation;
};

/**
 * An Uncopyable has no default copy constructor of operator=.  A subclass may derive from
 * Uncopyable at any level of visibility, even private, and subclasses will not have a default copy
 * constructor or operator=. See also, boost::noncopyable Uncopyable (from the Boost project) and
 * Item 6 from Scott Meyers Effective C++.
 */
class Uncopyable {
protected:
    Uncopyable() {}
    ~Uncopyable() {}

private:
    Uncopyable(const Uncopyable&);
    const Uncopyable &operator=(const Uncopyable&);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
