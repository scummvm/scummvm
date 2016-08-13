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

#ifndef TITANIC_BARBOT_H
#define TITANIC_BARBOT_H

#include "titanic/npcs/true_talk_npc.h"
#include "titanic/messages/messages.h"

namespace Titanic {

class CBarbot : public CTrueTalkNPC {
	struct FrameRange {
		int _startFrame;
		int _endFrame;
		FrameRange() : _startFrame(0), _endFrame(0) {}
	};
	class FrameRanges : public Common::Array<FrameRange> {
	public:
		FrameRanges();
	};

	DECLARE_MESSAGE_MAP;
	bool ActMsg(CActMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool TurnOn(CTurnOn *msg);
	bool TurnOff(CTurnOff *msg);
	bool LeaveViewMsg(CLeaveViewMsg *msg);
	bool MovieEndMsg(CMovieEndMsg *msg);
	bool TrueTalkSelfQueueAnimSetMsg(CTrueTalkSelfQueueAnimSetMsg *msg);
	bool TrueTalkQueueUpAnimSetMsg(CTrueTalkQueueUpAnimSetMsg *msg);
	bool TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg);
	bool TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg);
	bool FrameMsg(CFrameMsg *msg);
	bool LoadSuccessMsg(CLoadSuccessMsg *msg);
	bool MovieFrameMsg(CMovieFrameMsg *msg);
	bool EnterRoomMsg(CEnterRoomMsg *msg);
	bool TimerMsg(CTimerMsg *msg);
private:
	static int _v0;
private:
	FrameRanges _frames;
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
	int _volume;
	int _frameNum;
	int _field148;
	int _field14C;
	int _field150;
	int _field154;
	int _field158;
	int _field15C;
	int _field160;
private:
	/**
	 * Plays a given range of movie frames
	 */
	void playRange(const FrameRange &range, uint flags = 0);
public:
	CLASSDEF;
	CBarbot();

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

#endif /* TITANIC_BARBOT_H */
