
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef M4_GUI_GUI_INTERFACE_H
#define M4_GUI_GUI_INTERFACE_H

#include "m4/m4_types.h"

namespace M4 {

class Interface {
public:
	int _arrow = 0;
	int _wait = 0;
	int _look = 0;
	int _grab = 0;
	int _use = 0;

public:
	virtual ~Interface() {}

	virtual void init(int arrow, int wait, int look, int grab, int use);

	virtual void cancel_sentence() = 0;

	virtual bool set_interface_palette(RGB8 *myPalette) = 0;

	/**
	 * Show the wait cursor
	 */
	void showWaitCursor();
};

} // End of namespace M4

#endif
