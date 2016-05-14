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

#ifndef TITANIC_TT_PARSER_H
#define TITANIC_TT_PARSER_H

#include "titanic/true_talk/tt_input.h"

namespace Titanic {

class CScriptHandler;

class TTparser {
private:
	int normalize(TTinput *input);
public:
	CScriptHandler *_owner;
	int _field4;
	TTinput *_input;
	int _fieldC;
	int _field10;
	int _field14;
	int _field18;
public:
	TTparser(CScriptHandler *owner) : _owner(owner), _field4(0),
		_input(nullptr), _fieldC(0), _field10(0), _field14(0), _field18(0) {}

	/**
	 * Gets passed a newly created input wrapper during conversation text processing
	 */
	int processInput(TTinput *input);
};

} // End of namespace Titanic

#endif /* TITANIC_TT_PARSER_H */
