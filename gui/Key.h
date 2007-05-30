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
 * $URL$
 * $Id$
 *
 */

#ifndef GUI_KEY_H
#define GUI_KEY_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"

namespace GUI {

class Key {
public:
	Key(int ascii, int keycode = 0, int flags = 0);
	Key();
	void setAscii(int ascii);
	void setKeycode(int keycode);
	void setFlags(int flags);
	int ascii();
	int keycode();
	int flags();
private:
	int _ascii;
	int _keycode;
	int _flags;
};

} // namespace GUI

#endif
