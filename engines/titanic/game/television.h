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

#ifndef TITANIC_TELEVISION_H
#define TITANIC_TELEVISION_H

#include "titanic/core/background.h"
#include "titanic/messages/messages.h"
#include "titanic/messages/pet_messages.h"

namespace Titanic {

class CTelevision : public CBackground {
	DECLARE_MESSAGE_MAP
private:
	static int _v1;
	static bool _turnOn;
	static int _v3;
	static int _v4;
	static int _v5;
	static int _v6;
private:
	int _fieldE0;
	int _fieldE4;
	bool _isOn;
	int _fieldEC;
	int _fieldF0;
protected:
	virtual bool LeaveViewMsg(CLeaveViewMsg *msg);
	virtual bool ChangeSeasonMsg(CChangeSeasonMsg *msg);
	virtual bool EnterViewMsg(CEnterViewMsg *msg);
	virtual bool PETUpMsg(CPETUpMsg *msg);
	virtual bool PETDownMsg(CPETDownMsg *msg);
	virtual bool StatusChangeMsg(CStatusChangeMsg *msg);
	virtual bool ActMsg(CActMsg *msg);
	virtual bool PETActivateMsg(CPETActivateMsg *msg);
	virtual bool MovieEndMsg(CMovieEndMsg *msg);
	virtual bool ShipSettingMsg(CShipSettingMsg *msg);
	virtual bool TurnOff(CTurnOff *msg);
	virtual bool TurnOn(CTurnOn *msg);
	virtual bool LightsMsg(CLightsMsg *msg);
public:
	CLASSDEF
	CTelevision();
	static void init();
	static void deinit();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);
};

} // End of namespace Titanic

#endif /* TITANIC_TELEVISION_H */
