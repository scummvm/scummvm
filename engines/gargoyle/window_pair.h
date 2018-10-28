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

#ifndef GARGOYLE_WINDOW_PAIR_H
#define GARGOYLE_WINDOW_PAIR_H

#include "gargoyle/windows.h"

namespace Gargoyle {

/**
 * Pair window
 */
class PairWindow : public Window {
public:
	Window *_child1, *_child2;

	/* split info... */
	glui32 _dir;               ///< winmethod_Left, Right, Above, or Below
	bool _vertical, _backward; ///< flags
	glui32 _division;          ///< winmethod_Fixed or winmethod_Proportional
	Window *_key;              ///< NULL or a leaf-descendant (not a Pair)
	int _keyDamage;            ///< used as scratch space in window closing
	glui32 _size;              ///< size value
	glui32 _wBorder;           ///< winMethod_Border, NoBorder
public:
	/**
	 * Constructor
	 */
	PairWindow(Windows *windows, glui32 method, Window *key, glui32 size);

	/**
	 * Rearranges the window
	 */
	virtual void rearrange(const Rect &box) override;

	/**
	 * Redraw the window
	 */
	virtual void redraw() override;

	virtual void getArrangement(glui32 *method, glui32 *size, Window **keyWin) override;

	virtual void setArrangement(glui32 method, glui32 size, Window *keyWin) override;
};

} // End of namespace Gargoyle

#endif
