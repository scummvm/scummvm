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

#include "titanic/game/pet/pet_pellerator.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPETPellerator, CPETTransport)
	ON_MESSAGE(PETActivateMsg)
END_MESSAGE_MAP()

void CPETPellerator::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CPETTransport::save(file, indent);
}

void CPETPellerator::load(SimpleFile *file) {
	file->readNumber();
	CPETTransport::load(file);
}

bool CPETPellerator::PETActivateMsg(CPETActivateMsg *msg) {
	CStatusChangeMsg statusMsg;

	if (msg->_name == "PromenadeDeck")
		statusMsg._newStatus = 0;
	else if (msg->_name == "MusicRoom")
		statusMsg._newStatus = 1;
	else if (msg->_name == "Bar")
		statusMsg._newStatus = 2;
	else if (msg->_name == "TopOfWell")
		statusMsg._newStatus = 4;
	else if (msg->_name == "1stClassRestaurant")
		statusMsg._newStatus = 5;
	else if (msg->_name == "Arboretum")
		statusMsg._newStatus = 6;

	statusMsg.execute("PelleratorObject");
	return true;
}

} // End of namespace Titanic
