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

#ifndef TITANIC_PHOTOGRAPH_H
#define TITANIC_PHOTOGRAPH_H

#include "titanic/carry/carry.h"
#include "titanic/messages/messages.h"
#include "titanic/messages/mouse_messages.h"
#include "titanic/messages/pet_messages.h"

namespace Titanic {

class CPhotograph : public CCarry {
	DECLARE_MESSAGE_MAP;
	bool MouseDragEndMsg(CMouseDragEndMsg *msg);
	bool MouseDragStartMsg(CMouseDragStartMsg *msg);
	bool PETGainedObjectMsg(CPETGainedObjectMsg *msg);
	bool ActMsg(CActMsg *msg);
private:
	static int _v1;
private:
	int _field12C;
	int _field130;
public:
	CLASSDEF;
	CPhotograph();

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

#endif /* TITANIC_PHOTOGRAPH_H */
