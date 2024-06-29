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
#include "common/math.h"
#include "darkseed.h"

Darkseed::Player::Player() {
	_cPlayerSprites.load("cplayer.nsp");
	_gPlayerSprites.load("gplayer.nsp");
	_connectorList.resize(Room::MAX_CONNECTORS);
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
uint16 walkFrameOffsetTbl[] = { 0,   8,  16,   8 };

void Darkseed::Player::updateSprite() {
	if (!_playerIsChangingDirection) {
		if ((_direction == 3) || (_direction == 1)) {
			g_engine->player_sprite_related_2c85_82f3 = BYTE_ARRAY_2c85_41eb[_direction];
		}
		if (_position.x == _walkTarget.x && _position.y == _walkTarget.y && !_heroMoving) {
			_frameIdx = playerSpriteIndexDirectionTbl[_direction];
		} else {
			_frameIdx = playerWalkFrameIdx + walkFrameOffsetTbl[_direction];
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
bool Darkseed::Player::isAtPosition(int x, int y) const {
	return _position.x == x && _position.y == y;
}

bool Darkseed::Player::isAtWalkTarget() const {
	return _position ==_walkTarget;
}

void Darkseed::Player::changeDirection(int16 oldDir, int16 newDir) {
	if (oldDir != newDir) {
		_playerIsChangingDirection = true;
		playerSpriteWalkIndex_maybe = (int16)(oldDir * 2);
		playerNewFacingDirection_maybe = (int16)(newDir * 2);
		playerWalkFrameDeltaOffset = 1;
		if (oldDir < 4) {
			switch (oldDir) {
			case 0 :
				if (newDir == 3) {
					playerWalkFrameDeltaOffset = -1;
				}
				break;
			case 1 :
				if (newDir == 0) {
					playerWalkFrameDeltaOffset = -1;
				}
				break;
			case 2 :
				if (newDir == 1) {
					playerWalkFrameDeltaOffset = -1;
				}
				break;
			case 3 :
				if (newDir == 2) {
					playerWalkFrameDeltaOffset = -1;
				}
				break;
			}
		}
	}
}

void Darkseed::Player::playerFaceWalkTarget() {
	int previousDirection;
	int xDelta;
	int yDelta;
  
	previousDirection = _direction;
	if (_position.x < _walkTarget.x) {
		xDelta = _walkTarget.x - _position.x;
	}
	else {
		xDelta = _position.x - _walkTarget.x;
	}
	if (_position.y < _walkTarget.y) {
		yDelta = _walkTarget.y - _position.y;
	}
	else {
		yDelta = _position.y - _walkTarget.y;
	}
	if (yDelta * 2 <= xDelta) {
		if (_position.x < _walkTarget.x) {
			_direction = 1;
		}
		else if (_walkTarget.x < _position.x) {
			_direction = 3;
		}
	}
	else if (_position.y < _walkTarget.y) {
		_direction = 2;
	}
	else {
		_direction = 0;
	}
	changeDirection(previousDirection,_direction);
	updateSprite();
	_positionLong = _position;
}

void Darkseed::Player::calculateWalkTarget() {
	_heroMoving = true;
	playerWalkFrameIdx = 0;
	walkPathIndex = -1;
	numConnectorsInWalkPath = 0;
	int selectedObjNum = 0;
	if(g_engine->_actionMode == PointerAction) {
		selectedObjNum = g_engine->_room->getRoomExitAtCursor();
	}

	if (selectedObjNum == 0) {
		_walkTarget.x = g_engine->_cursor.getX();
		_walkTarget.y = g_engine->_cursor.getY();
	} else {
		int currentRoomNumber = g_engine->_room->_roomNumber;
		if (currentRoomNumber == 0x22 || (currentRoomNumber > 0x12 && currentRoomNumber < 0x18)) {
			g_engine->_previousRoomNumber = currentRoomNumber;
			if (currentRoomNumber == 0x22) {
				// TODO FUN_171d_88f4_change_rooms_maybe(0); // also set current room to 0x21
			} else {
				// TODO FUN_171d_88f4_change_rooms_maybe(0); // also set current room to 0x1c
			}
			return;
		}
		g_engine->_room->getWalkTargetForObjectType_maybe(selectedObjNum);
	}

	if (_walkTarget.y > 0xed) {
		_walkTarget.y = 0xee;
	}

	if (!g_engine->_room->canWalkAtLocation(_walkTarget.x, _walkTarget.y)) {
		int ty = _walkTarget.y;
		for (; !g_engine->_room->canWalkAtLocation(_walkTarget.x, ty) && ty <= 0xe9;) {
			ty += 4;
		}
		if (ty < 235) {
			_walkTarget.y = ty;
		}
	}

	if (g_engine->_room->canWalkInLineToTarget(_position.x, _position.y, _walkTarget.x, _walkTarget.y)) {
		return;
	}

	if (!g_engine->_room->canWalkAtLocation(_walkTarget.x, _walkTarget.y)) {
		// TODO find closest connector
	} else {
		createConnectorPathToDest();
	}
	// TODO more logic here.
}

int Darkseed::Player::getWidth() {
	return getSprite(_frameIdx).width;
}

int Darkseed::Player::getHeight() {
	return getSprite(_frameIdx).height;
}

void Darkseed::Player::updatePlayerPositionAfterRoomChange() {
	int currentRoomNumber = g_engine->_room->_roomNumber;
	g_engine->_room->calculateScaledSpriteDimensions(getWidth(), getHeight(), _position.y);
	if (currentRoomNumber == 0x29 && g_engine->_previousRoomNumber == 0x2c) {
		_position = Common::Point(0x13d, 0xa9);
	} else if (currentRoomNumber == 0x2c && g_engine->_previousRoomNumber == 0x29) {
		_position = Common::Point(0x16e, 0xb8);
	} else if (_direction == 0 || ((currentRoomNumber == 0x29 || currentRoomNumber == 0x2c) && _direction == 2)) {
		_position.y = 0xec;
		g_engine->_room->calculateScaledSpriteDimensions(getWidth(), getHeight(), _position.y);
		while (!g_engine->_room->canWalkAtLocation(_position.x, _position.y + 3) && _position.y > 100) {
			_position.y--;
		}
	} else if (_direction == 2) {
		while (!g_engine->_room->canWalkAtLocation(_position.x, _position.y - 5) && _position.y < 0xee && currentRoomNumber != 0x29 && currentRoomNumber != 0x2c) {
			_position.y++;
		}
	} else if (_direction == 3) {
		if (currentRoomNumber == 0x20 || currentRoomNumber == 0x1a) {
			g_engine->scaledSpriteHeight = 5;
		} else {
			g_engine->_room->calculateScaledSpriteDimensions(getWidth(), getHeight(), _position.y + g_engine->scaledSpriteHeight);
		}
		_position.y += g_engine->scaledSpriteHeight;
		if (_position.y > 0xee) {
			_position.y = 0xee;
		}
		if (_position.x > 0x27b) {
			_position.x = 0x27b;
		}

		int yUp = _position.y;
		int yDown = _position.y;
		while(!g_engine->_room->canWalkAtLocation(_position.x, yUp) && yUp < 0xee) {
			yUp++;
		}
		while(!g_engine->_room->canWalkAtLocation(_position.x, yDown) && yDown > 0x28) {
			yDown--;
		}
		if (yUp - _position.y < _position.y - yDown) {
			_position.y = yUp;
		} else {
			_position.y = yDown;
		}
	} else {
		g_engine->_room->calculateScaledSpriteDimensions(getWidth(), getHeight(), _position.y + g_engine->scaledSpriteHeight);
		_position.y += g_engine->scaledSpriteHeight;
		if (_position.y > 0xee) {
			_position.y = 0xee;
		}

		int yUp = _position.y;
		int yDown = _position.y;
		while(!g_engine->_room->canWalkAtLocation(_position.x, yUp) && yUp < 0xee) {
			yUp++;
		}
		while(!g_engine->_room->canWalkAtLocation(_position.x, yDown) && yDown > 0x28) {
			yDown--;
		}
		if (yUp - _position.y < _position.y - yDown) {
			_position.y = yUp;
		} else {
			_position.y = yDown;
		}
	}
}

void Darkseed::Player::updateBedAutoWalkSequence() {
	// TODO updateBedAutoWalkSequence.
}

void Darkseed::Player::createConnectorPathToDest() {
	constexpr Common::Point noConnectorFound(-1,-1);
	Common::Point origWalkTarget = _walkTarget;
	if (g_engine->_room->_roomNumber != 5 || _position.x > 320) {
		_walkTarget = _position;
	}
	numConnectorsInWalkPath = 0;
	Common::Point connector;
	if (!g_engine->_room->canWalkAtLocation(_position.x, _position.y)) {
		connector = getClosestUnusedConnector(_position.x, _position.y);
	} else {
		connector = getClosestUnusedConnector(_position.x, _position.y, true);
	}
	if (connector == noConnectorFound) {
		if (g_engine->_room->_roomNumber != 5 || _position.x > 320) {
			_walkTarget = origWalkTarget;
		}
		return;
	}

	walkPathIndex = 0;
	_connectorList[numConnectorsInWalkPath] = connector;
	numConnectorsInWalkPath++;

	while (numConnectorsInWalkPath < Room::MAX_CONNECTORS && connector != noConnectorFound) {
		if (g_engine->_room->canWalkInLineToTarget(connector.x, connector.y, _walkTarget.x, _walkTarget.y)) {
			break;
		}
		connector = getClosestUnusedConnector(connector.x, connector.y, true);
		if (connector == _walkTarget) {
			break;
		}
		if (connector != noConnectorFound) {
			_connectorList[numConnectorsInWalkPath] = connector;
			numConnectorsInWalkPath++;
		}
	}

	if (g_engine->_room->_roomNumber != 5 || _position.x > 320) {
//		FlipConnectorList();
		_walkTarget = origWalkTarget;
	}
	// Optimize();
	if (g_engine->_room->_roomNumber == 5 && _position.x < 321) {
		_finalTarget = _walkTarget;
	} else {
		_finalTarget = origWalkTarget;
	}
	_walkTarget = _connectorList[0];
}

Common::Point Darkseed::Player::getClosestUnusedConnector(int16 x, int16 y, bool mustHaveCleanLine) {
	Common::Point closestPoint = {-1, -1};
	int closestDist = 5000;
	for (auto &roomConnector : g_engine->_room->_connectors) {
		bool containsPoint = false;
		for (int i = 0; i < numConnectorsInWalkPath; i++) {
			if (_connectorList[i] == roomConnector) {
				containsPoint = true;
			}
		}
		if (!containsPoint) {
			int dist = Common::hypotenuse(ABS(roomConnector.x - x), ABS(roomConnector.y - y));
			if (dist < closestDist) {
				if (!mustHaveCleanLine || g_engine->_room->canWalkInLineToTarget(x, y, roomConnector.x, roomConnector.y)) {
					closestPoint = roomConnector;
				}
			}
		}
	}
	return closestPoint;
}

void Darkseed::Player::walkToNextConnector() {
	if (walkPathIndex == -1) {
		return;
	}
	if (walkPathIndex + 1 < numConnectorsInWalkPath) {
		walkPathIndex++;
		_walkTarget = _connectorList[walkPathIndex];
	} else {
		_walkTarget = _finalTarget;
		walkPathIndex = -1;
	}
	playerFaceWalkTarget();
}
