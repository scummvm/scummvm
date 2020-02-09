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

#ifndef TITANIC_HEAD_SLOT_H
#define TITANIC_HEAD_SLOT_H

#include "titanic/core/game_object.h"

namespace Titanic {

class CHeadSlot : public CGameObject {
	DECLARE_MESSAGE_MAP;
	bool AddHeadPieceMsg(CAddHeadPieceMsg *msg);
	bool SenseWorkingMsg(CSenseWorkingMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool LeaveViewMsg(CLeaveViewMsg *msg);
	bool LoadSuccessMsg(CLoadSuccessMsg *msg);
	bool TimerMsg(CTimerMsg *msg);
	bool ActMsg(CActMsg *msg);
	bool MouseDragStartMsg(CMouseDragStartMsg *msg);
public:
	static bool _titaniaWoken;
public:
	bool _occupied;
	CString _senseState;
	CString _target;
	int _timerDuration;
	int _frameNum1;
	int _frameNum2;
	int _frameNum3;
	int _frameNum4;
	bool _workingFlag;
public:
	CLASSDEF;
	CHeadSlot();

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

#endif /* TITANIC_HEAD_SLOT_H */
