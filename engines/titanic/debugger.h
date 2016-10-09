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

#ifndef TITANIC_DEBUGGER_H
#define TITANIC_DEBUGGER_H

#include "common/scummsys.h"
#include "gui/debugger.h"
#include "titanic/core/room_item.h"
#include "titanic/core/node_item.h"
#include "titanic/core/view_item.h"

namespace Titanic {

class TitanicEngine;

class Debugger : public GUI::Debugger {
private:
	/**
	 * Converts a decimal or hexadecimal string into a number
	 */
	int strToInt(const char *s);

	/**
	 * Find a room by name or number
	 */
	CRoomItem *findRoom(const char *name);

	/**
	 * Find a node within a room by name or number
	 */
	CNodeItem *findNode(CRoomItem *room, const char *name);

	/**
	 * Find a view within a room node by name or number
	 */
	CViewItem *findView(CNodeItem *node, const char *name);

	/**
	 * List all the rooms in the game
	 */
	void listRooms();

	/**
	 * List the nodes within a room
	 */
	void listRoom(CRoomItem *room);

	/**
	 * List the views within a room node
	 */
	void listNode(CNodeItem *node);

	/**
	 * Dump a portion of the game project
	 */
	bool cmdDump(int argc, const char **argv);

	/**
	 * List room details, or jump to a specific view
	 */
	bool cmdRoom(int argc, const char **argv);

	/**
	 * Turn the PET on or off
	 */
	bool cmdPET(int argc, const char **argv);

	/**
	 * Item handling
	 */
	bool cmdItem(int argc, const char **argv);
protected:
	TitanicEngine *_vm;
public:
	Debugger(TitanicEngine *vm);
	virtual ~Debugger() {}
};

} // End of namespace Titanic

#endif	/* TITANIC_DEBUGGER_H */
