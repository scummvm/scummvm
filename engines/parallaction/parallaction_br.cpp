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

#include "common/stdafx.h"
#include "common/system.h"

#include "parallaction/parallaction.h"
#include "parallaction/sound.h"

namespace Parallaction {

int Parallaction_br::init() {

	// Detect game
	if (!detectGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

	_screenWidth = 640;
	_screenHeight = 400;

	if (getGameType() == GType_BRA) {
		if (getPlatform() == Common::kPlatformPC) {
			_disk = new DosDisk_br(this);
		} else
			error("unsupported platform for Big Red Adventure");
	} else
		error("unknown game type");

	_soundMan = new DummySoundMan(this);

	initResources();

	Parallaction::init();

	return 0;
}

void Parallaction_br::callFunction(uint index, void* parm) {
	assert(index < 6);	// magic value 6 is maximum # of callables for Big Red Adventure

	(this->*_callables[index])(parm);
}

int Parallaction_br::go() {

	initGame();

	return 0;
}

void Parallaction_br::initGame() {

	Graphics::Surface* surf = _disk->loadStatic("dyna");
	_gfx->setPalette(_gfx->_palette);
	_gfx->flatBlitCnv(surf, (640 - surf->w) >> 1, (400 - surf->h) >> 1, Gfx::kBitFront);
	_gfx->updateScreen();
	_system->delayMillis(600);

	Palette pal;
	for (uint i = 0; i < 64; i++) {
		_gfx->_palette.fadeTo(pal, 1);
		_gfx->setPalette(_gfx->_palette);
		_gfx->updateScreen();
		_system->delayMillis(30);
	}
	surf->free();
	_gfx->clearScreen(Gfx::kBitFront);

	surf = _disk->loadStatic("core");
	_gfx->setPalette(_gfx->_palette);
	_gfx->flatBlitCnv(surf, (640 - surf->w) >> 1, (400 - surf->h) >> 1, Gfx::kBitFront);
	_gfx->updateScreen();
	_system->delayMillis(2000);

	for (uint i = 0; i < 64; i++) {
		_gfx->_palette.fadeTo(pal, 1);
		_gfx->setPalette(_gfx->_palette);
		_gfx->updateScreen();
		_system->delayMillis(30);
	}
	surf->free();
	_gfx->clearScreen(Gfx::kBitFront);

}

} // namespace Parallaction
