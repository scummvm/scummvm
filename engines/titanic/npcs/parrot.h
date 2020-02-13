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

#ifndef TITANIC_PARROT_H
#define TITANIC_PARROT_H

#include "titanic/npcs/true_talk_npc.h"
#include "titanic/moves/move_player_to.h"

namespace Titanic {

enum ParrotState {
	PARROT_IN_CAGE = 0, PARROT_1 = 1, PARROT_ESCAPED = 2,
	PARROT_MAILED = 3, PARROT_4 = 4
};

class CParrot : public CTrueTalkNPC {
	DECLARE_MESSAGE_MAP;
	bool ActMsg(CActMsg *msg);
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	bool MovieEndMsg(CMovieEndMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg);
	bool MouseDragStartMsg(CMouseDragStartMsg *msg);
	bool LeaveViewMsg(CLeaveViewMsg *msg);
	bool ParrotSpeakMsg(CParrotSpeakMsg *msg);
	bool NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg);
	bool NPCPlayIdleAnimationMsg(CNPCPlayIdleAnimationMsg *msg);
	bool FrameMsg(CFrameMsg *msg);
	bool MovieFrameMsg(CMovieFrameMsg *msg);
	bool PutParrotBackMsg(CPutParrotBackMsg *msg);
	bool PreEnterViewMsg(CPreEnterViewMsg *msg);
	bool PanningAwayFromParrotMsg(CPanningAwayFromParrotMsg *msg);
	bool LeaveRoomMsg(CLeaveRoomMsg *msg);
	bool TrueTalkNotifySpeechStartedMsg(CTrueTalkNotifySpeechStartedMsg *msg);
	bool TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg);
public:
	static bool _eatingChicken;
	static bool _takeOff;
	static bool _unused;
	static ParrotState _state;
	static bool _coreReplaced;
private:
	int _unused1;
	CString _carryParrot;
	bool _canDrag;
	int _unused2;
	uint _lastSpeakTime;
	int _newXp;
	int _newXc;
	bool _triedEatChicken;
	int _eatOffsetX;
	CMovePlayerTo *_panTarget;
private:
	/**
	 * Called for the Parrot to start or finish eating
	 */
	void setEatingChicken(bool eating);
public:
	CLASSDEF;
	CParrot();

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

#endif /* TITANIC_PARROT_H */
