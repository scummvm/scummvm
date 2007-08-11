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

void Parallaction_br::splash(const char *name) {

	BackgroundInfo *info;

	_gfx->clearScreen(Gfx::kBitFront);

	info = _disk->loadSlide(name);
	_gfx->setPalette(info->palette);
	_gfx->flatBlitCnv(&info->bg, (640 - info->width) >> 1, (400 - info->height) >> 1, Gfx::kBitFront);
	_gfx->updateScreen();
	_system->delayMillis(600);

	Palette pal;
	for (uint i = 0; i < 64; i++) {
		info->palette.fadeTo(pal, 1);
		_gfx->setPalette(info->palette);
		_gfx->updateScreen();
		_system->delayMillis(20);
	}
	info->bg.free();

	delete info;

	return;
}

void Parallaction_br::initGame() {

	splash("dyna");
	splash("core");

	return;
}

} // namespace Parallaction
