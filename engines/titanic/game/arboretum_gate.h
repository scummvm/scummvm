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

#ifndef TITANIC_ARBORETUM_GATE_H
#define TITANIC_ARBORETUM_GATE_H

#include "titanic/core/background.h"
#include "titanic/messages/messages.h"
#include "titanic/messages/mouse_messages.h"

namespace Titanic {

class CArboretumGate : public CBackground {
	DECLARE_MESSAGE_MAP;
	bool ActMsg(CActMsg *msg);
	bool LeaveViewMsg(CLeaveViewMsg *msg);
	bool TurnOff(CTurnOff *msg);
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool TurnOn(CTurnOn *msg);
	bool MovieEndMsg(CMovieEndMsg *msg);
private:
	static int _v1;
	static int _v2;
	static int _v3;
private:
	int _fieldE0;
	CString _string1;
	int _fieldE8;
	int _fieldEC;
	int _fieldF0;
	int _fieldF4;
	int _fieldF8;
	int _fieldFC;
	int _field100;
	int _field104;
	int _field108;
	int _field10C;
	int _field110;
	int _field114;
	int _field118;
	int _field11C;
	int _field120;
	int _field124;
	int _field128;
	int _field12C;
	int _field130;
	int _field134;
	int _field138;
	int _field13C;
	int _field140;
	int _field144;
	int _field148;
	int _field14C;
	int _field150;
	CString _string2;
public:
	CLASSDEF;
	CArboretumGate();

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

#endif /* TITANIC_ARBORETUM_GATE_H */
