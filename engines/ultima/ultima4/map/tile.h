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

#ifndef ULTIMA4_MAP_TILE_H
#define ULTIMA4_MAP_TILE_H

#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class ConfigElement;
class Image;
class Tileset;
class TileAnim;

/* attr masks */
#define MASK_SHIP                   0x0001
#define MASK_HORSE                  0x0002
#define MASK_BALLOON                0x0004
#define MASK_DISPEL                 0x0008
#define MASK_TALKOVER               0x0010
#define MASK_DOOR                   0x0020
#define MASK_LOCKEDDOOR             0x0040
#define MASK_CHEST                  0x0080
#define MASK_ATTACKOVER             0x0100
#define MASK_CANLANDBALLOON         0x0200
#define MASK_REPLACEMENT            0x0400
#define MASK_WATER_REPLACEMENT      0x0800
#define MASK_FOREGROUND             0x1000
#define MASK_LIVING_THING           0x2000


/* movement masks */
#define MASK_SWIMABLE           0x0001
#define MASK_SAILABLE           0x0002
#define MASK_UNFLYABLE          0x0004
#define MASK_CREATURE_UNWALKABLE 0x0008

/**
 * A Tile object represents a specific tile type.  Every tile is a
 * member of a Tileset.
 */
class Tile : private Uncopyable {
public:
	Tile(Tileset *tileset);
	~Tile();

	/**
	 * Loads tile information.
	 */
	void loadProperties(const ConfigElement &conf);

	TileId getId() const {
		return _id;
	}
	const Common::String &getName() const {
		return _name;
	}
	int getWidth() const {
		return _w;
	}
	int getHeight() const {
		return _h;
	}
	int getFrames() const {
		return _frames;
	}
	int getScale() const {
		return _scale;
	}
	TileAnim *getAnim() const {
		return _anim;
	}
	Image *getImage();
	const Common::String &getLooksLike() const {
		return _looksLike;
	}

	bool isTiledInDungeon() const {
		return _tiledInDungeon;
	}
	bool isLandForeground() const {
		return _foreground;
	}
	bool isWaterForeground() const {
		return _waterForeground;
	}

	int canWalkOn(Direction d) const {
		return DIR_IN_MASK(d, rule->_walkOnDirs);
	}
	int canWalkOff(Direction d) const {
		return DIR_IN_MASK(d, rule->_walkOffDirs);
	}

	/**
	 * All tiles that you can walk, swim, or sail on, can be attacked over. All others must declare
	 * themselves
	 */
	int  canAttackOver() const {
		return isWalkable() || isSwimable() || isSailable() || (rule->_mask & MASK_ATTACKOVER);
	}
	int  canLandBalloon() const {
		return rule->_mask & MASK_CANLANDBALLOON;
	}
	int  isLivingObject() const {
		return rule->_mask & MASK_LIVING_THING;
	}
	int  isReplacement() const {
		return rule->_mask & MASK_REPLACEMENT;
	}
	int  isWaterReplacement() const {
		return rule->_mask & MASK_WATER_REPLACEMENT;
	}

	int  isWalkable() const {
		return rule->_walkOnDirs > 0;
	}
	bool isCreatureWalkable() const {
		return canWalkOn(DIR_ADVANCE) && !(rule->_movementMask & MASK_CREATURE_UNWALKABLE);
	}
	bool isDungeonWalkable() const;
	bool isDungeonFloor() const;
	int  isSwimable() const {
		return rule->_movementMask & MASK_SWIMABLE;
	}
	int  isSailable() const {
		return rule->_movementMask & MASK_SAILABLE;
	}
	bool isWater() const {
		return (isSwimable() || isSailable());
	}
	int  isFlyable() const {
		return !(rule->_movementMask & MASK_UNFLYABLE);
	}
	int  isDoor() const {
		return rule->_mask & MASK_DOOR;
	}
	int  isLockedDoor() const {
		return rule->_mask & MASK_LOCKEDDOOR;
	}
	int  isChest() const {
		return rule->_mask & MASK_CHEST;
	}
	int  isShip() const {
		return rule->_mask & MASK_SHIP;
	}
	bool isPirateShip() const {
		return _name == "pirate_ship";
	}
	int  isHorse() const {
		return rule->_mask & MASK_HORSE;
	}
	int  isBalloon() const {
		return rule->_mask & MASK_BALLOON;
	}
	int  canDispel() const {
		return rule->_mask & MASK_DISPEL;
	}
	int  canTalkOver() const {
		return rule->_mask & MASK_TALKOVER;
	}
	TileSpeed getSpeed() const {
		return rule->_speed;
	}
	TileEffect getEffect() const {
		return rule->_effect;
	}

	bool isOpaque() const;

	/**
	 * Is tile a foreground tile (i.e. has transparent parts).
	 * Deprecated? Never used in XML. Other mechanisms exist, though this could help?
	 */
	bool isForeground() const;
	Direction directionForFrame(int frame) const;
	int frameForDirection(Direction d) const;

	static void resetNextId() {
		_nextId = 0;
	}
	static bool canTalkOverTile(const Tile *tile) {
		return tile->canTalkOver() != 0;
	}
	static bool canAttackOverTile(const Tile *tile) {
		return tile->canAttackOver() != 0;
	}
	void deleteImage();

private:
	/**
	 * Loads the tile image
	 */
	void loadImage();

private:
	TileId _id;          /**< an id that is unique across all tilesets */
	Common::String _name;        /**< The name of this tile */
	Tileset *_tileSet;   /**< The tileset this tile belongs to */
	int _w, _h;           /**< width and height of the tile */
	int _frames;         /**< The number of frames this tile has */
	int _scale;          /**< The scale of the tile */
	TileAnim *_anim;     /**< The tile animation for this tile */
	bool _opaque;        /**< Is this tile opaque? */

	bool _foreground;    /**< As a maptile, is a foreground that will search neighbour maptiles for a land-based background replacement. ex: chests */
	bool _waterForeground;/**< As a maptile, is a foreground that will search neighbour maptiles for a water-based background replacement. ex: chests */

	TileRule *rule;     /**< The rules that govern the behavior of this tile */
	Common::String _imageName;   /**< The name of the image that belongs to this tile */
	Common::String _looksLike;  /**< The name of the tile that this tile looks exactly like (if any) */

	Image *_image;       /**< The original image for this tile (with all of its frames) */
	bool _tiledInDungeon;
	Std::vector<Direction> _directions;

	Common::String _animationRule;


	static TileId _nextId;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
