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

#include "math/utils.h"
#include "darkseed/player.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

Player::Player() {
	_cPlayerSprites.load("cplayer.nsp");
	_gPlayerSprites.load("gplayer.nsp");
	_connectorList.resize(Room::MAX_CONNECTORS);
}

const Sprite &Player::getSprite(int frameNo) {
	if (g_engine->_room->isGiger()) {
		return _gPlayerSprites.getSpriteAt(frameNo);
	} else {
		return _cPlayerSprites.getSpriteAt(frameNo);
	}
}

bool Player::loadAnimations(const Common::Path &filename) {
	return _animations.load(filename);
}

uint8 playerSpriteIndexDirectionTbl[] = { 24,  26,  28,  26 };
bool BYTE_ARRAY_2c85_41eb[] = { false, false, false, true };
uint16 walkFrameOffsetTbl[] = { 0,   8,  16,   8 };

void Player::updateSprite() {
	if (!_playerIsChangingDirection) {
		if ((_direction == 3) || (_direction == 1)) {
			g_engine->_player_sprite_related_2c85_82f3 = BYTE_ARRAY_2c85_41eb[_direction];
		}
		if (_position.x == _walkTarget.x && _position.y == _walkTarget.y && !_heroMoving) {
			_frameIdx = playerSpriteIndexDirectionTbl[_direction];
		} else {
			_frameIdx = _playerWalkFrameIdx + walkFrameOffsetTbl[_direction];
		}
		if (_direction == 2) {
			if (_position.x < _walkTarget.x) {
				g_engine->_player_sprite_related_2c85_82f3 = true;
			} else if (_walkTarget.x < _position.x) {
				g_engine->_player_sprite_related_2c85_82f3 = false;
			}
		}
		if (_direction == 0) {
			if (_walkTarget.x < _position.x) {
				g_engine->_player_sprite_related_2c85_82f3 = true;
			} else if (_position.x < _walkTarget.x) {
				g_engine->_player_sprite_related_2c85_82f3 = false;
			}
		}
	} else {
		g_engine->_player_sprite_related_2c85_82f3 = 4 < _playerSpriteWalkIndex_maybe;
		if (g_engine->_player_sprite_related_2c85_82f3) {
			_frameIdx = 0x20 - _playerSpriteWalkIndex_maybe;
		} else {
			_frameIdx = _playerSpriteWalkIndex_maybe + 0x18;
		}
	}
}
bool Player::isAtPosition(int x, int y) const {
	return _position.x == x && _position.y == y;
}

bool Player::isAtWalkTarget() const {
	return _position == _walkTarget;
}

void Player::changeDirection(int16 oldDir, int16 newDir) {
	if (oldDir != newDir) {
		_playerIsChangingDirection = true;
		_playerSpriteWalkIndex_maybe = (int16)(oldDir * 2);
		_playerNewFacingDirection_maybe = (int16)(newDir * 2);
		_playerWalkFrameDeltaOffset = 1;
		if (oldDir < 4) {
			switch (oldDir) {
			case 0:
				if (newDir == 3) {
					_playerWalkFrameDeltaOffset = -1;
				}
				break;
			case 1:
				if (newDir == 0) {
					_playerWalkFrameDeltaOffset = -1;
				}
				break;
			case 2:
				if (newDir == 1) {
					_playerWalkFrameDeltaOffset = -1;
				}
				break;
			case 3:
				if (newDir == 2) {
					_playerWalkFrameDeltaOffset = -1;
				}
				break;
			}
		}
	}
}

void Player::playerFaceWalkTarget() {
	int previousDirection;
	int xDelta;
	int yDelta;

	previousDirection = _direction;
	if (_position.x < _walkTarget.x) {
		xDelta = _walkTarget.x - _position.x;
	} else {
		xDelta = _position.x - _walkTarget.x;
	}
	if (_position.y < _walkTarget.y) {
		yDelta = _walkTarget.y - _position.y;
	} else {
		yDelta = _position.y - _walkTarget.y;
	}
	if (yDelta * 2 <= xDelta) {
		if (_position.x < _walkTarget.x) {
			_direction = 1;
		} else if (_walkTarget.x < _position.x) {
			_direction = 3;
		}
	} else if (_position.y < _walkTarget.y) {
		_direction = 2;
	} else {
		_direction = 0;
	}
	changeDirection(previousDirection, _direction);
	updateSprite();
	_positionLong = _position;
}

