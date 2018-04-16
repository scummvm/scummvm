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

#include "titanic/moves/restaurant_pan_handler.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CRestaurantPanHandler, CMovePlayerTo)
	ON_MESSAGE(ArmPickedUpFromTableMsg)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

bool CRestaurantPanHandler::_armPickedUp;

void CRestaurantPanHandler::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_armPickedUp, indent);
	file->writeQuotedLine(_armlessDestination, indent);
	file->writeQuotedLine(_armDestination, indent);

	CMovePlayerTo::save(file, indent);
}

void CRestaurantPanHandler::load(SimpleFile *file) {
	file->readNumber();
	_armPickedUp = file->readNumber();
	_armlessDestination = file->readString();
	_armDestination = file->readString();

	CMovePlayerTo::load(file);
}

bool CRestaurantPanHandler::ArmPickedUpFromTableMsg(CArmPickedUpFromTableMsg *msg) {
	_armPickedUp = true;
	return true;
}

bool CRestaurantPanHandler::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CString clipName = _armPickedUp ? _armDestination : _armlessDestination;
	changeView(_destination, clipName);
	return true;
}

} // End of namespace Titanic
