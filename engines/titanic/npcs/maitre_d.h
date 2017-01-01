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

#ifndef TITANIC_MAITRED_H
#define TITANIC_MAITRED_H

#include "titanic/npcs/true_talk_npc.h"

namespace Titanic {

class CMaitreD : public CTrueTalkNPC {
	DECLARE_MESSAGE_MAP;
	bool RestaurantMusicChanged(CRestaurantMusicChanged *msg);
	bool TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool LeaveViewMsg(CLeaveViewMsg *msg);
	bool NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg);
	bool TimerMsg(CTimerMsg *msg);
	bool TrueTalkNotifySpeechStartedMsg(CTrueTalkNotifySpeechStartedMsg *msg);
	bool TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg);
	bool LoadSuccessMsg(CLoadSuccessMsg *msg);
	bool TextInputMsg(CTextInputMsg *msg);
	bool TriggerNPCEvent(CTriggerNPCEvent *msg);
private:
	static int _v1;
private:
	int _field108;
	CString _string2;
	int _field118;
	int _field11C;
	CString _string3;
	int _field12C;
	int _field130;
	int _field134;
	int _timerId;
public:
	CLASSDEF;
	CMaitreD();

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

#endif /* TITANIC_MAITRED_H */
