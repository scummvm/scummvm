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
	static bool _gotSpeechCentre;
	static bool _disabled;
	static int _initialFrame;
private:
	Season _seasonNum;
	CString _arboretumViewName;
	int _unused1;
	int _startFrameSpringOff;
	int _endFrameSpringOff;
	int _startFrameSummerOff;
	int _endFrameSummerOff;
	int _startFrameAutumnOff2;
	int _endFrameAutumnOff2;
	int _startFrameAutumnOff1;
	int _endFrameAutumnOff1;
	int _startFrameWinterOff2;
	int _endFrameWinterOff2;
	int _startFrameWinterOff1;
	int _endFrameWinterOff1;
	int _startFrameSpringOn;
	int _endFrameSpringOn;
	int _startFrameSummerOn;
	int _endFrameSummerOn;
	int _startFrameAutumnOn1;
	int _endFrameAutumnOn1;
	int _startFrameAutumnOn2;
	int _endFrameAutumnOn2;
	int _startFrameWinterOn1;
	int _endFrameWinterOn1;
	int _startFrameWinterOn2;
	int _endFrameWinterOn2;
	CString _exitViewName;
	// German specific fields
	int _field160;
	int _field164;
	int _field168;
	int _field16C;
	int _field170;
	int _field174;
	int _field178;
	int _field17C;
	int _field180;
	int _field184;
	int _field188;
	int _field18C;
	int _field190;
	int _field194;
	int _field198;
	int _field19C;
	int _field1A0;
	int _field1A4;
	int _field1A8;
	int _field1AC;
	int _field1B0;
	int _field1B4;
	int _field1B8;
	int _field1BC;
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
