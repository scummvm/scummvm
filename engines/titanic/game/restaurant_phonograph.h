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

#ifndef TITANIC_RESTAURANT_PHONOGRAPH_H
#define TITANIC_RESTAURANT_PHONOGRAPH_H

#include "titanic/game/phonograph.h"

namespace Titanic {

class CRestaurantPhonograph : public CPhonograph {
	DECLARE_MESSAGE_MAP;
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	bool PhonographPlayMsg(CPhonographPlayMsg *msg);
	bool PhonographStopMsg(CPhonographStopMsg *msg);
	bool PhonographReadyToPlayMsg(CPhonographReadyToPlayMsg *msg);
	bool EjectCylinderMsg(CEjectCylinderMsg *msg);
	bool QueryPhonographState(CQueryPhonographState *msg);
	bool LockPhonographMsg(CLockPhonographMsg *msg);
private:
	bool _isLocked;
	CString _ejectSoundName;
	CString _stopSoundName;
	int _field114;
public:
	CLASSDEF;
	CRestaurantPhonograph();

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

#endif /* TITANIC_RESTAURANT_PHONOGRAPH_H */