void Player::calculateWalkTarget() {
	_heroMoving = true;
	_playerWalkFrameIdx = 0;
	_walkPathIndex = -1;
	_numConnectorsInWalkPath = 0;
	int selectedObjNum = 0;
	if (g_engine->_actionMode == kPointerAction) {
		selectedObjNum = g_engine->_room->getRoomExitAtCursor();
	}

	if (selectedObjNum == 0) {
		_walkTarget.x = g_engine->_cursor.getX();
		_walkTarget.y = g_engine->_cursor.getY();
	} else {
		int currentRoomNumber = g_engine->_room->_roomNumber;
		if (currentRoomNumber == 34 || (currentRoomNumber > 18 && currentRoomNumber < 24)) {
			g_engine->_previousRoomNumber = currentRoomNumber;
			if (currentRoomNumber == 34) {
				g_engine->changeToRoom(33);
			} else {
				g_engine->changeToRoom(28);
			}
			return;
		}
		g_engine->_room->getWalkTargetForObjectType_maybe(selectedObjNum);
	}

	if (_walkTarget.y > 237) {
		_walkTarget.y = 238;
	}

	if (!g_engine->_room->canWalkAtLocation(_walkTarget.x, _walkTarget.y)) {
		int ty = _walkTarget.y;
		for (; !g_engine->_room->canWalkAtLocation(_walkTarget.x, ty) && ty <= 233;) {
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
		Common::Point connector = getClosestUnusedConnector(_walkTarget.x, _walkTarget.y);
		if (connector.x == -1 && connector.y == -1) {
			return;
		}
		int connectorToTargetDist = Math::hypotenuse(connector.x - _walkTarget.x, connector.y - _walkTarget.y);
		int playerToTargetDist = Math::hypotenuse(_position.x - _walkTarget.x, _position.y - _walkTarget.y);
		if (connectorToTargetDist < playerToTargetDist) {
			if (g_engine->_room->canWalkInLineToTarget(_position.x, _position.y, connector.x, connector.y)) {
				_finalTarget = _walkTarget;
				_walkTarget = connector;
			} else {
				Common::Point tmpDest = _walkTarget;
				_walkTarget = connector;
				if (_numConnectorsInWalkPath > 0 && _numConnectorsInWalkPath < Room::MAX_CONNECTORS - 1 && _connectorList[_numConnectorsInWalkPath - 1] != connector) {
					_connectorList[_numConnectorsInWalkPath] = connector;
					_numConnectorsInWalkPath++;
				}
				_finalTarget = tmpDest;
			}
		}
	} else {
		createConnectorPathToDest();
	}
}

int Player::getWidth() {
	return getSprite(_frameIdx)._width;
}

int Player::getHeight() {
	return getSprite(_frameIdx)._height;
}

void Player::updatePlayerPositionAfterRoomChange() {
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
			g_engine->_scaledSpriteHeight = 5;
		} else {
			g_engine->_room->calculateScaledSpriteDimensions(getWidth(), getHeight(), _position.y + g_engine->_scaledSpriteHeight);
		}
		_position.y += g_engine->_scaledSpriteHeight;
		if (_position.y > 0xee) {
			_position.y = 0xee;
		}
		if (_position.x > 0x27b) {
			_position.x = 0x27b;
		}

		int yUp = _position.y;
		int yDown = _position.y;
		while (!g_engine->_room->canWalkAtLocation(_position.x, yUp) && yUp < 0xee) {
			yUp++;
		}
		while (!g_engine->_room->canWalkAtLocation(_position.x, yDown) && yDown > 0x28) {
			yDown--;
		}
		if (yUp - _position.y < _position.y - yDown) {
			_position.y = yUp;
		} else {
			_position.y = yDown;
		}
	} else {
		g_engine->_room->calculateScaledSpriteDimensions(getWidth(), getHeight(), _position.y + g_engine->_scaledSpriteHeight);
		_position.y += g_engine->_scaledSpriteHeight;
		if (_position.y > 0xee) {
			_position.y = 0xee;
		}

		int yUp = _position.y;
		int yDown = _position.y;
		while (!g_engine->_room->canWalkAtLocation(_position.x, yUp) && yUp < 0xee) {
			yUp++;
		}
		while (!g_engine->_room->canWalkAtLocation(_position.x, yDown) && yDown > 0x28) {
			yDown--;
		}
		if (yUp - _position.y < _position.y - yDown) {
			_position.y = yUp;
		} else {
			_position.y = yDown;
		}
	}
}

void Player::createConnectorPathToDest() {
	constexpr Common::Point noConnectorFound(-1, -1);
	Common::Point origWalkTarget = _walkTarget;
	Common::Point startPoint = _position;
	if (g_engine->_room->_roomNumber != 5 || _position.x > 320) {
		startPoint = _walkTarget;
		_walkTarget = _position;
	}
	_numConnectorsInWalkPath = 0;
	Common::Point connector;
	if (!g_engine->_room->canWalkAtLocation(startPoint.x, startPoint.y)) {
		connector = getClosestUnusedConnector(startPoint.x, startPoint.y);
	} else {
		connector = getClosestUnusedConnector(startPoint.x, startPoint.y, true);
	}
	if (connector == noConnectorFound) {
		if (g_engine->_room->_roomNumber != 5 || _position.x > 320) {
			_walkTarget = origWalkTarget;
		}
		return;
	}

	_walkPathIndex = 0;
	_connectorList[_numConnectorsInWalkPath] = connector;
	_numConnectorsInWalkPath++;

	while (_numConnectorsInWalkPath < Room::MAX_CONNECTORS && connector != noConnectorFound) {
		if (g_engine->_room->canWalkInLineToTarget(connector.x, connector.y, _walkTarget.x, _walkTarget.y)) {
			break;
		}
		connector = getClosestUnusedConnector(connector.x, connector.y, true);
		if (connector == _walkTarget) {
			break;
		}
		if (connector != noConnectorFound) {
			_connectorList[_numConnectorsInWalkPath] = connector;
			_numConnectorsInWalkPath++;
		}
	}

	if (g_engine->_room->_roomNumber != 5 || _position.x > 320) {
		reverseConnectorList();
		_walkTarget = origWalkTarget;
	}
	OptimisePath();
	if (g_engine->_room->_roomNumber == 5 && _position.x < 321) {
		_finalTarget = _walkTarget;
	} else {
		_finalTarget = origWalkTarget;
	}
	_walkTarget = _connectorList[0];
}

