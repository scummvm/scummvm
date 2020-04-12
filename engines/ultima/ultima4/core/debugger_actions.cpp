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

#include "ultima/ultima4/core/debugger_actions.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/gfx/textcolor.h"
#include "ultima/ultima4/map/combat.h"

namespace Ultima {
namespace Ultima4 {

void DebuggerActions::summonCreature(const Common::String &name) {
	const Creature *m = NULL;
	Common::String creatureName = name;

	creatureName.trim();
	if (creatureName.empty()) {
		print("\n");
		return;
	}

	/* find the creature by its id and spawn it */
	unsigned int id = atoi(creatureName.c_str());
	if (id > 0)
		m = creatureMgr->getById(id);

	if (!m)
		m = creatureMgr->getByName(creatureName);

	if (m) {
		if (gameSpawnCreature(m))
			print("\n%s summoned!\n", m->getName().c_str());
		else
			print("\n\nNo place to put %s!\n\n", m->getName().c_str());

		return;
	}

	print("\n%s not found\n", creatureName.c_str());
}

Direction DebuggerActions::directionFromName(const Common::String &dirStr) {
	Common::String dir = dirStr;
	dir.toLowercase();

	if (dir == "up" || dir == "north")
		return DIR_NORTH;
	else if (dir == "down" || dir == "south")
		return DIR_SOUTH;
	else if (dir == "right" || dir == "east")
		return DIR_EAST;
	else if (dir == "left" || dir == "west")
		return DIR_WEST;

	return DIR_NONE;
}

bool DebuggerActions::destroyAt(const Coords &coords) {
	Object *obj = g_context->_location->_map->objectAt(coords);

	if (obj) {
		if (isCreature(obj)) {
			Creature *c = dynamic_cast<Creature *>(obj);
			screenMessage("%s Destroyed!\n", c->getName().c_str());
		} else {
			Tile *t = g_context->_location->_map->_tileset->get(obj->getTile()._id);
			screenMessage("%s Destroyed!\n", t->getName().c_str());
		}

		g_context->_location->_map->removeObject(obj);
		screenPrompt();

		return true;
	}

	return false;
}

bool DebuggerActions::attackAt(const Coords &coords) {
	Object *under;
	const Tile *ground;
	Creature *m;

	m = dynamic_cast<Creature *>(g_context->_location->_map->objectAt(coords));
	/* nothing attackable: move on to next tile */
	if (m == NULL || !m->isAttackable())
		return false;

	/* attack successful */
	/// TODO: CHEST: Make a user option to not make chests change battlefield
	/// map (1 of 2)
	ground = g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITH_GROUND_OBJECTS);
	if (!ground->isChest()) {
		ground = g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITHOUT_OBJECTS);
		if ((under = g_context->_location->_map->objectAt(g_context->_location->_coords)) &&
			under->getTile().getTileType()->isShip())
			ground = under->getTile().getTileType();
	}

	/* You're attacking a townsperson!  Alert the guards! */
	if ((m->getType() == Object::PERSON) && (m->getMovementBehavior() != MOVEMENT_ATTACK_AVATAR))
		g_context->_location->_map->alertGuards();

	/* not good karma to be killing the innocent.  Bad avatar! */
	if (m->isGood() || /* attacking a good creature */
			/* attacking a docile (although possibly evil) person in town */
		((m->getType() == Object::PERSON) && (m->getMovementBehavior() != MOVEMENT_ATTACK_AVATAR)))
		g_context->_party->adjustKarma(KA_ATTACKED_GOOD);

	CombatController *cc = new CombatController(CombatMap::mapForTile(ground, g_context->_party->getTransport().getTileType(), m));
	cc->init(m);
	cc->begin();
	return false;
}

} // End of namespace Ultima4
} // End of namespace Ultima
