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

#ifndef TITANIC_SUCCUBUS_H
#define TITANIC_SUCCUBUS_H

#include "titanic/npcs/true_talk_npc.h"
#include "titanic/messages/pet_messages.h"

namespace Titanic {

enum SuccUBusAction { SA_SENT = 0, SA_FEATHERS = 1, SA_EATEN = 2,
	SA_BILGE_FEATHERS = 3, SA_BILGE_SENT = 4, SA_BILGE_EATEN = 5 };

class CSuccUBus : public CTrueTalkNPC {
	DECLARE_MESSAGE_MAP;
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	bool SubAcceptCCarryMsg(CSubAcceptCCarryMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool LeaveViewMsg(CLeaveViewMsg *msg);
	bool PETDeliverMsg(CPETDeliverMsg *msg);
	bool PETReceiveMsg(CPETReceiveMsg *msg);
	bool MovieEndMsg(CMovieEndMsg *msg);
	bool TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg);
	bool SignalObject(CSignalObject *msg);
	bool TurnOn(CTurnOn *msg);
	bool TurnOff(CTurnOff *msg);
	bool SUBTransition(CSUBTransition *msg);
	bool SetChevRoomBits(CSetChevRoomBits *msg);
	bool ActMsg(CActMsg *msg);
	bool MouseDragStartMsg(CMouseDragStartMsg *msg);
protected:
	static bool _isOn;
	static bool _motherBlocked;
	static bool _fuseboxOn;
protected:
	int _initialStartFrame;
	int _initialEndFrame;
	int _endingStartFrame;
	int _endingEndFrame;
	int _sendStartFrame;
	int _sendEndFrame;
	int _receiveStartFrame;
	int _receiveEndFrame;
	int _onStartFrame;
	int _onEndFrame;
	int _offStartFrame;
	int _offEndFrame;
	int _okStartFrame;
	int _okEndFrame;
	RoomFlagsComparison _flagsComparison;
	CGameObject *_mailP;
	int _afterReceiveStartFrame;
	int _afterReceiveEndFrame;
	int _trayOutStartFrame;
	int _trayOutEndFrame;
	SuccUBusAction _sendAction;
	bool _signalFlag;
	CString _signalTarget;
	int _startFrame1;
	int _endFrame1;
	Rect _rect1;
	int _field184;
	bool _mailPresent;
	Rect _rect2;
	bool _sendLost;
	int _soundHandle;
	bool _isChicken;
	bool _isFeathers;
	int _priorRandomVal1;
	int _priorRandomVal2;
	int _emptyStartFrame;
	int _emptyEndFrame;
	int _smokeStartFrame;
	int _smokeEndFrame;
	int _hoseStartFrame;
	int _hoseEndFrame;
	int _pumpingStartFrame;
	int _pumpingEndFrame;
	uint _destRoomFlags;
	bool _inProgress;
public:
	CLASSDEF;
	CSuccUBus();

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

#endif /* TITANIC_SUCCUBUS_H */
