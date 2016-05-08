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

#ifndef TITANIC_SCRIPT_HANDLER_H
#define TITANIC_SCRIPT_HANDLER_H

#include "titanic/true_talk/tt_npc_script.h"
#include "titanic/true_talk/tt_room_script.h"
#include "titanic/true_talk/tt_string.h"

namespace Titanic {

class CTitleEngine;

class CScriptHandler {
private:
	CTitleEngine *_owner;
	TTScriptBase *_script;
	int _field8;
	int _fieldC;
	int _field10;
	int _field14;
	int _field18;
	int _inputCtr;
	int _field20;
	int _field24;
	int _field28;
	int _field2C;
	int _field30;
public:
	CScriptHandler(CTitleEngine *owner, int val1, int val2);

	/**
	 * Set the character and room
	 */
	int scriptChanged(TTRoomScript *roomScript, TTNpcScript *npcScript, uint dialogueId);

	void processInput(TTRoomScript *roomScript, TTNpcScript *npcScript,
		const TTString &line);
};

} // End of namespace Titanic

#endif /* TITANIC_SCRIPT_HANDLER_H */
