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

#include "immortal/room.h"

namespace Immortal {

Room::Room(uint8 x, uint8 y, RoomFlag f)
	: _xPos(x)
	, _yPos(y)
	, _flags(f)
	, _candleTmp(0)
	, _randomSource("Immortal") {
}

void Room::addMonster() {
	//_monsters->push_back(new Monster());
}

void Room::removeMonster() {
	//_monsters->pop_back();
}

void Room::addObject() {
	//_objects->push_back(new Object());
}

void Room::removeObject() {
	//_objects->pop_back();
}

Common::Array<Monster> Room::getMonsterList() {
	return _monsters;
}

Common::Array<Object> Room::getObjectList() {
	return _objects;
}

void Room::getXY(uint16 &x, uint16 &y) {
	x <<= 2;
	y <<= 2;
}

void Room::getCell(uint16 &x, uint16 &y) {
	x >>= 3;
	y >>= 3;
}

void Room::setHole() {}

void Room::drawContents(uint16 vX, uint16 vY) {
	flameDrawAll(vX, vY);
	//sparkDrawAll();
	//bulletDrawAll();
	//genSpriteDrawAll();
	//loop over monsters and draw each
	//loop over objects and draw each
	//doorDrawAll();
}

bool Room::getWideWallNormal(uint8 x, uint8 y, uint8 xPrev, uint8 yPrev, int id, int spacing) {
	return true;
}

bool Room::getWallNormal(uint8 x, uint8 y, uint8 xPrev, uint8 yPrev, int id) {
	return true;
}

} // namespace immortal

















