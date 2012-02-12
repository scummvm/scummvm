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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/str.h"
#include "common/textconsole.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/sprint.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

void fill_box(int x, int y, int dx) {
	g_vm->_screenSurface.fillBox(15, Common::Rect(pred(int, x) << 3, pred(int, y) << 3, 
		pred(int, x + dx) << 3, y << 3));
}

void clear_box(int x, int y, int dx) {
	g_vm->_screenSurface.fillBox(0, Common::Rect(pred(int, x) << 3, pred(int, y) << 3, 
		pred(int, x + dx) << 3, y << 3));
}

} // End of namespace Mortevielle
