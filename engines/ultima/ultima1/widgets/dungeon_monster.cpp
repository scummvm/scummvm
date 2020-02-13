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
#include "ultima/ultima1/maps/map_dungeon.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/utils.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

DungeonMonster::DungeonMonster(Ultima1Game *game, Maps::MapBase *map, DungeonWidgetId monsterId,
		int hitPoints, const Point &pt) :
		DungeonWidget(game, map, monsterId, pt), Shared::Maps::DungeonCreature(game, map, hitPoints) {
	_name = getGame()->_res->DUNGEON_MONSTER_NAMES[_widgetId];
}

DungeonMonster::DungeonMonster(Ultima1Game *game, Maps::MapBase *map) :
		DungeonWidget(game, map), Shared::Maps::DungeonCreature(game, map) {
}

void DungeonMonster::synchronize(Common::Serializer &s) {
	DungeonWidget::synchronize(s);
	Creature::synchronize(s);
	s.syncAsUint16LE(_widgetId);

	if (s.isLoading())
		_name = getGame()->_res->DUNGEON_MONSTER_NAMES[_widgetId];
}

bool DungeonMonster::isBlockingView() const {
	return _widgetId != MONSTER_INVISIBLE_SEEKER && _widgetId != MONSTER_MIMIC
		&& _widgetId != MONSTER_GELATINOUS_CUBE;
}

void DungeonMonster::draw(Shared::DungeonSurface &s, uint distance) {
	if (distance < 5) {
		if (_widgetId == MONSTER_GELATINOUS_CUBE) {
			s.drawWall(distance);
			s.drawLeftEdge(distance);
			s.drawRightEdge(distance);
		} else {
			DungeonWidget::draw(s, distance);
		}
	}
}

void DungeonMonster::update(bool isPreUpdate) {
	assert(isPreUpdate);
	Point playerPos = _map->_playerWidget->_position;
	Point delta = playerPos - _position;
	int distance = ABS(delta.x) + ABS(delta.y);

	if (distance == 1) {
		attackParty();
	} else if (distance < 8) {
		movement();
	}
}

void DungeonMonster::movement() {
	if (attackDistance())
		// Dungeon monsters don't move if they're already in attack range
		return;

	Point playerPos = _map->_playerWidget->_position;
	Point diff = playerPos - _position;

	if (diff.x != 0 && canMoveTo(Point(_position.x + SGN(diff.x), _position.y)))
		_position.x += SGN(diff.x);
	else if (diff.y != 0 && canMoveTo(Point(_position.x, _position.y + SGN(diff.y))))
		_position.y += SGN(diff.y);
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

void DungeonMonster::attackParty() {
	Ultima1Game *game = static_cast<Ultima1Game *>(_game);
	Point playerPos = _map->_playerWidget->_position;
	//Point delta = playerPos - _position;
	Shared::Character &c = *_game->_party;
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

	threshold = (c._stamina / 2) + (c._equippedArmour * 8) + 56;

	if (_game->getRandomNumber(1, 255) > threshold) {
		threshold = _game->getRandomNumber(1, 255);
		damage = (_widgetId * _widgetId) + _map->getLevel();
		if (damage > 255) {
			damage = _game->getRandomNumber(_widgetId + 1, 255);
		}

		if (_widgetId == MONSTER_GELATINOUS_CUBE && c.isArmourEquipped()) {
			addInfoMsg(game->_res->ARMOR_DESTROYED);
			c._armour[c._equippedArmour]->decrQuantity();
			c.removeArmour();
			isHit = false;
		} else if (_widgetId == MONSTER_GREMLIN) {
			addInfoMsg(game->_res->GREMLIN_STOLE);
			c._food /= 2;
			isHit = false;
		} else if (_widgetId == MONSTER_MIND_WHIPPER && threshold < 128) {
			addInfoMsg(game->_res->MENTAL_ATTACK);
			c._intelligence = (c._intelligence / 2) + 5;
			isHit = false;
		} else if (_widgetId == MONSTER_THIEF) {
			// Thief will steal the first spare weapon player has that isn't equipped
			for (int weaponNum = 1; weaponNum < (int)c._weapons.size(); ++weaponNum) {
				if (weaponNum != c._equippedWeapon && !c._weapons[weaponNum]->empty()) {
					// TODO: May need to worry about word wrapping long line
					addInfoMsg(Common::String::format(game->_res->THIEF_STOLE,

						Shared::isVowel(c._weapons[weaponNum]->_longName.firstChar()) ? game->_res->AN : game->_res->A
					));
					c._weapons[weaponNum]->decrQuantity();
					break;
				}
			}
		}

		if (isHit) {
			addInfoMsg(Common::String::format("%s %2d %s", game->_res->HIT, damage, game->_res->DAMAGE));
			c._hitPoints -= damage;
		}
	} else {
		addInfoMsg(game->_res->MISSED);
	}
}

void DungeonMonster::attackMonster(uint effectNum, uint agility, uint damage) {
	Ultima1Game *game = static_cast<Ultima1Game *>(_game);
	Maps::MapDungeon *map = static_cast<Maps::MapDungeon *>(_map);
	Point currPos = map->getPosition();
	Maps::U1MapTile playerTile, monsTile;
	map->getTileAt(currPos, &playerTile);
	map->getTileAt(_position, &monsTile);

	bool flag = true;
	if (!playerTile._isDoor) {
		if (!monsTile._isHallway && !monsTile._isLadderUp && !monsTile._isLadderDown)
			flag = false;
	}

	if (game->getRandomNumber(1, 100) <= agility && !playerTile._isWall && !playerTile._isSecretDoor
			&& !playerTile._isBeams && flag) {
		// Play effect and add hit message
		game->playFX(effectNum);
		if (damage != ITS_OVER_9000)
			addInfoMsg(Common::String::format("%s ", game->_res->HIT), false);

		if ((int)damage < _hitPoints) {
			addInfoMsg(Common::String::format("%u %s!", damage, game->_res->DAMAGE));
			_hitPoints -= damage;
		} else {
			addInfoMsg(Common::String::format("%s %s", _name.c_str(),
				damage == ITS_OVER_9000 ? game->_res->DESTROYED : game->_res->KILLED));
			monsterDead();

			// Give some treasure
			uint amount = game->getRandomNumber(2, map->getLevel() * 3 + (uint)_widgetId + 10);
			addInfoMsg(game->_res->THOU_DOST_FIND);
			game->giveTreasure(amount, 0);

			// Give experience
			Shared::Character &c = *game->_party;
			uint experience = game->getRandomNumber(2, map->getLevel() * map->getLevel() + 10);
			c._experience += experience;
			map->_dungeonExitHitPoints = MIN(map->_dungeonExitHitPoints + experience * 2, 9999U);

			// Delete the monster and create a new one
			map->removeWidget(this);
			map->spawnMonster();
		}
	} else {
		// Attack missed
		addInfoMsg(game->_res->MISSED);
	}
}

void DungeonMonster::monsterDead() {
	int index;
	switch (_widgetId) {
	case MONSTER_BALRON:
		index = 8;
		break;
	case MONSTER_CARRION_CREEPER:
		index = 4;
		break;
	case MONSTER_LICH:
		index = 6;
		break;
	case MONSTER_GELATINOUS_CUBE:
		index = 2;
		break;
	default:
		index = 0;
		break;
	}

	if (index) {
		// Mark monster-based quests as complete if in progress
		Ultima1Game *game = static_cast<Ultima1Game *>(_game);
		game->_quests[8 - index].complete();
	}
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
