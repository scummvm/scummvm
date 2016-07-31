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

#ifndef TITANIC_LIFT_H
#define TITANIC_LIFT_H

#include "titanic/game/transport/transport.h"
#include "titanic/messages/messages.h"

namespace Titanic {

class CLift : public CTransport {
	DECLARE_MESSAGE_MAP;
	bool EnterRoomMsg(CEnterRoomMsg *msg);
private:
	static int _v1;
	static int _v2;
	static int _v3;
	static int _v4;
	static int _v5;
	static int _v6;

	int _fieldF8;
public:
	CLASSDEF;
	CLift() : CTransport(), _fieldF8(1) {}

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);
};

} // End of namespace Titanic

#endif /* TITANIC_LIFT_H */
