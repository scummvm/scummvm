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

#ifndef TITANIC_STAR_CONTROL_H
#define TITANIC_STAR_CONTROL_H

#include "titanic/core/game_object.h"
#include "titanic/star_control/star_control_sub1.h"
#include "titanic/star_control/star_control_sub11.h"

namespace Titanic {

class CStarControl : public CGameObject {
private:
	int _fieldBC;
	CStarControlSub1 _sub1;
	CStarControlSub11 _sub11;
	int _field80A0;
	int _field80A4;
	int _field80A8;
	int _field80AC;
	int _field80B0;
public:
	CLASSDEF
	CStarControl();

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

#endif /* TITANIC_STAR_CONTROL_H */
