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

#include "player.h"
#include "darkseed.h"

Darkseed::Player::Player() {
	_cPlayerSprites.load("cplayer.nsp");
	_gPlayerSprites.load("gplayer.nsp");
}

const Darkseed::Sprite &Darkseed::Player::getSprite(int frameNo) {
	// TODO switch sprite based on over or underworld.
	return _cPlayerSprites.getSpriteAt(frameNo);
}

bool Darkseed::Player::loadAnimations(const Common::String &filename) {
	return _animations.load(filename);
}

uint8 playerSpriteIndexDirectionTbl[] = { 24,  26,  28,  26 };
bool BYTE_ARRAY_2c85_41eb[] = { false, false, false, true };
uint16 BYTE_ARRAY_2c85_41e3[] = { 0,   8,  16,   8 };

void Darkseed::Player::updateSprite() {
	if (!_playerIsMoving_maybe) {
		if ((_direction == 3) || (_direction == 1)) {
			g_engine->player_sprite_related_2c85_82f3 = BYTE_ARRAY_2c85_41eb[_direction];
		}
		if (_position.x == _walkTarget.x && _position.y == _walkTarget.y && !g_engine->BoolEnum_2c85_811c) {
			_frameIdx = playerSpriteIndexDirectionTbl[_direction];
		} else {
			_frameIdx = g_engine->DAT_2c85_7dd7 + BYTE_ARRAY_2c85_41e3[_direction];
		}
		if (_direction == 2) {
			if (_position.x < _walkTarget.x) {
				g_engine->player_sprite_related_2c85_82f3 = true;
			}
			else if (_walkTarget.x < _position.x) {
				g_engine->player_sprite_related_2c85_82f3 = false;
			}
		}
		if (_direction == 0) {
			if (_walkTarget.x < _position.x) {
				g_engine->player_sprite_related_2c85_82f3 = true;
			}
			else if (_position.x < _walkTarget.x) {
				g_engine->player_sprite_related_2c85_82f3 = false;
			}
		}
	}
	else {
		g_engine->player_sprite_related_2c85_82f3 = 4 < playerSpriteWalkIndex_maybe;
		if (g_engine->player_sprite_related_2c85_82f3) {
			_frameIdx = 0x20 - playerSpriteWalkIndex_maybe;
		}
		else {
			_frameIdx = playerSpriteWalkIndex_maybe + 0x18;
		}
	}
}
