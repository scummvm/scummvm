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

#include "mohawk/myst.h"
#include "mohawk/cursors.h"
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/dni.h"

#include "common/system.h"

namespace Mohawk {
namespace MystStacks {

Dni::Dni(MohawkEngine_Myst *vm) :
		MystScriptParser(vm, kDniStack),
		_notSeenAtrus(true),
		_atrusRunning(false),
		_waitForLoop(false),
		_atrusLeft(false),
		_atrusLeftTime(0),
		_loopStart(0),
		_loopEnd(0) {
	setupOpcodes();
}

Dni::~Dni() {
}

void Dni::setupOpcodes() {
	// "Stack-Specific" Opcodes
	REGISTER_OPCODE(100, Dni, NOP);
	REGISTER_OPCODE(101, Dni, o_handPage);

	// "Init" Opcodes
	REGISTER_OPCODE(200, Dni, o_atrus_init);

	// "Exit" Opcodes
	REGISTER_OPCODE(300, Dni, NOP);
}

void Dni::disablePersistentScripts() {
	_atrusRunning = false;
	_waitForLoop = false;
	_atrusLeft = false;
}

void Dni::runPersistentScripts() {
	if (_atrusRunning)
		atrus_run();

	if (_waitForLoop)
		loopVideo_run();

	if (_atrusLeft)
		atrusLeft_run();
}

uint16 Dni::getVar(uint16 var) {
	switch(var) {
	case 0: // Atrus Gone (from across room)
		return _globals.ending == kAtrusLeaves;
	case 1: // Myst Book Status
		if (_globals.ending != kBooksDestroyed)
			return _globals.ending == kForgotPage;
		else
			return 2; // Linkable
	case 2: // Music Type
		if (_notSeenAtrus) {
			_notSeenAtrus = false;
			return _globals.ending != kBooksDestroyed && _globals.heldPage != kWhitePage;
		} else
			return 2;
	default:
		return MystScriptParser::getVar(var);
	}
}

void Dni::o_handPage(uint16 var, const ArgumentsArray &args) {
	// Used in Card 5014 (Atrus)

	// Find Atrus movie
	VideoEntryPtr atrus = _vm->findVideo(_video, kDniStack);

	// Good ending and Atrus asked to give page
	if (_globals.ending == kAtrusWantsPage && atrus && atrus->getTime() > (uint)Audio::Timestamp(0, 6801, 600).msecs()) {
		_globals.ending = kAtrusLeaves;
		_globals.heldPage = kNoPage;
		_vm->setMainCursor(kDefaultMystCursor);

		// Play movie end (atrus leaving)
		atrus->setBounds(Audio::Timestamp(0, 14813, 600), atrus->getDuration());
		atrus->setLooping(false);

		_atrusLeft = true;
		_waitForLoop = false;
		_atrusLeftTime = _vm->getTotalPlayTime();
	}
}

void Dni::atrusLeft_run() {
	if (_vm->getTotalPlayTime() > _atrusLeftTime + 63333) {
		_video = "atrus2";
		_videoPos = Common::Point(215, 77);
		VideoEntryPtr atrus = _vm->playMovie(_video, kDniStack);
		atrus->moveTo(_videoPos.x, _videoPos.y);
		atrus->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 98000, 600));

		_atrusRunning = false;
		_waitForLoop = true;
		_loopStart = 73095;
		_loopEnd = 98000;

		// Good ending
		_globals.ending = kBooksDestroyed;
		_globals.bluePagesInBook = 63;
		_globals.redPagesInBook = 63;

		_atrusLeft = false;
	}
}

void Dni::loopVideo_run() {
	if (!_vm->_video->isVideoPlaying()) {
		VideoEntryPtr atrus = _vm->playMovie(_video, kDniStack);
		atrus->moveTo(_videoPos.x, _videoPos.y);
		atrus->setBounds(Audio::Timestamp(0, _loopStart, 600), Audio::Timestamp(0, _loopEnd, 600));
		atrus->setLooping(true);

		_waitForLoop = false;
	}
}

void Dni::atrus_run() {
	if (_globals.ending == kAtrusLeaves) {
		// Wait for atrus to come back
		_atrusLeft = true;
	} else if (_globals.ending == kAtrusWantsPage) {
		// Atrus asking for page
		if (!_vm->_video->isVideoPlaying()) {
			_video = "atr1page";
			_videoPos = Common::Point(215, 76);
			VideoEntryPtr atrus = _vm->playMovie(_video, kDniStack);
			atrus->moveTo(_videoPos.x, _videoPos.y);
			atrus->setLooping(true);
			atrus->setBounds(Audio::Timestamp(0, 7388, 600), Audio::Timestamp(0, 14700, 600));
		}
	} else if (_globals.ending != kForgotPage && _globals.ending != kBooksDestroyed) {
		if (_globals.heldPage == kWhitePage) {
			_video = "atr1page";
			_videoPos = Common::Point(215, 76);
			VideoEntryPtr atrus = _vm->playMovie(_video, kDniStack);
			atrus->moveTo(_videoPos.x, _videoPos.y);
			atrus->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 14700, 600));

			_waitForLoop = true;
			_loopStart = 7388;
			_loopEnd = 14700;

			// Wait for page
			_globals.ending = kAtrusWantsPage;

		} else {
			_video = "atr1nopg";
			_videoPos = Common::Point(215, 77);
			VideoEntryPtr atrus = _vm->playMovie(_video, kDniStack);
			atrus->moveTo(_videoPos.x, _videoPos.y);
			atrus->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 46175, 600));

			_atrusRunning = false;
			_waitForLoop = true;
			_loopStart = 30656;
			_loopEnd = 46175;

			// Bad ending
			_globals.ending = kForgotPage;
		}
	} else if (!_vm->_video->isVideoPlaying()) {
		VideoEntryPtr atrus = _vm->playMovie("atrwrite", kDniStack);
		atrus->moveTo(215, 77);
		atrus->setLooping(true);
	}
}

void Dni::o_atrus_init(uint16 var, const ArgumentsArray &args) {
	_atrusRunning = true;
}

} // End of namespace MystStacks
} // End of namespace Mohawk
