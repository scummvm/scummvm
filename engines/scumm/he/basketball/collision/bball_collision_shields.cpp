/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "scumm/he/intern_he.h"
#include "scumm/he/basketball/geo_translations.h"
#include "scumm/he/basketball/court.h"

#include "scumm/he/basketball/collision/bball_collision_support_obj.h"
#include "scumm/he/basketball/collision/bball_collision_object.h"
#include "scumm/he/basketball/collision/bball_collision_sphere.h"
#include "scumm/he/basketball/collision/bball_collision_box.h"
#include "scumm/he/basketball/collision/bball_collision_cylinder.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"
#include "scumm/he/basketball/collision/bball_collision_node.h"
#include "scumm/he/basketball/collision/bball_collision_tree.h"
#include "scumm/he/basketball/collision/bball_collision_shields.h"

namespace Scumm {

CCollisionShieldVector::CCollisionShieldVector() : _shieldUpCount(0) {
	CCollisionBox westShield;
	CCollisionBox northShield;
	CCollisionBox eastShield;
	CCollisionBox southShield;
	CCollisionBox topShield;

	CCollisionBox westShield2;
	CCollisionBox northShield2;
	CCollisionBox eastShield2;
	CCollisionBox southShield2;

	westShield.minPoint.x = 0 - SHIELD_DEPTH;
	westShield.maxPoint.x = 0;
	westShield.minPoint.y = 0 - SHIELD_EXTENSION;
	westShield.maxPoint.y = MAX_WORLD_Y + SHIELD_EXTENSION;
	westShield.minPoint.z = 0;
	westShield.maxPoint.z = SHIELD_HEIGHT;
	westShield._description = "West Shield";
	westShield._collisionEfficiency = 0.5F;
	westShield._friction = 0.3F;
	westShield._objectID = WEST_SHIELD_ID;
	westShield._ignore = true;

	westShield2.minPoint.x = 0 - SHIELD_DEPTH - BUFFER_WIDTH;
	westShield2.maxPoint.x = 0 - BUFFER_WIDTH;
	westShield2.minPoint.y = 0 - SHIELD_EXTENSION;
	westShield2.maxPoint.y = MAX_WORLD_Y + SHIELD_EXTENSION;
	westShield2.minPoint.z = 0;
	westShield2.maxPoint.z = SHIELD_HEIGHT;
	westShield2._description = "West Shield 2";
	westShield2._collisionEfficiency = 0.5F;
	westShield2._friction = 0.3F;
	westShield2._objectID = BACKUP_WEST_SHIELD_ID;
	westShield2._ignore = true;

	northShield.minPoint.x = 0 - SHIELD_EXTENSION;
	northShield.maxPoint.x = MAX_WORLD_X + SHIELD_EXTENSION;
	northShield.minPoint.y = MAX_WORLD_Y;
	northShield.maxPoint.y = MAX_WORLD_Y + SHIELD_DEPTH;
	northShield.minPoint.z = 0;
	northShield.maxPoint.z = SHIELD_HEIGHT;
	northShield._description = "North Shield";
	northShield._collisionEfficiency = 0.5F;
	northShield._friction = 0.3F;
	northShield._objectID = NORTH_SHIELD_ID;
	northShield._ignore = true;

	northShield2.minPoint.x = 0 - SHIELD_EXTENSION;
	northShield2.maxPoint.x = MAX_WORLD_X + SHIELD_EXTENSION;
	northShield2.minPoint.y = MAX_WORLD_Y + BUFFER_WIDTH;
	northShield2.maxPoint.y = MAX_WORLD_Y + SHIELD_DEPTH + BUFFER_WIDTH;
	northShield2.minPoint.z = 0;
	northShield2.maxPoint.z = SHIELD_HEIGHT;
	northShield2._description = "North Shield 2";
	northShield2._collisionEfficiency = 0.5F;
	northShield2._friction = 0.3F;
	northShield2._objectID = BACKUP_NORTH_SHIELD_ID;
	northShield2._ignore = true;

	eastShield.minPoint.x = MAX_WORLD_X;
	eastShield.maxPoint.x = MAX_WORLD_X + SHIELD_DEPTH;
	eastShield.minPoint.y = 0 - SHIELD_EXTENSION;
	eastShield.maxPoint.y = MAX_WORLD_Y + SHIELD_EXTENSION;
	eastShield.minPoint.z = 0;
	eastShield.maxPoint.z = SHIELD_HEIGHT;
	eastShield._description = "East Shield";
	eastShield._collisionEfficiency = 0.5F;
	eastShield._friction = 0.3F;
	eastShield._objectID = EAST_SHIELD_ID;
	eastShield._ignore = true;

	eastShield2.minPoint.x = MAX_WORLD_X + BUFFER_WIDTH;
	eastShield2.maxPoint.x = MAX_WORLD_X + SHIELD_DEPTH + BUFFER_WIDTH;
	eastShield2.minPoint.y = 0 - SHIELD_EXTENSION;
	eastShield2.maxPoint.y = MAX_WORLD_Y + SHIELD_EXTENSION;
	eastShield2.minPoint.z = 0;
	eastShield2.maxPoint.z = SHIELD_HEIGHT;
	eastShield2._description = "East Shield 2";
	eastShield2._collisionEfficiency = 0.5F;
	eastShield2._friction = 0.3F;
	eastShield2._objectID = BACKUP_EAST_SHIELD_ID;
	eastShield2._ignore = true;

	southShield.minPoint.x = 0 - SHIELD_EXTENSION;
	southShield.maxPoint.x = MAX_WORLD_X + SHIELD_EXTENSION;
	southShield.minPoint.y = 0 - SHIELD_DEPTH;
	southShield.maxPoint.y = 0;
	southShield.minPoint.z = 0;
	southShield.maxPoint.z = SHIELD_HEIGHT;
	southShield._description = "South Shield";
	southShield._collisionEfficiency = 0.5F;
	southShield._friction = 0.3F;
	southShield._objectID = SOUTH_SHIELD_ID;
	southShield._ignore = true;

	southShield2.minPoint.x = 0 - SHIELD_EXTENSION;
	southShield2.maxPoint.x = MAX_WORLD_X + SHIELD_EXTENSION;
	southShield2.minPoint.y = 0 - SHIELD_DEPTH - BUFFER_WIDTH;
	southShield2.maxPoint.y = 0 - BUFFER_WIDTH;
	southShield2.minPoint.z = 0;
	southShield2.maxPoint.z = SHIELD_HEIGHT;
	southShield2._description = "South Shield 2";
	southShield2._collisionEfficiency = 0.5F;
	southShield2._friction = 0.3F;
	southShield2._objectID = BACKUP_SOUTH_SHIELD_ID;
	southShield2._ignore = true;

	topShield.minPoint.x = 0 - SHIELD_EXTENSION;
	topShield.maxPoint.x = MAX_WORLD_X + SHIELD_EXTENSION;
	topShield.minPoint.y = 0 - SHIELD_EXTENSION;
	topShield.maxPoint.y = MAX_WORLD_Y + SHIELD_EXTENSION;
	topShield.minPoint.z = SHIELD_HEIGHT;
	topShield.maxPoint.z = SHIELD_HEIGHT + SHIELD_DEPTH;
	topShield._description = "Top Shield";
	topShield._collisionEfficiency = 0.5F;
	topShield._friction = 0.3F;
	topShield._objectID = TOP_SHIELD_ID;
	topShield._ignore = true;

	push_back(westShield);
	push_back(northShield);
	push_back(eastShield);
	push_back(southShield);
	push_back(topShield);

	push_back(westShield2);
	push_back(northShield2);
	push_back(eastShield2);
	push_back(southShield2);
}

CCollisionShieldVector::~CCollisionShieldVector() {
	CCollisionShieldVector::iterator shieldIt;

	for (shieldIt = begin(); shieldIt != end(); ++shieldIt) {
		shieldIt->_ignore = true;
	}
}

int LogicHEBasketball::u32_userRaiseShields(int shieldID) {
	assert(shieldID < MAX_SHIELD_COUNT || shieldID == ALL_SHIELD_ID);

	CCollisionShieldVector::iterator shieldIt;

	for (shieldIt = _vm->_basketball->_shields->begin(); shieldIt != _vm->_basketball->_shields->end(); ++shieldIt) {
		// Make sure we don't mess with the backup shields...
		if (shieldIt->_objectID < MAX_SHIELD_COUNT) {
			if (((shieldIt->_objectID == shieldID) || (shieldID == ALL_SHIELD_ID)) &&
				(shieldIt->_ignore == true)) {
				shieldIt->_ignore = false;
				++_vm->_basketball->_shields->_shieldUpCount;
			}
		}
	}

	if (shieldID == ALL_SHIELD_ID) {
		assert(_vm->_basketball->_shields->_shieldUpCount == MAX_SHIELD_COUNT);
	}

	return 1;
}

int LogicHEBasketball::u32_userLowerShields(int shieldID) {
	assert(shieldID < MAX_SHIELD_COUNT || shieldID == ALL_SHIELD_ID);

	CCollisionShieldVector::iterator shieldIt;

	for (shieldIt = _vm->_basketball->_shields->begin(); shieldIt != _vm->_basketball->_shields->end(); ++shieldIt) {
		// Make sure we don't mess with the backup shields...
		if (shieldIt->_objectID < MAX_SHIELD_COUNT) {
			if (((shieldIt->_objectID == shieldID) || (shieldID == ALL_SHIELD_ID)) &&
				(shieldIt->_ignore == false)) {
				shieldIt->_ignore = true;
				--_vm->_basketball->_shields->_shieldUpCount;
			}
		}
	}

	if (shieldID == ALL_SHIELD_ID) {
		assert(_vm->_basketball->_shields->_shieldUpCount == 0);
	}

	return 1;
}

int LogicHEBasketball::u32_userAreShieldsClear() {
	int shieldsAreClear = (_vm->_basketball->_shields->_shieldUpCount != MAX_SHIELD_COUNT);
	writeScummVar(_vm1->VAR_U32_USER_VAR_A, shieldsAreClear);
	return 1;
}

int LogicHEBasketball::u32_userShieldPlayer(int playerID, int shieldRadius) {
	CCollisionPlayer *player = _vm->_basketball->_court->getPlayerPtr(playerID);

	if (player->_shieldRadius < shieldRadius) {
		player->_shieldRadius += PLAYER_SHIELD_INCREMENT_VALUE;
		player->radius += PLAYER_SHIELD_INCREMENT_VALUE;
	}

	return 1;
}

int LogicHEBasketball::u32_userClearPlayerShield(int playerID) {
	CCollisionPlayer *player = _vm->_basketball->_court->getPlayerPtr(playerID);

	player->radius -= player->_shieldRadius;
	player->_shieldRadius = 0.0F;

	return 1;
}

} // End of namespace Scumm
