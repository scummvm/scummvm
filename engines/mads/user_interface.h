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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MADS_USER_INTERFACE_H
#define MADS_USER_INTERFACE_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str.h"
#include "mads/msurface.h"

namespace MADS {

class UserInterface : public MSurface {
private:
	MADSEngine *_vm;

	void loadElements();
public:
	ScrCategory _category;
	int _screenObjectsCount;
	Common::Rect _bounds;
	Common::Rect *_rectP;
	MSurface _surface;
public:
	/**
	* Constructor
	*/
	UserInterface(MADSEngine *vm);

	/**
	* Loads an interface from a specified resource
	*/
	void load(const Common::String &resName);

	/**
	* Set up the interface
	*/
	void setup(int id);

	void elementHighlighted();

	void writeText();

	void setBounds(const Common::Rect &r);
};

} // End of namespace MADS

#endif /* MADS_USER_INTERFACE_H */
