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

#include "titanic/core/mail_man.h"

namespace Titanic {

void CMailMan::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_value, indent);
	CGameObject::save(file, indent);
}

void CMailMan::load(SimpleFile *file) {
	file->readNumber();
	_value = file->readNumber();
	CGameObject::load(file);
}

CGameObject *CMailMan::getFirstObject() const {
	return dynamic_cast<CGameObject *>(getFirstChild());
}

CGameObject *CMailMan::getNextObject(CGameObject *prior) const {
	if (!prior || prior->getParent() != this)
		return nullptr;

	return dynamic_cast<CGameObject *>(prior->getNextSibling());
}

void CMailMan::addMail(CGameObject *obj, uint destRoomFlags) {
	obj->detach();
	obj->addUnder(this);
	setMailDest(obj, destRoomFlags);
}

void CMailMan::setMailDest(CGameObject *obj, uint roomFlags) {
	obj->_destRoomFlags = roomFlags;
	obj->_roomFlags = 0;
	obj->_isPendingMail = true;
}

CGameObject *CMailMan::findMail(uint roomFlags) const {
	for (CGameObject *obj = getFirstObject(); obj; obj = getNextObject(obj)) {
		if (obj->_isPendingMail && obj->_destRoomFlags == roomFlags)
			return obj;
	}

	return nullptr;
}

void CMailMan::sendMail(uint currRoomFlags, uint newRoomFlags) {
	for (CGameObject *obj = getFirstObject(); obj; obj = getNextObject(obj)) {
		if (obj->_isPendingMail && obj->_destRoomFlags == currRoomFlags) {
			obj->_isPendingMail = false;
			obj->_roomFlags = newRoomFlags;
			break;
		}
	}
}

} // End of namespace Titanic
