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
	CString _v1;
	CString _v2;
	CString _v3;
	CString _v4;
	CString _v5;
	CString _v6;
	CString _v7;
	CString _v8;
	CString _v9;
	CString _v10;
	CString _v11;
	CString _v12;
	int _v13;
	int _v14;
};

class CSGTStateRoom : public CBackground {
	DECLARE_MESSAGE_MAP;
	bool EnterRoomMsg(CEnterRoomMsg *msg);
protected:
	static CSGTStateRoomStatics *_statics;
protected:
	int _fieldE0;
	int _fieldE4;
	int _fieldE8;
	int _fieldEC;
	int _fieldF0;
public:
	CLASSDEF;
	CSGTStateRoom();
	static void init();
	static void deinit();

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

#endif /* TITANIC_SGT_STATE_ROOM_H */
