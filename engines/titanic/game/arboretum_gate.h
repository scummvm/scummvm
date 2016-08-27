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
	bool ChangeSeasonMsg(CChangeSeasonMsg *msg);
	bool ActMsg(CActMsg *msg);
	bool MovieEndMsg(CMovieEndMsg *msg);
	bool LeaveViewMsg(CLeaveViewMsg *msg);
	bool TurnOff(CTurnOff *msg);
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool TurnOn(CTurnOn *msg);
private:
	static int _v1;
	static int _initialFrame;
	static int _v3;
private:
	int _seasonNum;
	CString _viewName1;
	int _fieldF0;
	int _winterOffStartFrame;
	int _winterOffEndFrame;
	int _springOffStartFrame;
	int _springOffEndFrame;
	int _summerOffStartFrame2;
	int _summerOffEndFrame2;
	int _summerOffStartFrame1;
	int _summerOffEndFrame1;
	int _autumnOffStartFrame2;
	int _autumnOffEndFrame2;
	int _autumnOffStartFrame1;
	int _autumnOffEndFrame1;
	int _winterOnStartFrame;
	int _winterOnEndFrame;
	int _springOnStartFrame;
	int _springOnEndFrame;
	int _summerOnStartFrame1;
	int _summerOnEndFrame1;
	int _summerOnStartFrame2;
	int _summerOnEndFrame2;
	int _autumnOnStartFrame1;
	int _autumnOnEndFrame1;
	int _autumnOnStartFrame2;
	int _autumnOnEndFrame2;
	CString _viewName2;
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
