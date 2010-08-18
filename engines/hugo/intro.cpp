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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/system.h"

#include "hugo/game.h"
#include "hugo/hugo.h"
#include "hugo/intro.h"
#include "hugo/file.h"
#include "hugo/display.h"
#include "hugo/util.h"


namespace Hugo {

IntroHandler::IntroHandler(HugoEngine &vm) : _vm(vm) {
}

IntroHandler::~IntroHandler() {
}

intro_1w::intro_1w(HugoEngine &vm) : IntroHandler(vm) {
}

intro_1w::~intro_1w() {
}

void intro_1w::preNewGame() {
	// Auto-start a new game
	_vm.file().restoreGame(-1);
	_vm.getGameStatus().viewState = V_INTROINIT;
}

void intro_1w::introInit() {
}

bool intro_1w::introPlay() {
	return true;
}

intro_2w::intro_2w(HugoEngine &vm) : IntroHandler(vm) {
}

intro_2w::~intro_2w() {
}

void intro_2w::preNewGame() {
}

void intro_2w::introInit() {
}

bool intro_2w::introPlay() {
	return true;
}

intro_3w::intro_3w(HugoEngine &vm) : IntroHandler(vm) {
}

intro_3w::~intro_3w() {
}

void intro_3w::preNewGame() {
}

void intro_3w::introInit() {
// Hugo 3 - show map and set up for introPlay()
//#if STORY
	_vm.file().readBackground(22); // display screen MAP_3w
	_vm.screen().displayBackground();
	introTicks = 0;
//#endif
}

bool intro_3w::introPlay() {
	byte introSize = _vm.getIntroSize();

// Hugo 3 - Preamble screen before going into game.  Draws path of Hugo's plane.
// Called every tick.  Returns TRUE when complete
//TODO : Add proper check of story mode
//#if STORY
//	SetBkMode(TRANSPARENT);
	if (introTicks < introSize) {
		// Scale viewport x_intro,y_intro to screen (offsetting y)
		_vm.screen().writeChar(_vm._introX[introTicks], _vm._introY[introTicks] - DIBOFF_Y, 'x', _TBRIGHTWHITE);

		// Text boxes at various times
		switch (introTicks) {
		case 4:
			Utils::Box(BOX_OK, _vm._textIntro[kIntro1]);
			break;
		case 9:
			Utils::Box(BOX_OK, _vm._textIntro[kIntro2]);
			break;
		case 35:
			Utils::Box(BOX_OK, _vm._textIntro[kIntro3]);
			break;
		}
	}

	return (++introTicks >= introSize);
//#else //STORY
//	return true;
//#endif //STORY
}

intro_1d::intro_1d(HugoEngine &vm) : IntroHandler(_vm) {
}

intro_1d::~intro_1d() {
}

void intro_1d::preNewGame() {
}

void intro_1d::introInit() {
}

bool intro_1d::introPlay() {
	warning("STUB: intro_1d::introPlay()");
	return true;
}
//TODO : Add code for intro H2 DOS
intro_2d::intro_2d(HugoEngine &vm) : IntroHandler(_vm) {
}

intro_2d::~intro_2d() {
}

void intro_2d::preNewGame() {
}

void intro_2d::introInit() {
}

bool intro_2d::introPlay() {
	return true;
}

//TODO : Add code for intro H3 DOS
intro_3d::intro_3d(HugoEngine &vm) : IntroHandler(_vm) {
}

intro_3d::~intro_3d() {
}

void intro_3d::preNewGame() {
}

void intro_3d::introInit() {
}

bool intro_3d::introPlay() {
	return true;
}

} // end of namespace Hugo

