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

#ifndef TITANIC_GONDOLIER_BASE_H
#define TITANIC_GONDOLIER_BASE_H

#include "titanic/core/game_object.h"

namespace Titanic {

class CGondolierBase : public CGameObject {
	DECLARE_MESSAGE_MAP;
	bool PuzzleSolvedMsg(CPuzzleSolvedMsg *msg);
protected:
	static bool _chestOpen;
	static bool _puzzleSolved;
	static int _volume1;
	static int _slider1;
	static int _volume2;
	static int _slider2;
	static bool _leftSliderHooked;
	static bool _rightSliderHooked;
	static bool _priorLeftSliderHooked;
	static bool _priorRightSliderHooked;
public:
	CLASSDEF;

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

#endif /* TITANIC_GONDOLIER_BASE_H */
