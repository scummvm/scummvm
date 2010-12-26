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
		return _vm->_gameState->_globals.currentAge;
	default:
		return MystScriptParser::getVar(var);
	}
}

void MystScriptParser_Intro::o_useLinkBook(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Hard coded SoundId valid only for Intro Stack.
	// Other stacks use Opcode 40, which takes SoundId values as arguments.
	const uint16 soundIdLinkSrc = 5;

	debugC(kDebugScript, "Opcode %d: o_useLinkBook", op);
	debugC(kDebugScript, "\tvar: %d", var);

	// TODO: Merge with changeStack (Opcode 40) Implementation?
	if (getVar(var) == 5 || getVar(var) > 7) {
		// TODO: Dead Book i.e. Released Sirrus/Achenar
	} else {
		// Play Linking Sound, blocking...
		_vm->_sound->stopSound();
		Audio::SoundHandle *handle = _vm->_sound->replaceSound(soundIdLinkSrc);
		while (_vm->_mixer->isSoundHandleActive(*handle))
			_vm->_system->delayMillis(10);

		// Play Flyby Entry Movie on Masterpiece Edition. The Macintosh version is currently hooked
		// up to the Cinepak versions of the video (the 'c' suffix) until the SVQ1 decoder is completed.
		if ((_vm->getFeatures() & GF_ME)) {
			switch (_stackMap[getVar(var)]) {
			case kSeleniticStack:
				if (_vm->getPlatform() == Common::kPlatformMacintosh)
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("FLY_SEc", kMasterpieceOnly));
				else
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("selenitic flyby", kMasterpieceOnly));
				break;
			case kStoneshipStack:
				if (_vm->getPlatform() == Common::kPlatformMacintosh)
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("FLY_STc", kMasterpieceOnly));
				else
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("stoneship flyby", kMasterpieceOnly));
				break;
			// Myst Flyby Movie not used in Original Masterpiece Edition Engine
			case kMystStack:
				if (_vm->_tweaksEnabled) {
					if (_vm->getPlatform() == Common::kPlatformMacintosh)
						_vm->_video->playMovieCentered(_vm->wrapMovieFilename("FLY_MYc", kMasterpieceOnly));
					else
						_vm->_video->playMovieCentered(_vm->wrapMovieFilename("myst flyby", kMasterpieceOnly));
				}
				break;
			case kMechanicalStack:
				if (_vm->getPlatform() == Common::kPlatformMacintosh)
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("FLY_MEc", kMasterpieceOnly));
				else
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("mech age flyby", kMasterpieceOnly));
				break;
			case kChannelwoodStack:
				if (_vm->getPlatform() == Common::kPlatformMacintosh)
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("FLY_CHc", kMasterpieceOnly));
				else
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("channelwood flyby", kMasterpieceOnly));
				break;
			default:
				break;
			}
		}

		uint16 varValue = getVar(var);
		_vm->changeToStack(_stackMap[varValue]);
		_vm->changeToCard(_startCard[varValue], true);

		// TODO: No soundIdLinkDst for Opcode 100 link? Check Original.
	}
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

	_vm->changeToCard(_vm->getCurCard() + 1, true);
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
