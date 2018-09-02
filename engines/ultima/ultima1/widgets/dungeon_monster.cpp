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

#include "ultima/ultima1/widgets/dungeon_monster.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/utils.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

void DungeonMonster::synchronize(Common::Serializer &s) {
	DungeonWidget::synchronize(s);
	Creature::synchronize(s);
	s.syncAsUint16LE(_monsterId);
}

bool DungeonMonster::isBlockingView() const {
	return _monsterId != MONSTER_INVISIBLE_SEEKER && _monsterId != MONSTER_MIMIC
		&& _monsterId != MONSTER_GELATINOUS_CUBE;
}

void DungeonMonster::draw(Shared::DungeonSurface &s, uint distance) {
	if (distance < 5) {
		if (_monsterId == MONSTER_GELATINOUS_CUBE) {
			s.drawWall(distance);
			s.drawLeftEdge(distance);
			s.drawRightEdge(distance);
		} else {
			Ultima1Game *game = static_cast<Ultima1Game *>(g_vm->_game);
			Widgets::DungeonWidget::drawWidget(s, _monsterId, distance, game->_edgeColor);
		}
	}
}

void DungeonMonster::update(bool isPreUpdate) {
	assert(isPreUpdate);
	Point playerPos = _map->_playerWidget->_position;
	Point delta = playerPos - _position;
	int distance = ABS(delta.x) + ABS(delta.y);

	if (distance == 1) {
		attack();
	} else if (distance < 8) {
		movement();
	}
}

void DungeonMonster::movement() {
	if (attackDistance())
		// Dungeon monsters don't move if they're already in attack range
		return;

	Point playerPos = _map->_playerWidget->_position;
	Point delta = playerPos - _position;

	if (delta.x != 0 && canMoveTo(Point(_position.x + SGN(delta.x), _position.y)))
		_position.x += SGN(delta.x);
	else if (delta.y != 0 && canMoveTo(Point(_position.x, _position.y + SGN(delta.y))))
		_position.y += SGN(delta.y);
}

Shared::Maps::MapWidget::CanMove DungeonMonster::canMoveTo(const Point &destPos) {
	Shared::Maps::MapWidget::CanMove result = MapWidget::canMoveTo(destPos);
	if (result != UNSET)
		return result;

	return DungeonMonster::canMoveTo(_map, this, destPos);
}

Shared::Maps::MapWidget::CanMove DungeonMonster::canMoveTo(Shared::Maps::MapBase *map, MapWidget *widget, const Point &destPos) {
	// Get the details of the position
	Shared::Maps::MapTile currTile, destTile;
	
	map->getTileAt(map->getPosition(), &currTile);
	map->getTileAt(destPos, &destTile);

	// Can't move onto certain dungeon tile types
	if (destTile._isWall || destTile._isSecretDoor || destTile._isBeams)
		return NO;

	// Can't move to directly adjoining doorway cells (they'd be in parralel to each other, not connected)
	if (destTile._isDoor && currTile._isDoor)
		return NO;

	return YES;
}

void DungeonMonster::attack() {
	Ultima1Game *game = static_cast<Ultima1Game *>(_game);
	Point playerPos = _map->_playerWidget->_position;
	//Point delta = playerPos - _position;
	Shared::Character *c = _game->_party._currentCharacter;
	uint threshold, damage;
	bool isHit = true;

	// Get tile details for both the player and the attacking creature
	Maps::U1MapTile playerTile,creatureTile;
	_map->getTileAt(playerPos, &playerTile);
	_map->getTileAt(_position, &creatureTile);

	if (playerTile._isBeams || (creatureTile._isDoor && (playerTile._isDoor || playerTile._isWall || playerTile._isSecretDoor)))
		return;

	// Write attack line
	addInfoMsg(Common::String::format(game->_res->ATTACKED_BY, _name.c_str()));
	_game->playFX(3);

	threshold = (c->_stamina / 2) + (c->_equippedArmor * 8) + 56;

	if (_game->getRandomNumber(1, 255) > threshold) {
		threshold = _game->getRandomNumber(1, 255);
		damage = (_monsterId * _monsterId) + _map->getLevel();
		if (damage > 255) {
			damage = _game->getRandomNumber(_monsterId + 1, 255);
		}

		if (_monsterId == MONSTER_GELATINOUS_CUBE && c->isArmorEquipped()) {
			addInfoMsg(game->_res->ARMOR_DESTROYED);
			c->_armor[c->_equippedArmor]._quantity--;
			c->removeArmor();
			isHit = false;
		} else if (_monsterId == MONSTER_GREMLIN) {
			addInfoMsg(game->_res->GREMLIN_STOLE);
			c->_food /= 2;
			isHit = false;
		} else if (_monsterId == MONSTER_MIND_WHIPPER && threshold < 128) {
			addInfoMsg(game->_res->MENTAL_ATTACK);
			c->_intelligence = (c->_intelligence / 2) + 5;
			isHit = false;
		} else if (_monsterId == MONSTER_THIEF) {
			// Thief will steal the first spare weapon player has that isn't equipped
			for (int weaponNum = 1; weaponNum < (int)c->_weapons.size(); ++weaponNum) {
				if (weaponNum != c->_equippedWeapon && c->_weapons[weaponNum]._quantity > 0) {
					// TODO: May need to worry about word wrapping long line
					addInfoMsg(Common::String::format(game->_res->THIEF_STOLE,
						Shared::isVowel(c->_weapons[weaponNum]._longName.firstChar()) ? game->_res->AN : game->_res->A
					));
					c->_weapons[weaponNum]._quantity--;
					break;
				}
			}
		}

		if (isHit) {
			addInfoMsg(Common::String::format("%s %2d %s", game->_res->HIT, damage, game->_res->DAMAGE));
			c->_hitPoints -= damage;
		}
	} else {
		addInfoMsg(game->_res->MISSED);
	}
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
