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

#include "titanic/pet_control/pet_control.h"
#include "titanic/game_manager.h"
#include "titanic/game_state.h"

namespace Titanic {

void CPetControl::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeQuotedLine(_string2, indent);

	saveSubObjects(file, indent);
	CGameObject::save(file, indent);
}

void CPetControl::load(SimpleFile *file) {
	int val = file->readNumber();
	isValid();
	
	if (!val) {
		_fieldBC = file->readNumber();
		_string1 = file->readString();
		_string2 = file->readString();
		
		loadSubObjects(file, 0);
	}

	CGameObject::load(file);
}

bool CPetControl::isValid() const {
	return _convSection.isValid() && _roomsSection.isValid()
		&& _remoteSection.isValid() && _invSection.isValid()
		&& _sub5.isValid() && _sub6.isValid()
		&& _sub7.isValid() && _sub8.isValid();
}

void CPetControl::loadSubObjects(SimpleFile *file, int param) {
	_convSection.load(file, param);
	_roomsSection.load(file, param);
	_remoteSection.load(file, param);
	_invSection.load(file, param);
	_sub5.load(file, param);
	_sub6.load(file, param);
	_sub7.load(file, param);
	_sub8.load(file, param);
}

void CPetControl::saveSubObjects(SimpleFile *file, int indent) const {
	_convSection.save(file, indent);
	_roomsSection.save(file, indent);
	_remoteSection.save(file, indent);
	_invSection.save(file, indent);
	_sub5.save(file, indent);
	_sub6.save(file, indent);
	_sub7.save(file, indent);
	_sub8.save(file, indent);
}

void CPetControl::proc26() {
	warning("TODO: CPetControl::proc26");
}

void CPetControl::postLoad() {
	warning("TODO: CPetControl::postLoad");
}

void CPetControl::enterNode(CNodeItem *node) {
	getGameManager()->_gameState.enterNode();
}

void CPetControl::enterRoom(CRoomItem *room) {
	_roomsSection.enterRoom(room);
	_remoteSection.enterRoom(room);
}

void CPetControl::clear() {
	_field1394 = 0;
	_string2.clear();
}

bool CPetControl::fn1(int val) {
	warning("TODO: CPetControl::fn1");
	return false;
}

void CPetControl::fn2(int val) {
	warning("TODO: CPetControl::fn2");
}

void CPetControl::fn3(int val) {
	warning("TODO: CPetControl::fn3");
}

void CPetControl::fn4() {
	warning("TODO: CPetControl::fn4");
}

} // End of namespace Titanic
