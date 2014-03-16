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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/player.h"

namespace MADS {

Player::Player(MADSEngine *vm): _vm(vm) {
	_action = nullptr;
	_direction = 8;
	_newDirection = 8;
	_destFacing = 0;
	_spritesLoaded = false;
	_spritesStart = _numSprites = 0;
	_stepEnabled = false;
	_visible = false;
	_visible3 = false;
	_special = 0;
	_ticksAmount = 0;
	_priorTimer = 0;
	_unk3 = _unk4 = 0;
	_forceRefresh = false;
}

void Player::reset() {
	_action = &_vm->_game->_scene._action;
	_destPos = _playerPos;
	_destFacing = 5;
	_newDirection = _direction;
	_moving = false;
	_v844C0 = _v844BE = 0;
	_next = 0;
	_routeCount = 0;

	_vm->_game->_v4 = 0;
	_action->_startWalkFlag = false;
	_action->_walkFlag = false;
}

void Player::loadSprites(const Common::String &prefix) {
	warning("TODO: Player::loadSprites");
}

void Player::turnToDestFacing() {
	if (_destFacing != 5)
		_newDirection = _destFacing;
}

void Player::moveComplete() {
	reset();
	_action->_inProgress = false;
}

void Player::setupFrame() {
	resetActionList();
	warning("TODO: Player::setupFrame");
}

void Player::updateFrame() {
	warning("TODO: Player::updateFrame");
}

void Player::update() {
	warning("TODO: Player::update");
}

void Player::resetActionList() {
	warning("TODO: Player::resetActionList");
}

void Player::setDest(const Common::Point &pt, int facing) {
	warning("TODO: Player::setDest");
}

void Player::nextFrame() {
	Scene &scene = _vm->_game->_scene;

	_priorTimer += _ticksAmount;
	if (scene._frameStartTime >= _priorTimer) {
		_priorTimer = scene._frameStartTime;
		if (_moving) {
			move();
		} else {
			idle();
		}

		postUpdate();
		update();
	}
}

void Player::move() {
	warning("TODO: Player::move");
}

void Player::idle() {
	warning("TODO: Player::idle");
}

void Player::postUpdate() {
	warning("TODO: Player::postUpdate");
}

} // End of namespace MADS
