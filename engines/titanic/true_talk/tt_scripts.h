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

#ifndef TITANIC_TT_SCRIPTS_H
#define TITANIC_TT_SCRIPTS_H

#include "titanic/core/list.h"
#include "titanic/true_talk/tt_named_script.h"
#include "titanic/true_talk/tt_room_script.h"

namespace Titanic {

class CTitleEngine;

class TTNamedScriptListItem : public ListItem {
public:
	TTNamedScript *_script;
	TTRoomScript *_roomScript;
public:
	TTNamedScriptListItem() : _script(nullptr), _roomScript(nullptr) {}
	TTNamedScriptListItem(TTNamedScript *script, TTRoomScript *roomScript) :
		_script(script), _roomScript(roomScript) {}
	virtual ~TTNamedScriptListItem() { delete _script; }
};

PTR_LIST_ITEM(TTRoomScript);

class TTNamedScriptList : public List<TTNamedScriptListItem> {
public:
	TTNamedScript *findById(int charId) const;
};

class TTRoomScriptList : public List<TTRoomScriptListItem> {
public:
	TTRoomScript *findById(int scriptId) const;
};

class TTScripts {
private:
	TTNamedScriptList _namedScripts;
	TTRoomScriptList _roomScripts;
	CTitleEngine *_titleEngine;
	int _field24;
	int _field28;
private:
	/**
	 * Add a named script to the named scripts list
	 */
	void addScript(TTNamedScript *script, int charId);

	/**
	 * Add an unnamed script to the unnamed scripts list
	 */
	void addScript(TTRoomScript *script);
public:
	TTScripts(CTitleEngine *titleEngine);

	/**
	 * Return a pointer to the specified room script
	 */
	TTRoomScript *getRoomScript(int scriptId) const;

	/**
	 * Return a pointer to the specified named character script
	 */
	TTNamedScript *getNamedScript(int charId) const;
};

} // End of namespace Titanic

#endif /* TITANIC_TT_CHARACTERS_H */
