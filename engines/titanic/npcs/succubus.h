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
	static bool _enabled;
	static int _v1;
	static int _v2;
	static int _v3;
	static int _v4;
protected:
	int _startFrame8;
	int _endFrame8;
	int _startFrame11;
	int _endFrame11;
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
	int _field140;
	CGameObject *_mailP;
	int _afterReceiveStartFrame;
	int _afterReceiveEndFrame;
	int _startFrame12;
	int _endFrame12;
	int _field158;
	bool _field15C;
	CString _string2;
	int _startFrame1;
	int _endFrame1;
	Rect _rect1;
	int _field184;
	int _field188;
	Rect _rect2;
	int _field190;
	int _field194;
	int _field198;
	int _field19C;
	int _soundHandle;
	bool _isChicken;
	bool _isFeathers;
	int _field1AC;
	int _field1B0;
	int _emptyStartFrame;
	int _emptyEndFrame;
	int _smokeStartFrame;
	int _smokeEndFrame;
	int _field1C4;
	int _field1C8;
	int _field1CC;
	int _field1D0;
	int _field1D4;
	int _field1D8;
public:
	CLASSDEF;
	CSuccUBus();

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

#endif /* TITANIC_SUCCUBUS_H */
