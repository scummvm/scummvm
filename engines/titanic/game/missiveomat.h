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

#ifndef TITANIC_MISSIVEOMAT_H
#define TITANIC_MISSIVEOMAT_H

#include "titanic/core/game_object.h"

namespace Titanic {

enum MissiveOMatAction {
	MESSAGE_NONE = 1, MESSAGE_SHOW = 2, NEXT_MESSAGE = 3, PRIOR_MESSAGE = 4,
	MESSAGE_5 = 5, MESSAGE_DOWN = 6, MESSAGE_UP = 7, REDRAW_MESSAGE = 8,
	MESSAGE_9 = 9
};

class CMissiveOMat : public CGameObject {
	DECLARE_MESSAGE_MAP;
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool KeyCharMsg(CKeyCharMsg *msg);
	bool TimerMsg(CTimerMsg *msg);
	bool MissiveOMatActionMsg(CMissiveOMatActionMsg *msg);
	bool LeaveViewMsg(CLeaveViewMsg *msg);
private:
	CString _welcomeMessages[3];
	CString _messages[58];
	CString _from[58];
	CString _to[58];
private:
	void loadArray(CString *arr, const CString &resName, int count);
public:
	int _mode;
	int _totalMessages;
	int _messageNum;
	CString _string1;
	CString _string2;
	int _personIndex;
public:
	CLASSDEF;
	CMissiveOMat();

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

#endif /* TITANIC_MISSIVEOMAT_H */
