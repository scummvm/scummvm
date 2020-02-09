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

#ifndef TITANIC_DOORBOT_H
#define TITANIC_DOORBOT_H

#include "titanic/npcs/true_talk_npc.h"

namespace Titanic {

class CDoorbot : public CTrueTalkNPC {
	DECLARE_MESSAGE_MAP;
	bool MovieEndMsg(CMovieEndMsg *msg);
	bool OnSummonBotMsg(COnSummonBotMsg *msg);
	bool TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg);
	bool DoorbotNeededInHomeMsg(CDoorbotNeededInHomeMsg *msg);
	bool DoorbotNeededInElevatorMsg(CDoorbotNeededInElevatorMsg *msg);
	bool LeaveViewMsg(CLeaveViewMsg *msg);
	bool TimerMsg(CTimerMsg *msg);
	bool NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg);
	bool NPCPlayIdleAnimationMsg(CNPCPlayIdleAnimationMsg *msg);
	bool PutBotBackInHisBoxMsg(CPutBotBackInHisBoxMsg *msg);
	bool DismissBotMsg(CDismissBotMsg *msg);
	bool MovieFrameMsg(CMovieFrameMsg *msg);
	bool TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg);
	bool TextInputMsg(CTextInputMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool ActMsg(CActMsg *msg);
private:
	static int _v1;
	static int _v2;
private:
	int _introMovieNum;
	int _timerId;
	int _field110;
	int _field114;
public:
	CLASSDEF;
	CDoorbot();

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

#endif /* TITANIC_DOORBOT_H */
