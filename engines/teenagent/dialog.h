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
 */

#ifndef TEENAGENT_DIALOG_H
#define TEENAGENT_DIALOG_H

#include "common/endian.h"
#include "common/str.h"

namespace TeenAgent {

// Text Color Symbols
const byte textColorJohnNoty = 0xd0;
const byte textColorCampGuard = 0xd0;
const byte textColorShockedCaptain = 0xd0;
const byte textColorMark = 0xd1;
const byte textColorCredits = 0xd1;
const byte textColorBankGuard = 0xd7;
const byte textColorGrandpa = 0xd8;
const byte textColorMansionGuard = 0xd9;
const byte textColorMarkEnd = 0xe3;
const byte textColorProfessor = 0xe5;
const byte textColorOldLady = 0xe5;
const byte textColorAnne = 0xe5;
const byte textColorWellEcho = 0xe5;
const byte textColorSonny = 0xe5;
const byte textColorEskimo = 0xe5;
const byte textColorRGBBoss = 0xe7;
const byte textColorGoldDriver = 0xe7;
const byte textColorFortuneTeller = 0xeb;
const byte textColorCaptain = 0xec;
const byte textColorMike = 0xef;
const byte textColorCook = 0xef;
const byte textColorBarman = 0xef;

class Scene;
class TeenAgentEngine;

class Dialog {
public:
	Dialog(TeenAgentEngine *vm) : _vm(vm) { }

	uint16 pop(Scene *scene, uint16 addr, uint16 animation1, uint16 animation2, byte color1, byte color2, byte slot1, byte slot2);

	uint16 popMark(Scene *scene, uint16 addr) {
		return pop(scene, addr, 0, 0, textColorMark, textColorMark, 0, 0);
	}

	void show(uint16 dialogNum, Scene *scene, uint16 animation1, uint16 animation2, byte color1, byte color2, byte slot1, byte slot2);

	void showMono(uint16 dialogNum, Scene *scene, uint16 animation, byte color, byte slot) {
		show(dialogNum, scene, animation, animation, color, color, slot, slot);
	}

	void showMark(uint16 dialogNum, Scene *scene) {
		show(dialogNum, scene, 0, 0, textColorMark, textColorMark, 0, 0);
	}

private:
	TeenAgentEngine *_vm;

	void show(Scene *scene, uint16 addr, uint16 animation1, uint16 animation2, byte color1, byte color2, byte slot1, byte slot2);
};

} // End of namespace TeenAgent

#endif
