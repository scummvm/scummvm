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

#ifndef TITANIC_TT_INPUT_H
#define TITANIC_TT_INPUT_H

#include "titanic/true_talk/tt_npc_script.h"
#include "titanic/true_talk/tt_room_script.h"
#include "titanic/true_talk/tt_string.h"

namespace Titanic {

class CScriptHandler;

class TTinputSubBase {
public:
	int _field0;
	int _field4;
	int _field8;
	int _fieldC;
	int _field10;
	int _field14;
	int _field18;
	int _field1C;
	int _field20;
	int _field24;
public:
	TTinputSubBase();
};

class TTinputSub : public TTinputSubBase {
public:
};

class TTinput {
private:
	CScriptHandler *_owner;
	TTinputSub _sub;
	int _field2C;
	int _inputCtr;
	int _field34;
	int _field38;
	TTstring _string2;
	int _field4C;
	TTroomScript *_roomScript;
	TTnpcScript *_npcScript;
	int _field58;
	int _field5C;
	int _status;
public:
	TTstring _line;
public:
	TTinput(int inputCtr, const TTstring &line, CScriptHandler *owner,
		TTroomScript *roomScript, TTnpcScript *npcScript);
};

} // End of namespace Titanic

#endif /* TITANIC_TT_INPUT_H */
