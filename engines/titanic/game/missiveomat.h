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

enum MissiveOMatMode {
	MMODE_USERNAME = 1,
	MMODE_PASSWORD = 2,
	MMODE_DENIED = 3,
	MMODE_LOGGED_IN = 4,
	MMODE_5 = 5
};

enum MissiveOMatAccount {
	NO_ACCOUNT = -1, LEOVINUS = 0, SCRALIONTIS = 1, BROBOSTIGON = 2
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
	MissiveOMatMode _mode;
	int _totalMessages;
	int _messageNum;
	CString _username;
	CString _password;
	MissiveOMatAccount _account;
public:
	CLASSDEF;
	CMissiveOMat();

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

#endif /* TITANIC_MISSIVEOMAT_H */
