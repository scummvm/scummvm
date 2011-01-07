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

#include "mohawk/myst.h"
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_state.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/intro.h"

#include "gui/message.h"

namespace Mohawk {

MystScriptParser_Intro::MystScriptParser_Intro(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();
}

MystScriptParser_Intro::~MystScriptParser_Intro() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_Intro::x, #x))

void MystScriptParser_Intro::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, o_useLinkBook);

	// "Init" Opcodes
	OPCODE(200, o_playIntroMovies);
	OPCODE(201, opcode_201);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
}

#undef OPCODE

void MystScriptParser_Intro::disablePersistentScripts() {
}

void MystScriptParser_Intro::runPersistentScripts() {
}

uint16 MystScriptParser_Intro::getVar(uint16 var) {
	switch(var) {
	case 0:
		if (_globals.currentAge == 9 || _globals.currentAge == 10)
			return 2;
		else
			return _globals.currentAge;
	default:
		return MystScriptParser::getVar(var);
	}
}

void MystScriptParser_Intro::o_useLinkBook(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Hard coded SoundId valid only for Intro Stack.
	// Other stacks use Opcode 40, which takes SoundId values as arguments.
	const uint16 soundIdLinkSrc = 5;
	const uint16 soundIdLinkDst[] = { 2282, 3029, 6396, 7122, 3137, 0, 9038, 5134, 0, 4739, 4741 };

	debugC(kDebugScript, "Opcode %d: o_useLinkBook", op);
	debugC(kDebugScript, "\tvar: %d", var);

	// Change to dest stack
	_vm->changeToStack(_stackMap[_globals.currentAge], _startCard[_globals.currentAge], soundIdLinkSrc, soundIdLinkDst[_globals.currentAge]);
}

void MystScriptParser_Intro::o_playIntroMovies(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// TODO:  Clicking during the intro movies does not stop them and change to Card 5.
	//        This is due to the movies playing blocking, but making them non-blocking causes
	//        the card change here to prevent them playing. Need to move the following to the
	//        opcode_200_run process and wait for all movies to finish playing before the card
	//        change is performed.

	// Play Intro Movies..
	if ((_vm->getFeatures() & GF_ME) && _vm->getPlatform() == Common::kPlatformMacintosh) {
		_vm->_video->playMovieCentered(_vm->wrapMovieFilename("mattel", kIntroStack));
		_vm->_video->playMovieCentered(_vm->wrapMovieFilename("presto", kIntroStack));
	} else
		_vm->_video->playMovieCentered(_vm->wrapMovieFilename("broder", kIntroStack));

	_vm->_video->playMovieCentered(_vm->wrapMovieFilename("cyanlogo", kIntroStack));

	if (!(_vm->getFeatures() & GF_DEMO)) { // The demo doesn't have the intro video
		if ((_vm->getFeatures() & GF_ME) && _vm->getPlatform() == Common::kPlatformMacintosh)
			// intro.mov uses Sorenson, introc uses Cinepak. Otherwise, they're the same.
			_vm->_video->playMovieCentered(_vm->wrapMovieFilename("introc", kIntroStack));
		else
			_vm->_video->playMovieCentered(_vm->wrapMovieFilename("intro", kIntroStack));
	}

	_vm->changeToCard(2, true);
}

void MystScriptParser_Intro::opcode_201(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	_vm->_gfx->copyBackBufferToScreen(Common::Rect(544, 333));
	_vm->_system->updateScreen();
	_vm->_system->delayMillis(4 * 1000);
	_vm->_gfx->copyImageToBackBuffer(4, Common::Rect(544, 333));
	_vm->_gfx->copyBackBufferToScreen(Common::Rect(544, 333));
	_vm->_system->updateScreen();

	MystResourceType6 *resource = static_cast<MystResourceType6 *>(_invokingResource);
	resource->playMovie();
	// TODO: Complete / Fix
}

void MystScriptParser_Intro::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);
	// In the original engine, this opcode stopped Intro Movies if playing,
	// upon card change, but this behavior is now default in this engine.
}

} // End of namespace Mohawk
