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

#ifndef ULTIMA4_TILEANIM_H
#define ULTIMA4_TILEANIM_H

#include "ultima/ultima4/direction.h"

namespace Ultima {
namespace Ultima4 {

class ConfigElement;
class Image;
class Tile;
struct RGBA;

/**
 * The interface for tile animation transformations.
 */
class  TileAnimTransform {
public:
    static TileAnimTransform *create(const ConfigElement &config);    
    static RGBA *loadColorFromConf(const ConfigElement &conf);
    
    virtual void draw(Image *dest, Tile *tile, MapTile &mapTile) = 0;
    virtual ~TileAnimTransform() {}
    virtual bool drawsTile() const = 0;
    
    // Properties
    int random;

private:    
    bool replaces;
};

/**
 * A tile animation transformation that turns a piece of the tile
 * upside down.  Used for animating the flags on building and ships.
 */
class TileAnimInvertTransform : public TileAnimTransform {
public:
    TileAnimInvertTransform(int x, int y, int w, int h);
    virtual void draw(Image *dest, Tile *tile, MapTile &mapTile);
    virtual bool drawsTile() const;
    
private:
    int x, y, w, h;
};

/**
 * A tile animation transformation that changes a single pixels to a
 * random color selected from a list.  Used for animating the
 * campfire in EGA mode.
 */
class TileAnimPixelTransform : public TileAnimTransform {
public:
    TileAnimPixelTransform(int x, int y);
    virtual void draw(Image *dest, Tile *tile, MapTile &mapTile);
    virtual bool drawsTile() const;

    int x, y;
    Std::vector<RGBA *> colors;
};

/**
 * A tile animation transformation that scrolls the tile's contents
 * vertically within the tile's boundaries.
 */ 
class TileAnimScrollTransform : public TileAnimTransform {
public:    
    TileAnimScrollTransform(int increment);
    virtual void draw(Image *dest, Tile *tile, MapTile &mapTile);    
    virtual bool drawsTile() const;
private:
    int increment, current, lastOffset;
};

/**
 * A tile animation transformation that advances the tile's frame
 * by 1.
 */ 
class TileAnimFrameTransform : public TileAnimTransform {
public:
	TileAnimFrameTransform() : currentFrame(0){}
    virtual void draw(Image *dest, Tile *tile, MapTile &mapTile);
    virtual bool drawsTile() const;
protected:
    int currentFrame;
};

/**
 * A tile animation transformation that changes pixels with colors
 * that fall in a given range to another color.  Used to animate
 * the campfire in VGA mode.
 */ 
class TileAnimPixelColorTransform : public TileAnimTransform {
public:
    TileAnimPixelColorTransform(int x, int y, int w, int h);
    virtual void draw(Image *dest, Tile *tile, MapTile &mapTile);
    virtual bool drawsTile() const;

    int x, y, w, h;
    RGBA *start, *end;
};

/**
 * A context in which to perform the animation
 */ 
class TileAnimContext {    
public:
    typedef Std::vector<TileAnimTransform *> TileAnimTransformList;
    typedef enum {
        FRAME,
        DIR
    } Type;

    static TileAnimContext* create(const ConfigElement &config);
    
    void add(TileAnimTransform*);
    virtual bool isInContext(Tile *t, MapTile &mapTile, Direction d) = 0;
	TileAnimTransformList& getTransforms() {return animTransforms;}	/**< Returns a list of transformations under the context. */
    virtual ~TileAnimContext() {}
private:    
    
    TileAnimTransformList animTransforms;
};

/**
 * An animation context which changes the animation based on the tile's current frame
 */ 
class TileAnimFrameContext : public TileAnimContext {
public:
    TileAnimFrameContext(int frame);    
    virtual bool isInContext(Tile *t, MapTile &mapTile, Direction d);

private:
    int frame;
};

/**
 * An animation context which changes the animation based on the player's current facing direction
 */ 
class TileAnimPlayerDirContext : public TileAnimContext {
public:
    TileAnimPlayerDirContext(Direction dir);
    virtual bool isInContext(Tile *t, MapTile &mapTile, Direction d);

private:
    Direction dir;
};

/**
 * Instructions for animating a tile.  Each tile animation is made up
 * of a list of transformations which are applied to the tile after it
 * is drawn.
 */
class TileAnim {
public:
    TileAnim(const ConfigElement &conf);

    Common::String name;
    Std::vector<TileAnimTransform *> transforms;
    Std::vector<TileAnimContext *> contexts;

    /* returns the frame to set the mapTile to (only relevent if persistent) */
    void draw(Image *dest, Tile *tile, MapTile &mapTile, Direction dir);     

    int _random;   /* true if the tile animation occurs randomely */
};

/**
 * A set of tile animations.  Tile animations are associated with a
 * specific image set which shares the same name.
 */
class TileAnimSet {
    typedef Std::map<Common::String, TileAnim *> TileAnimMap;

public:
    TileAnimSet(const ConfigElement &conf);

    TileAnim *getByName(const Common::String &name);

    Common::String name;
    TileAnimMap tileanims;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
