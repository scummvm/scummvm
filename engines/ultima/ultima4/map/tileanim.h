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

#ifndef ULTIMA4_MAP_TILEANIM_H
#define ULTIMA4_MAP_TILEANIM_H

#include "ultima/ultima4/map/direction.h"

namespace Ultima {
namespace Ultima4 {

class ConfigElement;
class Image;
class Tile;
struct RGBA;

/**
 * The interface for tile animation transformations.
 */
class TileAnimTransform {
public:
	static TileAnimTransform *create(const ConfigElement &config);

	/**
	 * Loads a color from a config element
	 */
	static RGBA *loadColorFromConf(const ConfigElement &conf);

	virtual void draw(Image *dest, Tile *tile, MapTile &mapTile) = 0;
	virtual ~TileAnimTransform() {}
	virtual bool drawsTile() const = 0;

	// Properties
	int _random;
};

/**
 * A tile animation transformation that turns a piece of the tile
 * upside down.  Used for animating the flags on building and ships.
 */
class TileAnimInvertTransform : public TileAnimTransform {
public:
	TileAnimInvertTransform(int xp, int yp, int width, int height);
	void draw(Image *dest, Tile *tile, MapTile &mapTile) override;
	bool drawsTile() const override;

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
	TileAnimPixelTransform(int xp, int yp);
	void draw(Image *dest, Tile *tile, MapTile &mapTile) override;
	bool drawsTile() const override;

	int x, y;
	Std::vector<RGBA *> _colors;
};

/**
 * A tile animation transformation that scrolls the tile's contents
 * vertically within the tile's boundaries.
 */
class TileAnimScrollTransform : public TileAnimTransform {
public:
	TileAnimScrollTransform(int increment);
	void draw(Image *dest, Tile *tile, MapTile &mapTile) override;
	bool drawsTile() const override;
private:
	int _increment, _current, _lastOffset;
};

/**
 * A tile animation transformation that advances the tile's frame
 * by 1.
 */
class TileAnimFrameTransform : public TileAnimTransform {
public:
	TileAnimFrameTransform() : _currentFrame(0) {
	}
	void draw(Image *dest, Tile *tile, MapTile &mapTile) override;

	/**
	 * Advance the frame by one and draw it!
	 */
	bool drawsTile() const override;
protected:
	int _currentFrame;
};

/**
 * A tile animation transformation that changes pixels with colors
 * that fall in a given range to another color.  Used to animate
 * the campfire in VGA mode.
 */
class TileAnimPixelColorTransform : public TileAnimTransform {
public:
	TileAnimPixelColorTransform(int xp, int yp, int width, int height);
	~TileAnimPixelColorTransform() override;

	void draw(Image *dest, Tile *tile, MapTile &mapTile) override;
	bool drawsTile() const override;

	int x, y, w, h;
	RGBA *_start, *_end;
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

	/**
	 * Creates a new animation context which controls if animation transforms are performed or not
	 */
	static TileAnimContext *create(const ConfigElement &config);

	/**
	 * Adds a tile transform to the context
	 */
	void add(TileAnimTransform *);
	virtual bool isInContext(Tile *t, MapTile &mapTile, Direction d) = 0;
	TileAnimTransformList &getTransforms() {
		return _animTransforms;   /**< Returns a list of transformations under the context. */
	}
	virtual ~TileAnimContext() {}
private:

	TileAnimTransformList _animTransforms;
};

/**
 * An animation context which changes the animation based on the tile's current frame
 */
class TileAnimFrameContext : public TileAnimContext {
public:
	/**
	 * A context which depends on the tile's current frame for animation
	 */
	TileAnimFrameContext(int frame);
	bool isInContext(Tile *t, MapTile &mapTile, Direction d) override;

private:
	int _frame;
};

/**
 * An animation context which changes the animation based on the player's current facing direction
 */
class TileAnimPlayerDirContext : public TileAnimContext {
public:
	/**
	 * An animation context which changes the animation based on the player's current facing direction
	 */
	TileAnimPlayerDirContext(Direction dir);
	bool isInContext(Tile *t, MapTile &mapTile, Direction d) override;

private:
	Direction _dir;
};

/**
 * Instructions for animating a tile.  Each tile animation is made up
 * of a list of transformations which are applied to the tile after it
 * is drawn.
 */
class TileAnim {
public:
	TileAnim(const ConfigElement &conf);
	~TileAnim();

	Common::String _name;
	Std::vector<TileAnimTransform *> _transforms;
	Std::vector<TileAnimContext *> _contexts;

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
	~TileAnimSet();

	/**
	 * Returns the tile animation with the given name from the current set
	 */
	TileAnim *getByName(const Common::String &name);

	Common::String _name;
	TileAnimMap _tileAnims;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