Common::Point Player::getClosestUnusedConnector(int16 x, int16 y, bool mustHaveCleanLine) {
	Common::Point closestPoint = {-1, -1};
	int closestDist = 5000;
	for (auto &roomConnector : g_engine->_room->_connectors) {
		bool containsPoint = false;
		for (int i = 0; i < _numConnectorsInWalkPath; i++) {
			if (_connectorList[i] == roomConnector) {
				containsPoint = true;
			}
		}
		if (!containsPoint) {
			int dist = Math::hypotenuse((roomConnector.x - x), (roomConnector.y - y));
			if (dist < closestDist) {
				if (!mustHaveCleanLine || g_engine->_room->canWalkInLineToTarget(x, y, roomConnector.x, roomConnector.y)) {
					closestPoint = roomConnector;
					closestDist = dist;
				}
			}
		}
	}
	return closestPoint;
}

void Player::walkToNextConnector() {
	if (_walkPathIndex == -1) {
		return;
	}
	if (_walkPathIndex + 1 < _numConnectorsInWalkPath) {
		_walkPathIndex++;
		_walkTarget = _connectorList[_walkPathIndex];
	} else {
		_walkTarget = _finalTarget;
		_walkPathIndex = -1;
	}
	playerFaceWalkTarget();
}

void Player::draw() {
	if (g_engine->_debugShowWalkPath) {
		if (_walkPathIndex != -1) {
			for (int i = _walkPathIndex; i < _numConnectorsInWalkPath; i++) {
				if (i == _walkPathIndex) {
					g_engine->_screen->drawLine(_position.x, _position.y, _connectorList[i].x, _connectorList[i].y, 2);
				} else {
					g_engine->_screen->drawLine(_connectorList[i].x, _connectorList[i].y, _connectorList[i - 1].x, _connectorList[i - 1].y, 2);
				}
			}
			g_engine->_screen->drawLine(_connectorList[_numConnectorsInWalkPath - 1].x, _connectorList[_numConnectorsInWalkPath - 1].y, _finalTarget.x, _finalTarget.y, 2);
		}
	}
}

void Player::reverseConnectorList() {
	for (int i = 0; i < _numConnectorsInWalkPath / 2; i++) {
		SWAP(_connectorList[i], _connectorList[_numConnectorsInWalkPath - 1 - i]);
	}
}

void Player::OptimisePath() {
	if (g_engine->_room->_roomNumber != 7 && g_engine->_room->_roomNumber != 32) {
		while (_numConnectorsInWalkPath > 1) {
			if (g_engine->_room->canWalkInLineToTarget(_connectorList[_numConnectorsInWalkPath - 2].x, _connectorList[_numConnectorsInWalkPath - 2].y, _walkTarget.x, _walkTarget.y)) {
				_numConnectorsInWalkPath--;
			} else {
				break;
			}
		}
	}
}

static constexpr uint8 _closerroom[10] = {
	0, 5, 0, 9,
	0, 0, 5, 6,
	7, 6
};

void Player::setplayertowardsbedroom() {
	if (g_engine->_animation->_isPlayingAnimation_maybe) {
		return;
	}
	Common::Point currentCursor = g_engine->_cursor.getPosition();
	uint8 currentRoomNumber = g_engine->_room->_roomNumber;
	if (currentRoomNumber == 0) {
		Common::Point target = {223, 190};
		g_engine->_cursor.setPosition(target);
	} else {
		uint8 local_a = 0;
		if (currentRoomNumber < 10) {
			local_a = _closerroom[currentRoomNumber];
		} else if (currentRoomNumber == 13) {
			local_a = 61;
		} else if (currentRoomNumber == 61) {
			local_a = 5;
		} else if (currentRoomNumber == 62) {
			local_a = 8;
		}
		if (currentRoomNumber == 6 && g_engine->_objectVar[137] == 2) {
			local_a = 10;
		}

		Common::Point exitPosition = g_engine->_room->getExitPointForRoom(local_a);
		g_engine->_cursor.setPosition(exitPosition);
		uint16 exitObjNum = g_engine->_room->getRoomExitAtCursor();
		g_engine->_room->getWalkTargetForObjectType_maybe(exitObjNum);

		g_engine->_cursor.setPosition(_walkTarget);
	}

	calculateWalkTarget();
	playerFaceWalkTarget();
	g_engine->_cursor.setPosition(currentCursor);
}

} // End of namespace Darkseed
