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

#ifndef TITANIC_SGT_STATE_ROOM_H
#define TITANIC_SGT_STATE_ROOM_H

#include "titanic/core/background.h"
#include "titanic/messages/messages.h"

namespace Titanic {

struct CSGTStateRoomStatics {
	CString _bedhead;
	CString _bedfoot;
	CString _vase;
	CString _tv;
	CString _desk;
	CString _chestOfDrawers;
	CString _drawer;
	CString _armchair;
	CString _deskchair;
	CString _washstand;
	CString _basin;
	CString _toilet;
	bool _announcementFlag;
	uint _roomFlags;
};

class CSGTStateRoom : public CBackground {
	DECLARE_MESSAGE_MAP;
	bool ActMsg(CActMsg *msg);
	bool VisibleMsg(CVisibleMsg *msg);
	bool EnterRoomMsg(CEnterRoomMsg *msg);
	bool LeaveRoomMsg(CLeaveRoomMsg *msg);
protected:
	static CSGTStateRoomStatics *_statics;
protected:
	bool _isClosed;
	bool _displayFlag;
	int _savedFrame;
	bool _savedIsClosed;
	bool _savedVisible;
public:
	CLASSDEF;
	CSGTStateRoom();
	static void init();
	static void deinit();

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;
};

} // End of namespace Titanic

#endif /* TITANIC_SGT_STATE_ROOM_H */
