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

#ifndef TITANIC_GONDOLIER_MIXER_H
#define TITANIC_GONDOLIER_MIXER_H

#include "titanic/game/gondolier/gondolier_base.h"
#include "titanic/messages/messages.h"

namespace Titanic {

class CGondolierMixer : public CGondolierBase {
	DECLARE_MESSAGE_MAP;
	bool EnterRoomMsg(CEnterRoomMsg *msg);
	bool LeaveRoomMsg(CLeaveRoomMsg *msg);
	bool TurnOn(CTurnOn *msg);
	bool TurnOff(CTurnOff *msg);
	bool SetVolumeMsg(CSetVolumeMsg *msg);
	bool SignalObject(CSignalObject *msg);
	bool LoadSuccessMsg(CLoadSuccessMsg *msg);
private:
	int _soundHandle1;
	int _soundHandle2;
	int _fieldC4;
	int _fieldC8;
	CString _soundName1;
	CString _soundName2;
	bool _soundActive;
public:
	CLASSDEF;
	CGondolierMixer();

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

#endif /* TITANIC_GONDOLIER_MIXER_H */
