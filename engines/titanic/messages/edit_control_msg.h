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

#ifndef TITANIC_EDIT_CONTROL_MSG_H
#define TITANIC_EDIT_CONTROL_MSG_H

#include "titanic/messages/message.h"

namespace Titanic {

class CEditControlMsg : public CMessage {
private:
	int _field4;
	int _field8;
	CString _string1;
	int _field18;
	int _field1C;
	int _field20;
public:
	CEditControlMsg() : _field4(0), _field8(0), _field18(0),
		_field1C(0), _field20(0) {}

	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "CEditControlMsg"; }
};

} // End of namespace Titanic

#endif /* TITANIC_EDIT_CONTROL_MSG_H */
