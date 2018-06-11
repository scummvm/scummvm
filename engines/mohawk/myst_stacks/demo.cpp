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

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_stacks/demo.h"

#include "common/system.h"

namespace Mohawk {
namespace MystStacks {

Demo::Demo(MohawkEngine_Myst *vm) :
		Intro(vm, kDemoStack),
		_returnToMenuRunning(false),
		_returnToMenuStep(0),
		_returnToMenuNextTime(0) {
	setupOpcodes();
}

Demo::~Demo() {
}

void Demo::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OVERRIDE_OPCODE(100, Demo, o_stopIntro);
	REGISTER_OPCODE(101, Demo, o_fadeFromBlack);
	REGISTER_OPCODE(102, Demo, o_fadeToBlack);

	// "Init" Opcodes
	OVERRIDE_OPCODE(201, Demo, o_returnToMenu_init);
}

void Demo::disablePersistentScripts() {
	Intro::disablePersistentScripts();

	_returnToMenuRunning = false;
}

void Demo::runPersistentScripts() {
	Intro::runPersistentScripts();

	if (_returnToMenuRunning) {
		returnToMenu_run();
	}
}

void Demo::o_stopIntro(uint16 var, const ArgumentsArray &args) {
	// The original also seems to stop the movies. Not needed with this engine.
	_vm->_gfx->fadeToBlack();
}

void Demo::o_fadeFromBlack(uint16 var, const ArgumentsArray &args) {
	// FIXME: This glitches when enabled. The backbuffer is drawn to screen,
	// and then the fading occurs, causing the background to appear for one frame.
	// _vm->_gfx->fadeFromBlack();
}

void Demo::o_fadeToBlack(uint16 var, const ArgumentsArray &args) {
	_vm->_gfx->fadeToBlack();
}

void Demo::returnToMenu_run() {
	uint32 time = _vm->getTotalPlayTime();

	if (time < _returnToMenuNextTime)
		return;

	switch (_returnToMenuStep) {
	case 0:
		_vm->_gfx->fadeToBlack();
		_vm->changeToCard(2003, kNoTransition);
		_vm->_gfx->fadeFromBlack();

		_returnToMenuStep++;
		break;
	case 1:
		_vm->_gfx->fadeToBlack();
		_vm->changeToCard(2001, kNoTransition);
		_vm->_gfx->fadeFromBlack();
		_vm->_cursor->showCursor();

		_returnToMenuStep++;
		break;
	default:
		break;
	}
}

void Demo::o_returnToMenu_init(uint16 var, const ArgumentsArray &args) {
	// Used on Card 2001, 2002 and 2003
	_returnToMenuNextTime = _vm->getTotalPlayTime() + 5000;
	_returnToMenuRunning = true;
}

} // End of namespace MystStacks
} // End of namespace Mohawk
