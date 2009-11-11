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
 * $URL$
 * $Id$
 */

#ifndef TEENAGENT_DIALOG_H
#define TEENAGENT_DIALOG_H

#include "common/endian.h"
#include "common/str.h"

namespace TeenAgent {

class Scene;
class Dialog {
public:
	static uint16 pop(Scene *scene, uint16 addr, uint16 animation1, uint16 animation2, byte color1, byte color2, byte slot1, byte slot2);
	static uint16 popMark(Scene *scene, uint16 addr) {
		return pop(scene, addr, 0, 0, 0xd1, 0xd1, 0, 0);
	}
	static void show(Scene *scene, uint16 addr, uint16 animation1, uint16 animation2, byte color1, byte color2, byte slot1, byte slot2);
	static void showMono(Scene *scene, uint16 addr, uint16 animation, byte color, byte slot) {
		show(scene, addr, animation, animation, color, color, slot, slot);
	}
	static void showMark(Scene *scene, uint16 addr) {
		show(scene, addr, 0, 0, 0xd1, 0xd1, 0, 0);
	}
};

} // End of namespace TeenAgent

#endif
