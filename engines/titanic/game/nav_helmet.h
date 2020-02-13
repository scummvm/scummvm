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

#ifndef TITANIC_NAV_HELMET_H
#define TITANIC_NAV_HELMET_H

#include "titanic/core/game_object.h"
#include "titanic/messages/pet_messages.h"

namespace Titanic {

class CNavHelmet : public CGameObject {
	DECLARE_MESSAGE_MAP;
	bool MovieEndMsg(CMovieEndMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool LeaveViewMsg(CLeaveViewMsg *msg);
	bool PETHelmetOnOffMsg(CPETHelmetOnOffMsg *msg);
	bool PETPhotoOnOffMsg(CPETPhotoOnOffMsg *msg);
	bool PETStarFieldLockMsg(CPETStarFieldLockMsg *msg);
	bool PETSetStarDestinationMsg(CPETSetStarDestinationMsg *msg);
private:
	bool _helmetOn;
public:
	CLASSDEF;
	CNavHelmet() : CGameObject(), _helmetOn(false) {}

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

#endif /* TITANIC_NAV_HELMET_H */
