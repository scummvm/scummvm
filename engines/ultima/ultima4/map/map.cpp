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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/movement.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/game/person.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/map/tilemap.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/core/settings.h"

namespace Ultima {
namespace Ultima4 {

bool MapCoords::operator==(const MapCoords &a) const {
	return (x == a.x) && (y == a.y) && (z == a.z);
}

bool MapCoords::operator!=(const MapCoords &a) const {
	return !operator==(a);
}

bool MapCoords::operator<(const MapCoords &a)  const {
	if (x > a.x)
		return false;
	if (y > a.y)
		return false;
	return z < a.z;
}

MapCoords &MapCoords::wrap(const Map *map) {
	if (map && map->_borderBehavior == Map::BORDER_WRAP) {
		while (x < 0)
			x += map->_width;
		while (y < 0)
			y += map->_height;
		while (x >= (int)map->_width)
			x -= map->_width;
		while (y >= (int)map->_height)
			y -= map->_height;
	}
	return *this;
}

MapCoords &MapCoords::putInBounds(const Map *map) {
	if (map) {
		if (x < 0)
			x = 0;
		if (x >= (int) map->_width)
			x = map->_width - 1;
		if (y < 0)
			y = 0;
		if (y >= (int) map->_height)
			y = map->_height - 1;
		if (z < 0)
			z = 0;
		if (z >= (int) map->_levels)
			z = map->_levels - 1;
	}
	return *this;
}

MapCoords &MapCoords::move(Direction d, const Map *map) {
	switch (d) {
	case DIR_NORTH:
		y--;
		break;
	case DIR_EAST:
		x++;
		break;
	case DIR_SOUTH:
		y++;
		break;
	case DIR_WEST:
		x--;
		break;
	default:
		break;
	}

	// Wrap the coordinates if necessary
	wrap(map);

	return *this;
}

MapCoords &MapCoords::move(int dx, int dy, const Map *map) {
	x += dx;
	y += dy;

	// Wrap the coordinates if necessary
	wrap(map);

	return *this;
}

int MapCoords::getRelativeDirection(const MapCoords &c, const Map *map) const {
	int dx, dy, dirmask;

	dirmask = DIR_NONE;
	if (z != c.z)
		return dirmask;

	// Adjust our coordinates to find the closest path
	if (map && map->_borderBehavior == Map::BORDER_WRAP) {
		MapCoords me = *this;

		if (abs(int(me.x - c.x)) > abs(int(me.x + map->_width - c.x)))
			me.x += map->_width;
		else if (abs(int(me.x - c.x)) > abs(int(me.x - map->_width - c.x)))
			me.x -= map->_width;

		if (abs(int(me.y - c.y)) > abs(int(me.y + map->_width - c.y)))
			me.y += map->_height;
		else if (abs(int(me.y - c.y)) > abs(int(me.y - map->_width - c.y)))
			me.y -= map->_height;

		dx = me.x - c.x;
		dy = me.y - c.y;
	} else {
		dx = x - c.x;
		dy = y - c.y;
	}

	// Add x directions that lead towards to_x to the mask
	if (dx < 0)         dirmask |= MASK_DIR(DIR_EAST);
	else if (dx > 0)    dirmask |= MASK_DIR(DIR_WEST);

	// Add y directions that lead towards to_y to the mask
	if (dy < 0)         dirmask |= MASK_DIR(DIR_SOUTH);
	else if (dy > 0)    dirmask |= MASK_DIR(DIR_NORTH);

	// Return the result
	return dirmask;
}

Direction MapCoords::pathTo(const MapCoords &c, int valid_directions, bool towards, const Map *map) const {
	int directionsToObject;

	// Find the directions that lead [to/away from] our target
	directionsToObject = towards ? getRelativeDirection(c, map) : ~getRelativeDirection(c, map);

	// Make sure we eliminate impossible options
	directionsToObject &= valid_directions;

	// Get the new direction to move
	if (directionsToObject > DIR_NONE)
		return dirRandomDir(directionsToObject);
	// There are no valid directions that lead to our target, just move wherever we can!
	else
		return dirRandomDir(valid_directions);
}

Direction MapCoords::pathAway(const MapCoords &c, int valid_directions) const {
	return pathTo(c, valid_directions, false);
}

int MapCoords::movementDistance(const MapCoords &c, const Map *map) const {
	int dirmask = DIR_NONE;
	int dist = 0;
	MapCoords me = *this;

	if (z != c.z)
		return -1;

	// Get the direction(s) to the coordinates
	dirmask = getRelativeDirection(c, map);

	while ((me.x != c.x) || (me.y != c.y)) {
		if (me.x != c.x) {
			if (dirmask & MASK_DIR_WEST)
				me.move(DIR_WEST, map);
			else
				me.move(DIR_EAST, map);

			dist++;
		}
		if (me.y != c.y) {
			if (dirmask & MASK_DIR_NORTH)
				me.move(DIR_NORTH, map);
			else
				me.move(DIR_SOUTH, map);

			dist++;
		}
	}

	return dist;
}

int MapCoords::distance(const MapCoords &c, const Map *map) const {
	int dist = movementDistance(c, map);
	if (dist <= 0)
		return dist;

	// Calculate how many fewer movements there would have been
	dist -= abs(x - c.x) < abs(y - c.y) ? abs(x - c.x) : abs(y - c.y);

	return dist;
}

/*-------------------------------------------------------------------*/

Map::Map() : _id(0), _type(WORLD), _width(0), _height(0), _levels(1),
		_chunkWidth(0), _chunkHeight(0), _offset(0), _flags(0),
		_borderBehavior(BORDER_WRAP), _music(Music::NONE),
		_tileSet(nullptr), _tileMap(nullptr), _blank(0) {
	_annotations = new AnnotationMgr();
}

Map::~Map() {
	for (PortalList::iterator i = _portals.begin(); i != _portals.end(); i++)
		delete *i;
	delete _annotations;
}

Common::String Map::getName() {
	return _baseSource._fname;
}

Object *Map::objectAt(const Coords &coords) {
	// FIXME: return a list instead of one object
	ObjectDeque::const_iterator i;
	Object *objAt = nullptr;

	for (i = _objects.begin(); i != _objects.end(); i++) {
		Object *obj = *i;

		if (obj->getCoords() == coords) {
			// Get the most visible object
			if (objAt && (objAt->getType() == Object::UNKNOWN) && (obj->getType() != Object::UNKNOWN))
				objAt = obj;
			// Give priority to objects that have the focus
			else if (objAt && (!objAt->hasFocus()) && (obj->hasFocus()))
				objAt = obj;
			else if (!objAt)
				objAt = obj;
		}
	}
	return objAt;
}

const Portal *Map::portalAt(const Coords &coords, int actionFlags) {
	PortalList::const_iterator i;

	for (i = _portals.begin(); i != _portals.end(); i++) {
		if (((*i)->_coords == coords) &&
		        ((*i)->_triggerAction & actionFlags))
			return *i;
	}
	return nullptr;
}

MapTile *Map::getTileFromData(const Coords &coords) {
	if (MAP_IS_OOB(this, coords))
		return &_blank;

	int index = coords.x + (coords.y * _width) + (_width * _height * coords.z);
	return &_data[index];
}

MapTile *Map::tileAt(const Coords &coords, int withObjects) {
	// FIXME: this should return a list of tiles, with the most visible at the front
	MapTile *tile;
	Common::List<Annotation *> a = _annotations->ptrsToAllAt(coords);
	Common::List<Annotation *>::iterator i;
	Object *obj = objectAt(coords);

	tile = getTileFromData(coords);

	// FIXME: this only returns the first valid annotation it can find
	if (a.size() > 0) {
		for (i = a.begin(); i != a.end(); i++) {
			if (!(*i)->isVisualOnly())
				return &(*i)->getTile();
		}
	}

	if ((withObjects == WITH_OBJECTS) && obj)
		tile = &obj->getTile();
	else if ((withObjects == WITH_GROUND_OBJECTS) &&
	         obj &&
	         obj->getTile().getTileType()->isWalkable())
		tile = &obj->getTile();

	return tile;
}

const Tile *Map::tileTypeAt(const Coords &coords, int withObjects) {
	MapTile *tile = tileAt(coords, withObjects);
	return tile->getTileType();
}

bool Map::isWorldMap() {
	return _type == WORLD;
}

bool Map::isEnclosed(const Coords &party) {
	uint x, y;
	int *path_data;

	if (_borderBehavior != BORDER_WRAP)
		return true;

	path_data = new int[_width * _height];
	memset(path_data, -1, sizeof(int) * _width * _height);

	// Determine what's walkable (1), and what's border-walkable (2)
	findWalkability(party, path_data);

	// Find two connecting pathways where the avatar can reach both without wrapping
	for (x = 0; x < _width; x++) {
		int index = x;
		if (path_data[index] == 2 && path_data[index + ((_height - 1)*_width)] == 2)
			return false;
	}

	for (y = 0; y < _width; y++) {
		int index = (y * _width);
		if (path_data[index] == 2 && path_data[index + _width - 1] == 2)
			return false;
	}

	return true;
}

void Map::findWalkability(Coords coords, int *path_data) {
	const Tile *mt = tileTypeAt(coords, WITHOUT_OBJECTS);
	int index = coords.x + (coords.y * _width);

	if (mt->isWalkable()) {
		bool isBorderTile = (coords.x == 0) || (coords.x == signed(_width - 1)) || (coords.y == 0) || (coords.y == signed(_height - 1));
		path_data[index] = isBorderTile ? 2 : 1;

		if ((coords.x > 0) && path_data[coords.x - 1 + (coords.y * _width)] < 0)
			findWalkability(Coords(coords.x - 1, coords.y, coords.z), path_data);
		if ((coords.x < signed(_width - 1)) && path_data[coords.x + 1 + (coords.y * _width)] < 0)
			findWalkability(Coords(coords.x + 1, coords.y, coords.z), path_data);
		if ((coords.y > 0) && path_data[coords.x + ((coords.y - 1) * _width)] < 0)
			findWalkability(Coords(coords.x, coords.y - 1, coords.z), path_data);
		if ((coords.y < signed(_height - 1)) && path_data[coords.x + ((coords.y + 1) * _width)] < 0)
			findWalkability(Coords(coords.x, coords.y + 1, coords.z), path_data);
	} else {
		path_data[index] = 0;
	}
}

Creature *Map::addCreature(const Creature *creature, Coords coords) {
	Creature *m = new Creature();

	// Make a copy of the creature before placing it
	*m = *creature;

	m->setInitialHp();
	m->setStatus(STAT_GOOD);
	m->setCoords(coords);
	m->setMap(this);

	// initialize the creature before placing it
	if (m->wanders())
		m->setMovementBehavior(MOVEMENT_WANDER);
	else if (m->isStationary())
		m->setMovementBehavior(MOVEMENT_FIXED);
	else m->setMovementBehavior(MOVEMENT_ATTACK_AVATAR);

	// Hide camouflaged creatures from view during combat
	if (m->camouflages() && (_type == COMBAT))
		m->setVisible(false);

	// place the creature on the map
	_objects.push_back(m);
	return m;
}

Object *Map::addObject(Object *obj, Coords coords) {
	_objects.push_front(obj);
	return obj;
}

Object *Map::addObject(MapTile tile, MapTile prevtile, Coords coords) {
	Object *obj = new Object();

	obj->setTile(tile);
	obj->setPrevTile(prevtile);
	obj->setCoords(coords);
	obj->setPrevCoords(coords);
	obj->setMap(this);

	_objects.push_front(obj);

	return obj;
}

void Map::removeObject(const Object *rem, bool deleteObject) {
	ObjectDeque::iterator i;
	for (i = _objects.begin(); i != _objects.end(); i++) {
		if (*i == rem) {
			// Party members persist through different maps, so don't delete them!
			if (!isPartyMember(*i) && deleteObject)
				delete(*i);
			_objects.erase(i);
			return;
		}
	}
}

ObjectDeque::iterator Map::removeObject(ObjectDeque::iterator rem, bool deleteObject) {
	// Party members persist through different maps, so don't delete them!
	if (!isPartyMember(*rem) && deleteObject)
		delete(*rem);
	return _objects.erase(rem);
}

Creature *Map::moveObjects(MapCoords avatar) {
	Creature *attacker = nullptr;

	for (uint i = 0; i < _objects.size(); i++) {
		Creature *m = dynamic_cast<Creature *>(_objects[i]);

		if (m) {
			/* check if the object is an attacking creature and not
			   just a normal, docile person in town or an inanimate object */
			if ((m->getType() == Object::PERSON && m->getMovementBehavior() == MOVEMENT_ATTACK_AVATAR) ||
			        (m->getType() == Object::CREATURE && m->willAttack())) {
				MapCoords o_coords = m->getCoords();

				// Don't move objects that aren't on the same level as us
				if (o_coords.z != avatar.z)
					continue;

				if (o_coords.movementDistance(avatar, this) <= 1) {
					attacker = m;
					continue;
				}
			}

			// Before moving, Enact any special effects of the creature (such as storms eating objects, whirlpools teleporting, etc.)
			m->specialEffect();


			// Perform any special actions (such as pirate ships firing cannons, sea serpents' fireblast attect, etc.)
			if (!m->specialAction()) {
				if (moveObject(this, m, avatar)) {
					m->animateMovement();
					// After moving, Enact any special effects of the creature (such as storms eating objects, whirlpools teleporting, etc.)
					m->specialEffect();
				}
			}
		}
	}

	return attacker;
}

void Map::resetObjectAnimations() {
	ObjectDeque::iterator i;

	for (i = _objects.begin(); i != _objects.end(); i++) {
		Object *obj = *i;

		if (obj->getType() == Object::CREATURE)
			obj->setPrevTile(creatureMgr->getByTile(obj->getTile())->getTile());
	}
}

void Map::clearObjects() {
	_objects.clear();
}

int Map::getNumberOfCreatures() {
	ObjectDeque::const_iterator i;
	int n = 0;

	for (i = _objects.begin(); i != _objects.end(); i++) {
		Object *obj = *i;

		if (obj->getType() == Object::CREATURE)
			n++;
	}

	return n;
}

int Map::getValidMoves(MapCoords from, MapTile transport) {
	int retval;
	Direction d;
	Object *obj;
	const Creature *m, *to_m;
	int ontoAvatar, ontoCreature;
	MapCoords coords = from;

	// Get the creature object, if it exists (the one that's moving)
	m = creatureMgr->getByTile(transport);

	bool isAvatar = (g_context->_location->_coords == coords);
	if (m && m->canMoveOntoPlayer())
		isAvatar = false;

	retval = 0;
	for (d = DIR_WEST; d <= DIR_SOUTH; d = (Direction)(d + 1)) {
		coords = from;
		ontoAvatar = 0;
		ontoCreature = 0;

		// Move the coordinates in the current direction and test it
		coords.move(d, this);

		// You can always walk off the edge of the map
		if (MAP_IS_OOB(this, coords)) {
			retval = DIR_ADD_TO_MASK(d, retval);
			continue;
		}

		obj = objectAt(coords);

		// See if it's trying to move onto the avatar
		if ((_flags & SHOW_AVATAR) && (coords == g_context->_location->_coords))
			ontoAvatar = 1;

		// See if it's trying to move onto a person or creature
		else if (obj && (obj->getType() != Object::UNKNOWN))
			ontoCreature = 1;

		// Get the destination tile
		MapTile tile;
		if (ontoAvatar)
			tile = g_context->_party->getTransport();
		else if (ontoCreature)
			tile = obj->getTile();
		else
			tile = *tileAt(coords, WITH_OBJECTS);

		MapTile prev_tile = *tileAt(from, WITHOUT_OBJECTS);

		// Get the other creature object, if it exists (the one that's being moved onto)
		to_m = dynamic_cast<Creature *>(obj);

		// Move on if unable to move onto the avatar or another creature
		if (m && !isAvatar) { // some creatures/persons have the same tile as the avatar, so we have to adjust
			// If moving onto the avatar, the creature must be able to move onto the player
			// If moving onto another creature, it must be able to move onto other creatures,
			// and the creature must be able to have others move onto it.  If either of
			// these conditions are not met, the creature cannot move onto another.

			if ((ontoAvatar && m->canMoveOntoPlayer()) || (ontoCreature && m->canMoveOntoCreatures()))
				tile = *tileAt(coords, WITHOUT_OBJECTS); //Ignore all objects, and just consider terrain
			if ((ontoAvatar && !m->canMoveOntoPlayer())
			        || (
			            ontoCreature &&
			            (
			                (!m->canMoveOntoCreatures() && !to_m->canMoveOntoCreatures())
			                || (m->isForceOfNature() && to_m->isForceOfNature())
			            )
			        )
			   )
				continue;
		}

		// Avatar movement
		if (isAvatar) {
			// if the transport is a ship, check sailable
			if (transport.getTileType()->isShip() && tile.getTileType()->isSailable())
				retval = DIR_ADD_TO_MASK(d, retval);
			// if it is a balloon, check flyable
			else if (transport.getTileType()->isBalloon() && tile.getTileType()->isFlyable())
				retval = DIR_ADD_TO_MASK(d, retval);
			// avatar or horseback: check walkable
			else if (transport == _tileSet->getByName("avatar")->getId() || transport.getTileType()->isHorse()) {
				if (tile.getTileType()->canWalkOn(d) &&
				        (!transport.getTileType()->isHorse() || tile.getTileType()->isCreatureWalkable()) &&
				        prev_tile.getTileType()->canWalkOff(d))
					retval = DIR_ADD_TO_MASK(d, retval);
			}
//            else if (ontoCreature && to_m->canMoveOntoPlayer()) {
//              retval = DIR_ADD_TO_MASK(d, retval);
//            }
		}

		// Creature movement
		else if (m) {
			// Flying creatures
			if (tile.getTileType()->isFlyable() && m->flies()) {
				// FIXME: flying creatures behave differently on the world map?
				if (isWorldMap())
					retval = DIR_ADD_TO_MASK(d, retval);
				else if (tile.getTileType()->isWalkable() ||
				         tile.getTileType()->isSwimable() ||
				         tile.getTileType()->isSailable())
					retval = DIR_ADD_TO_MASK(d, retval);
			}
			// Swimming creatures and sailing creatures
			else if (tile.getTileType()->isSwimable() ||
			         tile.getTileType()->isSailable() ||
			         tile.getTileType()->isShip()) {
				if (m->swims() && tile.getTileType()->isSwimable())
					retval = DIR_ADD_TO_MASK(d, retval);
				if (m->sails() && tile.getTileType()->isSailable())
					retval = DIR_ADD_TO_MASK(d, retval);
				if (m->canMoveOntoPlayer() && tile.getTileType()->isShip())
					retval = DIR_ADD_TO_MASK(d, retval);
			}
			// Ghosts and other incorporeal creatures
			else if (m->isIncorporeal()) {
				// can move anywhere but onto water, unless of course the creature can swim
				if (!(tile.getTileType()->isSwimable() ||
				        tile.getTileType()->isSailable()))
					retval = DIR_ADD_TO_MASK(d, retval);
			}
			// Walking creatures
			else if (m->walks()) {
				if (tile.getTileType()->canWalkOn(d) &&
				        prev_tile.getTileType()->canWalkOff(d) &&
				        tile.getTileType()->isCreatureWalkable())
					retval = DIR_ADD_TO_MASK(d, retval);
			}
			// Creatures that can move onto player
			else if (ontoAvatar && m->canMoveOntoPlayer()) {

				// Tile should be transport
				if (tile.getTileType()->isShip() && m->swims())
					retval = DIR_ADD_TO_MASK(d, retval);
			}
		}
	}

	return retval;
}

bool Map::move(Object *obj, Direction d) {
	MapCoords new_coords = obj->getCoords();
	if (new_coords.move(d) != obj->getCoords()) {
		obj->setCoords(new_coords);
		return true;
	}
	return false;
}

void Map::alertGuards() {
	ObjectDeque::iterator i;
	const Creature *m;

	// Switch all the guards to attack mode
	for (i = _objects.begin(); i != _objects.end(); i++) {
		m = creatureMgr->getByTile((*i)->getTile());
		if (m && (m->getId() == GUARD_ID || m->getId() == LORDBRITISH_ID))
			(*i)->setMovementBehavior(MOVEMENT_ATTACK_AVATAR);
	}
}

MapCoords Map::getLabel(const Common::String &name) const {
	Std::map<Common::String, MapCoords>::const_iterator i = _labels.find(name);
	if (i == _labels.end())
		return MapCoords::nowhere();

	return i->_value;
}

bool Map::fillMonsterTable() {
	ObjectDeque::iterator current;
	Object *obj;
	ObjectDeque monsters;
	ObjectDeque other_creatures;
	ObjectDeque inanimate_objects;
	Object empty;

	int nCreatures = 0;
	int nObjects = 0;
	int i;

	for (int idx = 0; idx < MONSTERTABLE_SIZE; ++idx)
		_monsterTable[idx].clear();

	/**
	 * First, categorize all the objects we have
	 */
	for (current = _objects.begin(); current != _objects.end(); current++) {
		obj = *current;

		// Moving objects first
		if ((obj->getType() == Object::CREATURE) && (obj->getMovementBehavior() != MOVEMENT_FIXED)) {
			Creature *c = dynamic_cast<Creature *>(obj);
			assert(c);
			// Whirlpools and storms are separated from other moving objects
			if (c->getId() == WHIRLPOOL_ID || c->getId() == STORM_ID)
				monsters.push_back(obj);
			else
				other_creatures.push_back(obj);
		} else inanimate_objects.push_back(obj);
	}

	/**
	 * Add other monsters to our whirlpools and storms
	 */
	while (other_creatures.size() && nCreatures < MONSTERTABLE_CREATURES_SIZE) {
		monsters.push_back(other_creatures.front());
		other_creatures.pop_front();
	}

	/**
	 * Add empty objects to our list to fill things up
	 */
	while (monsters.size() < MONSTERTABLE_CREATURES_SIZE)
		monsters.push_back(&empty);

	/**
	 * Finally, add inanimate objects
	 */
	while (inanimate_objects.size() && nObjects < MONSTERTABLE_OBJECTS_SIZE) {
		monsters.push_back(inanimate_objects.front());
		inanimate_objects.pop_front();
	}

	/**
	 * Fill in the blanks
	 */
	while (monsters.size() < MONSTERTABLE_SIZE)
		monsters.push_back(&empty);

	/**
	 * Fill in our monster table
	 */
	TileMap *base = g_tileMaps->get("base");
	for (i = 0; i < MONSTERTABLE_SIZE; i++) {
		Coords c = monsters[i]->getCoords(),
			prevc = monsters[i]->getPrevCoords();

		_monsterTable[i]._tile = base->untranslate(monsters[i]->getTile());
		_monsterTable[i]._x = c.x;
		_monsterTable[i]._y = c.y;
		_monsterTable[i]._prevTile = base->untranslate(monsters[i]->getPrevTile());
		_monsterTable[i]._prevX = prevc.x;
		_monsterTable[i]._prevY = prevc.y;
	}

	return true;
}

MapTile Map::translateFromRawTileIndex(int raw) const {
	assertMsg(_tileMap != nullptr, "tilemap hasn't been set");

	return _tileMap->translate(raw);
}

uint Map::translateToRawTileIndex(MapTile &tile) const {
	return _tileMap->untranslate(tile);
}

} // End of namespace Ultima4
} // End of namespace Ultima
